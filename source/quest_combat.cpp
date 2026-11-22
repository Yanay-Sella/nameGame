// NAME QUEST combat (spec §5-§8): the hero's attacks, skills and dodge, enemy behaviours, shots, damage and hit feel.
#include "quest.h"
#include "screen.h"
#include "quest_sprites_1.h"
#include "quest_sprites_2.h"
#include "quest_sprites_3.h"
#include <algorithm>
#include <math.h>
#include <string.h>
#include <string>

// Hero (spec §5, §6), dp x DP.
const float ROLL_TIME = 0.35f, ROLL_SPEED = 140 * DP, ROLL_IFRAMES = 0.25f, ROLL_COOLDOWN = 0.6f;
const float BLINK_DISTANCE = 48 * DP, BLINK_IFRAMES = 0.2f, BLINK_COOLDOWN = 0.8f;
const float HURT_VX = 90 * DP, HURT_TIME = 0.25f, HURT_IFRAMES = 1.0f;
const int BASIC_COMBO_LENGTH = 3;
const float BASIC_MULT_BY_JOB[4] = {1.0f, 1.15f, 1.3f, 1.5f};
// The basic attack per class: cycle, multiplier, melee window and box, or the shot's speed, range and size.
struct BasicAttack { float cycle, mult, from, to, boxW, boxH, speed, range, shotW, shotH; Sound sound; };
const BasicAttack BASIC[CLASS_COUNT] = {
    {0.40f, 1.0f, 0.08f, 0.18f, 22 * DP, 18 * DP, 0, 0, 0, 0, SOUND_Q_SWING},
    {0.45f, 1.0f, 0, 0, 0, 0, 240 * DP, 170 * DP, 6 * DP, 2 * DP, SOUND_Q_ARROW},
    {0.50f, 1.1f, 0, 0, 0, 0, 200 * DP, 130 * DP, 6 * DP, 6 * DP, SOUND_Q_SPELL},
    {0.26f, 0.7f, 0.04f, 0.10f, 16 * DP, 12 * DP, 0, 0, 0, 0, SOUND_Q_STAB},
};
// Strong attacks by class and job tier (spec §6): MP cost and how long the skill plays.
const int SKILL_MP[CLASS_COUNT][4] = {{10, 12, 16, 24}, {8, 14, 18, 26}, {10, 14, 20, 34}, {8, 12, 16, 30}};
const float SKILL_TIME[CLASS_COUNT][4] = {{0.6f, 0.3f, 0.3f, 0.3f}, {0.25f, 0.25f, 0.25f, 0.45f}, {0.3f, 0.3f, 0.3f, 0.6f},
                                          {0.25f, 0.15f, 0.5f, 0.25f}};
const float PARTNER_TIME = 12, PARTNER_BEHIND = 10 * DP, PARTNER_MULT = 0.5f;
const float METEOR_TIME = 0.6f;   // Meteor lands 0.6 s after the press (spec §6)
const float SHADOW_DASH_SPEED = 72 * DP / 0.15f;
const float RUN_FRAME = 0.1f;   // s per run frame (spec §22)
// Hit feel (spec §7).
const float BOSS_FLASH = 0.2f;   // a boss is white for its first 0.08 s, then shows its hurt frame
const float FLASH_TIME = 0.08f, KNOCK_TIME = 0.1f, KNOCK_SPEED = 40 * DP, HIT_STOP = 0.04f, HURT_STOP = 0.08f, HURT_TRAUMA = 0.3f;
const float NUMBER_TIME = 0.6f;
const float BASIC_HIT_STOP = 0.025f, SPARK_TIME = 0.12f;   // melee basic hits stop the action briefly (D-063); hit sparks
const int MAX_NUMBERS = 24, KILL_PARTICLES = 8;
// Enemy stats by kind (spec §7): fodder, soldier, elite.
const float HIT_FRACTION[3] = {0.15f, 0.20f, 0.25f}, HITS_TO_KILL[3] = {3, 5, 12}, NAMES_MULT[3] = {1, 2, 5};
// Behaviours (spec §8).
const float ENEMY_GRAVITY = 700 * DP, ENEMY_MAX_FALL = 260 * DP;   // the hero's (spec §5)
const float WALKER_SPEED = 30 * DP;
const float CHARGER_PATROL = 25 * DP, CHARGER_SEE_X = 96 * DP, CHARGER_SEE_Y = 16 * DP, CHARGER_WINDUP = 0.5f;
const float CHARGE_SPEED = 160 * DP, CHARGE_TIME = 1.0f, CHARGER_STUN = 0.8f, STUNNED_MOD = 1.5f;
const float FLYER_BOB = 10 * DP, FLYER_BOB_RATE = 1.5f, FLYER_DRIFT = 48 * DP, FLYER_SPEED = 30 * DP, FLYER_SEE = 100 * DP;
const float FLYER_TELL = 0.4f, SWOOP_SPEED = 120 * DP, SWOOP_TIME = 0.8f, FLYER_RETURN = 50 * DP;
const float SHOOTER_SEE = 160 * DP, SHOOTER_PERIOD = 2.4f, SHOOTER_GLOW = 0.5f, SHOT_SPEED = 90 * DP, SHOT_LIFE = 3;
const int SHOT_SIZE = (int)(4 * DP);
const float JUMPER_SEE = 120 * DP, JUMPER_PERIOD = 1.2f, JUMPER_IDLE_PERIOD = 2, JUMPER_VX = 60 * DP, JUMPER_VY = 200 * DP;
const float ARMORED_SEE = 120 * DP, ARMORED_SPEED = 18 * DP, ARMORED_REACH = 24 * DP, ARMORED_WINDUP = 0.7f;
const float ARMORED_SWING_W = 28 * DP, ARMORED_SWING_H = 16 * DP, ARMORED_SWING_TIME = 0.2f, ARMORED_SWING_MULT = 1.3f;
const float ARMORED_RECOVER = 0.8f, GUARD_MOD = 0.25f, GUARD_OVER = 4 * DP;
const float DROPPER_SEE_X = 16 * DP, DROP_SPEED = 260 * DP, DROPPER_WALK = 40 * DP;
const float INFLATE_RANGE = 32 * DP, INFLATE_TIME = 1, INFLATE_MULT = 1.5f, EMBER_TIME = 2, STEAL_FRACTION = 0.1f, STEAL_TIME = 10;
// Traps use the hero's max HP (spec §7, §9).
const float SPIKE_DAMAGE = 0.20f, PANEL_DAMAGE = 0.15f, JET_DAMAGE = 0.20f, SPIKE_BOUNCE = 180 * DP;   // spikes bounce up (spec §4)
const float PANEL_ON = 2.0f, JET_CYCLE = 2.5f, JET_WARNING = 0.5f, JET_FLAME = 0.8f;
const int JET_TILES = 3;
// Buffs (spec §10): Whetstone ATK, Iron Tonic damage taken, Swift Feather walk; drinking a potion slows the walk.
const float WHETSTONE_ATK = 1.2f, TONIC_DAMAGE = 0.8f, FEATHER_WALK = 1.15f, DRINK_WALK = 0.4f;
const float LEVER_W = 8 * DP, LEVER_H = 14 * DP;   // the lever K's box, standing on the tile below its letter
// World 2 (spec §9, §12): coconut palms R, banana peels, stink mist.
const float COCONUT_REACH = 8 * DP, COCONUT_DELAY = 0.3f, COCONUT_SPEED = 200 * DP, COCONUT_REGROW = 4, COCONUT_DAMAGE = 0.20f;
const int COCONUT_SIZE = (int)(6 * DP);
// World 3 (spec §9): icicles drop 0.25 s after the hero is within 12 px below; crumbling blocks shake 0.6 s, break, return after 4 s.
const float ICICLE_REACH = 12 * DP, ICICLE_DELAY = 0.25f, ICICLE_SPEED = 280 * DP, ICICLE_DAMAGE = 0.20f;
const int ICICLE_W = (int)(4 * DP), ICICLE_H = (int)(9 * DP);
const int ICICLE_BIT = 40;   // killed bits 40+ are an area's dropped icicles (gone until rest), by 'Y' order
const float CRUMBLE_SHAKE = 0.6f, CRUMBLE_RETURN = 4;
const float WIND_FLIP = 3;   // world 3 and 6 wind zones flip every 3 s
const float FROZEN_TIME = 1.2f, FROZEN_PRESS_CUT = 0.15f;   // the Yeti's ice block (spec §12 world 3)
const float PEEL_W = 8 * DP, PEEL_H = 6 * DP, PEEL_SLIDE = 0.5f, PEEL_MIN_SPEED = 100 * DP;
const float MIST_RADIUS = 20 * DP, MIST_HURT = 0.03f, MIST_TIME = 6, PEEL_TIME = 6;   // 40 px across; 3 % of max HP per s inside

const char ROSTER_LETTERS[ROSTER_SLOTS + 1] = "ABDEF";

// Stand-in for the new enemies until their world's phase draws them (12 x 12 dp, doubled when used).
static const Sprite PLACEHOLDER_ENEMY_A = {12, 12,
    "..ssssssss.."
    ".snnnnnnnns."
    "snnnnnnnnnns"
    "snnwwnnwwnns"
    "snnwknnwknns"
    "snnnnnnnnnns"
    "snnnnnnnnnns"
    "snnssssssnns"
    "snnnnnnnnnns"
    ".snnnnnnnns."
    "..sn.ss.ns.."
    "..ss....ss.."};
static const Sprite PLACEHOLDER_ENEMY_B = {12, 12,
    "............"
    "..ssssssss.."
    ".snnnnnnnns."
    "snnnnnnnnnns"
    "snnwwnnwwnns"
    "snnkwnnkwnns"
    "snnnnnnnnnns"
    "snnssssssnns"
    "snnnnnnnnnns"
    ".snnnnnnnns."
    "...sn..ns..."
    "...ss..ss..."};
#define PLACEHOLDER_ENEMY {&PLACEHOLDER_ENEMY_A, &PLACEHOLDER_ENEMY_B}

