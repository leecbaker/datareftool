#pragma once

#include <memory>
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
    using widget_reverse_iterator_type = widget_container_type::reverse_iterator;
    using widget_const_reverse_iterator_type = widget_container_type::const_reverse_iterator;
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

    virtual bool previousKeyboardFocus() override {
        auto candidate_widget_it = widgets.rbegin();

        if(hasKeyboardFocus()) {
            candidate_widget_it = std::find_if(widgets.rbegin(), widgets.rend(), [](const std::shared_ptr<LayoutObject> & widget) -> bool { return widget->hasKeyboardFocus(); });
        }

        for(; candidate_widget_it != widgets.rend(); ++candidate_widget_it) {
            if((*candidate_widget_it)->previousKeyboardFocus()) {
                return true;
            }
        }

        return false;
    }

    virtual bool nextKeyboardFocus() override {
        auto candidate_widget_it = widgets.begin();

        if(hasKeyboardFocus()) {
            candidate_widget_it = std::find_if(widgets.begin(), widgets.end(), [](const std::shared_ptr<LayoutObject> & widget) -> bool { return widget->hasKeyboardFocus(); });
        }

        for(; candidate_widget_it != widgets.end(); ++candidate_widget_it) {
            if((*candidate_widget_it)->nextKeyboardFocus()) {
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

    virtual bool keyPress(char key, XPLMKeyFlags flags, uint8_t virtual_key) override {
        for(const std::shared_ptr<LayoutObject> & widget : widgets) {
            if(widget->hasKeyboardFocus()) {
                if(widget->keyPress(key, flags, virtual_key)) {
                    return true;
                }
            }
        }

        return false;
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

    widget_reverse_iterator_type rbegin() {
        return widgets.rbegin();
    }
    widget_const_reverse_iterator_type rbegin() const {
        return widgets.crbegin();
    }
    widget_reverse_iterator_type rend() {
        return widgets.rend();
    }
    widget_const_reverse_iterator_type rend() const {
        return widgets.crend();
    }

    widget_container_type::reference front() {
        return widgets.front();
    }

    widget_container_type::const_reference front() const {
        return widgets.front();
    }

    widget_container_type::reference back() {
        return widgets.back();
    }

    widget_container_type::const_reference back() const {
        return widgets.back();
    }

    size_t widget_count() const
    {
        return widgets.size();
    }
};
