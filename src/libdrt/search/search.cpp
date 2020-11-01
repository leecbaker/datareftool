#include "search.h"

#include <boost/algorithm/string.hpp>

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

bool SearchParams::filterByName(const RefRecord * record) {
    // Feels a bit messy using all these lambdas, and also I'm a bit skeptical if it all getting optimized well
    const auto string_search = [this](const std::string & haystack) -> bool {
        if(case_sensitive_) {
            const auto case_sensitive_single_search = [&haystack](const std::string & needle) -> bool {
                if('-' == needle.front()) {
                    return haystack.cend() == std::search(haystack.begin(), haystack.end(), needle.begin() + 1, needle.end());
                } else {
                    return haystack.cend() != std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end());
                }
            };
            return std::all_of(search_terms_.cbegin(), search_terms_.cend(), case_sensitive_single_search);
        } else {
            const auto case_insensitive_comparator = [](const char ch1, const char ch2) -> bool { return ::toupper(ch1) == ::toupper(ch2); };
            const auto case_insensitive_single_search = [&case_insensitive_comparator, &haystack](const std::string & needle) -> bool {
                if('-' == needle.front()) {
                    return haystack.cend() == std::search(haystack.begin(), haystack.end(), needle.begin() + 1, needle.end(), case_insensitive_comparator);
                } else {
                    return haystack.cend() != std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), case_insensitive_comparator);
                }
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

bool SearchParams::filterByTime(const RefRecord * record, const std::chrono::system_clock::time_point now) {
    if(!change_detection_) {
        return true;
    }

    float timediff;
    if(only_large_changes_) {
        timediff = float(std::chrono::duration_cast<std::chrono::seconds>(now - record->getLastBigUpdateTime()).count());
    } else {
        timediff = float(std::chrono::duration_cast<std::chrono::seconds>(now - record->getLastUpdateTime()).count());
    }
    if(timediff > 10.f) {
        return false;
    }

    return true;
}

void SearchParams::freshSearch(std::vector<RefRecord *> & results_out, const std::vector<RefRecord *> & commandrefs, const std::vector<RefRecord *> & datarefs) {
    std::back_insert_iterator<std::vector<RefRecord *>> result_inserter = std::back_inserter(results_out);
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    results_out.clear();

    //actually perform the search
    if(include_drs_) {
        std::copy_if(datarefs.cbegin(), datarefs.cend(), result_inserter, [this, now](const RefRecord * r) -> bool { return filterByTimeAndName(r, now); });
    }

    if(include_crs_) {
        std::copy_if(commandrefs.cbegin(), commandrefs.cend(), result_inserter, [this, now](const RefRecord * r) -> bool { return filterByTimeAndName(r, now); });
    }

    sort(results_out);
}

std::chrono::system_clock::time_point SearchParams::updateSearch(std::vector<RefRecord *> & results_in_out, const std::vector<RefRecord *> & new_refs, const std::vector<RefRecord *> & changed_cr, const std::vector<RefRecord *> & changed_dr) {
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    if(change_detection_) {
        //filter existing results by change date and not search term
        /*
        auto new_end = std::remove_if(results_in_out.begin(), results_in_out.end(), [this, now](const RefRecord * r) -> bool { return false == filterByTime(r, now); });
        results_in_out.erase(new_end, results_in_out.end());
        */

        //changed dr/cr filter by search term, and possibly filter out small changes.
        {
            auto filter = [this, now](const RefRecord * r) -> bool {
                if(useOnlyLargeChanges()) { // need additional filtering for only bigger changes then
                    return filterByTimeAndName(r, now);
                } else {
                    return filterByName(r);
                }
            };
            working_buffer.clear();
            std::back_insert_iterator<std::vector<RefRecord *>> working_inserter = std::back_inserter(working_buffer);
            if(include_drs_) {
                std::copy_if(changed_dr.cbegin(), changed_dr.cend(), working_inserter, filter);
            }
            if(include_crs_) {
                std::copy_if(changed_cr.cbegin(), changed_cr.cend(), working_inserter, filter);
            }
            sort(working_buffer);
            inplace_union(results_in_out, working_buffer);
        }

        //new refs, filter by search term and by update date
        if(false == new_refs.empty()) {
            working_buffer.clear();
            std::back_insert_iterator<std::vector<RefRecord *>> working_inserter = std::back_inserter(working_buffer);
            std::copy_if(new_refs.cbegin(), new_refs.cend(), working_inserter, [this, now](const RefRecord * r) -> bool { return filterByTimeAndName(r, now); });
            sort(working_buffer);
            inplace_union(results_in_out, working_buffer);
        }
    } else {
        //keep existing results
        //filter new refs by search term only
        if(false == new_refs.empty()) {
            working_buffer.clear();
            std::back_insert_iterator<std::vector<RefRecord *>> working_inserter = std::back_inserter(working_buffer);
            std::copy_if(new_refs.cbegin(), new_refs.cend(), working_inserter, [this, now](const RefRecord * r) -> bool { return filterByTimeAndName(r, now); });
            sort(working_buffer);
            inplace_union(results_in_out, working_buffer);
        }
    }

    return now;
}

SearchResults::SearchResults(SearchParams params, const std::vector<RefRecord *> & commandrefs, const std::vector<RefRecord *> & datarefs) : params(params) {
    params.freshSearch(refs, commandrefs, datarefs);
}

void SearchResults::update(const std::vector<RefRecord *> & new_refs, const std::vector<RefRecord *> & changed_cr, const std::vector<RefRecord *> & changed_dr) {
    last_update_timestamp = params.updateSearch(refs, new_refs, changed_cr, changed_dr);
}