// Rosters of worlds 1-6 (spec §8). Worlds 1-2 are redrawn side-view (quest_sprites_1, _2); worlds 3-6 still hold the 1x
// shooter sprites each will be redrawn from, which rosterType doubles.
#define ENEMY_ROW(N) {&ENEMY_##N##_WALK_1, &ENEMY_##N##_WALK_2, &ENEMY_##N##_WALK_3, &ENEMY_##N##_WALK_4}
static EnemyType ROSTERS[6][ROSTER_SLOTS] = {
    {{"CRAWLER", BEH_WALKER, KIND_FODDER, TRAIT_NONE, ENEMY_ROW(CRAWLER), 1, SOUND_Q_STEP_METAL, &ENEMY_CRAWLER_TELL, &ENEMY_CRAWLER_ATTACK},
     {"FLOATER", BEH_FLYER, KIND_FODDER, TRAIT_NONE, ENEMY_ROW(FLOATER), 1, SOUND_SCREECH, &ENEMY_FLOATER_TELL, &ENEMY_FLOATER_ATTACK},
     {"GUNNER", BEH_SHOOTER, KIND_SOLDIER, TRAIT_NONE, ENEMY_ROW(GUNNER), 1, SOUND_Q_ZAP, &ENEMY_GUNNER_TELL, &ENEMY_GUNNER_ATTACK},
     {"HOPPER DRONE", BEH_JUMPER, KIND_SOLDIER, TRAIT_NONE, ENEMY_ROW(HOPPER), 1, SOUND_Q_JUMP, &ENEMY_HOPPER_TELL, &ENEMY_HOPPER_ATTACK},
     {"SENTINEL", BEH_ARMORED, KIND_ELITE, TRAIT_NONE, ENEMY_ROW(SENTINEL), 1, SOUND_Q_SWING, &ENEMY_SENTINEL_TELL, &ENEMY_SENTINEL_ATTACK}},
    {{"LEAFHOPPER", BEH_JUMPER, KIND_FODDER, TRAIT_NONE, ENEMY_ROW(LEAFHOPPER), 1, SOUND_Q_JUMP, &ENEMY_LEAFHOPPER_TELL, &ENEMY_LEAFHOPPER_ATTACK},
     {"BEETLE", BEH_WALKER, KIND_FODDER, TRAIT_NONE, ENEMY_ROW(BEETLE), 1, SOUND_Q_STEP_LEAVES, &ENEMY_BEETLE_TELL, &ENEMY_BEETLE_ATTACK},
     {"STAG", BEH_CHARGER, KIND_SOLDIER, TRAIT_NONE, ENEMY_ROW(STAG), 1, SOUND_Q_ROAR, &ENEMY_STAG_TELL, &ENEMY_STAG_ATTACK},
     {"TREE MONKEY", BEH_DROPPER, KIND_SOLDIER, TRAIT_BANANA_ON_LAND, ENEMY_ROW(TREE_MONKEY), 1, SOUND_BANANA_THROW, &ENEMY_TREE_MONKEY_TELL, &ENEMY_TREE_MONKEY_ATTACK},
     {"RHINO", BEH_ARMORED, KIND_ELITE, TRAIT_NONE, ENEMY_ROW(RHINO), 1, SOUND_Q_SWING, &ENEMY_RHINO_TELL, &ENEMY_RHINO_ATTACK}},
    {{"SNOWFLAKE", BEH_FLYER, KIND_FODDER, TRAIT_NONE, ENEMY_ROW(SNOWFLAKE), 1, SOUND_FREEZE, &ENEMY_SNOWFLAKE_TELL, &ENEMY_SNOWFLAKE_ATTACK},
     {"ICE BAT", BEH_DROPPER, KIND_FODDER, TRAIT_NONE, ENEMY_ROW(ICE_BAT), 1, SOUND_SCREECH, &ENEMY_ICE_BAT_TELL, &ENEMY_ICE_BAT_ATTACK},
     {"FROST SKULL", BEH_SHOOTER, KIND_SOLDIER, TRAIT_NONE, ENEMY_ROW(FROST_SKULL), 1, SOUND_FREEZE, &ENEMY_FROST_SKULL_TELL, &ENEMY_FROST_SKULL_ATTACK},
     {"YETI CUB", BEH_CHARGER, KIND_SOLDIER, TRAIT_NONE, ENEMY_ROW(YETI_CUB), 1, SOUND_Q_ROAR, &ENEMY_YETI_CUB_TELL, &ENEMY_YETI_CUB_ATTACK},
     {"ICE KNIGHT", BEH_ARMORED, KIND_ELITE, TRAIT_NONE, ENEMY_ROW(ICE_KNIGHT), 1, SOUND_Q_SWING, &ENEMY_ICE_KNIGHT_TELL, &ENEMY_ICE_KNIGHT_ATTACK}},
    {{"EMBER", BEH_FLYER, KIND_FODDER, TRAIT_NONE, {&EMBER_A, &EMBER_B}, 1, SOUND_SIZZLE, nullptr, nullptr},
     {"FIRE BAT", BEH_DROPPER, KIND_FODDER, TRAIT_NONE, {&FIREBAT_A, &FIREBAT_B}, 1, SOUND_SCREECH, nullptr, nullptr},
     {"MAGMA SKULL", BEH_SHOOTER, KIND_SOLDIER, TRAIT_NONE, {&MAGMASKULL_A, &MAGMASKULL_B}, 1, SOUND_SIZZLE, nullptr, nullptr},
     {"MAGMA CRAB", BEH_CHARGER, KIND_SOLDIER, TRAIT_NONE, PLACEHOLDER_ENEMY, 1, SOUND_Q_ROAR, nullptr, nullptr},
     {"PHOENIX", BEH_FLYER, KIND_ELITE, TRAIT_REVIVE, {&PHOENIX_A, &PHOENIX_B}, 1, SOUND_SCREECH, nullptr, nullptr}},
    {{"PIRANHA", BEH_FLYER, KIND_FODDER, TRAIT_NONE, {&PIRANHA_A, &PIRANHA_B}, 1, SOUND_BUBBLE, nullptr, nullptr},
     {"PUFFER", BEH_WALKER, KIND_FODDER, TRAIT_INFLATE, {&PUFFER_A, &PUFFER_B}, 1, SOUND_BUBBLE, nullptr, nullptr},
     {"LIONFISH", BEH_SHOOTER, KIND_SOLDIER, TRAIT_NONE, {&LIONFISH_A, &LIONFISH_B}, 1, SOUND_BUBBLE, nullptr, nullptr},
     {"SHARK", BEH_CHARGER, KIND_SOLDIER, TRAIT_NONE, {&SHARK_A, &SHARK_B}, 1, SOUND_Q_ROAR, nullptr, nullptr},
     {"IRON CRAB", BEH_ARMORED, KIND_ELITE, TRAIT_NONE, PLACEHOLDER_ENEMY, 1, SOUND_Q_SWING, nullptr, nullptr}},
    {{"NAME WRAITH", BEH_FLYER, KIND_FODDER, TRAIT_STEAL_NAMES, PLACEHOLDER_ENEMY, 1, SOUND_SCREECH, nullptr, nullptr},
     {"GARGOYLE", BEH_DROPPER, KIND_FODDER, TRAIT_NONE, PLACEHOLDER_ENEMY, 1, SOUND_POUND, nullptr, nullptr},
     {"STATUE ARCHER", BEH_SHOOTER, KIND_SOLDIER, TRAIT_NONE, PLACEHOLDER_ENEMY, 1, SOUND_Q_ARROW, nullptr, nullptr},
     {"SHADE HOUND", BEH_CHARGER, KIND_SOLDIER, TRAIT_NONE, PLACEHOLDER_ENEMY, 1, SOUND_Q_ROAR, nullptr, nullptr},
     {"CITADEL KNIGHT", BEH_ARMORED, KIND_ELITE, TRAIT_NONE, PLACEHOLDER_ENEMY, 1, SOUND_Q_SWING, nullptr, nullptr}},
};

// A 1x shooter sprite drawn as chunky 2x2 blocks, like the old-school heroes (kept for the program's life).
static const Sprite* doubled(const Sprite* source) {
    static const int CAPACITY = 64;
    static const Sprite* sources[CAPACITY];
    static Sprite copies[CAPACITY];
    static std::string pixels[CAPACITY];
    static int count = 0;
    for (int i = 0; i < count; i++)
        if (sources[i] == source) return &copies[i];
    if (count == CAPACITY) return source;
    std::string& p = pixels[count];
    for (int row = 0; row < source->h * 2; row++)
        for (int col = 0; col < source->w * 2; col++) p += source->pixels[(row / 2) * source->w + col / 2];
    sources[count] = source;
    copies[count] = {source->w * 2, source->h * 2, p.c_str()};
    return &copies[count++];
}

int statsWorld(int world) {
    return world == WORLD_TEST ? 1 : world == WORLD_NAMULA ? 5 : world;   // Namula's Trial of Echoes uses world 5
}

const EnemyType& rosterType(int world, int slot) {
    static bool ready = false;
    if (!ready) {
        for (int w = 4; w <= 6; w++)
            for (EnemyType& t : ROSTERS[w - 1]) t.frames[0] = doubled(t.frames[0]), t.frames[1] = doubled(t.frames[1]);
        ready = true;
    }
    slot = slot < 0 ? 0 : slot % ROSTER_SLOTS;
    if (world == WORLD_SECRET) {   // world 3's A, world 4's D, world 5's E, world 6's B, world 6's F (spec §8)
        const int from[ROSTER_SLOTS][2] = {{3, 0}, {4, 2}, {5, 3}, {6, 1}, {6, 4}};
        return ROSTERS[from[slot][0] - 1][from[slot][1]];
    }
    world = statsWorld(world);
    return ROSTERS[(world < 1 || world > 6 ? 1 : world) - 1][slot];
}

float random01(Quest& q) {
    q.seed = q.seed * 1664525u + 1013904223u;
    return (q.seed >> 8) / 16777216.0f;
}

static bool overlaps(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) {
    return ax < bx + bw && bx < ax + aw && ay < by + bh && by < ay + ah;
}

static bool boxInSolid(const AreaState& area, float x, float y, float w, float h) {
    for (int r = (int)floorf(y / TILE); r <= (int)floorf((y + h - 0.001f) / TILE); r++)
        for (int c = (int)floorf(x / TILE); c <= (int)floorf((x + w - 0.001f) / TILE); c++)
            if (tileSolid(tileAt(area, c, r))) return true;
    return false;
}

static bool holdsFeet(char t) { return tileSolid(t) || t == '-' || t == '|'; }

static void addNumber(Quest& q, float x, float y, int value, char color, float life = NUMBER_TIME, bool names = false) {
    if ((int)q.numbers.size() >= MAX_NUMBERS) q.numbers.erase(q.numbers.begin());
    q.numbers.push_back({x, y, 0, value, paletteColor(color), life, names});
}

void questRefreshStats(Quest& q, bool fullHeal) {
    q.stats = heroStats(q.save);
    Hero& h = q.hero;
    h.hp = fullHeal ? q.stats.maxHp : h.hp > q.stats.maxHp ? q.stats.maxHp : h.hp;
    h.mp = fullHeal ? q.stats.maxMp : h.mp > q.stats.maxMp ? q.stats.maxMp : h.mp;
}

bool panelOn(const Quest& q) {
    float off = q.area.def && q.area.def->band >= 2 ? 1.0f : 1.5f;
    return fmodf(q.time, PANEL_ON + off) < PANEL_ON;
}

bool jetBurning(const Quest& q) {
    float t = fmodf(q.time, JET_CYCLE);
    return t >= JET_WARNING && t < JET_WARNING + JET_FLAME;
}

float jetWarning(const Quest& q) {
    float t = fmodf(q.time, JET_CYCLE);
    return t < JET_WARNING ? JET_WARNING - t : 0;
}

