#pragma once

#include "window11.h"

class DataRefRecord;
class Widget11Button;
class Widget11TextField;
class Widget11Text;

class DatarefWindow : public Window11<DatarefWindow> {
protected:
    DataRefRecord * drr = nullptr;
    std::shared_ptr<Widget11Button> set_button;
    std::shared_ptr<Widget11TextField> edit_field;
    std::shared_ptr<Widget11Text> current_value;
    std::shared_ptr<Widget11Text> last_change;

    bool checkValueOK() const;
    void setValue();
public:
    DatarefWindow(DataRefRecord * drr);

    void draw(Rect visible_bounds) override;

};
