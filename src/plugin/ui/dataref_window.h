#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include "window11.h"

class DataRefRecord;
class DataRefRow;
class Widget11Text;

class DatarefWindow : public Window11<DatarefWindow> {
protected:
    DataRefRecord * drr = nullptr;
    std::shared_ptr<Widget11Text> current_value;
    std::shared_ptr<Widget11Text> array_value;
    std::shared_ptr<Widget11Text> last_change;

    std::vector<std::shared_ptr<DataRefRow>> edit_rows;

    // This is the timestamp stored in the DRR the last time we updated
    // the displayed values. It's used to determine if we need to update
    // the displayed values, which can be expensive due to scalar-to-string
    // conversions.
    std::chrono::system_clock::time_point displayed_values_updated;
public:
    DatarefWindow(DataRefRecord * drr);

    void draw(Rect visible_bounds) override;

};