void spawnEnemies(Quest& q) {
    q.enemies.clear();
    q.projectiles.clear();
    q.hazards.clear();
    q.traps.clear();
    q.mistDamage = 0;
    const AreaDef& def = *q.area.def;
    std::vector<uint64_t>& killed = q.killed[q.area.world];
    int areas = worldDef(q.area.world).count > q.area.index ? worldDef(q.area.world).count : q.area.index + 1;
    if ((int)killed.size() < areas) killed.resize(areas);
    int icicle = 0;
    for (int t = 0; t < def.cols * def.rows; t++) {   // coconut palms (world 2), icicles (world 3), crumbling blocks
        char letter = def.tiles[t];
        if (letter == 'R' && q.area.world == 2) q.traps.push_back({t, 'R', 0, 0});
        if (letter == 'c') q.traps.push_back({t, 'c', 0, 0});
        if (letter == 'Y' && q.area.world == 3) {
            bool fallen = icicle < 64 - ICICLE_BIT && (killed[q.area.index] >> (ICICLE_BIT + icicle) & 1);
            q.traps.push_back({t, 'Y', fallen ? 2 : 0, 0});
            icicle++;
        }
    }
    int world = statsWorld(q.area.world), spawn = 0;
    bool ring = q.save.keys >> KEY_STRANGER_RING & 1;
    for (int t = 0; t < def.cols * def.rows; t++) {
        const char* letter = def.tiles[t] ? strchr(ROSTER_LETTERS, def.tiles[t]) : nullptr;
        if (!letter) continue;
        int index = spawn++;
        if (index < 64 && (killed[q.area.index] >> index & 1)) continue;
        Enemy e = {};
        e.type = (int)(letter - ROSTER_LETTERS);
        bool borrow = q.area.world == WORLD_TEST && q.area.index == 3 && (*letter == 'D' || *letter == 'E');
        e.world = borrow ? 2 : q.area.world;   // so TEST GROUNDS shows all 7 behaviours
        const EnemyType& type = rosterType(e.world, e.type);
        e.spawn = index;
        e.w = type.frames[0]->w - 4 * DP;
        e.h = type.frames[0]->h - 4 * DP;
        int tx = t % def.cols, ty = t / def.cols;
        e.x = tx * TILE + TILE / 2 - e.w / 2;
        e.y = (ty + 1) * TILE - e.h;
        if (type.behaviour == BEH_DROPPER) {   // hangs from the ceiling above its letter
            e.state = ES_PATROL;
            for (int r = ty; r >= 0; r--)
                if (tileSolid(tileAt(q.area, tx, r - 1))) { e.y = (float)(r * TILE); e.state = ES_HANG; break; }
        }
        e.homeX = e.x;
        e.homeY = e.y;
        e.atk = enemyAttack(world, HIT_FRACTION[type.kind], q.save.ngPlus);
        e.hp = e.maxHp = enemyHealth(world, HITS_TO_KILL[type.kind], q.save.ngPlus);
        e.names = enemyNames(world, NAMES_MULT[type.kind], q.save.ngPlus, ring);
        e.alive = true;
        e.facingLeft = q.hero.x < e.x;
        e.lastSwing = e.lastPartnerSwing = -1;
        q.enemies.push_back(e);
    }
}

void spawnSummon(Quest& q, int slot, float x, float y) {
    const EnemyType& type = rosterType(q.area.world, slot);
    int world = statsWorld(q.area.world);
    Enemy e = {};
    e.type = slot;
    e.world = q.area.world;
    e.spawn = 64;   // no killed bit: summons don't stay dead
    e.w = type.frames[0]->w - 4 * DP;
    e.h = type.frames[0]->h - 4 * DP;
    e.x = e.homeX = x - e.w / 2;
    e.y = e.homeY = y - e.h;
    e.atk = enemyAttack(world, HIT_FRACTION[type.kind], q.save.ngPlus);
    e.hp = e.maxHp = enemyHealth(world, HITS_TO_KILL[type.kind], q.save.ngPlus);
    e.names = 0;
    e.alive = true;
    e.facingLeft = q.hero.x < e.x;
    e.lastSwing = e.lastPartnerSwing = -1;
    q.enemies.push_back(e);
}

// Moves a walking enemy against the tiles (one-way tops hold it); true when a wall, a ledge or the area's edge stopped it.
static bool moveEnemyBody(const AreaState& area, Enemy& e, float dt) {
    bool blocked = false;
    float nx = e.x + e.vx * dt, maxX = area.def->cols * TILE - e.w;
    if (nx < 0 || nx > maxX) {
        nx = nx < 0 ? 0 : maxX;
        blocked = true;
    }
    if (boxInSolid(area, nx, e.y, e.w, e.h)) {
        blocked = true;
    } else if (e.onGround && e.vx != 0) {   // never walks off a ledge or into a pit, liquid or spikes
        float lead = e.vx > 0 ? nx + e.w - 0.001f : nx;
        blocked |= !holdsFeet(tileAt(area, (int)floorf(lead / TILE), (int)floorf((e.y + e.h + 1) / TILE)));
    }
    if (!blocked) e.x = nx;
    e.vy = fminf(e.vy + ENEMY_GRAVITY * dt, ENEMY_MAX_FALL);
    float ny = e.y + e.vy * dt;
    e.onGround = false;
    int c0 = (int)floorf(e.x / TILE), c1 = (int)floorf((e.x + e.w - 0.001f) / TILE);
    if (e.vy > 0) {
        int r = (int)floorf((ny + e.h) / TILE);
        for (int c = c0; c <= c1; c++) {
            char t = tileAt(area, c, r);
            if (tileSolid(t) || ((t == '-' || t == '|') && e.y + e.h <= r * TILE + 0.01f)) {
                ny = (float)(r * TILE) - e.h;
                e.vy = 0;
                e.onGround = true;
                break;
            }
        }
    } else if (e.vy < 0) {
        int r = (int)floorf(ny / TILE);
        for (int c = c0; c <= c1; c++)
            if (tileSolid(tileAt(area, c, r))) { ny = (float)((r + 1) * TILE); e.vy = 0; break; }
    }
    e.y = ny;
    return blocked;
}

// An aimed magenta shot (or a banana) from the enemy's centre toward the hero's (spec §8).
static void enemyShoot(Quest& q, const Enemy& e, int kind = PROJ_ENEMY) {
    float ex = e.x + e.w / 2, ey = e.y + e.h / 2;
    float dx = q.hero.x + HERO_W / 2 - ex, dy = q.hero.y + HERO_H / 2 - ey, length = fmaxf(1, sqrtf(dx * dx + dy * dy));
    Projectile p = {};
    p.x = ex - SHOT_SIZE / 2;
    p.y = ey - SHOT_SIZE / 2;
    p.vx = dx / length * SHOT_SPEED;
    p.vy = dy / length * SHOT_SPEED;
    p.w = p.h = SHOT_SIZE;
    p.damage = e.atk;
    p.range = SHOT_SPEED * SHOT_LIFE;
    p.kind = kind;
    q.projectiles.push_back(p);
}

static void updateEnemy(Quest& q, Enemy& e, float dt) {
    const EnemyType& type = rosterType(e.world, e.type);
    const AreaState& area = q.area;
    const Hero& h = q.hero;
    float speed = type.speedScale * (e.slowTimer > 0 ? 0.5f : 1);
    float dx = h.x + HERO_W / 2 - (e.x + e.w / 2), dy = h.y + HERO_H / 2 - (e.y + e.h / 2);
    float distance = sqrtf(dx * dx + dy * dy), dir = e.facingLeft ? -1 : 1;
    bool heroInFront = e.facingLeft ? dx < 0 : dx > 0;
    e.stateTimer += dt;
    e.flash = fmaxf(0, e.flash - dt);
    e.slowTimer = fmaxf(0, e.slowTimer - dt);
    e.stolenTimer = fmaxf(0, e.stolenTimer - dt);
    if (e.state == ES_EMBER) {   // the phoenix's ember pulses, then it revives at half HP
        if (e.stateTimer >= EMBER_TIME) {
            e.hp = e.maxHp / 2;
            e.revived = true;
            e.state = ES_RETURN;
            e.stateTimer = 0;
        }
        return;
    }
    if (e.stunTimer > 0 || e.knockTimer > 0) {   // stunned or knocked back: no behaviour
        e.stunTimer = fmaxf(0, e.stunTimer - dt);
        e.knockTimer = fmaxf(0, e.knockTimer - dt);
        if (e.knockTimer <= 0) e.vx = 0;
        if (type.behaviour == BEH_FLYER) e.x = fmaxf(0, fminf(e.x + e.vx * dt, area.def->cols * TILE - e.w));
        else if (e.state != ES_HANG) moveEnemyBody(area, e, dt);
        return;
    }

    switch (type.behaviour) {
    case BEH_WALKER:
        if (type.trait == TRAIT_INFLATE && e.state == ES_ACT) {   // inflated: still, invulnerable, contact x1.5
            e.vx = 0;
            moveEnemyBody(area, e, dt);
            if (e.stateTimer >= INFLATE_TIME) e.state = ES_RECOVER, e.stateTimer = 0;
            break;
        }
        if (type.trait == TRAIT_INFLATE && e.state == ES_PATROL && distance < INFLATE_RANGE) {
            e.state = ES_ACT;
            e.stateTimer = 0;
            q.soundRequested[type.attackSound] = true;
            break;
        }
        if (e.state == ES_RECOVER && e.stateTimer >= INFLATE_TIME) e.state = ES_PATROL;   // deflated 1 s before it can inflate again
        e.vx = dir * WALKER_SPEED * speed;
        if (moveEnemyBody(area, e, dt)) e.facingLeft = !e.facingLeft;
        break;

    case BEH_CHARGER:
        switch (e.state) {
        case ES_TELL:
            e.vx = 0;
            moveEnemyBody(area, e, dt);
            if (e.stateTimer >= CHARGER_WINDUP) e.state = ES_ACT, e.stateTimer = 0;
            break;
        case ES_ACT:
            e.vx = dir * CHARGE_SPEED * speed;
            if (moveEnemyBody(area, e, dt) || e.stateTimer >= CHARGE_TIME) e.state = ES_STUNNED, e.stateTimer = 0, e.vx = 0;
            break;
        case ES_STUNNED:
            e.vx = 0;
            moveEnemyBody(area, e, dt);
            if (e.stateTimer >= CHARGER_STUN) e.state = ES_PATROL, e.stateTimer = 0;
            break;
        default:
            e.vx = dir * CHARGER_PATROL * speed;
            if (moveEnemyBody(area, e, dt)) e.facingLeft = !e.facingLeft;
            if (heroInFront && fabsf(dx) <= CHARGER_SEE_X && fabsf(dy) <= CHARGER_SEE_Y && e.onGround) {
                e.state = ES_TELL;
                e.stateTimer = 0;
                q.soundRequested[type.attackSound] = true;
            }
            break;
        }
        break;

    case BEH_FLYER:
        switch (e.state) {
        case ES_TELL:
            e.facingLeft = e.targetX < e.x + e.w / 2;
            if (e.stateTimer >= FLYER_TELL) {
                float tx = e.targetX - (e.x + e.w / 2), ty = e.targetY - (e.y + e.h / 2), length = fmaxf(1, sqrtf(tx * tx + ty * ty));
                e.vx = tx / length * SWOOP_SPEED * speed;
                e.vy = ty / length * SWOOP_SPEED * speed;
                e.state = ES_ACT;
                e.stateTimer = 0;
                q.soundRequested[type.attackSound] = true;
            }
            break;
        case ES_ACT:
            e.x += e.vx * dt;
            e.y += e.vy * dt;
            if (e.stateTimer >= SWOOP_TIME) e.state = ES_RETURN, e.stateTimer = 0;
            break;
        case ES_RETURN: {
            float tx = e.homeX - e.x, ty = e.homeY - e.y, length = sqrtf(tx * tx + ty * ty), stepLength = FLYER_RETURN * speed * dt;
            if (length <= stepLength) {
                e.x = e.homeX;
                e.y = e.homeY;
                e.state = ES_PATROL;
                e.stateTimer = 0;
            } else {
                e.x += tx / length * stepLength;
                e.y += ty / length * stepLength;
                e.facingLeft = tx < 0;
            }
            break;
        }
        default:
            e.x += dir * FLYER_SPEED * speed * dt;
            if ((e.x > e.homeX + FLYER_DRIFT && !e.facingLeft) || (e.x < e.homeX - FLYER_DRIFT && e.facingLeft)) e.facingLeft = !e.facingLeft;
            e.y = e.homeY + FLYER_BOB * sinf(FLYER_BOB_RATE * q.time + e.spawn);
            if (distance < FLYER_SEE) {
                e.state = ES_TELL;
                e.stateTimer = 0;
                e.targetX = h.x + HERO_W / 2;
                e.targetY = h.y + HERO_H / 2;
            }
            break;
        }
        e.x = fmaxf(0, fminf(e.x, area.def->cols * TILE - e.w));
        e.y = fmaxf(0, fminf(e.y, area.def->rows * TILE - e.h));
        break;

    case BEH_SHOOTER:
        e.facingLeft = dx < 0;
        e.vx = 0;
        moveEnemyBody(area, e, dt);
        if (e.state == ES_TELL && e.stateTimer >= SHOOTER_GLOW) {
            enemyShoot(q, e, e.world == 3 ? PROJ_SNOWBALL : PROJ_ENEMY);   // the frost skull spits snowballs
            q.soundRequested[type.attackSound] = true;
            e.state = ES_PATROL;
            e.stateTimer = 0;
        } else if (e.state != ES_TELL && distance < SHOOTER_SEE && e.stateTimer >= SHOOTER_PERIOD - SHOOTER_GLOW) {
            e.state = ES_TELL;
            e.stateTimer = 0;
        }
        break;

    case BEH_JUMPER:
        if (e.onGround) {
            e.vx = 0;
            bool near = distance < JUMPER_SEE;
            if (e.stateTimer >= (near ? JUMPER_PERIOD : JUMPER_IDLE_PERIOD)) {
                e.facingLeft = near ? dx < 0 : e.facingLeft;
                // Hops toward the hero only onto ground (about 4 tiles ahead), else in place.
                float landX = e.x + e.w / 2 + (e.facingLeft ? -1 : 1) * JUMPER_VX * 2 * JUMPER_VY / ENEMY_GRAVITY;
                int feetRow = (int)floorf((e.y + e.h + 1) / TILE);
                bool ground = false;
                for (int r = feetRow - 1; r <= feetRow + 2; r++) ground |= holdsFeet(tileAt(area, (int)floorf(landX / TILE), r));
                e.vx = near && ground ? (e.facingLeft ? -1 : 1) * JUMPER_VX * speed : 0;
                e.vy = -JUMPER_VY;
                e.onGround = false;
                e.stateTimer = 0;
                q.soundRequested[type.attackSound] = true;
            }
        }
        moveEnemyBody(area, e, dt);
        break;

    case BEH_ARMORED:
        switch (e.state) {
        case ES_TELL:
            e.vx = 0;
            moveEnemyBody(area, e, dt);
            if (e.stateTimer >= ARMORED_WINDUP) {
                e.state = ES_ACT;
                e.stateTimer = 0;
                q.soundRequested[type.attackSound] = true;
            }
            break;
        case ES_ACT:
            moveEnemyBody(area, e, dt);
            if (e.stateTimer >= ARMORED_SWING_TIME) e.state = ES_RECOVER, e.stateTimer = 0;
            break;
        case ES_RECOVER:
            moveEnemyBody(area, e, dt);
            if (e.stateTimer >= ARMORED_RECOVER) e.state = ES_PATROL, e.stateTimer = 0;
            break;
        default: {
            bool near = distance < ARMORED_SEE;
            if (near) e.facingLeft = dx < 0;
            e.vx = near ? (e.facingLeft ? -1 : 1) * ARMORED_SPEED * speed : 0;
            moveEnemyBody(area, e, dt);
            float gap = fabsf(dx) - (e.w + HERO_W) / 2;
            if (near && gap <= ARMORED_REACH && fabsf(dy) < e.h) {
                e.state = ES_TELL;
                e.stateTimer = 0;
                e.vx = 0;
            }
            break;
        }
        }
        break;

    case BEH_DROPPER:
        if (e.state == ES_HANG) {
            if (fabsf(dx) <= DROPPER_SEE_X && h.y > e.y + e.h) {
                e.state = ES_ACT;
                e.stateTimer = 0;
                q.soundRequested[type.attackSound] = true;
            }
        } else if (e.state == ES_ACT) {   // drops straight down, then walks
            e.vx = 0;
            e.vy = DROP_SPEED;
            moveEnemyBody(area, e, dt);
            if (e.onGround) {
                e.state = ES_PATROL;
                e.facingLeft = dx < 0;
                if (type.trait == TRAIT_BANANA_ON_LAND) enemyShoot(q, e, PROJ_BANANA);
            }
        } else {
            e.vx = (e.facingLeft ? -1 : 1) * DROPPER_WALK * speed;
            if (moveEnemyBody(area, e, dt)) e.facingLeft = !e.facingLeft;
        }
        break;
    }
}

