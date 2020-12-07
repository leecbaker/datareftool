#include "drt_plugin.h"

#include "logging.h"
#include "prefs.h"

#include <cstring>
#include <memory>

#include "ui/about_window.h"
#include "ui/search_window.h"

#include "filesystem.h"

#include "XPLMPlanes.h"
#include "XPLMPlugin.h"

#include <boost/functional/hash.hpp>
#include <filesystem.h>

using namespace std::string_literals;

std::unique_ptr<DRTPlugin> plugin;

lb::filesystem::path getCurrentAircraftPath() {
    char filename[256] = {0};
    char path[512] = {0};
    XPLMGetNthAircraftModel(0, filename, path);
    return path;
}


void DRTPlugin::aircraftIsBeingLoaded() {
    load_acf_dr_callback();
}

//callback so we can load new aircraft datarefs when the aircraft is reloaded
void DRTPlugin::load_acf_dr_callback() {
    scanner.scanAircraft(getCurrentAircraftPath());
}

void DRTPlugin::rescanDatarefs() {
    load_dr_flcb.scheduleNextFlightLoop();
}

void DRTPlugin::update_dr_callback() {
    // Collect new datarefs
    while(true) {
        std::optional<ScanResults> results = scanner.getResults();
        if(results) {
            // Check if the strings represent commands or datarefs or nothing
            std::vector<RefRecord *> new_refs = refs.add(results->strings, results->source);

            // Add them to the list of updated this frame, so the UI is updated
            refs.addNewRefsThisFrame(new_refs.cbegin(), new_refs.cend());
        } else {
            break;
        }
    }

    // remove expired windows
    auto new_end = std::remove_if(search_window_refs.begin(), search_window_refs.end(), [](std::weak_ptr<SearchWindow> & wpw) -> bool { return wpw.expired(); });
    search_window_refs.erase(new_end, search_window_refs.end());

    // Update values of all datarefs
    if(false == search_window_refs.empty()) {
        refs.update();
    }

    // Update results windows
    for(std::weak_ptr<SearchWindow> & window_weak: search_window_refs) {
        std::shared_ptr<SearchWindow> window_lock = window_weak.lock();
        if(window_lock) {
            window_lock->update();
        }
    }
}

void DRTPlugin::load_dr_callback() {
    int num_plugins = XPLMCountPlugins();

    scanner.scanInitial();

    for(int i = 0; i < num_plugins; i++) {
        XPLMPluginID plugin_id = XPLMGetNthPlugin(i);
        if(0 == plugin_id) {
            continue;
        }

        char name[256] = {0};
        char path[512] = {0};
        char signature[512] = {0};
        char description[512] = {0};
        XPLMGetPluginInfo(plugin_id, name, path, signature, description);

        //find plugin directory path
        lb::filesystem::path xpl_path(path);
        lb::filesystem::path xpl_dir = xpl_path.parent_path();
        lb::filesystem::path plugin_dir = xpl_dir;
        std::string parent_dir_name = xpl_dir.filename().string();
        if(parent_dir_name == "64" || parent_dir_name == "mac_x64" || parent_dir_name == "win_x64" || parent_dir_name == "lin_x64") {
            plugin_dir = plugin_dir.parent_path();
        }

        scanner.scanPlugin(plugin_dir);

        xplog << "Found plugin with name=\"" << name << "\" desc=\"" << description << "\" signature=\"" << signature << "\"";
    }

    scanner.scanAircraft(getCurrentAircraftPath());
}

DRTPlugin::DRTPlugin()
: load_dr_flcb(std::bind(&DRTPlugin::load_dr_callback, this))
, update_dr_flcb(std::bind(&DRTPlugin::update_dr_callback, this))
, plugin_changed_flcb(std::bind(&DRTPlugin::plugin_changed_check_callback, this))
, load_acf_dr_flcb(std::bind(&DRTPlugin::load_acf_dr_callback, this))
, refs(xplog_debug) {
    {
        char prefs_dir_c[512];
        XPLMGetPrefsPath(prefs_dir_c);
        prefs_path = lb::filesystem::path(prefs_dir_c).parent_path() / "datareftool.json";
    }

    loadPrefs();

    load_dr_flcb.scheduleNextFlightLoop();
    update_dr_flcb.scheduleEveryFlightLoop();
    if(getAutoReloadPlugins()) {
        plugin_changed_flcb.schedule(1.f, 1.f);
    }
}

void DRTPlugin::loadPrefs() {
    std::function<void(const nlohmann::json &)> create_window_func = [this](const nlohmann::json & window_params){ openSearchWindow(window_params); };
    if(::loadPrefs(prefs_path, create_window_func)) {
        xplog << "Prefs loaded from " << prefs_path << "\n";
    } else {
        xplog << "Failed to load prefs from " << prefs_path << "\n";
    }

    setDebugMode(getDebugMode());

    menu.update();
}

