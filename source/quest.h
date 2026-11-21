// NAME QUEST: the RPG platformer mode (docs/features/name-quest). No SDL or PS3 headers, so `make test` builds it on the Mac.
#pragma once
#include "game.h"
#include <stdint.h>
#include <string>
#include <vector>

// Design pixels (dp, the 320x180 design's units) → real pixels at 640x360 (spec "Resolution", D-053).
const float DP = 2;
const int TILE = (int)(8 * DP), WIDE_COLS = 80, WIDE_ROWS = 23, TALL_COLS = 40, TALL_ROWS = 45;
const int WORLD_NAMULA = 0, WORLD_SECRET = 7, WORLD_TEST = 8, WORLD_COUNT = 9;   // 1-6 the worlds
const int SAVED_WORLDS = 9;   // per-world save keys for 0-7 (spec §17) and TEST GROUNDS' dev progress
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
    int npcs[8];             // NpcId by 'N' order, -1 unused (Namula's cliffs hold 7)
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
    uint32_t lit = 1 | 1u << 2;            // bit world*2 + id; bit 0: the hearth, bit 2: world 1's entry (lit from the start, §11)
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
// One roster slot of a world (spec §8 rosters): 4 movement frames (2 for the doubled shooter sprites), a tell and an
// attack pose (§22; nullptr: the movement frames are used).
struct EnemyType { const char* name; Behaviour behaviour; EnemyKind kind; EnemyTrait trait;
                   const Sprite* frames[4]; float speedScale; Sound attackSound; const Sprite* tell; const Sprite* attack; };
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
const int PROJ_BANANA = 7, PROJ_COCONUT = 8, PROJ_PEEL = 9, PROJ_POOP = 10;   // enemy throws drawn with their sprites (spec §21)
const int PROJ_ICICLE = 11, PROJ_SNOWBALL = 12, PROJ_ICE_CUBE = 13, PROJ_SHARD = 14;   // world 3's (the ice cube freezes, no damage)
// Hero shots: multiplier, skill or basic, enemies and bosses already hit. gravity: a lobbed throw's pull (px/s²);
// trap: damage is taken as it is (a share of the hero's max HP, spec §7), not through DEF.
struct Projectile { float x, y, vx, vy; int w, h; int damage; bool fromHero; int pierce; float range; int kind;
                    float mult; bool strong; uint64_t hitMask; uint8_t bossMask; float gravity; bool trap; };
struct DamageNumber { float x, y, age; int value; uint32_t color; float life; bool names; };   // names: "+n NAMES"
// Short-lived skill shapes for drawing (arcs, bursts, chains, Meteor's marker, a boss's beam and growing ball); x2/y2: a chain link's end.
enum EffectKind { FX_SWING, FX_ARC, FX_BURST, FX_CHAIN, FX_METEOR, FX_SPIN, FX_BEAM, FX_BALL, FX_SPARK, FX_ZONE, FX_GHOST };
const size_t PARTICLE_CAP = 200;   // no new particle while this many are alive (phase 5b readability)
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
    float attackTimer, strongTimer;   // s left of the basic cycle and of the strong attack
    float rollTimer, rollCooldown, iframes, hurtTimer;
    float partnerTimer;          // Shadow Partner's s left
    float buffTimers[3];         // ATK, DEF, SPD: s left of Whetstone, Iron Tonic, Swift Feather
    float drinkTimer;            // s left of drinking a potion (heals at its end)
    int drinkItem;               // the potion being drunk (ItemType)
    int swing;                   // id of the current melee swing (each enemy is hit once per swing)
    bool attackUp;               // the basic attack aims up (the Thief with up held)
    int skill = -1;              // the strong attack playing (job tier), -1 none
    int volleys;                 // Storm Volley fans still to fire
    float landTimer;             // s left of the landing pose (drawing only)
    bool fireHeld;               // R1 held this step (the bow and staff draw while it is)
    float dashDir, skillX, skillY;   // Shadow Dash's direction; Meteor's target
    float slideTimer, slideSpeed;    // a banana peel: s left of sliding without control, signed speed (spec §12 world 2)
    float frozenTimer;               // the Yeti's ice block: s left without control; X or R1 presses cut it (spec §12 world 3)
    bool fireWasHeld;                // R1 held last frame (a press is a new hold)
};
// The hero's drawn pose (spec §22): a body frame, or an arm pose over legs; t: s into the basic attack / skill, -1 none.
enum HeroBody { BODY_IDLE_1, BODY_IDLE_2, BODY_RUN_1, BODY_RUN_2, BODY_RUN_3, BODY_RUN_4, BODY_JUMP, BODY_PEAK, BODY_FALL,
                BODY_LAND, BODY_CLIMB_1, BODY_CLIMB_2, BODY_ROLL_1, BODY_ROLL_2, BODY_HURT };
