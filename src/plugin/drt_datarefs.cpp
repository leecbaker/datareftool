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
, impersonate_dre("leecbaker/datareftool/prefs/impersonate_dre",
    []() -> bool { return getImpersonateDRE(); },
    [](bool b){
        setImpersonateDRE(b);
    }
)
, auto_reload_plugins("leecbaker/datareftool/prefs/auto_reload_plugins",
    []() -> bool { return getAutoReloadPlugins(); },
    [](bool b){
        setAutoReloadPlugins(b);
    }
)
, build_date("leecbaker/datareftool/build_date", []() -> std::string {
    return std::string(__DATE__ " " __TIME__);
})
, toggle_debug("leecbaker/datareftool/debug_toggle", "Toggle debug mode", []() {
    plugin->setDebugMode(!getDebugMode());
    return true;
})
, toggle_impersonate_dre("leecbaker/datareftool/prefs/impersonate_dre_toggle", "Toggle impersonating DataRefEditor", []() {
    plugin->toggleImpersonateDRE();
    return true;
})
, toggle_auto_reload_plugins("leecbaker/datareftool/prefs/auto_reload_plugins_toggle", "Toggle auto-reload plugins on change", []() {
    plugin->toggleAutoReloadPlugins();
    return true;
})
, prefs_load("leecbaker/datareftool/prefs/load", "Reload preferences", []() {
    plugin->loadPrefs();
    return true;
})
, prefs_save("leecbaker/datareftool/prefs/save", "Save preferences", []() {
    plugin->savePrefs();
    return true;
})
, reload_plugins("leecbaker/datareftool/reload_plugins", "Reload plugins", []() {
    plugin->reloadPlugins();
    return true;
})
, reload_scenery("leecbaker/datareftool/reload_scenery", "Reload scenery", []() {
    plugin->reloadScenery();
    return true;
})
, reload_aircraft("leecbaker/datareftool/reload_aircraft", "Reload the current aircraft", []() {
    plugin->reloadAircraft();
    return true;
})
, new_window("leecbaker/datareftool/new_search_window", "Open a new search window", []() {
    plugin->openSearchWindow({});
    return true;
})
 {

}