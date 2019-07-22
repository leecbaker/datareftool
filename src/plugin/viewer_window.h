#pragma once

#include <json.hpp>

#include "XPLMDefs.h"
#include "XPLMDisplay.h"
#include "XPWidgetDefs.h"

#include "search.h"

class RefRecord;
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

    static const int mouse_drag_margin = 7;

    int change_filter_state = 0;	//0, 1, 2 for off, changes, only big changes
    int cr_dr_filter_state = 0;	//0 for dataref only, 1 for CR only, 2 for both
    bool params_changed = false;

    std::vector<RefRecord *> refs;

    SearchParams params;

    static int viewerWindowCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t);
    static int searchFieldCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t);

    static int filterClickCallback(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t, intptr_t);

    static int drawListCallback(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t, intptr_t);

public:
    ViewerWindow(int l, int t, int r, int b);

    void setDefaultPosition() {
        int width, height;
        XPLMGetScreenSize(&width, &height);
        const int window_width = 500;
        const int window_height = 400;
        const int left = width/2 - window_width / 2;
        const int top = height / 2 + window_height / 2;
        const int right = width / 2 + window_width / 2;
        const int bottom = height/2 - window_height / 2;
        resize(left, top, right, bottom);
    }

    void setInVr(bool in_vr);

    ~ViewerWindow();

    void deselectSearchField();

    std::string getSearchText() const;
    
    void setSearchText(const std::string & s);

    intptr_t getSearchSelectionStart() const;

    intptr_t getSearchSelectionStop() const;

    void setSearchSelection(intptr_t start, intptr_t stop);

    void updateChangeButton();
    void updateCrDrFilterButton();

    void doSearch(const std::vector<RefRecord *> & new_refs, std::vector<RefRecord *> & changed_crs, std::vector<RefRecord *> & changed_drs);

    void updateTitle();
    void draw();

    void resize();
    void resize(int left, int top, int right, int bottom);

    const SearchParams & getSearchParams() const { return params; }
    
    void setCaseSensitive(bool is_case_sensitive);
    void setIsRegex(bool is_regex);
    void setIsChanged(bool is_changed, bool only_big_changes);
    void setCrDrFilter(bool has_datarefs, bool has_commandrefs);
    
    int getWidth() const;
    
    int getHeight() const;
    
    //left
    int getX() const;
    
    //bottom, ogl coordinates
    int getY() const;
};

void showViewerWindow(bool show_dr, bool show_cr);
void showViewerWindow(const nlohmann::json & window_details = {});
nlohmann::json getViewerWindowsDetails();

class RefRecord;
void updateWindowsPerFrame(const std::vector<RefRecord *> & new_refs, std::vector<RefRecord *> & changed_crs, std::vector<RefRecord *> & changed_drs);
void closeViewerWindow(const ViewerWindow * window);
void closeViewerWindows();
size_t countViewerWindows();

void setAllWindowsInVr(bool in_vr);
