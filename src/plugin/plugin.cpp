#include <cstring>
#include <unordered_map>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>

#include "about_window.h"
#include "viewer_window.h"

#if IBM || LIN
#include "glew.h"
#endif

#include "logging.h"
#include "plugin.h"
#include "prefs.h"

#include "XPWidgets.h"
#include "XPLMDisplay.h"
#include "XPLMUtilities.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMPlanes.h"

#include "menu.h"


void load_acf_dr_callback();

void PluginData::aircraftIsBeingLoaded() {
    load_acf_dr_callback();
}

void PluginData::rescanDatarefs() {
    load_dr_flcb.scheduleNextFlightLoop();
}

std::optional<PluginData> plugin_data;

boost::filesystem::path getCurrentAircraftPath() {
    char filename[256] = {0};
    char path[512] = {0};
    XPLMGetNthAircraftModel(0, filename, path);
    return path;
}

//callback so we can load new aircraft datarefs when the aircraft is reloaded
void PluginData::load_acf_dr_callback() {
    scanner.scanAircraft(getCurrentAircraftPath());
}

void PluginData::update_dr_callback() {
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

    // Update values of all datarefs
    if(false == viewer_windows.empty()) {
        refs.update();
    }

    // Update results windows
    for(std::unique_ptr<ViewerWindow> & window: viewer_windows) {
        window->update();
    }
}

void PluginData::load_dr_callback() {
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

        scanner.scanPlugin(path);

        xplog << "Found plugin with name=\"" << name << "\" desc=\"" << description << "\" signature=\"" << signature << "\"";
    }

    scanner.scanAircraft(getCurrentAircraftPath());
}

namespace std {
    template<> struct hash<boost::filesystem::path> {
        size_t operator()(const boost::filesystem::path& p) const { 
            return boost::filesystem::hash_value(p); 
        }
    };
}

typedef std::unordered_map<boost::filesystem::path, std::time_t> plugin_last_modified_t;
plugin_last_modified_t plugin_last_modified;

