#include "logging.h"
#include "prefs.h"

#include "XPLMUtilities.h"

void LOG(const std::string & s) {
    if(false == getLoggingEnabled()) {
        return;
    }

    std::string s_out;
    s_out.reserve(6 + s.size());
    s_out += "DRT: ";
    s_out += s;
    s_out += "\n";

    XPLMDebugString(s_out.c_str());
}