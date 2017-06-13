#pragma once

#include <boost/filesystem/path.hpp>

bool getAutoReloadPlugins();
void setAutoReloadPlugins(bool reload_automatically);

bool getImpersonateDRE();
void setImpersonateDRE(bool impersonate);

bool loadPrefs(const boost::filesystem::path & path);
bool savePrefs(const boost::filesystem::path & path);
