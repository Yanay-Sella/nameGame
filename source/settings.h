// Player settings saved next to EBOOT.BIN as "key=value" lines (docs/features/screens).
#pragma once

struct Settings {
    bool music = true;   // the song plays during runs
};

Settings settingsLoad(const char* path);                         // defaults when the file is missing or unreadable
bool settingsSave(const char* path, const Settings& settings);   // false when the file couldn't be written
