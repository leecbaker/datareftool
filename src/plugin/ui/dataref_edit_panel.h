#pragma once

#include <chrono>

#include "containers.h"
#include "widgets/text.h"
#include "widgets/text_field.h"

class DataRefRecord;

class DatarefEditField: public Widget11TextField {
    DataRefRecord * drr;
    int element_index;

    std::chrono::system_clock::time_point displayed_values_updated;
public:
    DatarefEditField(DataRefRecord * drr, int element_index);
    void update();
    void draw(Rect visible_bounds);
};

class DatarefEditPanel: public SingleAxisLayoutContainer {
    DataRefRecord * drr;
    int element_index;

    std::shared_ptr<Widget11TextField> edit_field;
public:
    DatarefEditPanel(DataRefRecord * drr, int element_index);
    void update();

    std::shared_ptr<Widget11TextField> getEditField() {
        return edit_field;
    }
};
