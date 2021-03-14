#pragma once

#include <ostream>
#include <streambuf>

#include "XPLMProcessing.h"
#include "XPLMUtilities.h"

#include "../lb_common/lb_synchronized_queue.h"

/// Logging class for X-Plane.
/// @author Lee C Baker
/// @date 28 August 2018
//
/// @copyright 2018-2021 Lee C Baker


// Aggregate log data from multiple threads, and feed into X-Plane's logging system.
// One instance of this per log.
class XPLogAggregator {
    std::atomic_bool enabled = true;
    std::string prefix_;
    XPLMFlightLoopID callback_id;
    static float logging_callback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void * refcon);
    LBSynchronizedQueue<std::string> message_queue;
public:
    XPLogAggregator();
    ~XPLogAggregator();
    void setPrefix(const std::string & prefix) { prefix_ = prefix; }
    void setEnabled(bool e) { enabled = e; }
    // messages come in from 
    void log(std::string message) { 
        if(enabled) {
            message_queue.push(std::move(message));
        }
    }
};

// Ostream-style buffer for logging. One per thread.
class XPLoggingBuffer : public std::streambuf {
    bool next_has_prefix = true;

    std::string local_buffer;
    XPLogAggregator & aggregator;
protected:
    virtual int_type overflow (int_type c) {
        if (c != EOF) {
            bool is_newline = static_cast<char>(c) == '\n';
            next_has_prefix = is_newline;

            if(is_newline) { //flush
                local_buffer.append("\n");
                aggregator.log(std::move(local_buffer));
                local_buffer.clear();
            } else {
                local_buffer.append(1, static_cast<char>(c));
            }

        }
        return c;
    }

public:
    XPLoggingBuffer(XPLogAggregator & aggregator) : aggregator(aggregator) {}
    void setEnabled(bool e) { aggregator.setEnabled(e); }
};

class XPLogger : public std::ostream {
    XPLoggingBuffer logging_buffer;
public:
    XPLogger(XPLogAggregator & aggregator) : std::ostream(&logging_buffer), logging_buffer(aggregator) {}
    void setEnabled(bool e) { logging_buffer.setEnabled(e); }
};

extern XPLogAggregator xplog_aggregator;
extern XPLogAggregator xplog_debug_aggregator;

extern thread_local XPLogger xplog;
extern thread_local XPLogger xplog_debug;

