#include "dataref_window.h"

#include "container/single_axis_layout.h"
#include "widgets.h"
#include "containers.h"

#include "search/dataref.h"

#include <sstream>

bool checkValueOK(DataRefRecord * drr, const std::string & new_value) {
    if(drr->isDouble()) {
        try {
            std::stold(new_value);
        } catch(std::exception &) {
            return false;
        }
    } else if(drr->isFloat() || drr->isFloatArray()) {
        try {
            std::stof(new_value);
        } catch(std::exception &) {
            return false;
        }
    } else if(drr->isInt() || drr->isIntArray()) {
        try {
            std::stoi(new_value);
        } catch(std::exception &) {
            return false;
        }
    } else if(drr->isData()) {
    }

    return true;
}

void setValue(DataRefRecord * drr, const std::string & new_value, std::optional<int> index) {
    if(drr->isDouble()) {
        assert(!index);
        try {
            double d = std::stold(new_value);
            drr->setDouble(d);
        } catch(std::exception &) {
            return;
        }
    } else if(drr->isFloat()) {
        assert(!index);
        try {
            float f = std::stof(new_value);
            drr->setFloat(f);
        } catch(std::exception &) {
            return;
        }
    } else if(drr->isInt()) {
        assert(!index);
        try {
            int i = std::stoi(new_value);
            drr->setInt(i);
        } catch(std::exception &) {
            return;
        }
    } else if(drr->isFloatArray()) {
        assert(index);
        try {
            float f = std::stof(new_value);
            drr->setFloatArrayElement(f, *index);
        } catch(std::exception &) {
            return;
        }
    } else if(drr->isIntArray()) {
        assert(index);
        try {
            int i = std::stoi(new_value);
            drr->setIntArrayElement(i, *index);
        } catch(std::exception &) {
            return;
        }
    } else if(drr->isData()) {
        assert(!index);
        drr->setData(new_value);
    }
}

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

class DataRefRow: public SingleAxisLayoutContainer {
    DataRefRecord * drr;
    int element_index;

    std::shared_ptr<Widget11TextField> edit_field;
public:
    DataRefRow(DataRefRecord * drr, int element_index)
    : SingleAxisLayoutContainer(SingleAxisLayoutContainer::LayoutAxis::HORIZONTAL)
    , drr(drr)
    , element_index(element_index)
    {
        std::shared_ptr<Widget11Text> index_label;
        index_label = std::make_shared<Widget11Text>();
        index_label->setText(std::to_string(element_index));
        index_label->setMinimumSize({30, 0});

        edit_field = std::make_shared<Widget11TextField>();
        edit_field->setPlaceholder("New value");

        edit_field->setContents(drr->getArrayElementEditString(element_index));

        std::shared_ptr<Widget11Spacer> edit_spacer = std::make_shared<Widget11Spacer>(Size{20, 1});

        std::shared_ptr<Widget11Button> set_button;
        set_button = std::make_shared<Widget11Button>();
        set_button->setLabel("Set");
        set_button->setMinimumSize({40, 0});

        edit_field->setTypeAction([this, set_button, drr]() {
            if(checkValueOK(drr, edit_field->getContents())) {
                set_button->setEnabled(true);
            } else {
                set_button->setEnabled(false);
            }
        });

        std::optional<int> index_in_array_if_array;
        if(drr->isArray()) {
            index_in_array_if_array = element_index;
        }

        std::function<void()> set_value_edit_done = [element_index, this, drr, index_in_array_if_array]() {
            setValue(drr, edit_field->getContents(), index_in_array_if_array);
            
            // immediately read the value back into the field. maybe our write was unsuccessful.
            edit_field->setContents(drr->getArrayElementEditString(element_index));

            edit_field->selectAll();
        };
    
        edit_field->setSubmitAction(set_value_edit_done);
        set_button->setAction(set_value_edit_done);

        edit_field->setEnabled(drr->writable());

        if(index_label) {
            this->add(index_label, false, false);
        }
        this->add(edit_field, true, true);
        this->add(edit_spacer, false, false);
        if(drr->writable()) {
            this->add(set_button, false, false);
        }
    }

    void update() {
        if(false == edit_field->hasKeyboardFocus()) {
            edit_field->setContents(drr->getArrayElementEditString(element_index));
        }
    }
};

DatarefWindow::DatarefWindow(DataRefRecord * drr) : drr(drr) {
    std::shared_ptr<Widget11Text> dr_name = std::make_shared<Widget11Text>();
    dr_name->setText(drr->getName());

    int num_elements = (drr->isArray() && !drr->isData()) ? drr->getArrayLength() : 1;

    current_value = std::make_shared<Widget11Text>();
    current_value->setText("Value:");

    if(num_elements > 0) {
        array_value = std::make_shared<Widget11Text>();
    }

    last_change = std::make_shared<Widget11Text>();
    last_change->setText("Last change: ?\nLast big change: ?");

    std::shared_ptr<Widget11Text> dr_type = std::make_shared<Widget11Text>();

    std::stringstream type_ss;
    type_ss << "Type: " << getType(drr) << "\n";
    type_ss << "Writeable: " << (drr->writable() ? "yes" : "no") << "\n";
    type_ss << "Found in: " << drr->getSource();
    dr_type->setText(type_ss.str());

    std::shared_ptr<SingleAxisLayoutContainer> edit_container = std::make_shared<SingleAxisLayoutContainer>(SingleAxisLayoutContainer::LayoutAxis::VERTICAL);
    for(int element_index = 0; element_index < num_elements; element_index++) {
        std::shared_ptr<DataRefRow> edit_element_row = std::make_shared<DataRefRow>(drr, element_index);

        edit_rows.push_back(edit_element_row);

        edit_container->add(edit_element_row, true, true);
    }

    std::shared_ptr<SingleAxisLayoutContainer> window_container = std::make_shared<SingleAxisLayoutContainer>(SingleAxisLayoutContainer::LayoutAxis::VERTICAL);

    window_container->add(dr_name, false, true);
    window_container->add(current_value, false, true);
    if(array_value) {
        window_container->add(array_value, false, true);
    }
    window_container->add(dr_type, false, true);
    window_container->add(last_change, false, true);

    window_container->add(std::make_shared<Widget11HorizontalBar>(), false, true);

    if(num_elements < 10) {
        window_container->add(edit_container, false, true);
    } else {
        std::shared_ptr<ScrollContainer> edit_scroll_container = std::make_shared<ScrollContainer>();
        edit_scroll_container->setContents(edit_container);
        edit_scroll_container->setMinimumHeight(130);

        window_container->add(edit_scroll_container, true, true);
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

void DatarefWindow::draw(Rect visible_bounds) {
    if(displayed_values_updated != drr->getLastUpdateTime()) {
        if(drr->isArray() && !drr->isData()) {
            array_value->setText(drr->getDisplayString(visible_bounds.width()));
        } else {
            current_value->setText("Value: " + drr->getDisplayString(visible_bounds.width() - 30));
        }

        for(std::shared_ptr<DataRefRow> & row: edit_rows) {
            row->update();
        }

        displayed_values_updated = drr->getLastUpdateTime();
    }

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
