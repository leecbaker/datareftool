#include "viewer_window.h"
#include "datarefs.h"
#include "clipboard.h"

#include "XPLMDisplay.h"
#include "XPCWidget.h"
#include "XPStandardWidgets.h"
#include "XPLMDataAccess.h"
#include "XPLMGraphics.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <set>
#include <string>

const XPLMFontID font = xplmFont_Basic;

const int bottom_row_height = 20;
const int right_col_width = 20;
const int toggle_button_width = 28;
const int title_bar_height = 20;

int last_top = -1, last_bottom = -1, last_left = -1, last_right = -1;
std::string last_search_term;
bool last_case_sensitive = false, last_regex = false, last_changed = false;

class DatarefViewerWindow;

void closeViewerWindow(DatarefViewerWindow * window);

class DatarefViewerWindow {
	XPLMWindowID window = nullptr;
	XPWidgetID regex_toggle_button = nullptr;
	XPWidgetID case_sensitive_button = nullptr;
	XPWidgetID change_filter_button = nullptr;
	XPWidgetID search_field = nullptr;
	XPWidgetID edit_field = nullptr;
	XPWidgetID scroll_bar = nullptr;
	XPWidgetID custom_list = nullptr;

	int drag_start_mouse_x = 0, drag_start_mouse_y = 0;
	int drag_start_window_left = 0, drag_start_window_right = 0;
	int drag_start_window_top = 0, drag_start_window_bottom = 0;
	bool in_resize_left = false;
	bool in_resize_right = false;
	bool in_resize_bottom = false;

	static const int mouse_drag_margin = 7;
	int fontheight;
	int displayed_lines = 0;
	int list_start_index = 0;

	DataRefRecord * select_edit_dataref = nullptr;
	bool edit_modified = false;

	std::vector<DataRefRecord *> datarefs;

