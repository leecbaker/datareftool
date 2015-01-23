#include "XPLMUtilities.h"

#include "datarefs.h"
#include <vector>
#include <fstream>
#include <regex>
#include <iostream>
#include <locale>

std::vector<DataRefRecord> datarefs;

bool loadDatarefs() {
	char system_path_c[1000];
	XPLMGetSystemPath(system_path_c);

	std::string dr_path(system_path_c);

	const char * dir_sep = XPLMGetDirectorySeparator();

	dr_path += "Resources";
	dr_path += dir_sep;
	dr_path += "plugins";
	dr_path += dir_sep;
	dr_path += "DataRefs.txt";

	std::string dr_path_message = "Loading datarefs from path " + dr_path + "\n";
	XPLMDebugString(dr_path_message.c_str());

	std::ifstream dr_file;
	dr_file.open(dr_path);

	if(dr_file.bad()) {
		XPLMDebugString("DataRefs.txt file could not be loaded\n");
		return false;
	}

	std::string line;
	std::getline(dr_file, line);	//discard header
	while(std::getline(dr_file, line)) {
		size_t tab_offset = line.find('\t');
		if(tab_offset == std::string::npos) {
			continue;
		}

		line.erase(line.begin() + tab_offset, line.end());

		XPLMDataRef dr = XPLMFindDataRef(line.c_str());
		if(nullptr == dr) {
			continue;
		}
		datarefs.emplace_back(line, dr);
	}

	auto comparator = [](const DataRefRecord & a, const DataRefRecord & b)-> bool {
		return a.getName() < b.getName();
	};
	std::sort(datarefs.begin(), datarefs.end(), comparator);

	std::string dr_count_message = "Finished loading " + std::to_string(datarefs.size()) + " datarefs" + "\n";
	XPLMDebugString(dr_count_message.c_str());

	datarefUpdate();

	return false == datarefs.empty();
}

void cleanupDatarefs() {
	datarefs.clear();
}

void datarefUpdate() {
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	for(DataRefRecord & dr : datarefs) {
		dr.update(now);
	}
}

void doDatarefSearch(const std::string & search_term, bool regex, bool case_insensitive, bool changed_recently, std::vector<DataRefRecord *> & data_out) {

	std::cerr << "Doing search for " << search_term << std::endl;
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::regex search_regex;
	data_out.clear();
	if(regex) {
		try {
			search_regex = std::regex(search_term, std::regex::ECMAScript | std::regex::optimize | (case_insensitive ? std::regex::icase : std::regex::flag_type(0)));
		} catch (std::exception & e) {
			std::cerr << "Search expression isn't a valid regex." << std::endl;
			return;
		}
	}

	for(DataRefRecord & record : datarefs) {
		const std::string & haystack = record.getName();

		//first deal with search term
		if(false == search_term.empty()) {
			if(regex) {	//regex search
				if(false == std::regex_search(haystack.begin(), haystack.end(), search_regex))
					continue;

			} else {	//string search
				if(case_insensitive) {
					auto it = std::search(
				    	haystack.begin(), haystack.end(),
				    	search_term.begin(),   search_term.end(),
				    	[](char ch1, char ch2) { return ::toupper(ch1) == ::toupper(ch2); }
				  	);
					if (it == haystack.end() ) {
						continue;
					}
				} else {
					if(haystack.find(search_term) == std::string::npos) {
						continue;
					}
				}
			}
		}

		if(changed_recently) {
			float timediff = std::chrono::duration_cast<std::chrono::seconds>(now - record.getLastUpdated()).count();
			if(timediff > 10.) {
				continue;
			}
		}

		data_out.push_back(&record);
	}
}