void DRTPlugin::setDebugMode(bool debug_mode) {
    if(debug_mode == getDebugMode()) {
        return;
    }
    ::setDebugMode(debug_mode);

    xplog_debug.setEnabled(debug_mode);

    if(debug_mode) {
        xplog << "Debug mode enabled (more logging). To disable debug mode:\n";
    } else {
        xplog << "Debug mode disabled. To enable debug mode:\n";
    }

    xplog << "* Set dataref leecbaker/drt/debug, and restart the plugin; or\n";
    xplog << "* Change the preferences file at " << prefs_path << "\n";
}

void DRTPlugin::savePrefs() {
    std::vector<nlohmann::json> window_params;
    for(const std::weak_ptr<SearchWindow> & window_ref: search_window_refs) {
        const std::shared_ptr<SearchWindow> window = window_ref.lock();
        window_params.push_back(dumpSearchWindow(window));
    }

    if(::savePrefs(prefs_path, window_params)) {
        xplog << "Prefs saved to " << prefs_path << "\n";
    } else {
        xplog << "Failed to save prefs to " << prefs_path << "\n";
    }
}

DRTPlugin::~DRTPlugin() {
    savePrefs();
}

void DRTPlugin::openAboutWindow() {
    std::shared_ptr<AboutWindow> about_window = AboutWindow::make();
    about_window->show();
    this->about_window_ref = std::move(about_window);
}

std::shared_ptr<SearchWindow> DRTPlugin::openSearchWindow() {
    std::shared_ptr<SearchWindow> new_window = SearchWindow::make(std::ref(refs));
    new_window->show();
    search_window_refs.push_back(new_window);
    return new_window;
}

const char * WINDOW_PARAMS_SEARCH_TERM = "search_term";
const char * WINDOW_PARAMS_REGEX = "regex";
const char * WINDOW_PARAMS_CASE_SENSITIVE = "case_sensitive";
const char * WINDOW_PARAMS_COMMANDS_DATAREFS = "datarefs_commands";
const char * WINDOW_PARAMS_COMMANDS_DATAREFS_ALL = "all";
const char * WINDOW_PARAMS_COMMANDS_DATAREFS_COMMANDS = "commands";
const char * WINDOW_PARAMS_COMMANDS_DATAREFS_DATAREFS = "datarefs";
const char * WINDOW_PARAMS_CHANGE_FILTER = "change_filter";
const char * WINDOW_PARAMS_CHANGE_FILTER_ALL = "all";
const char * WINDOW_PARAMS_CHANGE_FILTER_ALL_CHANGES = "all_changes";
const char * WINDOW_PARAMS_CHANGE_FILTER_BIG_CHANGES = "big_changes";
const char * WINDOW_PARAMS_POSITION_LEFT = "pos_left";
const char * WINDOW_PARAMS_POSITION_TOP = "pos_top";
const char * WINDOW_PARAMS_POSITION_RIGHT = "pos_right";
const char * WINDOW_PARAMS_POSITION_BOTTOM = "pos_bottom";

