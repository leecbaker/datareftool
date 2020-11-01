#include "button.h"

#include <XPLMGraphics.h>

#include "gl_utils.h"

#include "../draw_basic.h"

void Widget11Button::setLabel(std::string label) {
    label_ = std::move(label);
    label_width = XPLMMeasureString(xplmFont_Proportional, label_.c_str(), label_.size());

    recomputeGeometry();
}

void Widget11Button::recomputeGeometry() {
    if(-1 != bounds_.size().width) {
        button_margin_left = (bounds_.size().width - label_width) / 2;
        button_margin_right = bounds_.size().width - label_width - button_margin_left;
    } else {
        button_margin_left = button_margin_right = 5;
    }

    switch(button_style) {
        case ButtonStyle::SECONDARY_MAIN:
        case ButtonStyle::PRIMARY_MAIN:
            button_margin_bottom = 8;
            button_margin_top = 4;
            break;
        case ButtonStyle::SECONDARY:
        case ButtonStyle::PRIMARY:
            button_margin_bottom = 5;
            button_margin_top = 1;
            break;
    }
}

void Widget11Button::draw(Rect) {
    // Shaded rect
    int rect_l = bounds_.left;
    int rect_t = bounds_.top;
    int rect_r = bounds_.right;
    int rect_b = bounds_.bottom;

    std::array<float, 3> text_color, text_color_inactive, click_color, hover_color, normal_color, inactive_color;

    switch(button_style) {
        case ButtonStyle::PRIMARY:
        case ButtonStyle::PRIMARY_MAIN:
            text_color = {1.f,1.f,1.f};
            text_color_inactive = color3fFromHex(0xaa, 0xaa, 0xaa);
            click_color = color3fFromHex(0x77, 0xc6, 0xf7);
            hover_color = color3fFromHex(0x1d, 0xa0, 0xf2);
            normal_color = color3fFromHex(0x16, 0x78, 0xb5);
            inactive_color = color3fFromHex(0x71, 0x7f, 0x80);
            break;
        case ButtonStyle::SECONDARY:
        case ButtonStyle::SECONDARY_MAIN:
            text_color = {1.f,1.f,1.f};
            text_color_inactive = color3fFromHex(0xaa, 0xaa, 0xaa);
            click_color = color3fFromHex(0x89, 0x92, 0x9b);
            hover_color = color3fFromHex(0x71, 0x78, 0x80);
            normal_color = color3fFromHex(0x47, 0x4f, 0x59);
            inactive_color = color3fFromHex(0x2c, 0x32, 0x38);
            break;
    }

    XPLMSetGraphicsState(0, 0, 0, 0, 1, 0, 0);

    if(is_enabled && ButtonStyle::PRIMARY_MAIN == button_style) {
        color3fGL(hover_color);
        drawRoundRect(Rect{rect_l, rect_t, rect_r, rect_b});
        rect_l += 2;
        rect_t -= 2;
        rect_r -= 2;
        rect_b += 2;
    }

    if(is_enabled) {
        if(is_click_down) {
            color3fGL(click_color); //in click
        } else {
            if(is_mouse_over) {
                color3fGL(hover_color); //hover
            } else {
                color3fGL(normal_color); //normal
            }
        }
    } else {
        color3fGL(inactive_color); //inactive
    }

    drawRoundRect(Rect{rect_l, rect_t, rect_r, rect_b});

    //draw text
    XPLMSetGraphicsState(0, 1, 0, 0, 1, 0, 0);
    XPLMDrawString(is_enabled ? text_color.data() : text_color_inactive.data(), bounds_.left + button_margin_left, bounds_.bottom + button_margin_bottom, const_cast<char *>(label_.c_str()), nullptr, xplmFont_Proportional);
}

bool Widget11Button::mouseClick(Point /* point */, XPLMMouseStatus status) {
    switch(status) {
        case xplm_MouseDown:
            is_click_down = true;
            if(push_action_ && is_enabled) {
                push_action_(true);
            }
            break;
        case xplm_MouseUp:
            if(click_action_ && is_click_down && is_enabled) {
                click_action_();
            }
            if(push_action_ && is_click_down && is_enabled) {
                push_action_(false);
            }
            is_click_down = false;
            break;
        case xplm_MouseDrag:
            break;
    }

    return true;
}

XPLMCursorStatus Widget11Button::handleCursor(Point /* point */, bool mouse_inside) {
    is_mouse_over = mouse_inside;
    return xplm_CursorDefault;
}
