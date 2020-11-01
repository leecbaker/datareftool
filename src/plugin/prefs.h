#pragma once

#include <functional>

#include "nlohmann/json.hpp"

#include "filesystem.h"

bool getAutoReloadPlugins();
void setAutoReloadPlugins(bool reload_automatically);

bool getImpersonateDRE();
bool getLoggingEnabled();
void setImpersonateDRE(bool impersonate);

bool getDebugMode();
void setDebugMode(bool enabled);

bool loadPrefs(const lb::filesystem::path & path, std::function<void(const nlohmann::json &)> create_window_func);
bool savePrefs(const lb::filesystem::path & path, const nlohmann::json & windows);
