#include "menu.h"

#include "about_window.h"

#include "plugin.h"
#include "prefs.h"

#include "logging.h"

#include "XPLMPlugin.h"
#include "XPLMUtilities.h"

PluginMenu::PluginMenu() {
    int plugin_submenu = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "DataRefTool", static_cast<void *>(this), 1);
    plugin_menu = XPLMCreateMenu("DataRefTool", XPLMFindPluginsMenu(), plugin_submenu, &PluginMenu::plugin_menu_handler, static_cast<void *>(this));

    XPLMAppendMenuItem(plugin_menu, "View Datarefs", reinterpret_cast<void *>(0), 1);
    XPLMAppendMenuItem(plugin_menu, "View Commands", reinterpret_cast<void *>(1), 1);
    XPLMAppendMenuSeparator(plugin_menu);
    XPLMAppendMenuItem(plugin_menu, "Rescan datarefs and commands", reinterpret_cast<void *>(2), 1);
    XPLMAppendMenuSeparator(plugin_menu);
    XPLMAppendMenuItem(plugin_menu, "Reload aircraft", reinterpret_cast<void *>(3), 1);
    XPLMAppendMenuItem(plugin_menu, "Reload plugins", reinterpret_cast<void *>(4), 1);
    XPLMAppendMenuItem(plugin_menu, "Reload scenery", reinterpret_cast<void *>(5), 1);
    XPLMAppendMenuSeparator(plugin_menu);
    reload_on_plugin_change_item = XPLMAppendMenuItem(plugin_menu, "Reload plugins on modification", reinterpret_cast<void *>(7), 1);
    XPLMAppendMenuSeparator(plugin_menu);
    impersonate_dre_menu_item = XPLMAppendMenuItem(plugin_menu, "Impersonate DRE (requires reload)", reinterpret_cast<void *>(8), 1);
    XPLMAppendMenuSeparator(plugin_menu);
    XPLMAppendMenuItem(plugin_menu, "About DataRefTool", reinterpret_cast<void *>(6), 1);

    XPLMEnableMenuItem(plugin_menu, 0, 1);
    XPLMEnableMenuItem(plugin_menu, 1, 1);
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
    XPLMEnableMenuItem(plugin_menu, 12, 1);	//sep
    XPLMEnableMenuItem(plugin_menu, 13, 1);
}

void PluginMenu::plugin_menu_handler(void * refcon, void * inItemRef) {
    PluginMenu * pmenu = static_cast<PluginMenu *>(refcon);
    intptr_t item_ref = reinterpret_cast<intptr_t>(inItemRef);

    pmenu->handleMenu(item_ref);
}

void PluginMenu::handleMenu(intptr_t item_ref) {
    switch (item_ref)
    {
        case 0: plugin_data->showViewerWindow(true, false); break;	
        case 1: plugin_data->showViewerWindow(false, true); break;	
        case 2:
            plugin_data->rescanDatarefs();
            break;
        case 3: 
            xplog << "Reloaded aircraft\n";
            reloadAircraft();
            break;
        case 4: 
            xplog << "Reloading plugins\n";
            XPLMReloadPlugins(); 
            break;
        case 5: 
            xplog << "Reloading scenery\n";
            XPLMReloadScenery(); 
            break;
        case 6: 
            showAboutWindow(); 
            break;
        case 7:
            setAutoReloadPlugins(!getAutoReloadPlugins());
            update();
            break;
        case 8:
            setImpersonateDRE(!getImpersonateDRE());
            update();
            break;
        default:
            break;
    }
}

void PluginMenu::update() {
    XPLMCheckMenuItem(plugin_menu, impersonate_dre_menu_item, getImpersonateDRE() ? xplm_Menu_Checked : xplm_Menu_Unchecked);
    XPLMCheckMenuItem(plugin_menu, reload_on_plugin_change_item, getAutoReloadPlugins() ? xplm_Menu_Checked : xplm_Menu_Unchecked);
}