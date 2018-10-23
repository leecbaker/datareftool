#include "prefs.h"

#include "logging.h"
#include "viewer_window.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <boost/foreach.hpp>

bool auto_reload_plugins;
bool impersonate_dre = false;
bool logging_enabled = true;

bool getAutoReloadPlugins() { return auto_reload_plugins; }
void setAutoReloadPlugins(bool reload_automatically) {
	auto_reload_plugins = reload_automatically;
}

bool getLoggingEnabled() { return logging_enabled; }

bool getImpersonateDRE() { return impersonate_dre; }
void setImpersonateDRE(bool impersonate) { impersonate_dre = impersonate; }

const char * auto_reload_plugin_key = "auto_reload_plugins";
const char * impersonate_dre_key = "impersonate_dre";
const char * logging_enabled_key = "logging_enabled";

bool loadPrefs(const boost::filesystem::path & path) {
	//open file, and deserialize
	std::ifstream f(path.string());

	if(f.fail()) {
		xplog << "Couldn't open properties file: " << path << "\n";
		return false;
	}

	boost::property_tree::ptree prefs;
	try {
		read_json(f, prefs);
	} catch (const boost::property_tree::json_parser_error & e) {
		xplog << "Error parsing preferences file at " << path << "\n";
		xplog << "Error: " << e.what() << "\n";
		xplog << "Preferences going back to default.\n";
		return false;
	}

	BOOST_FOREACH(boost::property_tree::ptree::value_type & window_detail_val, prefs.get_child("windows")) {
        showViewerWindow(window_detail_val.second);
    }

	auto_reload_plugins = prefs.get<bool>(auto_reload_plugin_key, true);
	impersonate_dre = prefs.get<bool>(impersonate_dre_key, false);
	bool logging_enabled_loaded = prefs.get<bool>(logging_enabled_key, true);

	if(false == logging_enabled_loaded) {
		xplog << "Logging disabled via prefs file\n";
	}
	xplog << "Loaded prefs from " << path << "\n";
	logging_enabled = logging_enabled_loaded;

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
	prefs.put(logging_enabled_key, logging_enabled);

	//serialize and save to file
	std::ofstream f(path.string());
    if(f.fail()) {
        return false;
    }
    try {
		write_json(f, prefs);
    } catch(boost::property_tree::json_parser_error & e) {
		xplog << "Error writing preferences file at " << path << "\n";
		xplog << e.filename() << ":" << e.line() << "\n";
		xplog << e.message() << "\n";
    	return false;
    }

	return false == f.fail();
}