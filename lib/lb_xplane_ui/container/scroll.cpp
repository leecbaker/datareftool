#include "scroll.h"

#include <XPLMGraphics.h>

#include "../widgets/scroll_bar.h"

#include "../draw_basic.h"
#include "gl_utils.h"

void ScrollContainer::setSize(int x, int y, int width, int height) {
    LayoutObject::setSize(x, y, width, height);

    recomputeScroll();
}
void ScrollContainer::setScrollFraction(float fraction) {
    int min_contents_height = object_->getMinimumSize().height;
    int max_scroll_distance = std::max(0, min_contents_height - getHeight());
    scroll_distance = fraction * max_scroll_distance;

    onScrollChange();
}

void ScrollContainer::onScrollChange() {
    int contained_height = object_->getMinimumSize().height;

    int scroll_travel_range = std::max(0, contained_height - getHeight());

    if(contained_height > getHeight()) {
        scroll_bar_->setFraction(static_cast<float>(scroll_distance) / scroll_travel_range);
    } else {
        scroll_bar_->setFraction(0.f);
    }
}

void ScrollContainer::recomputeScroll() {
    int height = getHeight();
    int width = getWidth();
    int min_contents_height = object_->getMinimumSize().height;
    int max_scroll_distance = std::max(0, min_contents_height - height);
    shows_scroll_bar_ = max_scroll_distance > 0;

    scroll_distance = std::min(scroll_distance, max_scroll_distance);

    Size min_size = object_->getMinimumSize();
    int object_height = std::max<int>(height, min_size.height);

    Rect bounds = getBounds();

    if(shows_scroll_bar_) {
        object_->setSize(bounds.left, bounds.top - object_height, width - 10, object_height);
        scroll_bar_->setSize(bounds.right - 10, bounds.bottom, 10, getHeight());
    } else {
        object_->setSize(bounds.left, bounds.top - object_height, width, object_height);
    }

    onScrollChange();
}

static std::array<GLfloat, 4> multMatrixVec4f(const std::array<GLfloat, 16> & m, const std::array<GLfloat, 4> & v)
{
    std::array<GLfloat, 4> dst;
    dst[0] = v[0] * m[0] + v[1] * m[4] + v[2] * m[8] + v[3] * m[12];
    dst[1] = v[0] * m[1] + v[1] * m[5] + v[2] * m[9] + v[3] * m[13];
    dst[2] = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + v[3] * m[14];
    dst[3] = v[0] * m[3] + v[1] * m[7] + v[2] * m[11] + v[3] * m[15];
    return dst;
}

static std::array<int, 2> modelview_to_window_coords(const std::array<GLfloat, 4> in_mv, const std::array<GLfloat, 16> & mv, const std::array<GLfloat, 16> & pr, const std::array<GLint, 4> & viewport) {
    std::array<GLfloat, 4> eye = multMatrixVec4f(mv, in_mv);
    std::array<GLfloat, 4> ndc = multMatrixVec4f(pr, eye);
    ndc[3] = 1.0f / ndc[3];
    ndc[0] *= ndc[3];
    ndc[1] *= ndc[3];

    std::array<int, 2> out_w;
    out_w[0] = (ndc[0] * 0.5f + 0.5f) * viewport[2] + viewport[0];
    out_w[1] = (ndc[1] * 0.5f + 0.5f) * viewport[3] + viewport[1];
    return out_w;
}

ScrollContainer::ScrollContainer() : scroll_bar_(std::make_shared<VerticalScrollBar>(this)) {
    mv_dref = XPLMFindDataRef("sim/graphics/view/modelview_matrix");
    vp_dref = XPLMFindDataRef("sim/graphics/view/viewport");
    pr_dref = XPLMFindDataRef("sim/graphics/view/projection_matrix");
}

void ScrollContainer::draw(Rect draw_bounds) {
    // fix scroll, if the contents have changed. We should figure out a more efficient way to do this.
    mouseWheel({}, 0, 0); //no-op scroll, but will at least clamp bounds
    
    // Generally this is the XP11+ path
    if(mv_dref && pr_dref && vp_dref) {
        // Get the current modelview matrix, viewport, and projection matrix from X-Plane
        std::array<float, 16> mv, pr;
        std::array<int, 4> vp;
        XPLMGetDatavf(mv_dref, mv.data(), 0, 16);
        XPLMGetDatavf(pr_dref, pr.data(), 0, 16);
        XPLMGetDatavi(vp_dref, vp.data(), 0, 4);
        
        // Our new modelview bounds
        std::array<GLfloat, 4> top_right_modelview{ static_cast<float>(bounds_.right), static_cast<float>(bounds_.top), 0.f, 1.f };
        std::array<GLfloat, 4> btm_left_modelview{ static_cast<float>(bounds_.left), static_cast<float>(bounds_.bottom), 0.f, 1.f };
        
        // Get our top-right and bottom-left window coordinates
        std::array<int, 2> top_right_window = modelview_to_window_coords(top_right_modelview, mv, pr, vp);
        std::array<int, 2> btm_left_window = modelview_to_window_coords(btm_left_modelview,  mv, pr, vp);
        
        glScissor(btm_left_window[0], btm_left_window[1], top_right_window[0] - btm_left_window[0], top_right_window[1] - btm_left_window[1]);
    } else {
        glScissor(bounds_.left, bounds_.bottom, getWidth(), getHeight());
    }

    glEnable(GL_SCISSOR_TEST);
    glTranslatef(0.f, scroll_distance, 0.f);
    Rect scrolled_draw_bounds = draw_bounds.translated(0, -scroll_distance);
    object_->draw(scrolled_draw_bounds);
    glTranslatef(0.f, -scroll_distance, 0.f);
    glDisable(GL_SCISSOR_TEST);

    //draw scroll bar if necessary?

    if(shows_scroll_bar_) {
        scroll_bar_->draw(draw_bounds);
    }
}

bool ScrollContainer::mouseWheel(Point, int /* wheel */, int clicks) {
    if(0 == clicks) {   //why?
        return true;
    }

    scroll_distance -= clicks * 8;

    int contents_height = object_->getMinimumSize().height;
    int scroll_bar_height = getHeight();

    int max_scroll = std::max(0, contents_height - scroll_bar_height);
    scroll_distance = std::max(0, std::min(scroll_distance, max_scroll));

    onScrollChange();
    return true;
}

std::shared_ptr<Widget11> ScrollContainer::mouseClick(Point point, XPLMMouseStatus status) {
    if(shows_scroll_bar_ && scroll_bar_->overlaps(point)) {
        if(scroll_bar_->mouseClick(point, status)) {
            return scroll_bar_;
        }
    }

    point.y -= scroll_distance;
    std::shared_ptr<WidgetContainer> object = std::dynamic_pointer_cast<WidgetContainer>(object_);
    std::shared_ptr<Widget11> widget = std::dynamic_pointer_cast<Widget11>(object_);
    if(object) {
        return object->mouseClick(point, status);
    } else if(widget) {
        if(widget->mouseClick(point, status)) {
            return widget;
        }
    }

    return nullptr;
}
