// NAME QUEST save slots: <folder>/quest1..3.txt, key=value lines (spec §17). stdio only, no logging.
#include "quest.h"
#include <stdio.h>
#include <string.h>

static std::string slotPath(const char* folder, int slot, const char* extension) {
    char name[16];
    snprintf(name, sizeof name, "/quest%d.%s", slot + 1, extension);
    return std::string(folder) + name;
}

// Writes every field; false when the file couldn't be written.
static bool writeFile(const char* path, const SaveData& s) {
    FILE* file = fopen(path, "w");
    if (!file) return false;
    fprintf(file, "version=1\nclass=%d\nlevel=%d\njob=%d\nnames=%d\nplaytime=%.1f\nhp=%d\nmp=%d\n", s.heroClass, s.level,
            s.job, s.names, s.playtime, s.hp, s.mp);
    fprintf(file, "checkpoint=%d,%d\nlit=%x\n", s.checkpointWorld, s.checkpointId, s.lit);
    if (s.echoNames > 0) fprintf(file, "echo=%d,%d,%d,%.1f,%.1f\n", s.echoNames, s.echoWorld, s.echoArea, s.echoX, s.echoY);
    for (int w = 0; w < SAVED_WORLDS; w++)
        fprintf(file, "visited%d=%x\nchests%d=%x\nshortcuts%d=%x\n", w, s.visited[w], w, s.chests[w], w, s.shortcuts[w]);
    fprintf(file, "gates=%x\nbosses=%x\nhighest=%d\nngplus=%d\ndeaths=%d\nkills=%d\n", s.gates, s.bosses, s.highest, s.ngPlus,
            s.deaths, s.kills);
    fprintf(file, "items=%d,%d,%d,%d,%d,%d\n", s.items[0], s.items[1], s.items[2], s.items[3], s.items[4], s.items[5]);
    fprintf(file, "weapon=%d\narmor=%d\nrefine=%d\nshards=%d\nkeys=%x\n", s.weapon, s.armor, s.refine, s.shards, s.keys);
    for (int i = 0; i < QUEST_COUNT; i++) fprintf(file, "quest%d=%d\n", i, s.quests[i]);
    fprintf(file, "ilsa=%d\n", s.ilsaTalks);
    return fclose(file) == 0;
}

bool saveWrite(const char* folder, int slot, const SaveData& save, bool* renameFailed) {
    std::string tmp = slotPath(folder, slot, "tmp"), path = slotPath(folder, slot, "txt");
    if (renameFailed) *renameFailed = false;
    if (!writeFile(tmp.c_str(), save)) return false;
    remove(path.c_str());
    if (rename(tmp.c_str(), path.c_str()) == 0) return true;
    if (renameFailed) *renameFailed = true;
    remove(tmp.c_str());
    return writeFile(path.c_str(), save);
}

bool saveRead(const char* folder, int slot, SaveData& save) {
    save = SaveData();
    FILE* file = fopen(slotPath(folder, slot, "txt").c_str(), "r");
    if (!file) return false;
    char line[128];
    while (fgets(line, sizeof line, file)) {
        char* equals = strchr(line, '=');
        if (!equals) continue;
        *equals = 0;
        const char* key = line;
        const char* value = equals + 1;
        int w;
        if (!strcmp(key, "class")) sscanf(value, "%d", &save.heroClass);
        else if (!strcmp(key, "level")) sscanf(value, "%d", &save.level);
        else if (!strcmp(key, "job")) sscanf(value, "%d", &save.job);
        else if (!strcmp(key, "names")) sscanf(value, "%d", &save.names);
        else if (!strcmp(key, "playtime")) sscanf(value, "%f", &save.playtime);
        else if (!strcmp(key, "hp")) sscanf(value, "%d", &save.hp);
        else if (!strcmp(key, "mp")) sscanf(value, "%d", &save.mp);
        else if (!strcmp(key, "checkpoint")) sscanf(value, "%d,%d", &save.checkpointWorld, &save.checkpointId);
        else if (!strcmp(key, "lit")) sscanf(value, "%x", &save.lit);
        else if (!strcmp(key, "echo"))
            sscanf(value, "%d,%d,%d,%f,%f", &save.echoNames, &save.echoWorld, &save.echoArea, &save.echoX, &save.echoY);
        else if (sscanf(key, "visited%d", &w) == 1 && w >= 0 && w < SAVED_WORLDS) sscanf(value, "%x", &save.visited[w]);
        else if (sscanf(key, "chests%d", &w) == 1 && w >= 0 && w < SAVED_WORLDS) sscanf(value, "%x", &save.chests[w]);
        else if (sscanf(key, "shortcuts%d", &w) == 1 && w >= 0 && w < SAVED_WORLDS) sscanf(value, "%x", &save.shortcuts[w]);
        else if (!strcmp(key, "gates")) sscanf(value, "%x", &save.gates);
        else if (!strcmp(key, "bosses")) sscanf(value, "%x", &save.bosses);
        else if (!strcmp(key, "highest")) sscanf(value, "%d", &save.highest);
        else if (!strcmp(key, "ngplus")) sscanf(value, "%d", &save.ngPlus);
        else if (!strcmp(key, "deaths")) sscanf(value, "%d", &save.deaths);
        else if (!strcmp(key, "kills")) sscanf(value, "%d", &save.kills);
        else if (!strcmp(key, "items"))
            sscanf(value, "%d,%d,%d,%d,%d,%d", &save.items[0], &save.items[1], &save.items[2], &save.items[3], &save.items[4],
                   &save.items[5]);
        else if (!strcmp(key, "weapon")) sscanf(value, "%d", &save.weapon);
        else if (!strcmp(key, "armor")) sscanf(value, "%d", &save.armor);
        else if (!strcmp(key, "refine")) sscanf(value, "%d", &save.refine);
        else if (!strcmp(key, "shards")) sscanf(value, "%d", &save.shards);
        else if (!strcmp(key, "keys")) sscanf(value, "%x", &save.keys);
        else if (sscanf(key, "quest%d", &w) == 1 && w >= 0 && w < QUEST_COUNT) sscanf(value, "%d", &save.quests[w]);
        else if (!strcmp(key, "ilsa")) sscanf(value, "%d", &save.ilsaTalks);
    }
    fclose(file);
    save.used = true;
    return true;
}

bool saveDelete(const char* folder, int slot) {
    return remove(slotPath(folder, slot, "txt").c_str()) == 0;
}
