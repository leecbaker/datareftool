#include "dataref.h"

#include "string_util.h"

#include <cmath>
#include <sstream>

#include <boost/functional/hash.hpp>

DataRefRecord::DataRefRecord(const std::string & name, XPLMDataRef ref, ref_src_t source) : RefRecord(name, source), last_updated(std::chrono::system_clock::now()), ref(ref) {
    type = 	XPLMGetDataRefTypes(ref);
    
    if(type & xplmType_Double) {
        value = 0.;
    } else if (type & xplmType_Float) {
        value = 0.f;
    } else if (type & xplmType_Int) {
        value = 0;
    } else if (type & xplmType_FloatArray) {
        value = std::vector<float>();
    } else if (type & xplmType_IntArray) {
        value = std::vector<int>();
    } else if (type & xplmType_Data) {
        value = std::vector<uint8_t>();
    } else {
        value = nullptr;
    }
}

bool DataRefRecord::writable() const {
    if(isInt() || isDouble() || isFloat() || isFloatArray() || isIntArray()) {
        return 0 != XPLMCanWriteDataRef(ref);
    }
    
    return false;
}


bool DataRefRecord::Updater::operator()(float & old_val) const {
    float newval = XPLMGetDataf(dr.ref);
    if(newval != old_val) {
        if(0.01f < fabs(newval - old_val) / old_val) {
            dr.last_updated_big = current_time;
        }
        dr.last_updated = current_time;
        dr.value = newval;
        return true;
    } else {
        return false;
    }
}

bool DataRefRecord::Updater::operator()(double& oldval) const {
    double newval = XPLMGetDatad(dr.ref);
    if(newval != oldval) {
        if(0.01 < fabsl(newval - oldval) / oldval) {
            dr.last_updated_big = current_time;
        }
        dr.last_updated = current_time;
        dr.value = newval;
        return true;
    } else {
        return false;
    }
}

bool DataRefRecord::Updater::operator()(int& oldval) const {
    int newval = XPLMGetDatai(dr.ref);
    if(newval != oldval) {
        dr.last_updated = current_time;
        dr.value = newval;
        return true;
    } else {
        return false;
    }
}

bool DataRefRecord::Updater::operator()(std::vector<float>&) const {
    int current_array_length = XPLMGetDatavf(dr.ref, nullptr, 0, 0);
    static std::vector<float> scratch_buffer;
    scratch_buffer.resize(current_array_length);
    int copied = XPLMGetDatavf(dr.ref, scratch_buffer.data(), 0, current_array_length);
    if(copied == current_array_length) {
        size_t new_hash = boost::hash_range(scratch_buffer.cbegin(), scratch_buffer.cend());
        if(new_hash != dr.array_hash) {
            dr.array_hash = new_hash;
            dr.last_updated = current_time;
            dr.value = scratch_buffer;
            return true;
        } else {
            return false;
        }
    } else {
        //Dataref is reporting an inconsistent size
        return false;
    }
    
}

bool DataRefRecord::Updater::operator()(std::vector<int>&) const {
    int current_array_length = XPLMGetDatavi(dr.ref, nullptr, 0, 0);
    static std::vector<int> scratch_buffer;
    scratch_buffer.resize(current_array_length);
    int copied = XPLMGetDatavi(dr.ref, scratch_buffer.data(), 0, current_array_length);
    if(copied == current_array_length) {
        size_t new_hash = boost::hash_range(scratch_buffer.cbegin(), scratch_buffer.cend());
        if(new_hash != dr.array_hash) {
            dr.array_hash = new_hash;
            dr.last_updated = current_time;
            dr.value = scratch_buffer;
            return true;
        } else {
            return false;
        }
    } else {
        //Dataref is reporting an inconsistent size
        return false;
    }
    
}

