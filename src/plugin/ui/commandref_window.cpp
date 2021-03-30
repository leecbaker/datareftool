#include "commandref_window.h"

#include <cstddef>
#include <sstream>

#include "container/single_axis_layout.h"
#include "widgets/button.h"
#include "widgets/horizontal_bar.h"
#include "widgets/text.h"

#include "search/commandref.h"

#include "search_window.h"

CommandrefWindow::CommandrefWindow(CommandRefRecord * crr, std::weak_ptr<SearchWindow> parent_search_window) : crr(crr), parent_search_window(parent_search_window) {
    std::shared_ptr<Widget11Text> cr_name = std::make_shared<Widget11Text>();
    cr_name->setText(crr->getName());
    cr_activated = std::make_shared<Widget11Text>();
    cr_activated->setText("Is activated: ?");
    last_change = std::make_shared<Widget11Text>();
    last_change->setText("Last change: ?");

    std::shared_ptr<Widget11Button> once_button = std::make_shared<Widget11Button>();
    once_button->setLabel("Command once");
    once_button->setAction([crr]() {
        crr->commandOnce();
    });
    

    std::shared_ptr<Widget11Button> press_button = std::make_shared<Widget11Button>();
    press_button->setLabel("Command press");
    press_button->setPushAction([crr](bool is_down) {
        if(is_down) {
            crr->commandBegin();
        } else {
            crr->commandEnd();
        }
    });

    std::shared_ptr<SingleAxisLayoutContainer> window_container = std::make_shared<SingleAxisLayoutContainer>(SingleAxisLayoutContainer::LayoutAxis::VERTICAL);

    window_container->add(cr_name, false, true);
    window_container->add(cr_activated, false, true);
    window_container->add(last_change, false, true);
    window_container->add(std::make_shared<Widget11HorizontalBar>(), false, true);
    window_container->add(press_button, false, true);
    window_container->add(once_button, false, true);

    std::string title = crr->getName();
    size_t last_slash = title.rfind("/");
    if(std::string::npos != last_slash) {
        title.erase(title.cbegin(), title.cbegin() + last_slash + 1);
    }

    setTitle("Command: " + title);

    setTopLevelWidget(window_container);
    setWindowCentered();
}

void CommandrefWindow::draw(Rect visible_bounds) {
    if(crr->isActivated()) {
        cr_activated->setText("Is activated: Yes");
    } else {
        cr_activated->setText("Is activated: No");
    }

    if(0 == crr->getLastUpdateTime().time_since_epoch().count()) {
        last_change->setText("Never activated");
    } else {
        std::chrono::duration time_since_change = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - crr->getLastUpdateTime());
        int seconds_since_change = time_since_change.count();
        last_change->setText("Last activated: " + std::to_string(seconds_since_change) + "s ago");
    }

    Window11<CommandrefWindow>::draw(visible_bounds);
}

bool CommandrefWindow::keyPress(char key, XPLMKeyFlags flags, uint8_t virtual_key) {
    // no modifiers
    if((flags & xplm_ShiftFlag) == 0 && (flags & xplm_ControlFlag) == 0 && (flags & xplm_OptionAltFlag) == 0) {
        switch(virtual_key) {
            case XPLM_VK_ESCAPE:
                closeWindow();
                {
                    std::shared_ptr<SearchWindow> search_window = parent_search_window.lock();
                    if(search_window) {
                        search_window->selectSearchField();
                    }
                }
                return true;
            default:
                break;
        }
    }

    return Window11::keyPress(key, flags, virtual_key);
}