enum HeroArms { ARMS_NONE = -1, ARMS_WINDUP, ARMS_ACTIVE, ARMS_RECOVER, ARMS_SKILL_1, ARMS_SKILL_2, ARMS_SKILL_3, ARMS_SKILL_4, ARMS_SKILL_5 };
enum HeroLegs { LEGS_STAND, LEGS_RUN_1, LEGS_RUN_2, LEGS_RUN_3, LEGS_RUN_4, LEGS_AIR };
struct HeroPose { int body, arms, legs; float attackT, skillT; bool slashLit; };   // slashLit: the melee active window
// Things lying in the area for a while: the Monkey King's peels and the Silverback's stink mist (spec §12 world 2).
enum FloorHazardKind { HAZARD_PEEL, HAZARD_MIST };
struct FloorHazard { int kind; float x, y, timer; };   // x, y: a peel's bottom-centre, a mist's centre; s left
// A tile that reacts to the hero: a coconut palm 'R' (world 2), an icicle 'Y' (world 3), a crumbling block 'c'.
// state 0 ready, 1 shaking until `timer` runs out, 2 spent (regrowing, gone, broken) until `timer` runs out.
struct TileTrap { int tile; char kind; int state; float timer; };

// --- Bosses (quest_bosses.cpp; spec §12). QuestBoss, since the shooter's game.h already has a QuestBoss. ---
struct Quest;
enum QBossId { QBOSS_OVERSEER, QBOSS_STAR_MAGE, QBOSS_MONKEY_KING, QBOSS_SILVERBACK, QBOSS_YETI, QBOSS_TITAN, QBOSS_IMP,
               QBOSS_FIRE_LORD, QBOSS_ANGLER, QBOSS_FISHERMAN, QBOSS_TWIN_SWORD, QBOSS_TWIN_SPEAR, QBOSS_NAME_EATER,
               QBOSS_CLAUDE, QBOSS_FIRST_NAME, QBOSS_COUNT };
struct QuestBoss {
    int id; float x, y, vx, vy; int w, h;   // top-left of the hitbox
    int hp, maxHp, atk, bigAtk, contactAtk, names;
    int phase;               // 1, 2 (Claude 3)
    int attack, lastAttack;  // the attack running (-1 none), the one before
    int step; float timer;   // the attack's own step machine: telegraph, act, recover
    float gap;               // s until the next attack
    float shift;             // s of the invulnerable phase-change pose
    float flash; bool facingLeft, alive;
    float started;           // q.time when the fight began (the log's duration)
    float data[8];           // per-boss scratch (anchors chosen, wave counts...); each use named in quest_bosses.cpp
    int lastSwing, lastPartnerSwing;   // the hero's swing ids that already hit it
    bool big;                // the world's big boss (stays dead), else its mini-boss (back after a rest)
};
struct Marker { float x, y, w, h, timer; char color; };   // a telegraph drawn as dashes until timer runs out
enum BossPose { POSE_IDLE, POSE_TELL, POSE_ATTACK, POSE_OPEN };
const char* bossName(int id);
void startBoss(Quest& q, int id, float x, float y);       // stats from §7, banner, bar, exits closed, log "start"
void startArenaFight(Quest& q);                           // a mini-boss or boss area whose boss is due: spawn it at Q / W, lock the arena
void updateBosses(Quest& q, float dt);                    // phases, gaps, the per-boss switch, deaths
bool bossHittable(const QuestBoss& b);                         // alive, not shifting phase, not faded out
int bossPose(const QuestBoss& b);                              // BossPose for drawing: idle, telegraph, attacking, open
float bossVisibility(const QuestBoss& b);                      // 1 shown, 0 faded out (the Star Mage's teleport)
int electricFloor(const Quest& q, int* col0, int* row);   // the Star Mage's phase-2 floor: 0 off, 1 flicker, 2 on; 6 tiles from col0
int titanSpikeLine(const Quest& q, int col, int* row);    // the Titan's SPIKE LINE at a floor column: 0 none, 1 crack, 2 spike up

