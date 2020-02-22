#include "clipboard.h"
#include "search/commandref.h"
#include "logging.h"
#include "plugin.h"
#include "search/search.h"
#include "util/string_util.h"
#include "viewer_window.h"

#include "viewer_window_list.h"

#include "XPLMDisplay.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMDataAccess.h"
#include "XPLMGraphics.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>

const int bottom_row_height = 20;
const int toggle_button_width = 28;
const int title_bar_height = 16;

const char * KEY_HAS_DR = "has_datarefs";
const char * KEY_HAS_CR = "has_commandrefs";

ViewerWindow::ViewerWindow(bool show_dr, bool show_cr, RefRecords & refs) : ViewerWindow({
        { KEY_HAS_DR, show_dr},
        { KEY_HAS_CR, show_cr}
    }, refs) {}

// Center the window on monitor 0. Ignore all other callbacks.
void getFirstMonitorSize(int inMonitorIndex, int inLeftBx, int inTopBx, int inRightBx, int inBottomBx,  void * inRefcon) {
    if(0 == inMonitorIndex) {
        std::array<int, 4> * coordinates = reinterpret_cast<std::array<int, 4> *>(inRefcon);

        *coordinates = {inLeftBx, inTopBx, inRightBx, inBottomBx};
    }

}

ViewerWindow::ViewerWindow(const nlohmann::json & window_details, RefRecords & refs) : refs(refs) {
    // Decode parameters from json
    std::array<int, 4> screen0_coordinates_boxels;
    XPLMGetAllMonitorBoundsGlobal(getFirstMonitorSize, &screen0_coordinates_boxels);

    int window_width = 500;
    int window_height = 400;
    int l = (screen0_coordinates_boxels[0] + screen0_coordinates_boxels[2]) / 2 - window_width / 2;
    int b = (screen0_coordinates_boxels[1] + screen0_coordinates_boxels[3]) / 2 - window_height / 2;

    bool is_case_sensitive = false;
    bool is_regex = true;
    bool is_changed = false;
    bool is_big_changes = false;
    bool has_datarefs = true;
    bool has_commandrefs = true;

    //extract parameters, if present
    try {
        window_width = window_details.value<int>("window_width", window_width);
        window_height = window_details.value<int>("window_height", window_height);
        l = window_details.value<int>("x", l);
        b = window_details.value<int>("y", b);
        
        is_case_sensitive = window_details.value<bool>("case_sensitive", is_case_sensitive);
        is_regex = window_details.value<bool>("regex", is_regex);
        is_changed = window_details.value<bool>("changed", is_changed);
        is_big_changes = window_details.value<bool>("big_changes_only", is_big_changes);
        has_datarefs = window_details.value<bool>(KEY_HAS_DR, has_datarefs);
        has_commandrefs = window_details.value<bool>(KEY_HAS_CR, has_commandrefs);
    } catch(nlohmann::json::exception) {

    }

    if(false == has_datarefs && false == has_commandrefs) {
        has_datarefs = has_commandrefs = true;
    }

    std::string search_term;
    if(window_details.count("search_term")) {
        try {
            search_term = window_details["search_term"].get<std::string>();
        } catch(nlohmann::json::exception) {

        }
    }

    //////
    window = XPCreateWidget(l, b + window_height, l + window_width, b,
                1,										// Visible
                "Data Ref Tool",	// desc
                1,										// root
                NULL,									// no container
                xpWidgetClass_MainWindow);

    results = refs.doSearch(params);

    XPSetWidgetProperty(window, xpProperty_MainWindowHasCloseBoxes, 1);
    XPSetWidgetProperty(window, xpProperty_MainWindowType, xpMainWindowStyle_Translucent);
    XPAddWidgetCallback(window, viewerWindowCallback);
    XPSetWidgetProperty(window, xpProperty_Object, reinterpret_cast<intptr_t>(this));
    
    list = std::make_unique<ViewerWindowList>(window, results);

    regex_toggle_button = XPCreateWidget(0, 0, 1, 1, 1,".*", 0, window, xpWidgetClass_Button);
    XPSetWidgetProperty(regex_toggle_button, xpProperty_ButtonType, xpPushButton);
    XPSetWidgetProperty(regex_toggle_button, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
    XPSetWidgetProperty(regex_toggle_button, xpProperty_ButtonState, 0);
    XPAddWidgetCallback(regex_toggle_button, filterClickCallback);
    XPSetWidgetProperty(regex_toggle_button, xpProperty_Object, reinterpret_cast<intptr_t>(this));

    case_sensitive_button = XPCreateWidget(0, 0, 1, 1, 1,"Aa", 0, window, xpWidgetClass_Button);
    XPSetWidgetProperty(case_sensitive_button, xpProperty_ButtonType, xpPushButton);
    XPSetWidgetProperty(case_sensitive_button, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
    XPSetWidgetProperty(case_sensitive_button, xpProperty_ButtonState, 0);
    XPAddWidgetCallback(case_sensitive_button, filterClickCallback);
    XPSetWidgetProperty(case_sensitive_button, xpProperty_Object, reinterpret_cast<intptr_t>(this));

    change_filter_button = XPCreateWidget(0, 0, 1, 1, 1,"Ch", 0, window, xpWidgetClass_Button);
    XPSetWidgetProperty(change_filter_button, xpProperty_ButtonType, xpPushButton);
    XPSetWidgetProperty(change_filter_button, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
    XPSetWidgetProperty(change_filter_button, xpProperty_ButtonState, 0);
    XPAddWidgetCallback(change_filter_button, filterClickCallback);
    XPSetWidgetProperty(change_filter_button, xpProperty_Object, reinterpret_cast<intptr_t>(this));
    updateChangeButton();

    cr_dr_filter_button = XPCreateWidget(0, 0, 1, 1, 1,"??", 0, window, xpWidgetClass_Button);
    XPSetWidgetProperty(cr_dr_filter_button, xpProperty_ButtonType, xpPushButton);
    XPSetWidgetProperty(cr_dr_filter_button, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
    XPSetWidgetProperty(cr_dr_filter_button, xpProperty_ButtonState, 0);
    XPAddWidgetCallback(cr_dr_filter_button, filterClickCallback);
    XPSetWidgetProperty(cr_dr_filter_button, xpProperty_Object, reinterpret_cast<intptr_t>(this));
    updateCrDrFilterButton();

    search_field = XPCreateWidget(0, 0, 1, 1, 1,"", 0, window, xpWidgetClass_TextField);
    XPSetWidgetProperty(search_field, xpProperty_TextFieldType, xpTextTranslucent);
    XPAddWidgetCallback(search_field, searchFieldCallback);
    XPSetWidgetProperty(search_field, xpProperty_Object, reinterpret_cast<intptr_t>(this));

    // Clamp window bounds to screen size. This could happen if, e.g.,
    // a window is closed in VR, and then re-opened in non-VR.
    int screen_width = screen0_coordinates_boxels[2] - screen0_coordinates_boxels[0];
    int screen_height = screen0_coordinates_boxels[3] - screen0_coordinates_boxels[1];
    if(l < 0 || window_width < 100 || l + window_width > screen_width || b < 0 || window_height < 100 || window_height > screen_height) {
        setDefaultPosition();
    } else {
        resize();
    }

    params_changed = true;
    list->updateScroll();

    setCaseSensitive(is_case_sensitive);
    setIsRegex(is_regex);
    setIsChanged(is_changed, is_big_changes);
    setCrDrFilter(has_datarefs, has_commandrefs);
    setSearchText(search_term);

    //VR
#ifdef XPLM301
    XPLMDataRef vr_dref = XPLMFindDataRef("sim/graphics/VR/enabled");
    bool vr_enabled = nullptr != vr_dref && XPLMGetDatai(vr_dref);
    if(vr_enabled) {
        setInVr(true);
    }
#endif
    resize();
}

ViewerWindow::~ViewerWindow() {
    XPHideWidget(window);
    XPLMDestroyWindow(window);
}

void ViewerWindow::deselectSearchField() {
    XPLoseKeyboardFocus(search_field);
}

#ifdef XPLM301
void ViewerWindow::setInVr(bool in_vr) {
    XPLMWindowID window_id = XPGetWidgetUnderlyingWindow(window);
    if(in_vr) {
        XPLMSetWindowPositioningMode(window_id, xplm_WindowVR, -1);
    } else {
        XPLMSetWindowPositioningMode(window_id, xplm_WindowPositionFree, -1);
        
        //resize is needed to position the window onscreen. Unfortunately this puts
        //all windows at the same place
        setDefaultPosition();
    }
}
#endif

void ViewerWindow::updateTitle() { //update title
    DataRefRecord * select_edit_dataref = list->getSelectedDataref();
    CommandRefRecord * selected_command = list->getSelectedCommand();

    std::stringstream window_title;
    window_title << "DataRefTool";

    if(select_edit_dataref) {
        window_title << ": edit dataref (";

        if(select_edit_dataref->isInt()) {
            window_title << "int/";
        }
        if(select_edit_dataref->isFloat()) {
            window_title << "float/";
        }
        if(select_edit_dataref->isDouble()) {
            window_title << "double/";
        }
        if(select_edit_dataref->isIntArray()) {
            window_title << "int[" << select_edit_dataref->getArrayLength() << "]/";
        }
        if(select_edit_dataref->isFloatArray()) {
            window_title << "float[" << select_edit_dataref->getArrayLength() << "]/";
        }
        if(select_edit_dataref->isData()) {
            window_title << "data/";
        }

        if(select_edit_dataref->writable()) {
            window_title << "rw";
        } else {
            window_title << "ro";
        }
        window_title << ")";
    } else if(selected_command) {
        window_title << ": edit command";
    } else {
        const std::string & search_term_text = params.getSearchField();
        if(false == search_term_text.empty()) {
            window_title << ": " << search_term_text;
        }

        window_title << " (" << results->size() << ")";

        if(params.invalidRegex()) {
            window_title << " (Invalid regex)";
        }
    }
    XPSetWidgetDescriptor(window, window_title.str().c_str());
}
void ViewerWindow::resize() {
    int left, top, right, bottom;
    XPGetWidgetGeometry(window, &left, &top, &right, &bottom);
    resize(left, top, right, bottom);
}

void ViewerWindow::resize(int left, int top, int right, int bottom) {

    int screen_width, screen_height;
    const constexpr int menu_bar_height = 28;
    XPLMGetScreenSize(&screen_width, &screen_height);
    if(screen_height - top < menu_bar_height) { //xp11 title bar height
        top = screen_height - menu_bar_height;
        bottom = top - 100; //min window height?
        XPSetWidgetGeometry(window, left, top, right, bottom);
    }

    //handle minimum window size
    int min_width = 140;
    int min_height = 60;

    if(top - bottom < min_height) {
        bottom = top - min_height;
        XPSetWidgetGeometry(window, left, top, right, bottom);
    }

    if(right - left < min_width) {
        right = left + min_width;
        XPSetWidgetGeometry(window, left, top, right, bottom);
    }

    top -= title_bar_height;
    left += mouse_drag_margin;
    right -= mouse_drag_margin;
    bottom += mouse_drag_margin;

    XPSetWidgetGeometry(regex_toggle_button, left + 0 * toggle_button_width, bottom + bottom_row_height, left + 1 * toggle_button_width, bottom);
    XPSetWidgetGeometry(case_sensitive_button, left + 1 * toggle_button_width, bottom + bottom_row_height, left + 2 * toggle_button_width, bottom);
    XPSetWidgetGeometry(change_filter_button, left + 2 * toggle_button_width, bottom + bottom_row_height, left + 3 * toggle_button_width, bottom);
    XPSetWidgetGeometry(cr_dr_filter_button, left + 3 * toggle_button_width, bottom + bottom_row_height, left + 4 * toggle_button_width, bottom);
    
    XPSetWidgetGeometry(search_field, left + 4 * toggle_button_width, bottom + bottom_row_height, right, bottom);

    list->resize(left, top, right, bottom + bottom_row_height);

    list->deselectEditField();
    updateTitle();
    list->updateScroll();
}

int ViewerWindow::filterClickCallback(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t, intptr_t) {
    ViewerWindow * obj = reinterpret_cast<ViewerWindow *>(XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr));
    switch(inMessage) {
        case xpMsg_ButtonStateChanged:
            if(inWidget == obj->change_filter_button) {
                obj->change_filter_state = (obj->change_filter_state + 1) % 3;
                obj->updateChangeButton();
            }
            if(inWidget == obj->cr_dr_filter_button) {
                obj->cr_dr_filter_state = (obj->cr_dr_filter_state + 1) % 3;
                obj->updateCrDrFilterButton();
            }
            if(inWidget == obj->case_sensitive_button) {        
                intptr_t property = XPGetWidgetProperty(obj->case_sensitive_button, xpProperty_ButtonState, nullptr);
                obj->params.setCaseSensitive(0 != property);
            }
            if(inWidget == obj->regex_toggle_button) {        
                intptr_t property = XPGetWidgetProperty(obj->regex_toggle_button, xpProperty_ButtonState, nullptr);
                obj->params.setUseRegex(0 != property);
            }
            obj->params_changed = true;
            return 1;
    }
    return 0;
}

int ViewerWindow::searchFieldCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t) {
    ViewerWindow * obj = reinterpret_cast<ViewerWindow *>(XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr));
    XPKeyState_t * keystruct = reinterpret_cast<XPKeyState_t *>(inParam1);
    switch(inMessage) {
        case xpMsg_DescriptorChanged:
            obj->params.setSearchTerms(obj->getSearchText());
            obj->params_changed = true;
            return 1;
        case xpMsg_KeyPress:
            if(keystruct->flags & 6 && keystruct->flags & xplm_DownFlag) {	//alt, command, control are down
                switch(keystruct->vkey) {
                    case XPLM_VK_A:	//select all
                    {
                        size_t length = obj->getSearchText().size();
                        obj->setSearchSelection(0, length);
                        return 1;
                    }
                    case XPLM_VK_X:	//cut
                    {
                        size_t start = obj->getSearchSelectionStart();
                        size_t stop = obj->getSearchSelectionStop();
                        std::string search_text = obj->getSearchText();
                        std::string cut_text = search_text.substr(start, stop - start);
                        search_text.erase(search_text.begin() + start, search_text.begin() + stop);
                        obj->setSearchText(search_text);
                        obj->setSearchSelection(start, start);
                        setClipboard(cut_text);
                        return 1;
                    }
                    case XPLM_VK_C:	//copy 
                    {
                        size_t start = obj->getSearchSelectionStart();
                        size_t stop = obj->getSearchSelectionStop();
                        std::string search_text = obj->getSearchText();
                        std::string cut_text = search_text.substr(start, stop - start);
                        setClipboard(cut_text);
                        return 1;
                    }
                    case XPLM_VK_V:	//paste 
                    {
                        std::string pasted_text = getClipboard();
                        size_t start = obj->getSearchSelectionStart();
                        size_t stop = obj->getSearchSelectionStop();
                        std::string search_text = obj->getSearchText();
                        search_text.replace(search_text.begin() + start, search_text.begin() + stop, pasted_text.begin(), pasted_text.end());
                        obj->setSearchText(search_text);
                        obj->setSearchSelection(start + pasted_text.size(), start + pasted_text.size());
                        return 1;
                    }

                    case XPLM_VK_W:	//close window 
                    {
                        plugin_data->closeViewerWindow(obj);
                        return 1;
                    }
                    case XPLM_VK_N: //new window
                    {
                        plugin_data->showViewerWindow();
                        return 1;
                    }
                }
            } else {
                switch(static_cast<uint8_t>(keystruct->vkey)) {
                    case XPLM_VK_NUMPAD_ENT:
                    case XPLM_VK_ENTER:
                    case XPLM_VK_RETURN:
                    case XPLM_VK_TAB:
                    case XPLM_VK_ESCAPE:
                        obj->deselectSearchField();
                        break;
                    case XPLM_VK_DELETE:
                    case XPLM_VK_BACK:
                        obj->params.setSearchTerms(obj->getSearchText());
                        obj->params_changed = true;
                        break;
                }
            }
    }
    return 0;
}

int ViewerWindow::viewerWindowCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t) {
    XPMouseState_t * mouse_info = reinterpret_cast<XPMouseState_t *>(inParam1);
    ViewerWindow * obj = reinterpret_cast<ViewerWindow *>(XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr));
    switch(inMessage) {
        case xpMsg_MouseWheel:
            obj->list->moveScroll(mouse_info->delta);
            return 1;
        case xpMessage_CloseButtonPushed:
            plugin_data->closeViewerWindow(obj);
            return 1;
        case xpMsg_MouseDown:
            XPGetWidgetGeometry(obj->window, &obj->drag_start_window_left, &obj->drag_start_window_top, &obj->drag_start_window_right, &obj->drag_start_window_bottom);
            if(mouse_info->x < obj->drag_start_window_left + mouse_drag_margin) { obj->in_resize_left = true; }
            if(mouse_info->x > obj->drag_start_window_right - mouse_drag_margin) { obj->in_resize_right = true; }
            if(mouse_info->y < obj->drag_start_window_bottom + mouse_drag_margin) { obj->in_resize_bottom = true; }

            if(obj->in_resize_left || obj->in_resize_right || obj->in_resize_bottom) {
                obj->drag_start_mouse_x = mouse_info->x;
                obj->drag_start_mouse_y = mouse_info->y;
                return 1;
            } else {
                int click_y_offset_from_list_top = obj->drag_start_window_top - mouse_info->y - title_bar_height;
                if(click_y_offset_from_list_top < 0) {	//click in title bar
                    obj->updateTitle();
                    return 0;
                }
            }

            return 0;
        case xpMsg_MouseUp:
            if(!obj->in_resize_left && !obj->in_resize_right && !obj->in_resize_bottom) { break; }
            obj->in_resize_left = false;
            obj->in_resize_right = false;
            obj->in_resize_bottom = false;
            break;
        case xpMsg_MouseDrag:
            if(!obj->in_resize_left && !obj->in_resize_right && !obj->in_resize_bottom) { break; }
            int delta_x = mouse_info->x - obj->drag_start_mouse_x;
            int delta_y = mouse_info->y - obj->drag_start_mouse_y;

            int left_delta = obj->in_resize_left ? delta_x : 0;
            int right_delta = obj->in_resize_right ? delta_x : 0;
            int bottom_delta = obj->in_resize_bottom ? delta_y : 0;
            
            XPSetWidgetGeometry(obj->window, obj->drag_start_window_left + left_delta, obj->drag_start_window_top, obj->drag_start_window_right + right_delta, obj->drag_start_window_bottom + bottom_delta);
            obj->resize();
            return 1;
    }

    return 0;
}

