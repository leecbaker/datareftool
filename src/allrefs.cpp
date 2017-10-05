
#include "allrefs.h"
#include "logging.h"
#include "search.h"

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

bool RefRecords::saveToFile(const boost::filesystem::path & dataref_filename, const boost::filesystem::path & commandref_filename) const {
	std::vector<const std::string *> dataref_names, commandref_names;
	dataref_names.reserve(datarefs.size());
	for(const DataRefRecord & dr : datarefs) {
		dataref_names.push_back(&dr.getName());
	}

	commandref_names.reserve(commandrefs.size());
	for(const CommandRefRecord & cr : commandrefs) {
		commandref_names.push_back(&cr.getName());
	}

	auto sort_and_write_names = [](std::vector<const std::string *> & names, const boost::filesystem::path & filename) -> bool {
		auto p_str_comparator = [](const std::string * s1, const std::string * s2) -> bool {
			return boost::ilexicographical_compare(*s1, *s2);
		};
		std::sort(names.begin(), names.end(), p_str_comparator);

		std::ofstream f(filename.string());
		for(const std::string * pstr : names) {
			f << *pstr << "\n";
		}
		f.close();
		return f.fail();
	};

	return sort_and_write_names(dataref_names, dataref_filename) && sort_and_write_names(commandref_names, commandref_filename);
}

std::vector<RefRecord *> RefRecords::add(const std::vector<std::string> & names, ref_src_t source) {
	std::string name;
	std::vector<RefRecord *> new_records;
	for(const std::string & name_untrimmed : names) {
		name = name_untrimmed;
		boost::algorithm::trim(name);

		//check for duplicates:
        NameMapType::iterator existing_location = ref_names_loaded.find(name);
        if(ref_names_loaded.cend() != existing_location) {
			continue;
		}

		XPLMDataRef dr = XPLMFindDataRef(name.c_str());
		XPLMCommandRef cr = XPLMFindCommand(name.c_str());
        if(nullptr != dr) {
        	datarefs.emplace_back(name, dr, source);
			new_records.emplace_back(&datarefs.back());
			dr_pointers.emplace_back(&datarefs.back());
			ref_names_loaded.insert(name);
		}

		if(nullptr != cr) {
        	commandrefs.emplace_back(name, cr, source);
			new_records.emplace_back(&commandrefs.back());
			cr_pointers.emplace_back(&commandrefs.back());
			ref_names_loaded.insert(name);
		}
	}

	// When adding more datarefs and commandrefs, it's possible that the storage vector
	// may need to reallocate. This means pointers to the elements may be invalidated, a

	{
		char system_path_c[1000];
		XPLMGetSystemPath(system_path_c);
		boost::filesystem::path system_path(system_path_c);
		boost::filesystem::path output_dir = system_path / "Output" / "preferences";

		saveToFile(output_dir / "drt_last_run_datarefs.txt", output_dir / "drt_last_run_commandrefs.txt");
	}

	return new_records;
}

std::vector<RefRecord *> RefRecords::update() {
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	updater.updateTime(now);
	std::vector<RefRecord *> changed_drs;
	for(DataRefRecord & dr : datarefs) {
        if(false == dr.isBlacklisted()) {
            if(dr.update(updater)) { //if updated
				changed_drs.push_back(&dr);
			}
        }
	}
	return changed_drs;
}
