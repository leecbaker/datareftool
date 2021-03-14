#include "logging.h"

XPLogAggregator xplog_aggregator;
XPLogAggregator xplog_debug_aggregator;

thread_local XPLogger xplog(xplog_aggregator);
thread_local XPLogger xplog_debug(xplog_debug_aggregator);


XPLogAggregator::XPLogAggregator() {
    XPLMCreateFlightLoop_t callback_info{
        .structSize = sizeof(XPLMCreateFlightLoop_t),
        .callbackFunc = &XPLogAggregator::logging_callback,
        .phase = xplm_FlightLoop_Phase_BeforeFlightModel,
        .refcon = this,
    };

    callback_id = XPLMCreateFlightLoop(&callback_info);

    XPLMScheduleFlightLoop(callback_id, -1, 0);
}

XPLogAggregator::~XPLogAggregator() {
    XPLMDestroyFlightLoop(callback_id);

}

float XPLogAggregator::logging_callback(float /* inElapsedSinceLastCall */, float /* inElapsedTimeSinceLastFlightLoop */, int /* inCounter */, void * refcon) {
    XPLogAggregator * xpl = static_cast<XPLogAggregator *>(refcon);

    while(true) {
        std::optional<std::string> opt_message = xpl->message_queue.get();

        if(opt_message) {
            XPLMDebugString(xpl->prefix_.c_str());
            XPLMDebugString(opt_message->c_str());
        } else {
            return -1;
        }
    }
}