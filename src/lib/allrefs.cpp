
#include "allrefs.h"
#include "search.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <fstream>
#include <locale>
#include <optional>
#include <regex>
#include <unordered_set>
#include <vector>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

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

	auto sort_and_write_names = [this](std::vector<const std::string *> & names, const boost::filesystem::path & filename) -> bool {
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
			log << "Error writing file " << filename << "\n";
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
        	commandrefs.emplace_back(name, cr, source, *this);
			new_records.emplace_back(&commandrefs.back());
			cr_pointers.emplace_back(&commandrefs.back());
			ref_names_loaded.insert(name);
		}
	}

	return new_records;
}

const char * current_dr_name = nullptr;

void sig_handler(int) {
	XPLMDebugString("DataRefTool: Crash detected while reading dataref ");
	XPLMDebugString(current_dr_name);
	XPLMDebugString("\n");
	XPLMDebugString("Please add this dataref to X-Plane 11/Resources/plugins/drt_blacklist.txt to prevent this happening again.\n");
	exit(-1);
}

std::vector<RefRecord *> RefRecords::updateValues() {
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


void RefRecords::update() {
	std::vector<RefRecord *> changed_drs = updateValues();

	if(false == new_datarefs_from_messages_this_frame.empty()) {
		std::vector<RefRecord *> refs_from_msg = add(new_datarefs_from_messages_this_frame, ref_src_t::USER_MSG);

		log << "Loaded : " << new_datarefs_from_messages_this_frame.size() << " commands/datarefs from messages; " << refs_from_msg.size() << " are ok\n";
		new_datarefs_from_messages_this_frame.clear();
	}

	//eliminate duplicate CRs
	if(1 < changed_cr_this_frame.size()) {
		auto comparator = [](const RefRecord * a, const RefRecord * b) -> bool {
			return a->getName() < b->getName();
		};
		auto record_equal = [](const RefRecord * a, const RefRecord * b) -> bool {
			return a->getName() == b->getName();
		};
		std::sort(changed_cr_this_frame.begin(), changed_cr_this_frame.end(), comparator);
		auto new_end = std::unique(changed_cr_this_frame.begin(), changed_cr_this_frame.end(), record_equal);
		changed_cr_this_frame.erase(new_end, changed_cr_this_frame.end());
	}

	{	// remove inactive searches
		auto new_end = std::remove_if(result_records.begin(), result_records.end(), [](const std::weak_ptr<SearchResults> & r) -> bool { return r.expired(); });
		result_records.erase(new_end, result_records.cend());
	}

	for(std::weak_ptr<SearchResults> & results_weak : result_records) {
		std::shared_ptr<SearchResults> results = results_weak.lock();
		results->update(new_refs_this_frame, changed_cr_this_frame, changed_drs);
	}

	changed_cr_this_frame.clear();
	changed_drs.clear();
	new_refs_this_frame.clear();
}

std::shared_ptr<SearchResults> RefRecords::doSearch(SearchParams params) {
	std::shared_ptr<SearchResults> results = std::make_shared<SearchResults>(params, cr_pointers, dr_pointers);
	result_records.push_back(results);
	return results;
}
