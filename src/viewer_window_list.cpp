#include "viewer_window_list.h"

#include "allrefs.h"
#include "clipboard.h"
#include "commandref.h"
#include "string_util.h"

#include <boost/range/iterator_range.hpp>

#include "XPLMGraphics.h"

#include <cctype>
#include <chrono>
#include <iterator>
#include <string>

#include "../lib/glew/glew.h"

#if APL
#include <OpenGL/gl.h>
#endif

const XPLMFontID font = xplmFont_Basic;

ViewerWindowList::ViewerWindowList(XPWidgetID window, std::vector<RefRecord *> & results)
: results(results) {

    XPLMGetFontDimensions(font, &fontwidth, &fontheight, nullptr);
    list_widget = XPCreateCustomWidget(0, 0, 1, 1, 1,"", 0, window, listCallback);
    XPSetWidgetProperty(list_widget, xpProperty_Object, reinterpret_cast<intptr_t>(this));

    scroll_bar_widget = XPCreateWidget(0, 0, 1, 1, 1,"", 0, window, xpWidgetClass_ScrollBar);
    XPSetWidgetProperty(scroll_bar_widget, xpProperty_ScrollBarType, xpScrollBarTypeScrollBar);	//might need changing
    updateScroll();

    edit_field = XPCreateWidget(0, 0, 1, 1, 1,"", 0, window, xpWidgetClass_TextField);
    XPSetWidgetProperty(edit_field, xpProperty_TextFieldType, xpTextEntryField);
    XPAddWidgetCallback(edit_field, editFieldCallback);
    XPSetWidgetProperty(edit_field, xpProperty_Object, reinterpret_cast<intptr_t>(this));
    XPHideWidget(edit_field);
}


std::string ViewerWindowList::getEditText() const {
    char editfield_text[1024];
    XPGetWidgetDescriptor(edit_field, editfield_text, 1024);
    return std::string(editfield_text);
}

intptr_t ViewerWindowList::getEditSelectionStart() const {
    return XPGetWidgetProperty(edit_field, xpProperty_EditFieldSelStart, NULL);
}

intptr_t ViewerWindowList::getEditSelectionStop() const {
    return XPGetWidgetProperty(edit_field, xpProperty_EditFieldSelEnd, NULL);
}

void ViewerWindowList::setEditSelection(intptr_t start, intptr_t stop) {
    XPSetWidgetProperty(edit_field, xpProperty_EditFieldSelStart, start);
    XPSetWidgetProperty(edit_field, xpProperty_EditFieldSelEnd, stop);
}

void ViewerWindowList::deselectEditField() {
    edit_modified = false;
    select_edit_dataref = nullptr;
    selected_command = nullptr;
    XPLoseKeyboardFocus(edit_field);
    XPHideWidget(edit_field);
}

int ViewerWindowList::listCallback(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t inParam1, intptr_t) {
    ViewerWindowList * obj = reinterpret_cast<ViewerWindowList *>(XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr));
    switch(inMessage) {
        case xpMsg_Draw:
            obj->draw();
            return 1;
        case xpMsg_MouseDown: {
            XPMouseState_t * mouse_info = reinterpret_cast<XPMouseState_t *>(inParam1);
            if(mouse_info->button == 0) {
                return obj->leftClick(mouse_info->x, mouse_info->y, MouseButton::LEFT) ? 1 : 0;
            }
            if(mouse_info->button == 1) {
                return obj->leftClick(mouse_info->x, mouse_info->y, MouseButton::RIGHT) ? 1 : 0;
            }

            return 0;
        }
    }
    return 0;
}

