#include <cstdint>
#include <cstring>

#include "XPLMDefs.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"

int dummy_dr_variable = -1337;
const intptr_t MSG_ADD_DATAREF = 0x01000000;
float register_dr_callback(float elapsedMe, float elapsedSim, int counter, void * refcon)
{
	XPLMPluginID plugin_id = XPLMFindPluginBySignature("com.leecbaker.datareftool");
	if(XPLM_NO_PLUGIN_ID != plugin_id) {
		XPLMSendMessageToPlugin(plugin_id, MSG_ADD_DATAREF, (void*)"datareftool/good_int");
		XPLMSendMessageToPlugin(plugin_id, MSG_ADD_DATAREF, (void*)"datareftool/bad_int");   
	}

	return 0; 
}

int dummy_dr_callback(void* inRefcon) {
	return 1337;
}

PLUGIN_API int XPluginStart(char * outName, char * outSig, char * outDesc) { 
	strcpy(outName, "DataRef Tool Custom Dataref Example");
	strcpy(outSig, "com.leecbaker.datareftool_custom_dataref_example");
	strcpy(outDesc, "Test for DataRefTool");
	XPLMRegisterFlightLoopCallback(register_dr_callback, 1, nullptr);
	XPLMDataRef dr = XPLMRegisterDataAccessor("datareftool/good_int", xplmType_Int, 0, dummy_dr_callback, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	return 1;
}

PLUGIN_API void	XPluginStop(void) { }
PLUGIN_API void XPluginDisable(void) { }
PLUGIN_API int XPluginEnable(void) { return 1; }
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, long inMessage, void * inParam) { }