void PluginData::plugin_changed_check_callback() {
    const char * xplane_plugin_path = "laminar.xplane.xplane";
    int num_plugins = XPLMCountPlugins();
    char plugin_path_array[256 + 1];
    for(int plugin_ix = 0; plugin_ix < num_plugins; plugin_ix++) {
        XPLMPluginID plugin = XPLMGetNthPlugin(plugin_ix);
        XPLMGetPluginInfo(plugin, nullptr, plugin_path_array, nullptr, nullptr);

        boost::filesystem::path plugin_path(plugin_path_array);
        if(xplane_plugin_path == plugin_path) {
            continue;
        }

        std::time_t modification_date;
        try {
            // We can't use boost::filesystem::canonical because it will make the path invalid. It doesn't seem to work properly
            // with windows paths.
            plugin_path = plugin_path.make_preferred();
            modification_date = boost::filesystem::last_write_time(plugin_path);
        } catch (boost::filesystem::filesystem_error & ec) {
            xplog << "Error reading modification date. Msg: " << ec.what() << " file:" << plugin_path << "\n";
            continue;
        }
        plugin_last_modified_t::iterator plugin_entry_it = plugin_last_modified.find(plugin_path);
        if(plugin_last_modified.end() == plugin_entry_it) {	// First sighting of this plugin; 
            plugin_last_modified.insert(std::make_pair<boost::filesystem::path, std::time_t>(std::move(plugin_path), std::move(modification_date)));
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

void reloadAircraft() {
    char acf_path[2048], acf_filename[1024];
    XPLMGetNthAircraftModel(0, acf_filename, acf_path);
    XPLMSetUsersAircraft(acf_path);
}

PluginData::~PluginData() {
    {
        char system_path_c[1000];
        XPLMGetSystemPath(system_path_c);
        boost::filesystem::path system_path(system_path_c);
        boost::filesystem::path output_dir = system_path / "Output" / "preferences";

        refs.saveToFile(output_dir / "drt_last_run_datarefs.txt", output_dir / "drt_last_run_commandrefs.txt");
    }

    closeAboutWindow();

    if(savePrefs(prefs_path, plugin_data->getViewerWindowsDetails())) {
        xplog << "Prefs saved to " << prefs_path << "\n";
    }
}


void PluginData::showViewerWindow(bool show_dr, bool show_cr) {
    viewer_windows.push_back(std::make_unique<ViewerWindow>(show_dr, show_cr, refs));
}
void PluginData::showViewerWindow(const nlohmann::json & window_details) {
    viewer_windows.push_back(std::make_unique<ViewerWindow>(window_details, refs));
}

XPLMCommandRef reload_aircraft_command = nullptr;
XPLMCommandRef reload_plugins_command = nullptr;
XPLMCommandRef reload_scenery_command = nullptr;
XPLMCommandRef show_datarefs_command = nullptr;

int command_handler(XPLMCommandRef command, XPLMCommandPhase phase, void * ) {
    if(xplm_CommandBegin == phase) {
        if(command == reload_aircraft_command) {
            reloadAircraft();
        } else if(command == reload_plugins_command) {
            XPLMReloadPlugins();
        } else if(command == reload_scenery_command) {
            XPLMReloadScenery();
        } else if(command == show_datarefs_command) {
            plugin_data->showViewerWindow();
        }
    }
    return 1;
}

const char * dre_signature = "xplanesdk.examples.DataRefEditor";
const char * dre_name = "DataRefEditor";
const char * dre_description = "A plugin that shows all data refs!.";

PluginData::PluginData() 
: load_dr_flcb(std::bind(&PluginData::load_dr_callback, this))
, update_dr_flcb(std::bind(&PluginData::update_dr_callback, this))
, plugin_changed_flcb(std::bind(&PluginData::plugin_changed_check_callback, this))
, load_acf_dr_flcb(std::bind(&PluginData::load_acf_dr_callback, this))
, refs(xplog) {
    load_dr_flcb.scheduleNextFlightLoop();
    update_dr_flcb.scheduleEveryFlightLoop();
    plugin_changed_flcb.schedule(1.f, 1.f);

    {
        char prefs_dir_c[512];
        XPLMGetPrefsPath(prefs_dir_c);
        prefs_path = boost::filesystem::path(prefs_dir_c).parent_path() / "datareftool.json";
    }
}

PLUGIN_API int XPluginStart(char * outName, char * outSig, char * outDesc) {

    xplog.setPrefix("DataRefTool: ");

    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
    XPLMEnableFeature("XPLM_USE_NATIVE_WIDGET_WINDOWS", 1);

#if IBM || LIN
    glewInit();
#endif

    plugin_data.emplace();
    if(loadPrefs(plugin_data->getPrefsPath())) {
        xplog << "prefs loaded from " << plugin_data->getPrefsPath() << "\n";
    }

    // let's try to find DRE before we register the plugin. If it's already here, we shouldn't register with the same signature!
    bool found_dre_early = XPLM_NO_PLUGIN_ID != XPLMFindPluginBySignature(dre_signature);
    if(found_dre_early && getImpersonateDRE()) {
        xplog << "Impersonating DataRefEditor failed, because DataRefEditor is currently running.\n";
    }
    if(false == found_dre_early && getImpersonateDRE()) {
        strcpy(outName, dre_name);
        strcpy(outSig, dre_signature);
        strcpy(outDesc, dre_description);
        xplog << "Impersonating DataRefEditor\n";
    } else {
        strcpy(outName, "DataRefTool");
        strcpy(outSig, "com.leecbaker.datareftool");
        strcpy(outDesc, "View and edit X-Plane Datarefs");
    }

    //commands
    reload_aircraft_command = XPLMCreateCommand("datareftool/reload_aircraft", "Reload the current aircraft");
    reload_plugins_command = XPLMCreateCommand("datareftool/reload_plugins", "Reload all plugins");
    reload_scenery_command = XPLMCreateCommand("datareftool/reload_scenery", "Reload the scenery");
    show_datarefs_command = XPLMCreateCommand("datareftool/show_datarefs", "Show the dataref search window");

    XPLMRegisterCommandHandler(reload_aircraft_command, command_handler, 0, nullptr);
    XPLMRegisterCommandHandler(reload_plugins_command, command_handler, 0, nullptr);
    XPLMRegisterCommandHandler(reload_scenery_command, command_handler, 0, nullptr);
    XPLMRegisterCommandHandler(show_datarefs_command, command_handler, 0, nullptr);

    plugin_data->updateMenu();
    
    return 1;
}

PLUGIN_API void	XPluginStop(void) {
    plugin_data.reset();
}

PLUGIN_API void XPluginDisable(void) {
}

PLUGIN_API int XPluginEnable(void) {
    plugin_data->updateMenu();
    return 1;
}

const intptr_t MSG_ADD_DATAREF = 0x01000000;
const intptr_t MSG_ADD_COMMANDREF = 0x01000099;

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID, intptr_t inMessage, void * inParam) {
    plugin_data->handleMessage(inMessage, inParam);
}

void PluginData::handleMessage(intptr_t inMessage, void * inParam) {
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
#ifdef XPLM301
        case XPLM_MSG_ENTERED_VR:
            for(const std::unique_ptr<ViewerWindow> & window : viewer_windows) {
                window->setInVr(true);
            }
            break;
        case XPLM_MSG_EXITING_VR:
            for(const std::unique_ptr<ViewerWindow> & window : viewer_windows) {
                window->setInVr(false);
            }
            break;
#endif
    }
}

void PluginData::closeViewerWindow(const ViewerWindow * window) {
    auto ptr_up_equal = [window](const std::unique_ptr<ViewerWindow> & up) -> bool {
        return up.get() == window;
    };

    std::vector<std::unique_ptr<ViewerWindow>>::const_iterator window_ptr_loc = std::find_if(viewer_windows.cbegin(), viewer_windows.cend(), ptr_up_equal);

    if(viewer_windows.cend() != window_ptr_loc) {
        viewer_windows.erase(window_ptr_loc);
    }
}

nlohmann::json PluginData::getViewerWindowsDetails() {
    nlohmann::json windows = nlohmann::json::array();
    
    for(const std::unique_ptr<ViewerWindow> & pwindow : viewer_windows) {
        windows.push_back(pwindow->to_json());
    }
    
    return windows;
}