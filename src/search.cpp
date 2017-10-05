#include "search.h"

#include "logging.h"

#include <boost/algorithm/string.hpp>

#include <iostream>

void SearchParams::setSearchTerms(const std::string & terms) {
    search_field_ = terms;
    //split search terms, and remove empty ones
    boost::split(search_terms_, terms, boost::is_any_of(" "));
    auto end_it = std::remove_if(search_terms_.begin(), search_terms_.end(), [](const std::string & s)-> bool { return s.empty(); });
    search_terms_.erase(end_it, search_terms_.end());

    regex_fail_ = false;
    if(useRegex()) {
        buildRegex();
    } else {
        search_regexes_.clear();
    }
}

void SearchParams::buildRegex() {
    regex_fail_ = false;
    search_regexes_.clear();
    search_regexes_.reserve(search_terms_.size());
    for(const std::string & st : search_terms_) {
        try {
            search_regexes_.emplace_back(st, std::regex::ECMAScript | std::regex::optimize | (case_sensitive_ ? std::regex::flag_type(0) : std::regex::icase));
        } catch (std::exception &) {
            search_regexes_.clear();
            regex_fail_ = true;
        }
    }
}

bool SearchParams::filterRecord(const RefRecord * record, const std::chrono::system_clock::time_point now) {
    // Feels a bit messy using all these lambdas, and also I'm a bit skeptical if it all getting optimized well
    const auto string_search = [this](const std::string & haystack) -> bool {
        if(case_sensitive_) {
            const auto case_sensitive_single_search = [&haystack, this](const std::string & needle) -> bool {
                return haystack.cend() != std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end());
            };
            return std::all_of(search_terms_.cbegin(), search_terms_.cend(), case_sensitive_single_search);
        } else {
            const auto case_insensitive_comparator = [](const char ch1, const char ch2) -> bool { return ::toupper(ch1) == ::toupper(ch2); };
            const auto case_insensitive_single_search = [&case_insensitive_comparator, &haystack](const std::string & needle) -> bool {
                return haystack.cend() != std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), case_insensitive_comparator);
            };
            return std::all_of(search_terms_.cbegin(), search_terms_.cend(), case_insensitive_single_search);
        }
    };

    const auto regex_search = [this](const std::string & haystack) -> bool {
        auto regex_single_search = [&haystack](const std::regex & r) -> bool {
            return std::regex_search(haystack.cbegin(), haystack.cend(), r);
        };
        return std::all_of(search_regexes_.cbegin(), search_regexes_.cend(), regex_single_search);
    };

    // Deal with update time first, as this will eliminate most regexes / string compares when it can
    if(change_detection_) {
        float timediff;
        if(only_large_changes_) {
            timediff = float(std::chrono::duration_cast<std::chrono::seconds>(now - record->getLastBigUpdateTime()).count());
        } else {
            timediff = float(std::chrono::duration_cast<std::chrono::seconds>(now - record->getLastUpdateTime()).count());
        }
        if(timediff > 10.f) {
            return false;
        }
    }

    const std::string & haystack = record->getName();

    if(false == search_terms_.empty()) {
        if(use_regex_) {
            if(false == regex_search(haystack)) {
                return false;
            }
        } else {
            if(false == string_search(haystack)) {
                return false;
            }
        }
    }

    return true;
}

std::vector<RefRecord *> SearchParams::freshSearch(const std::vector<RefRecord *> & commandrefs, const std::vector<RefRecord *> & datarefs) {
    std::vector<RefRecord *> results;
    std::back_insert_iterator<std::vector<RefRecord *>> result_inserter = std::back_inserter(results);

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    //actually perform the search
    if(include_drs_) {
        std::copy_if(datarefs.cbegin(), datarefs.cend(), result_inserter, [this, now](const RefRecord * r) -> bool { return filterRecord(r, now); });
    }

    if(include_crs_) {
        std::copy_if(commandrefs.cbegin(), commandrefs.cend(), result_inserter, [this, now](const RefRecord * r) -> bool { return filterRecord(r, now); });
    }

    sort(results);
    
    return results;
}

std::vector<RefRecord *> SearchParams::updateSearch(const std::vector<RefRecord *> & existing_results, const std::vector<RefRecord *> & new_refs, const std::vector<RefRecord *> & changed_cr, const std::vector<RefRecord *> & changed_dr) {
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    if(change_detection_) {
        std::vector<RefRecord *> results;
        std::back_insert_iterator<std::vector<RefRecord *>> result_inserter = std::back_inserter(results);
        results.reserve(existing_results.size());

        //filter existing results by change date and not search term
        std::copy_if(existing_results.cbegin(), existing_results.cend(), result_inserter, [this, now](const RefRecord * r) -> bool { return filterRecord(r, now); });

        //changed dr/cr filter by search term. Probably can assume change is ok
        {
            working_buffer.clear();
            std::back_insert_iterator<std::vector<RefRecord *>> working_inserter = std::back_inserter(working_buffer);
            if(include_drs_) {
                std::copy_if(changed_dr.cbegin(), changed_dr.cend(), working_inserter, [this, now](const RefRecord * r) -> bool { return filterRecord(r, now); });
            }
            if(include_crs_) {
                std::copy_if(changed_cr.cbegin(), changed_cr.cend(), working_inserter, [this, now](const RefRecord * r) -> bool { return filterRecord(r, now); });
            }
            sort(working_buffer);
            results = merge(working_buffer, results);
        }

        //new refs, filter by search term and by update date
        {
            working_buffer.clear();
            std::back_insert_iterator<std::vector<RefRecord *>> working_inserter = std::back_inserter(working_buffer);
            std::copy_if(new_refs.cbegin(), new_refs.cend(), working_inserter, [this, now](const RefRecord * r) -> bool { return filterRecord(r, now); });
            sort(working_buffer);
            results = merge(working_buffer, results);
        }

        return results;
    } else {
        //common case- if no new refs, we can just return existing results
        if(new_refs.empty()) {
            return existing_results;
        }

        //keep existing results
        //filter new refs by search term only
        {
            working_buffer.clear();
            std::back_insert_iterator<std::vector<RefRecord *>> working_inserter = std::back_inserter(working_buffer);
            std::copy_if(new_refs.cbegin(), new_refs.cend(), working_inserter, [this, now](const RefRecord * r) -> bool { return filterRecord(r, now); });
            sort(working_buffer);
        }

        return merge(existing_results, working_buffer);
    }
}