#include "dataref_files.h"

#include <fstream>

#include <boost/algorithm/string.hpp>

std::vector<std::string> loadBlacklistFile(std::ostream & log, const boost::filesystem::path & filename) {
    std::vector<std::string> entries;
    std::ifstream f(filename.string());
    if(f.fail()) {
        log << "Failed to open blacklist file at " << filename << "\n";
        return {};
    }
    
    std::string line;
    while(std::getline(f, line)) {
        size_t first_hash = line.find("#");
		if (first_hash != std::string::npos) {
			line = line.substr(0, first_hash);
		}
        boost::algorithm::trim(line);
		if (line.empty()) {
			continue;
		}
        
        entries.emplace_back(std::move(line));
    }

    log << "Loaded blacklist with " << entries.size() << " entries\n";
    
    return entries;
}

std::vector<std::string> loadDatarefsFile(std::ostream & log, const boost::filesystem::path & filename) {
    log << "Loading datarefs from path " << filename << "\n";
    
    std::ifstream dr_file(filename.string());
    
    if(dr_file.bad()) {
        log << "DataRefs.txt file could not be loaded\n";
        return {};
    }
    
    std::string line;
    std::getline(dr_file, line);	//discard header
    std::vector<std::string> datarefs;
    while(std::getline(dr_file, line)) {
        size_t tab_offset = line.find_first_of("\t ");
        if(tab_offset == std::string::npos) {
            continue;
        }
        
        line.erase(line.begin() + tab_offset, line.end());
        datarefs.emplace_back(std::move(line));
    }
    
    log << "Finished loading " << datarefs.size() << " datarefs\n";
    
    return datarefs;
}