// The hero takes a hit (spec §7): i-frames, knockback away from sourceX (up at bounceVy), hit-stop, trauma, a red number.
void hurtHero(Quest& q, int damage, float sourceX, float bounceVy) {
    Hero& h = q.hero;
    if (h.iframes > 0 || q.godMode || h.hp <= 0) return;
    if (h.buffTimers[1] > 0) damage = (int)fmaxf(1, lroundf(damage * TONIC_DAMAGE));   // Iron Tonic
    h.hp = h.hp > damage ? h.hp - damage : 0;
    float away = sourceX > h.x + HERO_W / 2 ? -1 : 1;
    h.vx = away * HURT_VX;
    h.vy = -bounceVy;
    h.onGround = h.climbing = false;
    h.hurtTimer = HURT_TIME;
    h.iframes = HURT_IFRAMES;
    h.rollTimer = 0;
    h.skill = -1;
    q.soundRequested[SOUND_Q_HURT] = true;
    q.hitStop = fmaxf(q.hitStop, HURT_STOP);
    q.trauma = fminf(1, q.trauma + HURT_TRAUMA);
    addNumber(q, h.x + HERO_W / 2, h.y, damage, 'r');
}

// The most used colour of a sprite, outline aside: the colour of its kill particles.
static uint32_t mainColor(const Sprite& s) {
    int counts[128] = {};
    char best = 's';
    for (const char* p = s.pixels; *p; p++)
        if (*p != '.' && *p != 'x' && *p != 'k' && (unsigned char)*p < 128 && ++counts[(int)*p] > counts[(int)best]) best = *p;
    return paletteColor(best);
}

static void addEffect(Quest& q, int kind, float x, float y, float w, float h, float life, char color, float x2 = 0, float y2 = 0) {
    q.effects.push_back({kind, x, y, w, h, x2, y2, 0, life, q.hero.facingLeft, color});
}

// A hit spark at the contact point: the spark shape and 4 particles flying out, in the hit's colour (phase 5b).
static void spark(Quest& q, float x, float y, char color) {
    addEffect(q, FX_SPARK, x, y, 0, 0, SPARK_TIME, color);
    for (int i = 0; i < 4 && q.particles.size() < PARTICLE_CAP; i++) {
        float angle = random01(q) * 6.2832f, speed = (40 + 40 * random01(q)) * DP;
        q.particles.push_back({x, y, cosf(angle) * speed, sinf(angle) * speed, 0.15f, paletteColor(color), 1});
    }
}

static void burst(Quest& q, float x, float y, uint32_t color, int count) {
    for (int i = 0; i < count && q.particles.size() < PARTICLE_CAP; i++) {
        float angle = random01(q) * 6.2832f, speed = (30 + 50 * random01(q)) * DP;
        q.particles.push_back({x, y, cosf(angle) * speed, sinf(angle) * speed, 0.4f + 0.3f * random01(q), color, 1});
    }
}

// One hit of the hero's on enemy `index` (spec §7): damage, number, flash, knockback, hit-stop; a kill drops its names.
static void hitEnemy(Quest& q, int index, float mult, bool strong, float sourceX) {
    Enemy& e = q.enemies[index];
    const EnemyType& type = rosterType(e.world, e.type);
    const Hero& h = q.hero;
    if (!e.alive || e.state == ES_EMBER) return;
    if (type.trait == TRAIT_INFLATE && e.state == ES_ACT) return;   // inflated puffer: invulnerable
    float ex = e.x + e.w / 2, mod = 1;
    bool fromFront = (sourceX < ex) == e.facingLeft;
    if (type.behaviour == BEH_ARMORED && fromFront && e.state != ES_TELL && e.state != ES_RECOVER && h.y + HERO_H >= e.y + GUARD_OVER)
        mod = GUARD_MOD;
    if (type.behaviour == BEH_CHARGER && e.state == ES_STUNNED) mod = STUNNED_MOD;
    bool crit = random01(q) < q.stats.crit;
    float atk = q.stats.atk * (h.buffTimers[0] > 0 ? WHETSTONE_ATK : 1);   // Whetstone
    int damage = damageToEnemy(atk, mult, 0.9f + 0.2f * random01(q), crit, q.stats.critMult, mod);
    e.hp -= damage;
    e.flash = FLASH_TIME;
    spark(q, fmaxf(e.x, fminf(sourceX, e.x + e.w)), e.y + e.h / 2, strong ? 'N' : 'w');
    addNumber(q, ex, e.y, damage, crit ? 'o' : 'w');
    q.soundRequested[crit ? SOUND_Q_CRIT : SOUND_Q_HIT] = true;
    if (type.kind == KIND_FODDER || type.kind == KIND_SOLDIER) {
        e.knockTimer = KNOCK_TIME;
        e.vx = (sourceX < ex ? 1 : -1) * KNOCK_SPEED;
    }
    if (strong) q.hitStop = fmaxf(q.hitStop, HIT_STOP);
    if (e.hp > 0) return;
    q.hitStop = fmaxf(q.hitStop, HIT_STOP);
    if (type.trait == TRAIT_REVIVE && !e.revived) {
        e.state = ES_EMBER;
        e.stateTimer = 0;
        e.hp = 0;
        return;
    }
    e.alive = false;
    burst(q, ex, e.y + e.h / 2, mainColor(*type.frames[0]), KILL_PARTICLES);
    int names = e.names + (e.stolenTimer > 0 ? e.stolen : 0);
    q.save.names = q.save.names > MAX_NAMES - names ? MAX_NAMES : q.save.names + names;
    q.save.kills++;
    std::vector<uint64_t>& killed = q.killed[q.area.world];
    if (e.spawn < 64 && q.area.index < (int)killed.size()) killed[q.area.index] |= 1ull << e.spawn;
}

// One hit of the hero's on boss `index` (spec §7): no knockback; updateBosses handles phases and the death.
static void hitBoss(Quest& q, int index, float mult, bool strong) {
    QuestBoss& b = q.bosses[index];
    if (!bossHittable(b)) return;
    bool crit = random01(q) < q.stats.crit;
    float atk = q.stats.atk * (q.hero.buffTimers[0] > 0 ? WHETSTONE_ATK : 1);
    int damage = damageToEnemy(atk, mult, 0.9f + 0.2f * random01(q), crit, q.stats.critMult, 1);
    b.hp -= damage;
    b.flash = BOSS_FLASH;
    float heroX = q.hero.x + HERO_W / 2, heroY = q.hero.y + HERO_H / 2;   // the boss box's edge nearest the hero
    spark(q, fmaxf(b.x, fminf(heroX, b.x + b.w)), fmaxf(b.y, fminf(heroY, b.y + b.h)), strong ? 'N' : 'w');
    addNumber(q, b.x + b.w / 2, b.y, damage, crit ? 'o' : 'w');
    q.soundRequested[crit ? SOUND_Q_CRIT : SOUND_Q_HIT] = true;
    if (strong) q.hitStop = fmaxf(q.hitStop, HIT_STOP);
}

// Any hero attack box touching the area's lever K opens its shortcut doors (spec §11).
static void touchLever(Quest& q, float x, float y, float w, float h) {
    const AreaDef& def = *q.area.def;
    if (def.shortcut < 0 || (q.save.shortcuts[q.area.world] >> def.shortcut & 1)) return;
    for (int t = 0; t < def.cols * def.rows; t++) {
        if (def.tiles[t] != 'K') continue;
        float lx = (t % def.cols) * TILE + TILE / 2 - LEVER_W / 2, ly = (t / def.cols + 1) * TILE - LEVER_H;
        if (overlaps(x, y, w, h, lx, ly, LEVER_W, LEVER_H)) openShortcut(q);
    }
}