std::shared_ptr<SearchWindow> DRTPlugin::openSearchWindow(const nlohmann::json & window_params) {
    std::shared_ptr<SearchWindow> new_window = openSearchWindow();

    try {
        std::string search_term = window_params[WINDOW_PARAMS_SEARCH_TERM].get<std::string>();
        new_window->setSearchTerm(search_term);
    } catch(...) {}

    try {
        bool is_case_sensitive = window_params[WINDOW_PARAMS_CASE_SENSITIVE].get<bool>();
        new_window->setIsCaseSensitive(is_case_sensitive);
    } catch(...) {
        new_window->setIsCaseSensitive(false);
    }

    try {
        bool is_regex = window_params[WINDOW_PARAMS_REGEX].get<bool>();
        new_window->setUseRegex(is_regex);
    } catch(...) {
        new_window->setUseRegex(false);
    }

    try {
        std::string change_filter_str = window_params[WINDOW_PARAMS_CHANGE_FILTER].get<std::string>();
        ChangeFilterType change_filter = ChangeFilterType::ALL;
        if(WINDOW_PARAMS_CHANGE_FILTER_ALL_CHANGES == change_filter_str) {
            change_filter = ChangeFilterType::ALL_CHANGES;
        } else if(WINDOW_PARAMS_CHANGE_FILTER_BIG_CHANGES == change_filter_str) {
            change_filter = ChangeFilterType::BIG_CHANGES;
        }
        new_window->setChangeFilter(change_filter);
    } catch(...) {
        new_window->setChangeFilter(ChangeFilterType::ALL);
    }

    if(window_params.find(WINDOW_PARAMS_COMMANDS_DATAREFS) != window_params.end()) {
        try {
            std::string datarefs_commands = window_params[WINDOW_PARAMS_COMMANDS_DATAREFS].get<std::string>();
            CommandDatarefFilterType c_dr_filter = CommandDatarefFilterType::ALL;
            if(WINDOW_PARAMS_COMMANDS_DATAREFS_COMMANDS == datarefs_commands) {
                c_dr_filter = CommandDatarefFilterType::COMMAND;
            } else if(WINDOW_PARAMS_COMMANDS_DATAREFS_DATAREFS == datarefs_commands) {
                c_dr_filter = CommandDatarefFilterType::DATAREF;
            }
            new_window->setCommandDatarefFilter(c_dr_filter);
        } catch(...) {
            new_window->setCommandDatarefFilter(CommandDatarefFilterType::ALL);
        }
    } else {
        new_window->setCommandDatarefFilter(CommandDatarefFilterType::ALL);
    }

    if(window_params.find(WINDOW_PARAMS_POSITION_LEFT) != window_params.end() &&
        window_params.find(WINDOW_PARAMS_POSITION_TOP) != window_params.end() &&
        window_params.find(WINDOW_PARAMS_POSITION_RIGHT) != window_params.end() &&
        window_params.find(WINDOW_PARAMS_POSITION_BOTTOM) != window_params.end()) {
        Rect bounds{
            window_params[WINDOW_PARAMS_POSITION_LEFT].get<int>(),
            window_params[WINDOW_PARAMS_POSITION_TOP].get<int>(),
            window_params[WINDOW_PARAMS_POSITION_RIGHT].get<int>(),
            window_params[WINDOW_PARAMS_POSITION_BOTTOM].get<int>(),
        };

        new_window->setWindowBounds(bounds);
    }

    new_window->updateSearch();


    return new_window;
}

nlohmann::json DRTPlugin::dumpSearchWindow(const std::shared_ptr<SearchWindow> & search_window) const {
    nlohmann::json window_params;

    window_params[WINDOW_PARAMS_SEARCH_TERM] = search_window->getSearchTerm();
    window_params[WINDOW_PARAMS_REGEX] = search_window->getUseRegex();
    window_params[WINDOW_PARAMS_CHANGE_FILTER] = search_window->getChangeFilter();
    window_params[WINDOW_PARAMS_CASE_SENSITIVE] = search_window->getIsCaseSensitive();

    const Rect window_position = search_window->getWindowBounds();
    window_params[WINDOW_PARAMS_POSITION_LEFT] = window_position.left;
    window_params[WINDOW_PARAMS_POSITION_TOP] = window_position.top;
    window_params[WINDOW_PARAMS_POSITION_RIGHT] = window_position.right;
    window_params[WINDOW_PARAMS_POSITION_BOTTOM] = window_position.bottom;

    switch(search_window->getCommandDatarefFilter()) {
        case CommandDatarefFilterType::ALL:
            window_params[WINDOW_PARAMS_COMMANDS_DATAREFS] = WINDOW_PARAMS_COMMANDS_DATAREFS_ALL;
            break;
        case CommandDatarefFilterType::COMMAND:
            window_params[WINDOW_PARAMS_COMMANDS_DATAREFS] = WINDOW_PARAMS_COMMANDS_DATAREFS_COMMANDS;
            break;
        case CommandDatarefFilterType::DATAREF:
            window_params[WINDOW_PARAMS_COMMANDS_DATAREFS] = WINDOW_PARAMS_COMMANDS_DATAREFS_DATAREFS;
            break;
    }

    switch(search_window->getChangeFilter()) {
        case ChangeFilterType::ALL:
            window_params[WINDOW_PARAMS_CHANGE_FILTER] = WINDOW_PARAMS_CHANGE_FILTER_ALL;
            break;
        case ChangeFilterType::ALL_CHANGES:
            window_params[WINDOW_PARAMS_CHANGE_FILTER] = WINDOW_PARAMS_CHANGE_FILTER_ALL_CHANGES;
            break;
        case ChangeFilterType::BIG_CHANGES:
            window_params[WINDOW_PARAMS_CHANGE_FILTER] = WINDOW_PARAMS_CHANGE_FILTER_BIG_CHANGES;
            break;
    }

    return window_params;
}

void DRTPlugin::reloadAircraft() {
    char acf_path[2048], acf_filename[1024];
    XPLMGetNthAircraftModel(0, acf_filename, acf_path);
    XPLMSetUsersAircraft(acf_path);
}

