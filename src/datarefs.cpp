#include "XPLMUtilities.h"

#include "datarefs.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <fstream>
#include <iostream>
#include <locale>
#include <regex>
#include <sstream>
#include <unordered_set>
#include <vector>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>

bool DataRefRecords::add(const std::string & name_untrimmed, dataref_src_t source) {
	std::string name = name_untrimmed;
	boost::algorithm::trim(name);

	//check for duplicates:
    NameMapType::iterator existing_location = dataref_ordered.find(name);
    if(dataref_ordered.cend() != existing_location) {
		return false;
	}

	if ("sim/aircraft/panel/acf_ins_size" == name) {
		return false;
	}

	XPLMDataRef dr = XPLMFindDataRef(name.c_str());
    if(nullptr == dr) {
		return false;
	}
	datarefs.emplace_back(name, dr, source);
    dataref_ordered.insert(std::make_pair(name, datarefs.cend() - 1));

	return true;
}

int DataRefRecords::add(const std::vector<std::string> & names, dataref_src_t source) {
	int ok = 0;
	std::string name;
	for(const std::string & name_untrimmed : names) {
		name = name_untrimmed;
		boost::algorithm::trim(name);

		//check for duplicates:
        NameMapType::iterator existing_location = dataref_ordered.find(name);
        if(dataref_ordered.cend() != existing_location) {
			continue;
		}

		if ("sim/aircraft/panel/acf_ins_size" == name) {
			continue;
		}

		XPLMDataRef dr = XPLMFindDataRef(name.c_str());
        if(nullptr == dr) {
			continue;
		}
        datarefs.emplace_back(name, dr, source);
        dataref_ordered.insert(std::make_pair(name, datarefs.cend() - 1));
        ok++;
	}

	return ok;
}
void DataRefRecords::update() {
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	for(DataRefRecord & dr : datarefs) {
        if(false == dr.isBlacklisted()) {
            dr.update(now);
        }
	}
}

std::vector<DataRefRecord *> DataRefRecords::search(const std::string & search_term, bool regex, bool case_insensitive, bool changed_recently, bool only_big_changes) {

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::vector<std::regex> search_regexes;
    

	std::vector<std::string> search_terms;
    boost::split(search_terms, search_term, boost::is_any_of(" "));

    //erase empty search terms
    auto end_it = std::remove_if(search_terms.begin(), search_terms.end(), [](const std::string & s)-> bool { return s.empty(); });
    search_terms.erase(end_it, search_terms.end());

	if(regex) {
		search_regexes.reserve(search_terms.size());
		for(const std::string & st : search_terms) {
			try {
				search_regexes.emplace_back(st, std::regex::ECMAScript | std::regex::optimize | (case_insensitive ? std::regex::icase : std::regex::flag_type(0)));
			} catch (std::exception &) {
				std::cerr << "Search expression \"" << st << "\" isn't a valid regex." << std::endl;
                return {};
			}
		}
	}

	// Feels a bit messy using all these lambdas, and also I'm a bit skeptical if it all getting optimized well
	const auto string_search = [case_insensitive](const std::string & haystack, const std::vector<std::string> & needles) -> bool {

		const auto case_insensitive_comparator = [](const char ch1, const char ch2) -> bool { return ::toupper(ch1) == ::toupper(ch2); };

		for(const std::string & needle : needles) {
			std::string::const_iterator it;
			if(case_insensitive) {
				it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), case_insensitive_comparator);
			} else {
				it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end());
			}
			if(it == haystack.cend()) {
				return false;
			}
		}
		return true;
	};

	const auto regex_search = [](const std::string & haystack, const std::vector<std::regex> & regexes) -> bool {
		for(const std::regex & needle_regex : regexes) {
			if(false == std::regex_search(haystack.begin(), haystack.end(), needle_regex)) {
				return false;
			}
		}
		return true;
	};

	//actually perform the search
    std::vector<DataRefRecord *> data_out;
	for(DataRefRecord & record : datarefs) {
		const std::string & haystack = record.getName();

		//first deal with search term
		if(false == search_terms.empty()) {
			if(regex) {
				if(false == regex_search(haystack, search_regexes)) {
					continue;
				}
			} else {
				if(false == string_search(haystack, search_terms)) {
					continue;
				}
			}
		}

		if(changed_recently) {
			float timediff;
			if(only_big_changes) {
				timediff = float(std::chrono::duration_cast<std::chrono::seconds>(now - record.getLastBigUpdateTime()).count());
			} else {
				timediff = float(std::chrono::duration_cast<std::chrono::seconds>(now - record.getLastUpdateTime()).count());
			}
			if(timediff > 10.f) {
				continue;
			}
		}

		data_out.push_back(&record);
	}
    
    //sort results
    {
        auto dr_name_sort = [](const DataRefRecord * a, const DataRefRecord * b) -> bool {
            return a->getName() < b->getName();
        };
        std::sort(data_out.begin(), data_out.end(), dr_name_sort);
    }
    
    {
        std::string count_message = "Search found " + std::to_string(data_out.size()) + " results";
        XPLMDebugString(count_message.c_str());
    }
    
    return data_out;
}
