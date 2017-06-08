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
#include "commandref.h"

//Store all commandrefs and datarefs here

class RefRecords {
    using DatarefStorageType = std::vector<DataRefRecord>;
    using CommandrefStorageType = std::vector<CommandRefRecord>;
    using RecordPointerType = std::vector<RefRecord *>;
    using NameMapType = std::unordered_multimap<std::string, RecordPointerType::const_iterator>;

    DatarefStorageType datarefs;
    CommandrefStorageType commandrefs;
    RecordPointerType ref_pointers;
    NameMapType refs_ordered;
public:
    int add(const std::vector<std::string> & names, ref_src_t source);
    
    std::vector<RefRecord *> search(const std::string & search_term, bool regex, bool case_insensitive, bool changed_recently, bool only_big_changes, bool include_drs, bool include_crs);
    
    void update();
    bool saveToFile(const boost::filesystem::path & dataref_filename, const boost::filesystem::path & commandref_filename) const;
};
