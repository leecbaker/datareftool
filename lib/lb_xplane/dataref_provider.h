#pragma once
#include "XPLMDataAccess.h"

#include <functional>
#include <string>

template <class Type>
class DatarefProvider {
    XPLMDataRef dr = nullptr;
    std::function<Type(void)> read_func;
    std::function<void(Type)> write_func;
public:
    DatarefProvider(std::string name, std::function<Type(void)> read_func, std::function<void(Type)> write_func = {});
    ~DatarefProvider();
};

template <>
class DatarefProvider<int> {
    XPLMDataRef dr = nullptr;
    std::function<int(void)> read_func;
    std::function<void(int)> write_func;

    inline static int read_int(void * refcon) {
        auto pthis = static_cast<DatarefProvider<int> *>(refcon);
        return pthis->read_func();
    }

    inline static void write_int(void * refcon, int value) {
        auto pthis = static_cast<DatarefProvider<int> *>(refcon);
        pthis->write_func(value);
    }

public:
    inline DatarefProvider(std::string name, std::function<int(void)> read_func, std::function<void(int)> write_func = {}) : read_func(read_func), write_func(write_func) {
        dr = XPLMRegisterDataAccessor(name.c_str(), xplmType_Int, bool(write_func),
                                      &DatarefProvider::read_int, &DatarefProvider::write_int, // Integer accessors
                                      NULL, NULL, // Float accessors
                                      NULL, NULL, // Doubles accessors
                                      NULL, NULL, // Int array accessors
                                      NULL, NULL, // Float array accessors
                                      NULL, NULL, // Raw data accessors
                                      static_cast<void *>(this), static_cast<void *>(this));
    }


    inline ~DatarefProvider() {
        XPLMUnregisterDataAccessor(dr);
    }
};

template <>
class DatarefProvider<bool> {
    XPLMDataRef dr = nullptr;
    std::function<bool(void)> read_func;
    std::function<void(bool)> write_func;

    inline static int read_bool(void * refcon) {
        auto pthis = static_cast<DatarefProvider<bool> *>(refcon);
        return pthis->read_func() ? 1 : 0;
    }

    inline static void write_bool(void * refcon, int value) {
        auto pthis = static_cast<DatarefProvider<bool> *>(refcon);
        pthis->write_func(0 == value ? false : true);
    }

public:
    inline DatarefProvider(std::string name, std::function<bool(void)> read_func, std::function<void(bool)> write_func = {}) : read_func(read_func), write_func(write_func) {
        dr = XPLMRegisterDataAccessor(name.c_str(), xplmType_Int, bool(write_func),
                                      &DatarefProvider::read_bool, &DatarefProvider::write_bool, // Integer accessors
                                      NULL, NULL, // Float accessors
                                      NULL, NULL, // Doubles accessors
                                      NULL, NULL, // Int array accessors
                                      NULL, NULL, // Float array accessors
                                      NULL, NULL, // Raw data accessors
                                      static_cast<void *>(this), static_cast<void *>(this));
    }


    inline ~DatarefProvider() {
        XPLMUnregisterDataAccessor(dr);
    }
};

template <>
class DatarefProvider<std::string> {
    XPLMDataRef dr = nullptr;
    std::function<std::string(void)> read_func;
    std::function<void(std::string)> write_func;

    inline static int read_string(void * refcon, void * outValue, int inOffset, int inMaxLength) {
        auto pthis = static_cast<DatarefProvider<std::string> *>(refcon);
        if(nullptr == outValue) {
            return static_cast<int>(pthis->read_func().size());
        }
        std::string value = pthis->read_func();
        if(inOffset >= static_cast<int>(value.size())) {
            return 0;
        }

        value.erase(value.begin(), value.begin() + inOffset);

        if(inMaxLength < static_cast<int>(value.size())) {
            value.erase(value.begin() + inMaxLength);
        }
        std::copy_n(value.begin(), value.size(), static_cast<char *>(outValue));
        return static_cast<int>(value.size());
    }

    inline static void write_string(void * refcon, void * inValue, int /* inOffset */, int inLength) {
        // intentionally ignoring inOffset.
        auto pthis = static_cast<DatarefProvider<std::string> *>(refcon);

        std::string value(static_cast<char *>(inValue), inLength);
        pthis->write_func(value);
    }

public:
    inline DatarefProvider(std::string name, std::function<std::string(void)> read_func, std::function<void(std::string)> write_func = {}) : read_func(read_func), write_func(write_func) {
        dr = XPLMRegisterDataAccessor(name.c_str(), xplmType_Data, bool(write_func),
                                      nullptr, nullptr, // Integer accessors
                                      NULL, NULL, // Float accessors
                                      NULL, NULL, // Doubles accessors
                                      NULL, NULL, // Int array accessors
                                      NULL, NULL, // Float array accessors
                                      &DatarefProvider::read_string, &DatarefProvider::write_string, // Raw data accessors
                                      static_cast<void *>(this), static_cast<void *>(this));
    }

    inline ~DatarefProvider() {
        XPLMUnregisterDataAccessor(dr);
    }
};

template <>
class DatarefProvider<float> {
    XPLMDataRef dr = nullptr;
    std::function<float(void)> read_func;
    std::function<void(float)> write_func;

    inline static float read_float(void * refcon) {
        auto pthis = static_cast<DatarefProvider<float> *>(refcon);
        return pthis->read_func();
    }

    inline static void write_float(void * refcon, float value) {
        auto pthis = static_cast<DatarefProvider<float> *>(refcon);
        pthis->write_func(value);
    }

public:
    inline DatarefProvider(std::string name, std::function<float(void)> read_func, std::function<void(float)> write_func = {}) : read_func(read_func), write_func(write_func) {
        dr = XPLMRegisterDataAccessor(name.c_str(), xplmType_Float, write_func != nullptr,
                                      nullptr, nullptr, // Integer accessors
                                      &DatarefProvider::read_float, &DatarefProvider::write_float, // Float accessors
                                      NULL, NULL, // Doubles accessors
                                      NULL, NULL, // Int array accessors
                                      NULL, NULL, // Float array accessors
                                      NULL, NULL, // Raw data accessors
                                      static_cast<void *>(this), static_cast<void *>(this));
    }


    inline ~DatarefProvider() {
        XPLMUnregisterDataAccessor(dr);
    }
};
