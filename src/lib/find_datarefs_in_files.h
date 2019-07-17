#pragma once

#include <algorithm>
#include <ostream>
#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>

template <class T>
void inline removeVectorUniques(std::vector<T> & v) {
	std::sort(v.begin(), v.end());
	auto last_unique = std::unique(v.begin(), v.end());
	v.erase(last_unique, v.end());
}

// use a `strings`-like approach to finding datarefs
std::vector<std::string> getDatarefsFromFile(std::ostream & log, const boost::filesystem::path & path);

// these parse a directory structure, calling getDatarefsFromFile() on appropriate files.
std::vector<std::string> getDatarefsFromAircraft(std::ostream & log, const boost::filesystem::path & acf_path);
std::vector<std::string> getDatarefsFromPluginFolder(std::ostream & log, const boost::filesystem::path & plugin_dir_path);
std::vector<std::string> getDatarefsFromListFile(std::ostream & log, const boost::filesystem::path & path);
