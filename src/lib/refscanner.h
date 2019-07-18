#pragma once

#include "allrefs.h"

#include <string>
#include <vector>

class RefScanner : public RefRecords {
    std::vector<std::string> blacklisted_datarefs;
public:
    RefScanner(std::ostream & log) : RefRecords(log) {}
    void loadBlacklist(const boost::filesystem::path & blacklist_path);
    void addBlacklist();
    void scanInitial(const std::vector<boost::filesystem::path> & plugin_paths, const boost::filesystem::path & aircraft_path);
    void scanPlugins();
    void scanAircraft(const boost::filesystem::path & aircraft);
};
