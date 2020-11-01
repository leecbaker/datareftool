#include "selectable_list.h"

#include "XPLMGraphics.h"

#include "draw_basic.h"
#include "gl_utils.h"

#include "search/ref.h"
#include "search/commandref.h"
#include "search/dataref.h"

#include <chrono>

const XPLMFontID font = xplmFont_Basic;

void SelectableListBase::draw(Rect visible_bounds) {
    XPLMSetGraphicsState(0,0,0,0,1,0,0);
    
    //draw black background around whole box
    glColor3f(0.f, 0.f, 0.f);
    drawRect(visible_bounds);

    // draw blue background behind selected element
    std::shared_ptr<Widget11> widget_locked = tentative_selected_element.lock();
    if(widget_locked) {
        std::optional<Rect> overlap_rect = visible_bounds.intersection(widget_locked->getBounds());
        if(overlap_rect) {
            color3fGL(color3fFromHex(0x36, 0x78, 0xb0));
            drawRect(*overlap_rect);
        }
    }

    SingleAxisLayoutContainer::draw(visible_bounds);
}

std::shared_ptr<Widget11> SelectableListBase::mouseClick(Point point, XPLMMouseStatus status) {
    for(std::shared_ptr<LayoutObject> & widget: *this) {
        if(widget->getBounds().contains(point)) {
            std::shared_ptr<Widget11> selected_widget = std::dynamic_pointer_cast<Widget11, LayoutObject>(widget);

            std::weak_ptr<Widget11> selected_widget_weak(selected_widget);
            tentative_selected_element = selected_widget_weak;
            if(status == xplm_MouseUp) {
                fireSelectionChange(selected_element, selected_widget_weak);
                selected_element = selected_widget_weak;
            }
            return selected_widget;
        }
    }

    std::weak_ptr<Widget11> no_selection;
    fireSelectionChange(selected_element, no_selection);
    selected_element.reset();

    return nullptr;
}

void SelectableListBase::keyPress(char key, XPLMKeyFlags flags, uint8_t virtual_key) {
    SelectableListBase::widget_iterator_type selected_it = std::find(begin(), end(), selected_element.lock());

    if(flags & xplm_DownFlag) {
        switch(virtual_key) {
            case XPLM_VK_UP:
                if(selected_it != end() && selected_it != begin()) {
                    selected_it--;
                    tentative_selected_element = selected_element = std::dynamic_pointer_cast<Widget11, LayoutObject>(*selected_it);
                }
                break;
            case XPLM_VK_DOWN:
                if(selected_it != end() && selected_it + 1 != end()) {
                    selected_it++;
                    tentative_selected_element = selected_element = std::dynamic_pointer_cast<Widget11, LayoutObject>(*selected_it);
                }
                break;
            case XPLM_VK_ESCAPE: {
                std::weak_ptr<Widget11> no_selection;
                fireSelectionChange(selected_element, no_selection);
                selected_element.reset();
                tentative_selected_element.reset();
            }
                break;
            default:
                break;
        }
    }
}

void ResultLine::setRecord(RefRecord * rr) {
    setText(rr->getName());
    this->record = rr;
}

void ResultLine::draw(Rect draw_bounds) {
    float timediff = 0.001f * std::chrono::duration_cast<std::chrono::milliseconds>(*last_update_timestamp - record->getLastUpdateTime()).count();
    float timediff_fraction = std::min<float>(1.f, timediff / 10.f);
    const DataRefRecord * dr_record = dynamic_cast<const DataRefRecord *>(record);
    std::string name;
    std::string value;
    std::array<float, 3> colors;
    if(nullptr != dr_record) {
        name = dr_record->getLabelString();

        float name_width = XPLMMeasureString(font, const_cast<char *>(name.c_str()), name.size());
        float space_for_value_string = getWidth() - name_width;

        int char_width = 0;
        XPLMGetFontDimensions(font, &char_width, nullptr, nullptr);

        value = dr_record->getDisplayString(static_cast<int>(space_for_value_string) / char_width);

        if(record->isBlacklisted()) {
            colors = {{1.f, .3f, .3f}}; //red ish
        } else {
            colors = {{0.2f + timediff_fraction * 0.8f, 1.f, 1.f}};
        }
    } else {
        const CommandRefRecord * pcr = dynamic_cast<const CommandRefRecord *>(record);
        colors = {{1.f - timediff_fraction, 1.f, 0.f}}; //green
        name = pcr->getName();
    }

    Rect widget_bounds = getBounds();

    XPLMDrawString(colors.data(), widget_bounds.left, widget_bounds.bottom + 2, const_cast<char *>(name.c_str()), nullptr, font);
    if(false == value.empty()) {
        float value_width = XPLMMeasureString(font, const_cast<char *>(value.c_str()), value.size());

        XPLMDrawString(colors.data(), widget_bounds.right - value_width, widget_bounds.bottom + 2, const_cast<char *>(value.c_str()), nullptr, font);
    }
}

// for now, recreate all widgets when necessary
void ResultsList::update() {
    clear();

    for(RefRecord * rr : (*results)) {
        std::shared_ptr<ResultLine> line_widget = std::make_shared<ResultLine>(&results->getLastUpdateTimestamp());
        line_widget->setRecord(rr);
        SingleAxisLayoutContainer::addNoLayout(std::move(line_widget), false, true);
    }

    recomputeLayout();
}

void ResultsList::draw(Rect draw_bounds)
{
    if (results->size() != widget_count())
    {
        update();
    }
    SelectableList<ResultLine>::draw(draw_bounds);
}
