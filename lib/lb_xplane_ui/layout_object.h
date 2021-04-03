#pragma once

#include "geometry.h"

#include <algorithm>

#include "XPLMDisplay.h"

///
class LayoutObject {
protected:
    Rect bounds_;
    Size min_size{0, 0}; //This is defined when laying out object. Widgets have a minimum size function
    LayoutObject * parent = nullptr;

    virtual Size getWidgetMinimumSize() const { return {0, 0}; }
public:
    LayoutObject() {}
    LayoutObject(int x, int y) : bounds_{x, y, x, y} {}

    void setParent(LayoutObject * new_parent) { parent = new_parent; }

    void setMinimumSize(Size new_min_size) {
        min_size = new_min_size;
    }

    virtual void setSize(int x, int y, int width, int height) {
        bounds_.left = x;
        bounds_.bottom = y;
        bounds_.right = x + width;
        bounds_.top = y + height;
    }

    void setBounds(Rect new_bounds) {
        setSize(new_bounds.left, new_bounds.bottom, new_bounds.size().width, new_bounds.size().height);
    }

    Rect getBounds() const {
        return bounds_;
    }

    virtual int getWidth() const { return bounds_.size().width; };
    virtual int getHeight() const { return bounds_.size().height; };

    [[nodiscard]] bool overlaps(Point test) {
        return bounds_.contains(test);
    }

    virtual Size getMinimumSize() const {
        Size widget_size = getWidgetMinimumSize();
        return Size{ std::max(widget_size.width, min_size.width), std::max(widget_size.height, min_size.height)};
    }

    virtual bool isStretchableX() const { return false; }
    virtual bool isStretchableY() const { return false; }

    // Perhaps this class is more broad than just layout objects?
    virtual void draw(Rect draw_bounds) = 0;

    virtual bool mouseWheel(Point point, int wheel, int clicks) = 0;

    /// @returns Tuple of XPLMCursorStatus, and whether or not to subscribe to
    /// future cursor movements outside this widget.
    virtual XPLMCursorStatus handleCursor(Point, bool /* mouse_inside */) { return xplm_CursorDefault; }

    virtual bool acceptsKeyboardFocus() const = 0;
    virtual bool hasKeyboardFocus() const = 0;

    virtual void removeKeyboardFocus() = 0;

    virtual bool nextKeyboardFocus() = 0;
    virtual bool previousKeyboardFocus() = 0;

    bool dispatchKeyPress(char key, XPLMKeyFlags flags, uint8_t virtual_key);

    virtual bool keyPress(char /* key */, XPLMKeyFlags /* flags */, uint8_t /* virtual_key */) = 0;
};
