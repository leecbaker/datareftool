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

void ThreadedScanner::scanAircraft(boost::filesystem::path aircraft_directory) {
    task_queue.push(ScanTaskMessage{ScanMessageType::SCAN_AIRCRAFT, std::move(aircraft_directory)});
}

void ThreadedScanner::scanPlugin(boost::filesystem::path plugin_directory) {
    task_queue.push(ScanTaskMessage{ScanMessageType::SCAN_PLUGIN, std::move(plugin_directory)});
}

void ThreadedScanner::thread_proc() {
    while(true) {
        ScanTaskMessage message = task_queue.get_blocking();

        switch(message.type) {
            case ScanMessageType::SCAN_INITIAL: {
                char system_path_c[1000];
                XPLMGetSystemPath(system_path_c);
                boost::filesystem::path system_path(system_path_c);

                { // Blacklist
                    boost::filesystem::path blacklist_path = system_path / "Resources" / "plugins" / "drt_blacklist.txt";
                    blacklist = loadListFile(xplog, blacklist_path);
                    results_queue.push(ScanResults{ref_src_t::BLACKLIST, blacklist});
                }

                { // DR file
                    boost::filesystem::path dr_file = system_path / "Resources" / "plugins" / "DataRefs.txt";
                    std::vector<std::string> dr_refs = loadListFile(xplog, dr_file);
                    results_queue.push(ScanResults{ref_src_t::FILE, dr_refs});
                }

                { // CR file
                    boost::filesystem::path cr_file = system_path / "Resources" / "plugins" / "Commands.txt";
                    std::vector<std::string> cr_refs = loadListFile(xplog, cr_file);
                    results_queue.push(ScanResults{ref_src_t::FILE, cr_refs});
                }

                { // FWL
                    boost::filesystem::path fwl_scripts_dir = system_path / "Resources" / "plugins" / "FlyWithLua" / "Scripts";
                    std::vector<std::string> script_refs = scanLuaFolder(xplog, fwl_scripts_dir);
                    results_queue.push(ScanResults{ref_src_t::FILE, script_refs});
                }
            }
                break;
            case ScanMessageType::SCAN_PLUGIN: {
                results_queue.push(ScanResults{ref_src_t::BLACKLIST, blacklist});
                std::vector<std::string> plugin_refs = ::scanPluginFolder(xplog, message.path);
                results_queue.push(ScanResults{ref_src_t::PLUGIN, plugin_refs});
            }
                break;
            case ScanMessageType::SCAN_AIRCRAFT: {
                results_queue.push(ScanResults{ref_src_t::BLACKLIST, blacklist});
                std::vector<std::string> aircraft_refs = ::scanAircraft(xplog, message.path);
                results_queue.push(ScanResults{ref_src_t::AIRCRAFT, aircraft_refs});
            }
                break;
            case ScanMessageType::QUIT:
                return;
        }
    }
}
