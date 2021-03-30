#pragma once

#include <memory>

#include "window11.h"

#include "search/search.h"

class CommandRefRecord;
class DataRefRecord;
class DatarefEditField;
class RefRecord;
class RefRecords;
class ResultsList;
class ScrollContainer;
class SingleAxisLayoutContainer;
class Widget11Button;
class WidgetContainer;
class Widget11TextField;

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
    std::weak_ptr<DatarefEditField> action_bar_edit_field;
protected:
    std::shared_ptr<Widget11TextField> search_box;

    std::shared_ptr<Widget11Button> filter_regex;
    std::shared_ptr<Widget11Button> filter_case;
    std::shared_ptr<Widget11Button> filter_change;
    std::shared_ptr<Widget11Button> filter_dataref_command;

    std::shared_ptr<ResultsList> selection_list;
    std::shared_ptr<ScrollContainer> list_scroll_container;

    std::shared_ptr<SingleAxisLayoutContainer> window_vertical_container;

    void searchTermChangeHandler();
    void setSelectionAvailable(RefRecord * new_ref_record);

    void updateTitle();

    std::shared_ptr<WidgetContainer> makeCommandActionBar();
    std::shared_ptr<WidgetContainer> makeDatarefActionBar(DataRefRecord * drr);
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

    void copyName();
    void copyValue();
    void actuateCommand(bool active);

    void updateSearch();

    void update();

    void showEditWindow(DataRefRecord * drr);
    void showEditWindow(CommandRefRecord * crr);
    void selectSearchField();

    // Try to set the keyboard focus on the edit row's dataref edit field if available.
    // Return it so that the caller can forward keyboard events if necessary.
    std::shared_ptr<DatarefEditField> setKeyboardFocusEditField();

    virtual bool keyPress(char /* key */, XPLMKeyFlags /* flags */, uint8_t /* virtual_key */) override;
};
