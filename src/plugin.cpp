#include <cstring>

#include "about_window.h"
#include "viewer_window.h"

#include "datarefs.h"

#include "XPWidgets.h"
#include "XPLMMenus.h"
#include "XPLMDisplay.h"
#include "XPLMUtilities.h"
#include "XPLMPlugin.h"


void plugin_menu_handler(void * inMenuRef, void * inItemRef)
{
	switch ( intptr_t(inItemRef) )
	{
		case 0: showViewerWindow(); break;	
		case 1: showAboutWindow(); break;
	}
}	

PLUGIN_API int XPluginStart(char * outName, char * outSig, char * outDesc) {
	strcpy(outName, "DataRef Tool");
	strcpy(outSig, "com.leecbaker.datareftool");
	strcpy(outDesc, "View and edit X-Plane Datarefs");
	XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);

	if(false == loadDatarefs()) {
		XPLMDebugString("Couldn't load datarefs from file. Not loading plugin.");
		return 0;
	}
	
	int plugin_submenu = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "DataRefTool", nullptr, 1);
	XPLMMenuID plugin_menu = XPLMCreateMenu("DataRefTool", XPLMFindPluginsMenu(), plugin_submenu, plugin_menu_handler, nullptr);

	XPLMAppendMenuItem(plugin_menu, "View Datarefs", (void *)0, 1);
	XPLMAppendMenuItem(plugin_menu, "About DataRefTool", (void *)1, 1);

	XPLMEnableMenuItem(plugin_menu, 0, 1);
	XPLMEnableMenuItem(plugin_menu, 1, 1);

	return 1;
}

PLUGIN_API void	XPluginStop(void) {
	closeAboutWindow();
	closeViewerWindows();
	cleanupDatarefs();
}

PLUGIN_API void XPluginDisable(void) {
}

PLUGIN_API int XPluginEnable(void) {
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, intptr_t inMessage, void * inParam) {
}