// Hits every living enemy overlapping the box once per swing id (the partner's swings are tracked apart).
static int hitBox(Quest& q, float x, float y, float w, float h, float mult, bool strong, int swing, bool partner, float stun = 0) {
    int hits = 0;
    touchLever(q, x, y, w, h);
    for (int i = 0; i < (int)q.enemies.size(); i++) {
        Enemy& e = q.enemies[i];
        int& last = partner ? e.lastPartnerSwing : e.lastSwing;
        if (!e.alive || e.state == ES_EMBER || last == swing || !overlaps(x, y, w, h, e.x, e.y, e.w, e.h)) continue;
        last = swing;
        hitEnemy(q, i, mult, strong, x + w / 2);
        if (stun > 0 && e.alive) e.stunTimer = stun;
        hits++;
    }
    for (int i = 0; i < (int)q.bosses.size(); i++) {   // bosses take the hit but never the stun
        QuestBoss& b = q.bosses[i];
        int& last = partner ? b.lastPartnerSwing : b.lastSwing;
        if (!bossHittable(b) || last == swing || !overlaps(x, y, w, h, b.x, b.y, (float)b.w, (float)b.h)) continue;
        last = swing;
        hitBoss(q, i, mult, strong);
        hits++;
    }
    return hits;
}

// Hits every living enemy whose hitbox touches the circle, once (each call is one hit).
static void hitCircle(Quest& q, float cx, float cy, float radius, float mult) {
    touchLever(q, cx - radius, cy - radius, 2 * radius, 2 * radius);
    for (int i = 0; i < (int)q.enemies.size(); i++) {
        Enemy& e = q.enemies[i];
        float nx = fmaxf(e.x, fminf(cx, e.x + e.w)), ny = fmaxf(e.y, fminf(cy, e.y + e.h));
        if (e.alive && e.state != ES_EMBER && (nx - cx) * (nx - cx) + (ny - cy) * (ny - cy) <= radius * radius) hitEnemy(q, i, mult, true, cx);
    }
    for (int i = 0; i < (int)q.bosses.size(); i++) {
        const QuestBoss& b = q.bosses[i];
        float nx = fmaxf(b.x, fminf(cx, b.x + b.w)), ny = fmaxf(b.y, fminf(cy, b.y + b.h));
        if ((nx - cx) * (nx - cx) + (ny - cy) * (ny - cy) <= radius * radius) hitBoss(q, i, mult, true);
    }
}

static void shoot(Quest& q, float cx, float cy, float angle, float speed, float range, float w, float h, int kind, float mult,
                  bool strong, int pierce) {
    Projectile p = {};
    p.vx = cosf(angle) * speed;
    p.vy = sinf(angle) * speed;
    p.w = (int)w;
    p.h = (int)h;
    p.x = cx - w / 2;
    p.y = cy - h / 2;
    p.fromHero = true;
    p.pierce = pierce;
    p.range = range;
    p.kind = kind;
    p.mult = mult;
    p.strong = strong;
    q.projectiles.push_back(p);
}

// The box `w` x `boxH` in front of the hero, centred on its middle (spec §6 boxes).
static void frontBox(const Hero& h, float w, float boxH, float* x, float* y) {
    *x = h.facingLeft ? h.x - w : h.x + HERO_W;
    *y = h.y + HERO_H / 2 - boxH / 2;
}

// Starts the job tier's strong attack (spec §6 table).
static void castSkill(Quest& q, int cls, int job) {
    Hero& h = q.hero;
    if (h.mp < SKILL_MP[cls][job] || (cls == CLASS_THIEF && job == 3 && h.partnerTimer > 0)) {
        q.soundRequested[SOUND_ROCKET_EMPTY] = true;
        return;
    }
    h.mp -= SKILL_MP[cls][job];
    logLine(q, "quest: skill %s", SKILL_NAMES[cls][job]);
    h.attackTimer = 0;   // the skill replaces a basic attack in progress (they would share the swing id)
    h.skill = job;
    h.strongTimer = SKILL_TIME[cls][job];
    h.swing++;
    h.volleys = 0;
    float dir = h.facingLeft ? -1 : 1, cx = h.x + HERO_W / 2, handY = h.y + 6 * DP, handX = cx + dir * 6 * DP;
    float aim = h.facingLeft ? 3.14159265f : 0;
    int pierce = cls == CLASS_ARCHER && job >= 1 ? 1 : 0;
    const float DEGREE = 3.14159265f / 180;
    switch (cls * 4 + job) {
    case CLASS_KNIGHT * 4 + 0:   // Power Slash: box in updateSkill, drawn from the hero's pose
        q.soundRequested[SOUND_Q_HEAVY] = true;
        break;
    case CLASS_KNIGHT * 4 + 1:   // Rising Slash: hop
        h.vy = -120 * DP;
        h.onGround = false;
        q.soundRequested[SOUND_Q_HEAVY] = true;
        break;
    case CLASS_KNIGHT * 4 + 2: {   // Ground Quake: a shockwave each way along the floor
        int col = (int)floorf(cx / TILE), row = (int)floorf((h.y + HERO_H) / TILE);
        for (int r = row; r < row + 6; r++) {
            if (!holdsFeet(tileAt(q.area, col, r))) continue;
            for (int side = -1; side <= 1; side += 2) {
                shoot(q, cx, r * TILE - 5 * DP, side < 0 ? 3.14159265f : 0, 200 * DP, 80 * DP, 8 * DP, 10 * DP, PROJ_WAVE, 3.4f, true, 99);
            }
            break;
        }
        burst(q, cx + dir * 10 * DP, h.y + HERO_H, paletteColor('H'), 5);   // dust at the sword point
        burst(q, cx + dir * 10 * DP, h.y + HERO_H, paletteColor('s'), 5);
        addEffect(q, FX_BURST, cx + dir * 10 * DP, h.y + HERO_H, 10 * DP, 10 * DP, 0.15f, 's');
        q.trauma = fminf(1, q.trauma + 0.15f);
        q.soundRequested[SOUND_POUND] = true;
        break;
    }
    case CLASS_KNIGHT * 4 + 3:   // Lion's Roar: box in updateSkill, drawn from the hero's pose
        q.trauma = fminf(1, q.trauma + 0.2f);
        q.soundRequested[SOUND_Q_ROAR] = true;
        break;
    case CLASS_ARCHER * 4 + 0:   // Double Shot
        for (int k = -1; k <= 1; k += 2) shoot(q, handX, handY + k * 2 * DP, aim, 240 * DP, 170 * DP, 6 * DP, 2 * DP, PROJ_ARROW, 1.5f, true, pierce);
        q.soundRequested[SOUND_Q_ARROW] = true;
        break;
    case CLASS_ARCHER * 4 + 1: {   // Arrow Rain: 6 arrows over a 56 px zone 80 px ahead, from 120 px above
        float zoneX = cx + dir * 80 * DP, floorY = h.y + HERO_H;   // the zone marked on the floor below its centre, like Meteor
        int col = (int)floorf(zoneX / TILE);
        for (int r = (int)floorf((h.y + HERO_H / 2) / TILE); r < (int)floorf((h.y + HERO_H) / TILE) + 8; r++)
            if (holdsFeet(tileAt(q.area, col, r)) && !tileSolid(tileAt(q.area, col, r - 1))) { floorY = (float)(r * TILE); break; }
        addEffect(q, FX_ZONE, zoneX, floorY, 56 * DP, 0, 0.5f, 'w');
        for (int k = 0; k < 6; k++)
            shoot(q, cx + dir * 80 * DP - 28 * DP + k * 56 * DP / 5, h.y - 120 * DP, 3.14159265f / 2, 240 * DP, 300 * DP, 2 * DP, 6 * DP,
                  PROJ_RAIN, 1.6f, true, pierce);
        q.soundRequested[SOUND_Q_ARROW] = true;
        break;
    }
    case CLASS_ARCHER * 4 + 2:   // Piercing Bolt
        shoot(q, handX, handY, aim, 360 * DP, 300 * DP, 10 * DP, 2 * DP, PROJ_ARROW, 4.0f, true, 99);
        q.soundRequested[SOUND_Q_ARROW] = true;
        break;
    case CLASS_ARCHER * 4 + 3: h.volleys = 3; break;   // Storm Volley: fans in updateSkill
    case CLASS_MAGICIAN * 4 + 0:   // Arcane Burst
        hitCircle(q, cx + dir * 40 * DP, h.y + HERO_H / 2, 20 * DP, 2.6f);
        addEffect(q, FX_BURST, cx + dir * 40 * DP, h.y + HERO_H / 2, 20 * DP, 20 * DP, 0.25f, 'p');
        q.soundRequested[SOUND_Q_SPELL] = true;
        break;
    case CLASS_MAGICIAN * 4 + 1:   // Frost Orb
        shoot(q, handX, handY, aim, 90 * DP, 200 * DP, 8 * DP, 8 * DP, PROJ_ORB, 2.8f, true, 0);
        q.soundRequested[SOUND_FREEZE] = true;
        break;
    case CLASS_MAGICIAN * 4 + 2: {   // Chain Lightning: the nearest within 140, then 3 more each within 64 of the last
        // Candidates: enemies 0-59, then bosses as 60 + their index.
        float fromX = handX, fromY = handY, reach = 140 * DP;
        uint64_t used = 0;
        for (int link = 0; link < 4; link++) {
            int best = -1;
            float bestDistance = reach, toX = 0, toY = 0;
            for (int i = 0; i < 64; i++) {
                bool boss = i >= 60;
                int k = boss ? i - 60 : i;
                if (boss ? k >= (int)q.bosses.size() || !bossHittable(q.bosses[k])
                         : k >= (int)q.enemies.size() || !q.enemies[k].alive || q.enemies[k].state == ES_EMBER)
                    continue;
                float x = boss ? q.bosses[k].x + q.bosses[k].w / 2 : q.enemies[k].x + q.enemies[k].w / 2;
                float y = boss ? q.bosses[k].y + q.bosses[k].h / 2 : q.enemies[k].y + q.enemies[k].h / 2;
                float d = hypotf(x - fromX, y - fromY);
                if (!(used >> i & 1) && d <= bestDistance) best = i, bestDistance = d, toX = x, toY = y;
            }
            if (best < 0) break;
            used |= 1ull << best;
            addEffect(q, FX_CHAIN, fromX, fromY, 0, 0, 0.2f, 'c', toX, toY);
            if (best >= 60) hitBoss(q, best - 60, 3.0f, true);
            else hitEnemy(q, best, 3.0f, true, fromX);
            fromX = toX;
            fromY = toY;
            reach = 64 * DP;
        }
        q.soundRequested[SOUND_Q_ZAP] = true;
        break;
    }
    case CLASS_MAGICIAN * 4 + 3: {   // Meteor: lands 0.6 s later 120 px ahead on the floor below
        h.skillX = cx + dir * 120 * DP;
        h.skillY = h.y + HERO_H;
        int col = (int)floorf(h.skillX / TILE);
        for (int r = (int)floorf((h.y + HERO_H / 2) / TILE); r < (int)floorf((h.y + HERO_H) / TILE) + 8; r++)
            if (holdsFeet(tileAt(q.area, col, r)) && !tileSolid(tileAt(q.area, col, r - 1))) { h.skillY = (float)(r * TILE); break; }
        h.volleys = 1;
        addEffect(q, FX_METEOR, h.skillX, h.skillY, 48 * DP, 48 * DP, METEOR_TIME, 'o');
        q.soundRequested[SOUND_Q_CHARGE] = true;
        break;
    }
    case CLASS_THIEF * 4 + 0:   // Star Toss
        for (int k = -1; k <= 1; k++) shoot(q, handX, handY, aim + dir * k * 10 * DEGREE, 260 * DP, 150 * DP, 4 * DP, 4 * DP, PROJ_STAR, 1.2f, true, 0);
        q.soundRequested[SOUND_Q_THROW] = true;
        break;
    case CLASS_THIEF * 4 + 1:   // Shadow Dash
        h.dashDir = dir;
        h.iframes = fmaxf(h.iframes, SKILL_TIME[cls][job]);
        q.soundRequested[SOUND_Q_DODGE] = true;
        break;
    case CLASS_THIEF * 4 + 2: h.volleys = 5; q.soundRequested[SOUND_Q_SWING] = true; break;   // Blade Fury: 5 hits in updateSkill
    case CLASS_THIEF * 4 + 3:   // Shadow Partner: smoke where the copy appears
        h.partnerTimer = PARTNER_TIME;
        burst(q, cx - dir * PARTNER_BEHIND, h.y + HERO_H / 2, paletteColor('x'), 6);
        burst(q, cx - dir * PARTNER_BEHIND, h.y + HERO_H / 2, paletteColor('p'), 6);
        q.soundRequested[SOUND_Q_SPELL] = true;
        break;
    }
}

