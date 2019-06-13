#include "commandref.h"

#include "plugin.h"

int CommandRefRecord::cr_callback(XPLMCommandRef /*inCommand*/, XPLMCommandPhase phase, void * inRefcon) {
    CommandRefRecord * record = reinterpret_cast<CommandRefRecord *>(inRefcon);

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    record->last_updated = now;
    record->last_updated_big = now;

    switch(phase) {
        case xplm_CommandBegin: record->activated = true; break;
        case xplm_CommandContinue: record->activated = true; break;
        case xplm_CommandEnd: record->activated = false; break;
    }

    addUpdatedCommandThisFrame(record);
    return 1;
}