	static int viewerWindowCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t) {
		XPMouseState_t * mouse_info = (XPMouseState_t *) inParam1;
		DatarefViewerWindow * obj = (DatarefViewerWindow *) XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr);
		switch(inMessage) {
			case xpMsg_MouseWheel:
				obj->moveScroll(mouse_info->delta);
				return 1;
			case xpMessage_CloseButtonPushed:
				closeViewerWindow(obj);
				return 1;
			case xpMsg_MouseDown:
				XPGetWidgetGeometry(obj->window, &obj->drag_start_window_left, &obj->drag_start_window_top, &obj->drag_start_window_right, &obj->drag_start_window_bottom);
				if(mouse_info->x < obj->drag_start_window_left + mouse_drag_margin) { obj->in_resize_left = true; }
				if(mouse_info->x > obj->drag_start_window_right - mouse_drag_margin) { obj->in_resize_right = true; }
				if(mouse_info->y < obj->drag_start_window_bottom + mouse_drag_margin) { obj->in_resize_bottom = true; }

				if(obj->in_resize_left || obj->in_resize_right || obj->in_resize_bottom) {
					obj->drag_start_mouse_x = mouse_info->x;
					obj->drag_start_mouse_y = mouse_info->y;
					return 1;
				} else {
					int click_y_offset_from_list_top = obj->drag_start_window_top - mouse_info->y - title_bar_height;
					if(click_y_offset_from_list_top < 0) {	//click in title bar
						return 0;
					}
					int click_y_offset_index = click_y_offset_from_list_top / obj->fontheight;
					int displayed_list_elements = std::min<int>(obj->displayed_lines, obj->datarefs.size());

					int click_list_index = click_y_offset_index + obj->list_start_index;

					if(click_y_offset_index < displayed_list_elements && 0 <= click_list_index) {	//click is over a real list entry
						DataRefRecord * record = obj->datarefs[click_list_index];
						const std::string name = record->getName();
						float dataref_name_width = XPLMMeasureString(font, name.c_str(), int(name.size()) + 1);
						float dataref_name_width_plus_eq = XPLMMeasureString(font, (name + "=").c_str(), int(name.size()) + 2);
						const int fontheight = obj->fontheight;

						int scroll_left, scroll_right; 
						XPGetWidgetGeometry(obj->scroll_bar, &scroll_left, nullptr, &scroll_right, nullptr);
						const int scroll_width = scroll_right - scroll_left;

						//click is over name?
						int top = obj->drag_start_window_top - title_bar_height - click_y_offset_index * fontheight;
						int bottom = obj->drag_start_window_top - title_bar_height - (1 + click_y_offset_index) * fontheight;

						int nameend_x = obj->drag_start_window_left + int(dataref_name_width);
						int valuestart_x = obj->drag_start_window_left + int(dataref_name_width_plus_eq);
						const int box_padding_x = 6;
						const int box_padding_y = 6;

						if(mouse_info->x < obj->drag_start_window_left + dataref_name_width) {
							XPSetWidgetGeometry(obj->edit_field, obj->drag_start_window_left, top, nameend_x + box_padding_x, bottom - box_padding_y);
							XPSetWidgetDescriptor(obj->edit_field, name.c_str());
							obj->setEditSelection(0, name.size());
						} else {
							const std::string value_str = record->getValueString();
							obj->select_edit_dataref = record;
							XPSetWidgetGeometry(obj->edit_field, valuestart_x, top, obj->drag_start_window_right - mouse_drag_margin - scroll_width + box_padding_x, bottom - box_padding_y);
							XPSetWidgetDescriptor(obj->edit_field, value_str.c_str());
							obj->setEditSelection(0, value_str.size());
						}

						XPShowWidget(obj->edit_field);
						obj->edit_modified = false;
						return 1;
					}
				}

				return 0;
			case xpMsg_MouseUp:
				if(!obj->in_resize_left && !obj->in_resize_right && !obj->in_resize_bottom) { break; }
				obj->in_resize_left = false;
				obj->in_resize_right = false;
				obj->in_resize_bottom = false;
				break;
			case xpMsg_MouseDrag:
				if(!obj->in_resize_left && !obj->in_resize_right && !obj->in_resize_bottom) { break; }
				int delta_x = mouse_info->x - obj->drag_start_mouse_x;
				int delta_y = mouse_info->y - obj->drag_start_mouse_y;

				int left_delta = obj->in_resize_left ? delta_x : 0;
				int right_delta = obj->in_resize_right ? delta_x : 0;
				int bottom_delta = obj->in_resize_bottom ? delta_y : 0;
				
				XPSetWidgetGeometry(obj->window, obj->drag_start_window_left + left_delta, obj->drag_start_window_top, obj->drag_start_window_right + right_delta, obj->drag_start_window_bottom + bottom_delta);
				obj->resize();
				return 1;
		}

		return 0;
	}

	static int searchFieldCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t) {
		DatarefViewerWindow * obj = (DatarefViewerWindow *) XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr);
		XPKeyState_t * keystruct = (XPKeyState_t *) inParam1;
		switch(inMessage) {
			case xpMsg_DescriptorChanged:
				obj->doSearch();
				return 1;
			case xpMsg_KeyPress:
				if(keystruct->flags & 6 && keystruct->flags & xplm_DownFlag) {	//alt, command, control are down
					switch(keystruct->vkey) {
						case XPLM_VK_A:	//select all
						{
							size_t length = obj->getSearchText().size();
							obj->setSearchSelection(0, length);
							return 1;
						}
						case XPLM_VK_X:	//cut
						{
							size_t start = obj->getSearchSelectionStart();
							size_t stop = obj->getSearchSelectionStop();
							std::string search_text = obj->getSearchText();
							std::string cut_text = search_text.substr(start, stop - start);
							search_text.erase(search_text.begin() + start, search_text.begin() + stop);
							obj->setSearchText(search_text);
							obj->setSearchSelection(start, start);
							setClipboard(cut_text);
							return 1;
						}
						case XPLM_VK_C:	//copy 
						{
							size_t start = obj->getSearchSelectionStart();
							size_t stop = obj->getSearchSelectionStop();
							std::string search_text = obj->getSearchText();
							std::string cut_text = search_text.substr(start, stop - start);
							setClipboard(cut_text);
							return 1;
						}
						case XPLM_VK_V:	//paste 
						{
							std::string pasted_text = getClipboard();
							size_t start = obj->getSearchSelectionStart();
							size_t stop = obj->getSearchSelectionStop();
							std::string search_text = obj->getSearchText();
							search_text.replace(search_text.begin() + start, search_text.begin() + stop, pasted_text.begin(), pasted_text.end());
							obj->setSearchText(search_text);
							obj->setSearchSelection(start + pasted_text.size(), start + pasted_text.size());
							return 1;
						}
					}
				} else {
					switch((uint8_t) keystruct->vkey) {
						case XPLM_VK_NUMPAD_ENT:
						case XPLM_VK_ENTER:
						case XPLM_VK_RETURN:
						case XPLM_VK_TAB:
						case XPLM_VK_ESCAPE:
							obj->deselectSearchField();
							break;
					}
				}
		}
		return 0;
	}


	static int editFieldCallback(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t  inParam1, intptr_t) {
		DatarefViewerWindow * obj = (DatarefViewerWindow *) XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr);
		XPKeyState_t * keystruct = (XPKeyState_t *) inParam1;
		switch(inMessage) {
			case xpMsg_DescriptorChanged:
				//obj->doSearch();
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
							} else if(std::isalpha(key) || key == ',' || (obj->select_edit_dataref->isInt() && key == '.')) {
								return 1;
							} else {
								obj->edit_modified = true;
								return 0;
							}

						case XPLM_VK_NUMPAD_ENT:
						case XPLM_VK_ENTER:
						case XPLM_VK_RETURN:
						case XPLM_VK_TAB:
						case XPLM_VK_ESCAPE:
							obj->deselectEditField();
							return 1;
					}
				}
		}
		return 0;
	}

	static int filterClickCallback(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t, intptr_t) {
		DatarefViewerWindow * obj = (DatarefViewerWindow *) XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr);
		switch(inMessage) {
			case xpMsg_ButtonStateChanged:
				obj->doSearch();
				return 1;
		}
		return 0;
	}

	static int drawListCallback(XPWidgetMessage  inMessage, XPWidgetID  inWidget, intptr_t, intptr_t) {
		DatarefViewerWindow * obj = (DatarefViewerWindow *) XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr);
		switch(inMessage) {
			case xpMsg_Draw:
				obj->draw();
				return 1;
		}
		return 0;
	}

