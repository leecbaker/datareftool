#pragma once

#include "widget.h"

#include <array>
#include <string>
#include <vector>

class Widget11Text : public Widget11 {
    std::vector<std::string> lines;
    std::array<float, 3> color_ = {1., 1., 1.};
    int line_height = 12;
    int widest_line_width = 0;
    static const constexpr int y_margin = 2;
    static const constexpr int x_margin = 2;
public:
    Widget11Text() {}
    void setText(const std::string & text);
    virtual void draw(Rect draw_bounds) override;

    void setColor(std::array<float, 3> color) { color_ = color; }

    virtual Size getWidgetMinimumSize() const override {
        return Size{widest_line_width, line_height * std::max<int>(1, static_cast<int>(lines.size()))};
    }
};