bool DataRefRecord::Updater::operator()(std::vector<uint8_t>&) const {
    int current_array_length = XPLMGetDatab(dr.ref, nullptr, 0, 0);
    static std::vector<uint8_t> scratch_buffer;
    scratch_buffer.resize(current_array_length);
    int copied = XPLMGetDatab(dr.ref, scratch_buffer.data(), 0, current_array_length);
    if(copied == current_array_length) {
        size_t new_hash = boost::hash_range(scratch_buffer.cbegin(), scratch_buffer.cend());
        if(new_hash != dr.array_hash) {
            dr.array_hash = new_hash;
            dr.last_updated = current_time;
            dr.value = scratch_buffer;
            return true;
        } else {
            return false;
        }
    } else {
        //Dataref is reporting an inconsistent size
        return false;
    }
    
}

bool DataRefRecord::Updater::operator()(std::nullptr_t&) const {
    dr.last_updated = current_time;
    return false;
}

bool DataRefRecord::update(const std::chrono::system_clock::time_point current_time) {
    return boost::apply_visitor(Updater(*this, current_time), value);
}

std::string DataRefRecord::getLabelString() const {
    std::string ret = getName();
    if(isArray()) {
        ret += "[" + std::to_string(getArrayLength()) + "]";
    }
    return ret;
}

template <class T>
std::string makeArrayString(std::string (*stringify_func)(T), const std::vector<T> & array, size_t max_chars) {
    std::stringstream s;
    s << "[";
    
    size_t current_length = 2;  //for the braces
    const std::string elipses = "...";
    
    for(const T & element : array) {
        const std::string & s_element = stringify_func(element);
        
        size_t max_length_if_ending_now = current_length + elipses.size();
        size_t max_length_if_ending_next = max_length_if_ending_now + 1 + s_element.size(); //comma + value
        
        if(max_length_if_ending_next < max_chars) {
            if(current_length != 2) {
                s << ",";
            }
            s << s_element;
            current_length += 1 + s_element.size();
        } else {
            s << elipses;
            break;
        }
    }
    
    s << "]";
    return s.str();
}

class DatarefDisplayStringifier : public boost::static_visitor<std::string> {
    size_t max_chars;
public:
    DatarefDisplayStringifier(size_t max_chars) : max_chars(max_chars) {}
    std::string operator()(const float & f) const { return compactFpString(f); }
    std::string operator()(const double & f) const { return compactFpString(f); }
    std::string operator()(const int & i) const { return std::to_string(i); }
    std::string operator()(const std::vector<float> & fv) const {
        return makeArrayString<float>(compactFpString<float>, fv, max_chars);
    }
    std::string operator()(const std::vector<int> & iv) const {
        std::string (*stringify_func)(int) = std::to_string;
        return makeArrayString<int>(stringify_func, iv, max_chars);
    }
    std::string operator()(const std::vector<uint8_t> & iv) const {
        return "\"" + printableFromByteArray(iv) + "\"";
    }
    std::string operator()(const std::nullptr_t &) const {
        return "(null)";
    }
};

class DatarefEditStringifier : public boost::static_visitor<std::string> {
public:
    std::string operator()(const float f) const { return compactFpString(f); }
    std::string operator()(const double f) const { return compactFpString(f); }
    std::string operator()(const int i) const { return std::to_string(i); }
    std::string operator()(const std::vector<float> & fv) const {
        return makeArrayString<float>(compactFpString<float>, fv, std::numeric_limits<size_t>::max());
    }
    std::string operator()(const std::vector<int> & iv) const {
        std::string (*stringify_func)(int) = std::to_string;
        return makeArrayString<int>(stringify_func, iv, std::numeric_limits<size_t>::max());
    }
    std::string operator()(const std::vector<uint8_t> & iv) const {
        return "\"" + printableFromByteArray(iv) + "\"";
    }
    std::string operator()(const std::nullptr_t ) const {
        return "(null)";
    }
};

std::string DataRefRecord::getDisplayString(size_t display_length) const {
    if(isBlacklisted()) {
        return "blacklisted";
    } else {
        return boost::apply_visitor(DatarefDisplayStringifier(display_length), value);
    }
}

std::string DataRefRecord::getEditString() const {
    return boost::apply_visitor(DatarefEditStringifier(), value);
}
