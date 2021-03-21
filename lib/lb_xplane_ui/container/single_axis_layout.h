#pragma once

#include "container.h"

#include <memory>
#include <vector>


/// This is very similar to BoxLayout in Spring- it aligns widgets along a single axis.
/// Widgets can be expandable or not. This means when a layout contains multiple widgets, it is
/// resizable down to a minimum size, and when resized bigger than that, will expand the child widgets.
class SingleAxisLayoutContainer : public WidgetsContainer {
public:
    enum class LayoutAxis {
        VERTICAL,
        HORIZONTAL,
    };
protected:
    LayoutAxis axis;
    std::vector<std::tuple<bool, bool>> widget_attributes; // expand along the layout axis, fill to full width orthogonal to it.
    int interelement_padding = 10;

    void recomputeLayout();
public:
    SingleAxisLayoutContainer(LayoutAxis axis) : axis(axis) {}
    virtual ~SingleAxisLayoutContainer() = default;

    void addNoLayout(std::shared_ptr<LayoutObject> widget, bool expand, bool fill) {
        widget->setParent(this);
        widgets.emplace_back(std::move(widget));
        widget_attributes.emplace_back(expand, fill);
    }

    void add(std::shared_ptr<LayoutObject> widget, bool expand, bool fill) {
        addNoLayout(std::move(widget), expand, fill);
        recomputeLayout();
    }

    void pop_back() {
        widgets.pop_back();
        widget_attributes.pop_back();
        recomputeLayout();
    }

    void setPaddingBetweenElements(int padding) {
        interelement_padding = padding;
    }

    size_t widgetCount() const { return widgets.size(); }
    virtual void clear() { widgets.clear(); widget_attributes.clear(); }

    virtual Size getMinimumSize() const override {
        int min_width = 0, min_height = 0;

        switch(axis) {
            case LayoutAxis::VERTICAL:
                for(auto & widget: widgets) {
                    Size widget_mins = widget->getMinimumSize();
                    min_width = std::max(min_width, widget_mins.width);
                    min_height += widget_mins.height;
                }
                min_height += interelement_padding * static_cast<int>(widgets.size() - 1);
                break;
            case LayoutAxis::HORIZONTAL:
                for(auto & widget: widgets) {
                    Size widget_mins = widget->getMinimumSize();
                    min_height = std::max(min_height, widget_mins.height);
                    min_width += widget_mins.width;
                }
                min_width += interelement_padding * static_cast<int>(widgets.size() - 1);
                break;
        }
        return { min_width, min_height};
    }

    //divide up the space proprotionally as necessary.
    virtual void setSize(int x, int y, int width, int height) override;

    virtual bool isStretchableX() const override;
    virtual bool isStretchableY() const override;
};
