#include "window11.h"

#include "XPWidgets.h"
#include "XPLMGraphics.h"

#include "gl_utils.h"
#include "logging.h"

#include "container/container.h"

#ifndef XPLM300
#include "XPStandardWidgets.h"
#endif

#include "widgets/widget.h"

Window11Base::Window11Base() 
: last_frame_window_bounds{-1, -1, -1, -1}
#ifdef XPLM300
, close_detection_callback([this]() -> float {
    if(false == isVisible()) {
        closeWindow();
    }
    return 1.f;
})
#endif
#ifdef XPLM301
, currently_in_vr("sim/graphics/VR/enabled")
#endif
{
#ifdef XPLM300
    XPLMCreateWindow_t window_params = {};
    window_params.structSize = sizeof(window_params);
    window_params.left = 0;
    window_params.right = 500;
    window_params.top = 800;
    window_params.bottom = 500;
    window_params.visible = 1;

    window_params.drawWindowFunc = Window11Base::draw;
    window_params.handleMouseClickFunc = Window11Base::mouseClick;
    window_params.handleKeyFunc = Window11Base::keyPress;
    window_params.handleCursorFunc = Window11Base::handleCursor;
    window_params.handleMouseWheelFunc = Window11Base::handleMouseWheel;
    window_params.refcon = static_cast<void *>(this);
    window_params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;
    window_params.layer = xplm_WindowLayerFloatingWindows;

    // window_params.handleRightClickFunc
    window = XPLMCreateWindowEx(&window_params);

    close_detection_callback.schedule(1.f);

#if defined(XPLM301)
    if(isSimInVR()) {
        setVR(true);
    }
#endif
#else
    int width, height;
    XPLMGetScreenSize(&width, &height);
    widget = XPCreateWidget(width / 2 - 400, height / 2 + 200, width / 2 + 400, height / 2 - 200, 1, "window title", 1, nullptr, xpWidgetClass_MainWindow);
    XPSetWidgetProperty(widget, xpProperty_MainWindowType, xpMainWindowStyle_MainWindow);
    XPSetWidgetProperty(widget, xpProperty_MainWindowHasCloseBoxes, 1);
    XPAddWidgetCallback(widget, &Window11Base::window_callback);
    XPSetWidgetProperty(widget, xpProperty_Refcon, reinterpret_cast<intptr_t>(this));


    widget_background = XPCreateCustomWidget(width / 2 - 400, height / 2 + 200, width / 2 + 400, height / 2 - 200, 1, "background", 0, widget, &Window11Base::window_background_callback);
    XPSetWidgetProperty(widget_container, xpProperty_Refcon, reinterpret_cast<intptr_t>(this));

    widget_container = XPCreateCustomWidget(width / 2 - 400, height / 2 + 200, width / 2 + 400, height / 2 - 200, 1, "widget title", 0, widget, &Window11Base::container_callback);
    XPSetWidgetProperty(widget_container, xpProperty_Refcon, reinterpret_cast<intptr_t>(this));
#endif

#ifdef XPLM300
    XPLMTakeKeyboardFocus(window);
#else
    XPSetKeyboardFocus(widget_container);
#endif
}  

#ifndef XPLM300
int Window11Base::window_callback(XPWidgetMessage inMessage, [[maybe_unused]] XPWidgetID inWidget, [[maybe_unused]] intptr_t inParam1, [[maybe_unused]] intptr_t inParam2) {
    intptr_t refcon = XPGetWidgetProperty(inWidget, xpProperty_Refcon, nullptr);
    Window11Base * pthis = reinterpret_cast<Window11Base *>(refcon);

    switch (inMessage) {
        case xpMessage_CloseButtonPushed:
            pthis->closeWindow();
            return 1;

        // this is how we resize a window in XP10. I think we can do without it now
        case xpMsg_MouseDown:
        case xpMsg_MouseUp:
        case xpMsg_MouseDrag:
            return 0;
    }
    return 0;
}

int Window11Base::window_background_callback(XPWidgetMessage inMessage, [[maybe_unused]] XPWidgetID inWidget, [[maybe_unused]] intptr_t inParam1, [[maybe_unused]] intptr_t inParam2) {
    // intptr_t refcon = XPGetWidgetProperty(inWidget, xpProperty_Refcon, nullptr);
    // Window11Base * pthis = reinterpret_cast<Window11Base *>(refcon);

    switch (inMessage) {
        case xpMsg_Draw: {
            int left, top, right, bottom;
            XPGetWidgetGeometry(inWidget, &left, &top, &right, &bottom);
            XPLMSetGraphicsState(0,0,0,0,1,0,0);
            color3fGL(color3fFromHex(0x26, 0x32, 0x40));
            glBegin(GL_TRIANGLE_STRIP);
            glVertex3f(right, top, 0);
            glVertex3f(right, bottom, 0);
            glVertex3f(left, top, 0);
            glVertex3f(left, bottom, 0);
            glEnd();
        }
            return 1;
    }
    return 0;
}

