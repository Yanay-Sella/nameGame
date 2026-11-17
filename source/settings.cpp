// Loads and saves settings.txt; stdio only, so the Mac tests build it too.
#include "settings.h"
#include "game.h"
#include <stdio.h>

Settings settingsLoad(const char* path) {
    Settings settings;
    FILE* file = fopen(path, "r");
    if (!file) return settings;
    char line[64];
    while (fgets(line, sizeof line, file)) {
        int value;
        if (sscanf(line, "music=%d", &value) == 1) settings.music = value != 0;
        if (sscanf(line, "shake=%d", &value) == 1) settings.shake = value < 0 ? 0 : value > 100 ? 100 : value;
        if (sscanf(line, "flash=%d", &value) == 1) settings.reduceFlashing = value != 0;
    }
    fclose(file);
    return settings;
}

bool settingsSave(const char* path, const Settings& settings) {
    FILE* file = fopen(path, "w");
    if (!file) return false;
    bool written = fprintf(file, "music=%d\nshake=%d\nflash=%d\n", settings.music ? 1 : 0, settings.shake, settings.reduceFlashing ? 1 : 0) > 0;
    return fclose(file) == 0 && written;
}

int highScoreLoad(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) return 0;
    char line[64];
    long best = 0;
    while (fgets(line, sizeof line, file))
        if (sscanf(line, "best=%ld", &best) == 1) break;
    fclose(file);
    return best < 0 ? 0 : best > MAX_SCORE ? MAX_SCORE : (int)best;
}

bool highScoreSave(const char* path, int score) {
    FILE* file = fopen(path, "w");
    if (!file) return false;
    bool written = fprintf(file, "best=%d\n", score) > 0;
    return fclose(file) == 0 && written;
}
