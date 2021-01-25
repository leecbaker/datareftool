#include "scan_files.h"

#include "deduplicate_vector.h"

#include <fstream>

#include "mio.hpp"

std::vector<std::string> loadListFile(std::ostream & log, const lb::filesystem::path & filename) {
    log << "Loading datarefs from path " << filename << "\n";
    
    std::ifstream dr_file(filename.string());
    
    if(dr_file.bad()) {
        log << "File could not be loaded: " << filename.string() << "\n";
        return {};
    }
    
    std::string line;
    std::vector<std::string> potential_datarefs;
    while(std::getline(dr_file, line)) {
        while(false == line.empty() && std::isspace(line.front())) {
            line.erase(line.cbegin());
        }
        size_t tab_offset = line.find_first_of("\t ");
        if(tab_offset != std::string::npos) {
            line.erase(line.begin() + tab_offset, line.end());
        }
        
        if(std::string::npos != line.find("/")) {
            potential_datarefs.emplace_back(std::move(line));
        }
    }
    
    log << "Finished loading " << potential_datarefs.size() << " potential datarefs from " << filename << "\n";
    
    return potential_datarefs;
}


// + character is necessary for A320neo
inline char isValidDatarefChar(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || '_' == c || '/' == c || '-' == c || '.' == c || '+' == c;
}

const size_t min_dataref_length = 8;

std::vector<std::string> scanFileForDatarefStrings(std::ostream & log, const lb::filesystem::path & filename) {
    std::error_code ec;
    mio::mmap_source mmap_file = mio::make_mmap_source(filename.string(), ec);
    if(ec) {
        return {};
    }

    std::vector<std::string> all_refs;
    size_t start_pos = 0;
    const char * file_data = mmap_file.data();
    bool last_char_valid = false;
    for(size_t i = 0; i < mmap_file.size(); i++) {
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