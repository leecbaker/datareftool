#include "dataref.h"

#include "../util/string_util.h"

#include <cassert>
#include <cmath>
#include <sstream>

#include "XPLMGraphics.h"

#include "lb_hash.h"

DataRefRecord::DataRefRecord(const std::string & name, XPLMDataRef ref, ref_src_t source) : RefRecord(name, source), ref(ref) {
    type = XPLMGetDataRefTypes(ref);
    
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
    if(isInt() || isDouble() || isFloat() || isFloatArray() || isIntArray() || isData()) {
        return 0 != XPLMCanWriteDataRef(ref);
    }
    
    return false;
}


bool DataRefUpdater::operator()(float & old_val) const {
    float newval = XPLMGetDataf(dr->ref);
    if(newval != old_val) {
        if(0.01f < fabs(newval - old_val) / old_val) {
            dr->last_updated_big = current_time;
        }
        dr->last_updated = current_time;
        dr->previous_value = dr->value;
        dr->value = newval;
        return true;
    } else {
        return false;
    }
}

bool DataRefUpdater::operator()(double& oldval) const {
    double newval = XPLMGetDatad(dr->ref);
    if(newval != oldval) {
        if(0.01 < fabsl(newval - oldval) / oldval) {
            dr->last_updated_big = current_time;
        }
        dr->last_updated = current_time;
        dr->previous_value = dr->value;
        dr->value = newval;
        return true;
    } else {
        return false;
    }
}

bool DataRefUpdater::operator()(int& oldval) const {
    int newval = XPLMGetDatai(dr->ref);
    if(newval != oldval) {
        dr->last_updated = current_time;
        dr->previous_value = dr->value;
        dr->value = newval;
        return true;
    } else {
        return false;
    }
}

bool DataRefUpdater::operator()(std::vector<float>&) const {
    int current_array_length = XPLMGetDatavf(dr->ref, nullptr, 0, 0);
    static std::vector<float> scratch_buffer;
    scratch_buffer.resize(current_array_length);
    std::fill(scratch_buffer.begin(), scratch_buffer.end(), 0.f);
    int copied = XPLMGetDatavf(dr->ref, scratch_buffer.data(), 0, current_array_length);
    if(copied == current_array_length) {
        size_t new_hash = hash_range(scratch_buffer.cbegin(), scratch_buffer.cend());
        if(new_hash != dr->array_hash) {
            dr->array_hash = new_hash;
            dr->last_updated = current_time;
            dr->value = scratch_buffer;
            return true;
        } else {
            return false;
        }
    } else {
        //Dataref is reporting an inconsistent size
        return false;
    }
    
}

bool DataRefUpdater::operator()(std::vector<int>&) const {
    int current_array_length = XPLMGetDatavi(dr->ref, nullptr, 0, 0);
    static std::vector<int> scratch_buffer;
    scratch_buffer.resize(current_array_length);
    std::fill(scratch_buffer.begin(), scratch_buffer.end(), 0);
    int copied = XPLMGetDatavi(dr->ref, scratch_buffer.data(), 0, current_array_length);
    if(copied == current_array_length) {
        size_t new_hash = hash_range(scratch_buffer.cbegin(), scratch_buffer.cend());
        if(new_hash != dr->array_hash) {
            dr->array_hash = new_hash;
            dr->last_updated = current_time;
            dr->value = scratch_buffer;
            return true;
        } else {
            return false;
        }
    } else {
        //Dataref is reporting an inconsistent size
        return false;
    }
    
}

bool DataRefUpdater::operator()(std::vector<uint8_t>&) const {
    int current_array_length = XPLMGetDatab(dr->ref, nullptr, 0, 0);
    static std::vector<uint8_t> scratch_buffer;
    scratch_buffer.resize(current_array_length);
    int copied = XPLMGetDatab(dr->ref, scratch_buffer.data(), 0, current_array_length);
    if(copied == current_array_length) {
        size_t new_hash = hash_range(scratch_buffer.cbegin(), scratch_buffer.cend());
        if(new_hash != dr->array_hash) {
            dr->array_hash = new_hash;
            dr->last_updated = current_time;
            dr->value = scratch_buffer;
            return true;
        } else {
            return false;
        }
    } else {
        //Dataref is reporting an inconsistent size
        return false;
    }
}

