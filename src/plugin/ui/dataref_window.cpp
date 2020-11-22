#include "dataref_window.h"

#include "container/single_axis_layout.h"
#include "widgets.h"

#include "search/dataref.h"

#include <sstream>


#include <boost/algorithm/string.hpp>


template <typename T>
inline T parseElement(const std::string &s);

template <>
inline float parseElement<float>(const std::string &s) {
    return std::stof(s);
}
template <>
inline int parseElement<int>(const std::string &s) {
    return std::stoi(s);
}

template <class T>
bool parseArray(const std::string & txt, std::vector<T> & data_out, int length) {
    std::string trimmed_txt = txt;
    if(trimmed_txt.front() == '[') {
        trimmed_txt.erase(trimmed_txt.begin());
    }
    
    if(trimmed_txt.back() == '[') {
        trimmed_txt.pop_back();
    }

    std::vector<std::string> txt_fields;
    boost::split(txt_fields, trimmed_txt, boost::is_any_of(","));
    
    if(length != int(txt_fields.size())) {
        return false;
    }
    
    data_out.clear();
    data_out.reserve(length);
    
    for(const std::string & txt_field : txt_fields) {
        try {
            data_out.push_back(parseElement<T>(txt_field));
        } catch (std::exception &) {
            return false;
        }
    }
    
    return true;
}

template
bool parseArray<float>(const std::string & txt, std::vector<float> & data_out, int length);

template
bool parseArray<int>(const std::string & txt, std::vector<int> & data_out, int length);


std::string getType(DataRefRecord * dr) {
    std::vector<std::string> type_list;
    if(dr->isDouble()) {
        type_list.push_back("double");
    }
    if(dr->isFloat()) {
        type_list.push_back("float");
    }
    if(dr->isInt()) {
        type_list.push_back("int");
    }
    if(dr->isFloatArray()) {
        int array_length = dr->getArrayLength();
        type_list.push_back("float[" + std::to_string(array_length) + "]");
    }
    if(dr->isIntArray()) {
        int array_length = dr->getArrayLength();
        type_list.push_back("int[" + std::to_string(array_length) + "]");
    }
    if(dr->isData()) {
        int array_length = dr->getArrayLength();
        type_list.push_back("data[" + std::to_string(array_length) + "]");
    }

    std::stringstream ss;
    for(const std::string & st: type_list) {
        ss << st;
        if(st != type_list.back()) {
            ss << ", ";
        }
    }

    return ss.str();
}

DatarefWindow::DatarefWindow(DataRefRecord * drr) : drr(drr) {
    std::shared_ptr<Widget11Text> dr_name = std::make_shared<Widget11Text>();
    dr_name->setText(drr->getName());

    current_value = std::make_shared<Widget11Text>();
    current_value->setText("Current value");

    last_change = std::make_shared<Widget11Text>();
    last_change->setText("Last change: ?\nLast big change: ?");

    std::shared_ptr<Widget11Text> dr_type = std::make_shared<Widget11Text>();

    std::stringstream type_ss;
    type_ss << "Type: " << getType(drr) << "\nWriteable: " << (drr->writable() ? "yes" : "no");
    dr_type->setText(type_ss.str());

    std::shared_ptr<SingleAxisLayoutContainer> edit_container;
    {

        edit_field = std::make_shared<Widget11TextField>();
        edit_field->setPlaceholder("New value");
        edit_field->setTypeAction([this]() { checkValueOK();});
        edit_field->setSubmitAction([this](){
            setValue();
        });

        edit_field->setContents(drr->getEditString());

        std::shared_ptr<Widget11Spacer> edit_spacer = std::make_shared<Widget11Spacer>(Size{20, 1});

        set_button = std::make_shared<Widget11Button>();
        set_button->setLabel("Set");
        set_button->setAction([this](){this->closeWindow();});
        set_button->setMinimumSize({40, 0});

        set_button->setAction([this]() {
            setValue();
        });

        edit_container = std::make_shared<SingleAxisLayoutContainer>(SingleAxisLayoutContainer::LayoutAxis::HORIZONTAL);

        edit_container->add(edit_field, true, false);
        edit_container->add(edit_spacer, false, false);
        edit_container->add(set_button, false, false);
    }

    std::shared_ptr<SingleAxisLayoutContainer> window_container = std::make_shared<SingleAxisLayoutContainer>(SingleAxisLayoutContainer::LayoutAxis::VERTICAL);

    window_container->add(dr_name, false, true);
    window_container->add(current_value, false, true);
    window_container->add(dr_type, false, true);
    window_container->add(last_change, false, true);

    if(drr->writable()) {
        window_container->add(std::make_shared<Widget11HorizontalBar>(), false, true);
        window_container->add(edit_container, false, true);
    }

    std::string title = drr->getName();
    size_t last_slash = title.rfind("/");
    if(std::string::npos != last_slash) {
        title.erase(title.cbegin(), title.cbegin() + last_slash + 1);
    }

    setTitle("Dataref: " + title);
    setTopLevelWidget(window_container);
    setWindowCentered();
}

