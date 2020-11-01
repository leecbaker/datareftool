#pragma once

class Widget11Spacer : public Widget11 {
    Size min_size;
public:
    Widget11Spacer(Size min_size = {0,0}) : min_size(min_size) {
    }

    virtual Size getWidgetMinimumSize() const override { return min_size; }

    virtual void draw(Rect) override {}
};
