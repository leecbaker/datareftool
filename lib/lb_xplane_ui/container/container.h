#pragma once

#include <unordered_set>
#include <vector>

#include "XPLMDataAccess.h"
#include "XPLMDefs.h"

#include "../layout_object.h"
#include "../widgets/widget.h"

class WidgetContainer : public LayoutObject {
public:
    virtual std::shared_ptr<Widget11> mouseClick(Point point, XPLMMouseStatus status) = 0;
};

class WidgetsContainer : public WidgetContainer {
public:
    using widget_container_type = std::vector<std::shared_ptr<LayoutObject>>;
    using widget_iterator_type = widget_container_type::iterator;
    using widget_const_iterator_type = widget_container_type::const_iterator;
protected:
    widget_container_type widgets;
    std::unordered_set<std::shared_ptr<LayoutObject>> current_hover;
public:
    size_t getWidgetCount() const { return widgets.size(); }

    virtual void draw(Rect draw_bounds) override {
        for(const std::shared_ptr<LayoutObject> & widget : widgets) {
            std::optional<Rect> overlap_bounds = widget->getBounds().intersection(draw_bounds);
            if(overlap_bounds) {
                widget->draw(*overlap_bounds);
            }
        }
    }

    virtual bool hasKeyboardFocus() const override {
        return std::any_of(widgets.cbegin(), widgets.cend(), [](std::shared_ptr<LayoutObject> widget) -> bool { return widget->hasKeyboardFocus(); });
    }

    virtual bool acceptsKeyboardFocus() const override {
        return std::any_of(widgets.cbegin(), widgets.cend(), [](std::shared_ptr<LayoutObject> widget) -> bool { return widget->acceptsKeyboardFocus(); });
     }

    virtual bool advanceKeyboardFocus() override {
        auto candidate_widget_it = widgets.begin();

        if(hasKeyboardFocus()) {
            candidate_widget_it = std::find_if(widgets.begin(), widgets.end(), [](const std::shared_ptr<LayoutObject> & widget) -> bool { return widget->hasKeyboardFocus(); });
        }

        for(; candidate_widget_it != widgets.end(); ++candidate_widget_it) {
            if((*candidate_widget_it)->advanceKeyboardFocus()) {
                return true;
            }
        }

        return false;
    }

    virtual void removeKeyboardFocus() override {
        for(const std::shared_ptr<LayoutObject> & widget : widgets) {
            if(widget->hasKeyboardFocus()) {
                widget->removeKeyboardFocus();
            }
        }
    }

    virtual void keyPress(char key, XPLMKeyFlags flags, uint8_t virtual_key) override {
        for(const std::shared_ptr<LayoutObject> & widget : widgets) {
            if(widget->hasKeyboardFocus()) {
                widget->keyPress(key, flags, virtual_key);
            }
        }
    }

    /// @return the widget that handled the click, or nullptr if it was not handled. This
    /// helps with keyboard focus downstream.
    virtual std::shared_ptr<Widget11> mouseClick(Point point, XPLMMouseStatus status) override;
    virtual bool mouseWheel(Point point, int wheel, int clicks) override;

    virtual XPLMCursorStatus handleCursor(Point, bool /* mouse_inside */) override;

    widget_iterator_type begin() {
        return widgets.begin();
    }
    widget_const_iterator_type begin() const {
        return widgets.cbegin();
    }
    widget_iterator_type end() {
        return widgets.end();
    }
    widget_const_iterator_type end() const {
        return widgets.cend();
    }

    size_t widget_count() const
    {
        return widgets.size();
    }
};
