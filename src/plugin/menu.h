#pragma once

#include "XPLMMenus.h"

class PluginMenu {
    XPLMMenuID plugin_menu = nullptr;
    static void plugin_menu_handler(void * refcon, void * inItemRef);

protected:
    void handleMenu(intptr_t item_ref);

    int impersonate_dre_menu_item = -1;
    int reload_on_plugin_change_item = -1;

public:
    PluginMenu();

    void update();

};
