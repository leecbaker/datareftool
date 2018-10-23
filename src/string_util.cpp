#include "logging.h"
#include "string_util.h"

#include <boost/algorithm/string.hpp>

#include "XPLMUtilities.h"


template <typename T>
inline T parseElement(const std::string &s);

template <>
inline float parseElement<float>(const std::string &s) {
    return std::stof(s);
}
template <>
inline int parseElement<int>(const std::string &s) {
    return std::stoi(s);
}

template <class T>
bool parseArray(const std::string & txt, std::vector<T> & data_out, int length) {
    std::vector<std::string> txt_fields;
    
    std::string trimmed_txt = txt;
    if(trimmed_txt.front() == '[') {
        trimmed_txt.erase(trimmed_txt.begin());
    }
    
    if(trimmed_txt.back() == '[') {
        trimmed_txt.pop_back();
    }
    
    boost::split(txt_fields, trimmed_txt, boost::is_any_of(","));
    
    if(length != int(txt_fields.size())) {
        xplog << "Save cancelled, as supplied data array doesn't match DR array length\n";
        return false;
    }
    
    data_out.clear();
    data_out.reserve(length);
    
    for(const std::string & txt_field : txt_fields) {
        try {
            data_out.push_back(parseElement<T>(txt_field));
        } catch (std::exception &) {
            xplog << "Save cancelled, failed to parse field\n";
            return false;
        }
    }
    
    return true;
}

template
bool parseArray<float>(const std::string & txt, std::vector<float> & data_out, int length);

template
bool parseArray<int>(const std::string & txt, std::vector<int> & data_out, int length);
