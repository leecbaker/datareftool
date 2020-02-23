#pragma once

#include "XPLMDataAccess.h"

#include <cassert>
#include <chrono>
#include <string>
#include <variant>
#include <vector>

#include "ref.h"
#include "../util/visit_backport.h"

class DataRefUpdater;

class DataRefRecord : public RefRecord {
    friend class DataRefUpdater;
    using value_type = std::variant<float,double, int, std::vector<float>, std::vector<int>, std::vector<uint8_t>, std::nullptr_t>;
    value_type value;
    
    XPLMDataTypeID type;
    XPLMDataRef ref;
    size_t array_hash = 0;

    class GetArraySize {
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
    bool isData() const { return 0 != (xplmType_Data & type); }

    bool isArray() const { return isFloatArray() || isIntArray(); }
    int getArrayLength() const { assert(isArray()); return lb::visit(GetArraySize(),value); }

    void setDouble(double d) { assert(isDouble()); XPLMSetDatad(ref, d); }
    void setFloat(float f) { assert(isFloat()); XPLMSetDataf(ref, f); }
    void setInt(int i) { assert(isInt()); XPLMSetDatai(ref, i); }
    
    void setIntArray(const std::vector<int> & i) { assert(isIntArray()); XPLMSetDatavi(ref, const_cast<int *>(i.data()), 0, static_cast<int>(i.size())); }
    void setFloatArray(const std::vector<float> & f) { assert(isFloatArray()); XPLMSetDatavf(ref, const_cast<float *>(f.data()), 0, static_cast<int>(f.size())); }
    void setData(const std::string & data_str) {
        // In some applications we may need to implement this as well:
        // void setData(const std::vector<uint8_t> &)
        // but taking a string fits best with our use case.
        assert(isData());
        XPLMSetDatab(ref, reinterpret_cast<void *>(const_cast<char *>(data_str.c_str())), 0, static_cast<int>(data_str.size() + 1)); }
};

class DataRefUpdater {
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