bool DataRefUpdater::operator()(std::nullptr_t&) const {
    dr->last_updated = current_time;
    return false;
}

bool DataRefRecord::update(DataRefUpdater & updater) {
    updater.setDataref(this);
    return lb::visit(updater, value);
}

std::string DataRefRecord::getLabelString() const {
    std::string ret = getName();
    if(isArray()) {
        ret += "[" + std::to_string(getArrayLength()) + "]";
    }
    return ret;
}

template <class T>
std::string makeArrayString(std::string (*stringify_func)(T), const std::vector<T> & array, size_t max_pixels) {
    std::stringstream s;
    s << "[";
    
    static const std::string elipses = "..";
    static const float elipses_length = XPLMMeasureString(xplmFont_Proportional, elipses.c_str(), elipses.size());
    static const float comma_length = XPLMMeasureString(xplmFont_Proportional, ",", 1);
    static const float braces_length = XPLMMeasureString(xplmFont_Proportional, "[]", 2);

    float current_length = braces_length;

    for(typename std::vector<T>::const_iterator element_it = array.cbegin(); element_it != array.cend(); element_it++) {
        const std::string & s_element = stringify_func(*element_it);

        float value_length = XPLMMeasureString(xplmFont_Proportional, s_element.c_str(), s_element.size());
        
        //float max_length_if_ending_now = current_length + elipses_length;
        float max_length_if_ending_next = current_length + comma_length + value_length;

        if(element_it + 1 != array.cend()) {
            max_length_if_ending_next += elipses_length;
        }

        if(max_length_if_ending_next < max_pixels) {
            if(element_it != array.cbegin()) {
                s << ",";
            }
            s << s_element;
            current_length += comma_length + value_length;
        } else {
            s << elipses;
            break;
        }
    }
    
    s << "]";
    return s.str();
}

class DatarefDisplayStringifier {
    size_t max_pixels;
public:
    DatarefDisplayStringifier(size_t max_pixels) : max_pixels(max_pixels) {}
    std::string operator()(const float & f) const { return compactFpString(f); }
    std::string operator()(const double & f) const { return compactFpString(f); }
    std::string operator()(const int & i) const { return std::to_string(i); }
    std::string operator()(const std::vector<float> & fv) const {
        return makeArrayString<float>(compactFpString<float>, fv, max_pixels);
    }
    std::string operator()(const std::vector<int> & iv) const {
        std::string (*stringify_func)(int) = std::to_string;
        return makeArrayString<int>(stringify_func, iv, max_pixels);
    }
    std::string operator()(const std::vector<uint8_t> & iv) const {
        return "\"" + printableFromByteArray(iv) + "\"";
    }
    std::string operator()(const std::nullptr_t &) const {
        return "(null)";
    }
};

class DatarefEditStringifier {
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
    std::string operator()(const std::vector<uint8_t> & fv) const {
        return printableFromByteArray(fv);
    }
    std::string operator()(const std::nullptr_t ) const {
        return "(null)";
    }
};

std::string DataRefRecord::getDisplayString(size_t display_length) const {
    if(isBlacklisted()) {
        return "ignored";
    } else {
        return lb::visit(DatarefDisplayStringifier(display_length), value);
    }
}

std::string DataRefRecord::getEditString() const {
    return lb::visit(DatarefEditStringifier(), value);
}

class DatarefArrayElementEditStringifier {
    int element_index;
public:
    DatarefArrayElementEditStringifier(int element_index) : element_index(element_index) {}
    std::string operator()(const float f) const { return compactFpString(f); }
    std::string operator()(const double f) const { return compactFpString(f); }
    std::string operator()(const int i) const { return std::to_string(i); }
    std::string operator()(const std::vector<float> & fv) const {
        return compactFpString<float>(fv[element_index]);
    }
    std::string operator()(const std::vector<int> & iv) const {
        return std::to_string(iv[element_index]);
    }
    std::string operator()(const std::vector<uint8_t> & v) const {
        return printableFromByteArray(v);
    }
    std::string operator()(const std::nullptr_t ) const {
        return "(null)";
    }
};

std::string DataRefRecord::getArrayElementEditString(int index) {
    return lb::visit(DatarefArrayElementEditStringifier(index), value);
}
