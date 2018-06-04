#pragma once

#include "XPLMDataAccess.h"

#include <chrono>
#include <string>
#include <vector>

#include <boost/variant/variant.hpp>
#include <boost/variant/static_visitor.hpp>

#include "ref.h"

class DataRefUpdater;

class DataRefRecord : public RefRecord {
    friend class DataRefUpdater;
    using value_type = boost::variant<float,double, int, std::vector<float>, std::vector<int>, std::vector<uint8_t>, std::nullptr_t>;
    value_type value;
    
    XPLMDataTypeID type;
    XPLMDataRef ref;
    size_t array_hash = 0;

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
    virtual bool CHECK_RESULT_USED update(DataRefUpdater & updater);
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
    
    void setIntArray(const std::vector<int> & i) { assert(isIntArray()); XPLMSetDatavi(ref, (int *) i.data(), 0, static_cast<int>(i.size())); }
    void setFloatArray(const std::vector<float> & f) { assert(isFloatArray()); XPLMSetDatavf(ref, (float *) f.data(), 0, static_cast<int>(f.size())); }
};

class DataRefUpdater : public boost::static_visitor<bool> {
    std::chrono::system_clock::time_point current_time;
    DataRefRecord * dr;
public:
    void setDataref(DataRefRecord * dr) { this->dr = dr; }
    void updateTime(const std::chrono::system_clock::time_point current_time) { this->current_time = current_time; }
    bool operator()(float&) const;
    bool operator()(double&) const;
    bool operator()(int&) const;
    bool operator()(std::vector<float>&) const;
    bool operator()(std::vector<int>&) const;
    bool operator()(std::vector<uint8_t>&) const;
    bool operator()(std::nullptr_t&) const;
};
