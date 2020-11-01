#pragma once

#include <algorithm>
#include <exception>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "XPLMDataAccess.h"

class DatarefError : public std::exception {
    std::string ref_name_;
    std::string message_;
protected:
    DatarefError(std::string ref_name, std::string message) : ref_name_(ref_name), message_(message) {}
public:
    virtual const char* what() const noexcept {
        return message_.c_str();
    }
};
class DatarefMissingError : public DatarefError {
public:
    DatarefMissingError(const std::string ref_name) : DatarefError(ref_name, "Failed to find dataref \"" + ref_name + "\"") {
    }
};

class DatarefTypeError : public DatarefError {
public:
    DatarefTypeError(const std::string ref_name, XPLMDataTypeID type) :
    DatarefError(ref_name, "Dataref is wrong type: " + ref_name + " expect type " + std::to_string(type)) { }
};

class DatarefAssignValueError : public DatarefError {
public:
    DatarefAssignValueError(const std::string ref_name) :
    DatarefError(ref_name, "Value assigned to dataref is invalid") { }
};


template <class Type>
class Dataref {
protected:
    int array_length = 1;
    XPLMDataRef ref;
    bool checkType();
public:
    Dataref(const std::string & name) {
        ref = XPLMFindDataRef(name.c_str());
        
        if(nullptr == ref) {
            throw DatarefMissingError(name);
        }
        
        checkType();
        
        if(false == checkType()) {
            throw DatarefTypeError(name, XPLMGetDataRefTypes(ref));
        }
    }
    
    Type get() const;
    void set(Type val) const;
};

template <class ElementType>
class Dataref<std::vector<ElementType>> {
    using Type = std::vector<ElementType>;
protected:
    int array_length = 1;
    XPLMDataRef ref;
    bool checkType();
public:
    Dataref(const std::string & name) {
        ref = XPLMFindDataRef(name.c_str());
        
        if(nullptr == ref) {
            throw DatarefMissingError(name);
        }
        
        checkType();
        
        if(false == checkType()) {
            throw DatarefTypeError(name, XPLMGetDataRefTypes(ref));
        }
    }
    
    Type get() const;
    // array only
    ElementType getElement(int index) const;
    void set(Type val) const;
};

template <>
inline bool Dataref<float>::checkType() {
    XPLMDataTypeID type = XPLMGetDataRefTypes(ref);
    return 0 != (type & xplmType_Float);
}

template <>
inline bool Dataref<double>::checkType() {
    XPLMDataTypeID type = XPLMGetDataRefTypes(ref);
    return 0 != (type & xplmType_Double);
}

template <>
inline bool Dataref<int>::checkType() {
    XPLMDataTypeID type = XPLMGetDataRefTypes(ref);
    return 0 != (type & xplmType_Int);
}

template <>
inline bool Dataref<bool>::checkType() {
    XPLMDataTypeID type = XPLMGetDataRefTypes(ref);
    return 0 != (type & xplmType_Int);
}

template <>
inline bool Dataref<std::vector<int>>::checkType() {
    XPLMDataTypeID type = XPLMGetDataRefTypes(ref);
    if(0 == (type & xplmType_IntArray)) {
        return false;
    }
    
    array_length = XPLMGetDatavi(ref, nullptr, 0 ,0);
    
    return true;
}

template <>
inline bool Dataref<std::vector<bool>>::checkType() {
    XPLMDataTypeID type = XPLMGetDataRefTypes(ref);
    if(0 == (type & xplmType_IntArray)) {
        return false;
    }
    
    array_length = XPLMGetDatavi(ref, nullptr, 0 ,0);
    
    return true;
}

template <>
inline bool Dataref<std::vector<float>>::checkType() {
    XPLMDataTypeID type = XPLMGetDataRefTypes(ref);
    if(0 == (type & xplmType_FloatArray)) {
        return false;
    }
    
    array_length = XPLMGetDatavf(ref, nullptr, 0 ,0);
    
    return true;
}

template <>
inline bool Dataref<std::string>::checkType() {
    XPLMDataTypeID type = XPLMGetDataRefTypes(ref);
    if(0 == (type & xplmType_Data)) {
        return false;
    }
    
    array_length = XPLMGetDatavf(ref, nullptr, 0 ,0);
    
    return true;
}

template <>
inline float Dataref<float>::get() const {
    return XPLMGetDataf(ref);
}

template <>
inline double Dataref<double>::get() const {
    return XPLMGetDatad(ref);
}

template <>
inline int Dataref<int>::get() const {
    return XPLMGetDatai(ref);
}

template <>
inline bool Dataref<bool>::get() const {
    return 0 != XPLMGetDatai(ref);
}

template <>
inline bool Dataref<std::vector<bool>>::getElement(int index) const {
    int ibool;
    XPLMGetDatavi(ref, &ibool, index, 1);
    return 0 != ibool;
}

template <>
inline int Dataref<std::vector<int>>::getElement(int index) const {
    int iout;
    XPLMGetDatavi(ref, &iout, index, 1);
    return iout;
}

