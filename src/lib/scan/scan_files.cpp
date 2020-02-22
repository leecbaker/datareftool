#include "scan_files.h"

#include "deduplicate_vector.h"

#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

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

std::vector<std::string> loadListFile(std::ostream & log, const boost::filesystem::path & filename) {
    log << "Loading datarefs from path " << filename << "\n";
    
    std::ifstream dr_file(filename.string());
    
    if(dr_file.bad()) {
        log << "File could not be loaded: " << filename.string() << "\n";
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


// + character is necessary for A320neo
inline char isValidDatarefChar(char c) {
	return std::isalnum(static_cast<unsigned char>(c)) || '_' == c || '/' == c || '-' == c || '.' == c || '+' == c;
}

const size_t min_dataref_length = 8;

std::vector<std::string> scanFileForDatarefStrings(std::ostream & log, const boost::filesystem::path & filename) {
    boost::iostreams::mapped_file_source file;
    
    try {
        file.open(filename);
    } catch(std::exception &) {
		return {};
    }

	std::vector<std::string> all_refs;
	size_t start_pos = 0;
	const char * file_data = file.data();
	bool last_char_valid = false;
    for(size_t i = 0; i < file.size(); i++) {
    	if(isValidDatarefChar(file_data[i])) {
    		if(false == last_char_valid) {
    			start_pos = i;
    		}
    		last_char_valid = true;
    		continue;
    	} else {
    		 if(last_char_valid && (i - start_pos) > min_dataref_length) {
    			std::string new_dataref(file_data + start_pos, file_data + i);

    			//get rid of some obvious false positives
    			if('/' != new_dataref.front() && std::string::npos != new_dataref.find('/')) {
    				all_refs.push_back(new_dataref);
    			}
	    	}

    		last_char_valid = false;
    	}
    }
	
	deduplicate_vector(all_refs);

	log << "Found " << all_refs.size() << " unique possible datarefs in file " << filename << "\n";

	return all_refs;
}