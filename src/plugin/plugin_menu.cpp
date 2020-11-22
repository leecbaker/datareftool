#include "plugin_menu.h"

#include "drt_plugin.h"
#include "prefs.h"

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
    reload_plugins_on_modification_item = datareftool_menu->appendItem("Reload plugins on modification", [this](){
        plugin->toggleAutoReloadPlugins();
        update();
        });
    datareftool_menu->appendSeparator();
    impersonate_dre_item = datareftool_menu->appendItem("Impersonate DRE (requires reload)", [this](){
        plugin->toggleImpersonateDRE();
        update();
        });
    datareftool_menu->appendSeparator();
    datareftool_menu->appendItem("About DataRefTool...", [](){plugin->openAboutWindow();});

    update();
}

void PluginMenu::update() {
    reload_plugins_on_modification_item->setChecked(getAutoReloadPlugins());
    impersonate_dre_item->setChecked(getImpersonateDRE());
}