int Window11Base::container_callback(XPWidgetMessage inMessage, [[maybe_unused]] XPWidgetID inWidget, [[maybe_unused]] intptr_t inParam1, [[maybe_unused]] intptr_t inParam2) {
    intptr_t refcon = XPGetWidgetProperty(inWidget, xpProperty_Refcon, nullptr);
    Window11Base * pthis = reinterpret_cast<Window11Base *>(refcon);
    switch (inMessage) {
        case xpMsg_Draw:
            pthis->draw(pthis->getWindowBounds());
            return 0;
        case xpMsg_MouseDown: {
            XPMouseState_t * mouse = reinterpret_cast<XPMouseState_t *>(inParam1);
            if(pthis->mouseClick({mouse->x, mouse->y}, xplm_MouseDown)) {
                return 1;
            }
        }
        case xpMsg_MouseUp: {
            XPMouseState_t * mouse = reinterpret_cast<XPMouseState_t *>(inParam1);
            if(pthis->mouseClick({mouse->x, mouse->y}, xplm_MouseUp)) {
                return 1;
            }
        }
        case xpMsg_MouseDrag: {
            XPMouseState_t * mouse = reinterpret_cast<XPMouseState_t *>(inParam1);
            if(pthis->mouseClick({mouse->x, mouse->y}, xplm_MouseDrag)) {
                return 1;
            }
        }
        case xpMsg_MouseWheel: {
            XPMouseState_t * mouse = reinterpret_cast<XPMouseState_t *>(inParam1);
            if(pthis->handleMouseWheel({mouse->x, mouse->y}, 0, mouse->delta)) {
                return 1;
            }
        }
        case xpMsg_KeyPress: {
            const XPKeyState_t * key_state = reinterpret_cast<const XPKeyState_t *>(inParam1);
            pthis->keyPress(key_state->key, key_state->flags, key_state->vkey, 0);
            return 1;
        }

        case xpMsg_KeyTakeFocus:    // we don't have to do anything, but we do have to tell xplane that we accept
            return 1;

        case xpMsg_KeyLoseFocus:
            pthis->keyPress(1, 0, 0, 1);
            return 1;
    }
    return 0;
}
#endif

Window11Base::~Window11Base() {
#ifdef XPLM300
    XPLMDestroyWindow(window);
#else
    XPDestroyWidget(widget_container, 0);
    XPDestroyWidget(widget_background, 0);
    XPDestroyWidget(widget, 1);
#endif
}


std::optional<int> Window11Base::getPoppedOutMonitor() const {
#ifdef XPLM300
    if(XPLMWindowIsPoppedOut(window)) {
        //TODO: it's complicated to figure out which monitor a window is on.
        // This is such a rare case, and I don't have time to work on it.
        return 0;
    }
#endif
    return std::nullopt;
}

void Window11Base::setPoppedOutMonitor([[maybe_unused]] int monitor_number) {
#ifdef XPLM300
    XPLMSetWindowPositioningMode(window, xplm_WindowPopOut, monitor_number);
#endif
}

void Window11Base::draw(Rect draw_bounds) {
    Rect current_window_bounds = getWindowBounds();

    if(current_window_bounds != last_frame_window_bounds) {
        layout_object->setBounds(current_window_bounds);
        last_frame_window_bounds = current_window_bounds;
    }

    layout_object->draw(draw_bounds);
}

void Window11Base::setKeyboardFocusToWidget(std::shared_ptr<Widget11> new_focus_widget) {
#ifdef XPLM300
    XPLMTakeKeyboardFocus(window);
#else
    XPSetKeyboardFocus(widget_container);
#endif

    if(layout_object->hasKeyboardFocus()) {
        layout_object->removeKeyboardFocus();
    }

    if(new_focus_widget->acceptsKeyboardFocus()) {
        new_focus_widget->giveKeyboardFocus();
    }
}

bool Window11Base::mouseClick(Point point, XPLMMouseStatus status) {
    std::shared_ptr<Widget11> click_handler;

    std::shared_ptr<WidgetContainer> container = std::dynamic_pointer_cast<WidgetContainer>(layout_object);
    std::shared_ptr<Widget11> layout_widget = std::dynamic_pointer_cast<Widget11>(layout_object);
    if(container) {
        click_handler = container->mouseClick(point, status);
    } else if(layout_widget) {
        if(layout_widget->mouseClick(point, status)) {
            click_handler = layout_widget;
        }
    }

    if(click_handler) {
        setKeyboardFocusToWidget(click_handler);
    } else {
    #ifdef XPLM300
        XPLMTakeKeyboardFocus(window);
    #else
        XPSetKeyboardFocus(widget_container);
    #endif
    }
    
    return nullptr != click_handler;
}

