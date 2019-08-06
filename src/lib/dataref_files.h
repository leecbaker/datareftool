#pragma once

#include <ostream>
#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>

std::vector<std::string> loadBlacklistFile(std::ostream & log, const boost::filesystem::path & filename);
std::vector<std::string> loadDatarefsFile(std::ostream & log, const boost::filesystem::path & filename);
