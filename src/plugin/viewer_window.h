#pragma once

#include <json.hpp>

#include "XPLMDefs.h"
#include "XPLMDisplay.h"
#include "XPWidgetDefs.h"

#include "search/search.h"

#include "window_bounds.h"

class RefRecord;
class RefRecords;
class ViewerWindowList;

class ViewerWindow {
    XPWidgetID window = nullptr;
    XPWidgetID regex_toggle_button = nullptr;
    XPWidgetID case_sensitive_button = nullptr;
    XPWidgetID change_filter_button = nullptr;
    XPWidgetID cr_dr_filter_button = nullptr;
    XPWidgetID search_field = nullptr;

    std::unique_ptr<ViewerWindowList> list;

    int drag_start_mouse_x = 0, drag_start_mouse_y = 0;
    int drag_start_window_left = 0, drag_start_window_right = 0;
    int drag_start_window_top = 0, drag_start_window_bottom = 0;
    bool in_resize_left = false;
    bool in_resize_right = false;
    bool in_resize_bottom = false;

    WindowBounds desired_bounds;

    static const int mouse_drag_margin = 7;

    int change_filter_state = 0;	//0, 1, 2 for off, changes, only big changes
    int cr_dr_filter_state = 0;	//0 for dataref only, 1 for CR only, 2 for both
    bool params_changed = false;

    std::shared_ptr<SearchResults> results;

    SearchParams params;
    RefRecords & refs;

    static int viewerWindowCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t);
    static int searchFieldCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t);

    static int filterClickCallback(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t, intptr_t);

public:
    ViewerWindow(const nlohmann::json & window_params, RefRecords & refs);
    ViewerWindow(bool show_dr, bool show_cr, RefRecords & refs);

#ifdef XPLM301
    void setInVr(bool in_vr);
#endif

    ~ViewerWindow();

    void deselectSearchField();

    std::string getSearchText() const;
    
    void setSearchText(const std::string & s);

    intptr_t getSearchSelectionStart() const;
    intptr_t getSearchSelectionStop() const;

    void setSearchSelection(intptr_t start, intptr_t stop);

    void updateChangeButton();
    void updateCrDrFilterButton();

    void update(); //< When results have changed, update the window

    void updateTitle();

    void resize();

    void setCaseSensitive(bool is_case_sensitive);
    void setIsRegex(bool is_regex);
    void setIsChanged(bool is_changed, bool only_big_changes);
    void setCrDrFilter(bool has_datarefs, bool has_commandrefs);

    int getWidth() const;
    int getHeight() const;

    int getX() const; //<left
    int getY() const; //<bottom, ogl coordinates

    nlohmann::json to_json() const;
};
