#pragma once
#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <vector>
#include <string>
#include <unordered_map>
#include "XPLMDataAccess.h"

#include <boost/filesystem.hpp>

#include "dataref.h"

class DataRefRecords {
    using RecordStorageType = std::vector<DataRefRecord>;
    using NameMapType = std::unordered_map<std::string, RecordStorageType::const_iterator>;

    RecordStorageType datarefs;
    NameMapType dataref_ordered;
public:
    bool add(const std::string & name, dataref_src_t source);
    int add(const std::vector<std::string> & names, dataref_src_t source);
    
    std::vector<DataRefRecord *> search(const std::string & search_term, bool regex, bool case_insensitive, bool changed_recently, bool only_big_changes);
    
    void update();
};
