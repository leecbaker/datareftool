#pragma once

#include "flight_loop.h"

#include <functional>

#include "XPLMProcessing.h"

template <class T>
class FlightLoopCallback {
    using this_type = FlightLoopCallback<T>;
    XPLMFlightLoopID callback_id = nullptr;
    using callback_type = std::function<float(float, float, int, T *)>;
    callback_type callback;
    T * client_refcon;
    static float callback_handler(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void * refcon) {
        this_type * pthis = static_cast<this_type *>(refcon);
        return pthis->callback(inElapsedSinceLastCall, inElapsedTimeSinceLastFlightLoop, inCounter, pthis->client_refcon);
    }
public:
    FlightLoopCallback(std::function<float(float, float, int, T *)> callback, T * refcon, XPLMFlightLoopPhaseType phase = xplm_FlightLoop_Phase_AfterFlightModel) : callback(callback), client_refcon(refcon) {
        XPLMCreateFlightLoop_t callback_info;
        callback_info.structSize = sizeof(XPLMCreateFlightLoop_t);
        callback_info.callbackFunc = &callback_handler;
        callback_info.phase = phase;
        callback_info.refcon = this;

        callback_id = XPLMCreateFlightLoop(&callback_info);
    }

    FlightLoopCallback(std::function<float()> callback_simple, XPLMFlightLoopPhaseType phase = xplm_FlightLoop_Phase_AfterFlightModel) 
    : FlightLoopCallback([callback_simple](float, float, int, T *){return callback_simple();}, nullptr, phase) {}

    ~FlightLoopCallback() {
        XPLMDestroyFlightLoop(callback_id);
    }
    void schedule(float time) {
        XPLMScheduleFlightLoop(callback_id, time, 0);
    }
};