void ViewerWindow::updateChangeButton() {
    switch(change_filter_state) {
        case 0:
            XPSetWidgetDescriptor(change_filter_button, "Ch");
            XPSetWidgetProperty(change_filter_button, xpProperty_ButtonState, 0);
            params.setChangeDetection(false, false);
            break;
        case 1:
            XPSetWidgetDescriptor(change_filter_button, "ch");
            XPSetWidgetProperty(change_filter_button, xpProperty_ButtonState, 1);
            params.setChangeDetection(true, false);
            break;
        case 2:
            XPSetWidgetDescriptor(change_filter_button, "CH");
            XPSetWidgetProperty(change_filter_button, xpProperty_ButtonState, 1);
            params.setChangeDetection(true, true);
            break;
    }
}

void ViewerWindow::updateCrDrFilterButton() {
    switch(cr_dr_filter_state) {
        case 0:
            XPSetWidgetDescriptor(cr_dr_filter_button, "dat");
            XPSetWidgetProperty(cr_dr_filter_button, xpProperty_ButtonState, 1);
            params.setIncludeRefs(false, true);
            break;
        case 1:
            XPSetWidgetDescriptor(cr_dr_filter_button, "com");
            XPSetWidgetProperty(cr_dr_filter_button, xpProperty_ButtonState, 1);
            params.setIncludeRefs(true, false);
            break;
        case 2:
            XPSetWidgetDescriptor(cr_dr_filter_button, "d+c");
            XPSetWidgetProperty(cr_dr_filter_button, xpProperty_ButtonState, 0);
            params.setIncludeRefs(true, true);
            break;
    }
}

