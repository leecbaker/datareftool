#include "threaded_scanner.h"

#include "scan/deduplicate_vector.h"

#include "internal_dataref_list.h"

#include "logging.h"

#include "scan/scan_entity.h"

#include "search/ref.h"

#include "XPLMUtilities.h"

ThreadedScanner::ThreadedScanner() {

    {
        char system_path_c[1000];
        XPLMGetSystemPath(system_path_c);
        system_path = lb::filesystem::path(system_path_c);
    }

    worker_thread = std::thread([this]() {
        this->thread_proc();
    });
}

ThreadedScanner::~ThreadedScanner() {
    task_queue.push(ScanTaskMessage{ScanMessageType::QUIT, {}});
    worker_thread.join();
}

void ThreadedScanner::scanInitial() {
    task_queue.push(ScanTaskMessage{ScanMessageType::SCAN_INITIAL, {}});
}

void ThreadedScanner::scanAircraft(lb::filesystem::path aircraft_directory) {
    task_queue.push(ScanTaskMessage{ScanMessageType::SCAN_AIRCRAFT, std::move(aircraft_directory)});
}

void ThreadedScanner::scanPlugin(lb::filesystem::path plugin_directory) {
    task_queue.push(ScanTaskMessage{ScanMessageType::SCAN_PLUGIN, std::move(plugin_directory)});
}

void ThreadedScanner::thread_proc() {
    { 
        // Ignore list. This file was previous called blacklist, so that filename
        // is included for backwards compatibility. These files should be scanned before anything else,
        // so I am scanning them before actually processing messages at all. This should happen as soon as the plugin is loaded.
        lb::filesystem::path blacklist_path = system_path / "Resources" / "plugins" / "drt_blacklist.txt";
        std::vector<std::string> ignore_list_1 = loadListFile(xplog_debug, blacklist_path);
        ignore_list.insert(ignore_list.end(), ignore_list_1.cbegin(), ignore_list_1.cend());

        lb::filesystem::path ignorelist_path = system_path / "Resources" / "plugins" / "drt_ignore.txt";
        std::vector<std::string> ignore_list_2 = loadListFile(xplog_debug, ignorelist_path);
        ignore_list.insert(ignore_list.end(), ignore_list_2.cbegin(), ignore_list_2.cend());

        // Work around bug in X-Plane 11
        ignore_list.push_back("iphone/flightmodel/ground_status");
        deduplicate_vector(ignore_list);
        results_queue.push(ScanResults{ref_src_t::IGNORE_FILE, ignore_list});
        xplog_debug << "Found " << ignore_list.size() << " entries to be ignored.\n";
    }

    while(true) {
        ScanTaskMessage message = task_queue.get_blocking();

        switch(message.type) {
            case ScanMessageType::SCAN_INITIAL: {
                { // Scan X-Plane binary
#if __APPLE__
                    lb::filesystem::path xplane_binary = system_path / "X-Plane.app" / "Contents" / "MacOS" / "X-Plane";
#elif _WIN32
                    lb::filesystem::path xplane_binary = system_path / "X-Plane.exe";
#else
                    lb::filesystem::path xplane_binary = system_path / "X-Plane-x86_64";
#endif
                    if(lb::filesystem::exists(xplane_binary)) {
                        std::vector<std::string> xplane_refs = scanXplaneBinary(xplog_debug, xplane_binary);
                        results_queue.push(ScanResults{ref_src_t::X_PLANE, xplane_refs});
                    } else {
                        xplog_debug << "Expected to find X-Plane binary at " << xplane_binary << ", but no file was found at that location.";
                    }
                }

                { // DR file
                    lb::filesystem::path dr_file = system_path / "Resources" / "plugins" / "DataRefs.txt";
                    std::vector<std::string> dr_refs = loadListFile(xplog_debug, dr_file);
                    results_queue.push(ScanResults{ref_src_t::FILE, dr_refs});
                }

                { // CR file
                    lb::filesystem::path cr_file = system_path / "Resources" / "plugins" / "Commands.txt";
                    std::vector<std::string> cr_refs = loadListFile(xplog_debug, cr_file);
                    results_queue.push(ScanResults{ref_src_t::FILE, cr_refs});
                }

                { // FWL
                    lb::filesystem::path fwl_scripts_dir = system_path / "Resources" / "plugins" / "FlyWithLua" / "Scripts";
                    const std::vector<std::string> & script_refs = scanLuaFolder(xplog_debug, fwl_scripts_dir);
                    results_queue.push(ScanResults{ref_src_t::LUA, script_refs});
                }

                { // internal list
                    const std::vector<std::string> & internal_list = getInternalList();
                    results_queue.push(ScanResults{ref_src_t::DRT_INTERNAL_LIST, internal_list});
                }
            }
                break;
            case ScanMessageType::SCAN_PLUGIN: {
                results_queue.push(ScanResults{ref_src_t::IGNORE_FILE, ignore_list});
                std::vector<std::string> plugin_refs = ::scanPluginFolder(xplog_debug, message.path);
                results_queue.push(ScanResults{ref_src_t::PLUGIN, plugin_refs});
            }
                break;
            case ScanMessageType::SCAN_AIRCRAFT: {
                results_queue.push(ScanResults{ref_src_t::IGNORE_FILE, ignore_list});
                std::vector<std::string> aircraft_refs = ::scanAircraft(xplog_debug, message.path);
                results_queue.push(ScanResults{ref_src_t::AIRCRAFT, aircraft_refs});
            }
                break;
            case ScanMessageType::QUIT:
                return;
        }
    }
}
