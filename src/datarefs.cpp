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
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string.hpp>

std::vector<DataRefRecord> datarefs;
std::unordered_set<std::string> datarefs_loaded;	//check for duplicates

DataRefRecord::DataRefRecord(const std::string & name, XPLMDataRef ref, dataref_src_t source) : name(name), last_updated(std::chrono::system_clock::now()), ref(ref), source(source) {
	type = 	XPLMGetDataRefTypes(ref);
    
    if(type & xplmType_Double) {
        value = 0.;
    } else if (type & xplmType_Float) {
        value = 0.f;
    } else if (type & xplmType_Int) {
        value = 0;
    } else if (type & xplmType_FloatArray) {
        value = std::vector<float>();
    } else if (type & xplmType_IntArray) {
        value = std::vector<int>();
    } else if (type & xplmType_Data) {
        value = std::vector<uint8_t>();
    } else {
        value = nullptr;
    }
}

bool DataRefRecord::writable() const {
	if(isInt() || isDouble() || isFloat() || isFloatArray() || isIntArray()) {
		if(0 == XPLMCanWriteDataRef(ref)) {
			return false;
		}

		return true;
	}

	return false;
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
}

bool DataRefRecord::Updater::operator()(float & old_val) const {
    float newval = XPLMGetDataf(dr.ref);
    if(newval != old_val) {
        if(0.01f < fabs(newval - old_val) / old_val) {
            dr.last_updated_big = current_time;
        }
        dr.last_updated = current_time;
        dr.value = newval;
        return true;
    } else {
        return false;
    }
}

bool DataRefRecord::Updater::operator()(double& oldval) const {
    double newval = XPLMGetDatad(dr.ref);
    if(newval != oldval) {
        if(0.01 < fabsl(newval - oldval) / oldval) {
            dr.last_updated_big = current_time;
        }
        dr.last_updated = current_time;
        dr.value = newval;
        return true;
    } else {
        return false;
    }
}

bool DataRefRecord::Updater::operator()(int& oldval) const {
    int newval = XPLMGetDatai(dr.ref);
    if(newval != oldval) {
        dr.last_updated = current_time;
        dr.value = newval;
        return true;
    } else {
        return false;
    }
}

bool DataRefRecord::Updater::operator()(std::vector<float>&) const {
    int current_array_length = XPLMGetDatavf(dr.ref, nullptr, 0, 0);
    static std::vector<float> scratch_buffer;
    scratch_buffer.resize(current_array_length);
    int copied = XPLMGetDatavf(dr.ref, scratch_buffer.data(), 0, current_array_length);
    if(copied == current_array_length) {
        size_t new_hash = boost::hash_range(scratch_buffer.cbegin(), scratch_buffer.cend());
        if(new_hash != dr.array_hash) {
            dr.array_hash = new_hash;
            dr.last_updated = current_time;
            dr.value = scratch_buffer;
            return true;
        } else {
            return false;
        }
    } else {
        //Dataref is reporting an inconsistent size
        return false;
    }
    
}

bool DataRefRecord::Updater::operator()(std::vector<int>&) const {
    int current_array_length = XPLMGetDatavi(dr.ref, nullptr, 0, 0);
    static std::vector<int> scratch_buffer;
    scratch_buffer.resize(current_array_length);
    int copied = XPLMGetDatavi(dr.ref, scratch_buffer.data(), 0, current_array_length);
    if(copied == current_array_length) {
        size_t new_hash = boost::hash_range(scratch_buffer.cbegin(), scratch_buffer.cend());
        if(new_hash != dr.array_hash) {
            dr.array_hash = new_hash;
            dr.last_updated = current_time;
            dr.value = scratch_buffer;
            return true;
        } else {
            return false;
        }
    } else {
        //Dataref is reporting an inconsistent size
        return false;
    }
    
}

bool DataRefRecord::Updater::operator()(std::vector<uint8_t>&) const {
    int current_array_length = XPLMGetDatab(dr.ref, nullptr, 0, 0);
    static std::vector<uint8_t> scratch_buffer;
    scratch_buffer.resize(current_array_length);
    int copied = XPLMGetDatab(dr.ref, scratch_buffer.data(), 0, current_array_length);
    if(copied == current_array_length) {
        size_t new_hash = boost::hash_range(scratch_buffer.cbegin(), scratch_buffer.cend());
        if(new_hash != dr.array_hash) {
            dr.array_hash = new_hash;
            dr.last_updated = current_time;
            dr.value = scratch_buffer;
            return true;
        } else {
            return false;
        }
    } else {
        //Dataref is reporting an inconsistent size
        return false;
    }
    
}

bool DataRefRecord::Updater::operator()(std::nullptr_t&) const {
    dr.last_updated = current_time;
    return false;
}

