#include <string.h>

#include <XPLMPlugin.h>
#include <XPLMProcessing.h>

#include "drt_client.h"
#include "example_queries.h"

XPLMFlightLoopID example_flight_loop = NULL;

float example_callback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void * inRefcon) {
    run_example_queries();

    return 0; //don't run queries again.
}

PLUGIN_API int XPluginStart(char * outName, char * outSig, char * outDesc) {
    strcpy(outName, "DRT Client");
    strcpy(outSig, "com.leecbaker.drt_client");
    strcpy(outDesc, "Example code for how to search datarefs from another plugin");

    XPLMCreateFlightLoop_t flight_loop = {
        sizeof(XPLMCreateFlightLoop_t),
        xplm_FlightLoop_Phase_AfterFlightModel,
        example_callback,
        NULL
    };
    example_flight_loop = XPLMCreateFlightLoop(&flight_loop);

    return 1;
}

PLUGIN_API void XPluginStop(void) {
    XPLMDestroyFlightLoop(example_flight_loop);
}

PLUGIN_API void XPluginDisable(void) {
}

PLUGIN_API int XPluginEnable(void) {
    // Run the examples 5 seconds after the we start.
    XPLMScheduleFlightLoop(example_flight_loop, 5, 1);
    return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID plugin_id, int inMessage, void * inParam) {
}
