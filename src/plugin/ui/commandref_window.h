#pragma once

#include "window11.h"

#include <memory>
#include "geometry.h"

class CommandRefRecord;
class Widget11Text;

class CommandrefWindow : public Window11<CommandrefWindow> {
protected:
    CommandRefRecord * crr = nullptr;
    std::shared_ptr<Widget11Text> cr_activated;
    std::shared_ptr<Widget11Text> last_change;
public:
    CommandrefWindow(CommandRefRecord * crr);

    void draw(Rect visible_bounds) override;

};