bool DataRefRecord::update(const std::chrono::system_clock::time_point current_time) {
	if(type & xplmType_Double) {
		double newval = XPLMGetDatad(ref);
		if(newval != lf_val) {
			last_updated = current_time;
			lf_val = newval;
			return true;
		} else {
			return false;
		}
	} else if (type & xplmType_Float) {
		float newval = XPLMGetDataf(ref);
		if(newval != f_val) {
			last_updated = current_time;
			f_val = newval;
			return true;
		} else {
			return false;
		}
	} else if (type & xplmType_Int) {
		int newval = XPLMGetDatai(ref);
		if(newval != i_val) {
			last_updated = current_time;
			i_val = newval;
			return true;
		} else {
			return false;
		}
	} else if (type & xplmType_FloatArray) {
		float data[PREVIEW_DATAREF_ARRAY_COUNT];	
		int copied = XPLMGetDatavf(ref, data, 0, PREVIEW_DATAREF_ARRAY_COUNT);
		if(copied == PREVIEW_DATAREF_ARRAY_COUNT) {
			if(0 != memcmp(data, fv_val, sizeof(*data) * PREVIEW_DATAREF_ARRAY_COUNT)) {
				last_updated = current_time;
				for(int i = 0; i < PREVIEW_DATAREF_ARRAY_COUNT; i++) {
					fv_val[i] = data[i];
				}
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	} else if (type & xplmType_IntArray) {
		int data[PREVIEW_DATAREF_ARRAY_COUNT];	
		int copied = XPLMGetDatavi(ref, data, 0, PREVIEW_DATAREF_ARRAY_COUNT);
		if(copied == PREVIEW_DATAREF_ARRAY_COUNT) {
			if(0 != memcmp(data, iv_val, sizeof(*data) * PREVIEW_DATAREF_ARRAY_COUNT)) {
				last_updated = current_time;
				for(int i = 0; i < PREVIEW_DATAREF_ARRAY_COUNT; i++) {
					iv_val[i] = data[i];
				}
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	} else if (type & xplmType_Data) {
		char data[PREVIEW_DATAREF_BYTEARRAY_COUNT];	
		int copied = XPLMGetDatab(ref, data, 0, PREVIEW_DATAREF_BYTEARRAY_COUNT);
		if(copied == PREVIEW_DATAREF_BYTEARRAY_COUNT) {
			if(0 != memcmp(data, iv_val, sizeof(*data) * PREVIEW_DATAREF_BYTEARRAY_COUNT)) {
				last_updated = current_time;
				for(int i = 0; i < PREVIEW_DATAREF_BYTEARRAY_COUNT; i++) {
					b_val[i] = data[i];
				}
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	} else {
		last_updated = current_time;
		return false;
	}
}

std::string DataRefRecord::getDisplayString() const {
	return getName() + "=" + getValueString();
}

template <class T>
std::string compactFpString(T f) {
	std::string ret = std::to_string(f);

	while(false == ret.empty() && ret.back() == '0') {
		ret.erase(ret.end() - 1);
	}

	return ret;
}

std::string printableFromByteArray(char * bytes) {
	std::string ret;
	for(int i = 0; i < PREVIEW_DATAREF_BYTEARRAY_COUNT; i++) {
		if(bytes[i] == 0) {
			break;
		} else if(::isprint(bytes[i])) {
			ret.push_back(bytes[i]);
		} else {
			break;
		}
	}

	return ret;
}

std::string DataRefRecord::getValueString() const {
	if(type & xplmType_Double) {
		return compactFpString(lf_val);
	} else if (type & xplmType_Float) {
		return compactFpString(f_val);
	} else if (type & xplmType_Int) {
		return std::to_string(i_val);
	} else if (type & xplmType_FloatArray) {
		return "[" + compactFpString(fv_val[0]) + "," + compactFpString(fv_val[1]) + "," + compactFpString(fv_val[2]) + "," + compactFpString(fv_val[3]) + "...]";
	} else if (type & xplmType_IntArray) {
		return "[" + std::to_string(iv_val[0]) + "," + std::to_string(iv_val[1]) + "," + std::to_string(iv_val[2]) + "," + std::to_string(iv_val[3]) + "...]";
	} else if (type & xplmType_Data) {
		return "\"" + printableFromByteArray((char *)b_val) + "\"";
	} else {
		return "--";
	}
}

