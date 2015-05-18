#include "find_datarefs_in_files.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string.hpp>

#include "XPLMUtilities.h"

inline char isValidDatarefChar(char c) {
	return std::isalnum(c) || '_' == c || '/' == c || '-' == c || '.' == c;
}

const size_t min_dataref_length = 8;

std::vector<std::string> getDatarefsFromFile(const std::string & filename) {
    boost::iostreams::mapped_file_source file;
    
    try {
        file.open(filename);
    } catch(std::exception & e) {
		return {};
    }

	std::vector<std::string> all_refs;
	size_t start_pos = 0;
	const char * file_data = file.data();
	bool last_char_valid = false;
    for(size_t i = 0; i < file.size(); i++) {
    	if(isValidDatarefChar(file_data[i])) {
    		if(false == last_char_valid) {
    			start_pos = i;
    		}
    		last_char_valid = true;
    		continue;
    	} else {
    		 if(last_char_valid && (i - start_pos) > min_dataref_length) {
    			std::string new_dataref(file_data + start_pos, file_data + i);

    			//get rid of some obvious false positives
    			if('/' != new_dataref.front() && std::string::npos != new_dataref.find('/')) {
    				all_refs.push_back(new_dataref);
    			}
	    	}

    		last_char_valid = false;
    	}
    }
	
	removeVectorUniques(all_refs);

	const std::string message = std::string("DRT: Found ") + std::to_string(all_refs.size()) + std::string(" unique possible datarefs in file ") + filename + std::string("\n");
	XPLMDebugString(message.c_str());

	return all_refs;
}

std::vector<std::string> getDatarefsFromAircraft(const std::string & acf_path) {
	boost::filesystem::path aircraft_dir = boost::filesystem::path(acf_path).parent_path();

	std::vector<std::string> all_refs;

	// list file
	boost::filesystem::path list_file_path = aircraft_dir / "dataref.txt";
	if(boost::filesystem::exists(list_file_path)) {
		std::vector<std::string> refs = getDatarefsFromFile(list_file_path.string());
		all_refs.insert(all_refs.begin(), refs.begin(), refs.end());
	}

	//cdataref.txt
	boost::filesystem::path cdataref_path = aircraft_dir / "cdataref.txt";
	if(boost::filesystem::exists(cdataref_path) && boost::filesystem::is_regular_file(cdataref_path)) {
		std::ifstream inFile(cdataref_path.native());
        std::stringstream sstr;
        sstr << inFile.rdbuf();

		std::vector<std::string> cdataref_entries;
		std::string s = sstr.str();
		boost::split(cdataref_entries, s, boost::is_any_of("\n\r\t, "));
		for(std::string & s : cdataref_entries) {
			boost::algorithm::trim(s);
		}

		//remove empty lines
		cdataref_entries.erase(std::remove_if(cdataref_entries.begin(), cdataref_entries.end(), [](const std::string & a)-> bool { return a.size() < min_dataref_length; }), cdataref_entries.end());

		all_refs.insert(all_refs.begin(), cdataref_entries.begin(), cdataref_entries.end());

		const std::string message = std::string("DRT: Found ") + std::to_string(cdataref_entries.size()) + std::string(" unique possible datarefs in ") + cdataref_path.string() + std::string("\n");
		XPLMDebugString(message.c_str());
	}

	//plugins
	boost::filesystem::path plugin_dir_path = aircraft_dir / "plugins";

	//for each directory inside plugin path
	if(boost::filesystem::exists(plugin_dir_path)) {
		boost::filesystem::directory_iterator dir_end_it;
		for(boost::filesystem::directory_iterator dir_it(plugin_dir_path); dir_it != dir_end_it; dir_it++) {
			std::vector<std::string> refs = getDatarefsFromPluginFolder(dir_it->path().string());
			all_refs.insert(all_refs.begin(), refs.begin(), refs.end());
		}
	}

	//object files. Recurse over directory structure
	std::vector<boost::filesystem::path> paths;
	paths.push_back(aircraft_dir / "objects");

	while(false == paths.empty()) {
		boost::filesystem::path path = paths.back();
		paths.pop_back();

		if(boost::filesystem::is_directory(path)) {
			//iterate over directory, pushing back
			boost::filesystem::directory_iterator dir_end_it;
			for(boost::filesystem::directory_iterator dir_it(path); dir_it != dir_end_it; dir_it++) {
				paths.push_back(dir_it->path());
			}
		}

		if(boost::filesystem::is_regular_file(path)) {
			if(".obj" == path.extension() || ".acf" == path.extension()) {
				std::vector<std::string> refs = getDatarefsFromFile(path.string());
				all_refs.insert(all_refs.begin(), refs.begin(), refs.end());
			}
		}
	}
	removeVectorUniques(all_refs);
	const std::string message = std::string("DRT: Found ") + std::to_string(all_refs.size()) + std::string(" unique possible datarefs for aircraft ") + acf_path + std::string("\n");
	XPLMDebugString(message.c_str());

	return all_refs;
}

std::vector<std::string> getDatarefsFromListFile(const std::string & filename) {
	return getDatarefsFromFile(filename);
}

std::vector<std::string> getDatarefsFromPluginFolder(const std::string & plugin_dir_path) {
#ifdef __APPLE__
	static const std::string plugin_name = "mac.xpl";
#elif defined _WIN32 || defined _WIN64
	static const std::string plugin_name = "win.xpl";
#else
	static const std::string plugin_name = "lin.xpl";
#endif

	std::vector<std::string> all_refs;
	boost::filesystem::path plugin_dir(plugin_dir_path);
	boost::filesystem::path plugin_old_path = plugin_dir / plugin_name;
	boost::filesystem::path plugin_new_path = plugin_dir / "64" / plugin_name;

	if(boost::filesystem::exists(plugin_old_path)) {
		std::vector<std::string> refs = getDatarefsFromFile(plugin_old_path.string());
		all_refs.insert(all_refs.begin(), refs.begin(), refs.end());
	}
	if(boost::filesystem::exists(plugin_new_path)) {
		std::vector<std::string> refs = getDatarefsFromFile(plugin_new_path.string());
		all_refs.insert(all_refs.begin(), refs.begin(), refs.end());
	}

	removeVectorUniques(all_refs);

	return all_refs;
}