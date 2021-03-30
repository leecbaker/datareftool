#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include "window11.h"

class DataRefRecord;
class DatarefEditPanel;
class SearchWindow;
class Widget11Text;

class DatarefWindow : public Window11<DatarefWindow> {
protected:
    DataRefRecord * drr = nullptr;
    std::shared_ptr<Widget11Text> current_value;
    std::shared_ptr<Widget11Text> array_value;
    std::shared_ptr<Widget11Text> last_change;

    std::vector<std::shared_ptr<DatarefEditPanel>> edit_rows;

    // This is the timestamp stored in the DRR the last time we updated
    // the displayed values. It's used to determine if we need to update
    // the displayed values, which can be expensive due to scalar-to-string
    // conversions.
    std::chrono::system_clock::time_point displayed_values_updated;

    // When this window is closed, we'll try to give focus back to the window that opened us
    std::weak_ptr<SearchWindow> parent_search_window;
public:
    DatarefWindow(DataRefRecord * drr, std::weak_ptr<SearchWindow> parent_search_window = {});

    void draw(Rect visible_bounds) override;

    virtual bool keyPress(char /* key */, XPLMKeyFlags /* flags */, uint8_t /* virtual_key */) override;
};
