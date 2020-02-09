#include "prefs.h"

#include "logging.h"
#include "viewer_window.h"

#include "plugin.h"

#include <fstream>

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

    nlohmann::json prefs;

    try {
        f >> prefs;
    } catch (const nlohmann::json::parse_error & pe) {
        xplog << "Error parsing preferences file at " << path << "\n";
        xplog << "Error: " << pe.what() << "\n";
        xplog << "Preferences going back to default.\n";
        return false;
    }

    try {
        for(const nlohmann::json & window: prefs["windows"]) {
            plugin_data->showViewerWindow(window);
        }
    } catch(nlohmann::json::exception) {

    }

    bool logging_enabled_loaded = true;
    try {
        auto_reload_plugins = prefs.value<bool>(auto_reload_plugin_key, true);
        impersonate_dre = prefs.value<bool>(impersonate_dre_key, true);
        logging_enabled_loaded = prefs.value<bool>(logging_enabled_key, true);
    } catch(nlohmann::json::exception) {

    }


    if(false == logging_enabled_loaded) {
        xplog << "Logging disabled via prefs file\n";
    }
    xplog << "Loaded prefs from " << path << "\n";
    logging_enabled = logging_enabled_loaded;

    return true;
}

bool savePrefs(const boost::filesystem::path & path, const nlohmann::json & windows) {

    nlohmann::json prefs = {
        {"author", "Lee C. Baker"},
        {"compile_date", __DATE__ " " __TIME__},
        {"windows", windows},
        {auto_reload_plugin_key, auto_reload_plugins},
        {impersonate_dre_key, impersonate_dre},
        {logging_enabled_key, logging_enabled}
    };

    //serialize and save to file
    std::ofstream f(path.string());
    if(f.fail()) {
        return false;
    }
    try {
        f << prefs;
    } catch(nlohmann::json::exception & e) {
        xplog << "Error writing preferences file at " << path << "\n";
        xplog << e.what() << "\n";
        return false;
    }

    return false == f.fail();
}
