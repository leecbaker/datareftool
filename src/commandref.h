#pragma once

#include "ref.h"
#include "XPLMUtilities.h"

class CommandRefRecord : public RefRecord {
    XPLMCommandRef ref = nullptr;
public:
    CommandRefRecord(const std::string & name, XPLMCommandRef ref, ref_src_t source) 
    : RefRecord(name, source), ref(ref) {}

    virtual std::string getDisplayString(size_t display_length) const { return name; }

    void commandOnce() const { XPLMCommandOnce(ref); }
    void commandBegin() const { XPLMCommandBegin(ref); }
    void commandEnd() const { XPLMCommandEnd(ref); }
};
