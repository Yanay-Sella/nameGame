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

// --- Stats and formulas (quest_stats.cpp; spec §6, §7, §10). Pure functions and tables. ---
struct ClassInfo { const char* name; float hp, hpG, mp, mpG, atk, atkG, def, defG, walk, mpRegen, crit, critMult; };
extern const ClassInfo CLASSES[CLASS_COUNT];               // spec §6 base table (walk in dp/s)
extern const char* const JOB_NAMES[CLASS_COUNT][4];        // KNIGHT VANGUARD WARDEN LIONHEART, ...
extern const char* const SKILL_NAMES[CLASS_COUNT][4];      // POWER SLASH ... SHADOW PARTNER
struct HeroStats { int maxHp, maxMp, atk, def; float walk, mpRegen, crit, critMult; };
HeroStats heroStats(const SaveData& save);                 // class, level, job, weapon, armor, refine, charm (spec §6, §10)
struct RefStats { float hp, atk, def; };
RefStats referenceHero(int world);                         // spec §7: average class at REF_LEVEL with REF_GEAR and the level's job
enum EnemyKind { KIND_FODDER, KIND_SOLDIER, KIND_ELITE, KIND_MINIBOSS, KIND_BOSS };
int enemyAttack(int world, float hitFraction, int ngPlus);
int enemyHealth(int world, float hitsToKill, int ngPlus);
int enemyNames(int world, float namesMult, int ngPlus, bool strangerRing);
int levelCost(int level);                                  // names to go from level to level + 1 (spec §10)
int damageToHero(int enemyAtk, int heroDef);               // spec §7
int damageToEnemy(float heroAtk, float mult, float roll, bool crit, float critMult, float enemyMod);   // spec §7, roll 0.9..1.1
float jobMultiplier(int job);                              // 1, 1.1, 1.2, 1.3
const int MAX_NAMES = 99999999;

// --- Enemies (quest_combat.cpp; spec §8). ---
struct Sprite;
enum Behaviour { BEH_WALKER, BEH_CHARGER, BEH_FLYER, BEH_SHOOTER, BEH_JUMPER, BEH_ARMORED, BEH_DROPPER };
enum EnemyTrait { TRAIT_NONE, TRAIT_BANANA_ON_LAND, TRAIT_REVIVE, TRAIT_INFLATE, TRAIT_STEAL_NAMES };
// One roster slot of a world (spec §8 rosters).
struct EnemyType { const char* name; Behaviour behaviour; EnemyKind kind; EnemyTrait trait;
                   const Sprite* frames[2]; float speedScale; Sound attackSound; };
const int ROSTER_SLOTS = 5;
extern const char ROSTER_LETTERS[ROSTER_SLOTS + 1];       // "ABDEF"
const EnemyType& rosterType(int world, int slot);          // slot 0..4 = A, B, D, E, F
int statsWorld(int world);                                 // the world whose reference hero tunes an area's enemies
enum EnemyState { ES_PATROL, ES_TELL, ES_ACT, ES_RECOVER, ES_STUNNED, ES_RETURN, ES_HANG, ES_EMBER };
struct Enemy {
    int type;                 // roster slot
    int world;                // its roster's world (TEST GROUNDS area 3 borrows world 2's charger and dropper)
    int spawn;                // index of its letter in the area (bit in the killed list)
    float x, y, vx, vy;       // top-left of the hitbox
    float w, h;               // hitbox: the sprite minus 2 dp each side
    float homeX, homeY;       // spawn point (flyers drift around it)
    int hp, maxHp, atk, names;
    int state; float stateTimer;   // behaviour state machine (EnemyState)
    bool facingLeft, alive, onGround;
    float flash, knockTimer, slowTimer, stunTimer;
    float targetX, targetY;   // flyer swoop target, taken at the tell's start
    bool revived;             // phoenix
    int lastSwing, lastPartnerSwing;   // the hero's swing ids that already hit it
    int stolen; float stolenTimer;     // name wraith: names taken, s left to win them back
};
const int PROJ_ENEMY = 0, PROJ_ARROW = 1, PROJ_BOLT = 2, PROJ_STAR = 3, PROJ_ORB = 4, PROJ_WAVE = 5, PROJ_RAIN = 6;
struct Projectile { float x, y, vx, vy; int w, h; int damage; bool fromHero; int pierce; float range; int kind;
                    float mult; bool strong; uint64_t hitMask; };   // hero shots: multiplier, skill or basic, enemies already hit
struct DamageNumber { float x, y, age; int value; uint32_t color; float life; bool names; };   // names: "+n NAMES"
// Short-lived skill shapes for drawing (arcs, bursts, chains, Meteor's marker); x2/y2: a chain link's end.
enum EffectKind { FX_SWING, FX_ARC, FX_BURST, FX_CHAIN, FX_METEOR, FX_SPIN };
struct Effect { int kind; float x, y, w, h, x2, y2, age, life; bool flip; char color; };

// The hero's body and combat state.
struct Hero {
    float x, y, vx, vy;          // top-left of the 8x14 hitbox
    bool facingLeft, onGround, climbing, doubleJumpUsed;
    float coyote, jumpBuffer;    // s left of each grace window
    float lastSafeX, lastSafeY;  // where the hero last stood on safe ground (pit deaths drop names there)
    float stepTimer;             // s until the next footstep sound
    float dropTimer;             // s left of falling through one-way platforms (down + X)
    bool jumped;                 // moveHero started a jump (or double jump) this step
    int hp, mp; float mpFraction;   // MP regen accumulates in mpFraction
    float attackTimer, strongTimer, castTimer;   // s left of the basic cycle, the strong attack, its rooted cast
    float rollTimer, rollCooldown, iframes, hurtTimer;
    float partnerTimer;          // Shadow Partner's s left
    float buffTimers[3];         // ATK, DEF, SPD (phase 3)
    float drinkTimer;            // phase 3
    int swing;                   // id of the current melee swing (each enemy is hit once per swing)
    int skill = -1;              // the strong attack playing (job tier), -1 none
    int volleys;                 // Storm Volley fans still to fire
    float dashDir, skillX, skillY;   // Shadow Dash's direction; Meteor's target
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
    // Combat (phase 2)
    HeroStats stats;                    // recomputed by questRefreshStats
    std::vector<Enemy> enemies;
    std::vector<Projectile> projectiles;
    std::vector<DamageNumber> numbers;
    std::vector<Particle> particles;
    std::vector<Effect> effects;
    std::vector<uint64_t> killed[WORLD_COUNT];   // per area, bits by spawn index; cleared on rest, travel, death
    float hitStop;
    uint32_t seed = 12345;              // the combat RNG
    float selectHeldTime;               // dev: Select held 2 s → TEST GROUNDS
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
// Combat (quest_combat.cpp):
void questRefreshStats(Quest& q, bool fullHeal);          // stats from the save; fullHeal: HP and MP full
void spawnEnemies(Quest& q);                              // the current area's A B D E F whose killed bit is clear
void updateCombat(Quest& q, const Actions& a, float dt);  // attacks, dodge, movement, enemies, shots, damage (PLAY)
void updateEffects(Quest& q, float dt);                   // particles, damage numbers, skill shapes (also during hit-stop)
bool panelOn(const Quest& q);                             // electric panels `z` (spec §9)
bool jetBurning(const Quest& q);                          // fire jets `f`
float jetWarning(const Quest& q);                         // > 0 while a jet sparks before burning
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
