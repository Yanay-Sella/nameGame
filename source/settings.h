// Player settings saved next to EBOOT.BIN as "key=value" lines (docs/features/screens).
#pragma once

enum Music { MUSIC_OFF, MUSIC_MIZRAHIT, MUSIC_DUBSTEP, MUSIC_CHOICES };   // saved as music=0, 1, 2 (old files: 0 off, 1 on)

struct Settings {
    Music music = MUSIC_MIZRAHIT;   // the song played during runs
    int shake = 100;     // screen shake strength in percent: 0, 25, 50, 75 or 100
    bool reduceFlashing = false;   // no full-screen flashes
};

Settings settingsLoad(const char* path);                         // defaults when the file is missing or unreadable
bool settingsSave(const char* path, const Settings& settings);   // false when the file couldn't be written

int highScoreLoad(const char* path);                // 0 when the file is missing or unreadable; clamped to 0..MAX_SCORE
bool highScoreSave(const char* path, int score);    // false when the file couldn't be written
