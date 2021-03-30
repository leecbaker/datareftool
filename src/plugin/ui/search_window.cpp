#include "search_window.h"

#include "clipboard.h"

#include "container/scroll.h"
#include "container/single_axis_layout.h"

#include "selectable_list.h"

#include "widgets/button.h"
#include "widgets/spacer.h"
#include "widgets/text_field.h"

#include "commandref_window.h"
#include "dataref_window.h"

#include "search/allrefs.h"

#include "../drt_plugin.h"

#include "dataref_edit_panel.h"

#include <sstream>


std::shared_ptr<WidgetContainer> SearchWindow::makeCommandActionBar() {
    std::shared_ptr<Widget11Button> copy_name_button = std::make_shared<Widget11Button>();
    copy_name_button->setLabel("Copy name");
    copy_name_button->setMinimumSize({90, 0});
    copy_name_button->setButtonStyle(Widget11Button::ButtonStyle::SECONDARY);
    copy_name_button->setAction([this](){
        copyName();
    });

    std::shared_ptr<Widget11Button> actuate_button = std::make_shared<Widget11Button>();
    actuate_button->setLabel("Actuate");
    actuate_button->setMinimumSize({90, 0});
    actuate_button->setPushAction([this](bool active) {
        actuateCommand(active);
    });

    std::shared_ptr<Widget11Button> details_button = std::make_shared<Widget11Button>();
    details_button->setLabel("Details...");
    details_button->setMinimumSize({90, 0});
    details_button->setAction([this](){
        std::shared_ptr<ResultLine> rl_selected = selection_list->getSelection();
        if(nullptr == rl_selected) {
            return;
        }

        RefRecord * rr = rl_selected->getRecord();
        CommandRefRecord * crr = dynamic_cast<CommandRefRecord *>(rr);
        showEditWindow(crr);
    });

    std::shared_ptr<SingleAxisLayoutContainer> command_action_bar = std::make_shared<SingleAxisLayoutContainer>(SingleAxisLayoutContainer::LayoutAxis::HORIZONTAL);
    command_action_bar->add(copy_name_button, false, true);

    command_action_bar->add(std::make_shared<Widget11Spacer>(), true, true);

    command_action_bar->add(actuate_button, false, true);
    command_action_bar->add(details_button, false, true);

    return command_action_bar;
}

std::shared_ptr<WidgetContainer> SearchWindow::makeDatarefActionBar(DataRefRecord * drr) {
    std::shared_ptr<Widget11Button> copy_name_button = std::make_shared<Widget11Button>();
    copy_name_button->setLabel("Copy name");
    copy_name_button->setMinimumSize({90, 0});
    copy_name_button->setButtonStyle(Widget11Button::ButtonStyle::SECONDARY);
    copy_name_button->setAction([this](){
        copyName();
    });
    std::shared_ptr<Widget11Button> copy_value_button = std::make_shared<Widget11Button>();
    copy_value_button->setLabel("Copy value");
    copy_value_button->setMinimumSize({90, 0});
    copy_value_button->setButtonStyle(Widget11Button::ButtonStyle::SECONDARY);
    copy_value_button->setAction([this](){
        copyValue();
    });

    std::shared_ptr<Widget11Button> edit_button = std::make_shared<Widget11Button>();
    edit_button->setLabel(drr->writable() ? "Edit..." : "View...");
    edit_button->setMinimumSize({90, 0});
    edit_button->setAction([this](){
        std::shared_ptr<ResultLine> rl_selected = selection_list->getSelection();
        if(nullptr == rl_selected) {
            return;
        }

        RefRecord * rr = rl_selected->getRecord();
        DataRefRecord * drr = dynamic_cast<DataRefRecord *>(rr);
        if(nullptr != drr) {
            showEditWindow(drr);
        }
    });

    std::shared_ptr<DatarefEditField> edit_field;
    if(false == drr->isArray() && drr->writable()) {
        edit_field = std::make_shared<DatarefEditField>(drr, 0);
        edit_field->setMinimumSize({90, 0});
        action_bar_edit_field = edit_field;
    }

    std::shared_ptr<SingleAxisLayoutContainer> dataref_action_bar = std::make_shared<SingleAxisLayoutContainer>(SingleAxisLayoutContainer::LayoutAxis::HORIZONTAL);
    dataref_action_bar->add(copy_name_button, false, true);
    dataref_action_bar->add(copy_value_button, false, true);

    dataref_action_bar->add(std::make_shared<Widget11Spacer>(), true, true);

    if(edit_field) {
        dataref_action_bar->add(edit_field, false, true);
    }

    dataref_action_bar->add(edit_button, false, true);
    return dataref_action_bar;
}


