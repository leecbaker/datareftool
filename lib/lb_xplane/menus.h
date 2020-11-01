#pragma once

#include <cassert>
#include <functional>
#include <string>
#include <vector>

#include "XPLMMenus.h"

class MenuItem {
    XPLMMenuID parent_menu_id = nullptr;
    int menu_item_index = -1;
public:
    MenuItem(XPLMMenuID parent_menu_id, int menu_item_index) : parent_menu_id(parent_menu_id), menu_item_index(menu_item_index) {
        assert(nullptr != parent_menu_id);
        assert(-1 != menu_item_index);
    }
    
    void setChecked(bool checked) {
        XPLMCheckMenuItem(parent_menu_id, menu_item_index, checked ? xplm_Menu_Checked : xplm_Menu_Unchecked);
    }
    
    void setEnabled(bool enabled) {
        XPLMEnableMenuItem(parent_menu_id, menu_item_index, enabled ? 1 : 0);
    }
    
    void setName(const std::string & name) {
        XPLMSetMenuItemName(parent_menu_id, menu_item_index, name.c_str(), 1);  //force english
    }
};

class Menu {
    XPLMMenuID menu_id = nullptr;
    std::vector<std::function<void()>> menu_callbacks;
protected:
    static void PluginMenuHandler(void * inMenuRef, void * inItemRef);
public:
    
    Menu(const std::string & name, const XPLMMenuID parent_menu_id) {
        int parent_menu_item_number = XPLMAppendMenuItem(parent_menu_id, name.c_str(), nullptr, 1);
        menu_id = XPLMCreateMenu(name.c_str(), parent_menu_id, parent_menu_item_number, Menu::PluginMenuHandler, this);
    }
    Menu(const std::string & name, const Menu & parent_menu) : Menu(name, parent_menu.menu_id) {}
    
    
    MenuItem appendItem(const std::string & name, std::function<void()> f) {
        intptr_t cb_id = menu_callbacks.size();
        menu_callbacks.emplace_back(f);
        int item_index = XPLMAppendMenuItem(menu_id, name.c_str(), reinterpret_cast<void *>(cb_id), 1);
        return MenuItem(menu_id, item_index);
    }
    
    void appendSeparator() {
        XPLMAppendMenuSeparator(menu_id);
    }
    
    void enableItem(int index) {
        XPLMEnableMenuItem(menu_id, index, 1);
    }
    void disableItem(int index) {
        XPLMEnableMenuItem(menu_id, index, 0);
    }
    
    ~Menu() {
        XPLMDestroyMenu(menu_id);
    }
};
