
#include "text_field.h"

#include <cmath>

#include "clipboard.h"

#include "../draw_basic.h"

#include <XPLMGraphics.h>

#include "gl_utils.h"

void Widget11TextField::draw(Rect) {
     XPLMSetGraphicsState(0, 0, 0, 0, 1, 0, 0);

    std::array<float, 3> text_color, background_color;
    if(is_enabled) {
        if(hasKeyboardFocus()) {
            background_color = {1.f, 1.f, 1.f};
            text_color = color3fFromHex(0x0d, 0x13, 0x1a);
        } else {
            background_color = color3fFromHex(0x47, 0x4f, 0x59);
            text_color = color3fFromHex(0x71, 0x78, 0x80);
        }
    } else {
        background_color = color3fFromHex(0x12, 0x15, 0x1b);
        text_color = color3fFromHex(0x71, 0x78, 0x80);
    }

    color3fGL(background_color);
    int right = bounds_.right;
    int top = bounds_.bottom + 20;
    drawRoundRect(Rect{bounds_.left, top, right, bounds_.bottom});

    constexpr const int x_margin = 5;
    constexpr const int y_margin = 3;
    constexpr const int y_margin_text = 7;
    constexpr const int y_selection_height = 14;

    if(is_enabled) {
        if(hasKeyboardFocus()) {
        } else {
        }
    } else {
    }

    //draw bar
    color3fGL(text_color);
    if(is_enabled && hasKeyboardFocus()) {
        if(cursor1 == cursor2) {
            const char * bar = "|";
            XPLMDrawString(text_color.data(), bounds_.left + cursor_1x + x_margin, bounds_.bottom + y_margin_text, const_cast<char *>(bar), nullptr, xplmFont_Proportional);
        } else {
            color3fGL(color3fFromHex(0x16, 0x78, 0xb5));
            glBegin(GL_TRIANGLE_STRIP);
            glVertex3f(float(bounds_.left + cursor_2x + x_margin), float(bounds_.bottom + y_margin + y_selection_height), 0);
            glVertex3f(float(bounds_.left + cursor_2x + x_margin), float(bounds_.bottom + y_margin), 0);
            glVertex3f(float(bounds_.left + cursor_1x + x_margin), float(bounds_.bottom + y_margin + y_selection_height), 0);
            glVertex3f(float(bounds_.left + cursor_1x + x_margin), float(bounds_.bottom + y_margin), 0);
            glEnd();
        }
    }

    if(false == contents.empty()) {
        XPLMDrawString(text_color.data(), bounds_.left + x_margin, bounds_.bottom + y_margin_text, const_cast<char *>(contents.c_str()), nullptr, xplmFont_Proportional);
    } else if (false == placeholder.empty() && false == hasKeyboardFocus()) {
        XPLMDrawString(text_color.data(), bounds_.left + x_margin, bounds_.bottom + y_margin_text, const_cast<char *>(placeholder.c_str()), nullptr, xplmFont_Proportional);
    }
}

bool Widget11TextField::mouseClick(Point point, XPLMMouseStatus status) {
    point.x -= 2 + bounds_.left;

    float min_dx = 100000000.f;
    size_t min_index = 0;

    for(size_t index = 0; index <= contents.size(); index++) {
        float dx = abs(point.x - XPLMMeasureString(xplmFont_Proportional, const_cast<char *>(contents.c_str()), index));
        if(dx < min_dx) {
            min_dx = dx;
            min_index = index;
        }
    }

    switch(status) {
        case xplm_MouseDown:
            clickdown_index = min_index;
            cursor2 = cursor1 = min_index;
            break;
        case xplm_MouseDrag:
            cursor1 = std::min<size_t>(clickdown_index, min_index);
            cursor2 = std::max<size_t>(clickdown_index, min_index);
            break;
        case xplm_MouseUp:
            cursor1 = std::min<size_t>(clickdown_index, min_index);
            cursor2 = std::max<size_t>(clickdown_index, min_index);
            break;

    }

    // TODO: handle selection: is shift down? Do we even have the API for this?
    recomputeCursor();
    
    return true;
}

