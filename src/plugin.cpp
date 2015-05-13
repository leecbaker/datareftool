#include <cstring>

#include "about_window.h"
#include "viewer_window.h"
#include "find_datarefs_in_files.h"

#include "datarefs.h"

#include "XPWidgets.h"
#include "XPLMMenus.h"
#include "XPLMDisplay.h"
#include "XPLMUtilities.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMPlanes.h"

void loadAircraftDatarefs() {
	//get path
	char filename[256] = {0};
	char path[512] = {0};
	XPLMGetNthAircraftModel(0, filename, path);
	std::vector<std::string> aircraft_datarefs = getDatarefsFromAircraft(path);

	int loaded_ok = 0;
	for(const std::string & dataref : aircraft_datarefs) {
		if(addUserDataref(dataref) ) {
			loaded_ok++;
		}
	}
	const std::string message = std::string("DRT: Found ") + std::to_string(aircraft_datarefs.size()) + std::string(" possible datarefs from aircraft files; " + std::to_string(loaded_ok) + " loaded OK.\n");
	XPLMDebugString(message.c_str());
}

float load_dr_callback(float, float, int, void *) {
	if(false == loadDatarefsFile()) {
		XPLMDebugString("Couldn't load datarefs from file.");
		return 0;
	}

	loadAircraftDatarefs();

	//load plugins
	int num_plugins = XPLMCountPlugins();
	XPLMPluginID my_id = XPLMGetMyID();

	std::vector<std::string> all_plugin_datarefs;

	for(int i = 0; i < num_plugins; i++) {
		XPLMPluginID id = XPLMGetNthPlugin(i);
		if(id == my_id) {
			continue;
		}

		char filename[256] = {0};
		char path[512] = {0};
		XPLMGetPluginInfo(id, filename, path, nullptr, nullptr);

		std::vector<std::string> this_plugin_datarefs = getDatarefsFromFile(path);
		all_plugin_datarefs.insert(all_plugin_datarefs.end(), this_plugin_datarefs.begin(), this_plugin_datarefs.end());
	}

	removeVectorUniques(all_plugin_datarefs);

	int loaded_ok = 0;
	for(const std::string & dataref : all_plugin_datarefs) {
		if(addUserDataref(dataref) ) {
			loaded_ok++;
		}
	}
	const std::string message = std::string("DRT: Found ") + std::to_string(all_plugin_datarefs.size()) + std::string(" possible datarefs from plugin files; " + std::to_string(loaded_ok) + " loaded OK.\n");
	XPLMDebugString(message.c_str());

	updateViewerResults();

	return 0; 
}

void plugin_menu_handler(void *, void * inItemRef)
{
	switch ( intptr_t(inItemRef) )
	{
		case 0: showViewerWindow(); break;	
		//case 1: showCommandWindow(); break;	
		case 2: showAboutWindow(); break;
	}
}	

PLUGIN_API int XPluginStart(char * outName, char * outSig, char * outDesc) {
	strcpy(outName, "DataRef Tool");
	strcpy(outSig, "com.leecbaker.datareftool");
	strcpy(outDesc, "View and edit X-Plane Datarefs");
	XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);

	XPLMRegisterFlightLoopCallback(load_dr_callback, -1, nullptr);
	
	int plugin_submenu = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "DataRefTool", nullptr, 1);
	XPLMMenuID plugin_menu = XPLMCreateMenu("DataRefTool", XPLMFindPluginsMenu(), plugin_submenu, plugin_menu_handler, nullptr);

	XPLMAppendMenuItem(plugin_menu, "View Datarefs", (void *)0, 1);
	XPLMAppendMenuItem(plugin_menu, "View Commands", (void *)1, 1);
	XPLMAppendMenuItem(plugin_menu, "About DataRefTool", (void *)2, 1);

	XPLMEnableMenuItem(plugin_menu, 0, 1);
	XPLMEnableMenuItem(plugin_menu, 1, 0);
	XPLMEnableMenuItem(plugin_menu, 2, 1);

	return 1;
}

PLUGIN_API void	XPluginStop(void) {
	//closeCommandWindows();
	closeAboutWindow();
	closeViewerWindows();
	cleanupDatarefs();
	XPLMUnregisterFlightLoopCallback(load_dr_callback, nullptr);
}

PLUGIN_API void XPluginDisable(void) {
}

PLUGIN_API int XPluginEnable(void) {
	return 1;
}

const intptr_t MSG_ADD_DATAREF = 0x01000000;
const intptr_t MSG_ADD_COMMANDREF = 0x01000099;

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID, intptr_t inMessage, void * inParam) {
	switch(inMessage) {
		// Add custom datarefs in the style of DRE:
		// http://www.xsquawkbox.net/xpsdk/mediawiki/Register_Custom_DataRef_in_DRE
		case MSG_ADD_DATAREF: {
			char * dataref_name = (char *) inParam;
			bool added_ok = addUserDataref(dataref_name);
			if(added_ok) {
				updateViewerResults();
			} else {
				const std::string message = std::string("Couldn't load dataref from message: ") + dataref_name + std::string("\n");
				XPLMDebugString(message.c_str());
			}
			break;
		}
		case MSG_ADD_COMMANDREF: {
			char * commandref_name = (char *) inParam;
			bool added_ok = true;//addUserCommandref(commandref_name);
			if(added_ok) {
				//updateCommandWindows();
			} else {
				const std::string message = std::string("Couldn't load commandref from message: ") + commandref_name + std::string("\n");
				XPLMDebugString(message.c_str());
			}
			break;
		}

		case XPLM_MSG_PLANE_LOADED: {
			int64_t plane_num = int64_t(inParam);
			const std::string message = std::string("DRT: Plane loaded #: ") + std::to_string(plane_num) + std::string("\n");
			XPLMDebugString(message.c_str());
			if(0 == plane_num) {	//user's plane
				loadAircraftDatarefs();
			}
			break;
		}
	}
}
