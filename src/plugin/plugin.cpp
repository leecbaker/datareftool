#include "plugin.h"

#include <ostream>

#include <string.h> //memcpy

#include "drt_plugin.h"

#include "prefs.h"

#include "logging.h"

#include "XPLMDefs.h"
#include "XPLMPlugin.h"


const char * dre_signature = "xplanesdk.examples.DataRefEditor";
const char * dre_name = "DataRefEditor";
const char * dre_description = "A plugin that shows all data refs!.";


PLUGIN_API int XPluginStart(char * outName, char * outSig, char * outDesc) {
    // You probably want this on
    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);

    // Plugin details
    strcpy(outName, "DataRefTool");
    strcpy(outSig, "com.leecbaker.datareftool");
    strcpy(outDesc, "More information https://github.com/leecbaker/datareftool");

    xplog_aggregator.setPrefix("DataRefTool: ");
    xplog_debug_aggregator.setPrefix("DataRefToolDebug: ");

    xplog << "DataRefTool by Lee C. Baker\n";
    xplog << "www.datareftool.com\n";
    xplog << "Compiled " __TIMESTAMP__ "\n";

    plugin = std::make_unique<DRTPlugin>();

    // let's try to find DRE before we register the plugin. If it's already here, we shouldn't register with the same signature!
    bool found_dre_early = XPLM_NO_PLUGIN_ID != XPLMFindPluginBySignature(dre_signature);
    if(found_dre_early && getImpersonateDRE()) {
        xplog << "Impersonating DataRefEditor failed, because DataRefEditor is currently running.\n";
    }
    if(false == found_dre_early && getImpersonateDRE()) {
        strcpy(outName, dre_name);
        strcpy(outSig, dre_signature);
        strcpy(outDesc, dre_description);
        xplog << "Impersonating DataRefEditor\n";
    } else {
        strcpy(outName, "DataRefTool");
        strcpy(outSig, "com.leecbaker.datareftool");
        strcpy(outDesc, "View and edit X-Plane Datarefs");
    }

    return 1;
}

PLUGIN_API void XPluginStop(void) {
    plugin.reset();
}

PLUGIN_API void XPluginDisable(void) {
}

PLUGIN_API int XPluginEnable(void) {
    return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID, int inMessage, void * inParam) {
    plugin->handleMessage(inMessage, inParam);
}
