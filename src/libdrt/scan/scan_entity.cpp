#include "scan_entity.h"

#include "scan_files.h"

#include "deduplicate_vector.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>

#include <filesystem.h>
#include <boost/algorithm/string.hpp>

using namespace std::string_literals;


std::vector<std::string> scanAircraft(std::ostream & log, const lb::filesystem::path & acf_path) {
	lb::filesystem::path aircraft_dir = lb::filesystem::path(acf_path).parent_path();

	std::vector<std::string> all_refs;

	// list file
	lb::filesystem::path list_file_path = aircraft_dir / "dataref.txt";
	if(lb::filesystem::exists(list_file_path)) {
		std::vector<std::string> refs = loadListFile(log, list_file_path);
		all_refs.insert(all_refs.begin(), refs.begin(), refs.end());
	}

	//cdataref.txt
	lb::filesystem::path cdataref_path = aircraft_dir / "cdataref.txt";
	if(lb::filesystem::exists(cdataref_path) && lb::filesystem::is_regular_file(cdataref_path)) {
		std::ifstream inFile(cdataref_path.string());

        std::stringstream file_contents_ss;
        file_contents_ss << inFile.rdbuf();
		std::string file_contents = file_contents_ss.str();

		std::vector<std::string> cdataref_entries;
		boost::split(cdataref_entries, file_contents, boost::is_any_of("\n\r\t, "));
		for(std::string & s : cdataref_entries) {
			boost::algorithm::trim(s);
		}

		//remove empty lines
		cdataref_entries.erase(std::remove_if(cdataref_entries.begin(), cdataref_entries.end(), [](const std::string & a)-> bool { return a.size() < 8; }), cdataref_entries.end());

		all_refs.insert(all_refs.begin(), cdataref_entries.begin(), cdataref_entries.end());

		log << "Found " << cdataref_entries.size() << " unique possible datarefs in " << cdataref_path << "\n";
	}

	//plugins
	lb::filesystem::path plugin_dir_path = aircraft_dir / "plugins";

	//for each directory inside plugin path
	if(lb::filesystem::exists(plugin_dir_path)) {
		std::vector<std::string> refs = scanPluginFolder(log, plugin_dir_path);
		all_refs.insert(all_refs.begin(), refs.begin(), refs.end());
	}

	//object files in aircraft directory
	std::vector<lb::filesystem::path> paths;
	lb::filesystem::directory_iterator dir_end_it;
	for(lb::filesystem::directory_iterator dir_it(aircraft_dir); dir_it != dir_end_it; dir_it++) {
		lb::filesystem::path file_path = dir_it->path();

		if(lb::filesystem::is_regular_file(file_path)) {
			if(".obj" == file_path.extension() || ".acf" == file_path.extension()) {
				paths.push_back(file_path);
			}
		}
	}
    
    paths.push_back(aircraft_dir / "Custom Avionics");  // apparently SASL / LUA code is often in this directory
    paths.push_back(aircraft_dir / "objects");
    lb::filesystem::path xlua_scripts_dir = aircraft_dir / "plugins" / "xlua" / "scripts"; // find xlua LUA scripts from X-Plane 11.00+
    if(lb::filesystem::exists(xlua_scripts_dir)) {
	    paths.push_back(xlua_scripts_dir);
    }

    std::unordered_set<std::string> extensions_to_scan = {".obj", ".acf", ".lua"};
	while(false == paths.empty()) {
		lb::filesystem::path path = paths.back();
		paths.pop_back();

		if(lb::filesystem::is_directory(path)) {
			//iterate over directory, pushing back
			for(lb::filesystem::directory_iterator dir_it(path); dir_it != dir_end_it; dir_it++) {
				paths.push_back(dir_it->path());
			}
		}

		if(lb::filesystem::is_regular_file(path)) {
            std::string extension = path.extension().string();
            boost::algorithm::to_lower(extension);
            
			if(extensions_to_scan.cend() != extensions_to_scan.find(extension)) {
				std::vector<std::string> refs = scanFileForDatarefStrings(log, path);
				all_refs.insert(all_refs.begin(), refs.begin(), refs.end());
			}
		}
	}
	deduplicate_vector(all_refs);
	log << "Found " << all_refs.size() << " unique possible datarefs for aircraft " << acf_path << "\n";

	return all_refs;
}

std::vector<std::string> scanLuaFolder(std::ostream & log, const lb::filesystem::path & lua_dir_path) {
	std::vector<std::string> lua_folder_datarefs;
	if(lb::filesystem::is_directory(lua_dir_path)) { // if this is true, it also exists
		for(auto& file_de : boost::make_iterator_range(lb::filesystem::directory_iterator(lua_dir_path), {})) {
			if(file_de.path().extension() == ".lua") {
				std::vector<std::string> this_script_datarefs = scanFileForDatarefStrings(log, file_de.path());
				lua_folder_datarefs.insert(lua_folder_datarefs.cend(), this_script_datarefs.begin(), this_script_datarefs.end());
			}
		}
	}

	return lua_folder_datarefs;
}

std::vector<std::string> scanPluginFolder(std::ostream & log, const lb::filesystem::path & plugin_dir_path) {
	// make a list of all XPL files in the plugin folder
	using recursive_directory_iterator = lb::filesystem::recursive_directory_iterator;

	std::vector<lb::filesystem::path> xpl_paths;
	std::vector<std::string> all_refs;
	for (const lb::filesystem::directory_entry & dir_entry : recursive_directory_iterator(plugin_dir_path)) {
		lb::filesystem::path path(dir_entry);
		if(lb::filesystem::is_regular_file(path) && ".xpl"s == path.extension()) {
			std::vector<std::string> refs = scanFileForDatarefStrings(log, path);
			all_refs.insert(all_refs.begin(), refs.begin(), refs.end());
		}
	}

	deduplicate_vector(all_refs);

	return all_refs;
}

std::vector<std::string> scanPluginXPL(std::ostream & log, const lb::filesystem::path & plugin_xpl_path) {
	std::vector<std::string> refs = scanFileForDatarefStrings(log, plugin_xpl_path);

	deduplicate_vector(refs);

	return refs;
}

std::vector<std::string> scanXplaneBinary(std::ostream & log, const lb::filesystem::path & xplane_binary_path) {
	std::vector<std::string> refs = scanFileForDatarefStrings(log, xplane_binary_path);

	deduplicate_vector(refs);

	return refs;
}