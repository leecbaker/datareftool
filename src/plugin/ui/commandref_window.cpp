#include "commandref_window.h"

#include "widgets.h"
#include "containers.h"

#include "search/commandref.h"

#include <sstream>

CommandrefWindow::CommandrefWindow(CommandRefRecord * crr) : crr(crr) {
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
