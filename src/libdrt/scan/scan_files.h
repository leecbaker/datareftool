#pragma once

#include <ostream>
#include <string>
#include <vector>

#include <filesystem.h>

// Load a file that lists a dataref at the beginning of each line, optionally with other material after a whitespace
std::vector<std::string> loadListFile(std::ostream & log, const lb::filesystem::path & filename);

// use a `strings`-like approach to finding datarefs
std::vector<std::string> scanFileForDatarefStrings(std::ostream & log, const lb::filesystem::path & path);
