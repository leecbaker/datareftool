#pragma once

#include <boost/filesystem/path.hpp>

bool loadPrefs(const boost::filesystem::path & path);
bool savePrefs(const boost::filesystem::path & path);
