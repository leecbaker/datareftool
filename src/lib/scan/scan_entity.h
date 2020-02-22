#pragma once

#include <algorithm>
#include <ostream>
#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>

// Scan an entity, like an aircraft or a plugin.

// these parse a directory structure, calling getDatarefsFromFile() on appropriate files.
std::vector<std::string> scanAircraft(std::ostream & log, const boost::filesystem::path & acf_path);


std::vector<std::string> scanPluginFolder(std::ostream & log, const boost::filesystem::path & plugin_dir_path);

// Scan only the lua files in a directory
std::vector<std::string> scanLuaFolder(std::ostream & log, const boost::filesystem::path & lua_dir_path);

// For example, the blacklist
std::vector<std::string> loadListFile(std::ostream & log, const boost::filesystem::path & path);
