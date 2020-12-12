#pragma once

#include <algorithm>
#include <ostream>
#include <string>
#include <vector>

#include <filesystem.h>

// Scan an entity, like an aircraft or a plugin.

// these parse a directory structure, calling getDatarefsFromFile() on appropriate files.
std::vector<std::string> scanAircraft(std::ostream & log, const lb::filesystem::path & acf_path);

std::vector<std::string> scanPluginFolder(std::ostream & log, const lb::filesystem::path & plugin_dir_path);
std::vector<std::string> scanPluginXPL(std::ostream & log, const lb::filesystem::path & plugin_xpl_path);

std::vector<std::string> scanXplaneBinary(std::ostream & log, const lb::filesystem::path & xplane_binary_path);

// Scan only the lua files in a directory
std::vector<std::string> scanLuaFolder(std::ostream & log, const lb::filesystem::path & lua_dir_path);

// For example, the ignore list
std::vector<std::string> loadListFile(std::ostream & log, const lb::filesystem::path & path);
