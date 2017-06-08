#pragma once

#include "XPLMDataAccess.h"

#include <chrono>
#include <string>
#include <vector>

#include <boost/variant/variant.hpp>
#include <boost/variant/static_visitor.hpp>

#include "ref.h"


class DataRefRecord : public RefRecord {
    std::chrono::system_clock::time_point last_updated;
    std::chrono::system_clock::time_point last_updated_big;
    using value_type = boost::variant<float,double, int, std::vector<float>, std::vector<int>, std::vector<uint8_t>, std::nullptr_t>;
    value_type value;
    
    XPLMDataTypeID type;
    XPLMDataRef ref;
    size_t array_hash = 0;
    
    class Updater : public boost::static_visitor<bool> {
        DataRefRecord & dr;
        const std::chrono::system_clock::time_point current_time;
    public:
        Updater(DataRefRecord & dr, const std::chrono::system_clock::time_point current_time)
        : dr(dr), current_time(current_time) {}
        bool operator()(float&) const;
        bool operator()(double&) const;
        bool operator()(int&) const;
        bool operator()(std::vector<float>&) const;
        bool operator()(std::vector<int>&) const;
        bool operator()(std::vector<uint8_t>&) const;
        bool operator()(std::nullptr_t&) const;
    };
    class GetArraySize : public boost::static_visitor<int> {
    public:
        int operator()(const float&) const { return -1; }
        int operator()(const double&) const { return -1; }
        int operator()(const int&) const { return -1; }
        int operator()(const std::vector<float>& v) const { return int(v.size()); }
        int operator()(const std::vector<int>& v) const { return int(v.size()); }
        int operator()(const std::vector<uint8_t>& v) const { return int(v.size()); }
        int operator()(const std::nullptr_t&) const { return -1; }
    };
public:
    DataRefRecord(const std::string & name, XPLMDataRef ref, ref_src_t source);
    
    /// @return true if updated, false if not
    std::string getLabelString() const;
    virtual std::string getDisplayString(size_t display_length) const override;
    std::string getEditString() const;
    virtual bool update(const std::chrono::system_clock::time_point current_time);
    const std::chrono::system_clock::time_point & getLastUpdateTime() const { return last_updated; }
    const std::chrono::system_clock::time_point & getLastBigUpdateTime() const { return last_updated_big; }
    bool writable() const;
    
    bool isDouble() const { return 0 != (xplmType_Double & type); }
    bool isFloat() const { return 0 != (xplmType_Float & type); }
    bool isInt() const { return 0 != (xplmType_Int & type); }
    bool isFloatArray() const { return 0 != (xplmType_FloatArray & type); }
    bool isIntArray() const { return 0 != (xplmType_IntArray & type); }
    
    bool isArray() const { return isFloatArray() || isIntArray(); }
    int getArrayLength() const { assert(isArray()); return boost::apply_visitor(GetArraySize(),value); }
    
    void setDouble(double d) { assert(isDouble()); XPLMSetDatad(ref, d); }
    void setFloat(float f) { assert(isFloat()); XPLMSetDataf(ref, f); }
    void setInt(int i) { assert(isInt()); XPLMSetDatai(ref, i); }
    
    void setIntArray(const std::vector<int> & i) { assert(isIntArray()); XPLMSetDatavi(ref, (int *) i.data(), 0, i.size()); }
    void setFloatArray(const std::vector<float> & f) { assert(isFloatArray()); XPLMSetDatavf(ref, (float *) f.data(), 0, f.size()); }
};