template <>
inline float Dataref<std::vector<float>>::getElement(int index) const {
    float f;
    XPLMGetDatavf(ref, &f, index, 1);
    return f;
}

template <>
inline std::vector<float> Dataref<std::vector<float>>::get() const {
    std::vector<float> v;
    v.resize(array_length);
    int elements_read = XPLMGetDatavf(ref, v.data(), 0, array_length);
    if(elements_read != array_length) {
        v.clear();
    }
    return v;
}

template <>
inline std::vector<int> Dataref<std::vector<int>>::get() const {
    std::vector<int> v;
    v.resize(array_length);
    int elements_read = XPLMGetDatavi(ref, v.data(), 0, array_length);
    if(elements_read != array_length) {
        v.clear();
    }
    return v;
}

template <>
inline std::vector<bool> Dataref<std::vector<bool>>::get() const {
    std::vector<int> v_int;
    std::vector<bool> v_bool;
    v_int.resize(array_length);
    v_bool.resize(array_length);
    int elements_read = XPLMGetDatavi(ref, v_int.data(), 0, array_length);
    if(elements_read != array_length) {
        v_bool.clear();
        return v_bool;
    }
    for(int i = 0; i < array_length; i++) {
        v_bool[i] = 0 != v_int[i];
    }
    return v_bool;
}

template <>
inline std::string Dataref<std::string>::get() const {
    std::vector<char> ret;
    int expected_bytes = XPLMGetDatab(ref, nullptr, 0 ,0);
    ret.resize(expected_bytes);

    int bytes_read = XPLMGetDatab(ref, reinterpret_cast<void *>(ret.data()), 0, expected_bytes);
    if(bytes_read != expected_bytes) {
        ret.clear();
    }

    //null terminate / resize
    std::vector<char>::const_iterator first_null = std::find(ret.cbegin(), ret.cend(), '\0');
    if(ret.cend() != first_null) {
        ret.erase(first_null, ret.cend());
    }

    return std::string(ret.cbegin(), ret.cend());
}


template <>
inline void Dataref<float>::set(float val) const {
    return XPLMSetDataf(ref, val);
}

template <>
inline void Dataref<int>::set(int val) const {
    return XPLMSetDatai(ref, val);
}

template <>
inline void Dataref<bool>::set(bool val) const {
    return XPLMSetDatai(ref, val ? 1 : 0);
}


inline std::tuple<std::string, int> splitDRNameIfArray(const std::string & full_name) {
    size_t brace_start = full_name.find('[');
    if(brace_start == std::string::npos) {
        return std::make_tuple(full_name, -1);
    } else {
        std::string dr_name = full_name.substr(0, brace_start);
        size_t index_length = full_name.size() - brace_start - 2;
        std::string index_bit = full_name.substr(brace_start + 1, index_length);
        int index = std::stoi(full_name.substr(brace_start + 1, index_length));
        return std::make_tuple(dr_name, index);
    }
}

template <class Type>
class DatarefOrElement {
    std::optional<Dataref<Type>> dr;
    std::optional<Dataref<std::vector<Type>>> dr_vector;

    int element = -1;
public:
    DatarefOrElement(const std::string & name) {
        std::string dr_name;
        std::tie(dr_name, element) = splitDRNameIfArray(name);
        if(-1 == element) {
            dr.emplace(dr_name);
        } else {
            dr_vector.emplace(dr_name);
        }
    }

    Type get() const {
        if(-1 == element) {
            return dr->get();
        } else{
            return dr_vector->getElement(element);
        }
    }
};

// Read only for now, because that's all we need
class DatarefFloatConvertable {
    int index;
    XPLMDataTypeID type;
    XPLMDataRef ref = nullptr;
public:
    DatarefFloatConvertable(const std::string & name) {
        std::string dr_name;
        std::tie(dr_name, index) = splitDRNameIfArray(name);
        ref = XPLMFindDataRef(dr_name.c_str());
        if(nullptr == ref) {
            throw DatarefMissingError(name);
        }
        type = XPLMGetDataRefTypes(ref);
        XPLMDataTypeID acceptable_types = xplmType_IntArray | xplmType_Int | xplmType_FloatArray | xplmType_Float;
        if(0 == (type & acceptable_types)) {
            throw DatarefTypeError(name, type);
        }
    }
    float get() {
        if(type & xplmType_Float) {
            return static_cast<float>(XPLMGetDataf(ref));
        } else if(type & xplmType_FloatArray) {
            float f;
            XPLMGetDatavf(ref, &f, index, 1);
            return f;
        } else if(type & xplmType_Int) {
            return static_cast<float>(XPLMGetDatai(ref));
        } else if(xplmType_IntArray & type) { //must be int array
            int i;
            XPLMGetDatavi(ref, &i, index, 1);
            return static_cast<float>(i);
        } else {
            //error should have been caught in constructor
            throw std::runtime_error("Dataref is not one of the accepted types");
        }
    }
};
