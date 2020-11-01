#pragma once

#include "widget.h"

#include <filesystem.h>

class Widget11Image : public Widget11 {
    int texture_id = -1;
    int image_width = -1, image_height = -1;
public:
    Widget11Image() {}
    virtual void draw(Rect draw_bounds) override;

    bool load(std::ostream & log, const lb::filesystem::path & path);

    int getImageWidth() const { return image_width; }
    int getImageHeight() const { return image_height; }

    virtual Size getWidgetMinimumSize() const override { return Size{getImageWidth(), getImageHeight()}; }
};
