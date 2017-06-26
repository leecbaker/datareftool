#pragma once

#include "ref.h"
#include "XPLMUtilities.h"

class CommandRefRecord : public RefRecord {
    XPLMCommandRef ref = nullptr;
    static int cr_callback(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void * inRefcon);
public:
    CommandRefRecord(const std::string & name, XPLMCommandRef ref, ref_src_t source) 
    : RefRecord(name, source), ref(ref) {
        XPLMRegisterCommandHandler(ref, &cr_callback, 0, this);
        XPLMRegisterCommandHandler(ref, &cr_callback, 1, this);
    }

    virtual ~CommandRefRecord() {
        XPLMUnregisterCommandHandler(ref, &cr_callback, 0, this);
        XPLMUnregisterCommandHandler(ref, &cr_callback, 1, this);
    }

    virtual std::string getDisplayString(size_t /*display_length*/) const { return name; }

    void commandOnce() const { XPLMCommandOnce(ref); }
    void commandBegin() const { XPLMCommandBegin(ref); }
    void commandEnd() const { XPLMCommandEnd(ref); }
};