// The playing strong attack's timed parts: boxes in their windows, Storm Volley's fans, Blade Fury's ticks, Meteor's impact.
static void updateSkill(Quest& q, int cls, int job) {
    Hero& h = q.hero;
    float elapsed = SKILL_TIME[cls][job] - h.strongTimer, dir = h.facingLeft ? -1 : 1, cx = h.x + HERO_W / 2, cy = h.y + HERO_H / 2;
    float x, y;
    switch (cls * 4 + job) {
    case CLASS_KNIGHT * 4 + 0:   // Power Slash: box 30 x 20, x2.4
        if (elapsed >= 0.1f && elapsed <= 0.25f) {
            frontBox(h, 30 * DP, 20 * DP, &x, &y);
            hitBox(q, x, y, 30 * DP, 20 * DP, 2.4f, true, h.swing, false);
        }
        break;
    case CLASS_KNIGHT * 4 + 1:   // Rising Slash: box 24 x 36 above-front, x3.0
        if (elapsed <= 0.15f) hitBox(q, h.facingLeft ? h.x - 24 * DP : h.x + HERO_W, h.y + HERO_H - 36 * DP, 24 * DP, 36 * DP, 3.0f, true, h.swing, false);
        break;
    case CLASS_KNIGHT * 4 + 3:   // Lion's Roar: arc 72 x 40, x5.0, stuns 0.5 s
        if (elapsed <= 0.15f) {
            frontBox(h, 72 * DP, 40 * DP, &x, &y);
            hitBox(q, x, y, 72 * DP, 40 * DP, 5.0f, true, h.swing, false, 0.5f);
        }
        break;
    case CLASS_ARCHER * 4 + 3:   // Storm Volley: a 5-arrow fan at 0, 0.225 and 0.45 s
        if (h.volleys > 0 && elapsed >= (3 - h.volleys) * 0.225f - 0.001f) {
            float aim = h.facingLeft ? 3.14159265f : 0;
            for (int k = -2; k <= 2; k++)
                shoot(q, cx + dir * 6 * DP, h.y + 6 * DP, aim + k * 10 * 3.14159265f / 180, 240 * DP, 170 * DP, 6 * DP, 2 * DP, PROJ_ARROW, 1.8f, true, 1);
            burst(q, cx + dir * 6 * DP, h.y + 6 * DP, paletteColor('c'), 4);
            q.soundRequested[SOUND_Q_ARROW] = true;
            h.volleys--;
        }
        break;
    case CLASS_MAGICIAN * 4 + 3:   // Meteor lands METEOR_TIME after the press
        if (h.volleys > 0 && elapsed < METEOR_TIME - 0.001f) {
            // 2 sparks per step spiral in toward the staff tip (art column 24, row 1 of the hero sprite), and the rock's trail.
            float tipX = h.x - 12 * DP + (h.facingLeft ? 7.5f : 24.5f) * DP, tipY = h.y + HERO_H - 27 * DP;
            for (int k = 0; k < 2 && q.particles.size() < PARTICLE_CAP; k++) {
                float a = random01(q) * 6.2832f, sx = tipX + cosf(a) * 16 * DP, sy = tipY + sinf(a) * 16 * DP;
                float vx = (tipX - sx) / 0.2f - sinf(a) * 40 * DP, vy = (tipY - sy) / 0.2f + cosf(a) * 40 * DP;
                q.particles.push_back({sx, sy, vx, vy, 0.2f, paletteColor(k ? 'N' : 'o'), 1});
            }
            float fall = 1 - elapsed / METEOR_TIME, rockX = h.skillX - dir * 30 * DP * fall, rockY = h.skillY - 120 * DP * fall;
            for (int k = 0; k < 2 && q.particles.size() < PARTICLE_CAP; k++)
                q.particles.push_back({rockX + (random01(q) - 0.5f) * 8 * DP, rockY - 8 * DP, 0, -20 * DP, 0.25f, paletteColor('O'), 1});
        }
        if (h.volleys > 0 && elapsed >= METEOR_TIME - 0.001f) {
            hitCircle(q, h.skillX, h.skillY, 48 * DP, 6.0f);
            addEffect(q, FX_BURST, h.skillX, h.skillY, 48 * DP, 48 * DP, 0.3f, 'o');
            q.trauma = fminf(1, q.trauma + 0.25f);
            burst(q, h.skillX, h.skillY - 4 * DP, paletteColor('V'), 8);   // debris
            burst(q, h.skillX, h.skillY - 4 * DP, paletteColor('O'), 8);
            q.soundRequested[SOUND_EXPLOSION] = true;
            h.volleys = 0;
        }
        break;
    case CLASS_THIEF * 4 + 1: {   // Shadow Dash: x2.6 to every enemy passed, a slash line through each; afterimages
        size_t before = q.effects.size();
        hitBox(q, h.x, h.y, HERO_W, HERO_H, 2.6f, true, h.swing, false);
        for (size_t k = before; k < q.effects.size(); k++)
            if (q.effects[k].kind == FX_SPARK) q.effects[k].w = 12 * DP;
        if ((int)(elapsed / 0.05f) != (int)((elapsed - 1.0f / 60) / 0.05f) || elapsed < 0.001f) addEffect(q, FX_GHOST, h.x, h.y, 0, 0, 0.2f, 'x');
        break;
    }
    case CLASS_THIEF * 4 + 2:   // Blade Fury: 5 hits of x1.0 in a radius of 30, one every 0.1 s
        if (h.volleys > 0 && elapsed >= (5 - h.volleys) * 0.1f - 0.001f) {
            hitCircle(q, cx, cy, 30 * DP, 1.0f);
            if (h.volleys == 5) addEffect(q, FX_SPIN, cx, cy, 30 * DP, 30 * DP, 0.5f, 'p');
            h.volleys--;
        }
        break;
    }
}

// The hero's drawn pose (spec §22, phase 5b): pure, from the hero's timers and the attack tables.
HeroPose heroPose(const Quest& q) {
    const Hero& h = q.hero;
    int cls = q.save.heroClass;
    const BasicAttack& basic = BASIC[cls];
    HeroPose p;
    p.attackT = h.attackTimer > 0 ? basic.cycle - h.attackTimer : -1;
    p.skillT = h.skill >= 0 ? SKILL_TIME[cls][h.skill] - h.strongTimer : -1;
    bool running = fabsf(h.vx) > 10 * DP;
    p.legs = !h.onGround ? LEGS_AIR : running ? LEGS_RUN_1 + (int)(q.time / RUN_FRAME) % 4 : LEGS_STAND;

    bool hurt = q.mode == QMODE_DEAD || h.hurtTimer > 0;
    if (hurt) p.body = BODY_HURT;
    else if (h.rollTimer > 0) p.body = (int)(q.time / 0.1f) % 2 ? BODY_ROLL_2 : BODY_ROLL_1;
    else if (h.climbing) p.body = h.vy != 0 && (int)(q.time / 0.2f) % 2 ? BODY_CLIMB_2 : BODY_CLIMB_1;
    else if (!h.onGround) p.body = h.vy < -60 * DP ? BODY_JUMP : h.vy > 40 * DP ? BODY_FALL : BODY_PEAK;
    else if (h.landTimer > 0) p.body = BODY_LAND;
    else if (running) p.body = BODY_RUN_1 + (int)(q.time / RUN_FRAME) % 4;
    else p.body = (int)(q.time / 0.6f) % 2 ? BODY_IDLE_2 : BODY_IDLE_1;

    bool melee = basic.speed == 0;
    float t = p.skillT;
    p.arms = ARMS_NONE;
    if (hurt || h.rollTimer > 0 || h.climbing) {
        p.arms = ARMS_NONE;
    } else if (h.skill >= 0) {
        switch (cls * 4 + h.skill) {
        case CLASS_KNIGHT * 4 + 0: p.arms = t < 0.1f ? ARMS_SKILL_1 : t <= 0.25f ? ARMS_SKILL_2 : ARMS_RECOVER; break;
        case CLASS_KNIGHT * 4 + 1: p.arms = t <= 0.15f ? ARMS_SKILL_3 : ARMS_RECOVER; break;
        case CLASS_KNIGHT * 4 + 2: p.arms = ARMS_SKILL_4; break;
        case CLASS_KNIGHT * 4 + 3: p.arms = t <= 0.15f ? ARMS_SKILL_5 : ARMS_RECOVER; break;
        case CLASS_ARCHER * 4 + 0:
        case CLASS_ARCHER * 4 + 2: p.arms = t < 0.12f ? ARMS_SKILL_2 : ARMS_RECOVER; break;
        case CLASS_ARCHER * 4 + 1: p.arms = t < 0.12f ? ARMS_SKILL_4 : ARMS_SKILL_3; break;
        case CLASS_ARCHER * 4 + 3: p.arms = fmodf(t, 0.225f) < 0.08f ? ARMS_SKILL_2 : ARMS_SKILL_5; break;
        case CLASS_MAGICIAN * 4 + 0: p.arms = t < 0.12f ? ARMS_SKILL_2 : ARMS_RECOVER; break;
        case CLASS_MAGICIAN * 4 + 1: p.arms = t < 0.12f ? ARMS_SKILL_5 : ARMS_RECOVER; break;
        case CLASS_MAGICIAN * 4 + 2: p.arms = ARMS_SKILL_4; break;
        case CLASS_MAGICIAN * 4 + 3: p.arms = t < 0.3f ? ARMS_SKILL_1 : ARMS_SKILL_3; break;
        case CLASS_THIEF * 4 + 0: p.arms = t < 0.1f ? ARMS_SKILL_1 : ARMS_RECOVER; break;
        case CLASS_THIEF * 4 + 1: p.arms = ARMS_SKILL_2; break;
        case CLASS_THIEF * 4 + 2: p.arms = (int)(t / 0.05f) % 2 == 0 ? ARMS_SKILL_3 : ARMS_SKILL_4; break;
        case CLASS_THIEF * 4 + 3: p.arms = ARMS_SKILL_5; break;
        }
    } else if (p.attackT >= 0) {
        // Each phase has three drawings. Melee contact tracks the hit window, and ranged release starts at the shot.
        float firstEnd = melee ? basic.from : 0.08f;
        float secondEnd = melee ? basic.to : basic.cycle - 0.20f;
        int phase = 0;
        float start = 0, end = firstEnd;
        if (p.attackT > secondEnd) {
            phase = 2;
            start = secondEnd;
            end = basic.cycle;
        } else if (p.attackT >= firstEnd) {
            phase = 1;
            start = firstEnd;
            end = secondEnd;
        }
        int frame = std::min(2, (int)(3 * (p.attackT - start) / (end - start)));
        p.arms = ARMS_BASIC + h.basicStep * BASIC_STRIKE_FRAMES + phase * 3 + frame;
        if (!melee && phase == 2 && h.queuedAttack != BASIC_NONE) {
            int nextStrike = (h.basicStep + 1) % BASIC_COMBO_LENGTH;
            p.arms = ARMS_QUEUED_DRAW + nextStrike * 3 + frame;
        }
    }

    p.slashLit = melee && p.attackT >= basic.from && p.attackT <= basic.to;
    if (cls == CLASS_KNIGHT && h.skill == 0) p.slashLit = t >= 0.1f && t <= 0.25f;
    if (cls == CLASS_KNIGHT && (h.skill == 1 || h.skill == 3)) p.slashLit = t >= 0 && t <= 0.15f;
    return p;
}