void Window11Base::keyPress(char key, XPLMKeyFlags flags, uint8_t virtual_key, int losingFocus) {
    if(losingFocus) {
        layout_object->removeKeyboardFocus();
        return;
    }
    if((flags & xplm_ShiftFlag) == 0 && (flags & xplm_ControlFlag) == 0 && (flags & xplm_OptionAltFlag) == 0) {
        switch(virtual_key) {
            case XPLM_KEY_TAB:
                if(flags & xplm_DownFlag) {
                    // tab to next field
                    if(!layout_object->nextKeyboardFocus()) { // perhaps we reached the end of the dialog, and should start at the beginning
                        layout_object->nextKeyboardFocus();
                    }
                }
                return;
            default:
                break;
        }
    }
    if((flags & xplm_ShiftFlag) != 0 && (flags & xplm_ControlFlag) == 0 && (flags & xplm_OptionAltFlag) == 0) {
        switch(virtual_key) {
            case XPLM_KEY_TAB:
                if(flags & xplm_DownFlag) {
                    // tab to previous field
                    if(!layout_object->previousKeyboardFocus()) { // perhaps we reached the beginning of the dialog, and should start at the end
                        layout_object->previousKeyboardFocus();
                    }
                }
                return;
            default:
                break;
        }
    }

    if(layout_object->hasKeyboardFocus()) {
        if(layout_object->dispatchKeyPress(key, flags, virtual_key)) {
            return;
        }
    }

    //If no object handles it, then the window finally should get a try.
    this->keyPress(key, flags, virtual_key);
}

bool Window11Base::keyPress(char /* key */, XPLMKeyFlags flags, uint8_t virtual_key) {
    if((flags & xplm_ShiftFlag) == 0 && (flags & xplm_ControlFlag) != 0 && (flags & xplm_OptionAltFlag) == 0) {
        switch(virtual_key) {
            case XPLM_VK_W:
                if((flags & xplm_DownFlag) != 0) {
                    closeWindow();
                }
                return true;
            default:
                break;
        }
    }
    return false;
}

/// Widgets get to handle cursor if the mouse is over them. They are also
/// notified if the cursor was over them but is not any more (so we can disable hover, etc)
/// but they can't affect the cursor then.
XPLMCursorStatus Window11Base::handleCursor(Point point) {
    bool overlaps = layout_object->overlaps(point);
    return layout_object->handleCursor(point, overlaps);
}

bool Window11Base::handleMouseWheel(Point point, int wheel, int clicks) {
    return layout_object->mouseWheel(point, wheel, clicks);
}

void Window11Base::setTopLevelWidget(std::shared_ptr<LayoutObject> new_layout_obj) {
    layout_object = std::move(new_layout_obj);
    auto [min_width, min_height] = layout_object->getMinimumSize();

#if defined(XPLM301)
    if(isSimInVR()) {
        XPLMSetWindowGeometryVR(window, min_width, min_height);
    } else {
        setWindowBounds(Rect{0, min_height, min_width, 0});
        setWindowResizeLimits(min_width, min_height, layout_object->isStretchableX() ? 1000000 : min_width, layout_object->isStretchableY() ? 100000 : min_height);
        setWindowCentered();
    }
#else
    setWindowBounds(Rect{0, min_height, min_width, 0});
    setWindowResizeLimits(min_width, min_height, layout_object->isStretchableX() ? 1000000 : min_width, layout_object->isStretchableY() ? 100000 : min_height);
    setWindowCentered();
#endif

    Rect window_bounds = getWindowBounds();
    layout_object->setSize(window_bounds.left, window_bounds.bottom, min_width, min_height);
}

#if defined(XPLM301)
bool Window11Base::isSimInVR() {
    return currently_in_vr.get();
}

void Window11Base::setVR(bool is_vr) {
    if(is_vr) {
        XPLMSetWindowPositioningMode(window, xplm_WindowVR, -1);
        XPLMSetWindowGeometryVR(window, last_frame_window_bounds.size().width, last_frame_window_bounds.size().height);
    } else {
        XPLMSetWindowPositioningMode(window, xplm_WindowPositionFree, 0);
        setWindowBounds(Rect{0, last_frame_window_bounds.size().height, last_frame_window_bounds.size().width, 0});

        auto [min_width, min_height] = layout_object->getMinimumSize();
        setWindowResizeLimits(last_frame_window_bounds.size().width, last_frame_window_bounds.size().height, layout_object->isStretchableX() ? 1000000 : min_width, layout_object->isStretchableY() ? 100000 : min_height);
        setWindowCentered();
    }
}
#endif