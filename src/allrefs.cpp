
#include "allrefs.h"
#include "logging.h"
#include "search.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <fstream>
#include <locale>
#include <regex>
#include <sstream>
#include <unordered_set>
#include <vector>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>

#include <signal.h>

void RefRecords::saveToFile(const boost::filesystem::path & dataref_filename, const boost::filesystem::path & commandref_filename) const {
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
		if(f.fail()) {
			LOG("Error writing file " + filename.string());
		}
		return f.fail();
	};

	sort_and_write_names(dataref_names, dataref_filename);
	sort_and_write_names(commandref_names, commandref_filename);
}

std::vector<RefRecord *> RefRecords::add(const std::vector<std::string> & names, ref_src_t source) {
	std::string name;
	std::vector<RefRecord *> new_records;
	for(const std::string & name_untrimmed : names) {
		name = name_untrimmed;
		boost::algorithm::trim(name);

		//check for duplicates:
        NameMapType::const_iterator existing_location = ref_names_loaded.find(name);
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

	return new_records;
}

const char * current_dr_name = nullptr;

void sig_handler(int) {
	XPLMDebugString("DRT: Crash detected while reading dataref ");
	XPLMDebugString(current_dr_name);
	XPLMDebugString("\n");
	XPLMDebugString("Please add this dataref to X-Plane 11/Resources/plugins/drt_blacklist.txt to prevent this happening again.\n");
	exit(-1);
}

std::vector<RefRecord *> RefRecords::update() {
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	updater.updateTime(now);
	std::vector<RefRecord *> changed_drs;

#if IBM
    typedef void (*sig_t)(int);
#endif

	sig_t previous_sigsegv = signal(SIGSEGV, sig_handler);
	sig_t previous_sigfpe = signal(SIGFPE, sig_handler);

	for(DataRefRecord & dr : datarefs) {
        if(false == dr.isBlacklisted()) {
			current_dr_name = dr.getName().c_str();

            if(dr.update(updater)) { //if updated
				changed_drs.push_back(&dr);
			}
        }
	}

	signal(SIGSEGV, previous_sigsegv);
	signal(SIGFPE, previous_sigfpe);
	current_dr_name = nullptr;

	return changed_drs;
}
