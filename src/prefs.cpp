#include <iostream>

#include "prefs.h"

#include "XPLMUtilities.h"

#include "viewer_window.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <boost/foreach.hpp>

bool auto_reload_plugins;
bool impersonate_dre = false;

bool getAutoReloadPlugins() { return auto_reload_plugins; }
void setAutoReloadPlugins(bool reload_automatically) {
	auto_reload_plugins = reload_automatically;
}


bool getImpersonateDRE() { return impersonate_dre; }
void setImpersonateDRE(bool impersonate) { impersonate_dre = impersonate; }

const char * auto_reload_plugin_key = "auto_reload_plugins";
const char * impersonate_dre_key = "impersonate_dre";

bool loadPrefs(const boost::filesystem::path & path) {
	//open file, and deserialize
	std::ifstream f(path.string());

	if(f.fail()) {
		std::cerr << "Couldn't open properties file: " << path.string() << std::endl;
		return false;
	}

	boost::property_tree::ptree prefs;
	read_json(f, prefs);

	BOOST_FOREACH(boost::property_tree::ptree::value_type & window_detail_val, prefs.get_child("windows")) {
        showViewerWindow(window_detail_val.second);
    }

	auto_reload_plugins = prefs.get<bool>(auto_reload_plugin_key, true);
	impersonate_dre = prefs.get<bool>(impersonate_dre_key, false);

	return true;
}

bool savePrefs(const boost::filesystem::path & path) {

	boost::property_tree::ptree prefs;
	boost::property_tree::ptree windows = getViewerWindowsDetails();
    
    prefs.put("author", "Lee C. Baker");
    prefs.put("compile_date", __DATE__ " " __TIME__);
    prefs.add_child("windows", windows);
	prefs.put(auto_reload_plugin_key, auto_reload_plugins);
	prefs.put(impersonate_dre_key, impersonate_dre);

	//serialize and save to file
	std::ofstream f(path.string());
    if(f.fail()) {
        return false;
    }
    try {
		write_json(f, prefs);
    } catch(boost::property_tree::json_parser_error & e) {
    	const std::string message = std::string("DRT: Error writing preferences file at ") + path.string() + std::string("\n");
		XPLMDebugString(message.c_str());
		XPLMDebugString(("DRT: " + e.filename() + ":" + std::to_string(e.line())).c_str());
		XPLMDebugString(("DRT: " + e.message()).c_str());
    	return false;
    }

	return false == f.fail();
}