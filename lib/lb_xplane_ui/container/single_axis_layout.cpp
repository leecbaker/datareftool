#include "single_axis_layout.h"

#include <numeric>


template<int Size::* p_member>
struct LayoutGetFunctor {
    int operator()(int init, const Size & from) const
    { return from.*p_member + init; }
};

bool SingleAxisLayoutContainer::isStretchableX() const {
    switch(axis) {
        case LayoutAxis::VERTICAL:
            return std::any_of(widgets.cbegin(), widgets.cend(), [](const std::shared_ptr<LayoutObject> & obj) -> bool { return obj->isStretchableX(); });

        case LayoutAxis::HORIZONTAL:
            return std::any_of(widget_attributes.cbegin(), widget_attributes.cend(), [](const std::tuple<bool, bool> & attr) -> bool { return std::get<0>(attr); });
    }
}

bool SingleAxisLayoutContainer::isStretchableY() const {
    switch(axis) {
        case LayoutAxis::VERTICAL:
            return std::any_of(widget_attributes.cbegin(), widget_attributes.cend(), [](const std::tuple<bool, bool> & attr) -> bool { return std::get<0>(attr); });

        case LayoutAxis::HORIZONTAL:
            return std::any_of(widgets.cbegin(), widgets.cend(), [](const std::shared_ptr<LayoutObject> & obj) -> bool { return obj->isStretchableY(); });
    }
}

//divide up the space proprotionally as necessary.
void SingleAxisLayoutContainer::setSize(int x, int y, int width, int height) {
    LayoutObject::setSize(x, y, width, height);
    recomputeLayout();
}
void SingleAxisLayoutContainer::recomputeLayout() {
    auto is_expandable = [](const std::tuple<bool, bool> & t) -> bool { return std::get<0>(t); };
    int expandable_count = static_cast<int>(std::count_if(widget_attributes.cbegin(), widget_attributes.cend(), is_expandable));

    std::vector<Size> widget_minimum_sizes;
    auto get_minimum_size = [](const std::shared_ptr<LayoutObject> & widget) -> Size {
        return widget->getMinimumSize();
    };
    std::transform(widgets.cbegin(), widgets.cend(), std::back_inserter(widget_minimum_sizes), get_minimum_size);

    int height = getHeight();
    int width = getWidth();

    switch(axis) {
        case LayoutAxis::VERTICAL: {
            int total_minimum_height = std::accumulate(widget_minimum_sizes.cbegin(), widget_minimum_sizes.cend(), 0, LayoutGetFunctor<&Size::height>());
            int extra_pixels = height - total_minimum_height - interelement_padding * static_cast<int>(widgets.size() - 1);
            int invert_y = getBounds().top;
            for(size_t index = 0; index < widgets.size(); index++) {
                auto & widget = widgets[index];
                auto & [expand, fill] = widget_attributes[index];
                Size widget_min_size = widget_minimum_sizes[index];

                int widget_height = widget_min_size.height;
                int widget_width = fill ? width : widget_min_size.width;

                if(expand) {
                    widget_height += extra_pixels / expandable_count;
                }

                invert_y -= widget_height;

                widget->setSize(getBounds().left, invert_y, widget_width, widget_height);
                invert_y -= interelement_padding;
            }
            break;
        }

        case LayoutAxis::HORIZONTAL: {
            int total_minimum_width = std::accumulate(widget_minimum_sizes.cbegin(), widget_minimum_sizes.cend(), 0, LayoutGetFunctor<&Size::width>());
            int extra_pixels = width - total_minimum_width - interelement_padding * static_cast<int>(widgets.size() - 1);
            int x = getBounds().left;
            for(size_t index = 0; index < widgets.size(); index++) {
                auto & widget = widgets[index];
                auto & [expand, fill] = widget_attributes[index];
                Size widget_min_size = widget_minimum_sizes[index];

                int widget_width = widget_min_size.width;
                int widget_height = fill ? height : widget_min_size.height;

                if(expand) {
                    widget_width += extra_pixels / expandable_count;
                }

                widget->setSize(x, getBounds().bottom, widget_width, widget_height);
                x += widget_width + 10;
            }
            break;
        }
    }
}
