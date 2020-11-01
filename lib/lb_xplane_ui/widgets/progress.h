#pragma once

#include "widget.h"

class Widget11Progress : public Widget11 {
protected:
    float fraction_ = 0.f;
public:
    Widget11Progress() {}

    virtual void draw(Rect draw_bounds) override;

    void setFraction(float fraction) { fraction_ = std::max<float>(0.f, std::min<float>(1.f, fraction)); }

    virtual Size getWidgetMinimumSize() const override { return Size{24, 4}; } // 4 px of margin, 4 px of bar, 4 px of margin
};
