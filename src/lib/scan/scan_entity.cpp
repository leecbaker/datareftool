#include "scan_entity.h"

#include "scan_files.h"

#include "deduplicate_vector.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>


std::vector<std::string> scanAircraft(std::ostream & log, const boost::filesystem::path & acf_path) {
	boost::filesystem::path aircraft_dir = boost::filesystem::path(acf_path).parent_path();

	std::vector<std::string> all_refs;

	// list file
	boost::filesystem::path list_file_path = aircraft_dir / "dataref.txt";
	if(boost::filesystem::exists(list_file_path)) {
		std::vector<std::string> refs = loadListFile(log, list_file_path);
		all_refs.insert(all_refs.begin(), refs.begin(), refs.end());
	}

	//cdataref.txt
	boost::filesystem::path cdataref_path = aircraft_dir / "cdataref.txt";
	if(boost::filesystem::exists(cdataref_path) && boost::filesystem::is_regular_file(cdataref_path)) {
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
	boost::filesystem::path plugin_dir_path = aircraft_dir / "plugins";

	//for each directory inside plugin path
	if(boost::filesystem::exists(plugin_dir_path)) {
		boost::filesystem::directory_iterator dir_end_it;
		for(boost::filesystem::directory_iterator dir_it(plugin_dir_path); dir_it != dir_end_it; dir_it++) {
			std::vector<std::string> refs = scanPluginFolder(log, dir_it->path());
			all_refs.insert(all_refs.begin(), refs.begin(), refs.end());
		}
	}

	//object files in aircraft directory
	std::vector<boost::filesystem::path> paths;
	boost::filesystem::directory_iterator dir_end_it;
	for(boost::filesystem::directory_iterator dir_it(aircraft_dir); dir_it != dir_end_it; dir_it++) {
		boost::filesystem::path file_path = dir_it->path();

		if(boost::filesystem::is_regular_file(file_path)) {
			if(".obj" == file_path.extension() || ".acf" == file_path.extension()) {
				paths.push_back(file_path);
			}
		}
	}
    
    paths.push_back(aircraft_dir / "Custom Avionics");  // apparently SASL / LUA code is often in this directory
    paths.push_back(aircraft_dir / "objects");
    boost::filesystem::path xlua_scripts_dir = aircraft_dir / "plugins" / "xlua" / "scripts"; // find xlua LUA scripts from X-Plane 11.00+
    if(boost::filesystem::exists(xlua_scripts_dir)) {
	    paths.push_back(xlua_scripts_dir);
    }

    std::unordered_set<std::string> extensions_to_scan = {".obj", ".acf", ".lua"};
	while(false == paths.empty()) {
		boost::filesystem::path path = paths.back();
		paths.pop_back();

		if(boost::filesystem::is_directory(path)) {
			//iterate over directory, pushing back
			for(boost::filesystem::directory_iterator dir_it(path); dir_it != dir_end_it; dir_it++) {
				paths.push_back(dir_it->path());
			}
		}

		if(boost::filesystem::is_regular_file(path)) {
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

std::vector<std::string> scanLuaFolder(std::ostream & log, const boost::filesystem::path & lua_dir_path) {
	std::vector<std::string> lua_folder_datarefs;
	if(boost::filesystem::is_directory(lua_dir_path)) { // if this is true, it also exists
		for(auto& file_de : boost::make_iterator_range(boost::filesystem::directory_iterator(lua_dir_path), {})) {
			if(file_de.path().extension() == ".lua") {
				std::vector<std::string> this_script_datarefs = scanFileForDatarefStrings(log, file_de.path());
				lua_folder_datarefs.insert(lua_folder_datarefs.cend(), this_script_datarefs.begin(), this_script_datarefs.end());
			}
		}
	}

	return lua_folder_datarefs;
}

std::vector<std::string> scanPluginFolder(std::ostream & log, const boost::filesystem::path & plugin_xpl_path) {
#ifdef __APPLE__
	static const std::string plugin_name = "mac.xpl";
	static const std::string xp11_plugin_dir_name = "mac_x64";
#elif defined _WIN32 || defined _WIN64
	static const std::string plugin_name = "win.xpl";
	static const std::string xp11_plugin_dir_name = "win_x64";
#else
	static const std::string plugin_name = "lin.xpl";
	static const std::string xp11_plugin_dir_name = "lin_x64";
#endif

	std::vector<std::string> all_refs;
	boost::filesystem::path plugin_dir(plugin_xpl_path.parent_path());
	boost::filesystem::path plugin_old_path = plugin_dir / plugin_name;
	boost::filesystem::path plugin_new_path = plugin_dir / "64" / plugin_name;
	boost::filesystem::path plugin_xp11_path = plugin_dir / xp11_plugin_dir_name / (plugin_dir.filename().string() + ".xpl");

	if(boost::filesystem::exists(plugin_old_path)) {
		std::vector<std::string> refs = scanFileForDatarefStrings(log, plugin_old_path);
		all_refs.insert(all_refs.begin(), refs.begin(), refs.end());
	}
	if(boost::filesystem::exists(plugin_new_path)) {
		std::vector<std::string> refs = scanFileForDatarefStrings(log, plugin_new_path);
		all_refs.insert(all_refs.begin(), refs.begin(), refs.end());
	}
	if(boost::filesystem::exists(plugin_xp11_path)) {
		std::vector<std::string> refs = scanFileForDatarefStrings(log, plugin_xp11_path);
		all_refs.insert(all_refs.begin(), refs.begin(), refs.end());
	}

	deduplicate_vector(all_refs);

	return all_refs;
}

