#pragma once

#include <memory>

#include "widgets.h"
#include "window11.h"

#include "selectable_list.h"

#include "search/search.h"

class RefRecords;

enum class ChangeFilterType {
    ALL,
    ALL_CHANGES,
    BIG_CHANGES,
};

enum class DataSourceFilterType {
    ALL,
    AIRCRAFT,
    PLUGIN,
};

enum class CommandDatarefFilterType {
    ALL,
    DATAREF,
    COMMAND,
};

class SearchWindow : public Window11<SearchWindow> {
    ChangeFilterType change_filter_mode = ChangeFilterType::ALL;
    bool use_regex = false;
    bool case_sensitive = false;
    CommandDatarefFilterType command_dataref_mode = CommandDatarefFilterType::ALL;

    SearchParams params;
    RefRecords & refs;

    std::shared_ptr<SearchResults> results;
protected:
    std::shared_ptr<Widget11TextField> search_box;

    std::shared_ptr<Widget11Button> filter_regex;
    std::shared_ptr<Widget11Button> filter_case;
    std::shared_ptr<Widget11Button> filter_change;
    std::shared_ptr<Widget11Button> filter_dataref_command;

    std::shared_ptr<Widget11Button> edit_button;

    std::shared_ptr<ResultsList> selection_list;
    std::shared_ptr<ScrollContainer> list_scroll_container;

    std::shared_ptr<SingleAxisLayoutContainer> command_action_bar;
    std::shared_ptr<SingleAxisLayoutContainer> dataref_action_bar;

    std::shared_ptr<SingleAxisLayoutContainer> window_vertical_container;

    void searchTermChangeHandler();
    void setSelectionAvailable(RefRecord * new_ref_record);

    void updateTitle();
public:
    SearchWindow(RefRecords & refs);

    std::string getSearchTerm() const;
    void setSearchTerm(const std::string & search_term);

    ChangeFilterType getChangeFilter() const { return change_filter_mode; }
    void setChangeFilter(ChangeFilterType new_filter_type);

    bool getUseRegex() const { return use_regex; }
    void setUseRegex(bool use_regex);

    bool getIsCaseSensitive() const { return case_sensitive; }
    void setIsCaseSensitive(bool case_sensitive);

    CommandDatarefFilterType getCommandDatarefFilter() const { return command_dataref_mode; }
    void setCommandDatarefFilter(CommandDatarefFilterType new_filter_type);

    void clickChangeFilterButton();
    void clickUseRegexButton();
    void clickCaseSensitiveButton();
    void clickCommandDatarefFilterButton();

    void updateSearch();

    void update();
};
