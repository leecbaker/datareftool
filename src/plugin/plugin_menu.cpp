#include "plugin_menu.h"

#include "drt_plugin.h"

using namespace std::string_literals;

PluginMenu::PluginMenu() {
    XPLMMenuID xp_plugin_menu = XPLMFindPluginsMenu();
    datareftool_menu.emplace("DataRefTool"s, xp_plugin_menu);

    search_item = datareftool_menu->appendItem("Search...", [](){plugin->openSearchWindow();});
    datareftool_menu->appendSeparator();
    datareftool_menu->appendItem("Rescan datarefs and commands", [](){plugin->rescan();});
    datareftool_menu->appendSeparator();
    datareftool_menu->appendItem("Reload aircraft", [](){plugin->reloadAircraft();});
    datareftool_menu->appendItem("Reload plugins", [](){plugin->reloadPlugins();});
    datareftool_menu->appendItem("Reload scenery", [](){plugin->reloadScenery();});
    datareftool_menu->appendSeparator();
    reload_plugins_on_modification_item = datareftool_menu->appendItem("Reload plugins on modification", [](){plugin->openSearchWindow();});
    datareftool_menu->appendSeparator();
    impersonate_dre_item = datareftool_menu->appendItem("Impersonate DRE (requires reload)", [](){plugin->openSearchWindow();});
    datareftool_menu->appendSeparator();
    datareftool_menu->appendItem("About DataRefTool...", [](){plugin->openAboutWindow();});
}

void PluginMenu::update() {
    
}