int ViewerWindow::getWidth() const {
    int l,r;
    XPGetWidgetGeometry(window, &l, nullptr, &r, nullptr);
    return r - l;
}

int ViewerWindow::getHeight() const {
    int t, b;
    XPGetWidgetGeometry(window, nullptr, &t, nullptr, &b);
    return t - b;
}

//left
int ViewerWindow::getX() const {
    int x;
    XPGetWidgetGeometry(window, &x, nullptr, nullptr, nullptr);
    return x;
}

//bottom, ogl coordinates
int ViewerWindow::getY() const {
    int y;
    XPGetWidgetGeometry(window, nullptr, nullptr, nullptr, &y);
    return y;
}

void ViewerWindow::setCaseSensitive(bool is_case_sensitive) {
    params.setCaseSensitive(is_case_sensitive);
    XPSetWidgetProperty(case_sensitive_button, xpProperty_ButtonState, is_case_sensitive ? 1 : 0);
}

void ViewerWindow::setIsRegex(bool is_regex) {
    params.setUseRegex(is_regex);
    XPSetWidgetProperty(regex_toggle_button, xpProperty_ButtonState, is_regex ? 1 : 0);
}

void ViewerWindow::setIsChanged(bool is_changed, bool only_big_changes) {
    change_filter_state = only_big_changes ? 2 : (is_changed ? 1 : 0);
    updateChangeButton();
}
void ViewerWindow::setCrDrFilter(bool has_datarefs, bool has_commandrefs) {
    if(has_datarefs && has_commandrefs) {
        cr_dr_filter_state = 2;
    } else if(has_datarefs) {
        cr_dr_filter_state = 0;
    } else {
        cr_dr_filter_state = 1;
    }
    updateCrDrFilterButton();
}

