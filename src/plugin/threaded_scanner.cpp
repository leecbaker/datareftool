#include "threaded_scanner.h"

#include "logging.h"

#include "scan/scan_entity.h"

#include "search/ref.h"

#include "XPLMUtilities.h"

ThreadedScanner::ThreadedScanner() {
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
    while(true) {
        ScanTaskMessage message = task_queue.get_blocking();

        switch(message.type) {
            case ScanMessageType::SCAN_INITIAL: {
                char system_path_c[1000];
                XPLMGetSystemPath(system_path_c);
                lb::filesystem::path system_path(system_path_c);

                { // Blacklist
                    lb::filesystem::path blacklist_path = system_path / "Resources" / "plugins" / "drt_blacklist.txt";
                    blacklist = loadListFile(xplog_debug, blacklist_path);

                    // Work around bug in X-Plane 11
                    blacklist.push_back("iphone/flightmodel/ground_status");
                    results_queue.push(ScanResults{ref_src_t::BLACKLIST, blacklist});
                }

                { // Scan X-Plane binary
#if __APPLE__
                    lb::filesystem::path xplane_binary = system_path / "X-Plane.app" / "Contents" / "MacOS" / "X-Plane";
#elif _WIN32
                    lb::filesystem::path xplane_binary = system_path / "X-Plane.exe";
#else
                    lb::filesystem::path xplane_binary = system_path / "X-Plane";
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
                    std::vector<std::string> script_refs = scanLuaFolder(xplog_debug, fwl_scripts_dir);
                    results_queue.push(ScanResults{ref_src_t::FILE, script_refs});
                }
            }
                break;
            case ScanMessageType::SCAN_PLUGIN: {
                results_queue.push(ScanResults{ref_src_t::BLACKLIST, blacklist});
                std::vector<std::string> plugin_refs = ::scanPluginFolder(xplog_debug, message.path);
                results_queue.push(ScanResults{ref_src_t::PLUGIN, plugin_refs});
            }
                break;
            case ScanMessageType::SCAN_AIRCRAFT: {
                results_queue.push(ScanResults{ref_src_t::BLACKLIST, blacklist});
                std::vector<std::string> aircraft_refs = ::scanAircraft(xplog_debug, message.path);
                results_queue.push(ScanResults{ref_src_t::AIRCRAFT, aircraft_refs});
            }
                break;
            case ScanMessageType::QUIT:
                return;
        }
    }
}
