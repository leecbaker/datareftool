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
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string.hpp>

std::vector<DataRefRecord> datarefs;
std::unordered_set<std::string> datarefs_loaded;	//check for duplicates

DataRefRecord::DataRefRecord(const std::string & name, XPLMDataRef ref, dataref_src_t source) : name(name), last_updated(std::chrono::system_clock::now()), ref(ref), source(source) {
	type = 	XPLMGetDataRefTypes(ref);
	iv_val.fill(0);

	updateArrayLength();
}

void DataRefRecord::updateArrayLength() {
	//determine array length by assuming memory past the end of the array won't be written
	if(isArray()) {
		const size_t test_array_elements = 4095;
		const size_t test_array_bytes = test_array_elements * sizeof(int);
		static int test_array[test_array_elements];
		
		if(xplmType_IntArray & type) {
			array_length = XPLMGetDatavi(ref, test_array, 0, test_array_elements);
		} else if(xplmType_FloatArray & type) {
			array_length = XPLMGetDatavf(ref, (float *)test_array, 0, test_array_elements);
		} else if(xplmType_Data & type) {
			array_length = XPLMGetDatab(ref, (char *)test_array, 0, test_array_bytes);
		}
	}
}

bool DataRefRecord::writable() const {
	if(xplmType_Int != type && xplmType_Float != type && xplmType_Double != type) {
		return false;
	}

	if(0 == XPLMCanWriteDataRef(ref)) {
		return false;
	}

	return true;
}

void sortDatarefs() {
	auto comparator = [](const DataRefRecord & a, const DataRefRecord & b)-> bool {
		return a.getName() < b.getName();
	};
	std::sort(datarefs.begin(), datarefs.end(), comparator);
}

bool addUserDataref(const std::string & name_untrimmed, dataref_src_t source) {
	std::string name = name_untrimmed;
	boost::algorithm::trim(name);

	//check for duplicates:
	if(0 != datarefs_loaded.count(name)) {
		return false;
	}

	XPLMDataRef dr = XPLMFindDataRef(name.c_str());
	if(nullptr == dr) {
		return false;
	}
	datarefs.emplace_back(name, dr, source);
	datarefs_loaded.insert(name);
	sortDatarefs();

	return true;
}

int addUserDatarefs(const std::vector<std::string> & names, dataref_src_t source) {
	int ok = 0;
	std::string name;
	for(const std::string & name_untrimmed : names) {
		name = name_untrimmed;
		boost::algorithm::trim(name);

		//check for duplicates:
		if(0 != datarefs_loaded.count(name)) {
			continue;
		}

		XPLMDataRef dr = XPLMFindDataRef(name.c_str());
		if(nullptr == dr) {
			continue;
		}
		datarefs.emplace_back(name, dr, source);
		datarefs_loaded.insert(name);
		ok++;
	}

	sortDatarefs();

	return ok;
}

