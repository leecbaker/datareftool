#pragma once

#include "container.h"

class VerticalScrollBar;

// Simple scrollview. Has a scrollbar, and scrolls content inside if the size of the content is greater than the
// size of this view. Contains a single widget.
//
// Since there's no X-axis scroll support in X-Plane, this only scrolls the Y axis. The X axis acts as a normal
// container.
class ScrollContainer : public WidgetContainer {
    std::shared_ptr<LayoutObject> object_;
    std::shared_ptr<VerticalScrollBar> scroll_bar_;
    int scroll_distance = 0;
    bool shows_scroll_bar_ = false;
    int min_height_scroll = 0; // minimum height for the scrollable section.

    XPLMDataRef mv_dref = nullptr;
    XPLMDataRef vp_dref = nullptr;
    XPLMDataRef pr_dref = nullptr;

    void onScrollChange();
public:
    ScrollContainer();
    virtual ~ScrollContainer() = default;

    void setContents(std::shared_ptr<LayoutObject> contents) {
        object_ = std::move(contents);
    }

    virtual void draw(Rect draw_bounds) override;

    virtual Size getMinimumSize() const override {
        Size contained_min_size = object_->getMinimumSize();
        return {contained_min_size.width + 10, std::max(32 ,min_height_scroll)};
    }

    void setMinimumHeight(int new_min_height) { min_height_scroll = new_min_height; }

    virtual void setSize(int x, int y, int width, int height) override;
    void setScrollFraction(float fraction);
    void recomputeScroll();

    virtual bool isStretchableY() const override { return true; }

    virtual bool mouseWheel(Point point, int wheel, int clicks) override;

    virtual std::shared_ptr<Widget11> mouseClick(Point point, XPLMMouseStatus status) override;


    virtual bool acceptsKeyboardFocus() const override { return object_->acceptsKeyboardFocus(); }
    virtual bool hasKeyboardFocus() const override { return object_->hasKeyboardFocus(); }

    virtual void removeKeyboardFocus() override { object_->removeKeyboardFocus(); }

    virtual void keyPress(char key, XPLMKeyFlags flags, uint8_t virtual_key) override { object_->keyPress(key, flags, virtual_key); }

    virtual bool advanceKeyboardFocus() override { return object_->advanceKeyboardFocus(); }


    virtual XPLMCursorStatus handleCursor(Point point, bool mouse_inside) override { return object_->handleCursor(point.translated(0, -scroll_distance), mouse_inside); }
};