// The current area's tiles as they are now (gates, crumbles, shortcut doors change them).
struct AreaState {
    int world, index; const AreaDef* def; std::vector<char> tiles;
    bool windFlipped = false;   // world 3 and 6 wind zones push the other way (they flip every 3 s)
    float wind = 0;             // px/s pushed on the hero everywhere (the Glacier Titan's blizzard)
};

enum QuestMode { QMODE_SLOTS, QMODE_DELETE, QMODE_CLASS, QMODE_INTRO, QMODE_PLAY, QMODE_DEAD, QMODE_DIALOGUE, QMODE_MENU,
                 QMODE_SHOP, QMODE_MAP, QMODE_CREDITS };   // CREDITS: phase 8

// One generic list menu (spec §15); the confirm is one switch on kind in questUpdate.
enum MenuKind { MENU_CHECKPOINT, MENU_TRAVEL, MENU_LEVEL_UP, MENU_JOB_CONFIRM, MENU_YES_NO };
struct MenuState { int kind; std::vector<std::string> items; int cursor, scroll; };
const int MENU_ROWS = 8;                                   // rows shown before the list scrolls
// The NPC dialogue box (spec §14): pages of up to 4 lines split by '\n'; `after` opens when the last page closes.
enum DialogueAfter { AFTER_NONE, AFTER_POTION_SHOP, AFTER_GEAR_SHOP, AFTER_LEVEL_UP, AFTER_JOB, AFTER_MAP, AFTER_YES_NO };
struct DialogueState { std::vector<std::string> pages; int page; float typed; std::string speaker; int after; };
const float TALK_CHARS_PER_S = 50;

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
    // RPG systems (phase 3)
    char bannerColor;
    DialogueState dialogue;
    MenuState menu;
    int atCheckpoint;                   // world * 2 + id of the checkpoint whose menu is open
    int shopKind, shopCursor;           // AFTER_POTION_SHOP or AFTER_GEAR_SHOP; the selected row
    int selectedItem;                   // ItemType Square uses
    std::string popup; float popupTimer, popupX, popupY;   // chest text over the chest (shown after the lid opens)
    int chestTile; float chestTimer;    // the chest whose lid is opening, s left
    float gateTimer;                    // s left of the gate shaking and vanishing (spec §11)
    float fadeTimer;                    // s left of the rest's white fade
    int ambient;                        // the place for audioSetQuestSound: 0 off, 1 Namula, 2-8 worlds 1-7
    // Bosses (phase 4)
    std::vector<QuestBoss> bosses;
    std::vector<Marker> markers;
    bool arenaLocked;                   // a boss fight runs: every exit of the area is closed (spec §12)
    // Worlds 2-3 (phase 5)
    std::vector<FloorHazard> hazards;
    std::vector<TileTrap> traps;
    float mistDamage;                   // the stink mist's HP loss not yet taken (whole points are taken as they add up)
};