bool ViewerWindowList::leftClick(int x, int y, MouseButton button) {
    int list_left, list_top, list_bottom, list_right;
    XPGetWidgetGeometry(list_widget, &list_left, &list_top, &list_right, &list_bottom);

    x -= list_left;

    int y_from_top = list_top - y;

    const int scroll_position = getScrollPosition();
    int list_y = y_from_top + getScrollPosition();
    int element_index = list_y / fontheight;

    int element_top = list_top + scroll_position - element_index * fontheight;
    int element_bottom = element_top - fontheight;

    // if not a click on an element
    if(element_index < 0 || element_index >= static_cast<int>(results.size())) {
        return false;
    }

    RefRecord * record = results[element_index];
    const std::string & name = record->getName();
    float dataref_name_width = XPLMMeasureString(font, name.c_str(), int(name.size()) + 1);
    float dataref_name_width_plus_eq = XPLMMeasureString(font, (name + "=").c_str(), int(name.size()) + 2);

    const int box_padding_x = 2, box_padding_y = 2;

    DataRefRecord * dr_record = dynamic_cast<DataRefRecord *>(record);
    CommandRefRecord * cr_record = dynamic_cast<CommandRefRecord *>(record);
    switch(button) {
        case MouseButton::LEFT:
            if(nullptr != dr_record) {
                if(x < dataref_name_width) {
                    XPSetWidgetGeometry(edit_field, list_left, element_top + box_padding_y, list_left + dataref_name_width + box_padding_x, element_bottom - box_padding_y);
                    XPSetWidgetDescriptor(edit_field, name.c_str());
                    setEditSelection(0, name.size());
                } else {
                    const std::string value_str = dr_record->getEditString();
                    select_edit_dataref = dr_record;
                    selected_command = nullptr;
                    XPSetWidgetGeometry(edit_field, list_left + dataref_name_width_plus_eq - box_padding_x, element_top + box_padding_y, list_right, element_bottom - box_padding_y);
                    XPSetWidgetDescriptor(edit_field, value_str.c_str());
                    setEditSelection(0, value_str.size());
                }
                XPShowWidget(edit_field);
                edit_modified = false;
            } else  if(nullptr != cr_record) {
                selected_command = cr_record;
                select_edit_dataref = nullptr;

                XPSetWidgetGeometry(edit_field, list_left, element_top + box_padding_y, list_left + dataref_name_width + box_padding_x, element_bottom - box_padding_y);
                XPSetWidgetDescriptor(edit_field, name.c_str());
                setEditSelection(0, name.size());
                XPShowWidget(edit_field);
                edit_modified = false;
            } else {
                selected_command = nullptr;
                select_edit_dataref = nullptr;
                XPHideWidget(edit_field);
            }
            break;
        case MouseButton::RIGHT:
            if(nullptr != record) {
                setClipboard(record->getName());
            }
            break;
    }
    return 1;
}

size_t ViewerWindowList::getScrollPosition() const {
    const int scroll_pos = static_cast<int>(XPGetWidgetProperty(scroll_bar_widget, xpProperty_ScrollBarSliderPosition, nullptr));
    const int scroll_pos_max = static_cast<int>(XPGetWidgetProperty(scroll_bar_widget, xpProperty_ScrollBarMax, nullptr));
    return scroll_pos_max - scroll_pos;
}

void ViewerWindowList::draw() {
    int list_left, list_top, list_right, list_bottom;
    XPGetWidgetGeometry(list_widget, &list_left, &list_top, &list_right, &list_bottom);
    const int list_width = list_right - list_left;
    const int list_width_in_chars = list_width / fontwidth;
    glScissor(list_left, list_bottom, list_right - list_left, list_top - list_bottom);
    glEnable(GL_SCISSOR_TEST);

    const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    size_t scroll_position = getScrollPosition();
    size_t result_index = std::max<size_t>(0, scroll_position / fontheight);

    std::string linetext;
    int top = list_top + scroll_position - (fontheight * result_index);
    for(const RefRecord * record : boost::iterator_range<std::vector<RefRecord *>::const_iterator>(results.cbegin() + result_index, results.cend())) {
        std::array<float, 3> colors;
        int bottom = top - fontheight;
        if(top < list_bottom) {
            break;
        }
        float timediff = 0.001f * std::chrono::duration_cast<std::chrono::milliseconds>(now - record->getLastUpdateTime()).count();
        float timediff_fraction = std::min<float>(1.f, timediff / 10.f);
        const DataRefRecord * dr_record = dynamic_cast<const DataRefRecord *>(record);
        if(nullptr != dr_record) {
            const std::string & label = dr_record->getLabelString();
            size_t max_value_chars = std::max<int>(0, list_width_in_chars - 1 - int(label.size()));
            linetext += label;
            linetext += "=";
            linetext += record->getDisplayString(max_value_chars);
            if(record->isBlacklisted()) {
                colors = {{1.f, .3f, .3f}}; //red ish
            } else {
                colors = {{0.2f + timediff_fraction * 0.8f, 1.f, 1.f}};
            }
        } else {
            const CommandRefRecord * pcr = dynamic_cast<const CommandRefRecord *>(record);
            colors = {{1.f - timediff_fraction, 1.f, 0.f}}; //green
            linetext = pcr->getName();
        }

        XPLMDrawString(colors.data(), list_left, bottom + 2, const_cast<char *>(linetext.c_str()), nullptr, font);
        linetext.clear();
        top -= fontheight;
    }

    glDisable(GL_SCISSOR_TEST);
}

