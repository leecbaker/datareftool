#pragma once

#include <optional>

#include "menus.h"

class PluginMenu {
    std::optional<Menu> datareftool_menu;
    std::optional<MenuItem> search_item;
    std::optional<MenuItem> about_item;

    std::optional<MenuItem> reload_plugins_on_modification_item;
    std::optional<MenuItem> impersonate_dre_item;
public:
    PluginMenu();

    /// Update checkmarks and such
    void update();
};