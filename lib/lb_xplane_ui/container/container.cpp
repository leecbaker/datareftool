#include "container.h"


bool WidgetsContainer::mouseWheel(Point point, int wheel, int clicks) {
    for(std::shared_ptr<LayoutObject> & object : widgets) {
        if(object->overlaps(point)) {
            if(object->mouseWheel(point, wheel, clicks)) {
                return true;
            }
        }
    }

    return false;
}

std::shared_ptr<Widget11> WidgetsContainer::mouseClick(Point point, XPLMMouseStatus status) {
    for(std::shared_ptr<LayoutObject> & object : widgets) {
        if(object->overlaps(point)) {
            std::shared_ptr<WidgetContainer> container = std::dynamic_pointer_cast<WidgetContainer>(object);
            std::shared_ptr<Widget11> widget = std::dynamic_pointer_cast<Widget11>(object);
            if(container) {
                std::shared_ptr<Widget11> handling_widget = container->mouseClick(point, status);

                if(handling_widget) {
                    return handling_widget;
                }
            } else if(widget) {
                if(widget->mouseClick(point, status)) {
                    return widget;
                }
            }
        }
    }

    return nullptr;
}

XPLMCursorStatus WidgetsContainer::handleCursor(Point point, bool mouse_inside) {
    XPLMCursorStatus cursor = xplm_CursorDefault;

    if(mouse_inside) {
        for(std::shared_ptr<LayoutObject> & widget : widgets) {
            bool overlaps = widget->overlaps(point);

            if(0 != current_hover.count(widget)) {
                widget->handleCursor(point, overlaps);

                if(false == overlaps) {
                    current_hover.erase(widget);
                }
            } else {
                if(overlaps) {
                    cursor = widget->handleCursor(point, overlaps);
                    current_hover.insert(widget);
                }
            }
        }
    } else {
        for(const std::shared_ptr<LayoutObject> & widget : current_hover) {
            widget->handleCursor(point, false);
        }
        current_hover.clear();
    }

    return cursor;
}