void ViewerWindowList::resize(int left, int top, int right, int bottom) {
    const int scroll_bar_width = 16;
    XPSetWidgetGeometry(list_widget, left, top, right - scroll_bar_width, bottom);

    XPSetWidgetGeometry(scroll_bar_widget, right - scroll_bar_width, top, right, bottom);
    updateScroll(left, top, right, bottom);
}


void ViewerWindowList::updateScroll(int /* left */, int top, int /* right */, int bottom) {

    int results_height = results.size() * fontheight;
    int window_height = top - bottom;
    int scroll_pixels = std::max<int>(0, results_height - window_height);

    //update the scrollbar
    int scroll_pos = static_cast<int>(XPGetWidgetProperty(scroll_bar_widget, xpProperty_ScrollBarSliderPosition, nullptr));

    scroll_pos = std::max<int>(0, std::min(scroll_pixels, scroll_pos));

    XPSetWidgetProperty(scroll_bar_widget, xpProperty_ScrollBarMin, 0);
    XPSetWidgetProperty(scroll_bar_widget, xpProperty_ScrollBarMax, scroll_pixels);
    XPSetWidgetProperty(scroll_bar_widget, xpProperty_ScrollBarSliderPosition, scroll_pos);
    updateCommandButtons();
}

void ViewerWindowList::updateScroll() {
    int left, top, right, bottom;
    XPGetWidgetGeometry(list_widget, &left, &top, &right, &bottom);
    updateScroll(left, top, right, bottom);
}

void ViewerWindowList::updateCommandButtons() {
    int list_left, list_top, list_right, list_bottom;
    XPGetWidgetGeometry(list_widget, &list_left, &list_top, &list_right, &list_bottom);

    size_t scroll_position = getScrollPosition();
    size_t result_index = std::max<size_t>(0, scroll_position / fontheight);

    int top = list_top + scroll_position - (fontheight * result_index);

    // we need one command button for each possible line. This is at most one for each fontheight pixels, plus one for the top and one for the bottom.
    size_t desired_buttons = (list_top - list_bottom) / fontheight + 2;
    while(desired_buttons > command_buttons.size()) {
        command_buttons.emplace_back(std::make_unique<CommandButtonRow>(list_widget));
    }

    for(std::unique_ptr<CommandButtonRow> & cbr : command_buttons) {
        int bottom = top - fontheight;
        if(bottom < list_bottom || result_index >= results.size()) { // if not visible or past end of results
            cbr->hide();
        } else {
            CommandRefRecord * crr = dynamic_cast<CommandRefRecord *>(results[result_index]);
            if(nullptr != crr) {
                float command_name_width = XPLMMeasureString(font, crr->getName().c_str(), int(crr->getName().size()));
                cbr->showAtPosition(list_left + ceil(command_name_width), top, list_right, bottom);
                cbr->setCommand(crr);
            } else {
                cbr->hide();
            }
        }

        result_index++;
        top -= fontheight;
    }
}