SearchWindow::SearchWindow(RefRecords & refs)
: refs(refs) {

    results = refs.doSearch(params);

    // Top bar
    std::shared_ptr<SingleAxisLayoutContainer> search_bar;
    {
        search_box = std::make_shared<Widget11TextField>();
        search_box->setPlaceholder("Search terms");
        search_box->setTypeAction(
            [this](){ searchTermChangeHandler();}
        );
        search_box->setMinimumSize({80, 0});

        filter_regex = std::make_shared<Widget11Button>();
        filter_regex->setLabel(".*");
        filter_regex->setMinimumSize({25, 0});
        std::function<void()> regex_callback = [this]() { clickUseRegexButton(); };
        filter_regex->setAction(regex_callback);
        setUseRegex(false);

        filter_case = std::make_shared<Widget11Button>();
        filter_case->setLabel("Aa");
        filter_case->setMinimumSize({25, 0});
        std::function<void()> case_sensitive_callback = [this](){ clickCaseSensitiveButton(); };
        filter_case->setAction(case_sensitive_callback);
        setIsCaseSensitive(false);

        filter_change = std::make_shared<Widget11Button>();
        filter_change->setMinimumSize({25, 0});
        std::function<void()> change_filter_callback = [this]() { clickChangeFilterButton(); };
        filter_change->setAction(change_filter_callback);
        setChangeFilter(ChangeFilterType::ALL);

        filter_dataref_command = std::make_shared<Widget11Button>();
        filter_dataref_command->setMinimumSize({32, 0});
        std::function<void()> dc_callback = [this](){ clickCommandDatarefFilterButton(); };
        filter_dataref_command->setAction(dc_callback);
        setCommandDatarefFilter(CommandDatarefFilterType::ALL);

        search_bar = std::make_shared<SingleAxisLayoutContainer>(SingleAxisLayoutContainer::LayoutAxis::HORIZONTAL);
        search_bar->add(search_box, true, false);
        search_bar->add(filter_regex, false, false);
        search_bar->add(filter_case, false, false);
        search_bar->add(filter_change, false, false);
        search_bar->add(filter_dataref_command, false, false);
    }

    list_scroll_container = std::make_shared<ScrollContainer>();
    {
        selection_list = std::make_shared<ResultsList>(this, results);
        selection_list->setPaddingBetweenElements(0);
        selection_list->setSelectionChangeAction([this](ResultLine * /* old_selection */, ResultLine * new_selection) {
            if(nullptr == new_selection) {
                setSelectionAvailable(nullptr);
            } else {
                setSelectionAvailable(new_selection->getRecord());
            }
        });

        list_scroll_container->setContents(selection_list);
    }

    window_vertical_container = std::make_shared<SingleAxisLayoutContainer>(SingleAxisLayoutContainer::LayoutAxis::VERTICAL);
    window_vertical_container->add(search_bar, false, true);
    window_vertical_container->add(list_scroll_container, true, true);

    updateSearch();

    setTitle("Search");
    setTopLevelWidget(window_vertical_container);
    setWindowSize({400, 300});
}

void SearchWindow::copyName() {
    std::shared_ptr<ResultLine> rl_selected = selection_list->getSelection();
    if(nullptr == rl_selected) {
        return;
    }

    setClipboard(rl_selected->getRecord()->getName());
}

void SearchWindow::copyValue() {
    std::shared_ptr<ResultLine> rl_selected = selection_list->getSelection();
    if(nullptr == rl_selected) {
        return;
    }

    DataRefRecord * drr = dynamic_cast<DataRefRecord *>(rl_selected->getRecord());
    if(nullptr != drr) {
        setClipboard(drr->getEditString());
    }
}

void SearchWindow::setSelectionAvailable(RefRecord * new_ref_record) {

    if(window_vertical_container->widgetCount() == 3) { // remove existing bar
        window_vertical_container->pop_back();
    }

    if(new_ref_record != nullptr) {
        CommandRefRecord * crr = dynamic_cast<CommandRefRecord*>(new_ref_record);
        DataRefRecord * drr = dynamic_cast<DataRefRecord*>(new_ref_record);

        if(nullptr != drr) {
            window_vertical_container->add(makeDatarefActionBar(drr), false, true);
        }

        if(nullptr != crr) {
            window_vertical_container->add(makeCommandActionBar(), false, true);
        }
    }
}

void SearchWindow::searchTermChangeHandler() {
    params.setSearchTerms(search_box->getContents());
    
    results = refs.doSearch(params);
    selection_list->update();
    list_scroll_container->recomputeScroll();
}

std::string SearchWindow::getSearchTerm() const {
    return search_box->getContents();
}

void SearchWindow::setSearchTerm(const std::string & search_term) {
    search_box->setContents(search_term);
    params.setSearchTerms(search_term);
}

