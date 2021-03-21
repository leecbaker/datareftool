#pragma once

#include <functional>
#include <string>

#include "widget.h"

class Widget11TextField : public Widget11 {
protected:
    std::string contents;
    std::string placeholder;

    bool is_enabled = true;
    size_t cursor1 = 0, cursor2 = 0;
    float cursor_1x = 0.f, cursor_2x = 0.f;
    void recomputeCursor();
    std::function<void()> submit_action;
    std::function<void()> type_action;

    size_t clickdown_index = 0;
public:
    Widget11TextField() {
        recomputeCursor();
    }

    virtual void draw(Rect draw_bounds) override;

    void setContents(std::string new_contents) {
        contents = new_contents;
        cursor1 = std::min(cursor1, new_contents.size());
        cursor2 = std::min(cursor2, new_contents.size());
    }

    void setPlaceholder(std::string new_placeholder) {
        placeholder = new_placeholder;
    }
    const std::string & getContents() const { return contents; }

    void setEnabled(bool enable) { is_enabled = enable; }

    virtual Size getWidgetMinimumSize() const override { return Size{24, 20}; }

    virtual bool acceptsKeyboardFocus() const override { return true; }

    virtual bool keyPress(char key, XPLMKeyFlags flags, uint8_t virtual_key) override;
    virtual bool mouseClick(Point point, XPLMMouseStatus /* status */ ) override;

    void setTypeAction(std::function<void()> action) { type_action = std::move(action); }
    void setSubmitAction(std::function<void()> action) { submit_action = std::move(action); }

    void cut();
    void copy() const;
    void paste();
    void selectAll();
};
