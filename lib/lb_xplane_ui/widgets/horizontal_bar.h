#pragma once

#include "widget.h"

class Widget11HorizontalBar : public Widget11 {
public:
    Widget11HorizontalBar() = default;

    virtual Size getWidgetMinimumSize() const override { return Size{ 0, 1}; }
    virtual void draw(Rect draw_bounds) override;
};
