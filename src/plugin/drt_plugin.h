#pragma once

#include "plugin.h"

#include "drt_datarefs.h"

#include "threaded_scanner.h"
#include "search/allrefs.h"

#include "next_flight_loop_callback.h"

class DRTPlugin: public Plugin {
    NextFlightLoopCallback load_dr_flcb;
    NextFlightLoopCallback update_dr_flcb;
    NextFlightLoopCallback plugin_changed_flcb;
    NextFlightLoopCallback load_acf_dr_flcb;

    std::weak_ptr<AboutWindow> about_window_ref;
    std::vector<std::weak_ptr<SearchWindow>> search_window_refs;
    PluginMenu menu;
    lb::filesystem::path prefs_path;

    ThreadedScanner scanner;
    RefRecords refs;
    DRTDatarefs datarefs;

    // Callbacks
    void load_dr_callback();
    void update_dr_callback();
    void load_acf_dr_callback();
    void plugin_changed_check_callback();
public:
    DRTPlugin();
    ~DRTPlugin();

    void loadPrefs();
    void savePrefs();
    void setDebugMode(bool debug_mode);

    void openAboutWindow();
    std::shared_ptr<SearchWindow> openSearchWindow();
    std::shared_ptr<SearchWindow> openSearchWindow(const nlohmann::json & window_params);

    nlohmann::json dumpSearchWindow(const std::shared_ptr<SearchWindow> & search_window) const;

    void rescan();

    void reloadAircraft();
    void reloadPlugins();
    void reloadScenery();

    void rescanDatarefs();
    void aircraftIsBeingLoaded();

    void toggleImpersonateDRE();
    void toggleAutoReloadPlugins();

    void handleMessage(intptr_t inMessage, void * inParam);

    RefRecords & getRefs() { return refs; }

    void saveAllRefs();
};

extern std::unique_ptr<DRTPlugin> plugin;
