#include "about_window.h"

#include "XPLMDisplay.h"
#include "XPCWidget.h"
#include "XPStandardWidgets.h"
#include "XPLMDataAccess.h"

#include "logging.h"

XPLMWindowID about_window = nullptr;

void closeAboutWindow() {
	XPHideWidget(about_window);
	XPLMDestroyWindow(about_window);
	about_window = nullptr;
}

int	aboutWindowCallback(XPWidgetMessage message, XPWidgetID, intptr_t, intptr_t)
{
	if (message == xpMessage_CloseButtonPushed) {
		closeAboutWindow();
		return 1;
	}

	return 0;
}

void showAboutWindow() {

	if(nullptr != about_window) {
		XPShowWidget(about_window);
	} else {
		XPLMDataRef window_width_ref = XPLMFindDataRef("sim/graphics/view/window_width");
		XPLMDataRef window_height_ref = XPLMFindDataRef("sim/graphics/view/window_height");

		if(nullptr == window_width_ref || nullptr == window_height_ref) {
			LOG("Couldn't open datarefs for window width and height");
			return;
		}
		int width = XPLMGetDatai(window_width_ref);
		int height = XPLMGetDatai(window_height_ref);

		const int window_width = 250;
		const int window_height = 150;

		int x = width/2 - window_width / 2, x2 = width/2 + window_width / 2;
		int y = height/2 + window_height / 2, y2 = height/2 - window_height / 2;

		about_window = XPCreateWidget(x, y, x2, y2,
					1,										// Visible
					"About Data Ref Tool",	// desc
					1,										// root
					NULL,									// no container
					xpWidgetClass_MainWindow);

		XPSetWidgetProperty(about_window, xpProperty_MainWindowHasCloseBoxes, 1);
		XPSetWidgetProperty(about_window, xpProperty_MainWindowType, xpMainWindowStyle_Translucent);
		XPAddWidgetCallback(about_window, aboutWindowCallback);

		const int margin = 20;
		const int row_sep = 16;

		int row_top = y - 20;	//minus 20 for the header bar
		int left_bound = x + margin;
		int right_bound = x2 - margin;

		std::string top_line("Data Ref Tool");
		std::string by_line("by Lee C. Baker");
		std::string www_line("https://github.com/leecbaker/datareftool/");
		std::string compile_date("Compiled " __DATE__ " at " __TIME__);
		XPWidgetID widget = XPCreateWidget(left_bound, row_top, right_bound, row_top - 20, 1,top_line.c_str(), 0, about_window, xpWidgetClass_Caption);
		XPSetWidgetProperty(widget, xpProperty_CaptionLit, 1);
		row_top -= row_sep;
		widget = XPCreateWidget(left_bound, row_top, right_bound, row_top - 20, 1,by_line.c_str(), 0, about_window, xpWidgetClass_Caption);
		XPSetWidgetProperty(widget, xpProperty_CaptionLit, 1);
		row_top -= row_sep;
		widget = XPCreateWidget(left_bound, row_top, right_bound, row_top - 20, 1,www_line.c_str(), 0, about_window, xpWidgetClass_Caption);
		XPSetWidgetProperty(widget, xpProperty_CaptionLit, 1);
		row_top -= row_sep;
		widget = XPCreateWidget(left_bound, row_top, right_bound, row_top - 20, 1,compile_date.c_str(), 0, about_window, xpWidgetClass_Caption);
		XPSetWidgetProperty(widget, xpProperty_CaptionLit, 1);
		row_top -= row_sep;
	}
}
