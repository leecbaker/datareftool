#include "menus.h"

void Menu::PluginMenuHandler(void * inMenuRef, void * inItemRef) {
    intptr_t item_id = reinterpret_cast<intptr_t>(inItemRef);
    Menu * menu = static_cast<Menu *>(inMenuRef);
    
    std::function<void()> & f = menu->menu_callbacks[item_id];
    
    f();
}