void questOpen(Quest& q, const char* folder);             // reads the slot summaries, shows the slot screen
void questUpdate(Quest& q, const Actions& a, float dt);
void questDraw(const Quest& q);
extern double (*questClock)();                            // dev timing in s, set by main.cpp; nullptr: drawPlay doesn't measure
extern float questDrawMs[3];                              // worst ms of layers, tiles + props, lights + vignette; main.cpp resets it
bool questCanPause(const Quest& q);                       // true in PLAY, DEAD and the quest menus
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
HeroPose heroPose(const Quest& q);                          // quest_combat.cpp (it owns the attack tables)
void updateEffects(Quest& q, float dt);                   // particles, damage numbers, skill shapes (also during hit-stop)
bool panelOn(const Quest& q);                             // electric panels `z` (spec §9)
bool jetBurning(const Quest& q);                          // fire jets `f`
float jetWarning(const Quest& q);                         // > 0 while a jet sparks before burning
int objectIndex(int world, int area, int tile, char letter);   // the n-th `letter` of the world (areas in order, rows top to bottom)
std::string checkpointName(int bit);                      // spec §11: <WORLD> ENTRY / DEPTHS, HEARTH OF NAMES
std::vector<int> travelCheckpoints(const SaveData& save); // world * 2 + id: the hearth, then every lit checkpoint in world order
void openGate(Quest& q, int world);                       // spec §11: the mini-boss's death opens the world's gate
void openShortcut(Quest& q);                              // a hero attack touched the area's lever K
const float HURT_VY = 140 * DP;                           // the hurt hop (spec §5)
void hurtHero(Quest& q, int damage, float sourceX, float bounceVy = HURT_VY);   // knockback, i-frames, number (not in i-frames or god mode)
void spawnSummon(Quest& q, int slot, float x, float y);   // a roster enemy standing at (x, y) bottom-centre; gives no names (spec §12)
float random01(Quest& q);                                 // the combat RNG, 0..1
void logLine(Quest& q, const char* format, ...);          // one spec §18 line for main.cpp to print

// --- NPCs, shops, levels and chests (quest_story.cpp; spec §10, §14). ---
enum NpcId { NPC_ILSA, NPC_MARRA, NPC_BRANN, NPC_LIO, NPC_PELL, NPC_SELA, NPC_ODO, NPC_WREN, NPC_YSOLDE, NPC_KIT,
             NPC_GATEKEEPER, NPC_TOBIN, NPC_GARRICK, NPC_STRANGER, NPC_CALDER, NPC_CLAUDE, NPC_SURVEYOR_FIRST,
             NPC_COUNT = NPC_SURVEYOR_FIRST + 12 };
const char* npcName(int id);
bool npcPresent(const SaveData& save, int id, int world, int area);   // the Stranger after world 3, Garrick moving, Claude after the credits
void talkTo(Quest& q, int id);                                        // fills the dialogue pages and `after`, applies quest effects
extern const char* const ITEM_NAMES[ITEM_COUNT];
extern const char* const KEY_NAMES[KEY_COUNT];
extern const char* const WEAPON_NAMES[CLASS_COUNT][7];
extern const char* const ARMOR_NAMES[6];
// One shop row: an item, or the next weapon or armor tier (spec §10).
enum ShopGoods { GOODS_WEAPON = ITEM_COUNT, GOODS_ARMOR };
struct ShopRow { int goods; std::string name, detail; int price; bool canBuy; };
std::vector<ShopRow> shopRows(const SaveData& save, int shopKind);   // AFTER_POTION_SHOP or AFTER_GEAR_SHOP
void shopBuy(Quest& q, int row);                          // buys the row if it can, else SOUND_ROCKET_EMPTY
bool levelUp(Quest& q);                                   // spends levelCost(level) with Ilsa; false when unaffordable or at 40
void grantJob(Quest& q);                                  // the next job tier from the class mentor (spec §6 "Jobs")
enum ChestKind { CHEST_ITEM, CHEST_NAMES, CHEST_KEY, CHEST_SHARD, CHEST_PAGE };
struct ChestContent { ChestKind kind; int item; int count; };   // item: ItemType, KeyItem or page number
const ChestContent& chestContent(int world, int chestId);        // spec §10 chest table; world 8 has 2 test chests
int itemPrice(const SaveData& save, int item);            // base x PRICE_SCALE[highest] (quest_stats.cpp)
int carryLimit(const SaveData& save, int item);           // spec §10 with shards, the locket and Garrick's gift
int gearPrice(int tier, bool weapon);                     // tiers 1..6

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
