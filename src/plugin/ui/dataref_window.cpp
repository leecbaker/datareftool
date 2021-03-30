#include "dataref_window.h"

#include <cassert>
#include <sstream>

#include "container/single_axis_layout.h"
#include "container/scroll.h"
#include "widgets/button.h"
#include "widgets/horizontal_bar.h"
#include "widgets/spacer.h"
#include "widgets/text.h"
#include "widgets/text_field.h"

#include "search/dataref.h"

#include "dataref_edit_panel.h"
#include "search_window.h"


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

DatarefWindow::DatarefWindow(DataRefRecord * drr, std::weak_ptr<SearchWindow> parent_search_window) : drr(drr), parent_search_window(parent_search_window) {
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
        std::shared_ptr<DatarefEditPanel> edit_element_row = std::make_shared<DatarefEditPanel>(drr, element_index);

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


    if(false == edit_rows.empty()) {
        setKeyboardFocusToWidget(edit_rows.front()->getEditField());
        edit_rows.front()->getEditField()->selectAll();
    }
}

void DatarefWindow::draw(Rect visible_bounds) {
    if(displayed_values_updated != drr->getLastUpdateTime()) {
        if(drr->isArray() && !drr->isData()) {
            array_value->setText(drr->getDisplayString(visible_bounds.width()));
        } else {
            current_value->setText("Value: " + drr->getDisplayString(visible_bounds.width() - 30));
        }

        for(std::shared_ptr<DatarefEditPanel> & row: edit_rows) {
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

bool DatarefWindow::keyPress(char key, XPLMKeyFlags flags, uint8_t virtual_key) {
    // no modifiers
    if((flags & xplm_ShiftFlag) == 0 && (flags & xplm_ControlFlag) == 0 && (flags & xplm_OptionAltFlag) == 0) {
        switch(virtual_key) {
            case XPLM_VK_ESCAPE:
                closeWindow();
                {
                    std::shared_ptr<SearchWindow> search_window = parent_search_window.lock();
                    if(search_window) {
                        search_window->selectSearchField();
                    }
                }
                return true;
            default:
                break;
        }
    }

    return Window11::keyPress(key, flags, virtual_key);
}
