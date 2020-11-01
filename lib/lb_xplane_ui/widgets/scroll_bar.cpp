#include "scroll_bar.h"

#include "../container/scroll.h"

#include "../draw_basic.h"
#include "gl_utils.h"

#include <XPLMGraphics.h>


bool VerticalScrollBar::mouseClick(Point point, XPLMMouseStatus /* status */) {
    float half_scroll_handle_height = 0.5 * 20;

    float draggable_top = getBounds().top - half_scroll_handle_height;
    float draggable_bottom = getBounds().bottom + half_scroll_handle_height;

    float scroll_fraction = (draggable_top - point.y) / (draggable_top - draggable_bottom);

    float clamped_scroll_fraction = std::min(std::max(scroll_fraction, 0.f), 1.f);

    container_->setScrollFraction(clamped_scroll_fraction);

    return true;
}

void VerticalScrollBar::draw(Rect /* draw_bounds */) {
    const float min_handle_height = 20; //px
    float bar_travel = getHeight() - min_handle_height;

    const Rect bounds = getBounds();

    int rect_bottom = bounds.top - fraction_ * bar_travel - min_handle_height;
    int rect_top = bounds.top - fraction_ * bar_travel;

    XPLMSetGraphicsState(0,0,0,0,1,0,0);

    color3fGL(color3fFromHex(0x47, 0x4f, 0x59));
    drawRoundRect(Rectf{static_cast<float>(bounds.left), static_cast<float>(rect_top), static_cast<float>(bounds.right), static_cast<float>(rect_bottom)});
}
