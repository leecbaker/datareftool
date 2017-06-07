#include <cstring>
#include <unordered_map>
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp> 

#include "about_window.h"
#include "viewer_window.h"
#include "find_datarefs_in_files.h"

#include "datarefs.h"
#include "dataref_files.h"

#include "prefs.h"

#include "XPWidgets.h"
#include "XPLMMenus.h"
#include "XPLMDisplay.h"
#include "XPLMUtilities.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMPlanes.h"

boost::filesystem::path prefs_path;

XPLMMenuID plugin_menu = nullptr;

boost::optional<DataRefRecords> datarefs;
std::vector<std::string> blacklisted_datarefs;

std::vector<std::string> commandrefs;
using commands_storage = std::unordered_map<std::string, XPLMCommandRef>;
commands_storage commands;

size_t addPotentialCommandrefs(const std::vector<std::string> & new_commands) {
	size_t success_count = 0;
	for(const std::string & name : new_commands) {
		commands_storage::const_iterator cr_location = commands.find(name);

		if(commands.cend() == cr_location) {
			XPLMCommandRef cr = XPLMFindCommand(name.c_str());
			if(nullptr != cr) {
				commands.insert(std::make_pair(name, cr));
				success_count++;
			}
		}
	}

	{    
		char system_path_c[1000];
    	XPLMGetSystemPath(system_path_c);
    	boost::filesystem::path system_path(system_path_c);

		boost::filesystem::path output_path = system_path / "Output" / "preferences" / "drt_last_run_commandrefs.txt";
		std::vector<const std::string *> sorted_names;
		sorted_names.reserve(commands.size());
		for(std::pair<const std::string, XPLMCommandRef> & command : commands) {
			sorted_names.emplace_back(&command.first);
		}
		auto p_str_comparator = [](const std::string * s1, const std::string * s2) -> bool {
			return boost::ilexicographical_compare(*s1, *s2);
		};
		std::sort(sorted_names.begin(), sorted_names.end(), p_str_comparator);

		std::ofstream f(output_path.string());
		for(const std::string * pstr : sorted_names) {
			f << *pstr << "\n";
		}
	}

	return success_count;
}

void loadAircraftDatarefs() {
	//get path
	char filename[256] = {0};
	char path[512] = {0};
	XPLMGetNthAircraftModel(0, filename, path);
	std::vector<std::string> aircraft_datarefs = getDatarefsFromAircraft(path);

	int loaded_ok = datarefs->add(aircraft_datarefs, dataref_src_t::AIRCRAFT);
	const std::string message = std::string("DRT: Found ") + std::to_string(aircraft_datarefs.size()) + std::string(" possible datarefs from aircraft files; " + std::to_string(loaded_ok) + " loaded OK.\n");
	XPLMDebugString(message.c_str());
}


//callback so we can load new aircraft datarefs when the aircraft is reloaded
float load_acf_dr_callback(float, float, int, void *) {
	std::cerr << "load acf callback running" << std::endl;
	{ // re-add the blacklisted datarefs in case a new plugin was loaded. needed for, eg, x737
		int success_count = datarefs->add(blacklisted_datarefs, dataref_src_t::BLACKLIST);
		std::string success_message = "DRT: " + std::to_string(success_count) + " datarefs from blacklist opened successfully.\n";
	}
	loadAircraftDatarefs();

    updateWindowsAsDatarefsAdded();

	return 0; 
}

float update_dr_callback(float, float, int, void *) {
	if(datarefs) {
		datarefs->update();
	}

	return -1.f; 
}

