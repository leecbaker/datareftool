#pragma once

#include "command_provider.h"
#include "dataref_provider.h"

class DRTDatarefs {
    DatarefProvider<bool> debug_mode;

    DatarefProvider<bool> impersonate_dre;
    DatarefProvider<bool> auto_reload_plugins;

    DatarefProvider<std::string> build_date;

    CommandProvider toggle_debug;
    CommandProvider toggle_impersonate_dre;
    CommandProvider toggle_auto_reload_plugins;

    CommandProvider prefs_load;
    CommandProvider prefs_save;

    CommandProvider reload_plugins;
    CommandProvider reload_scenery;
    CommandProvider reload_aircraft;
    CommandProvider new_window;
public:
    DRTDatarefs();
};
