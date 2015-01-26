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





void XPluginReceiveMessage__XPlaneMessage(XPLMPluginID inFromWho, intptr_t inMessage, void * inParam){

	switch( inMessage ){
		case XPLM_MSG_PLANE_CRASHED:
			//This message is sent to your plugin whenever the user's plane crashes.
			break;

		case XPLM_MSG_PLANE_LOADED:
			//This message is sent to your plugin whenever a new plane is loaded. The parameter is the number of the plane being loaded; 0 indicates the user's plane.
			if( inParam == 0 ){ //player 1
			}
			break;

		case XPLM_MSG_AIRPORT_LOADED:
			//User has been placed at a new airport.
			break;

		case XPLM_MSG_SCENERY_LOADED:
			//This message is sent whenever new scenery is loaded. Use datarefs to determine the new scenery files that were loaded.
			break;

		case XPLM_MSG_AIRPLANE_COUNT_CHANGED:
			//This message is sent whenever the user adjusts the number of X-Plane aircraft models. You must use XPLMCountPlanes to find out how many planes are now available. This message will only be sent in XP7 and higher because in XP6 the number of aircraft is not user-adjustable.
			break;


		//SDK 2.00+
		case XPLM_MSG_PLANE_UNLOADED:
			//This message is sent to your plugin whenever a plane is unloaded. The parameter is the number of the plane being unloaded; 0 indicates the user's plane.
			break;

		//SDK 2.1+
		#if 0
		case XPLM_MSG_WILL_WRITE_PREFS:
		case XPLM_MSG_LIVERY_LOADED:
		#endif
		
		default:
			//Unkown message from X-Plane, do nothing.
			break;
	}//end switch(message type)

} //XPluginReceiveMessage__XPlaneMessage(...)




void XPluginReceiveMessage__RegisterDataref(XPLMPluginID inFromWho, intptr_t inMessage, void * inParam){

	char caDbg[2048];

	// Adding support for DRE style custom-dataref registration.
	// http://www.xsquawkbox.net/xpsdk/mediawiki/Register_Custom_DataRef_in_DRE

	// Decode inParam as C-String.
	
	//do a length sanity check.
	long param_string_len = strlen( (char*)inParam );
	
	if( param_string_len < 1024 ){
	
		//convert the inParam value to a std::string that gives us the name of the new dataref.
		std::string custom_dataref_name = std::string( (char*)inParam );
		
		sprintf( caDbg, "DRT: Custom Dataref: (%s)\n", custom_dataref_name.c_str() );
		XPLMDebugString( caDbg );
		
	
	}else{
		XPLMDebugString("DRT: Custom Dataref rejected. Name is extremely long.\n");
	}


} //XPluginReceiveMessage__RegisterDataref(...)




// Some inMessage values that we want to respond to.
#define DRE_MSG_ADD_DATAREF 0x01000000           //  Add dataref to DRE message


PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, intptr_t inMessage, void * inParam) {

	char caDbg[2048];
	

	if( XPLM_PLUGIN_XPLANE == inFromWho ){
	
		//Filter for X-Plane as sender. Some messages are best ignored if from random XPL client.
		XPluginReceiveMessage__XPlaneMessage( inFromWho, inMessage, inParam );
		
	}else{
		switch( inMessage ){
			case DRE_MSG_ADD_DATAREF:
				XPluginReceiveMessage__RegisterDataref( inFromWho, inMessage, inParam );
				break;
	
			default:
				sprintf( caDbg, "DRT: Unknown XPL message. (From:%i) (Type:%i)\n", 1,2 );
				XPLMDebugString( caDbg );
	
		} //switch( inMessage )
	}

} //XPluginReceiveMessage(...)
