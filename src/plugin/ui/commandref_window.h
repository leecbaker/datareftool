#pragma once

#include "window11.h"

#include <memory>
#include "geometry.h"

class CommandRefRecord;
class SearchWindow;
class Widget11Text;

class CommandrefWindow : public Window11<CommandrefWindow> {
protected:
    CommandRefRecord * crr = nullptr;
    std::shared_ptr<Widget11Text> cr_activated;
    std::shared_ptr<Widget11Text> last_change;
    std::weak_ptr<SearchWindow> parent_search_window;
public:
    CommandrefWindow(CommandRefRecord * crr, std::weak_ptr<SearchWindow> parent_search_window = {});

    void draw(Rect visible_bounds) override;

    virtual bool keyPress(char /* key */, XPLMKeyFlags /* flags */, uint8_t /* virtual_key */) override;
};
