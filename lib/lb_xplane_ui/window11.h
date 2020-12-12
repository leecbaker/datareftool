#pragma once

#include <memory>
#include <string>
#include <vector>

#include "XPLMDisplay.h"
#include "XPWidgetDefs.h"

#include "dataref.h"

#ifndef XPLM300
#include "XPWidgets.h"
#endif

#include "geometry.h"
#include "layout_object.h"

#include "flight_loop.h"

#define XP10_WIDGET_HEADER_HEIGHT 16
#define XP10_WIDGET_GUTTER_WIDTH 10

class LayoutObject;


// This is an XPLM300 window
class Window11Base {
protected:
    Rect last_frame_window_bounds;
    std::shared_ptr<Window11Base> this_ref;

#ifdef XPLM300
    XPLMWindowID window = nullptr;

    FlightLoopCallback<void> close_detection_callback;
#else
	XPWidgetID widget = nullptr;
    XPWidgetID widget_background = nullptr;
    XPWidgetID widget_container = nullptr;
	static int window_callback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t inParam2);
    static int window_background_callback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t inParam2);
	static int container_callback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t inParam2);
#endif

public:
    bool isVisible() const {
#ifdef XPLM300
        return XPLMGetWindowIsVisible(window);
#else
        return XPIsWidgetVisible(widget);
#endif
    }

    void show() {
#ifdef XPLM300
        XPLMSetWindowIsVisible(window, 1);
#else
        XPShowWidget(widget);
#endif
    }

    void hide() {
#ifdef XPLM300
        XPLMSetWindowIsVisible(window, 0);
#else
        XPHideWidget(widget);
#endif
    }

    void bringToFront() {
#ifdef XPLM300
        XPLMBringWindowToFront(window);
#else
        XPBringRootWidgetToFront(widget);
#endif
    }

    void closeWindow() {
        this_ref.reset();
    }

    int getWidth() const {
        return getWindowBounds().size().width;
    }

    int getHeight() const {
        return getWindowBounds().size().height;
    }

    void setTitle(const std::string & title) {
#ifdef XPLM300
        XPLMSetWindowTitle(window, title.c_str());
#else
		XPSetWidgetDescriptor(widget, title.c_str());
#endif
    }

    void setWindowBounds(Rect new_bounds) {
#ifdef XPLM300
        XPLMSetWindowGeometry(window, new_bounds.left, new_bounds.top, new_bounds.right, new_bounds.bottom);
#else
        XPSetWidgetGeometry(widget, new_bounds.left - XP10_WIDGET_GUTTER_WIDTH, new_bounds.top + XP10_WIDGET_HEADER_HEIGHT + XP10_WIDGET_GUTTER_WIDTH, new_bounds.right + XP10_WIDGET_GUTTER_WIDTH, new_bounds.bottom - XP10_WIDGET_GUTTER_WIDTH);
        XPSetWidgetGeometry(widget_background, new_bounds.left - XP10_WIDGET_GUTTER_WIDTH, new_bounds.top + XP10_WIDGET_GUTTER_WIDTH, new_bounds.right + XP10_WIDGET_GUTTER_WIDTH, new_bounds.bottom - XP10_WIDGET_GUTTER_WIDTH);
        XPSetWidgetGeometry(widget_container, new_bounds.left, new_bounds.top, new_bounds.right, new_bounds.bottom);
#endif
    }

    Rect getWindowBounds() const {
        Rect window_bounds;
#ifdef XPLM300
        XPLMGetWindowGeometry(window, &window_bounds.left, &window_bounds.top, &window_bounds.right, &window_bounds.bottom);
#else
        XPGetWidgetGeometry(widget_container, &window_bounds.left, &window_bounds.top, &window_bounds.right, &window_bounds.bottom);
#endif
        return window_bounds;
    }

    void setWindowSize(int window_width, int window_height) {
        Rect window_bounds = getWindowBounds();
        window_bounds.right = window_bounds.left + window_width;
        window_bounds.top = window_bounds.bottom + window_height;
        setWindowBounds(window_bounds);
    }

    void setWindowSize(Size window_size) {
        setWindowSize(window_size.width, window_size.height);
    }

    // Centered on main screen
    void setWindowCentered() {
        int screen_width, screen_height;
        XPLMGetScreenSize(&screen_width, &screen_height);
        int left = screen_width / 2 - getWidth() / 2;
        int right = screen_width / 2 + getWidth() / 2;
        int top = screen_height / 2 + getHeight() / 2;
        int bottom = screen_height / 2 - getHeight() / 2;
        setWindowBounds(Rect{left, top, right, bottom});
    }

    void setWindowResizeLimits([[maybe_unused]] int min_width, [[maybe_unused]] int min_height, [[maybe_unused]] int max_width, [[maybe_unused]] int max_height) {
#ifdef XPLM300
        XPLMSetWindowResizingLimits(window, min_width, min_height, max_width, max_height);
#else
		/// todo: is this possible?
#endif
    }

    static inline void draw(XPLMWindowID /* inWindowID */, void * inRefcon) {
        Window11Base * pthis = static_cast<Window11Base *>(inRefcon);

        pthis->draw(pthis->getWindowBounds());
    }
    static inline int mouseClick(XPLMWindowID /* inWindowID */, int x, int y, XPLMMouseStatus inMouse, void *inRefcon) {
        Window11Base * pthis = static_cast<Window11Base *>(inRefcon);
        return pthis->mouseClick(Point{x, y}, inMouse) ? 1 : 0;
    }

    static inline void keyPress(XPLMWindowID /* inWindowID */, char inKey, XPLMKeyFlags inFlags, char inVirtualKey, void * inRefcon, int losingFocus) {
        Window11Base * pthis = static_cast<Window11Base *>(inRefcon);
        pthis->keyPress(inKey, inFlags, inVirtualKey, losingFocus);
    }

    static inline XPLMCursorStatus handleCursor(XPLMWindowID /* inWindowID */, int x,int y, void * inRefcon) {
        Window11Base * pthis = static_cast<Window11Base *>(inRefcon);
        return pthis->handleCursor(Point{x, y});
    }

    static inline int handleMouseWheel(XPLMWindowID /* inWindowID */, int x, int y, int wheel, int clicks, void * inRefcon) {
        Window11Base * pthis = static_cast<Window11Base *>(inRefcon);
        return pthis->handleMouseWheel({x, y}, wheel, clicks) ? 1 : 0;
    }

    virtual void draw(Rect draw_bounds);
    virtual bool mouseClick(Point point, XPLMMouseStatus status);
    virtual void keyPress(char key, XPLMKeyFlags flags, uint8_t virtual_key, int losingFocus);
    virtual XPLMCursorStatus handleCursor(Point point);
    virtual bool handleMouseWheel(Point point, int wheel, int clicks);

#if defined(XPLM301)
protected:
    Dataref<bool> currently_in_vr;
    bool isSimInVR();
public:
    void setVR(bool is_vr);
#endif
protected:
    std::shared_ptr<LayoutObject> layout_object;
public:

    Window11Base();
    virtual ~Window11Base();

    void setTopLevelWidget(std::shared_ptr<LayoutObject> new_layout_obj);
};

template <class WindowClass>
class Window11 : public Window11Base {
public:
    template <class ... Params>
    static inline std::shared_ptr<WindowClass> make(Params... params) {
        std::shared_ptr<WindowClass> sp = std::make_shared<WindowClass>(params...);
        sp->this_ref = sp;
        return sp;
    }

    std::shared_ptr<WindowClass> getWindowSharedPtr() {
        return std::dynamic_pointer_cast<WindowClass>(this_ref);
    }
};
