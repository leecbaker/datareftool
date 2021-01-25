#include "about_window.h"

#include "container/single_axis_layout.h"
#include "widgets/text.h"

#include <memory>
#include <sstream>

AboutWindow::AboutWindow() {
    std::stringstream text;

    text << "DataRefTool\n";
    text << "by Lee Baker\n";
    text << "\n";
    text << "https://datareftool.com\n";
    text << "\n";
    text << "Compiled at " __DATE__ " " __TIME__ "\n";
    text << "\n";
    text << "(c) 2014-2021 Lee C. Baker";

    std::shared_ptr<Widget11Text> text_widget = std::make_shared<Widget11Text>();

    text_widget->setText(text.str());

    std::shared_ptr<SingleAxisLayoutContainer> window_vertical_container = std::make_shared<SingleAxisLayoutContainer>(SingleAxisLayoutContainer::LayoutAxis::VERTICAL);
    window_vertical_container->add(text_widget, true, true);

    setTitle("About DataRefTool");
    setTopLevelWidget(window_vertical_container);
    setWindowSize({200, 100});
    setWindowCentered();
}
