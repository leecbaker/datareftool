#pragma once

#include <chrono>
#include <ostream>
#include <string>

enum class ref_src_t {
    AIRCRAFT,
    IGNORE_FILE,
    FILE,
    PLUGIN,
    USER_MSG,
    X_PLANE,
    LUA,
    DRT_INTERNAL_LIST
};

std::ostream & operator<<(std::ostream & o, const ref_src_t source);

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

    bool isBlacklisted() const { return ref_src_t::IGNORE_FILE == source; }

    const std::chrono::system_clock::time_point & getLastUpdateTime() const { return last_updated; }
    const std::chrono::system_clock::time_point & getLastBigUpdateTime() const { return last_updated_big; }
};
