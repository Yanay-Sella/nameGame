// NAME QUEST: the RPG platformer mode (docs/features/name-quest). No SDL or PS3 headers, so `make test` builds it on the Mac.
#pragma once
#include "game.h"
#include <stdint.h>
#include <string>
#include <vector>

// Design pixels (dp, the 320x180 design's units) → real pixels at 640x360 (spec "Resolution", D-053).
const float DP = 2;
const int TILE = (int)(8 * DP), WIDE_COLS = 80, WIDE_ROWS = 23, TALL_COLS = 40, TALL_ROWS = 45;
const int WORLD_NAMULA = 0, WORLD_SECRET = 7, WORLD_TEST = 8, WORLD_COUNT = 9, SAVED_WORLDS = 8;   // 1-6 the worlds
const int SLOT_COUNT = 3, MAX_LEVEL = 40;
enum HeroClass { CLASS_KNIGHT, CLASS_ARCHER, CLASS_MAGICIAN, CLASS_THIEF, CLASS_COUNT };
enum AreaFlag { AREA_MINIBOSS = 1, AREA_BOSS = 2, AREA_DARK = 4, AREA_RISING_LAVA = 8 };
enum ItemType { ITEM_RED, ITEM_BIG, ITEM_MANA, ITEM_WHETSTONE, ITEM_TONIC, ITEM_FEATHER, ITEM_COUNT };
enum QuestId { QUEST_JOB, QUEST_TOBIN, QUEST_PELL, QUEST_STRANGER, QUEST_SMITH, QUEST_SELA, QUEST_SKY, QUEST_COUNT };
enum KeyItem { KEY_STAR_IRON, KEY_FROZEN_FEATHER, KEY_PICKAXE, KEY_HEART_OF_MAGMA, KEY_LETTER, KEY_CALDER_RING,
               KEY_TOBIN_CHARM, KEY_STRANGER_RING, KEY_LOCKET, KEY_GARRICK_GIFT, KEY_PAGE_1, KEY_PAGE_2, KEY_PAGE_3,
               KEY_PAGE_4, KEY_PAGE_5, KEY_TRIAL_DONE, KEY_SECRET_DONE, KEY_CREDITS_SEEN, KEY_COUNT };   // bits of SaveData::keys

struct AreaLink { int area, tileX, tileY; };   // where the hero arrives (bottom-centre of that tile); area -1: closed
struct AreaDef {
    const char* name;        // banner on the first visit, at most 20 chars, e.g. "HULL BREACH"
    int cols, rows;          // 80 x 23 (wide) or 40 x 45 (tall)
    const char* tiles;       // cols * rows chars, rows concatenated top to bottom
    AreaLink left, right, up, down;   // edge exits
    AreaLink portals[4];     // by 'P' order
    int chests[4];           // ChestId by 'T' order, -1 unused
    int npcs[2];             // NpcId by 'N' order, -1 unused
    int mapX, mapY;          // top-left cell on the world map: wide areas take 2x1 cells, tall 1x2
    int band;                // 0 calm, 1 medium, 2 intense (§13 ramp)
    int flags;               // AREA_MINIBOSS, AREA_BOSS, AREA_DARK (world 5), AREA_RISING_LAVA (world 4)
    int shortcut;            // the world's shortcut index for this area's lever, -1 none
    int landmarks[4];        // the world's landmark ids by 'L' order, 1-based; 0 unused (§22; last, so older initialisers stay valid)
};

// How a world looks and sounds; tile drawing and the map read it (spec §13).
struct WorldStyle { char top, fill, platform, ladder, liquidA, liquidB, mapColor; Sound step; };
struct WorldDef { const char* name; const AreaDef* areas; int count; WorldStyle style; };
const WorldDef& worldDef(int world);   // 0..WORLD_COUNT-1; worlds with no areas yet have count 0