float load_dr_callback(float, float, int, void *) {
	{	//re-add the blacklisted datarefs in case a new plugin was loaded
		int success_count = datarefs->add(blacklisted_datarefs, dataref_src_t::BLACKLIST);
		std::string success_message = "DRT: " + std::to_string(success_count) + " datarefs from blacklist opened successfully.\n";
	}

    char system_path_c[1000];
    XPLMGetSystemPath(system_path_c);
    boost::filesystem::path system_path(system_path_c);

    {
        std::vector<std::string> dr_file = loadDatarefsFile(system_path / "Resources" / "plugins" / "DataRefs.txt");
        int success_count = datarefs->add(dr_file, dataref_src_t::FILE);
        std::string success_message = "DRT: " + std::to_string(success_count) + " datarefs from DataRefs.txt opened successfully.\n";
        XPLMDebugString(success_message.c_str());
    }

	{
        std::vector<std::string> cr_file = loadDatarefsFile(system_path / "Resources" / "plugins" / "Commands.txt");
		size_t success_count = addPotentialCommandrefs(cr_file);
        std::string success_message = "DRT: " + std::to_string(success_count) + " datarefs from Commands.txt opened successfully.\n";
        XPLMDebugString(success_message.c_str());
    }

	loadAircraftDatarefs();

	//load plugins
	int num_plugins = XPLMCountPlugins();
	XPLMPluginID my_id = XPLMGetMyID();

	std::vector<std::string> all_plugin_datarefs;

	for(int i = 0; i < num_plugins; i++) {
		XPLMPluginID id = XPLMGetNthPlugin(i);
		if(id == my_id) {
			continue;
		}

		char filename[256] = {0};
		char path[512] = {0};
		XPLMGetPluginInfo(id, filename, path, nullptr, nullptr);

		std::vector<std::string> this_plugin_datarefs = getDatarefsFromFile(path);
		all_plugin_datarefs.insert(all_plugin_datarefs.end(), this_plugin_datarefs.begin(), this_plugin_datarefs.end());
	}

	removeVectorUniques(all_plugin_datarefs);

	size_t new_cr_count = addPotentialCommandrefs(all_plugin_datarefs);
	int loaded_ok = datarefs->add(all_plugin_datarefs, dataref_src_t::PLUGIN);
	const std::string message = std::string("DRT: Found ") + std::to_string(all_plugin_datarefs.size()) + std::string(" possible datarefs from plugin files; " + std::to_string(loaded_ok) + "datarefs and " + std::to_string(new_cr_count) + " commands loaded OK.\n");
	XPLMDebugString(message.c_str());
    
    updateWindowsAsDatarefsAdded();

	return 0; 
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
const std::string xplane_plugin_path("laminar.xplane.xplane");
float plugin_changed_check_callback(float, float, int, void *) {
	int num_plugins = XPLMCountPlugins();
	char plugin_path_array[256 + 1];
	for(int plugin_ix = 0; plugin_ix < num_plugins; plugin_ix++) {
		XPLMPluginID plugin = XPLMGetNthPlugin(plugin_ix);
		XPLMGetPluginInfo(plugin, nullptr, plugin_path_array, nullptr, nullptr);

		std::string plugin_path(plugin_path_array);
		if(xplane_plugin_path == plugin_path) {
			continue;
		}
		std::time_t modification_date;
		boost::filesystem::path plugin_path_canonical;
		try {
			plugin_path_canonical = boost::filesystem::canonical(plugin_path);
			modification_date = boost::filesystem::last_write_time(plugin_path_canonical);
		} catch (boost::filesystem::filesystem_error & ec) {
			std::string message = "Error reading modification date. Msg: " + std::string(ec.what()) + " file:" + plugin_path_canonical.string() + "\n";
			XPLMDebugString(message.c_str());
			continue;
		}
		plugin_last_modified_t::iterator plugin_entry_it = plugin_last_modified.find(plugin_path_canonical);
		if(plugin_last_modified.end() == plugin_entry_it) {	// First sighting of this plugin; 
			plugin_last_modified.insert(std::make_pair<boost::filesystem::path, std::time_t>(std::move(plugin_path_canonical), std::move(modification_date)));
		} else {
			if(plugin_entry_it->second != modification_date) {
				plugin_entry_it->second = modification_date;
				if(getAutoReloadPlugins()) {
					std::stringstream message_ss;
					message_ss << "DRT: Observed plugin with new modification (reloading):" << plugin_path_canonical << std::string("\n");
					std::string message = message_ss.str();
					XPLMDebugString(message.c_str());
					XPLMReloadPlugins();
				}
			}
		}
	}
	return 1.f;
}

void reloadAircraft() {
	char acf_path[2048], acf_filename[1024];
	XPLMGetNthAircraftModel(0, acf_filename, acf_path);
	XPLMSetUsersAircraft(acf_path);
}

void plugin_menu_handler(void *, void * inItemRef)
{
	switch ( intptr_t(inItemRef) )
	{
		case 0: showViewerWindow(); break;	
		//case 1: showCommandWindow(); break;	
		case 2:
			XPLMSetFlightLoopCallbackInterval(load_dr_callback, -1, 1, nullptr);
			break;
		case 3: 
			XPLMDebugString("DRT: reloaded aircraft\n");
			reloadAircraft();
			break;
		case 4: 
			XPLMDebugString("DRT: reloaded plugins\n");
			XPLMReloadPlugins(); 
			break;
		case 5: 
			XPLMDebugString("DRT: reloaded scenery\n");
			XPLMReloadScenery(); 
			break;
		case 6: 
			showAboutWindow(); 
			break;
		case 7: {
			bool auto_reload_plugins = getAutoReloadPlugins();
			auto_reload_plugins = !auto_reload_plugins;
			setAutoReloadPlugins(auto_reload_plugins);
			XPLMCheckMenuItem(plugin_menu, 9, auto_reload_plugins ? xplm_Menu_Checked : xplm_Menu_Unchecked);
			break;
		}
		default:
			break;
	}
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
				showViewerWindow();
		}
	}
	return 1;
}