bool DataRefRecord::update(const std::chrono::system_clock::time_point current_time) {
    return boost::apply_visitor(Updater(*this, current_time), value);
}

template <class T>
std::string compactFpString(T f) {
	std::string ret = std::to_string(f);

	while(false == ret.empty() && ret.back() == '0') {
		ret.erase(ret.end() - 1);
	}

	return ret;
}

std::string printableFromByteArray(const std::vector<uint8_t> & bytes) {
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
std::string makeArrayString(std::string (*stringify_func)(T), const std::vector<T> & array, size_t max_chars) {
	std::stringstream s;
	s << "[";
    
    size_t current_length = 2;  //for the braces
    const std::string elipses = "...";

    for(const T & element : array) {
        const std::string & s_element = stringify_func(element);
        
        size_t max_length_if_ending_now = current_length + elipses.size();
        size_t max_length_if_ending_next = max_length_if_ending_now + 1 + s_element.size(); //comma + value
        
        if(max_length_if_ending_next < max_chars) {
            if(current_length != 2) {
                s << ",";
            }
            s << s_element;
            current_length += 1 + s_element.size();
        } else {
            s << elipses;
            break;
        }
	}

	s << "]";
	return s.str();
}

std::string DataRefRecord::getLabelString() const {
	std::string ret = getName();
	if(isArray()) {
		ret += "[" + std::to_string(getArrayLength()) + "]";
	}
	return ret;
}

class DatarefDisplayStringifier : public boost::static_visitor<std::string> {
    size_t max_chars;
public:
    DatarefDisplayStringifier(size_t max_chars) : max_chars(max_chars) {}
	std::string operator()(const float & f) const { return compactFpString(f); }
	std::string operator()(const double & f) const { return compactFpString(f); }
	std::string operator()(const int & i) const { return std::to_string(i); }
	std::string operator()(const std::vector<float> & fv) const {
		return makeArrayString<float>(compactFpString<float>, fv, max_chars);
	}
    std::string operator()(const std::vector<int> & iv) const {
        std::string (*stringify_func)(int) = std::to_string;
		return makeArrayString<int>(stringify_func, iv, max_chars);
	}
	std::string operator()(const std::vector<uint8_t> & iv) const {
		return "\"" + printableFromByteArray(iv) + "\"";
    }
    std::string operator()(const std::nullptr_t &) const {
        return "(null)";
    }
};

class DatarefEditStringifier : public boost::static_visitor<std::string> {
public:
    std::string operator()(const float f) const { return compactFpString(f); }
    std::string operator()(const double f) const { return compactFpString(f); }
    std::string operator()(const int i) const { return std::to_string(i); }
    std::string operator()(const std::vector<float> & fv) const {
        return makeArrayString<float>(compactFpString<float>, fv, std::numeric_limits<size_t>::max());
    }
    std::string operator()(const std::vector<int> & iv) const {
        std::string (*stringify_func)(int) = std::to_string;
        return makeArrayString<int>(stringify_func, iv, std::numeric_limits<size_t>::max());
    }
    std::string operator()(const std::vector<uint8_t> & iv) const {
        return "\"" + printableFromByteArray(iv) + "\"";
    }
    std::string operator()(const std::nullptr_t ) const {
        return "(null)";
    }
};

std::string DataRefRecord::getDisplayString(size_t display_length) const {
	return boost::apply_visitor(DatarefDisplayStringifier(display_length), value);
}

std::string DataRefRecord::getEditString() const {
    return boost::apply_visitor(DatarefEditStringifier(), value);
}

template <typename T>
inline T parseElement(const std::string &s);

template <>
inline float parseElement<float>(const std::string &s) {
	return std::stof(s);
}
template <>
inline int parseElement<int>(const std::string &s) {
	return std::stoi(s);
}

template <class T>
bool parseArray(const std::string & txt, std::vector<T> & data_out, int length) {
	std::vector<std::string> txt_fields;

	std::string trimmed_txt = txt;
	if(trimmed_txt.front() == '[') {
		trimmed_txt.erase(trimmed_txt.begin());
	}

	if(trimmed_txt.back() == '[') {
		trimmed_txt.pop_back();
	}

	boost::split(txt_fields, trimmed_txt, boost::is_any_of(","));

	if(length != int(txt_fields.size())) {
		XPLMDebugString("Save cancelled, as supplied data array doesn't match DR array length");
		return false;
	}

	data_out.clear();
	data_out.reserve(length);

	for(const std::string & txt_field : txt_fields) {
		try {
			data_out.push_back(parseElement<T>(txt_field));
		} catch (std::exception &) {
			XPLMDebugString("Save cancelled, failed to parse field");
			return false;
		}
	}

	return true;
}

template
bool parseArray<float>(const std::string & txt, std::vector<float> & data_out, int length);

template
bool parseArray<int>(const std::string & txt, std::vector<int> & data_out, int length);