// One save slot (spec §17); the defaults are a new hero's.
struct SaveData {
    bool used = false;
    int heroClass = 0, level = 1, job = 0, names = 0;
    float playtime = 0;
    int hp = 0, mp = 0;
    int checkpointWorld = 0, checkpointId = 0;
    uint32_t lit = 1;                      // bit world*2 + id; bit 0: the hearth
    int echoNames = 0, echoWorld = 0, echoArea = 0;
    float echoX = 0, echoY = 0;
    uint32_t visited[SAVED_WORLDS] = {}, chests[SAVED_WORLDS] = {}, shortcuts[SAVED_WORLDS] = {};
    uint32_t gates = 0, bosses = 0;
    int highest = 1, ngPlus = 0, deaths = 0, kills = 0;
    int items[ITEM_COUNT] = {5, 0, 0, 0, 0, 0};   // a new hero starts with 5 RED POTION
    int weapon = 0, armor = 0, refine = 0, shards = 0;
    uint32_t keys = 0;
    int quests[QUEST_COUNT] = {};
    int ilsaTalks = 0;
};

struct SlotSummary { bool used; int heroClass, level, job, highest, ngPlus; float playtime; bool secretDone; };

// The hero's body; combat fields come in phase 2.
struct Hero {
    float x, y, vx, vy;          // top-left of the 8x14 hitbox
    bool facingLeft, onGround, climbing, doubleJumpUsed;
    float coyote, jumpBuffer;    // s left of each grace window
    float lastSafeX, lastSafeY;  // where the hero last stood on safe ground (pit deaths drop names there)
    float stepTimer;             // s until the next footstep sound
    float dropTimer;             // s left of falling through one-way platforms (down + X)
    bool jumped;                 // moveHero started a jump (or double jump) this step
};

// The current area's tiles as they are now (gates, crumbles, shortcut doors change them).
struct AreaState { int world, index; const AreaDef* def; std::vector<char> tiles; };

enum QuestMode { QMODE_SLOTS, QMODE_DELETE, QMODE_CLASS, QMODE_INTRO, QMODE_PLAY, QMODE_DEAD };   // later phases add more

struct Quest {
    QuestMode mode;
    std::string folder;                 // folder of quest1..3.txt
    SlotSummary slots[SLOT_COUNT];
    int slot, cursor;
    SaveData save;
    Hero hero;
    AreaState area;
    float camX, camY;
    float time, modeTimer;              // s since questOpen; s in the current mode
    int introPage;
    const char* banner; int bannerScale; float bannerTimer;
    bool soundRequested[SOUND_COUNT];
    bool wantsMainMenu;                 // main.cpp goes back to the main menu and clears it
    bool devAids, godMode;
    float trauma;
    float shakeStrength = 1; bool reduceFlashing;   // draw-only, copied from Settings by main.cpp
    std::vector<std::string> logLines;  // spec §18 lines for main.cpp to print
};

void questOpen(Quest& q, const char* folder);             // reads the slot summaries, shows the slot screen
void questUpdate(Quest& q, const Actions& a, float dt);
void questDraw(const Quest& q);
extern double (*questClock)();                            // dev timing in s, set by main.cpp; nullptr: drawPlay doesn't measure
extern float questDrawMs[3];                              // worst ms of layers, tiles + props, lights + vignette; main.cpp resets it
bool questCanPause(const Quest& q);                       // true in PLAY (and, later, its menus)
bool questSave(Quest& q);                                 // writes the current slot (spec §17); false on failure
void questStartDev(Quest& q, const char* spec);           // NAMEGAME_QUEST (spec §20)
// Movement and tiles, also used by the reachability test:
char tileAt(const AreaState& area, int tx, int ty);       // outside: '#' past a closed edge, '.' past an open one; '.' below (a pit)
void loadArea(AreaState& area, int world, int index, uint32_t gates, uint32_t shortcuts);   // copies the tiles, opens gates and doors
void moveHero(Hero& hero, const Actions& a, const AreaState& area, HeroClass heroClass, float dt);
bool heroInLiquid(const Hero& hero, const AreaState& area);
// Save files (quest_save.cpp; stdio only):
bool saveWrite(const char* folder, int slot, const SaveData& save, bool* renameFailed = nullptr);   // renameFailed: wrote directly
bool saveRead(const char* folder, int slot, SaveData& save);    // false when missing
bool saveDelete(const char* folder, int slot);

// Shared by quest.cpp, quest_draw.cpp and the tests.
const float HERO_W = 8 * DP, HERO_H = 14 * DP;
extern const char* const CLASS_NAMES[CLASS_COUNT];        // KNIGHT, ARCHER, MAGICIAN, THIEF
extern const char* const INTRO_PAGES[4];                  // spec §1, lines split by '\n'
const float INTRO_CHARS_PER_S = 50;
bool tileSolid(char t);                                   // blocks movement from every side