public:
	DatarefViewerWindow(int x, int y, int x2, int y2) {
		XPLMGetFontDimensions(font, nullptr, &fontheight, nullptr);

		window = XPCreateWidget(x, y, x2, y2,
					1,										// Visible
					"Data Ref Tool",	// desc
					1,										// root
					NULL,									// no container
					xpWidgetClass_MainWindow);

		XPSetWidgetProperty(window, xpProperty_MainWindowHasCloseBoxes, 1);
		XPSetWidgetProperty(window, xpProperty_MainWindowType, xpMainWindowStyle_Translucent);
		XPAddWidgetCallback(window, viewerWindowCallback);
		XPSetWidgetProperty(window, xpProperty_Object, (intptr_t)this);

		custom_list = XPCreateCustomWidget(0, 0, 1, 1, 1,"", 0, window, drawListCallback);
		//XPAddWidgetCallback(custom_list, drawListCallback);
		XPSetWidgetProperty(custom_list, xpProperty_Object, (intptr_t)this);

		regex_toggle_button = XPCreateWidget(0, 0, 1, 1, 1,".*", 0, window, xpWidgetClass_Button);
		XPSetWidgetProperty(regex_toggle_button, xpProperty_ButtonType, xpPushButton);
		XPSetWidgetProperty(regex_toggle_button, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
		XPSetWidgetProperty(regex_toggle_button, xpProperty_ButtonState, 0);
		XPAddWidgetCallback(regex_toggle_button, filterClickCallback);
		XPSetWidgetProperty(regex_toggle_button, xpProperty_Object, (intptr_t)this);

		case_sensitive_button = XPCreateWidget(0, 0, 1, 1, 1,"Aa", 0, window, xpWidgetClass_Button);
		XPSetWidgetProperty(case_sensitive_button, xpProperty_ButtonType, xpPushButton);
		XPSetWidgetProperty(case_sensitive_button, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
		XPSetWidgetProperty(case_sensitive_button, xpProperty_ButtonState, 0);
		XPAddWidgetCallback(case_sensitive_button, filterClickCallback);
		XPSetWidgetProperty(case_sensitive_button, xpProperty_Object, (intptr_t)this);

		change_filter_button = XPCreateWidget(0, 0, 1, 1, 1,"Ch", 0, window, xpWidgetClass_Button);
		XPSetWidgetProperty(change_filter_button, xpProperty_ButtonType, xpPushButton);
		XPSetWidgetProperty(change_filter_button, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
		XPSetWidgetProperty(change_filter_button, xpProperty_ButtonState, 0);
		XPAddWidgetCallback(change_filter_button, filterClickCallback);
		XPSetWidgetProperty(change_filter_button, xpProperty_Object, (intptr_t)this);

		search_field = XPCreateWidget(0, 0, 1, 1, 1,"", 0, window, xpWidgetClass_TextField);
		XPSetWidgetProperty(search_field, xpProperty_TextFieldType, xpTextTranslucent);
		XPAddWidgetCallback(search_field, searchFieldCallback);
		XPSetWidgetProperty(search_field, xpProperty_Object, (intptr_t)this);

		edit_field = XPCreateWidget(0, 0, 1, 1, 1,"", 0, window, xpWidgetClass_TextField);
		//XPSetWidgetProperty(edit_field, xpProperty_TextFieldType, xpTextTranslucent);
		XPAddWidgetCallback(edit_field, editFieldCallback);
		XPSetWidgetProperty(edit_field, xpProperty_Object, (intptr_t)this);
		XPHideWidget(edit_field);

		scroll_bar = XPCreateWidget(0, 0, 1, 1, 1,"", 0, window, xpWidgetClass_ScrollBar);
		XPSetWidgetProperty(scroll_bar, xpProperty_ScrollBarType, xpScrollBarTypeScrollBar);	//might need changing
		XPSetWidgetProperty(scroll_bar, xpProperty_ScrollBarMin, 0);
		XPSetWidgetProperty(scroll_bar, xpProperty_ScrollBarMax, 0);

		if(last_left != -1) {
			XPSetWidgetGeometry(window, last_left, last_top, last_right, last_bottom);
			XPSetWidgetDescriptor(search_field, last_search_term.c_str());
			XPSetWidgetProperty(change_filter_button, xpProperty_ButtonState, last_changed ? 1 : 0);
			XPSetWidgetProperty(case_sensitive_button, xpProperty_ButtonState, last_case_sensitive ? 1 : 0);
			XPSetWidgetProperty(regex_toggle_button, xpProperty_ButtonState, last_regex ? 1 : 0);
			last_left = last_top = last_right = last_bottom = -1;
			last_search_term.clear();
			last_case_sensitive = last_regex = last_changed = false;
		}

		resize();

		doSearch();
		int max_scroll = std::max<int>(0, datarefs.size() - displayed_lines);
		XPSetWidgetProperty(scroll_bar, xpProperty_ScrollBarMax, max_scroll);
		XPSetWidgetProperty(scroll_bar, xpProperty_ScrollBarSliderPosition, max_scroll);
	}

	~DatarefViewerWindow() {
		XPGetWidgetGeometry(window, &last_left, &last_top, &last_right, &last_bottom);
		last_search_term = getSearchText();

		last_case_sensitive = 0 != XPGetWidgetProperty(case_sensitive_button, xpProperty_ButtonState, nullptr);
		last_regex = 0 != XPGetWidgetProperty(regex_toggle_button, xpProperty_ButtonState, nullptr);
		last_changed = 0 != XPGetWidgetProperty(change_filter_button, xpProperty_ButtonState, nullptr);

		XPHideWidget(window);
		XPLMDestroyWindow(window);
	}

	void updateScroll() {
		deselectEditField();

		//update the scrollbar
		int scroll_pos = (int)XPGetWidgetProperty(scroll_bar, xpProperty_ScrollBarSliderPosition, nullptr);
		int max_scroll_pos = std::max<int>(0, int(datarefs.size()) - displayed_lines);
		XPSetWidgetProperty(scroll_bar, xpProperty_ScrollBarMin, 0);
		XPSetWidgetProperty(scroll_bar, xpProperty_ScrollBarMax, max_scroll_pos);
		if(scroll_pos > max_scroll_pos) {
			XPSetWidgetProperty(scroll_bar, xpProperty_ScrollBarSliderPosition, max_scroll_pos);
		}
	}

	void deselectSearchField() {
		XPLoseKeyboardFocus(search_field);
	}

	void moveScroll(int amount) {
		intptr_t scroll_pos = XPGetWidgetProperty(scroll_bar, xpProperty_ScrollBarSliderPosition, nullptr) + amount;
		int max_scroll_pos = std::max<int>(0, int(datarefs.size()) - displayed_lines);
		scroll_pos = std::min<intptr_t>(max_scroll_pos, std::max<intptr_t>(0, scroll_pos));

		XPSetWidgetProperty(scroll_bar, xpProperty_ScrollBarSliderPosition, scroll_pos);
		deselectEditField();
	}

	std::string getSearchText() const {
		char searchfield_text[1024];
		XPGetWidgetDescriptor(search_field, searchfield_text, 1024);
		return std::string(searchfield_text);
	}

	intptr_t getSearchSelectionStart() const {
		return XPGetWidgetProperty(search_field, xpProperty_EditFieldSelStart, NULL);
	}

	intptr_t getSearchSelectionStop() const {
		return XPGetWidgetProperty(search_field, xpProperty_EditFieldSelEnd, NULL);
	}

	void setSearchSelection(intptr_t start, intptr_t stop) {
		XPSetWidgetProperty(search_field, xpProperty_EditFieldSelStart, start);
		XPSetWidgetProperty(search_field, xpProperty_EditFieldSelEnd, stop);
	}

	void setSearchText(const std::string & s) {
		XPSetWidgetDescriptor(search_field, s.c_str());
	}

	void deselectEditField() {
		if(edit_modified) {
			const std::string edit_txt = getEditText();

			if(select_edit_dataref->isDouble()) {
				try {
					double d = std::stold(edit_txt);
					select_edit_dataref->setDouble(d);
				} catch(std::exception &) { }
			} else if(select_edit_dataref->isFloat()) {
				try {
					float f = std::stof(edit_txt);
					select_edit_dataref->setFloat(f);
				} catch(std::exception &) { }
			} else if(select_edit_dataref->isInt()) {
				try {
					int i = std::stoi(edit_txt);
					select_edit_dataref->setInt(i);
				} catch(std::exception &) { }
			}
		}

		edit_modified = false;
		select_edit_dataref = nullptr;
		XPLoseKeyboardFocus(edit_field);
		XPHideWidget(edit_field);
	}

	std::string getEditText() const {
		char editfield_text[1024];
		XPGetWidgetDescriptor(edit_field, editfield_text, 1024);
		return std::string(editfield_text);
	}

	intptr_t getEditSelectionStart() const {
		return XPGetWidgetProperty(edit_field, xpProperty_EditFieldSelStart, NULL);
	}

	intptr_t getEditSelectionStop() const {
		return XPGetWidgetProperty(edit_field, xpProperty_EditFieldSelEnd, NULL);
	}

	void setEditSelection(intptr_t start, intptr_t stop) {
		XPSetWidgetProperty(edit_field, xpProperty_EditFieldSelStart, start);
		XPSetWidgetProperty(edit_field, xpProperty_EditFieldSelEnd, stop);
	}

	void doSearch() {
		deselectEditField();
		intptr_t property = XPGetWidgetProperty(case_sensitive_button, xpProperty_ButtonState, nullptr);
		bool case_insensitive_selected = property != 0;
		property = XPGetWidgetProperty(regex_toggle_button, xpProperty_ButtonState, nullptr);
		bool regex_selected = property != 0;
		property = XPGetWidgetProperty(change_filter_button, xpProperty_ButtonState, nullptr);
		bool changed_selected = property != 0;

		char searchfield_text[1024];
		XPGetWidgetDescriptor(search_field, searchfield_text, 1024);

		doDatarefSearch(searchfield_text, regex_selected, case_insensitive_selected, changed_selected, datarefs);

		updateScroll();

		std::string window_title = std::string("DataRef Tool (") + std::to_string(datarefs.size()) + ")"; 
		XPSetWidgetDescriptor(window, window_title.c_str());
	}

	void draw() {
		updateResults();

		int left, top, right, bottom;
		XPGetWidgetGeometry(window, &left, &top, &right, &bottom);

		top -= title_bar_height;
		left += mouse_drag_margin;

		const int scroll_pos = (int)XPGetWidgetProperty(scroll_bar, xpProperty_ScrollBarSliderPosition, nullptr);
		const int scroll_pos_max = (int)XPGetWidgetProperty(scroll_bar, xpProperty_ScrollBarMax, nullptr);

		//high scroll_pos is the top of the scroll bar, opposite how we expect
		const int lines_to_render = std::min<int>(displayed_lines, int(datarefs.size()));
		list_start_index = scroll_pos_max - scroll_pos;

		const std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

		for(int i = 0; i < lines_to_render; i++) {
			const DataRefRecord * record = datarefs[i + list_start_index];

			float timediff = 0.001f * std::chrono::duration_cast<std::chrono::milliseconds>(now - record->getLastUpdated()).count();
			float timediff_fraction = std::min<float>(1.f, timediff / 10.f);
			float colors[3] = {0.2f + timediff_fraction * 0.8f, 1.f, 1.f};
			int xstart = left;
			int ystart = top - (i + 1) * fontheight;
			std::string linetext = record->getDisplayString();
			XPLMDrawString(colors, xstart, ystart, (char *)linetext.c_str(), nullptr, font);
		}
	}

	void updateResults() {
		datarefUpdate();
	}

	void resize() {
		int left, top, right, bottom;
		XPGetWidgetGeometry(window, &left, &top, &right, &bottom);
		resize(left, top, right, bottom);
	}

	void resize(int left, int top, int right, int bottom) {
		top -= title_bar_height;
		left += mouse_drag_margin;
		right -= mouse_drag_margin;
		bottom += mouse_drag_margin;

		XPSetWidgetGeometry(regex_toggle_button, left + 0 * toggle_button_width, bottom + bottom_row_height, left + 1 * toggle_button_width, bottom);
		XPSetWidgetGeometry(case_sensitive_button, left + 1 * toggle_button_width, bottom + bottom_row_height, left + 2 * toggle_button_width, bottom);
		XPSetWidgetGeometry(change_filter_button, left + 2 * toggle_button_width, bottom + bottom_row_height, left + 3 * toggle_button_width, bottom);
		XPSetWidgetGeometry(search_field, left + 3 * toggle_button_width, bottom + bottom_row_height, right, bottom);

		XPSetWidgetGeometry(scroll_bar, right - right_col_width, top, right, bottom + bottom_row_height);
		displayed_lines = (top - bottom - bottom_row_height - mouse_drag_margin) / fontheight;

		XPSetWidgetGeometry(custom_list, left, top, right - right_col_width, bottom + bottom_row_height);
		updateScroll();
	}

	void show() {
		XPShowWidget(window);
	}
};
/////////////////
std::set<DatarefViewerWindow *> viewer_windows;

void closeViewerWindow(DatarefViewerWindow * window) {
	delete window;
	viewer_windows.erase(window);
}

void closeViewerWindows() {
	for(DatarefViewerWindow * window : viewer_windows) {
		delete window;
	}

	viewer_windows.clear();
}

void updateSearchResults() {
	for(DatarefViewerWindow * window : viewer_windows) {
		window->doSearch();
	}
}

void updateViewerResults() {
	for(DatarefViewerWindow * window : viewer_windows) {
		window->updateResults();
	}
}

void showViewerWindow() {
	XPLMDataRef window_width_ref = XPLMFindDataRef("sim/graphics/view/window_width");
	XPLMDataRef window_height_ref = XPLMFindDataRef("sim/graphics/view/window_height");

	if(nullptr == window_width_ref || nullptr == window_height_ref) {
		std::cerr << "Couldn't open datarefs for window width and height" << std::endl;
		return;
	}
	int width = XPLMGetDatai(window_width_ref);
	int height = XPLMGetDatai(window_height_ref);

	const int window_width = 500;
	const int window_height = 400;

	int x = width/2 - window_width / 2, x2 = width/2 + window_width / 2;
	int y = height/2 + window_height / 2, y2 = height/2 - window_height / 2;

	DatarefViewerWindow * viewer_window = new DatarefViewerWindow(x, y, x2, y2);
	viewer_windows.insert(viewer_window);
}