void updateEffects(Quest& q, float dt) {
    for (size_t i = 0; i < q.particles.size();) {
        Particle& p = q.particles[i];
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.life -= dt;
        if (p.life <= 0) q.particles.erase(q.particles.begin() + i);
        else i++;
    }
    for (size_t i = 0; i < q.numbers.size();) {
        q.numbers[i].age += dt;
        if (q.numbers[i].age >= q.numbers[i].life) q.numbers.erase(q.numbers.begin() + i);
        else i++;
    }
    for (size_t i = 0; i < q.effects.size();) {
        q.effects[i].age += dt;
        if (q.effects[i].age >= q.effects[i].life) q.effects.erase(q.effects.begin() + i);
        else i++;
    }
}

void updateCombat(Quest& q, const Actions& a, float dt) {
    Hero& h = q.hero;
    int cls = q.save.heroClass;
    if (a.firePressed && h.frozenTimer <= 0 && h.skill < 0)
        h.queuedAttack = cls == CLASS_THIEF && a.moveY < -0.5f ? BASIC_UP : BASIC_FORWARD;
    if (q.hitStop > 0) {   // hit-stop freezes the hero, enemies and shots (not particles and numbers)
        q.hitStop = fmaxf(0, q.hitStop - dt);
        return;
    }
    const HeroStats& s = q.stats;
    int job = q.save.job < 0 ? 0 : q.save.job > 3 ? 3 : q.save.job;
    const BasicAttack& basic = BASIC[cls];
    float walk = CLASSES[cls].walk * DP;
    h.landTimer = fmaxf(0, h.landTimer - dt);

    // Timers and MP regen (HP never regenerates).
    bool previousBasic = h.attackTimer > 0;
    h.attackTimer = fmaxf(0, h.attackTimer - dt);
    h.strongTimer = fmaxf(0, h.strongTimer - dt);
    h.rollTimer = fmaxf(0, h.rollTimer - dt);
    h.rollCooldown = fmaxf(0, h.rollCooldown - dt);
    h.iframes = fmaxf(0, h.iframes - dt);
    h.hurtTimer = fmaxf(0, h.hurtTimer - dt);
    h.partnerTimer = fmaxf(0, h.partnerTimer - dt);
    h.slideTimer = fmaxf(0, h.slideTimer - dt);
    if (h.frozenTimer > 0) {   // frozen in the Yeti's ice block: X or R1 presses chip it away
        h.frozenTimer -= dt + (a.jump || a.firePressed ? FROZEN_PRESS_CUT : 0);
        if (h.frozenTimer <= 0) {
            h.frozenTimer = 0;
            q.soundRequested[SOUND_CRACK] = true;
        }
    }
    q.area.windFlipped = fmodf(q.time, 2 * WIND_FLIP) >= WIND_FLIP;
    for (float& t : h.buffTimers) t = fmaxf(0, t - dt);
    h.mpFraction += s.mpRegen * dt;
    while (h.mpFraction >= 1) {
        h.mpFraction -= 1;
        if (h.mp < s.maxMp) h.mp++;
    }

    // Dodge (Circle): the Magician blinks (also in the air), the others roll on the ground.
    bool free = h.hurtTimer <= 0 && !h.climbing && h.rollTimer <= 0 && h.slideTimer <= 0 && h.frozenTimer <= 0;
    if (a.back && free && h.rollCooldown <= 0 && h.skill < 0) {
        if (cls == CLASS_MAGICIAN) {
            float dir = h.facingLeft ? -1 : 1;
            for (float moved = 0; moved < BLINK_DISTANCE; moved += DP) {
                if (boxInSolid(q.area, h.x + dir * DP, h.y, HERO_W, HERO_H) || h.x + dir * DP < 0 ||
                    h.x + dir * DP > q.area.def->cols * TILE - HERO_W)
                    break;
                h.x += dir * DP;
            }
            h.iframes = fmaxf(h.iframes, BLINK_IFRAMES);
            h.rollCooldown = BLINK_COOLDOWN;
            q.soundRequested[SOUND_Q_DODGE] = true;
        } else if (h.onGround) {
            h.rollTimer = ROLL_TIME;
            h.iframes = fmaxf(h.iframes, ROLL_IFRAMES);
            h.rollCooldown = ROLL_COOLDOWN;
            q.soundRequested[SOUND_Q_DODGE] = true;
        }
    }
    free = h.hurtTimer <= 0 && !h.climbing && h.rollTimer <= 0 && h.slideTimer <= 0 && h.frozenTimer <= 0;

    // R2 takes priority. One R1 press can queue the next basic attack until the current cycle ends.
    if (a.rocket && free && h.skill < 0) castSkill(q, cls, job);
    if (!free || h.skill >= 0) h.queuedAttack = BASIC_NONE;
    if (h.queuedAttack != BASIC_NONE && h.attackTimer <= 0) {
        h.basicStep = previousBasic ? (h.basicStep + 1) % BASIC_COMBO_LENGTH : 0;
        h.attackTimer = basic.cycle;
        h.attackUp = h.queuedAttack == BASIC_UP;
        h.queuedAttack = BASIC_NONE;
        h.swing++;
        q.soundRequested[basic.sound] = true;
        float dir = h.facingLeft ? -1 : 1;
        if (basic.speed > 0)
            shoot(q, h.x + HERO_W / 2 + dir * 6 * DP, h.y + 6 * DP, h.facingLeft ? 3.14159265f : 0, basic.speed, basic.range, basic.shotW,
                  basic.shotH, cls == CLASS_ARCHER ? PROJ_ARROW : PROJ_BOLT, basic.mult * BASIC_MULT_BY_JOB[job], false,
                  cls == CLASS_ARCHER && job >= 1 ? 1 : 0);
    }

    // Movement: knockback, roll and dash keep their speed; R1 and R2 never slow or root the hero (D-062, D-064).
    Actions move = a;
    float dir = h.facingLeft ? -1 : 1;
    if (h.frozenTimer > 0) {   // in the ice block: no control
        h.vx = 0;
        move.moveX = move.moveY = 0;
        move.jump = false;
    } else if (h.slideTimer > 0) {   // on a peel: sliding the way it faced, no control
        h.vx = h.slideSpeed;
        move.moveX = h.slideSpeed / walk;
        move.moveY = 0;
        move.jump = false;
    } else if (h.hurtTimer > 0) {
        move.moveX = h.vx / walk;
        move.jump = false;
        move.jumpHeld = true;
    } else if (h.rollTimer > 0) {
        h.vx = dir * ROLL_SPEED;
        move.moveX = dir * ROLL_SPEED / walk;
        move.moveY = 0;
        move.jump = false;
    } else if (h.skill == 1 && cls == CLASS_THIEF && h.strongTimer > 0) {
        h.vx = h.dashDir * SHADOW_DASH_SPEED;
        h.vy = 0;
        move.moveX = h.dashDir * SHADOW_DASH_SPEED / walk;
        move.moveY = 0;
        move.jump = false;
    } else {
        if (h.buffTimers[2] > 0) move.moveX *= FEATHER_WALK;
        if (h.drinkTimer > 0) move.moveX *= DRINK_WALK;
    }
    if (h.skill >= 0) move.jumpHeld = true;   // the Rising Slash hop isn't cut
    moveHero(h, move, q.area, (HeroClass)cls, dt);

    // The melee basic attack's window (the Shadow Partner repeats it 10 px behind at 50 %).
    if (h.attackTimer > 0 && basic.speed == 0) {
        float elapsed = basic.cycle - h.attackTimer, x, y;
        if (elapsed >= basic.from && elapsed <= basic.to) {
            frontBox(h, basic.boxW, basic.boxH, &x, &y);
            float w = basic.boxW, bh = basic.boxH;
            if (h.attackUp) {   // the box turned upright, centred above the head
                w = basic.boxH;
                bh = basic.boxW;
                x = h.x + HERO_W / 2 - w / 2;
                y = h.y - bh;
            }
            int hits = hitBox(q, x, y, w, bh, basic.mult * BASIC_MULT_BY_JOB[job], false, h.swing, false);
            if (hits > 0) q.hitStop = fmaxf(q.hitStop, BASIC_HIT_STOP);
            h.mp = h.mp + hits < s.maxMp ? h.mp + hits : s.maxMp;   // every landed basic hit gives +1 MP
            if (h.partnerTimer > 0)
                hitBox(q, x - dir * PARTNER_BEHIND, y, w, bh, basic.mult * BASIC_MULT_BY_JOB[job] * PARTNER_MULT, false, h.swing, true);
        }
    }
    if (h.skill >= 0) {
        updateSkill(q, cls, h.skill);
        if (h.strongTimer <= 0) h.skill = -1;
    }

    // Enemies.
    for (Enemy& e : q.enemies)
        if (e.alive) updateEnemy(q, e, dt);

    // Shots: move, stop at tiles or range, hit.
    for (size_t i = 0; i < q.projectiles.size();) {
        Projectile& p = q.projectiles[i];
        p.vy += p.gravity * dt;
        float mx = p.vx * dt, my = p.vy * dt;
        p.x += mx;
        p.y += my;
        p.range -= sqrtf(mx * mx + my * my);
        bool gone = p.range <= 0;
        if (p.fromHero && p.kind == PROJ_WAVE && (int)(q.time * 60) % 3 == 0 && q.particles.size() < PARTICLE_CAP)   // quake dust
            q.particles.push_back({p.x + p.w / 2, p.y, -p.vx * 0.1f, -30 * DP, 0.3f, paletteColor('H'), 1});
        if (p.kind == PROJ_WAVE) {   // follows the floor; ends at a wall or a ledge
            float lead = p.vx > 0 ? p.x + p.w - 0.001f : p.x;
            int col = (int)floorf(lead / TILE), feetRow = (int)floorf((p.y + p.h + 1) / TILE);
            gone |= boxInSolid(q.area, p.x, p.y, (float)p.w, (float)p.h) || !holdsFeet(tileAt(q.area, col, feetRow));
        } else if (p.kind != PROJ_RAIN || p.range < 300 * DP - 120 * DP) {   // rain falls through tiles until it reaches the hero's height
            gone |= boxInSolid(q.area, p.x, p.y, (float)p.w, (float)p.h);
        }
        if (!gone && p.fromHero) {
            touchLever(q, p.x, p.y, (float)p.w, (float)p.h);
            for (int k = 0; k < (int)q.enemies.size() && k < 64 && !gone; k++) {
                Enemy& e = q.enemies[k];
                const EnemyType& type = rosterType(e.world, e.type);
                bool grounded = type.behaviour != BEH_FLYER && e.state != ES_HANG;
                if (!e.alive || e.state == ES_EMBER || (p.hitMask >> k & 1) || (p.kind == PROJ_WAVE && !grounded) ||
                    !overlaps(p.x, p.y, (float)p.w, (float)p.h, e.x, e.y, e.w, e.h))
                    continue;
                p.hitMask |= 1ull << k;
                hitEnemy(q, k, p.mult, p.strong, p.x + p.w / 2);
                if (p.kind == PROJ_ORB) burst(q, p.x + p.w / 2, p.y + p.h / 2, paletteColor('i'), 6);
                if (p.kind == PROJ_ARROW && p.w >= 10 * DP) spark(q, p.x + p.w / 2, p.y + p.h / 2, 'c');   // the Piercing Bolt
                if (p.kind == PROJ_ORB && e.alive) e.slowTimer = 3;
                if (!p.strong) h.mp = h.mp < s.maxMp ? h.mp + 1 : s.maxMp;
                if (p.pierce-- <= 0) gone = true;
            }
            for (int k = 0; k < (int)q.bosses.size() && k < 8 && !gone; k++) {
                QuestBoss& b = q.bosses[k];
                if (!bossHittable(b) || (p.bossMask >> k & 1) || !overlaps(p.x, p.y, (float)p.w, (float)p.h, b.x, b.y, (float)b.w, (float)b.h)) continue;
                p.bossMask |= 1 << k;
                hitBoss(q, k, p.mult, p.strong);
                if (!p.strong) h.mp = h.mp < s.maxMp ? h.mp + 1 : s.maxMp;
                if (p.pierce-- <= 0) gone = true;
            }
        } else if (!gone && p.kind == PROJ_ICE_CUBE && overlaps(p.x, p.y, (float)p.w, (float)p.h, h.x, h.y, HERO_W, HERO_H)) {
            if (h.iframes <= 0 && h.frozenTimer <= 0) {
                h.frozenTimer = FROZEN_TIME;
                q.soundRequested[SOUND_FREEZE] = true;
            }
            gone = true;
        } else if (!gone && p.damage > 0 && overlaps(p.x, p.y, (float)p.w, (float)p.h, h.x, h.y, HERO_W, HERO_H)) {
            hurtHero(q, p.trap ? p.damage : damageToHero(p.damage, s.def), p.x + p.w / 2);
            gone = true;
        }
        if (gone && (p.kind == PROJ_PEEL || p.kind == PROJ_POOP) && p.range > 0) {   // a lobbed peel or poop lands where it hit
            float bottom = p.y + p.h;
            int row = (int)floorf(bottom / TILE);
            if (tileSolid(tileAt(q.area, (int)floorf((p.x + p.w / 2) / TILE), row))) bottom = (float)(row * TILE);
            if (p.kind == PROJ_PEEL) q.hazards.push_back({HAZARD_PEEL, p.x + p.w / 2, bottom, PEEL_TIME});
            else q.hazards.push_back({HAZARD_MIST, p.x + p.w / 2, bottom - MIST_RADIUS / 2, MIST_TIME});
        }
        if (gone) q.projectiles.erase(q.projectiles.begin() + i);
        else i++;
    }

    // Contact with enemies, the Sentinel's swing, and traps.
    float heroCx = h.x + HERO_W / 2;
    for (Enemy& e : q.enemies) {
        if (!e.alive || e.state == ES_EMBER) continue;
        const EnemyType& type = rosterType(e.world, e.type);
        if (type.behaviour == BEH_ARMORED && e.state == ES_ACT) {
            float sx = e.facingLeft ? e.x - ARMORED_SWING_W : e.x + e.w, sy = e.y + e.h - ARMORED_SWING_H;
            if (overlaps(sx, sy, ARMORED_SWING_W, ARMORED_SWING_H, h.x, h.y, HERO_W, HERO_H))
                hurtHero(q, damageToHero((int)lroundf(e.atk * ARMORED_SWING_MULT), s.def), e.x + e.w / 2);
        }
        if (!overlaps(e.x, e.y, e.w, e.h, h.x, h.y, HERO_W, HERO_H) || h.iframes > 0 || q.godMode) continue;
        float mult = type.trait == TRAIT_INFLATE && e.state == ES_ACT ? INFLATE_MULT : 1;
        if (type.trait == TRAIT_STEAL_NAMES) {   // the name wraith takes 10 % of the carried names; kill it within 10 s to win them back
            int stolen = (int)(q.save.names * STEAL_FRACTION);
            q.save.names -= stolen;
            e.stolen += stolen;
            e.stolenTimer = STEAL_TIME;
        }
        hurtHero(q, damageToHero((int)lroundf(e.atk * mult), s.def), e.x + e.w / 2);
    }
    int c0 = (int)floorf(h.x / TILE), c1 = (int)floorf((h.x + HERO_W - 0.001f) / TILE);
    int r0 = (int)floorf(h.y / TILE), r1 = (int)floorf((h.y + HERO_H - 0.001f) / TILE);
    float away = heroCx + (h.facingLeft ? 1 : -1);   // traps knock the hero back the way it came
    for (int c = c0; c <= c1; c++)
        for (int r = r0; r <= r1 + 1; r++) {
            char t = tileAt(q.area, c, r);
            if (r <= r1 && t == '^') hurtHero(q, (int)fmaxf(1, lroundf(s.maxHp * SPIKE_DAMAGE)), away, SPIKE_BOUNCE);
            if (r <= r1 && t == 'z' && panelOn(q)) hurtHero(q, (int)fmaxf(1, lroundf(s.maxHp * PANEL_DAMAGE)), away);
            for (int k = 1; k <= JET_TILES && r <= r1; k++)
                if (tileAt(q.area, c, r + k) == 'f' && jetBurning(q)) hurtHero(q, (int)fmaxf(1, lroundf(s.maxHp * JET_DAMAGE)), away);
        }
    // Tile traps (spec §9): coconut palms (world 2), icicles (world 3) and crumbling blocks, each its own little cycle.
    const AreaDef& def = *q.area.def;
    int icicle = 0;
    for (TileTrap& trap : q.traps) {
        int col = trap.tile % def.cols, row = trap.tile / def.cols;
        float tx = col * TILE + TILE / 2.0f, bottom = (float)((row + 1) * TILE);
        trap.timer -= dt;
        if (trap.kind == 'R') {   // the hero's centre passing within 8 px below drops the coconut 0.3 s later; it regrows in 4 s
            if (trap.state == 0 && fabsf(heroCx - tx) <= COCONUT_REACH && h.y > bottom - TILE) trap.state = 1, trap.timer = COCONUT_DELAY;
            else if (trap.state == 1 && trap.timer <= 0) {
                Projectile p = {};
                p.x = tx - COCONUT_SIZE / 2;
                p.y = bottom - COCONUT_SIZE;
                p.vy = COCONUT_SPEED;
                p.w = p.h = COCONUT_SIZE;
                p.damage = (int)fmaxf(1, lroundf(s.maxHp * COCONUT_DAMAGE));
                p.trap = true;
                p.range = (float)(def.rows * TILE);
                p.kind = PROJ_COCONUT;
                q.projectiles.push_back(p);
                trap.state = 2;
                trap.timer = COCONUT_REGROW;
            } else if (trap.state == 2 && trap.timer <= 0) trap.state = 0;
        } else if (trap.kind == 'Y') {   // the hero within 12 px below: it shakes 0.25 s and falls; gone until a rest
            float top = (float)(row * TILE);
            if (trap.state == 0 && fabsf(heroCx - tx) <= ICICLE_REACH && h.y > top) trap.state = 1, trap.timer = ICICLE_DELAY;
            else if (trap.state == 1 && trap.timer <= 0) {
                Projectile p = {};
                p.x = tx - ICICLE_W / 2;
                p.y = top;
                p.vy = ICICLE_SPEED;
                p.w = ICICLE_W;
                p.h = ICICLE_H;
                p.damage = (int)fmaxf(1, lroundf(s.maxHp * ICICLE_DAMAGE));
                p.trap = true;
                p.range = (float)(def.rows * TILE);
                p.kind = PROJ_ICICLE;
                q.projectiles.push_back(p);
                trap.state = 2;
                std::vector<uint64_t>& killed = q.killed[q.area.world];
                if (icicle < 64 - ICICLE_BIT && q.area.index < (int)killed.size()) killed[q.area.index] |= 1ull << (ICICLE_BIT + icicle);
            }
            icicle++;
        } else if (trap.kind == 'c') {   // stood on: shakes 0.6 s, breaks, returns after 4 s (not into the hero)
            bool standing = h.onGround && (int)lroundf((h.y + HERO_H) / TILE) == row && h.x < (col + 1) * TILE && h.x + HERO_W > col * TILE;
            if (trap.state == 0 && standing) trap.state = 1, trap.timer = CRUMBLE_SHAKE;
            else if (trap.state == 1 && trap.timer <= 0) {
                q.area.tiles[trap.tile] = '.';
                q.soundRequested[SOUND_CRACK] = true;
                trap.state = 2;
                trap.timer = CRUMBLE_RETURN;
            } else if (trap.state == 2 && trap.timer <= 0 && !overlaps(col * TILE, row * TILE, TILE, TILE, h.x, h.y, HERO_W, HERO_H)) {
                q.area.tiles[trap.tile] = 'c';
                trap.state = 0;
            }
        }
    }
    // Peels and stink mists (spec §12 world 2): a peel slides the hero standing on it; a mist takes 3 % of max HP per s.
    for (size_t i = 0; i < q.hazards.size();) {
        FloorHazard& z = q.hazards[i];
        if (z.kind == HAZARD_PEEL && h.onGround && h.slideTimer <= 0 && overlaps(z.x - PEEL_W / 2, z.y - PEEL_H, PEEL_W, PEEL_H, h.x, h.y, HERO_W, HERO_H)) {
            h.slideTimer = PEEL_SLIDE;
            h.slideSpeed = (h.facingLeft ? -1 : 1) * fmaxf(fabsf(h.vx), PEEL_MIN_SPEED);
            q.soundRequested[SOUND_Q_DODGE] = true;
        }
        float mx = heroCx - z.x, my = h.y + HERO_H / 2 - z.y;
        if (z.kind == HAZARD_MIST && mx * mx + my * my <= MIST_RADIUS * MIST_RADIUS && !q.godMode) {
            q.mistDamage += MIST_HURT * s.maxHp * dt;
            int whole = (int)q.mistDamage;
            q.mistDamage -= whole;
            h.hp = h.hp > whole ? h.hp - whole : 0;
        }
        if ((z.timer -= dt) <= 0) q.hazards.erase(q.hazards.begin() + i);
        else i++;
    }
    // Panels turning on buzz when one is on screen (spec §9).
    float panelCycle = PANEL_ON + (q.area.def->band >= 2 ? 1.0f : 1.5f);
    if (panelOn(q) && fmodf(q.time - dt, panelCycle) >= PANEL_ON) {
        const AreaDef& def = *q.area.def;
        for (int t = 0; t < def.cols * def.rows; t++) {
            float px = (t % def.cols) * TILE - q.camX, py = (t / def.cols) * TILE - q.camY;
            if (q.area.tiles[t] == 'z' && px > -TILE && px < QUEST_W && py > -TILE && py < QUEST_H) {
                q.soundRequested[SOUND_Q_ZAP] = true;
                break;
            }
        }
    }
}
