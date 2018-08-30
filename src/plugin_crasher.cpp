#include <cstdint>
#include <cstring>

#include "XPLMDefs.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"

#include <signal.h>

int segfault_callback(void*) {
	raise(SIGSEGV);
    return 0;
}

int fpe_callback(void*) {
	raise(SIGFPE);
    return 0;
}

PLUGIN_API int XPluginStart(char * outName, char * outSig, char * outDesc) { 
	strcpy(outName, "DataRef Tool Crasher");
	strcpy(outSig, "com.leecbaker.datareftool_crasher");
	strcpy(outDesc, "Test for DataRefTool");
	XPLMDataRef dr1 = XPLMRegisterDataAccessor("datareftool/segfault", xplmType_Int, 0, segfault_callback, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	XPLMDataRef dr2 = XPLMRegisterDataAccessor("datareftool/float_point_exception", xplmType_Int, 0, fpe_callback, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	if(nullptr == dr1 || nullptr == dr2) {
		return 0;
	}
	return 1;
}

PLUGIN_API void	XPluginStop(void) { }
PLUGIN_API void XPluginDisable(void) { }
PLUGIN_API int XPluginEnable(void) { return 1; }
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID, intptr_t, void *) { }
