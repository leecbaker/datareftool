#pragma once

#include <string>
#include <vector>

template <class T>
bool parseArray(const std::string & txt, std::vector<T> & data_out, int length);

template <class T>
inline std::string compactFpString(T f) {
    std::string ret = std::to_string(f);
    
    while(false == ret.empty() && ret.back() == '0') {
        ret.erase(ret.end() - 1);
    }
    
    return ret;
}

inline std::string printableFromByteArray(const std::vector<uint8_t> & bytes) {
    std::string ret;
    for(const uint8_t byte : bytes) {
        if(0 == byte) {
            break;
        } else if(::isprint(byte)) {
            ret.push_back(byte);
        } else {
            break;
        }
    }
    
    return ret;
}
