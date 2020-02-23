#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

#include <json.hpp>

#include "search/allrefs.h"

#include "threaded_scanner.h"

#include "menu.h"

#include "flight_loop_callback.h"

class ViewerWindow;

// Plugin state
class PluginData {
protected:
    NextFlightLoopCallback load_dr_flcb;
    NextFlightLoopCallback update_dr_flcb;
    NextFlightLoopCallback plugin_changed_flcb;
    NextFlightLoopCallback load_acf_dr_flcb;

    std::vector<std::unique_ptr<ViewerWindow>> viewer_windows;
    boost::filesystem::path prefs_path;

    RefRecords refs;
    ThreadedScanner scanner;
    PluginMenu menu;
public:
    PluginData();
    ~PluginData();

    void aircraftIsBeingLoaded();
    void rescanDatarefs();

    // Callbacks
    void load_dr_callback();
    void update_dr_callback();
    void load_acf_dr_callback();
    void plugin_changed_check_callback();

    void closeViewerWindow(const ViewerWindow * window);

    void showViewerWindow(bool show_dr, bool show_cr);
    void showViewerWindow(const nlohmann::json & window_details = {});
    
    nlohmann::json getViewerWindowsDetails();

    void updateMenu() { menu.update(); }

    const boost::filesystem::path getPrefsPath() const { return prefs_path; }
public:
    void handleMessage(intptr_t inMessage, void * inParam);
};

extern std::optional<PluginData> plugin_data;
void reloadAircraft();
