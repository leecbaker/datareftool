#pragma once

#include "ref.h"
#include "XPLMUtilities.h"

class RefRecords;

class CommandRefRecord : public RefRecord {
    RefRecords & all_records;
    XPLMCommandRef ref = nullptr;
    static int cr_callback(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void * inRefcon);
    bool activated = false;
public:
    CommandRefRecord(const std::string & name, XPLMCommandRef ref, ref_src_t source, RefRecords & all_records) 
    : RefRecord(name, source), all_records(all_records), ref(ref) {
        XPLMRegisterCommandHandler(ref, &cr_callback, 0, this);
        XPLMRegisterCommandHandler(ref, &cr_callback, 1, this);
    }

    virtual bool isCommand() const override final { return true; }
    virtual bool isDataref() const override final { return false; }

    virtual ~CommandRefRecord() {
        XPLMUnregisterCommandHandler(ref, &cr_callback, 0, this);
        XPLMUnregisterCommandHandler(ref, &cr_callback, 1, this);
    }

    virtual std::string getDisplayString(size_t /*display_length*/) const override { return name; }

    void commandOnce() const { XPLMCommandOnce(ref); }
    void commandBegin() const { XPLMCommandBegin(ref); }
    void commandEnd() const { XPLMCommandEnd(ref); }
    void touch() { last_updated_big = last_updated = std::chrono::system_clock::now(); }
    bool isActivated() const { return activated; }
};
