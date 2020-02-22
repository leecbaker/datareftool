#include <optional>
#include <thread>

#include <boost/filesystem/path.hpp>

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
        boost::filesystem::path path;
    };

    std::thread worker_thread;

    SynchronizedQueue<ScanTaskMessage> task_queue;
    SynchronizedQueue<ScanResults> results_queue;

    // The blacklist must be stored. If we load a new aircraft, we need to try to 
    // load the blacklist again as well in case the aircraft added new DR or CR.
    std::vector<std::string> blacklist;

public:
    ThreadedScanner();
    ~ThreadedScanner();

    std::optional<ScanResults> getResults() { return results_queue.get(); }

    void scanInitial();
    void scanAircraft(boost::filesystem::path aircraft_directory);
    void scanPlugin(boost::filesystem::path plugin_directory);

    void thread_proc();
};