bool DatarefWindow::checkValueOK() const {
    std::string new_value = edit_field->getContents();

    if(drr->isDouble()) {
        try {
            std::stold(new_value);
        } catch(std::exception &) {
            set_button->setEnabled(false);
            return false;
        }
    } else if(drr->isFloat()) {
        try {
            std::stof(new_value);
        } catch(std::exception &) {
            set_button->setEnabled(false);
            return false;
        }
    } else if(drr->isInt()) {
        try {
            std::stoi(new_value);
        } catch(std::exception &) {
            set_button->setEnabled(false);
            return false;
        }
    } else if(drr->isFloatArray()) {
        std::vector<float> array;
        if(false == parseArray<float>(new_value, array, drr->getArrayLength())) {
            set_button->setEnabled(false);
            return false;
        }
    } else if(drr->isIntArray()) {
        std::vector<int> array;
        if(false == parseArray<int>(new_value, array, drr->getArrayLength())) {
            set_button->setEnabled(false);
            return false;
        }
    } else if(drr->isData()) {
    }

    // enable / disable the set button
    set_button->setEnabled(true);
    return true;
}

void DatarefWindow::setValue() {
    std::string new_value = edit_field->getContents();

    if(drr->isDouble()) {
        try {
            double d = std::stold(new_value);
            drr->setDouble(d);
        } catch(std::exception &) {
            return;
        }
    } else if(drr->isFloat()) {
        try {
            float f = std::stof(new_value);
            drr->setFloat(f);
        } catch(std::exception &) {
            return;
        }
    } else if(drr->isInt()) {
        try {
            int i = std::stoi(new_value);
            drr->setInt(i);
        } catch(std::exception &) {
            return;
        }
    } else if(drr->isFloatArray()) {
        std::vector<float> array;
        if(false == parseArray<float>(new_value, array, drr->getArrayLength())) {
            return;
        }
        drr->setFloatArray(array);
    } else if(drr->isIntArray()) {
        std::vector<int> array;
        if(false == parseArray<int>(new_value, array, drr->getArrayLength())) {
            return;
        }
        drr->setIntArray(array);
    } else if(drr->isData()) {
        drr->setData(new_value);
    }
}

void DatarefWindow::draw(Rect visible_bounds) {
    current_value->setText("Value: " + drr->getDisplayString(120));

    std::stringstream ss;
    ss << "Last change     : ";
    if(0 == drr->getLastUpdateTime().time_since_epoch().count()) {
        ss << "Never";
    } else {
        std::chrono::duration time_since_change = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - drr->getLastUpdateTime());
        int seconds_since_change = time_since_change.count();
        ss << seconds_since_change << "s ago";
    }

    ss << "\nLast big change: ";
    if(0 == drr->getLastBigUpdateTime().time_since_epoch().count()) {
        ss << "Never";
    } else {
        std::chrono::duration time_since_change = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - drr->getLastBigUpdateTime());
        int seconds_since_change = time_since_change.count();
        ss << seconds_since_change << "s ago";
    }

    last_change->setText(ss.str());

    Window11<DatarefWindow>::draw(visible_bounds);
}