void ViewerWindowList::moveScroll(int amount) {
    intptr_t scroll_current = XPGetWidgetProperty(scroll_bar_widget, xpProperty_ScrollBarSliderPosition, nullptr) + amount;
    intptr_t scroll_max = XPGetWidgetProperty(scroll_bar_widget, xpProperty_ScrollBarMax, nullptr);
    intptr_t scroll_new = std::max<intptr_t>(0, std::min<intptr_t>(scroll_max, scroll_current + amount));

    XPSetWidgetProperty(scroll_bar_widget, xpProperty_ScrollBarSliderPosition, scroll_new);
    deselectEditField();
    updateCommandButtons();
}

bool ViewerWindowList::saveEditField() {
    if(edit_modified) {
        const std::string edit_txt = getEditText();

        if(select_edit_dataref->isDouble()) {
            try {
                double d = std::stold(edit_txt);
                select_edit_dataref->setDouble(d);
            } catch(std::exception &) {
                return false;
            }
        } else if(select_edit_dataref->isFloat()) {
            try {
                float f = std::stof(edit_txt);
                select_edit_dataref->setFloat(f);
            } catch(std::exception &) {
                return false;
            }
        } else if(select_edit_dataref->isInt()) {
            try {
                int i = std::stoi(edit_txt);
                select_edit_dataref->setInt(i);
            } catch(std::exception &) {
                return false;
            }
        } else if(select_edit_dataref->isFloatArray()) {
            std::vector<float> array;
            if(false == parseArray<float>(edit_txt, array, select_edit_dataref->getArrayLength())) {
                return false;
            }
            select_edit_dataref->setFloatArray(array);
        } else if(select_edit_dataref->isIntArray()) {
            std::vector<int> array;
            if(false == parseArray<int>(edit_txt, array, select_edit_dataref->getArrayLength())) {
                return false;
            }
            select_edit_dataref->setIntArray(array);
        }
    }

    edit_modified = false;
    return true;
}

int ViewerWindowList::editFieldCallback(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t  inParam1, intptr_t) {
    ViewerWindowList * obj = reinterpret_cast<ViewerWindowList *>(XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr));
    XPKeyState_t * keystruct = reinterpret_cast<XPKeyState_t *>(inParam1);
    switch(inMessage) {
        case xpMsg_DescriptorChanged:
            return 1;
        case xpMsg_KeyPress:
            if(keystruct->flags & 6 && keystruct->flags & xplm_DownFlag) {	//alt, command, control are down
                switch(keystruct->vkey) {
                    case XPLM_VK_A:	//select all
                    {
                        size_t length = obj->getEditText().size();
                        obj->setEditSelection(0, length);
                        return 1;
                    }
                    case XPLM_VK_X:	//cut
                    case XPLM_VK_C:	//copy 
                    {
                        size_t start = obj->getEditSelectionStart();
                        size_t stop = obj->getEditSelectionStop();
                        std::string search_text = obj->getEditText();
                        std::string cut_text = search_text.substr(start, stop - start);
                        setClipboard(cut_text);
                        return 1;
                    }
                }
            } else {
                uint8_t vkey = keystruct->vkey;
                uint8_t key = keystruct->key;
                switch(vkey) {
                    default:
                        if(nullptr == obj->select_edit_dataref || false == obj->select_edit_dataref->writable()) {
                            return 1;
                        } else if(std::isalpha(key) || (obj->select_edit_dataref->isInt() && key == '.')) {
                            return 1;
                        } else if(key == ',') {
                            return obj->select_edit_dataref->isArray() ? 0 : 1;
                        } else {
                            obj->edit_modified = true;
                            return 0;
                        }

                    case XPLM_VK_NUMPAD_ENT:
                    case XPLM_VK_ENTER:
                    case XPLM_VK_RETURN:
                    case XPLM_VK_TAB:
                        if(obj->saveEditField()) {
                            obj->deselectEditField();
                        }
                        return 1;
                    case XPLM_VK_ESCAPE:
                        obj->deselectEditField();
                        return 1;
                }
            }
    }
    return 0;
}
