#include "commandref.h"

#include "plugin.h"

int CommandRefRecord::cr_callback(XPLMCommandRef /*inCommand*/, XPLMCommandPhase /*inPhase*/, void * inRefcon) {
    CommandRefRecord * record = (CommandRefRecord *) inRefcon;

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    record->last_updated = now;
    record->last_updated_big = now;

    requestSearchUpdate();
    return 1;
}