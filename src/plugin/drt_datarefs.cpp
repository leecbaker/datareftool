#include "drt_datarefs.h"

#include "drt_plugin.h"

#include "prefs.h"

DRTDatarefs::DRTDatarefs()
: debug_mode("leecbaker/datareftool/debug",
    []() -> bool { return getDebugMode(); },
    [](bool b){
        plugin->setDebugMode(b);
    }
)
 {

}