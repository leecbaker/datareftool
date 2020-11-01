// Vertical scroll bar for use with the scroll container
#pragma once

#include "widget.h"

class ScrollContainer;

class VerticalScrollBar : public Widget11 {
    ScrollContainer * container_;
    float fraction_ = 0.f;

public:
    VerticalScrollBar(ScrollContainer * container) : container_(container) {}

    virtual void draw(Rect draw_bounds) override;

    void setFraction(float fraction) {
        fraction_ = std::max(0.f, std::min(1.f, fraction));
    }

    virtual Size getWidgetMinimumSize() const override { return Size{10, 10}; }
    virtual bool acceptsKeyboardFocus() const override { return false; }

    virtual bool mouseClick(Point point, XPLMMouseStatus status) override;
};