PLUGIN_API int XPluginStart(char * outName, char * outSig, char * outDesc) {
	strcpy(outName, "DataRef Tool");
	strcpy(outSig, "com.leecbaker.datareftool");
	strcpy(outDesc, "View and edit X-Plane Datarefs");
	XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);


    datarefs.emplace();

	char prefs_dir_c[512];
	XPLMGetPrefsPath(prefs_dir_c);
	prefs_path = boost::filesystem::path(prefs_dir_c).parent_path() / "datareftool.json";
    if(loadPrefs(prefs_path)) {
        std::stringstream ss;
        ss << "DRT: prefs loaded from " << prefs_path.string() << "\n";
        XPLMDebugString(ss.str().c_str());
    }
    
	XPLMRegisterFlightLoopCallback(load_dr_callback, -1, nullptr);
	XPLMRegisterFlightLoopCallback(update_dr_callback, -1, nullptr);

	XPLMRegisterFlightLoopCallback(plugin_changed_check_callback, 1., nullptr);
	
	int plugin_submenu = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "DataRefTool", nullptr, 1);
	plugin_menu = XPLMCreateMenu("DataRefTool", XPLMFindPluginsMenu(), plugin_submenu, plugin_menu_handler, nullptr);

	XPLMAppendMenuItem(plugin_menu, "View Datarefs", (void *)0, 1);
	XPLMAppendMenuItem(plugin_menu, "View Commands", (void *)1, 1);
	XPLMAppendMenuSeparator(plugin_menu);
	XPLMAppendMenuItem(plugin_menu, "Rescan for datarefs", (void *)2, 1);
	XPLMAppendMenuSeparator(plugin_menu);
	XPLMAppendMenuItem(plugin_menu, "Reload aircraft", (void *)3, 1);
	XPLMAppendMenuItem(plugin_menu, "Reload plugins", (void *)4, 1);
	XPLMAppendMenuItem(plugin_menu, "Reload scenery", (void *)5, 1);
	XPLMAppendMenuSeparator(plugin_menu);
	XPLMAppendMenuItem(plugin_menu, "Reload plugins on modification", (void *)7, 1);
	XPLMAppendMenuSeparator(plugin_menu);
	XPLMAppendMenuItem(plugin_menu, "About DataRefTool", (void *)6, 1);

	XPLMEnableMenuItem(plugin_menu, 0, 1);
	XPLMEnableMenuItem(plugin_menu, 1, 0);
	XPLMEnableMenuItem(plugin_menu, 2, 1);	//sep
	XPLMEnableMenuItem(plugin_menu, 3, 1);
	XPLMEnableMenuItem(plugin_menu, 4, 1);	//sep
	XPLMEnableMenuItem(plugin_menu, 5, 1);
	XPLMEnableMenuItem(plugin_menu, 6, 1);
	XPLMEnableMenuItem(plugin_menu, 7, 1);
	XPLMEnableMenuItem(plugin_menu, 8, 1);	//sep
	XPLMEnableMenuItem(plugin_menu, 9, 1);
	XPLMEnableMenuItem(plugin_menu, 10, 1);	//sep
	XPLMEnableMenuItem(plugin_menu, 11, 1);

	//commands
	reload_aircraft_command = XPLMCreateCommand("datareftool/reload_aircraft", "Reload the current aircraft");
	reload_plugins_command = XPLMCreateCommand("datareftool/reload_plugins", "Reload all plugins");
	reload_scenery_command = XPLMCreateCommand("datareftool/reload_scenery", "Reload the scenery");
	show_datarefs_command = XPLMCreateCommand("datareftool/show_datarefs", "Show the dataref search window");

	XPLMRegisterCommandHandler(reload_aircraft_command, command_handler, 0, nullptr);
	XPLMRegisterCommandHandler(reload_plugins_command, command_handler, 0, nullptr);
	XPLMRegisterCommandHandler(reload_scenery_command, command_handler, 0, nullptr);
	XPLMRegisterCommandHandler(show_datarefs_command, command_handler, 0, nullptr);
    
	{	//load blacklist first, before everything else
		char system_path_c[1000];
		XPLMGetSystemPath(system_path_c);
		boost::filesystem::path system_path(system_path_c);
		
		blacklisted_datarefs = loadBlacklistFile(system_path / "Resources" / "plugins" / "drt_blacklist.txt");
	}

    updateWindowsAsDatarefsAdded();
    
	return 1;
}