bool loadDatarefsFile() {
	char system_path_c[1000];
	XPLMGetSystemPath(system_path_c);

	std::string dr_path(system_path_c);

	const char * dir_sep = XPLMGetDirectorySeparator();

	dr_path += "Resources";
	dr_path += dir_sep;
	dr_path += "plugins";
	dr_path += dir_sep;
	dr_path += "DataRefs.txt";

	std::string dr_path_message = "DRT: Loading datarefs from path " + dr_path + "\n";
	XPLMDebugString(dr_path_message.c_str());

	std::ifstream dr_file;
	dr_file.open(dr_path);

	if(dr_file.bad()) {
		XPLMDebugString("DRT: DataRefs.txt file could not be loaded\n");
		return false;
	}

	//remove existing datarefs from file:
	auto is_from_file = [](const DataRefRecord & record) -> bool {
		return record.getSource() == dataref_src_t::FILE;
	};

	datarefs.erase(std::remove_if(datarefs.begin(), datarefs.end(), is_from_file), datarefs.end());

	std::string line;
	std::getline(dr_file, line);	//discard header
	while(std::getline(dr_file, line)) {
		size_t tab_offset = line.find('\t');
		if(tab_offset == std::string::npos) {
			continue;
		}

		line.erase(line.begin() + tab_offset, line.end());

		addUserDataref(line, dataref_src_t::FILE);
	}

	sortDatarefs();

	std::string dr_count_message = "DRT: Finished loading " + std::to_string(datarefs.size()) + " datarefs" + "\n";
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

void doDatarefSearch(const std::string & search_term, bool regex, bool case_insensitive, bool changed_recently, bool only_big_changes, std::vector<DataRefRecord *> & data_out) {

	std::cerr << "Doing search for " << search_term << std::endl;
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::vector<std::regex> search_regexes;

	std::vector<std::string> search_terms;
    boost::split(search_terms, search_term, boost::is_any_of(" "));

    //erase empty search terms
    auto end_it = std::remove_if(search_terms.begin(), search_terms.end(), [](const std::string & s)-> bool { return s.empty(); });
    search_terms.erase(end_it, search_terms.end());

	data_out.clear();
	if(regex) {
		search_regexes.reserve(search_terms.size());
		for(const std::string & st : search_terms) {
			try {
				search_regexes.emplace_back(st, std::regex::ECMAScript | std::regex::optimize | (case_insensitive ? std::regex::icase : std::regex::flag_type(0)));
			} catch (std::exception &) {
				std::cerr << "Search expression \"" << st << "\" isn't a valid regex." << std::endl;
				return;
			}
		}
	}

	// Feels a bit messy using all these lambdas, and also I'm a bit skeptical if it all getting optimized well
	const auto case_insensitive_comparator = [](char ch1, char ch2) { return ::toupper(ch1) == ::toupper(ch2); };
	const auto case_sensitive_comparator = [](char ch1, char ch2) { return ch1 == ch2; };
	const auto str_comparator = case_insensitive ? case_insensitive_comparator : case_sensitive_comparator;

	const auto string_search = [str_comparator](const std::string & haystack, const std::vector<std::string> & needles) -> bool {
		for(const std::string & needle : needles) {
			const auto it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(), str_comparator);
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

		record.updateArrayLength();
		data_out.push_back(&record);
	}
}

bool DataRefRecord::update(const std::chrono::system_clock::time_point current_time) {
	if(type & xplmType_Double) {
		double newval = XPLMGetDatad(ref);
		if(newval != lf_val) {
			if(0.01 < fabsl(newval - lf_val) / lf_val) {
				last_updated_big = current_time;
			}
			last_updated = current_time;
			lf_val = newval;
			return true;
		} else {
			return false;
		}
	} else if (type & xplmType_Float) {
		float newval = XPLMGetDataf(ref);
		if(newval != f_val) {
			if(0.01f < fabs(newval - f_val) / f_val) {
				last_updated_big = current_time;
			}
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
		static std::vector<float> scratch_buffer;
		scratch_buffer.resize(array_length);
		int copied = XPLMGetDatavf(ref, scratch_buffer.data(), 0, array_length);
		if(copied == array_length) {
			size_t new_hash = boost::hash_range(scratch_buffer.cbegin(), scratch_buffer.cend());
			if(new_hash != array_hash) {
				array_hash = new_hash;
				last_updated = current_time;
				size_t copy_length =  std::min(PREVIEW_DATAREF_ARRAY_COUNT, copied);
				std::copy(scratch_buffer.cbegin(), scratch_buffer.cbegin() + copy_length, fv_val.begin());
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	} else if (type & xplmType_IntArray) {
		static std::vector<int> scratch_buffer;
		scratch_buffer.resize(array_length);
		int copied = XPLMGetDatavi(ref, scratch_buffer.data(), 0, array_length);
		if(copied == array_length) {
			size_t new_hash = boost::hash_range(scratch_buffer.cbegin(), scratch_buffer.cend());
			if(new_hash != array_hash) {
				array_hash = new_hash;
				last_updated = current_time;
				size_t copy_length =  std::min(PREVIEW_DATAREF_ARRAY_COUNT, copied);
				std::copy(scratch_buffer.cbegin(), scratch_buffer.cbegin() + copy_length, iv_val.begin());
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	} else if (type & xplmType_Data) {
		static std::vector<uint8_t> scratch_buffer;
		scratch_buffer.resize(array_length);
		int copied = XPLMGetDatab(ref, scratch_buffer.data(), 0, array_length);
		if(copied == array_length) {
			size_t new_hash = boost::hash_range(scratch_buffer.cbegin(), scratch_buffer.cend());
			if(new_hash != array_hash) {
				array_hash = new_hash;
				last_updated = current_time;
				size_t copy_length =  std::min(PREVIEW_DATAREF_ARRAY_COUNT, copied);
				std::copy(scratch_buffer.cbegin(), scratch_buffer.cbegin() + copy_length, b_val.begin());
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
	std::string ret = getName();
	if(isArray()) {
		ret += "[" + std::to_string(array_length) + "]";
	}
	ret += "=" + getValueString();
	return ret;
}

template <class T>
std::string compactFpString(T f) {
	std::string ret = std::to_string(f);

	while(false == ret.empty() && ret.back() == '0') {
		ret.erase(ret.end() - 1);
	}

	return ret;
}

std::string printableFromByteArray(const std::array<uint8_t, PREVIEW_DATAREF_ARRAY_COUNT> & bytes) {
	std::string ret;
	for(const uint8_t byte : bytes) {
		if(0 == byte) {
			break;
		} else if(::isprint(byte)) {
			ret.push_back(byte);
		} else {
			break;
		}
	}

	return ret;
}

template <class T>
std::string makeArrayString(std::function<std::string(T)> stringify, const std::array<T, PREVIEW_DATAREF_ARRAY_COUNT> & array, int count) {
	std::stringstream s;
	s << "[";

	for(int i = 0; i < std::min<int>(array.size(), count); i++) {
		s << stringify(array[i]);

		if(i != count-1) {
			s << ",";
		}
	}

	if(4 < count) {
		s << "...";
	}

	s << "]";
	return s.str();
}

std::string DataRefRecord::getValueString() const {
	if(type & xplmType_Double) {
		return compactFpString(lf_val);
	} else if (type & xplmType_Float) {
		return compactFpString(f_val);
	} else if (type & xplmType_Int) {
		return std::to_string(i_val);
	} else if (type & xplmType_FloatArray) {
		return makeArrayString<float>(&compactFpString<float>, fv_val, array_length);
	} else if (type & xplmType_IntArray) {
		typedef std::string (*s_type)(int);
		s_type stringify_func = std::to_string;
		return makeArrayString<int>(stringify_func, iv_val, array_length);
	} else if (type & xplmType_Data) {
		return "\"" + printableFromByteArray(b_val) + "\"";
	} else {
		return "--";
	}
}
