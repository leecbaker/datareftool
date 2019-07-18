#pragma once
#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <ostream>
#include <vector>
#include <string>
#include <unordered_set>
#include "XPLMDataAccess.h"

#include <boost/filesystem.hpp>

#include "dataref.h"
#include "commandref.h"

class SearchParams;

class SearchQuery {
    std::vector<RefRecord *> results;
public:
};

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

    std::vector<RefRecord *> new_refs_this_frame, changed_cr_this_frame;
    std::vector<std::string> new_datarefs_from_messages_this_frame;
protected:
    std::ostream & log;
public:
    RefRecords(std::ostream & log) : log(log) {}
    std::vector<RefRecord *> CHECK_RESULT_USED add(const std::vector<std::string> & names, ref_src_t source);

    /// @return changed refs
    std::vector<RefRecord *> updateValues();

    // For change detection
    void addUpdatedCommandThisFrame(CommandRefRecord * record) {
        changed_cr_this_frame.push_back(record);
    }

    template <class Iterator>
    void addNewRefsThisFrame(Iterator begin, Iterator end) {
        new_refs_this_frame.insert(new_refs_this_frame.end(), begin, end);
    }

    void addNewRefFromMessage(std::string s) {
        new_datarefs_from_messages_this_frame.push_back(std::move(s));
    }

    void update(void (* update_func)(const std::vector<RefRecord *> &, std::vector<RefRecord *> &, std::vector<RefRecord *> &));
    void saveToFile(const boost::filesystem::path & dataref_filename, const boost::filesystem::path & commandref_filename) const;

    const RecordPointerType & getAllCommandrefs() const { return cr_pointers; }
    const RecordPointerType & getAllDatarefs() const { return dr_pointers; }
};
