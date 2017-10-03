#pragma once

#include <chrono>
#include <string>

enum class ref_src_t {
    FILE,
    AIRCRAFT,
    PLUGIN,
    USER_MSG,
    BLACKLIST,
};

/// Superclass defining some common interface items for dataref and commandref.

class RefRecord {
protected:
    std::string name;
    ref_src_t source;
    std::chrono::system_clock::time_point last_updated;
    std::chrono::system_clock::time_point last_updated_big;
    RefRecord(const std::string & name, ref_src_t source) : name(name), source(source), last_updated(std::chrono::system_clock::from_time_t(0)), last_updated_big(std::chrono::system_clock::from_time_t(0)) {}
public:
    virtual ~RefRecord() {}
    const std::string & getName() const { return name; }
    ref_src_t getSource() const { return source; }
    virtual std::string getDisplayString(size_t display_length) const  = 0;

    bool isBlacklisted() const { return ref_src_t::BLACKLIST == source; }

    const std::chrono::system_clock::time_point & getLastUpdateTime() const { return last_updated; }
    const std::chrono::system_clock::time_point & getLastBigUpdateTime() const { return last_updated_big; }
};