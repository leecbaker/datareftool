#pragma once

#include <algorithm>
#include <chrono>
#include <cstring>
#include <regex>
#include <vector>

#include <locale>
#include <boost/algorithm/string/compare.hpp>

#include "ref.h"

class SearchParams {
    std::vector<std::string> search_terms_;
	std::vector<std::regex> search_regexes_;
    std::string search_field_;
    bool use_regex_ = false, case_sensitive_ = false, change_detection_ = false, only_large_changes_ = false, include_drs_ = false, include_crs_ = false;

    bool regex_fail_ = false;

    bool filterByName(const RefRecord * record);
    bool filterByTime(const RefRecord * record, const std::chrono::system_clock::time_point now);
    bool filterByTimeAndName(const RefRecord * record, const std::chrono::system_clock::time_point now) {
        // filter by time first, as it's the cheapest
        return filterByTime(record, now) && filterByName(record);
    }

    std::vector<RefRecord *> working_buffer;
    void buildRegex();
public:
    // Sort an unsorted list
    void sort(std::vector<RefRecord *> & records) {
        std::sort(records.begin(), records.end(), nameComparator);
    }
    std::vector<RefRecord *> merge(const std::vector<RefRecord *> & a, const std::vector<RefRecord *> & b) {
        std::vector<RefRecord *> merged;
        merged.reserve(a.size() + b.size());
        std::set_union(a.cbegin(), a.cend(), b.cbegin(), b.cend(), std::back_inserter(merged), nameComparator);
        return merged;
    }

    // For a completely fresh search
    std::vector<RefRecord *> freshSearch(const std::vector<RefRecord *> & commandrefs, const std::vector<RefRecord *> & datarefs);

    // Filter by modification time. Needed for searches by modification to be updated in real time
    std::vector<RefRecord *> updateSearch(const std::vector<RefRecord *> & existing_results, const std::vector<RefRecord *> & new_refs, const std::vector<RefRecord *> & changed_cr, const std::vector<RefRecord *> & changed_dr);

    bool useChangeDetection() const { return change_detection_; }
    bool useOnlyLargeChanges() const { return only_large_changes_; }
    bool includeDatarefs() const { return include_drs_; }
    bool includeCommandrefs() const { return include_crs_; }
    bool isCaseSensitive() const { return case_sensitive_; }
    bool useRegex() const { return use_regex_; }
    const std::string & getSearchField() const { return search_field_; }
    bool invalidRegex() const { return regex_fail_; }

    void setSearchTerms(const std::string & terms);
    void setChangeDetection(bool detection_on, bool only_big) { change_detection_ = detection_on; only_large_changes_ = only_big; }
    void setIncludeRefs(bool include_crs, bool include_drs) { include_crs_ = include_crs; include_drs_ = include_drs; }
    void setCaseSensitive(bool sensitive) { case_sensitive_ = sensitive; }
    void setUseRegex(bool use_regex) { 
        regex_fail_ = false;
        use_regex_ = use_regex; 
        if(use_regex_) {
            buildRegex();
        }
    }

protected:
    inline static bool nameComparator(const RefRecord * a, const RefRecord * b) {
        const std::string & lhs = a->getName();
        const std::string & rhs = b->getName();
        
        const auto result = mismatch(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), [](const auto& lhs, const auto& rhs){
            return tolower(lhs) == tolower(rhs);
        });
    
        return result.second != rhs.cend() && (result.first == lhs.cend() || tolower(*result.first) < tolower(*result.second));
    }
};
