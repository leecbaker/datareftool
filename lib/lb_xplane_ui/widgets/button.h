#pragma once

#include <cmath>
#include <functional>
#include <string>

#include "../geometry.h"

#include "widget.h"

// Basically a label plus a box.
class Widget11Button : public Widget11 {
public:
    enum class ButtonStyle {
        PRIMARY,
        SECONDARY, // grey, secondary actions
        PRIMARY_MAIN, // default action. Has a white halo and a larger size
        SECONDARY_MAIN, // secondary action. Has a grey color, and larger size
    };
protected:
    bool is_enabled = true;
    bool is_mouse_over = false;
    bool is_click_down = false;
    int button_margin_top = 1;
    int button_margin_bottom = 5;
    int line_height = 12;
    int button_margin_left = 5;
    int button_margin_right = 5;
    ButtonStyle button_style = ButtonStyle::PRIMARY;

    std::function<void()> click_action_;
    std::function<void(bool)> push_action_; //parameter: is button down
    std::string label_;
    float label_width = 0; // width of label when it is rendered. Avoid re-computing this all the time.
    void recomputeGeometry();
public:
    Widget11Button() {}
    virtual void draw(Rect draw_bounds) override;

    void setLabel(std::string label);
    void setEnabled(bool enabled) { is_enabled = enabled; }
    void setButtonStyle(ButtonStyle new_style) { this->button_style = new_style; recomputeGeometry(); }

    void setAction(std::function<void()> action) { click_action_ = std::move(action); }
    void setPushAction(std::function<void(bool)> push_action) { push_action_ = std::move(push_action); }

    virtual void setSize(int x, int y, int width, int height) override {
        Widget11::setSize(x, y, width, height);
        recomputeGeometry();
    }

    virtual Size getWidgetMinimumSize() const override {
        return Size{button_margin_left + button_margin_right + static_cast<int>(std::ceil(label_width)), button_margin_bottom + button_margin_top + line_height};
    }

    virtual bool mouseClick(Point point, XPLMMouseStatus status) override;

    virtual XPLMCursorStatus handleCursor(Point point, bool mouse_inside) override;
};