void Widget11TextField::keyPress(char key, XPLMKeyFlags flags, uint8_t virtual_key) {
    if(virtual_key == XPLM_VK_RETURN || virtual_key == XPLM_VK_ENTER) {
        if(submit_action) {
            submit_action();
        }
        return;
    }

    if((virtual_key == XPLM_VK_X) && (xplm_ControlFlag & flags)) {
        if(xplm_DownFlag & flags) {
            cut();
        }
        return;
    }

    if((virtual_key == XPLM_VK_C) && (xplm_ControlFlag & flags)) {
        if(xplm_DownFlag & flags) {
            copy();
        }
        return;
    }
    if((virtual_key == XPLM_VK_V) && (xplm_ControlFlag & flags)) {
        if(xplm_DownFlag & flags) {
            paste();
        }
        return;
    }
    if((virtual_key == XPLM_VK_A) && (xplm_ControlFlag & flags)) {
        if(xplm_DownFlag & flags) {
            selectAll();
        }
        return;
    }

    if(virtual_key == XPLM_VK_LEFT) {
        if(xplm_DownFlag & flags) {
            if(cursor1 != 0) {
                cursor1--;
            }
            if(0 == (flags & xplm_ShiftFlag)) {
                cursor2 = cursor1;
            }
        }
        recomputeCursor();
        return;
    }
    if(virtual_key == XPLM_VK_RIGHT) {
        if(xplm_DownFlag & flags) {
            if(cursor2 != contents.size()) {
                cursor2++;
            }

            if(0 == (flags & xplm_ShiftFlag)) {
                cursor1 = cursor2;
            }
        }
        recomputeCursor();
        return;
    }

    if(virtual_key == XPLM_VK_UP || virtual_key == XPLM_VK_HOME || virtual_key == XPLM_VK_PRIOR) {
        if(xplm_DownFlag & flags) {
            cursor1 = 0;
            if(0 == (flags & xplm_ShiftFlag)) {
                cursor2 = cursor1;
            }
            recomputeCursor();
        }
        return;
    }
    if(virtual_key == XPLM_VK_DOWN || virtual_key == XPLM_VK_END || virtual_key == XPLM_VK_NEXT) {
        if(xplm_DownFlag & flags) {
            cursor2 = contents.size();
            if(0 == (flags & xplm_ShiftFlag)) {
                cursor1 = cursor2;
            }
            recomputeCursor();
        }
        return;
    }
    if(virtual_key == XPLM_KEY_DELETE) {
        if(xplm_DownFlag & flags) {
            if(cursor1 != cursor2) {
                contents.erase(contents.begin() + cursor1, contents.begin() + cursor2);
            } else {
                if(0 != cursor1) {
                    cursor1--;
                    contents.erase(cursor1, 1);
                }
            }
            cursor2 = cursor1;
            if(type_action) {
                type_action();
            }
            recomputeCursor();
        }
        return;
    }

    if(virtual_key == XPLM_VK_DELETE) {
        if(xplm_DownFlag & flags) {
            if(cursor1 != cursor2) {
                contents.erase(contents.begin() + cursor1, contents.begin() + cursor2);
            } else {
                if(contents.size() != cursor1) {
                    contents.erase(cursor1);
                }
            }
            cursor2 = cursor1;
            if(type_action) {
                type_action();
            }
            recomputeCursor();
        }
        return;
    }

    if(xplm_DownFlag & flags) {
        if(cursor1 != cursor2) {
            contents.erase(contents.begin() + cursor1, contents.begin() + cursor2);
        }
        contents.insert(cursor1, 1, key);
        cursor1++;

        cursor2 = cursor1;
        if(type_action) {
            type_action();
        }
        recomputeCursor();
    }
}

void Widget11TextField::recomputeCursor() {
    cursor_1x = XPLMMeasureString(xplmFont_Proportional, const_cast<char *>(contents.c_str()), cursor1);
    cursor_2x = XPLMMeasureString(xplmFont_Proportional, const_cast<char *>(contents.c_str()), cursor2);
}

void Widget11TextField::cut() {
    if(cursor1 != cursor2){
        setClipboard(contents.substr(cursor1, cursor2 - cursor1));
        contents.erase(contents.begin() + cursor1, contents.begin() + cursor2);
        cursor2 = cursor1;
        if(type_action) {
            type_action();
        }
        recomputeCursor();
    }
}

void Widget11TextField::copy() const {
    if(cursor1 != cursor2){
        setClipboard(contents.substr(cursor1, cursor2 - cursor1));
    }
}

void Widget11TextField::paste() {
    contents.erase(contents.begin() + cursor1, contents.begin() + cursor2);
    const std::string & clipboard_contents = getClipboard();
    contents.insert(contents.begin() + cursor1, clipboard_contents.cbegin(), clipboard_contents.cend());
    cursor1 = cursor2 = cursor1 + clipboard_contents.size();
    if(type_action) {
        type_action();
    }
    recomputeCursor();
}

void Widget11TextField::selectAll() {
    cursor1 = 0;
    cursor2 = contents.size();
    recomputeCursor();
}