PLUGIN_API void	XPluginStop(void) {
    if(savePrefs(prefs_path)) {
        std::stringstream ss;
        ss << "DRT: prefs saved to " << prefs_path.string() << "\n";
        XPLMDebugString(ss.str().c_str());
    }
	//closeCommandWindows();
	closeAboutWindow();
	closeViewerWindows();
    datarefs = boost::none;
	XPLMUnregisterFlightLoopCallback(load_dr_callback, nullptr);
	XPLMUnregisterFlightLoopCallback(load_acf_dr_callback, nullptr);
	XPLMUnregisterFlightLoopCallback(update_dr_callback, nullptr);
}

PLUGIN_API void XPluginDisable(void) {
}

PLUGIN_API int XPluginEnable(void) {
	{
		bool auto_reload_plugins = getAutoReloadPlugins();
		XPLMCheckMenuItem(plugin_menu, 9, auto_reload_plugins ? xplm_Menu_Checked : xplm_Menu_Unchecked);
	}
	return 1;
}

const intptr_t MSG_ADD_DATAREF = 0x01000000;
const intptr_t MSG_ADD_COMMANDREF = 0x01000099;

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID, intptr_t inMessage, void * inParam) {
	switch(inMessage) {
		// Add custom datarefs in the style of DRE:
		// http://www.xsquawkbox.net/xpsdk/mediawiki/Register_Custom_DataRef_in_DRE
		case MSG_ADD_DATAREF: {
			char * dataref_name = (char *) inParam;
			bool added_ok = datarefs->add(dataref_name, dataref_src_t::USER_MSG);
            if(added_ok) {
                updateWindowsAsDatarefsAdded();
			} else {
				const std::string message = std::string("DRT: Couldn't load dataref from message: ") + dataref_name + std::string("\n");
				XPLMDebugString(message.c_str());
			}
			break;
		}
		case MSG_ADD_COMMANDREF: {
			char * commandref_name = (char *) inParam;
			bool added_ok = true;//addUserCommandref(commandref_name);
			if(added_ok) {
				//updateCommandWindows();
			} else {
				const std::string message = std::string("DRT: Couldn't load commandref from message: ") + commandref_name + std::string("\n");
				XPLMDebugString(message.c_str());
			}
			break;
		}

		case XPLM_MSG_PLANE_LOADED: {
			int64_t plane_num = int64_t(inParam);
			const std::string message = std::string("DRT: Plane loaded #: ") + std::to_string(plane_num) + std::string("\n");
			XPLMDebugString(message.c_str());
			if(0 == plane_num) {	//user's plane
				XPLMRegisterFlightLoopCallback(load_acf_dr_callback, -1, nullptr);
			}
			break;
		}

		case XPLM_MSG_WILL_WRITE_PREFS: {
			break;
		}
	}
}