void SearchWindow::setChangeFilter(ChangeFilterType new_filter_type) {
    change_filter_mode = new_filter_type;
    switch(change_filter_mode) {
        case ChangeFilterType::ALL:
            filter_change->setLabel("Ch");
            filter_change->setButtonStyle(Widget11Button::ButtonStyle::SECONDARY);
            break;
        case ChangeFilterType::BIG_CHANGES:
            filter_change->setLabel("CH");
            filter_change->setButtonStyle(Widget11Button::ButtonStyle::PRIMARY);
            break;
        case ChangeFilterType::ALL_CHANGES:
            filter_change->setLabel("ch");
            filter_change->setButtonStyle(Widget11Button::ButtonStyle::PRIMARY);
            break;
    }
    params.setChangeDetection(ChangeFilterType::ALL != change_filter_mode, ChangeFilterType::BIG_CHANGES == change_filter_mode);
}

void SearchWindow::setUseRegex(bool new_use_regex) {
    this->use_regex = new_use_regex;
    if(new_use_regex) {
        filter_regex->setButtonStyle(Widget11Button::ButtonStyle::PRIMARY);
    } else {
        filter_regex->setButtonStyle(Widget11Button::ButtonStyle::SECONDARY);
    }
    params.setUseRegex(new_use_regex);
}

void SearchWindow::setIsCaseSensitive(bool case_sensitive_now) {
    this->case_sensitive = case_sensitive_now;
    if(case_sensitive_now) {
        filter_case->setButtonStyle(Widget11Button::ButtonStyle::PRIMARY);
    } else {
        filter_case->setButtonStyle(Widget11Button::ButtonStyle::SECONDARY);
    }
    params.setCaseSensitive(case_sensitive_now);
}

void SearchWindow::setCommandDatarefFilter(CommandDatarefFilterType new_filter_type) {
    command_dataref_mode = new_filter_type;
    switch(command_dataref_mode) {
        case CommandDatarefFilterType::ALL:
            filter_dataref_command->setButtonStyle(Widget11Button::ButtonStyle::SECONDARY);
            filter_dataref_command->setLabel("D+C");
            break;
        case CommandDatarefFilterType::COMMAND:
            filter_dataref_command->setButtonStyle(Widget11Button::ButtonStyle::PRIMARY);
            filter_dataref_command->setLabel("Com");
            break;
        case CommandDatarefFilterType::DATAREF:
            filter_dataref_command->setButtonStyle(Widget11Button::ButtonStyle::PRIMARY);
            filter_dataref_command->setLabel("Dat");
            break;
    }
    params.setIncludeRefs(
        CommandDatarefFilterType::ALL == command_dataref_mode || CommandDatarefFilterType::COMMAND == command_dataref_mode,
        CommandDatarefFilterType::ALL == command_dataref_mode || CommandDatarefFilterType::DATAREF == command_dataref_mode
        );
}

void SearchWindow::clickChangeFilterButton() {
    switch(change_filter_mode) {
        case ChangeFilterType::ALL:
            setChangeFilter(ChangeFilterType::ALL_CHANGES);
            break;
        case ChangeFilterType::ALL_CHANGES:
            setChangeFilter(ChangeFilterType::BIG_CHANGES);
            break;
        case ChangeFilterType::BIG_CHANGES:
            setChangeFilter(ChangeFilterType::ALL);
            break;
    }
    updateSearch();
}

void SearchWindow::clickUseRegexButton() {
    setUseRegex(!use_regex);
    updateSearch();
}

void SearchWindow::clickCaseSensitiveButton() {
    setIsCaseSensitive(!case_sensitive);
    updateSearch();
}

void SearchWindow::clickCommandDatarefFilterButton() {
    switch(command_dataref_mode) {
        case CommandDatarefFilterType::ALL:
            setCommandDatarefFilter(CommandDatarefFilterType::DATAREF);
            break;
        case CommandDatarefFilterType::DATAREF:
            setCommandDatarefFilter(CommandDatarefFilterType::COMMAND);
            break;
        case CommandDatarefFilterType::COMMAND:
            setCommandDatarefFilter(CommandDatarefFilterType::ALL);
            break;
    }
    updateSearch();
}

void SearchWindow::update() {
    updateTitle();
}

void SearchWindow::updateSearch() {
    results = refs.doSearch(params);
    selection_list->update();
    list_scroll_container->recomputeScroll();

    updateTitle();
}

void SearchWindow::updateTitle() { //update title
    std::stringstream window_title;
    window_title << "DataRefTool (" << results->size() << ")";

    setTitle(window_title.str());
}


void SearchWindow::showEditWindow(DataRefRecord * drr) {
    std::weak_ptr<SearchWindow> this_window_backref = std::dynamic_pointer_cast<SearchWindow>(this_ref);
    std::shared_ptr<DatarefWindow> dr_window = DatarefWindow::make(drr, this_window_backref);

    // if this window is popped out into an os window, do the same with the edit window
    std::optional<int> popped_out_monitor = this->getPoppedOutMonitor();
    if(popped_out_monitor) {
        dr_window->setPoppedOutMonitor(*popped_out_monitor);
    }
}

