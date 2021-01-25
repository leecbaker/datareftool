#pragma once

#include <memory>
#include <vector>

#include "nlohmann/json.hpp"

#include "filesystem.h"

class AboutWindow;
class SearchWindow;

class Plugin {
    std::weak_ptr<AboutWindow> about_window_ref;
    std::vector<std::weak_ptr<SearchWindow>> search_window_refs;
    lb::filesystem::path prefs_path;
public:
    Plugin() = default;
    ~Plugin() = default;

    void openAboutWindow();
    std::shared_ptr<SearchWindow> openSearchWindow();
    std::shared_ptr<SearchWindow> openSearchWindow(const nlohmann::json & window_params);

    nlohmann::json dumpSearchWindow(const std::shared_ptr<SearchWindow> & search_window) const;
};