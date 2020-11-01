#include "search_window.h"

#include "clipboard.h"

#include "widgets.h"

#include "commandref_window.h"
#include "dataref_window.h"

#include "search/allrefs.h"

#include <sstream>

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
        selection_list = std::make_shared<ResultsList>(results);
        selection_list->setPaddingBetweenElements(0);
        selection_list->setSelectionChangeAction([this](ResultLine * /* old_selection */, ResultLine * new_selection) {
            if(nullptr == new_selection) {
                setSelectionAvailable(false, false);
            } else {
                CommandRefRecord * crr = dynamic_cast<CommandRefRecord*>(new_selection->getRecord());
                DataRefRecord * drr = dynamic_cast<DataRefRecord*>(new_selection->getRecord());
                setSelectionAvailable(bool(drr), bool(crr));
            }
        });

        list_scroll_container->setContents(selection_list);
    }

    {
        std::shared_ptr<Widget11Button> copy_name_button = std::make_shared<Widget11Button>();
        copy_name_button->setLabel("Copy name");
        copy_name_button->setMinimumSize({90, 0});
        copy_name_button->setButtonStyle(Widget11Button::ButtonStyle::SECONDARY);
        copy_name_button->setAction([this](){
            std::shared_ptr<ResultLine> rl_selected = selection_list->getSelection();
            if(nullptr == rl_selected) {
                return;
            }

            setClipboard(rl_selected->getRecord()->getName());
        });
        std::shared_ptr<Widget11Button> copy_value_button = std::make_shared<Widget11Button>();
        copy_value_button->setLabel("Copy value");
        copy_value_button->setMinimumSize({90, 0});
        copy_value_button->setButtonStyle(Widget11Button::ButtonStyle::SECONDARY);
        copy_value_button->setAction([this](){
            std::shared_ptr<ResultLine> rl_selected = selection_list->getSelection();
            if(nullptr == rl_selected) {
                return;
            }

            DataRefRecord * drr = dynamic_cast<DataRefRecord *>(rl_selected->getRecord());
            if(nullptr != drr) {
                setClipboard(drr->getEditString());
            }
        });

        std::shared_ptr<Widget11Button> edit_button = std::make_shared<Widget11Button>();
        edit_button->setLabel("Edit...");
        edit_button->setMinimumSize({90, 0});
        edit_button->setAction([this](){
            std::shared_ptr<ResultLine> rl_selected = selection_list->getSelection();
            if(nullptr == rl_selected) {
                return;
            }

            RefRecord * rr = rl_selected->getRecord();
            DataRefRecord * drr = dynamic_cast<DataRefRecord *>(rr);
            if(nullptr != drr) {
                DatarefWindow::make(drr);
            }
        });

        dataref_action_bar = std::make_shared<SingleAxisLayoutContainer>(SingleAxisLayoutContainer::LayoutAxis::HORIZONTAL);
        dataref_action_bar->add(copy_name_button, false, true);
        dataref_action_bar->add(copy_value_button, false, true);

        dataref_action_bar->add(std::make_shared<Widget11Spacer>(), true, true);

        dataref_action_bar->add(edit_button, false, true);
    }

    {
        std::shared_ptr<Widget11Button> copy_name_button = std::make_shared<Widget11Button>();
        copy_name_button->setLabel("Copy name");
        copy_name_button->setMinimumSize({90, 0});
        copy_name_button->setButtonStyle(Widget11Button::ButtonStyle::SECONDARY);
        copy_name_button->setAction([this](){
            std::shared_ptr<ResultLine> rl_selected = selection_list->getSelection();
            if(nullptr == rl_selected) {
                return;
            }

            setClipboard(rl_selected->getRecord()->getName());
        });

        std::shared_ptr<Widget11Button> once_button = std::make_shared<Widget11Button>();
        once_button->setLabel("Command once");
        once_button->setMinimumSize({90, 0});
        once_button->setAction([this](){
            std::shared_ptr<ResultLine> rl_selected = selection_list->getSelection();
            if(nullptr == rl_selected) {
                return;
            }

            RefRecord * rr = rl_selected->getRecord();
            CommandRefRecord * crr = dynamic_cast<CommandRefRecord *>(rr);
            crr->commandOnce();
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
            CommandrefWindow::make(crr);
        });

        command_action_bar = std::make_shared<SingleAxisLayoutContainer>(SingleAxisLayoutContainer::LayoutAxis::HORIZONTAL);
        command_action_bar->add(copy_name_button, false, true);

        command_action_bar->add(std::make_shared<Widget11Spacer>(), true, true);

        command_action_bar->add(details_button, false, true);
        command_action_bar->add(once_button, false, true);
    }

    window_vertical_container = std::make_shared<SingleAxisLayoutContainer>(SingleAxisLayoutContainer::LayoutAxis::VERTICAL);
    window_vertical_container->add(search_bar, false, true);
    window_vertical_container->add(list_scroll_container, true, true);

    updateSearch();

    setTitle("Search");
    setTopLevelWidget(window_vertical_container);
}

void SearchWindow::setSelectionAvailable(bool dataref_selected, bool command_selected) {
    if(window_vertical_container->widgetCount() == 3) {
        window_vertical_container->pop_back();
    }

    if(dataref_selected) {
        window_vertical_container->add(dataref_action_bar, false, true);
    }

    if(command_selected) {
        window_vertical_container->add(command_action_bar, false, true);
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
