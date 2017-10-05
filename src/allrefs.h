#pragma once
#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <vector>
#include <string>
#include <unordered_set>
#include "XPLMDataAccess.h"

#include <boost/filesystem.hpp>

#include "dataref.h"
#include "commandref.h"

class SearchParams;

//Store all commandrefs and datarefs here

class RefRecords {
    using DatarefStorageType = std::deque<DataRefRecord>;
    using CommandrefStorageType = std::deque<CommandRefRecord>;
    using RecordPointerType = std::vector<RefRecord *>;
    using NameMapType = std::unordered_set<std::string>;

    DatarefStorageType datarefs;
    CommandrefStorageType commandrefs;
    RecordPointerType cr_pointers;
    RecordPointerType dr_pointers;
    NameMapType ref_names_loaded;

	DataRefUpdater updater;
public:
    std::vector<RefRecord *> CHECK_RESULT_USED add(const std::vector<std::string> & names, ref_src_t source);

    std::vector<RefRecord *> update();
    bool saveToFile(const boost::filesystem::path & dataref_filename, const boost::filesystem::path & commandref_filename) const;

    const RecordPointerType & getAllCommandrefs() const { return cr_pointers; }
    const RecordPointerType & getAllDatarefs() const { return dr_pointers; }
};