void DRTPlugin::reloadPlugins() {
    savePrefs();
    XPLMReloadPlugins();
}

void DRTPlugin::reloadScenery() {
    XPLMReloadScenery();
}


const intptr_t MSG_ADD_DATAREF = 0x01000000;
const intptr_t MSG_ADD_COMMANDREF = 0x01000099;

void DRTPlugin::handleMessage(intptr_t inMessage, void * inParam) {
    switch(inMessage) {
        // Add custom datarefs in the style of DRE:
        // http://www.xsquawkbox.net/xpsdk/mediawiki/Register_Custom_DataRef_in_DRE
        case MSG_ADD_DATAREF:
            refs.addNewRefFromMessage(reinterpret_cast<char *>(inParam));
            break;
        case MSG_ADD_COMMANDREF:
            refs.addNewRefFromMessage(reinterpret_cast<char *>(inParam));
            break;
        case XPLM_MSG_PLANE_LOADED: {
            int64_t plane_num = int64_t(inParam);
            xplog << "Plane loaded #: " << plane_num << "\n";
            if(0 == plane_num) {	//user's plane
                aircraftIsBeingLoaded();
            }
            break;
        }

        case XPLM_MSG_WILL_WRITE_PREFS:
            break;
#if defined(XPLM301)
        case XPLM_MSG_ENTERED_VR:
            for(const std::weak_ptr<SearchWindow> & window: search_window_refs) {
                const std::shared_ptr<SearchWindow> window_locked = window.lock();
                if(window_locked) {
                    window_locked->setVR(true);
                }
            }
            break;
        case XPLM_MSG_EXITING_VR:
            for(const std::weak_ptr<SearchWindow> & window: search_window_refs) {
                const std::shared_ptr<SearchWindow> window_locked = window.lock();
                if(window_locked) {
                    window_locked->setVR(false);
                }
            }
            break;
#endif
    }
}

namespace std {
    template<> struct hash<lb::filesystem::path> {
        size_t operator()(const lb::filesystem::path& p) const { 
            return lb::filesystem::hash_value(p); 
        }
    };
}


typedef std::unordered_map<lb::filesystem::path, lb::filesystem::file_time_type> plugin_last_modified_t;
plugin_last_modified_t plugin_last_modified;

void DRTPlugin::plugin_changed_check_callback() {
    const char * xplane_plugin_path = "laminar.xplane.xplane";
    int num_plugins = XPLMCountPlugins();
    char plugin_path_array[256 + 1];
    for(int plugin_ix = 0; plugin_ix < num_plugins; plugin_ix++) {
        XPLMPluginID plugin_id = XPLMGetNthPlugin(plugin_ix);
        XPLMGetPluginInfo(plugin_id, nullptr, plugin_path_array, nullptr, nullptr);

        lb::filesystem::path plugin_path(plugin_path_array);
        if(xplane_plugin_path == plugin_path) {
            continue;
        }

        lb::filesystem::file_time_type modification_date;
        try {
            // We can't use lb::filesystem::canonical because it will make the path invalid. It doesn't seem to work properly
            // with windows paths.
            plugin_path = plugin_path.make_preferred();
            modification_date = lb::filesystem::last_write_time(plugin_path);
        } catch (lb::filesystem::filesystem_error & ec) {
            xplog << "Error reading modification date. Msg: " << ec.what() << " file:" << plugin_path << "\n";
            continue;
        }
        plugin_last_modified_t::iterator plugin_entry_it = plugin_last_modified.find(plugin_path);
        if(plugin_last_modified.end() == plugin_entry_it) {	// First sighting of this plugin; 
            plugin_last_modified.insert(std::make_pair<lb::filesystem::path, lb::filesystem::file_time_type>(std::move(plugin_path), std::move(modification_date)));
        } else {
            if(plugin_entry_it->second != modification_date) {
                plugin_entry_it->second = modification_date;
                if(getAutoReloadPlugins()) {
                    xplog << "Observed plugin with new modification (reloading):" << plugin_path << "\n";
                    XPLMReloadPlugins();
                }
            }
        }
    }
}

void DRTPlugin::rescan() {
    load_dr_flcb.scheduleNextFlightLoop();
}

void DRTPlugin::toggleImpersonateDRE() {
    setImpersonateDRE(!getImpersonateDRE());
    plugin->reloadPlugins();
}

void DRTPlugin::toggleAutoReloadPlugins() {
    setAutoReloadPlugins(!getAutoReloadPlugins());
    savePrefs();
    if(getAutoReloadPlugins()) {
        plugin_changed_flcb.schedule(1.f, 1.f);
    } else {
        plugin_changed_flcb.unschedule();
    }
}