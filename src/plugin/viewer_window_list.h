#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "XPWidgets.h"
#include "XPStandardWidgets.h"

#include "search.h"
#include "viewer_window_command_button.h"

#include "modelview_to_window.h"

class RefRecord;
class CommandRefRecord;
class DataRefRecord;

// These are the Once/Begin/End buttons for a single row of results.
class CommandButtonRow {
    XPWidgetID command_once_button = nullptr;
    XPWidgetID command_press_button = nullptr;
    XPWidgetID command_begin_button = nullptr;
public:
    CommandButtonRow(XPWidgetID window) {
        assert(nullptr != window);
        command_once_button = XPCreateCustomWidget(0, 0, 1, 1, 1,"Once", 0, window, commandOnceButtonCallback);
        command_press_button = XPCreateCustomWidget(0, 0, 1, 1, 1,"Press", 0, window, commandPressButtonCallback);
        command_begin_button = XPCreateCustomWidget(0, 0, 1, 1, 1,"Begin", 0, window, commandHoldButtonCallback);
        XPSetWidgetProperty(command_once_button, xpProperty_Object, reinterpret_cast<intptr_t>(nullptr));
        XPSetWidgetProperty(command_press_button, xpProperty_Object, reinterpret_cast<intptr_t>(nullptr));
        XPSetWidgetProperty(command_begin_button, xpProperty_Object, reinterpret_cast<intptr_t>(nullptr));
    }

    ~CommandButtonRow() {
        XPDestroyWidget(command_once_button, 1);
        XPDestroyWidget(command_press_button, 1);
        XPDestroyWidget(command_begin_button, 1);
    }

    void showAtPosition(int left, int top, int right, int bottom) {
        const int gap = 5;

        auto position_and_show_widget = [&](XPWidgetID widget, int width, int & right_bound) -> void {
            int widget_right = right_bound;
            int widget_left = right_bound - width;
            right_bound -= width + gap;
            XPSetWidgetGeometry(widget, widget_left, top, widget_right, bottom);
            if(widget_left >= left) {
                XPShowWidget(widget);
            } else {
                XPHideWidget(widget);
            }
        };

        int button_right_bound = right;

        position_and_show_widget(command_once_button, 33, button_right_bound);
        position_and_show_widget(command_begin_button, 40, button_right_bound);
        position_and_show_widget(command_press_button, 51, button_right_bound);
    }

    void setCommand(CommandRefRecord * new_command) {
        XPSetWidgetProperty(command_once_button, xpProperty_Object, reinterpret_cast<intptr_t>(new_command));
        XPSetWidgetProperty(command_press_button, xpProperty_Object, reinterpret_cast<intptr_t>(new_command));
        XPSetWidgetProperty(command_begin_button, xpProperty_Object, reinterpret_cast<intptr_t>(new_command));
    }

    void hide() {
        XPHideWidget(command_once_button);
        XPHideWidget(command_press_button);
        XPHideWidget(command_begin_button);
    }
};

class ViewerWindowList {
	int fontheight, fontwidth;
    XPWidgetID list_widget = nullptr;
	XPWidgetID scroll_bar_widget = nullptr;
	XPWidgetID edit_field = nullptr;

    ModelviewToWindowCoordinateConverter scissor_coordinate_converter;

	DataRefRecord * select_edit_dataref = nullptr;
	CommandRefRecord * selected_command = nullptr;
	bool edit_modified = false;

	std::shared_ptr<SearchResults> & results;

	std::vector<std::unique_ptr<CommandButtonRow>> command_buttons;

    static int listCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t, intptr_t);
    static int editFieldCallback(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t  inParam1, intptr_t);

    bool saveEditField();
    std::string getEditText() const;
    void setEditText(const std::string & text);

    intptr_t getEditSelectionStart() const;
    intptr_t getEditSelectionStop() const;

	void setEditSelection(intptr_t start, intptr_t stop);
    size_t getScrollPosition() const;

    enum class MouseButton {
        LEFT,
        RIGHT,
    };

    bool leftClick(int x, int y, MouseButton button);

public:
    ViewerWindowList(XPWidgetID window, std::shared_ptr<SearchResults> & results);
    void draw();
    void resize(int left, int top, int right, int bottom);
    void updateScroll();
    void updateScroll(int left, int top, int right, int bottom);
    void moveScroll(int amount);

    void updateCommandButtons();

	void deselectEditField();

    DataRefRecord * getSelectedDataref() const { return select_edit_dataref; }
    CommandRefRecord * getSelectedCommand() const { return selected_command; }
};
