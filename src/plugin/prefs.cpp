#include "prefs.h"

#include "logging.h"

#include <fstream>

bool auto_reload_plugins;
bool impersonate_dre = false;
bool debug_enabled = true;

bool getAutoReloadPlugins() { return auto_reload_plugins; }
void setAutoReloadPlugins(bool reload_automatically) {
    auto_reload_plugins = reload_automatically;
}

void setDebugMode(bool enabled) { debug_enabled = enabled; }
bool getDebugMode() { return debug_enabled; }

bool getImpersonateDRE() { return impersonate_dre; }
void setImpersonateDRE(bool impersonate) { impersonate_dre = impersonate; }

const char * auto_reload_plugin_key = "auto_reload_plugins";
const char * impersonate_dre_key = "impersonate_dre";
const char * debug_mode_key = "debug_mode";


bool loadPrefs(const lb::filesystem::path & path, std::function<void(const nlohmann::json &)> create_window_func) {
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
            create_window_func(window);
        }
    } catch(nlohmann::json::exception) {
        return false;
    }

    try {
        auto_reload_plugins = prefs.value<bool>(auto_reload_plugin_key, true);
        impersonate_dre = prefs.value<bool>(impersonate_dre_key, true);
        debug_enabled = prefs.value<bool>(debug_mode_key, true);
    } catch(nlohmann::json::exception) {
        return false;
    }

    return true;
}

bool savePrefs(const lb::filesystem::path & path, const nlohmann::json & windows) {

    nlohmann::json prefs = {
        {"author", "Lee C. Baker"},
        {"compile_date", __DATE__ " " __TIME__},
        {"windows", windows},
        {auto_reload_plugin_key, auto_reload_plugins},
        {impersonate_dre_key, impersonate_dre},
        {debug_mode_key, debug_enabled}
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
