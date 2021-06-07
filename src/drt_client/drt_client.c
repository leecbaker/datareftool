#include "drt_client.h"

#include <XPLMPlugin.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This message ID is sent to DRT to perform a search.
 * This value is simply "DRT SRCH" in ASCII.
 */
#define DRT_API_SEARCH_MESSAGE 0x44525453


const char * const dre_signature = "xplanesdk.examples.DataRefEditor";
const char * const drt_signature = "com.leecbaker.datareftool";

int DRTPerformSearch(DRTSearchParameters * search_parameters) {
    search_parameters->result_count = 0;

    /* Find the DRT plugin's ID */
    XPLMPluginID drt_plugin_id = XPLMFindPluginBySignature(drt_signature);

    if(XPLM_NO_PLUGIN_ID == drt_plugin_id) {
        /* DRT plugin not found- maybe it's in DRE emulation mode? */
        drt_plugin_id = XPLMFindPluginBySignature(dre_signature);
    }

    /* If no plugin was found, we're done */
    if(XPLM_NO_PLUGIN_ID == drt_plugin_id) {
        return DRT_SEARCH_RESULT_PLUGIN_NOT_FOUND;
    }

    /* 
     * Before searching, we seed the result count (which is used to return errors)
     * with DRT_SEARCH_RESULT_PLUGIN_NOT_FOUND. This way, if the message is not handled,
     * we will get a response saying that the plugin wasn't found. If the message is handled,
     * we'll get the result count back.
    */
   search_parameters->result_count = DRT_SEARCH_RESULT_PLUGIN_NOT_FOUND;

    /* Perform the search */
    XPLMSendMessageToPlugin(drt_plugin_id, DRT_API_SEARCH_MESSAGE, search_parameters);

    /*
     * DRT may send back an error code in the result count. This is a negative
     * number.
     */
    if(search_parameters->result_count < 0) {
        int error_code = search_parameters->result_count;
        search_parameters->result_count = 0;
        return error_code;
    }

    return DRT_SEARCH_RESULT_SUCCESS;
}

#ifdef __cplusplus
}
#endif
