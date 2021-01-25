#include <optional>
#include <thread>
#include <vector>

#include <filesystem.h>

#include "search/ref.h"
#include "synchronized_queue.h"

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

    SynchronizedQueue<ScanTaskMessage> task_queue;
    SynchronizedQueue<ScanResults> results_queue;

    // The ignore list must be stored. If we load a new aircraft, we need to try to 
    // load the ignore_list again as well in case the aircraft added new DR or CR.
    std::vector<std::string> ignore_list;

    void thread_proc();

public:
    ThreadedScanner();
    ~ThreadedScanner();

    std::optional<ScanResults> getResults() { return results_queue.get(); }

    void scanInitial();
    void scanAircraft(lb::filesystem::path aircraft_directory);
    void scanPlugin(lb::filesystem::path plugin_directory);
};