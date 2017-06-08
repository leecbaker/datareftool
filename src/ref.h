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
    RefRecord(const std::string & name, ref_src_t source) : name(name), source(source) {}
public:
    const std::string & getName() const { return name; }
    ref_src_t getSource() const { return source; }
    virtual std::string getDisplayString(size_t display_length) const  = 0;

    bool isBlacklisted() const { return ref_src_t::BLACKLIST == source; }
};