nlohmann::json ViewerWindow::to_json() const {
    nlohmann::json window = {
        {"window_height", getHeight()},
        {"window_width", getWidth()},
        {"x", getX()},
        {"y", getY()},
        {"case_sensitive", params.isCaseSensitive()},
        {"regex", params.useRegex()},
        {"changed", params.useChangeDetection()},
        {"big_changes_only", params.useOnlyLargeChanges()},
        {"search_term", params.getSearchField()},
    };
    return window;
}

/////////////////

void ViewerWindow::update() {
    if(params_changed) {
        results = refs.doSearch(params);
        list->deselectEditField();
        params_changed = false;
    }

    list->updateScroll();
    updateTitle();
}

std::string ViewerWindow::getSearchText() const {
    char searchfield_text[1024];
    XPGetWidgetDescriptor(search_field, searchfield_text, 1024);
    return std::string(searchfield_text);
}

void ViewerWindow::setSearchText(const std::string & s) {
    XPSetWidgetDescriptor(search_field, s.c_str());
}

intptr_t ViewerWindow::getSearchSelectionStart() const {
    return XPGetWidgetProperty(search_field, xpProperty_EditFieldSelStart, NULL);
}

intptr_t ViewerWindow::getSearchSelectionStop() const {
    return XPGetWidgetProperty(search_field, xpProperty_EditFieldSelEnd, NULL);
}

void ViewerWindow::setSearchSelection(intptr_t start, intptr_t stop) {
    XPSetWidgetProperty(search_field, xpProperty_EditFieldSelStart, start);
    XPSetWidgetProperty(search_field, xpProperty_EditFieldSelEnd, stop);
}
