#pragma once

#include <ostream>
#include <streambuf>

#include "XPLMUtilities.h"

/// Logging class for X-Plane.
/// @author Lee C Baker
/// @date 28 August 2018
//
/// @copyright 2018 Lee C Baker

class XPLoggingBuffer : public std::streambuf {
	std::string prefix_;
	bool next_has_prefix = true;
    bool enabled = true;
protected:
	virtual int_type overflow (int_type c) {
	    if (c != EOF && enabled) {
			if(next_has_prefix) {
				XPLMDebugString(prefix_.c_str());
			}
			next_has_prefix = (static_cast<char>(c) == '\n');

            char msg[2] = {static_cast<char>(c), 0};
            XPLMDebugString(msg);
	    }
	    return c;
	}

public:
    void setEnabled(bool e) { enabled = e; }
	void setPrefix(const std::string & prefix) { prefix_ = prefix;}
};

class XPLogger : public std::ostream {
    XPLoggingBuffer logging_buffer;
public:
    XPLogger() : std::ostream(&logging_buffer), logging_buffer() {}
	void setPrefix(const std::string & prefix) { logging_buffer.setPrefix(prefix); }
    void setEnabled(bool e) { logging_buffer.setEnabled(e); }
};

extern XPLogger xplog;
extern XPLogger xplog_debug;
