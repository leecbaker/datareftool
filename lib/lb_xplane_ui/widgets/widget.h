#pragma once

#include "../geometry.h"
#include "../layout_object.h"

/// Abstract interface for a widget.
class Widget11 : public LayoutObject {
protected:
    bool visible = true;
    bool has_keyboard_focus_ = false;
public:
    Widget11() {}
    Widget11(int x, int y) : LayoutObject(x,y) {}
    virtual ~Widget11() = default;

    void show() { visible = true; }
    void hide() { visible = false; }
    [[nodiscard]] bool isVisible() { return visible; }


    virtual bool mouseWheel(Point, int /* wheel */, int /* clicks */) override { return false; }
    virtual bool mouseClick(Point, XPLMMouseStatus /* status */ ) { return false; }
    virtual bool keyPress(char /* key */, XPLMKeyFlags /* flags */, uint8_t /* virtual_key */) override { return false; }

    virtual bool hasKeyboardFocus() const override { return has_keyboard_focus_; }

    virtual bool nextKeyboardFocus() override {
        if(hasKeyboardFocus()) {
            removeKeyboardFocus();
            return false;
        }

        if(acceptsKeyboardFocus()) {
            giveKeyboardFocus();
            return true;
        }

        return false;
    }

    virtual bool previousKeyboardFocus() override {
        if(hasKeyboardFocus()) {
            removeKeyboardFocus();
            return false;
        }

        if(acceptsKeyboardFocus()) {
            giveKeyboardFocus();
            return true;
        }

        return false;
    }

    virtual bool acceptsKeyboardFocus() const override { return false; }

    virtual void giveKeyboardFocus() { has_keyboard_focus_ = true; }
    virtual void removeKeyboardFocus() override { has_keyboard_focus_ = false; }
};
