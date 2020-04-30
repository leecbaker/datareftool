/// This represents coordinates for the 4 sides of a window.

#include <array>

#include "XPWidgets.h"

class WindowBounds {
    std::array<int, 4> bounds;
public:
    //WindowBounds() = default;
    WindowBounds(int left, int top, int right, int bottom) : bounds({left, top, right, bottom}) {}
    WindowBounds(std::array<int, 4> bounds) : bounds(bounds) {}
    static WindowBounds fromWidget(XPWidgetID widget_id) {
        int left, top, right, bottom;
        XPGetWidgetGeometry(widget_id, &left, &top, &right, &bottom);

        return WindowBounds(left, top, right, bottom);
    }

    static WindowBounds screenBoundsGlobal() {
        int left, top, right, bottom;
        XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);
        return WindowBounds(left, top, right, bottom);
    }

    int getX() const { return bounds[0]; }
    int getY() const { return bounds[3]; }

    int getWidth() const { return bounds[2] - bounds[0]; }
    int getHeight() const { return bounds[1] - bounds[3]; }

    int & left() { return bounds[0]; }
    int left() const { return bounds[0]; }
    int & top() { return bounds[1]; }
    int top() const { return bounds[1]; }
    int & right() { return bounds[2]; }
    int right() const { return bounds[2]; }
    int & bottom() { return bounds[3]; }
    int bottom() const { return bounds[3]; }

    void setWidgetGeometry(XPWidgetID widget_id) {
        XPSetWidgetGeometry(widget_id, bounds[0], bounds[1], bounds[2], bounds[3]);
    }

    void constrainMaxSize(int max_width, int max_height) {
        if(getWidth() > max_width) {
            right() = left() + max_width;
        }
        if(getHeight() > max_height) {
            top() = bottom() + max_height;
        }
    }

    void constrainMinSize(int min_width, int min_height) {
        if(getWidth() < min_width) {
            right() = left() + min_width;
        }
        if(getHeight() < min_height) {
            top() = bottom() + min_height;
        }
    }

    void constrainToBounds(WindowBounds constraint) {
        // positive 'distance' values signify correction needed.
        int distance_left = std::max(0, constraint.left() - left());
        int distance_top = -std::max(0, top() - constraint.top());
        int distance_right = -std::max(0, right() - constraint.right());
        int distance_bottom = std::max(0, constraint.bottom() - bottom());

        int distance_x = distance_right + distance_left;
        int distance_y = distance_top + distance_bottom;

        bounds[0] += distance_x;
        bounds[1] += distance_y;
        bounds[2] += distance_x;
        bounds[3] += distance_y;
    }
};
