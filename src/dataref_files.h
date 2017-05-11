#pragma once

#include <string>
#include <vector>

#include <boost/filesystem.hpp>

std::vector<std::string> loadBlacklistFile(const boost::filesystem::path & filename);
std::vector<std::string> loadDatarefsFile(const boost::filesystem::path & filename);
