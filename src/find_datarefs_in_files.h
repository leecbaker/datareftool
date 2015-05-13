#pragma once

#include <algorithm>
#include <string>
#include <vector>

template <class T>
void inline removeVectorUniques(std::vector<T> & v) {
	std::sort(v.begin(), v.end());
	auto last_unique = std::unique(v.begin(), v.end());
	v.erase(last_unique, v.end());
}

// use a `strings`-like approach to finding datarefs
std::vector<std::string> getDatarefsFromFile(const std::string & path);

// these parse a directory structure, calling getDatarefsFromFile() on appropriate files.
std::vector<std::string> getDatarefsFromAircraft(const std::string & acf_path);
std::vector<std::string> getDatarefsFromPluginFolder(const std::string & plugin_dir_path);
std::vector<std::string> getDatarefsFromListFile(const std::string & path);