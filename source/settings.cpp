// Loads and saves settings.txt; stdio only, so the Mac tests build it too.
#include "settings.h"
#include <stdio.h>

Settings settingsLoad(const char* path) {
    Settings settings;
    FILE* file = fopen(path, "r");
    if (!file) return settings;
    char line[64];
    while (fgets(line, sizeof line, file)) {
        int value;
        if (sscanf(line, "music=%d", &value) == 1) settings.music = value != 0;
    }
    fclose(file);
    return settings;
}

bool settingsSave(const char* path, const Settings& settings) {
    FILE* file = fopen(path, "w");
    if (!file) return false;
    bool written = fprintf(file, "music=%d\n", settings.music ? 1 : 0) > 0;
    return fclose(file) == 0 && written;
}
