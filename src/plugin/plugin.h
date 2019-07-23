#pragma once

#include <cstdint>
#include <functional>
#include <vector>

#include <json.hpp>

#include <boost/optional.hpp>

#include "refscanner.h"

#include "XPLMProcessing.h"

class ViewerWindow;

// The callback function returns a bool- true if the callback should repeat, or false if it should only happen once
class NextFlightLoopCallback {
    std::function<void()> callback_function;
    XPLMFlightLoopID flight_loop = nullptr;
    float reschedule_time = 0;

    static float callback(float /* elapsed_since_last_call */, float /* elapsed_since_last_flight_loop */, int /* counter */, void * refcon) {
        NextFlightLoopCallback * pthis = static_cast<NextFlightLoopCallback *>(refcon);
        pthis->callback_function();
        return pthis->reschedule_time;
    }

public:
    NextFlightLoopCallback(std::function<void()> f) : callback_function(f) {
        XPLMCreateFlightLoop_t flight_loop_details;
        flight_loop_details.structSize = sizeof(flight_loop_details);
        flight_loop_details.phase = xplm_FlightLoop_Phase_BeforeFlightModel;
        flight_loop_details.callbackFunc = &NextFlightLoopCallback::callback;
        flight_loop_details.refcon = static_cast<void *>(this);

        flight_loop = XPLMCreateFlightLoop(&flight_loop_details);
    }

    ~NextFlightLoopCallback() {
        XPLMDestroyFlightLoop(flight_loop);
    }

    void scheduleNextFlightLoop() {
        reschedule_time = 0;
        XPLMScheduleFlightLoop(flight_loop, -1, 1);
    }

    void scheduleEveryFlightLoop() {
        reschedule_time = -1;
        XPLMScheduleFlightLoop(flight_loop, -1, 1);
    }

    void schedule(float this_call, float next_call) {
        XPLMScheduleFlightLoop(flight_loop, this_call, 1);
        reschedule_time = next_call;
    }
};

// Plugin state
class PluginData {
public:
    static void plugin_menu_handler(void * refcon, void * inItemRef);
protected:
    NextFlightLoopCallback load_dr_flcb;
    NextFlightLoopCallback update_dr_flcb;
    NextFlightLoopCallback plugin_changed_flcb;
    NextFlightLoopCallback load_acf_dr_flcb;

    std::vector<std::unique_ptr<ViewerWindow>> viewer_windows;
    boost::filesystem::path prefs_path;
public:
    RefScanner refs; //TODO public member var

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

    const boost::filesystem::path getPrefsPath() const { return prefs_path; }
public:
    void handleMessage(intptr_t inMessage, void * inParam);
protected:
    void handleMenu(void * item_ref);
};

extern boost::optional<PluginData> plugin_data;
