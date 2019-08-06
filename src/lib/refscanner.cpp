#include "refscanner.h"

#include "dataref_files.h"
#include "find_datarefs_in_files.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/range/iterator_range.hpp>

void RefScanner::loadBlacklist(const boost::filesystem::path & blacklist_path) {
    blacklisted_datarefs = loadBlacklistFile(log, blacklist_path);
}

///re-add the blacklisted datarefs in case a new plugin was loaded
void RefScanner::addBlacklist() {
    std::vector<RefRecord *> bl_refs = add(blacklisted_datarefs, ref_src_t::BLACKLIST);
    if(false == bl_refs.empty()) {
        log << bl_refs.size() << " datarefs from blacklist opened successfully.\n";
    }
}

void RefScanner::scanAircraft(const boost::filesystem::path & path) {
    std::vector<std::string> aircraft_datarefs = getDatarefsFromAircraft(log, path);

    addBlacklist();
    std::vector<RefRecord *> acf_refs = add(aircraft_datarefs, ref_src_t::AIRCRAFT);
    log << "Found " << aircraft_datarefs.size() << " possible datarefs from aircraft files; " << acf_refs.size() << " commandrefs and datarefs OK.\n";

    addNewRefsThisFrame(acf_refs.cbegin(), acf_refs.cend());
}

void RefScanner::scanInitial(const std::vector<boost::filesystem::path> & plugin_paths, const boost::filesystem::path & aircraft_path) {
    addBlacklist();

    char system_path_c[1000];
    XPLMGetSystemPath(system_path_c);
    boost::filesystem::path system_path(system_path_c);
    std::vector<RefRecord *> dr_file_refs, cr_file_refs;

    {
        std::vector<std::string> dr_file = loadDatarefsFile(log, system_path / "Resources" / "plugins" / "DataRefs.txt");
        dr_file_refs = add(dr_file, ref_src_t::FILE);
        log << dr_file_refs.size() << " datarefs from DataRefs.txt opened successfully.\n";
    }

    {
        std::vector<std::string> cr_file = loadDatarefsFile(log, system_path / "Resources" / "plugins" / "Commands.txt");
        cr_file_refs = add(cr_file, ref_src_t::FILE);
        log << cr_file_refs.size() << " datarefs from Commands.txt opened successfully.\n";
    }

    scanAircraft(aircraft_path);

    //load plugins
    std::vector<std::string> all_plugin_datarefs;

    for(const boost::filesystem::path & plugin_path: plugin_paths) {
        std::vector<std::string> this_plugin_datarefs = getDatarefsFromFile(log, plugin_path);
        all_plugin_datarefs.insert(all_plugin_datarefs.end(), this_plugin_datarefs.begin(), this_plugin_datarefs.end());
    }

    { //FWL directory
        char xplane_dir[1024];
        XPLMGetSystemPath(xplane_dir);

        boost::filesystem::path fwl_scripts_dir = boost::filesystem::path(xplane_dir) / "Resources" / "plugins" / "FlyWithLua" / "Scripts";

        if(boost::filesystem::is_directory(fwl_scripts_dir)) { // if this is true, it also exists
            for(auto& file_de : boost::make_iterator_range(boost::filesystem::directory_iterator(fwl_scripts_dir), {})) {
                if(file_de.path().extension() == ".lua") {
                    std::vector<std::string> this_script_datarefs = getDatarefsFromFile(log, file_de.path());
                    all_plugin_datarefs.insert(all_plugin_datarefs.cend(), this_script_datarefs.begin(), this_script_datarefs.end());
                }
            }
        }
    }

    removeVectorUniques(all_plugin_datarefs);

    std::vector<RefRecord *> plugin_refs = add(all_plugin_datarefs, ref_src_t::PLUGIN);
    log << "Found " << all_plugin_datarefs.size() << " possible datarefs from plugin files; " << plugin_refs.size() << " datarefs and commands loaded OK.\n";
    
    addNewRefsThisFrame(cr_file_refs.cbegin(), cr_file_refs.cend());
    addNewRefsThisFrame(dr_file_refs.cbegin(), dr_file_refs.cend());
    addNewRefsThisFrame(plugin_refs.cbegin(), plugin_refs.cend());
}