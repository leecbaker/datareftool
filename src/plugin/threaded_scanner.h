#include <optional>
#include <thread>
#include <vector>

#include <filesystem.h>

#include "search/ref.h"
#include "../lb_common/lb_synchronized_queue.h"

struct ScanResults {
    ref_src_t source;
    std::vector<std::string> strings;
};

// Scan for datarefs from another thread.
class ThreadedScanner {
    enum class ScanMessageType {
        SCAN_INITIAL,
        SCAN_AIRCRAFT,
        SCAN_PLUGIN,
        QUIT
    };

    struct ScanTaskMessage {
        ScanMessageType type;
        lb::filesystem::path path;
    };

    std::thread worker_thread;

    LBSynchronizedQueue<ScanTaskMessage> task_queue;
    LBSynchronizedQueue<ScanResults> results_queue;

    // The ignore list must be stored. If we load a new aircraft, we need to try to 
    // load the ignore_list again as well in case the aircraft added new DR or CR.
    std::vector<std::string> ignore_list;

    // We need the system directory for some things; can't access that API from a worker
    // thread, so we need get a copy before starting the worker.
    lb::filesystem::path system_path;

    void thread_proc();

public:
    ThreadedScanner();
    ~ThreadedScanner();

    std::optional<ScanResults> getResults() { return results_queue.get(); }

    void scanInitial();
    void scanAircraft(lb::filesystem::path aircraft_directory);
    void scanPlugin(lb::filesystem::path plugin_directory);
};