void SearchWindow::showEditWindow(CommandRefRecord * crr) {
    std::weak_ptr<SearchWindow> this_window_backref = std::dynamic_pointer_cast<SearchWindow>(this_ref);
    std::shared_ptr<CommandrefWindow> cr_window = CommandrefWindow::make(crr, this_window_backref);

    // if this window is popped out into an os window, do the same with the edit window
    std::optional<int> popped_out_monitor = this->getPoppedOutMonitor();
    if(popped_out_monitor) {
        cr_window->setPoppedOutMonitor(*popped_out_monitor);
    }
}

void SearchWindow::selectSearchField() {
    setKeyboardFocusToWidget(search_box);
}

std::shared_ptr<DatarefEditField> SearchWindow::setKeyboardFocusEditField() {
    std::shared_ptr<DatarefEditField> edit_field_sp = action_bar_edit_field.lock();

    if(!edit_field_sp) {
        return {};
    }

    setKeyboardFocusToWidget(edit_field_sp);
    return edit_field_sp;
}

bool SearchWindow::keyPress(char key, XPLMKeyFlags flags, uint8_t virtual_key) {
    // no modifiers
    if((flags & xplm_ShiftFlag) == 0 && (flags & xplm_ControlFlag) == 0 && (flags & xplm_OptionAltFlag) == 0) {
        switch(virtual_key) {
            case XPLM_VK_SPACE:
                if((flags & xplm_DownFlag) != 0) {
                    actuateCommand(true);
                }
                if((flags & xplm_UpFlag) != 0) {
                    actuateCommand(false);
                }
                return true;
            default:
                break;
        }
    }

    // command/control shortcuts
    if((flags & xplm_ShiftFlag) == 0 && (flags & xplm_ControlFlag) != 0 && (flags & xplm_OptionAltFlag) == 0) {
        switch(virtual_key) {
            case XPLM_VK_N: // command-N is a standard new-window shortcut
                if((flags & xplm_DownFlag) != 0) {
                    plugin->openSearchWindow();
                }
                return true;
            case XPLM_VK_F: // command-F is a standard find shortcut
            case XPLM_VK_L: // command-L is common in browsers to go to the URL bar
                if((flags & xplm_DownFlag) != 0) {
                    selectSearchField();
                }
                return true;
            case XPLM_VK_C: // normal copy shortcut
                if(flags & xplm_DownFlag) {
                    copyName();
                }
                return true;
            default:
                break;
        }
    }

    // command-option shortcuts- generally for search terms.
    // Some of these match VSCode.
    if((flags & xplm_ShiftFlag) == 0 && (flags & xplm_ControlFlag) != 0 && (flags & xplm_OptionAltFlag) != 0) {
        switch(virtual_key) {
            case XPLM_VK_I: // I = insensitive
                if((flags & xplm_DownFlag) != 0) {
                    clickCaseSensitiveButton();
                }
                return true;
            case XPLM_VK_R: // R = regex
                if((flags & xplm_DownFlag) != 0) {
                    clickUseRegexButton();
                }
                return true;
            case XPLM_VK_S: // S = source
                if((flags & xplm_DownFlag) != 0) {
                    clickCommandDatarefFilterButton();
                }
                return true;
            case XPLM_VK_C: // C = change
                if((flags & xplm_DownFlag) != 0) {
                    clickChangeFilterButton();
                }
                return true;
            default:
                break;
        }
    }
    //command-shift shortcuts. For the case where the normal shortcut has been taken.
    if((flags & xplm_ShiftFlag) != 0 && (flags & xplm_ControlFlag) != 0 && (flags & xplm_OptionAltFlag) == 0) {
        switch(virtual_key) {
            case XPLM_VK_C: // because command-C and command-option-C are taken, this is what we have.
                if((flags & xplm_DownFlag) != 0) {
                    copyValue();
                }
                return true;
            case XPLM_VK_F: // VSCode's command-shift-F for find in all files
                if((flags & xplm_DownFlag) != 0) {
                    selectSearchField();
                }
                return true;
            default:
                break;
        }
    }

    return Window11::keyPress(key, flags, virtual_key);
}

void SearchWindow::actuateCommand(bool active) {
    std::shared_ptr<ResultLine> rl_selected = selection_list->getSelection();
    if(nullptr == rl_selected) {
        return;
    }

    RefRecord * rr = rl_selected->getRecord();
    CommandRefRecord * crr = dynamic_cast<CommandRefRecord *>(rr);
    if(active) {
        crr->commandBegin();
    } else {
        crr->commandEnd();
    }
}