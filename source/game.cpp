// Game rules and drawing. Every tuning number comes from a design note in docs/features/.
#include "game.h"
#include "screen.h"
#include "sprites.h"
#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <string.h>

// Ship controls (ship-controls/design.md)
const float SHIP_SPEED = 150;          // px/s
const float FIRE_INTERVAL = 0.12f;     // s
const float BULLET_SPEED = 320;        // px/s
const float MOVE_ZONE_TOP = 100;       // px
// Enemy waves (enemy-waves/design.md)
const int FORMATION_COLUMNS = 8;
const float COLUMN_SPACING = 18, ROW_SPACING = 14, FORMATION_TOP = 18;   // px
const float SWAY_RANGE = 40;           // px either side of the centre
const float WAVE_PAUSE = 2;            // s between waves
const float BANNER_TIME = 1.5f;        // s
const int ALIEN_HP[4] = {1, 2, 4, 12};
const int ALIEN_SCORE[4] = {100, 200, 400, 1000};
const int WAVE_CLEAR_BONUS = 1000;     // points x wave
const float ENEMY_SHOT_SPEED = 70;     // px/s
const int SHOT_SPEEDUP_AFTER_WAVE = 10;
const float ENEMY_SHOT_SPEED_STEP = 2, ENEMY_SHOT_MAX_SPEED = 90;   // px/s per wave, px/s
const int MORE_ARMOUR_WAVE = 13;       // two red and two blue rows from here
const int TWO_DIVERS_WAVE = 15;        // two aliens may dive at once from here
const float ENTRY_DISTANCE = 150, ENTRY_SPEED = 150;      // px, px/s
const float ENTRY_COLUMN_DELAY = 0.06f, ENTRY_ROW_DELAY = 0.12f;   // s
const float DIVE_SPEED = 90, DIVE_STEER = 120, DIVE_MAX_SIDEWAYS = 70, RETURN_SPEED = 100;   // px/s, px/s^2
// Rocket (rocket/design.md)
const int START_ROCKETS = 3, MAX_ROCKETS = 9;
const float ROCKET_COOLDOWN = 0.5f, ROCKET_BUFFER = 0.15f;   // s
const float ROCKET_START_SPEED = 60, ROCKET_TOP_SPEED = 260, ROCKET_ACCEL = 500;   // px/s, px/s^2
const float EXPLOSION_RADIUS = 40;     // px
const int EXPLOSION_DAMAGE = 20;       // HP
const float PICKUP_CHANCE = 0.02f;     // per kill
const float PICKUP_FALL_SPEED = 30;    // px/s
const float EXPLOSION_GROW = 0.1f, EXPLOSION_FADE = 0.35f;   // s
// Weapon upgrades (weapon-upgrades/design.md)
const int MAX_WEAPON_LEVEL = 4;
const float FAST_FIRE_INTERVAL = 0.08f;   // s, level 4
const float DOUBLE_OFFSET = 3, SPREAD_SPEED = 70;   // px, px/s
const float UPGRADE_CHANCE = 0.03f;    // per kill, rolled together with PICKUP_CHANCE
const int UPGRADE_MAX_POINTS = 1000;
// Bosses (bosses/design.md)
const int BOSS_W = 32, BOSS_H = 20;
const float BOSS_STOP_Y = 20;          // px, where it hovers after flying in
const int BOSS_BURST_SHOTS = 3, BOSS_FAN_SHOTS = 5;
const float BOSS_BURST_GAP = 0.15f;    // s
const float BOSS_AIMED_SPEED = 90, BOSS_FAN_DOWN_SPEED = 70, BOSS_FAN_SIDEWAYS = 60;   // px/s
const int BIG_BOSS_WAVE = 15;          // waves 15 and 20 bring each level's big boss
const float TWIN_FIRST_ATTACK = 1.5f;      // x the interval: the second twin's first attack
const float MAGE_BURST_CHANCE = 0.35f, MAGE_FAN_CHANCE = 0.35f;   // the rest is a power ball
const float MAGE_GAP_MIN = 0.6f, MAGE_GAP_MAX = 1.4f;             // x the attack interval
const int WAND_TIP_X = 30, WAND_TIP_Y = 8;    // px in the mage sprite: the wand's star, where the power ball appears
const float BALL_CHARGE_TIME = 1.5f, BEAM_TIME = 0.8f;   // s
const int BALL_MAX_RADIUS = 5, BEAM_W = 24, BEAM_CORE_W = 8;   // px
const float TRAUMA_BEAM = 0.3f;
const int BOSS_REWARD = 5000;          // points x (appearance + the level's reward offset)
const float BOSS_EXPLOSION_RADIUS = 48;   // px
const int BOSS_HEALTH_BAR_W = 120, BOSS_HEALTH_BAR_Y = 16;   // px
// Game feel (game-feel/design.md)
const float TRAUMA_DECAY = 1.5f;       // per s
const float SHAKE_X = 4, SHAKE_Y = 3;  // px at trauma 1 and full strength
const float FLASH_MIN_GAP = 0.33f;     // s between screen flashes
const float TRAUMA_BLAST = 0.6f, STOP_BLAST = 0.06f, FLASH_BLAST = 0.05f;
const float TRAUMA_BOSS_DEATH = 1.0f, STOP_BOSS_DEATH = 0.15f, FLASH_BOSS_DEATH = 0.08f;
const float TRAUMA_PLAYER_DEATH = 0.8f, STOP_PLAYER_DEATH = 0.12f, FLASH_PLAYER_DEATH = 0.08f;
const int BLAST_PARTICLES = 20, PICKUP_PARTICLES = 4;
// Lives and score (lives-and-score/design.md)
const int START_LIVES = 3, MAX_LIVES = 9;
const float RESPAWN_DELAY = 1.5f, INVINCIBLE_TIME = 2, BLINK_PERIOD = 0.1f;   // s
const int EXTRA_LIFE_EVERY = 50000;
const float SHIP_EXPLOSION_RADIUS = 24;   // px
// Co-op (co-op/design.md)
const float CO_OP_SPAWN_OFFSET = 40;     // px right of the centre for player 2
// Level 1 (level-1/design.md)
const int LEVEL_LAST_WAVE = 20;        // the level boss's wave; killing it ends the run
const float LEVEL_BANNER_TIME = 2;     // s of "LEVEL 1" before "WAVE 1"
const float LEVEL_CLEAR_TIME = 3;      // s of "LEVEL 1 CLEAR" before the end screen
// Levels (jungle, glacier, volcano and deep-sea design notes): index level - 1; boss tables by n - 1, n = wave / 5
const char* const LEVEL_NAMES[LEVEL_COUNT] = {"LEVEL 1", "JUNGLE", "GLACIER", "VOLCANO", "DEEP SEA"};
const char* const LEVEL_LABELS[LEVEL_COUNT] = {"WAVE", "JUNGLE", "GLACIER", "VOLCANO", "DEEP SEA"};
const int LEVEL_WAVE_OFFSET[LEVEL_COUNT] = {0, 5, 8, 11, 14};       // wave n uses level 1's formation numbers for n + this
const int HEAVY_EVERY[LEVEL_COUNT] = {0, 8, 6, 5, 4};              // heavies per formation wave: wave / this (0: none)
const int MAX_HEAVIES = 3;
const float HEAVY_SLOT_X[MAX_HEAVIES] = {60, 244, 152};            // px, where each heavy flies in
const float SHOT_SPEED_BONUS[LEVEL_COUNT] = {0, 0, 5, 10, 15};      // px/s added to the formation's shots
const BossType BOSS_TYPES[LEVEL_COUNT][2] = {{BOSS_CLASSIC, BOSS_MAGE}, {BOSS_MONKEY, BOSS_GORILLA}, {BOSS_YETI, BOSS_GOLEM},
                                             {BOSS_IMP, BOSS_FIRELORD}, {BOSS_ANGLER, BOSS_FISHERMAN}};   // small, big
const int LEVEL_BOSS_HP[LEVEL_COUNT][4] = {{100, 100, 200, 200}, {150, 150, 300, 300}, {180, 180, 340, 340},
                                           {200, 200, 380, 380}, {220, 220, 420, 420}};
const float LEVEL_BOSS_SPEED[LEVEL_COUNT][4] = {{40, 50, 60, 70}, {50, 60, 45, 55}, {50, 60, 45, 55}, {55, 65, 50, 60},
                                                {55, 65, 50, 60}};   // px/s
const float LEVEL_BOSS_INTERVAL[LEVEL_COUNT][4] = {{2.5f, 2.2f, 1.9f, 1.6f}, {2.2f, 2.0f, 1.8f, 1.6f}, {2.1f, 1.9f, 1.7f, 1.5f},
                                                   {2.0f, 1.8f, 1.6f, 1.4f}, {1.9f, 1.7f, 1.5f, 1.3f}};   // s
const int REWARD_OFFSET[LEVEL_COUNT] = {0, 4, 8, 12, 16};          // boss n pays BOSS_REWARD x (n + this)
const float FAR_SPEED[LEVEL_COUNT] = {0, 8, 7, 8, 6}, MID_SPEED[LEVEL_COUNT] = {0, 18, 16, 18, 15};   // px/s, scenery layers
const int TILE_H = 360;                // px: a scenery tile is two screens tall and wraps
// Heavies (jungle/design.md)
const float HEAVY_SPEED_X = 40, HEAVY_SPEED_Y = 25;   // px/s
const float HEAVY_MIN_Y = 18, HEAVY_MAX_Y = 84;       // px, the roaming box's top and bottom (sprite top)
const float HEAVY_SHOT_FACTOR = 2;     // x the formation's shot chance
// Jungle (jungle/design.md)
const float BANANA_SPEED = 85;         // px/s, straight down
const int BUNCH_BANANAS = 5;
const float BUNCH_DOWN_SPEED = 75, BUNCH_SIDEWAYS = 50;   // px/s
const float GORILLA_THROW_CHANCE = 0.3f, GORILLA_POOP_CHANCE = 0.3f, GORILLA_BUNCH_CHANCE = 0.2f;   // the rest is a pound
const float LOB_TIME = 0.8f, LOB_ARC = 40, PEEL_DELAY = 0.45f;   // s, px, s
const float PEEL_LIFE = 5, PEEL_BLINK = 1;   // s
const int PEEL_W = 8, PEEL_H = 6;
const float SLIDE_TIME = 0.5f, SLIDE_SPEED = 140, STUN_TIME = 1.0f;   // s, px/s, s
const float MIST_RADIUS = 20, MIST_GROW = 0.3f, MIST_TIME = 6, MIST_FADE = 1;   // px, s, s, s
const int MAX_MISTS = 3;               // mists plus poops in flight
const float LAUGH_TIME = 0.6f, POUND_TIME = 0.6f, TRAUMA_POUND = 0.25f;
const int COCONUTS = 5;
const float COCONUT_SPEED = 110;       // px/s
const int THROWN_SIZE = 6;             // px: bananas and coconuts
const int FIREFLIES = 14;
const float FIREFLY_SPEED = 22;        // px/s
// Glacier (glacier/design.md)
const float ICE_GRIP = 5;              // per s: how fast the ship's speed follows the stick on the ice
const float SNOWBALL_SPEED = 90, ICE_BLOCK_SPEED = 80;   // px/s, both aimed
const float FREEZE_TIME = 1.6f, FREEZE_MASH = 0.15f;     // s; s off per fresh R1 press
const float SNEEZE_WINDUP = 0.5f, SAY_TIME = 0.6f;       // s
const int SNEEZE_SNOWBALLS = 7;
const float SNEEZE_DOWN_SPEED = 80, SNEEZE_SIDEWAYS = 60, TRAUMA_SNEEZE = 0.15f;   // px/s
const float SNOW_FAN_DOWN_SPEED = 75, SNOW_FAN_SIDEWAYS = 60;                     // px/s
const float GOLEM_VOLLEY_CHANCE = 0.3f, GOLEM_BLOCKS_CHANCE = 0.25f, GOLEM_FAN_CHANCE = 0.2f;   // the rest is icicle rain
const int ICICLES = 6;
const float ICICLE_HANG = 0.8f, ICICLE_SPEED = 140;      // s, px/s
const int SNOWFLAKES = 30;
const float SNOW_SPEED = 14;           // px/s
// Volcano (volcano/design.md)
const float FIREBALL_SPEED = 95;       // px/s, aimed
const int LAVA_RAIN_DROPS = 5;
const float LAVA_DROP_SPEED = 60, LAVA_RAIN_SPEED = 70;   // px/s: the imp's big drop falls straight down
const float FIRE_FAN_DOWN_SPEED = 80, FIRE_FAN_SIDEWAYS = 60;                     // px/s
const float SCORCH_TIME = 3, SCORCH_SPEED = 0.6f, HOT_TEXT_TIME = 0.6f;          // s, x ship speed, s
const int SCORCH_DAMAGE = 2;           // per bullet while scorched
const float FIRELORD_VOLLEY_CHANCE = 0.3f, FIRELORD_FAN_CHANCE = 0.2f, FIRELORD_RAIN_CHANCE = 0.25f;   // the rest erupts
const float ERUPTION_WARN = 1.0f, ERUPTION_TIME = 0.6f, TRAUMA_ERUPTION = 0.3f;   // s, s
const int ERUPTION_W = 16;             // px
const int EMBERS = 20;
const float EMBER_RISE_SPEED = 20;     // px/s
// Deep sea (deep-sea/design.md)
const float SEA_CURRENT = 18, CURRENT_RATE = 0.45f;      // px/s, rad/s: 18 sin(0.45 t) sideways
const int STREAM_BUBBLES = 5;
const float STREAM_GAP = 0.12f, BUBBLE_SPEED = 70, BUBBLE_WOBBLE = 25;   // s, px/s, px/s
const float BIG_BUBBLE_SPEED = 45, BIG_BUBBLE_POP_TIME = 1.4f, BIG_BUBBLE_POP_Y = 150, POP_SPEED = 60;   // px/s, s, px, px/s
const int POP_BUBBLES = 8, BUBBLE_FAN = 7;
const float BUBBLE_FAN_DOWN_SPEED = 65, BUBBLE_FAN_SIDEWAYS = 70;       // px/s
const float FISHER_STREAM_CHANCE = 0.3f, FISHER_BIG_CHANCE = 0.15f, FISHER_FAN_CHANCE = 0.15f, FISHER_FISH_CHANCE = 0.2f;   // the rest casts the hook
const int ROD_TIP_X = 31, ROD_TIP_Y = 0;   // px in the fisherman sprite
const int BUCKET_X = 16, BUCKET_Y = 17;    // px in the fisherman sprite: live fish are thrown from here
const int FISH_PER_THROW = 2, MAX_FISH = 3;   // live fish out: landed plus in flight
const float FISH_SPEED = 35, FISH_LIFE = 6, FISH_BLINK = 1, FISH_THROW_GAP = 0.2f;   // px/s, s, s, s
const int FISH_W = 9, FISH_H = 6, FISH_SCORE = 200;
const float TRAIL_GAP = 0.05f, TRAIL_RISE = 15, TRAIL_LIFE = 0.6f;   // s between bubbles, px/s, s
const float HOOK_DOWN_SPEED = 110, HOOK_REEL_SPEED = 160, HOOK_WAIT_TIME = 0.3f, HOOK_BOTTOM = SCREEN_H - 16;   // px/s, s, px
const float HOOK_LIFT = 24, HOOK_LIFT_SPEED = 60, HOOK_STUN = 1.6f;   // px, px/s, s
const float BOOT_CHANCE = 0.35f, BOOT_SPEED = 85, GOT_ONE_TIME = 1, BOOT_TEXT_TIME = 0.8f;   // per empty cast, px/s, s, s
const int SEA_BUBBLES = 16;
const float SEA_BUBBLE_SPEED = 25;     // px/s
// Pixel art (pixel-art/design.md)
const float ANIMATION_FRAME = 0.25f;   // s
const float HIT_FLASH = 0.06f;         // s
const int STAR_COUNTS[3] = {30, 20, 10};
const float STAR_SPEEDS[3] = {6, 14, 30};   // px/s
const char STAR_COLORS[3] = {'n', 'd', 's'};

const int SHIP_W = 16, SHIP_H = 14;
const int ALIEN_WIDTH[4] = {12, 12, 12, 16}, ALIEN_HEIGHT[4] = {10, 10, 10, 12};
const int HITBOX = 6, HITBOX_X = 5, HITBOX_Y = 4;   // ship hitbox: 6x6, centred on the sprite
const Sprite* const ALIEN_SPRITES[LEVEL_COUNT][4][2] = {
    {{&GREEN_ALIEN_A, &GREEN_ALIEN_B}, {&BLUE_ALIEN_A, &BLUE_ALIEN_B}, {&RED_ALIEN_A, &RED_ALIEN_B}, {&RHINO_A, &RHINO_B}},
    {{&LEAFHOPPER_A, &LEAFHOPPER_B}, {&BEETLE_A, &BEETLE_B}, {&STAG_A, &STAG_B}, {&RHINO_A, &RHINO_B}},
    {{&FLAKE_A, &FLAKE_B}, {&ICEBAT_A, &ICEBAT_B}, {&FROSTSKULL_A, &FROSTSKULL_B}, {&YETICLOUD_A, &YETICLOUD_B}},
    {{&EMBER_A, &EMBER_B}, {&FIREBAT_A, &FIREBAT_B}, {&MAGMASKULL_A, &MAGMASKULL_B}, {&PHOENIX_A, &PHOENIX_B}},
    {{&PIRANHA_A, &PIRANHA_B}, {&PUFFER_A, &PUFFER_B}, {&LIONFISH_A, &LIONFISH_B}, {&SHARK_A, &SHARK_B}}};
const char ALIEN_COLORS[LEVEL_COUNT][4] = {{'g', 'u', 'r', 'X'}, {'l', 'h', 'X', 'H'}, {'i', 'I', 'w', 'b'},
                                           {'O', 'Z', 'Z', 'N'}, {'b', 'Y', 'r', 's'}};   // death particles
const int THROWN_W[] = {6, 6, 6, 24, 4, 6, 5, 5, 9, 7, 10}, THROWN_H[] = {6, 6, 6, 24, 9, 6, 6, 5, 9, 6, 12};   // by ThrownKind
const Sprite* const THROWN_SPRITES[][2] = {{&BANANA_A, &BANANA_B}, {&COCONUT, &COCONUT}, {&SNOWBALL, &SNOWBALL},
                                           {&ICE_BLOCK, &ICE_BLOCK}, {&ICICLE, &ICICLE}, {&FIREBALL_A, &FIREBALL_B},
                                           {&LAVA_DROP, &LAVA_DROP}, {&BUBBLE, &BUBBLE}, {&BIG_BUBBLE, &BIG_BUBBLE},
                                           {&BOOT, &BOOT}, {&BIG_LAVA_DROP, &BIG_LAVA_DROP}};   // by ThrownKind, two animation frames

static float randomUnit(Game& game) {
    uint32_t& s = game.random;
    s ^= s << 13;
    s ^= s >> 17;
    s ^= s << 5;
    return (s & 0xFFFFFF) / (float)0x1000000;
}

static bool overlap(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) {
    return ax < bx + bw && bx < ax + aw && ay < by + bh && by < ay + ah;
}

int formationRows(int wave) {
    return std::min(2 + wave / 2, 5);
}

float shotChance(int wave) {
    return std::min(0.02f + 0.005f * wave, 0.08f);
}

float enemyShotSpeed(int wave) {
    return std::min(ENEMY_SHOT_SPEED + ENEMY_SHOT_SPEED_STEP * std::max(wave - SHOT_SPEEDUP_AFTER_WAVE, 0), ENEMY_SHOT_MAX_SPEED);
}

float diveInterval(int wave) {
    return std::max(4 - 0.2f * wave, 1.5f);
}

bool isBossWave(int wave) {
    return wave > 0 && wave % 5 == 0;
}

int bossHealth(int level, int wave) {
    return LEVEL_BOSS_HP[level - 1][wave / 5 - 1];
}

int bossCount(int wave) {
    return wave % 10 == 0 ? 2 : 1;
}

BossType bossType(int level, int wave) {
    return BOSS_TYPES[level - 1][wave >= BIG_BOSS_WAVE ? 1 : 0];
}

int difficultyWave(int level, int wave) {
    return wave + LEVEL_WAVE_OFFSET[level - 1];
}

int heavyCount(int level, int wave) {
    int every = HEAVY_EVERY[level - 1];
    return every > 0 && !isBossWave(wave) ? std::min(wave / every, MAX_HEAVIES) : 0;
}

const char* levelName(int level) {
    return LEVEL_NAMES[level - 1];
}

const char* levelLabel(int level) {
    return LEVEL_LABELS[level - 1];
}

static float bossAttackInterval(int level, int wave) {
    return LEVEL_BOSS_INTERVAL[level - 1][wave / 5 - 1];
}

static void startWave(Game& game, int wave) {
    game.wave = wave;
    game.aliens.clear();
    int dw = difficultyWave(game.level, wave);
    int rows = formationRows(dw);
    game.bosses.clear();
    if (isBossWave(wave)) {
        int count = bossCount(wave);
        float speed = LEVEL_BOSS_SPEED[game.level - 1][wave / 5 - 1];
        for (int i = 0; i < count; i++) {
            Boss boss = {};
            boss.type = bossType(game.level, wave);
            boss.minX = count == 1 ? 0 : i * SCREEN_W / 2.0f;
            boss.maxX = count == 1 ? SCREEN_W - BOSS_W : boss.minX + SCREEN_W / 2.0f - BOSS_W;
            boss.x = (boss.minX + boss.maxX) / 2;
            boss.y = -BOSS_H;
            boss.vx = count == 2 && i == 0 ? -speed : speed;   // twins move apart first
            boss.hp = boss.maxHp = bossHealth(game.level, wave);
            boss.attackTimer = bossAttackInterval(game.level, wave) * (i == 0 ? 1 : TWIN_FIRST_ATTACK);
            boss.alive = true;
            game.bosses.push_back(boss);
        }
        rows = 0;
    }
    float left = (SCREEN_W - (FORMATION_COLUMNS - 1) * COLUMN_SPACING - ALIEN_WIDTH[ALIEN_GREEN]) / 2;
    for (int row = 0; row < rows; row++) {
        // Tougher aliens take the top rows: red from wave 7, blue from wave 4, two of each from wave 13.
        int redRows = dw >= MORE_ARMOUR_WAVE ? 2 : dw >= 7 ? 1 : 0;
        int blueEnd = dw >= MORE_ARMOUR_WAVE ? 4 : dw >= 4 ? 2 : 0;   // rows above this that aren't red are blue
        AlienType type = row < redRows ? ALIEN_RED : row < blueEnd ? ALIEN_BLUE : ALIEN_GREEN;
        for (int col = 0; col < FORMATION_COLUMNS; col++) {
            Alien alien = {};
            alien.type = type;
            alien.state = ALIEN_ENTERING;
            alien.slotX = left + col * COLUMN_SPACING;
            alien.slotY = FORMATION_TOP + row * ROW_SPACING;
            alien.x = alien.slotX + game.swayOffset;
            alien.y = alien.slotY - ENTRY_DISTANCE;
            alien.vx = 0;
            alien.entryDelay = col * ENTRY_COLUMN_DELAY + row * ENTRY_ROW_DELAY;
            alien.hp = ALIEN_HP[type];
            alien.flash = 0;
            alien.alive = true;
            game.aliens.push_back(alien);
        }
    }
    for (int i = 0; i < heavyCount(game.level, wave); i++) {   // roaming heavies (jungle rule, later levels too)
        Alien heavy = {};
        heavy.type = ALIEN_HEAVY;
        heavy.state = ALIEN_ENTERING;
        heavy.slotX = HEAVY_SLOT_X[i];
        heavy.slotY = 70;
        heavy.x = heavy.slotX + game.swayOffset;
        heavy.y = heavy.slotY - ENTRY_DISTANCE;
        heavy.vx = i == 1 ? -HEAVY_SPEED_X : HEAVY_SPEED_X;
        heavy.vy = HEAVY_SPEED_Y;
        heavy.entryDelay = 0.6f + 0.4f * i;
        heavy.hp = ALIEN_HP[ALIEN_HEAVY];
        heavy.alive = true;
        game.aliens.push_back(heavy);
    }
    game.diveTimer = diveInterval(dw);
    game.bannerTimer = BANNER_TIME + (wave == 1 ? LEVEL_BANNER_TIME : 0);   // "LEVEL 1" first, then "WAVE 1"
    game.nextWaveTimer = 0;
    game.soundRequested[SOUND_WAVE_START] = true;
}

// Player 1 starts at the bottom centre, player 2 to its right.
static void placeShipAtStart(Player& player, int index) {
    player.x = (SCREEN_W - SHIP_W) / 2.0f + index * CO_OP_SPAWN_OFFSET;
    player.y = SCREEN_H - SHIP_H - 6;
}

static Player newPlayer(int index) {
    Player player = {};
    placeShipAtStart(player, index);
    player.alive = true;
    player.rocketPressedAt = -1;
    player.lives = START_LIVES;
    player.rockets = START_ROCKETS;
    player.nextExtraLife = EXTRA_LIFE_EVERY;
    player.weaponLevel = 1;
    return player;
}

void startGame(Game& game, int playerCount, uint32_t seed, int level, bool singleLevel) {
    game = Game();
    game.level = level;
    game.singleLevel = singleLevel;
    game.random = seed ? seed : 1;
    game.swayDirection = 1;
    for (int i = 0; i < playerCount; i++) game.players.push_back(newPlayer(i));
    for (int layer = 0; layer < 3; layer++)
        for (int i = 0; i < STAR_COUNTS[layer]; i++) {
            Star star = {randomUnit(game) * SCREEN_W, randomUnit(game) * SCREEN_H, layer};
            game.stars.push_back(star);
        }
    startWave(game, 1);
}

void joinGame(Game& game) {
    Player player = newPlayer((int)game.players.size());
    player.invincibleTimer = INVINCIBLE_TIME;
    game.players.push_back(player);
    game.soundRequested[SOUND_EXTRA_LIFE] = true;
}

// One event's feel: trauma adds (capped), the longest hit-stop wins, a screen flash is dropped inside the rate limit.
static void feedback(Game& game, float trauma, float hitStop, float flash) {
    game.trauma = std::min(game.trauma + trauma, 1.0f);
    game.hitStop = std::max(game.hitStop, hitStop);
    if (flash > 0 && game.feelTime - game.lastFlashTime >= FLASH_MIN_GAP) {
        game.flashTimer = flash;
        game.lastFlashTime = game.feelTime;
    }
}

// Adds points and gives an extra life for every threshold crossed (lives-and-score rule 6).
static void addScore(Game& game, Player& player, int points) {
    player.score = std::min(player.score + points, MAX_SCORE);
    while (player.score >= player.nextExtraLife) {
        player.lives = std::min(player.lives + 1, MAX_LIVES);
        player.nextExtraLife += EXTRA_LIFE_EVERY;
        game.soundRequested[SOUND_EXTRA_LIFE] = true;
    }
}

static void spawnParticles(Game& game, float x, float y, int count, uint32_t color, float speed) {
    for (int i = 0; i < count; i++) {
        float angle = i * 6.283f / count + randomUnit(game) * 0.5f, s = speed * (0.5f + randomUnit(game));
        Particle p = {x, y, cosf(angle) * s, sinf(angle) * s, 0.3f + randomUnit(game) * 0.2f, color};
        game.particles.push_back(p);
    }
}

static void damageAlien(Game& game, Alien& alien, int damage, int owner) {
    alien.hp -= damage;
    alien.flash = HIT_FLASH;
    if (alien.hp > 0) {
        game.soundRequested[SOUND_ALIEN_HIT] = true;
        return;
    }
    alien.alive = false;
    if (owner >= 0) addScore(game, game.players[owner], ALIEN_SCORE[alien.type]);
    float cx = alien.x + ALIEN_WIDTH[alien.type] / 2.0f, cy = alien.y + ALIEN_HEIGHT[alien.type] / 2.0f;
    spawnParticles(game, cx, cy, 8, paletteColor(ALIEN_COLORS[game.level - 1][alien.type]), 50);
    float roll = randomUnit(game);   // below 2 %: rocket pickup, below 5 %: upgrade pickup
    if (roll < PICKUP_CHANCE + UPGRADE_CHANCE) {
        Pickup pickup = {cx - 4, cy - 4, roll < PICKUP_CHANCE ? PICKUP_ROCKET : PICKUP_UPGRADE};
        game.pickups.push_back(pickup);
    }
    game.soundRequested[SOUND_ALIEN_DEATH] = true;
}

static void damageBoss(Game& game, Boss& boss, int damage, int owner) {
    boss.hp -= damage;
    boss.flash = HIT_FLASH;
    if (boss.hp > 0) {
        game.soundRequested[SOUND_ALIEN_HIT] = true;
        return;
    }
    boss.alive = false;
    boss.burstShotsLeft = 0;
    boss.ballTimer = boss.beamTimer = 0;
    boss.laughTimer = boss.windupTimer = 0;
    boss.hookState = HOOK_NONE;   // a ship on the hook dangles until its own timer ends
    feedback(game, TRAUMA_BOSS_DEATH, STOP_BOSS_DEATH, FLASH_BOSS_DEATH);
    if (owner >= 0) addScore(game, game.players[owner], BOSS_REWARD * (game.wave / 5 + REWARD_OFFSET[game.level - 1]));
    float cx = boss.x + BOSS_W / 2.0f, cy = boss.y + BOSS_H / 2.0f;
    Explosion explosion = {cx, cy, 0, BOSS_EXPLOSION_RADIUS};
    game.explosions.push_back(explosion);
    spawnParticles(game, cx, cy, 24, paletteColor('g'), 70);
    spawnParticles(game, cx, cy, 16, paletteColor('o'), 50);
    game.pickups.push_back({cx - 12 - 4, cy - 4, PICKUP_UPGRADE});
    game.pickups.push_back({cx + 12 - 4, cy - 4, PICKUP_ROCKET});
    game.soundRequested[SOUND_ALIEN_DEATH] = true;
    game.soundRequested[SOUND_EXPLOSION] = true;
}

// A rocket blast: every alien whose centre is within the radius (plus half an alien) takes the damage once.
static void explode(Game& game, float x, float y, int owner) {
    Explosion explosion = {x, y, 0, EXPLOSION_RADIUS};
    game.explosions.push_back(explosion);
    spawnParticles(game, x, y, BLAST_PARTICLES, paletteColor('o'), 80);
    feedback(game, TRAUMA_BLAST, STOP_BLAST, FLASH_BLAST);
    for (Alien& alien : game.aliens) {
        float reach = EXPLOSION_RADIUS + ALIEN_WIDTH[alien.type] / 2.0f;
        float dx = alien.x + ALIEN_WIDTH[alien.type] / 2.0f - x, dy = alien.y + ALIEN_HEIGHT[alien.type] / 2.0f - y;
        if (alien.alive && dx * dx + dy * dy <= reach * reach) damageAlien(game, alien, EXPLOSION_DAMAGE, owner);
    }
    // The boss is hit when the circle overlaps its rectangle.
    for (Boss& boss : game.bosses) {
        float nearestX = std::max(boss.x, std::min(x, boss.x + BOSS_W));
        float nearestY = std::max(boss.y, std::min(y, boss.y + BOSS_H));
        if (boss.alive && (nearestX - x) * (nearestX - x) + (nearestY - y) * (nearestY - y) <= EXPLOSION_RADIUS * EXPLOSION_RADIUS)
            damageBoss(game, boss, EXPLOSION_DAMAGE, owner);
    }
    game.soundRequested[SOUND_EXPLOSION] = true;
}

static void killPlayer(Game& game, Player& player) {
    player.alive = false;
    player.slideTimer = player.stunTimer = player.frozenTimer = player.scorchTimer = player.hookedTimer = 0;
    player.vx = player.vy = 0;
    player.lives--;
    player.respawnTimer = RESPAWN_DELAY;
    player.rocketPressedAt = -1;
    player.weaponLevel = std::max(1, player.weaponLevel - 1);
    feedback(game, TRAUMA_PLAYER_DEATH, STOP_PLAYER_DEATH, FLASH_PLAYER_DEATH);
    float cx = player.x + SHIP_W / 2.0f, cy = player.y + SHIP_H / 2.0f;
    Explosion explosion = {cx, cy, 0, SHIP_EXPLOSION_RADIUS};
    game.explosions.push_back(explosion);
    spawnParticles(game, cx, cy, 24, paletteColor('b'), 70);
    spawnParticles(game, cx, cy, 12, paletteColor('c'), 40);
    game.soundRequested[SOUND_PLAYER_DEATH] = true;
}

static void updatePlayer(Game& game, Player& player, int index, const Actions& a, float dt) {
    if (!player.alive) {
        player.respawnTimer -= dt;
        if (player.respawnTimer <= 0 && player.lives > 0) {
            placeShipAtStart(player, index);
            player.alive = true;
            player.invincibleTimer = INVINCIBLE_TIME;
            player.fireTimer = 0;
        }
        return;
    }
    player.invincibleTimer = std::max(0.0f, player.invincibleTimer - dt);
    player.scorchTimer = std::max(0.0f, player.scorchTimer - dt);
    if (player.scorchTimer > 0 && randomUnit(game) < 0.3f)   // a scorched ship trails flames
        spawnParticles(game, player.x + SHIP_W / 2.0f, player.y + SHIP_H / 2.0f, 1, paletteColor('O'), 20);
    bool freshFire = a.fire && !player.fireWasHeld;
    player.fireWasHeld = a.fire;
    if (player.stunTimer > 0 || player.frozenTimer > 0 || player.hookedTimer > 0) {   // dizzy, frozen or hooked: no moving, shooting or rockets
        player.stunTimer = std::max(0.0f, player.stunTimer - dt);
        player.frozenTimer = std::max(0.0f, player.frozenTimer - dt - (freshFire ? FREEZE_MASH : 0));   // mashing R1 cracks the ice
        if (player.hookedTimer > 0) {   // the hook lifts the ship, then lets go
            player.hookedTimer = std::max(0.0f, player.hookedTimer - dt);
            player.y = std::max(player.hookLiftTo, player.y - HOOK_LIFT_SPEED * dt);
        }
        player.vx = player.vy = 0;
        player.fireTimer = std::max(0.0f, player.fireTimer - dt);
        player.rocketPressedAt = -1;
        return;
    }
    float speed = SHIP_SPEED * (player.scorchTimer > 0 ? SCORCH_SPEED : 1);
    float targetX = a.moveX * speed, targetY = a.moveY * speed;
    if (game.level == 5) targetX += SEA_CURRENT * sinf(game.time * CURRENT_RATE);   // the deep sea's current pushes every ship
    float grip = game.level == 3 ? std::min(1.0f, ICE_GRIP * dt) : 1;   // the glacier's ice: the speed eases toward the stick's
    player.vx += (targetX - player.vx) * grip;
    player.vy += (targetY - player.vy) * grip;
    float vx = player.vx, vy = player.vy;
    if (player.slideTimer > 0) {   // slipping on a peel: the stick does nothing, the gun still works
        vx = player.slideVX * SLIDE_SPEED;
        vy = player.slideVY * SLIDE_SPEED;
        spawnParticles(game, player.x + SHIP_W / 2.0f, player.y + SHIP_H, 1, paletteColor('H'), 20);
        player.slideTimer -= dt;
        if (player.slideTimer <= 0) {
            player.slideTimer = 0;
            player.stunTimer = STUN_TIME;
            game.soundRequested[SOUND_STUN] = true;
        }
    }
    player.x = std::max(0.0f, std::min(player.x + vx * dt, (float)(SCREEN_W - SHIP_W)));
    player.y = std::max(MOVE_ZONE_TOP, std::min(player.y + vy * dt, (float)(SCREEN_H - SHIP_H)));
    if (player.slideTimer == 0 && player.stunTimer == 0) {
        for (Peel& peel : game.peels) {   // stepping on a peel starts a slide the way the stick points
            if (peel.life <= 0 || !overlap(peel.x, peel.y, PEEL_W, PEEL_H, player.x + HITBOX_X, player.y + HITBOX_Y, HITBOX, HITBOX)) continue;
            float length = sqrtf(a.moveX * a.moveX + a.moveY * a.moveY);
            player.slideVX = length < 0.2f ? (randomUnit(game) < 0.5f ? -1.0f : 1.0f) : a.moveX / length;
            player.slideVY = length < 0.2f ? 0 : a.moveY / length;
            player.slideTimer = SLIDE_TIME;
            peel.life = 0;   // squashed; removed in updateGame
            game.soundRequested[SOUND_SLIP] = true;
            break;
        }
    }

    // Keeps the fire rhythm exact while R1 is held; the first bullet leaves on the press step.
    player.fireTimer -= dt;
    if (a.fire && player.fireTimer <= 0) {
        float noseX = player.x + SHIP_W / 2 - 1, noseY = player.y - 4;
        if (player.weaponLevel == 1) {
            game.bullets.push_back({noseX, noseY, -BULLET_SPEED, index, 0});
        } else if (player.weaponLevel == 2) {
            game.bullets.push_back({noseX - DOUBLE_OFFSET, noseY, -BULLET_SPEED, index, 0});
            game.bullets.push_back({noseX + DOUBLE_OFFSET, noseY, -BULLET_SPEED, index, 0});
        } else {
            game.bullets.push_back({noseX, noseY, -BULLET_SPEED, index, -SPREAD_SPEED});
            game.bullets.push_back({noseX, noseY, -BULLET_SPEED, index, 0});
            game.bullets.push_back({noseX, noseY, -BULLET_SPEED, index, SPREAD_SPEED});
        }
        player.fireTimer += player.weaponLevel == MAX_WEAPON_LEVEL ? FAST_FIRE_INTERVAL : FIRE_INTERVAL;
        game.soundRequested[SOUND_SHOOT] = true;
    }
    if (player.fireTimer < 0) player.fireTimer = 0;

    // R2 is remembered for ROCKET_BUFFER, so a press just before the cooldown ends still fires.
    if (a.rocket) player.rocketPressedAt = game.time;
    if (player.rocketPressedAt >= 0 && game.time - player.rocketPressedAt > ROCKET_BUFFER) player.rocketPressedAt = -1;
    player.rocketCooldown = std::max(0.0f, player.rocketCooldown - dt);
    if (player.rocketPressedAt >= 0 && player.rocketCooldown == 0) {
        player.rocketPressedAt = -1;
        if (player.rockets > 0) {
            player.rockets--;
            player.rocketCooldown = ROCKET_COOLDOWN;
            Rocket rocket = {player.x + SHIP_W / 2 - 2, player.y - 8, ROCKET_START_SPEED, index};
            game.rockets.push_back(rocket);
            game.soundRequested[SOUND_ROCKET_LAUNCH] = true;
        } else {
            game.soundRequested[SOUND_ROCKET_EMPTY] = true;
        }
    }
}

static void updateAlien(Game& game, Alien& alien, float dt) {
    alien.flash = std::max(0.0f, alien.flash - dt);
    float formationX = alien.slotX + game.swayOffset;
    switch (alien.state) {
        case ALIEN_ENTERING:
            alien.x = formationX;
            if (alien.entryDelay > 0) {
                alien.entryDelay -= dt;
                break;
            }
            alien.y += ENTRY_SPEED * dt;
            if (alien.y >= alien.slotY) {
                alien.y = alien.slotY;
                alien.state = alien.type == ALIEN_HEAVY ? ALIEN_ROAMING : ALIEN_IN_FORMATION;
            }
            break;
        case ALIEN_IN_FORMATION:
            alien.x = formationX;
            alien.y = alien.slotY;
            break;
        case ALIEN_DIVING: {
            // Steers toward the nearest living ship; with none alive, it heads back (enemy-waves edge case).
            const Player* target = NULL;
            for (const Player& p : game.players)
                if (p.alive && (!target || fabsf(p.x - alien.x) < fabsf(target->x - alien.x))) target = &p;
            if (!target) {
                alien.state = ALIEN_RETURNING;
                break;
            }
            float towards = target->x + SHIP_W / 2.0f - (alien.x + ALIEN_WIDTH[alien.type] / 2.0f);
            alien.vx += (towards > 0 ? 1 : -1) * DIVE_STEER * dt;
            alien.vx = std::max(-DIVE_MAX_SIDEWAYS, std::min(alien.vx, DIVE_MAX_SIDEWAYS));
            alien.x += alien.vx * dt;
            alien.y += DIVE_SPEED * dt;
            if (alien.y > SCREEN_H) {   // out the bottom, back in from the top
                alien.y = -ALIEN_HEIGHT[alien.type];
                alien.state = ALIEN_RETURNING;
            }
            break;
        }
        case ALIEN_RETURNING: {
            float dx = formationX - alien.x, dy = alien.slotY - alien.y, distance = sqrtf(dx * dx + dy * dy);
            if (distance <= RETURN_SPEED * dt) {
                alien.state = ALIEN_IN_FORMATION;
                alien.x = formationX;
                alien.y = alien.slotY;
            } else {
                alien.x += dx / distance * RETURN_SPEED * dt;
                alien.y += dy / distance * RETURN_SPEED * dt;
            }
            break;
        }
        case ALIEN_ROAMING:   // the heavy beetle bounces around its box
            alien.x += alien.vx * dt;
            alien.y += alien.vy * dt;
            if (alien.x < 0 || alien.x > SCREEN_W - ALIEN_WIDTH[ALIEN_HEAVY]) {
                alien.x = std::max(0.0f, std::min(alien.x, (float)(SCREEN_W - ALIEN_WIDTH[ALIEN_HEAVY])));
                alien.vx = -alien.vx;
            }
            if (alien.y < HEAVY_MIN_Y || alien.y > HEAVY_MAX_Y) {
                alien.y = std::max(HEAVY_MIN_Y, std::min(alien.y, HEAVY_MAX_Y));
                alien.vy = -alien.vy;
            }
            break;
    }
    int dw = difficultyWave(game.level, game.wave);
    bool canShoot = alien.state == ALIEN_IN_FORMATION || alien.state == ALIEN_DIVING || alien.state == ALIEN_ROAMING;
    float chance = shotChance(dw) * (alien.type == ALIEN_HEAVY ? HEAVY_SHOT_FACTOR : 1);
    if (canShoot && !game.over && randomUnit(game) < chance * dt) {
        float speed = enemyShotSpeed(dw) + SHOT_SPEED_BONUS[game.level - 1];
        Shot shot = {alien.x + ALIEN_WIDTH[alien.type] / 2 - 1, alien.y + ALIEN_HEIGHT[alien.type], speed, -1, 0};
        game.enemyShots.push_back(shot);
    }
}

// A thrown hazard from (x, y) toward the target's centre; x, y is where the sprite's centre starts.
static void throwAt(Game& game, ThrownKind kind, float x, float y, float tx, float ty, float speed) {
    float dx = tx - x, dy = ty - y, length = sqrtf(dx * dx + dy * dy) + 0.001f;
    Thrown t = {x - THROWN_W[kind] / 2.0f, y - THROWN_H[kind] / 2.0f, dx / length * speed, dy / length * speed, kind, 0, 0};
    game.thrown.push_back(t);
}

// A fan of `count` thrown hazards from (x, y): sideways -sideways..+sideways, all falling at `down`.
static void throwFan(Game& game, ThrownKind kind, float x, float y, int count, float sideways, float down) {
    for (int i = 0; i < count; i++) {
        float spread = (i - (count - 1) / 2.0f) / ((count - 1) / 2.0f);
        Thrown t = {x - THROWN_W[kind] / 2.0f, y, spread * sideways, down, kind, 0, 0};
        game.thrown.push_back(t);
    }
}

// The fisherman's hook: drops on its line, catches a ship, holds it, reels back up (deep-sea rules).
static void updateHook(Game& game, Boss& boss, const Player* target, float dt) {
    float rodX = boss.x + ROD_TIP_X, rodY = boss.y + ROD_TIP_Y;
    if (boss.hookState == HOOK_DOWN || boss.hookState == HOOK_WAIT) {
        if (boss.hookState == HOOK_DOWN) {
            boss.hookY += HOOK_DOWN_SPEED * dt;
            if (boss.hookY >= HOOK_BOTTOM) {
                boss.hookY = HOOK_BOTTOM;
                boss.hookState = HOOK_WAIT;
                boss.hookTimer = HOOK_WAIT_TIME;
            }
        } else if ((boss.hookTimer -= dt) <= 0) {
            boss.hookState = HOOK_REEL;
        }
        for (size_t i = 0; i < game.players.size() && boss.hookState != HOOK_REEL; i++) {
            Player& p = game.players[i];
            if (!p.alive || p.invincibleTimer > 0 || p.hookedTimer > 0) continue;
            if (!overlap(boss.hookX - 2, boss.hookY, 5, 6, p.x + 3, p.y + 2, 10, 10)) continue;
            p.hookedTimer = HOOK_STUN;
            p.hookLiftTo = std::max(MOVE_ZONE_TOP, p.y - HOOK_LIFT);
            p.slideTimer = 0;
            boss.hookState = HOOK_HOLD;
            boss.hookPlayer = (int)i;
            boss.hookCaught = true;
            boss.say = "GOT ONE!";
            boss.sayTimer = GOT_ONE_TIME;
            game.soundRequested[SOUND_HOOKED] = true;
            break;
        }
    } else if (boss.hookState == HOOK_HOLD) {
        const Player& p = game.players[boss.hookPlayer];
        if (!p.alive || p.hookedTimer == 0) {
            boss.hookState = HOOK_REEL;
        } else {
            boss.hookX = p.x + SHIP_W / 2.0f;
            boss.hookY = p.y - 6;
        }
    } else if (boss.hookState == HOOK_REEL) {
        boss.hookY -= HOOK_REEL_SPEED * dt;
        if (boss.hookY <= rodY) {
            boss.hookState = HOOK_NONE;
            if (!boss.hookCaught && target && randomUnit(game) < BOOT_CHANCE) {   // an empty cast brings up an old boot
                throwAt(game, THROWN_BOOT, rodX, rodY + 4, target->x + SHIP_W / 2.0f, target->y + SHIP_H / 2.0f, BOOT_SPEED);
                boss.say = "OLD BOOT";
                boss.sayTimer = BOOT_TEXT_TIME;
                game.soundRequested[SOUND_CAST] = true;
            }
        }
    }
}

// Flies in, sways in its range and attacks; holds new attacks with no living ship.
static void updateBoss(Game& game, Boss& boss, float dt) {
    boss.flash = std::max(0.0f, boss.flash - dt);
    boss.laughTimer = std::max(0.0f, boss.laughTimer - dt);
    boss.sayTimer = std::max(0.0f, boss.sayTimer - dt);
    if (boss.y < BOSS_STOP_Y) {
        boss.y = std::min(boss.y + ENTRY_SPEED * dt, BOSS_STOP_Y);
        return;
    }
    boss.x += boss.vx * dt;
    if (boss.x < boss.minX || boss.x > boss.maxX) {
        boss.x = std::max(boss.minX, std::min(boss.x, boss.maxX));
        boss.vx = -boss.vx;
    }
    if (boss.ballTimer > 0) {   // the mage's ball, or the fire lord's eruption warning
        boss.ballTimer -= dt;
        if (boss.ballTimer <= 0) {
            boss.ballTimer = 0;
            boss.beamTimer = boss.type == BOSS_FIRELORD ? ERUPTION_TIME : BEAM_TIME;
            feedback(game, boss.type == BOSS_FIRELORD ? TRAUMA_ERUPTION : TRAUMA_BEAM, 0, 0);
            game.soundRequested[SOUND_EXPLOSION] = true;
        }
    } else {
        boss.beamTimer = std::max(0.0f, boss.beamTimer - dt);
    }

    const Player* target = NULL;
    float centreX = boss.x + BOSS_W / 2.0f, bottomY = boss.y + BOSS_H;
    for (const Player& p : game.players)
        if (p.alive && (!target || fabsf(p.x - centreX) < fabsf(target->x - centreX))) target = &p;
    if (boss.windupTimer > 0) {
        boss.windupTimer -= dt;
        if (boss.windupTimer <= 0 && boss.type == BOSS_GORILLA) {   // the pound shakes coconuts loose: one in each fifth
            boss.windupTimer = 0;
            float lane = (SCREEN_W - THROWN_SIZE) / (float)COCONUTS;
            for (int i = 0; i < COCONUTS; i++)
                game.thrown.push_back({(i + randomUnit(game)) * lane, (float)-THROWN_SIZE, 0, COCONUT_SPEED, THROWN_COCONUT, 0, 0});
            feedback(game, TRAUMA_POUND, 0, 0);
        } else if (boss.windupTimer <= 0) {   // the yeti sneezes a fan of snowballs
            boss.windupTimer = 0;
            throwFan(game, THROWN_SNOWBALL, centreX, bottomY - 6, SNEEZE_SNOWBALLS, SNEEZE_SIDEWAYS, SNEEZE_DOWN_SPEED);
            boss.say = "ACHOO!";
            boss.sayTimer = SAY_TIME;
            feedback(game, TRAUMA_SNEEZE, 0, 0);
            game.soundRequested[SOUND_SNEEZE] = true;
        }
    }
    if (boss.hookState != HOOK_NONE) updateHook(game, boss, target, dt);
    if (!target) {
        boss.burstShotsLeft = 0;
        return;
    }

    bool jungle = boss.type == BOSS_MONKEY || boss.type == BOSS_GORILLA;
    bool glacier = boss.type == BOSS_YETI || boss.type == BOSS_GOLEM;
    bool volcano = boss.type == BOSS_IMP || boss.type == BOSS_FIRELORD;
    bool sea = boss.type == BOSS_ANGLER || boss.type == BOSS_FISHERMAN;
    float targetX = target->x + SHIP_W / 2.0f, targetY = target->y + SHIP_H / 2.0f;
    boss.attackTimer -= dt;
    if (boss.attackTimer <= 0) {
        // Shared attack ids: 0 aimed volley (shots, bananas, snowballs, fireballs, bubbles), 1 fan (or bunch), 2 power ball.
        // Level ids: 3 poop, 4 pound; 5 ice block, 6 sneeze, 7 two ice blocks, 8 icicle rain; 9 lava drip, 10 lava rain,
        // 11 eruption; 12 big bubble, 13 hook cast.
        int attack = boss.nextAttack;
        boss.attackTimer = bossAttackInterval(game.level, game.wave);
        bool big = boss.type == BOSS_MAGE || boss.type == BOSS_GORILLA || boss.type == BOSS_GOLEM ||
                   boss.type == BOSS_FIRELORD || boss.type == BOSS_FISHERMAN;   // random order and random gaps
        float roll = big ? randomUnit(game) : 0;
        if (boss.type == BOSS_CLASSIC || boss.type == BOSS_MONKEY) {
            boss.nextAttack = 1 - boss.nextAttack;
        } else if (boss.type == BOSS_YETI || boss.type == BOSS_IMP || boss.type == BOSS_ANGLER) {   // a cycle of three
            const int CYCLES[3][3] = {{0, 5, 6}, {0, 9, 1}, {0, 12, 1}};   // yeti, imp, angler
            attack = CYCLES[boss.type == BOSS_YETI ? 0 : boss.type == BOSS_IMP ? 1 : 2][boss.nextAttack];
            boss.nextAttack = (boss.nextAttack + 1) % 3;
        } else if (boss.type == BOSS_MAGE) {
            attack = roll < MAGE_BURST_CHANCE ? 0 : roll < MAGE_BURST_CHANCE + MAGE_FAN_CHANCE ? 1 : 2;
            if (attack == 2 && (boss.ballTimer > 0 || boss.beamTimer > 0)) attack = 1;
        } else if (boss.type == BOSS_GORILLA) {
            attack = roll < GORILLA_THROW_CHANCE ? 0 : roll < GORILLA_THROW_CHANCE + GORILLA_POOP_CHANCE ? 3
                   : roll < GORILLA_THROW_CHANCE + GORILLA_POOP_CHANCE + GORILLA_BUNCH_CHANCE ? 1 : 4;
            int stink = (int)game.mists.size();
            for (const Lob& lob : game.lobs) stink += lob.poop;
            if ((attack == 3 && stink >= MAX_MISTS) || (attack == 4 && boss.windupTimer > 0)) attack = 1;
        } else if (boss.type == BOSS_GOLEM) {
            attack = roll < GOLEM_VOLLEY_CHANCE ? 0 : roll < GOLEM_VOLLEY_CHANCE + GOLEM_BLOCKS_CHANCE ? 7
                   : roll < GOLEM_VOLLEY_CHANCE + GOLEM_BLOCKS_CHANCE + GOLEM_FAN_CHANCE ? 1 : 8;
            bool hanging = std::any_of(game.thrown.begin(), game.thrown.end(), [](const Thrown& t) { return t.delay > 0; });
            if (attack == 8 && hanging) attack = 1;
        } else if (boss.type == BOSS_FIRELORD) {
            attack = roll < FIRELORD_VOLLEY_CHANCE ? 0 : roll < FIRELORD_VOLLEY_CHANCE + FIRELORD_FAN_CHANCE ? 1
                   : roll < FIRELORD_VOLLEY_CHANCE + FIRELORD_FAN_CHANCE + FIRELORD_RAIN_CHANCE ? 10 : 11;
            if (attack == 11 && (boss.ballTimer > 0 || boss.beamTimer > 0)) attack = 1;
        } else {   // fisherman
            attack = roll < FISHER_STREAM_CHANCE ? 0 : roll < FISHER_STREAM_CHANCE + FISHER_BIG_CHANCE ? 12
                   : roll < FISHER_STREAM_CHANCE + FISHER_BIG_CHANCE + FISHER_FAN_CHANCE ? 1 : 13;
            if (attack == 13 && boss.hookState != HOOK_NONE) attack = 0;
        }
        if (big) boss.attackTimer *= MAGE_GAP_MIN + (MAGE_GAP_MAX - MAGE_GAP_MIN) * randomUnit(game);

        if (attack == 0) {
            boss.burstShotsLeft = sea ? STREAM_BUBBLES : BOSS_BURST_SHOTS;
            boss.burstTimer = 0;
            if (jungle) {   // the banana throw also lobs a peel into the ship zone
                float toX = 8 + randomUnit(game) * (SCREEN_W - 16);
                float toY = MOVE_ZONE_TOP + 7 + randomUnit(game) * (SCREEN_H - MOVE_ZONE_TOP - 14);
                game.lobs.push_back({centreX, bottomY, toX, toY, -PEEL_DELAY, false});
                game.soundRequested[boss.type == BOSS_MONKEY ? SOUND_SCREECH : SOUND_BANANA_THROW] = true;
            }
            if (glacier || volcano) game.soundRequested[SOUND_BANANA_THROW] = true;
            if (sea) game.soundRequested[SOUND_BUBBLE] = true;
        } else if (attack == 1) {
            if (jungle) {
                throwFan(game, THROWN_BANANA, centreX, bottomY, BUNCH_BANANAS, BUNCH_SIDEWAYS, BUNCH_DOWN_SPEED);
                game.soundRequested[SOUND_BANANA_THROW] = true;
            } else if (glacier) {
                throwFan(game, THROWN_SNOWBALL, centreX, bottomY, BOSS_FAN_SHOTS, SNOW_FAN_SIDEWAYS, SNOW_FAN_DOWN_SPEED);
                game.soundRequested[SOUND_BANANA_THROW] = true;
            } else if (volcano) {
                throwFan(game, THROWN_FIREBALL, centreX, bottomY, BOSS_FAN_SHOTS, FIRE_FAN_SIDEWAYS, FIRE_FAN_DOWN_SPEED);
                game.soundRequested[SOUND_BANANA_THROW] = true;
            } else if (sea) {
                throwFan(game, THROWN_BUBBLE, centreX, bottomY, BUBBLE_FAN, BUBBLE_FAN_SIDEWAYS, BUBBLE_FAN_DOWN_SPEED);
                game.soundRequested[SOUND_BUBBLE] = true;
            } else {
                for (int i = 0; i < BOSS_FAN_SHOTS; i++) {
                    float spread = (i - (BOSS_FAN_SHOTS - 1) / 2.0f) / ((BOSS_FAN_SHOTS - 1) / 2.0f);
                    game.enemyShots.push_back({centreX - 1, bottomY, BOSS_FAN_DOWN_SPEED, -1, spread * BOSS_FAN_SIDEWAYS});
                }
            }
        } else if (attack == 2) {
            boss.ballX = boss.x + WAND_TIP_X;
            boss.ballY = boss.y + WAND_TIP_Y;
            boss.ballTimer = BALL_CHARGE_TIME;
        } else if (attack == 3) {   // poop lobbed at the nearest ship
            float toX = std::max(8.0f, std::min(targetX, SCREEN_W - 8.0f));
            float toY = std::max(MOVE_ZONE_TOP + 7, std::min(targetY, SCREEN_H - 7.0f));
            game.lobs.push_back({centreX, bottomY, toX, toY, 0, true});
            game.soundRequested[SOUND_BANANA_THROW] = true;
        } else if (attack == 4) {
            boss.windupTimer = POUND_TIME;
            game.soundRequested[SOUND_POUND] = true;
        } else if (attack == 5 || attack == 7) {   // ice blocks: one at the nearest ship, the golem adds one anywhere
            throwAt(game, THROWN_ICE_BLOCK, centreX, bottomY, targetX, targetY, ICE_BLOCK_SPEED);
            if (attack == 7) {
                float toX = 8 + randomUnit(game) * (SCREEN_W - 16);
                float toY = MOVE_ZONE_TOP + 7 + randomUnit(game) * (SCREEN_H - MOVE_ZONE_TOP - 14);
                throwAt(game, THROWN_ICE_BLOCK, centreX, bottomY, toX, toY, ICE_BLOCK_SPEED);
            }
            game.soundRequested[SOUND_CRACK] = true;
        } else if (attack == 6) {
            boss.windupTimer = SNEEZE_WINDUP;
            boss.say = "AH...";
            boss.sayTimer = SNEEZE_WINDUP;
        } else if (attack == 8) {   // icicles hang shaking at the top, one in each sixth of the screen, then fall
            float lane = (SCREEN_W - THROWN_W[THROWN_ICICLE]) / (float)ICICLES;
            for (int i = 0; i < ICICLES; i++)
                game.thrown.push_back({(i + randomUnit(game)) * lane, 0, 0, ICICLE_SPEED, THROWN_ICICLE, ICICLE_HANG, 0});
            game.soundRequested[SOUND_CRACK] = true;
        } else if (attack == 9) {
            throwFan(game, THROWN_LAVA_DROP, centreX, bottomY, LAVA_DRIP_DROPS, LAVA_DRIP_SIDEWAYS, LAVA_DROP_SPEED);
        } else if (attack == 10) {   // lava rain: one drop in each fifth of the screen
            float lane = (SCREEN_W - THROWN_W[THROWN_LAVA_DROP]) / (float)LAVA_RAIN_DROPS;
            for (int i = 0; i < LAVA_RAIN_DROPS; i++)
                game.thrown.push_back({(i + randomUnit(game)) * lane, (float)-THROWN_H[THROWN_LAVA_DROP], 0, LAVA_RAIN_SPEED,
                                       THROWN_LAVA_DROP, 0, 0});
        } else if (attack == 11) {   // eruption under the nearest ship
            boss.ballX = std::max(ERUPTION_W / 2.0f, std::min(targetX, SCREEN_W - ERUPTION_W / 2.0f));
            boss.ballY = SCREEN_H;
            boss.ballTimer = ERUPTION_WARN;
        } else if (attack == 12) {
            throwAt(game, THROWN_BIG_BUBBLE, centreX, bottomY, targetX, targetY, BIG_BUBBLE_SPEED);
            game.soundRequested[SOUND_BUBBLE] = true;
        } else {   // 13: the hook drops at the nearest ship's x
            boss.hookState = HOOK_DOWN;
            boss.hookX = std::max(4.0f, std::min(targetX, SCREEN_W - 4.0f));
            boss.hookY = boss.y + ROD_TIP_Y;
            boss.hookCaught = false;
            game.soundRequested[SOUND_CAST] = true;
        }
    }
    if (boss.burstShotsLeft > 0) {
        boss.burstTimer -= dt;
        if (boss.burstTimer <= 0) {
            if (jungle) game.thrown.push_back({centreX - THROWN_SIZE / 2.0f, bottomY, 0, BANANA_SPEED, THROWN_BANANA, 0, 0});   // straight down
            else if (glacier) throwAt(game, THROWN_SNOWBALL, centreX, bottomY, targetX, targetY, SNOWBALL_SPEED);
            else if (volcano) throwAt(game, THROWN_FIREBALL, centreX, bottomY, targetX, targetY, FIREBALL_SPEED);
            else if (sea) throwAt(game, THROWN_BUBBLE, centreX, bottomY, targetX, targetY, BUBBLE_SPEED);
            else {
                float dx = targetX - centreX, dy = targetY - bottomY, length = sqrtf(dx * dx + dy * dy) + 0.001f;
                game.enemyShots.push_back({centreX - 1, bottomY, dy / length * BOSS_AIMED_SPEED, -1, dx / length * BOSS_AIMED_SPEED});
            }
            boss.burstShotsLeft--;
            boss.burstTimer = sea ? STREAM_GAP : BOSS_BURST_GAP;
        }
    }
}

static void startDive(Game& game) {
    std::vector<Alien*> candidates;
    int divers = 0;
    for (Alien& alien : game.aliens) {
        if (alien.alive && alien.state == ALIEN_DIVING) divers++;
        if (alien.alive && alien.state == ALIEN_IN_FORMATION) candidates.push_back(&alien);
    }
    if (divers >= (difficultyWave(game.level, game.wave) >= TWO_DIVERS_WAVE ? 2 : 1) || candidates.empty()) return;
    Alien* diver = candidates[(size_t)(randomUnit(game) * candidates.size())];
    diver->state = ALIEN_DIVING;
    diver->vx = 0;
}

void updateBackground(Game& game, float dt) {
    game.sceneryTime += dt;
    for (Star& star : game.stars) {
        star.y += STAR_SPEEDS[star.layer] * dt;
        if (star.y >= SCREEN_H) {
            star.y -= SCREEN_H;
            star.x = randomUnit(game) * SCREEN_W;
        }
    }
}

void updateGame(Game& game, const Actions* actions, float dt) {
    game.feelTime += dt;
    game.trauma = std::max(0.0f, game.trauma - TRAUMA_DECAY * dt);
    game.flashTimer = std::max(0.0f, game.flashTimer - dt);
    if (game.hitStop > 0) {   // frozen: game time stands still, so a recorded R2 press keeps its buffer
        game.hitStop -= dt;
        for (size_t i = 0; i < game.players.size(); i++)
            if (actions[i].rocket && game.players[i].alive) game.players[i].rocketPressedAt = game.time;
        return;
    }
    game.time += dt;
    bool anyPlayerAlive = false;
    for (const Player& p : game.players) anyPlayerAlive |= p.alive;

    updateBackground(game, dt);

    for (size_t i = 0; i < game.players.size(); i++) updatePlayer(game, game.players[i], (int)i, actions[i], dt);
    for (Player& player : game.players) {   // a ship whose centre is in a stink mist makes its bullets heal gorillas
        player.inMist = false;
        float cx = player.x + SHIP_W / 2.0f, cy = player.y + SHIP_H / 2.0f;
        for (const Mist& mist : game.mists) {
            float r = MIST_RADIUS * std::min(mist.age / MIST_GROW, 1.0f);
            if (player.alive && (cx - mist.x) * (cx - mist.x) + (cy - mist.y) * (cy - mist.y) <= r * r) player.inMist = true;
        }
    }

    int dw = difficultyWave(game.level, game.wave);
    float swaySpeed = std::min(20.0f + 2 * dw, 50.0f);
    game.swayOffset += game.swayDirection * swaySpeed * dt;
    if (fabsf(game.swayOffset) > SWAY_RANGE) {
        game.swayOffset = game.swayOffset > 0 ? SWAY_RANGE : -SWAY_RANGE;
        game.swayDirection = -game.swayDirection;
    }
    for (Alien& alien : game.aliens)
        if (alien.alive) updateAlien(game, alien, dt);
    for (Boss& boss : game.bosses) if (boss.alive) updateBoss(game, boss, dt);
    if (dw >= 3 && anyPlayerAlive && !game.over) {
        game.diveTimer -= dt;
        if (game.diveTimer <= 0) {
            game.diveTimer = diveInterval(dw);
            startDive(game);
        }
    }

    for (Shot& bullet : game.bullets) {
        bullet.x += bullet.vx * dt;
        bullet.y += bullet.vy * dt;
        int damage = game.players[bullet.owner].scorchTimer > 0 ? SCORCH_DAMAGE : 1;   // a scorched ship's bullets burn
        for (Alien& alien : game.aliens) {
            if (!alien.alive || alien.y + ALIEN_HEIGHT[alien.type] < 0 || !overlap(bullet.x, bullet.y, 2, 4, alien.x, alien.y, ALIEN_WIDTH[alien.type], ALIEN_HEIGHT[alien.type])) continue;
            damageAlien(game, alien, damage, bullet.owner);
            bullet.y = -100;   // spent; removed below
            break;
        }
        for (Boss& boss : game.bosses) {
            if (bullet.y <= -50 || !boss.alive || boss.y + BOSS_H < 0 || !overlap(bullet.x, bullet.y, 2, 4, boss.x, boss.y, BOSS_W, BOSS_H)) continue;
            if (boss.type == BOSS_GORILLA && game.players[bullet.owner].inMist) {   // stink mist: the shot heals
                boss.hp = std::min(boss.hp + 1, boss.maxHp);
                if (boss.laughTimer == 0) game.soundRequested[SOUND_LAUGH] = true;
                boss.laughTimer = LAUGH_TIME;
                spawnParticles(game, bullet.x + 1, bullet.y, 2, paletteColor('l'), 30);
            } else {
                damageBoss(game, boss, damage, bullet.owner);
            }
            bullet.y = -100;   // spent; removed below
        }
    }

    for (Rocket& rocket : game.rockets) {
        rocket.speed = std::min(rocket.speed + ROCKET_ACCEL * dt, ROCKET_TOP_SPEED);
        rocket.y -= rocket.speed * dt;
        bool hit = rocket.y < 0;
        for (const Alien& alien : game.aliens)
            if (alien.alive && alien.y + ALIEN_HEIGHT[alien.type] >= 0 && overlap(rocket.x, rocket.y, 4, 8, alien.x, alien.y, ALIEN_WIDTH[alien.type], ALIEN_HEIGHT[alien.type])) hit = true;
        for (const Boss& boss : game.bosses)
            if (boss.alive && boss.y + BOSS_H >= 0 && overlap(rocket.x, rocket.y, 4, 8, boss.x, boss.y, BOSS_W, BOSS_H)) hit = true;
        if (hit) {
            explode(game, rocket.x + 2, std::max(rocket.y, 0.0f) + 4, rocket.owner);
            rocket.y = -1000;   // spent; removed below
        }
    }

    for (Shot& shot : game.enemyShots) {
        shot.x += shot.vx * dt;
        shot.y += shot.vy * dt;
    }
    for (Thrown& t : game.thrown) {
        if (t.delay > 0) {   // an icicle still hangs
            t.delay -= dt;
            continue;
        }
        t.age += dt;
        t.x += t.vx * dt;
        t.y += t.vy * dt;
        if (t.kind == THROWN_BUBBLE) t.x += BUBBLE_WOBBLE * cosf(t.age * 7) * dt;
    }
    for (size_t i = 0; i < game.thrown.size(); i++) {   // a big bubble pops into a ring of small ones (by index: this adds)
        Thrown big = game.thrown[i];
        if (big.kind != THROWN_BIG_BUBBLE || (big.age < BIG_BUBBLE_POP_TIME && big.y < BIG_BUBBLE_POP_Y)) continue;
        game.thrown[i].y = SCREEN_H + 100;   // popped; removed below
        float cx = big.x + THROWN_W[THROWN_BIG_BUBBLE] / 2.0f, cy = big.y + THROWN_H[THROWN_BIG_BUBBLE] / 2.0f;
        for (int k = 0; k < POP_BUBBLES; k++) {
            float angle = k * 6.2831853f / POP_BUBBLES;
            game.thrown.push_back({cx - 2.5f, cy - 2.5f, cosf(angle) * POP_SPEED, sinf(angle) * POP_SPEED, THROWN_BUBBLE, 0, 0});
        }
        game.soundRequested[SOUND_BUBBLE] = true;
    }
    for (Lob& lob : game.lobs) {
        lob.age += dt;
        if (lob.age < LOB_TIME) continue;
        if (lob.poop) {   // splats into a stink mist
            game.mists.push_back({lob.toX, lob.toY, 0});
            spawnParticles(game, lob.toX, lob.toY, 6, paletteColor('h'), 30);
            game.soundRequested[SOUND_SPLAT] = true;
        } else {
            game.peels.push_back({lob.toX - PEEL_W / 2.0f, lob.toY - PEEL_H / 2.0f, PEEL_LIFE});
        }
    }
    for (Peel& peel : game.peels) peel.life -= dt;
    for (Mist& mist : game.mists) mist.age += dt;
    for (Pickup& pickup : game.pickups) pickup.y += PICKUP_FALL_SPEED * dt;

    for (Player& player : game.players) {
        if (!player.alive) continue;
        float hx = player.x + HITBOX_X, hy = player.y + HITBOX_Y;
        for (Thrown& t : game.thrown) {   // ice blocks freeze and lava drops scorch, even a blinking ship; neither kills
            if (t.kind != THROWN_ICE_BLOCK && t.kind != THROWN_LAVA_DROP) continue;
            if (!overlap(t.x + 1, t.y + 1, THROWN_W[t.kind] - 2, THROWN_H[t.kind] - 2, hx, hy, HITBOX, HITBOX)) continue;
            if (t.kind == THROWN_ICE_BLOCK) {
                player.frozenTimer = FREEZE_TIME;
                player.slideTimer = 0;
                spawnParticles(game, t.x + 4, t.y + 4, 8, paletteColor('i'), 40);
                game.soundRequested[SOUND_FREEZE] = true;
            } else {
                player.scorchTimer = SCORCH_TIME;
                spawnParticles(game, t.x + 2, t.y + 3, 6, paletteColor('O'), 30);
                game.soundRequested[SOUND_SIZZLE] = true;
            }
            t.y = SCREEN_H + 100;   // spent; removed below
        }
        if (player.invincibleTimer == 0) {
            bool hit = false;
            for (Shot& shot : game.enemyShots) {
                if (!hit && overlap(shot.x, shot.y, 2, 4, hx, hy, HITBOX, HITBOX)) {
                    hit = true;
                    shot.y = SCREEN_H + 100;   // spent; removed below
                }
            }
            for (Thrown& t : game.thrown) {   // the deadly box is 1 px inside the sprite
                if (t.kind == THROWN_ICE_BLOCK || t.kind == THROWN_LAVA_DROP) continue;
                if (!hit && overlap(t.x + 1, t.y + 1, THROWN_W[t.kind] - 2, THROWN_H[t.kind] - 2, hx, hy, HITBOX, HITBOX)) {
                    hit = true;
                    t.y = SCREEN_H + 100;   // spent; removed below
                }
            }
            for (const Alien& alien : game.aliens)
                if (alien.alive && overlap(alien.x, alien.y, ALIEN_WIDTH[alien.type], ALIEN_HEIGHT[alien.type], hx, hy, HITBOX, HITBOX)) hit = true;
            for (const Boss& boss : game.bosses) {
                if (boss.alive && overlap(boss.x, boss.y, BOSS_W, BOSS_H, hx, hy, HITBOX, HITBOX)) hit = true;
                int beamW = boss.type == BOSS_FIRELORD ? ERUPTION_W : BEAM_W;   // the mage's beam or the fire lord's eruption
                if (boss.alive && boss.beamTimer > 0 && overlap(boss.ballX - beamW / 2.0f, 0, beamW, SCREEN_H, hx, hy, HITBOX, HITBOX)) hit = true;
            }
            if (hit) {
                killPlayer(game, player);
                continue;
            }
        }
        for (Pickup& pickup : game.pickups) {
            if (!overlap(pickup.x, pickup.y, 8, 8, player.x, player.y, SHIP_W, SHIP_H)) continue;
            if (pickup.type == PICKUP_ROCKET) player.rockets = std::min(player.rockets + 1, MAX_ROCKETS);
            else if (player.weaponLevel < MAX_WEAPON_LEVEL) player.weaponLevel++;
            else addScore(game, player, UPGRADE_MAX_POINTS);
            spawnParticles(game, pickup.x + 4, pickup.y + 4, PICKUP_PARTICLES, paletteColor(pickup.type == PICKUP_ROCKET ? 'p' : 'y'), 30);
            pickup.y = SCREEN_H + 100;   // collected; removed below
            game.soundRequested[SOUND_PICKUP] = true;
        }
    }

    for (Explosion& explosion : game.explosions) explosion.age += dt;
    for (Particle& p : game.particles) {
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.life -= dt;
    }

    game.bullets.erase(std::remove_if(game.bullets.begin(), game.bullets.end(),
                                      [](const Shot& s) { return s.y < -8; }), game.bullets.end());
    game.enemyShots.erase(std::remove_if(game.enemyShots.begin(), game.enemyShots.end(),
                                         [](const Shot& s) { return s.y > SCREEN_H; }), game.enemyShots.end());
    game.thrown.erase(std::remove_if(game.thrown.begin(), game.thrown.end(), [](const Thrown& t) {
                          return t.y > SCREEN_H || t.y < -20 || t.x < -20 || t.x > SCREEN_W + 20; }), game.thrown.end());
    game.lobs.erase(std::remove_if(game.lobs.begin(), game.lobs.end(),
                                   [](const Lob& l) { return l.age >= LOB_TIME; }), game.lobs.end());
    game.peels.erase(std::remove_if(game.peels.begin(), game.peels.end(),
                                    [](const Peel& p) { return p.life <= 0; }), game.peels.end());
    game.mists.erase(std::remove_if(game.mists.begin(), game.mists.end(),
                                    [](const Mist& m) { return m.age >= MIST_TIME; }), game.mists.end());
    game.rockets.erase(std::remove_if(game.rockets.begin(), game.rockets.end(),
                                      [](const Rocket& r) { return r.y < -100; }), game.rockets.end());
    game.pickups.erase(std::remove_if(game.pickups.begin(), game.pickups.end(),
                                      [](const Pickup& p) { return p.y > SCREEN_H; }), game.pickups.end());
    game.explosions.erase(std::remove_if(game.explosions.begin(), game.explosions.end(),
                                         [](const Explosion& e) { return e.age > EXPLOSION_GROW + EXPLOSION_FADE; }),
                          game.explosions.end());
    game.particles.erase(std::remove_if(game.particles.begin(), game.particles.end(),
                                        [](const Particle& p) { return p.life <= 0; }), game.particles.end());

    game.bannerTimer = std::max(0.0f, game.bannerTimer - dt);
    if (game.over) return;

    // The next wave waits for a living ship (lives-and-score edge case).
    bool anyAlienAlive = std::any_of(game.bosses.begin(), game.bosses.end(), [](const Boss& b) { return b.alive; }) ||
                         std::any_of(game.aliens.begin(), game.aliens.end(), [](const Alien& a) { return a.alive; });
    if (!anyAlienAlive && game.nextWaveTimer == 0 && !game.levelClear) {
        for (Player& player : game.players)
            if (player.lives > 0) addScore(game, player, WAVE_CLEAR_BONUS * game.wave);
        if (game.wave == LEVEL_LAST_WAVE) {   // the level boss is dead (level-1 rule 4)
            game.levelClear = true;
            game.clearTimer = LEVEL_CLEAR_TIME;
            game.soundRequested[SOUND_WAVE_START] = true;
        } else {
            game.nextWaveTimer = WAVE_PAUSE;
        }
    } else if (game.levelClear) {
        // The banner's end is a win only while someone has lives left; otherwise the game over below ends the run.
        game.clearTimer = std::max(0.0f, game.clearTimer - dt);
        bool livesLeft = std::any_of(game.players.begin(), game.players.end(), [](const Player& p) { return p.lives > 0; });
        if (game.clearTimer == 0 && livesLeft) {
            if (game.level < LEVEL_COUNT && !game.singleLevel) {   // a classic run continues into the next level with the same players
                game.level++;
                game.levelClear = false;
                game.thrown.clear();   // the last level's hazards stay behind
                game.lobs.clear();
                game.peels.clear();
                game.mists.clear();
                startWave(game, 1);
            } else {
                game.won = true;
                game.over = true;
                game.overTime = game.time;
                return;
            }
        }
    } else if (!anyAlienAlive && anyPlayerAlive) {
        game.nextWaveTimer -= dt;
        if (game.nextWaveTimer <= 0) startWave(game, game.wave + 1);
    }

    bool runOver = true;
    for (const Player& p : game.players) runOver &= !p.alive && p.lives == 0 && p.respawnTimer <= 0;
    if (runOver) {
        game.over = true;
        game.overTime = game.time;
    }
}

static void drawCentredText(int y, const char* text, uint32_t color, int scale) {
    drawText((SCREEN_W - textWidth(text, scale)) / 2, y, text, color, scale);
}

// Grows to full size, then dissolves pixel by pixel; white core, yellow middle, orange edge.
static void drawExplosion(const Explosion& e) {
    float radius = e.radius * std::min(e.age / EXPLOSION_GROW, 1.0f);
    float fade = e.age > EXPLOSION_GROW ? (e.age - EXPLOSION_GROW) / EXPLOSION_FADE : 0;
    int r = (int)radius;
    for (int dy = -r; dy <= r; dy++)
        for (int dx = -r; dx <= r; dx++) {
            float d = sqrtf((float)(dx * dx + dy * dy)) / (radius + 0.001f);
            int px = (int)e.x + dx, py = (int)e.y + dy;
            float noise = ((((unsigned)px * 73856093u) ^ ((unsigned)py * 19349663u)) & 255) / 256.0f;
            if (d > 1 || noise < fade) continue;
            fillRect(px, py, 1, 1, paletteColor(d < 0.35f ? 'w' : d < 0.65f ? 'y' : 'o'));
        }
}

static uint32_t sceneryFar[TILE_H][SCREEN_W], sceneryMid[TILE_H][SCREEN_W];   // the level's two layers; mid: 0 is see-through

// The scenery's own xorshift, so the tests' game.random stays untouched and every run draws the same picture.
static float sceneryUnit(uint32_t& seed) {
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;
    return (seed & 0xFFFFFF) / (float)0x1000000;
}

// A dithered blob (leaf cluster, rock, frost) in a scenery tile; y wraps around the tile.
static void ditherBlob(uint32_t tile[][SCREEN_W], int cx, int cy, float r, char ink, char light) {
    int ri = (int)r;
    for (int dy = -ri; dy <= ri; dy++)
        for (int dx = -ri; dx <= ri; dx++) {
            float d2 = (float)(dx * dx + dy * dy);
            int x = cx + dx, y = ((cy + dy) % TILE_H + TILE_H) % TILE_H;
            if (d2 > r * r || x < 0 || x >= SCREEN_W) continue;
            if ((x * 7 + y * 3) % 11 == 0 && d2 < r * r * 0.4f) tile[y][x] = paletteColor(light);
            else if ((x + y) % 2 == 0 || d2 < r * r * 0.3f) tile[y][x] = paletteColor(ink);
        }
}

// Fills the far tile with one colour and clears the near one.
static void clearScenery(char far) {
    for (int y = 0; y < TILE_H; y++)
        for (int x = 0; x < SCREEN_W; x++) {
            sceneryFar[y][x] = paletteColor(far);
            sceneryMid[y][x] = 0;
        }
}

// Draws the jungle's two scenery tiles, the same picture every run.
static void buildJungle() {
    uint32_t seed = 0x4A554E47;
    auto unit = [&seed]() { return sceneryUnit(seed); };
    const float TWO_PI = 6.2831853f;
    const int H = TILE_H;
    for (int y = 0; y < H; y++)
        for (int x = 0; x < SCREEN_W; x++) {
            sceneryFar[y][x] = paletteColor('J');
            sceneryMid[y][x] = 0;
        }
    // Far layer: six thin trunks with dark grooves, then leaf clusters.
    for (int i = 0; i < 6; i++) {
        float cx = 24 + i * 56 + (unit() - 0.5f) * 16;
        int width = 10 + (int)(unit() * 8);
        float phase = unit() * TWO_PI;
        for (int y = 0; y < H; y++) {
            int left = (int)(cx + 3 * sinf(TWO_PI * 2 * y / H + phase) - width / 2.0f);
            for (int x = std::max(left, 0); x < std::min(left + width, SCREEN_W); x++) {
                bool groove = (x - left) % 4 == 1 && (y / 3 + i) % 5 != 0;
                sceneryFar[y][x] = paletteColor(groove ? 'J' : 'j');
            }
        }
    }
    for (int i = 0; i < 70; i++) {
        int cx = (int)(unit() * SCREEN_W);
        int cy = (int)(unit() * H);
        float r = 5 + unit() * 9;
        ditherBlob(sceneryFar, cx, cy, r, 'f', 'F');
    }
    // Near layer: the two big edge trunks with knot holes.
    for (int side = 0; side < 2; side++) {
        float phase = unit() * TWO_PI;
        for (int y = 0; y < H; y++) {
            int width = (int)(26 + 6 * sinf(TWO_PI * 2 * y / H + phase) + 3 * sinf(TWO_PI * 5 * y / H + phase * 2));
            for (int i = 0; i < width; i++) {
                int x = side == 0 ? i : SCREEN_W - 1 - i;
                bool bark = (i + (y / 6) * 2) % 7 == 0;
                sceneryMid[y][x] = paletteColor(i >= width - 3 ? 'T' : bark ? 'J' : 't');
            }
        }
        for (int k = 0; k < 2; k++) {
            int hy = (int)(unit() * H);
            int hx = 8 + (int)(unit() * 10);
            if (side == 1) hx = SCREEN_W - 1 - hx;
            for (int dy = -4; dy <= 4; dy++)
                for (int dx = -2; dx <= 2; dx++)
                    if (dx * dx * 16 + dy * dy * 4 <= 64) sceneryMid[((hy + dy) % H + H) % H][hx + dx] = paletteColor('J');
        }
    }
    // Two rope bridges.
    for (int b = 0; b < 2; b++) {
        int y0 = 70 + b * 180;
        for (int x = 20; x < 300; x++) {
            int deck = y0 + (int)(12 * sinf(3.14159265f * (x - 20) / 280));
            sceneryMid[deck % H][x] = paletteColor('T');
            sceneryMid[(deck - 6) % H][x] = paletteColor('T');
            if (x % 5 != 0) {
                sceneryMid[(deck + 1) % H][x] = paletteColor('B');
                sceneryMid[(deck + 2) % H][x] = paletteColor('B');
            }
            if (x % 10 == 0)
                for (int y = deck - 5; y < deck; y++) sceneryMid[(y % H + H) % H][x] = paletteColor('T');
        }
    }
    // Hanging vines.
    for (int i = 0; i < 7; i++) {
        float x0 = 40 + unit() * 240;
        int top = (int)(unit() * H);
        int length = 30 + (int)(unit() * 50);
        float phase = unit() * TWO_PI;
        for (int t = 0; t < length; t++) {
            int x = (int)(x0 + 3 * sinf(t * 0.15f + phase)), y = (top + t) % H;
            sceneryMid[y][x] = paletteColor('f');
            if (t % 6 == 0) {
                sceneryMid[y][x - 1] = paletteColor('F');
                sceneryMid[y][x + 1] = paletteColor('F');
            }
        }
    }
    // Leaf clumps on the trunks' inner edges.
    for (int i = 0; i < 22; i++) {
        int cx = i % 2 == 0 ? 30 + (int)(unit() * 8) : SCREEN_W - 31 - (int)(unit() * 8);
        int cy = (int)(unit() * H);
        float r = 6 + unit() * 6;
        ditherBlob(sceneryMid, cx, cy, r, 'f', 'F');
    }
}

// Draws the glacier: dark ice-crystal pillars far away, jagged ice walls with shelves and icicles near.
static void buildGlacier() {
    uint32_t seed = 0x49434531;
    auto unit = [&seed]() { return sceneryUnit(seed); };
    const float TWO_PI = 6.2831853f;
    const int H = TILE_H;
    clearScenery('a');
    for (int i = 0; i < 9; i++) {   // crystal pillars with diagonal facets
        float cx = 18 + i * 36 + (unit() - 0.5f) * 14;
        int width = 8 + (int)(unit() * 10);
        float phase = unit() * TWO_PI;
        for (int y = 0; y < H; y++) {
            int w = width + (int)(2 * sinf(TWO_PI * 3 * y / H + phase)), left = (int)cx - w / 2;
            bool crack = (y + i * 17) % 47 == 0;
            for (int x = std::max(left, 0); x < std::min(left + w, SCREEN_W); x++) {
                int col = x - left;
                sceneryFar[y][x] = paletteColor(crack ? 'a' : col == 0 || (col + y / 5) % 6 == 0 ? 'e' : 'A');
            }
        }
    }
    for (int i = 0; i < 40; i++) {   // frost patches
        int cx = (int)(unit() * SCREEN_W);
        int cy = (int)(unit() * H);
        float r = 3 + unit() * 5;
        ditherBlob(sceneryFar, cx, cy, r, 'A', 'e');
    }
    for (int side = 0; side < 2; side++) {   // jagged ice walls
        float phase = unit() * TWO_PI;
        for (int y = 0; y < H; y++) {
            int width = (int)(18 + 5 * sinf(TWO_PI * 2 * y / H + phase) + 3 * sinf(TWO_PI * 7 * y / H + phase * 3));
            for (int i = 0; i < width; i++) {
                int x = side == 0 ? i : SCREEN_W - 1 - i;
                sceneryMid[y][x] = paletteColor(i >= width - 2 ? 'E' : (i + y / 3) % 5 == 0 ? 'e' : 'A');
            }
        }
    }
    for (int k = 0; k < 6; k++) {   // ice shelves from the walls, icicles hanging under them
        int y0 = 30 + k * 60, length = 30 + (int)(unit() * 30);
        for (int j = 0; j < length; j++) {
            int x = k % 2 == 0 ? j : SCREEN_W - 1 - j;
            sceneryMid[y0 % H][x] = paletteColor('E');
            for (int y = y0 + 1; y < y0 + 4; y++) sceneryMid[y % H][x] = paletteColor('e');
            if (j % 4 != 1) continue;
            int icicle = 3 + (int)(unit() * 7);
            for (int t = 0; t < icicle; t++) {
                sceneryMid[(y0 + 4 + t) % H][x] = paletteColor('E');
                if (t < icicle / 2 && x > 0 && x < SCREEN_W - 1) {
                    sceneryMid[(y0 + 4 + t) % H][x - 1] = paletteColor('e');
                    sceneryMid[(y0 + 4 + t) % H][x + 1] = paletteColor('e');
                }
            }
        }
    }
}

// Draws the volcano: rock, lava rivers and glowing cracks far away, rock walls with glowing seams near.
static void buildVolcano() {
    uint32_t seed = 0x4C415641;
    auto unit = [&seed]() { return sceneryUnit(seed); };
    const float TWO_PI = 6.2831853f;
    const int H = TILE_H;
    clearScenery('v');
    for (int i = 0; i < 70; i++) {   // rock blobs
        int cx = (int)(unit() * SCREEN_W);
        int cy = (int)(unit() * H);
        float r = 4 + unit() * 8;
        ditherBlob(sceneryFar, cx, cy, r, 'V', 'K');
    }
    for (int i = 0; i < 3; i++) {   // meandering lava rivers: a bright middle, glowing banks
        float x0 = 60 + i * 100 + (unit() - 0.5f) * 30;
        float phase = unit() * TWO_PI;
        float amplitude = 14 + unit() * 10;
        for (int y = 0; y < H; y++) {
            int cx = (int)(x0 + amplitude * sinf(TWO_PI * 2 * y / H + phase) + 4 * sinf(TWO_PI * 5 * y / H + phase * 2));
            for (int dx = -3; dx <= 3; dx++) {
                int x = cx + dx;
                if (x >= 0 && x < SCREEN_W) sceneryFar[y][x] = paletteColor(abs(dx) <= 1 ? 'C' : abs(dx) == 2 ? 'D' : 'K');
            }
        }
    }
    for (int i = 0; i < 40; i++) {   // glowing cracks: short random walks downward
        int x = (int)(unit() * SCREEN_W);
        int y = (int)(unit() * H);
        int length = 8 + (int)(unit() * 16);
        for (int t = 0; t < length; t++, y++) {
            x += unit() < 0.5f ? -1 : 1;
            if (x >= 0 && x < SCREEN_W) sceneryFar[y % H][x] = paletteColor('D');
        }
    }
    for (int side = 0; side < 2; side++) {   // rock walls lit by the lava along their inner edge
        float phase = unit() * TWO_PI;
        for (int y = 0; y < H; y++) {
            int width = (int)(20 + 5 * sinf(TWO_PI * 2 * y / H + phase) + 3 * sinf(TWO_PI * 6 * y / H + phase * 2));
            for (int i = 0; i < width; i++) {
                int x = side == 0 ? i : SCREEN_W - 1 - i;
                sceneryMid[y][x] = paletteColor(i == width - 1 ? 'D' : i == width - 2 ? 'K' : (i * 3 + y) % 9 == 0 ? 'K' : 'V');
            }
        }
        for (int k = 0; k < 6; k++) {   // glowing seams in the wall
            int y0 = (int)(unit() * H);
            int x0 = 4 + (int)(unit() * 10);
            int length = 10 + (int)(unit() * 15);
            for (int t = 0; t < length; t++) {
                int x = x0 + (t / 3) % 2;
                sceneryMid[(y0 + t) % H][side == 0 ? x : SCREEN_W - 1 - x] = paletteColor('D');
            }
        }
    }
}

// Draws the deep sea: light rays and distant rocks far away, rocks and swaying kelp near.
static void buildSea() {
    uint32_t seed = 0x53454121;
    auto unit = [&seed]() { return sceneryUnit(seed); };
    const float TWO_PI = 6.2831853f;
    const int H = TILE_H;
    clearScenery('1');
    for (int y = 0; y < H; y++)   // dithered light rays slanting down (90 px apart, seamless over the tile)
        for (int x = 0; x < SCREEN_W; x++)
            if ((x + y / 2) % 90 < 14 && (x + y) % 2 == 0) sceneryFar[y][x] = paletteColor('2');
    for (int i = 0; i < 30; i++) {   // distant rocks
        int cx = (int)(unit() * SCREEN_W);
        int cy = (int)(unit() * H);
        float r = 6 + unit() * 8;
        ditherBlob(sceneryFar, cx, cy, r, '2', '3');
    }
    for (int side = 0; side < 2; side++) {   // rock walls
        float phase = unit() * TWO_PI;
        for (int y = 0; y < H; y++) {
            int width = (int)(12 + 4 * sinf(TWO_PI * 2 * y / H + phase) + 2 * sinf(TWO_PI * 5 * y / H + phase * 2));
            for (int i = 0; i < width; i++) {
                int x = side == 0 ? i : SCREEN_W - 1 - i;
                sceneryMid[y][x] = paletteColor(i == width - 1 || (i + y / 4) % 7 == 0 ? '5' : '2');
            }
        }
    }
    for (int k = 0; k < 16; k++) {   // kelp stalks with leaves, near the walls
        int baseX = k % 2 == 0 ? 14 + (int)(unit() * 30) : SCREEN_W - 16 - (int)(unit() * 30);
        float phase = unit() * TWO_PI;
        for (int y = 0; y < H; y++) {
            int x = baseX + (int)(3 * sinf(TWO_PI * 5 * y / H + phase));
            sceneryMid[y][x] = sceneryMid[y][x + 1] = paletteColor('3');
            if ((y + k * 7) % 9 != 0) continue;
            for (int d = 1; d <= 3; d++) {
                int leafX = (y / 9 + k) % 2 == 0 ? x + 1 + d : x - d;
                if (leafX >= 0 && leafX < SCREEN_W) sceneryMid[(y + d / 2) % H][leafX] = paletteColor('4');
            }
        }
    }
}

void drawBackground(const Game& game) {
    if (game.level >= 2) {
        static int built = 0;   // the level whose scenery is in the tiles
        if (built != game.level) {
            if (game.level == 2) buildJungle();
            else if (game.level == 3) buildGlacier();
            else if (game.level == 4) buildVolcano();
            else buildSea();
            built = game.level;
        }
        int farOffset = (int)(game.sceneryTime * FAR_SPEED[game.level - 1]) % TILE_H;
        int midOffset = (int)(game.sceneryTime * MID_SPEED[game.level - 1]) % TILE_H;
        for (int y = 0; y < SCREEN_H; y++) {   // two layers scrolling down; the near one is drawn where it isn't 0
            const uint32_t* far = sceneryFar[(y - farOffset + TILE_H) % TILE_H];
            const uint32_t* mid = sceneryMid[(y - midOffset + TILE_H) % TILE_H];
            uint32_t* row = screenPixels + y * SCREEN_W;
            for (int x = 0; x < SCREEN_W; x++) row[x] = mid[x] ? mid[x] : far[x];
        }
        // Dim drifting specks (fireflies, snow, embers, bubbles): slow and in background colours, never shot-like.
        float t = game.sceneryTime;
        if (game.level == 2) {
            for (int i = 0; i < FIREFLIES; i++) {
                if (sinf(t * 2.5f + i * 1.7f) <= 0.2f) continue;
                int x = (int)((i * 97 + 13) % SCREEN_W + 6 * sinf(t * 0.5f + i));
                int y = (int)fmodf(i * 53 + 29 + t * FIREFLY_SPEED + 4 * sinf(t * 0.7f + i * 1.3f), (float)SCREEN_H);
                fillRect(x, y, 1, 1, paletteColor('L'));
            }
        } else if (game.level == 3) {
            for (int i = 0; i < SNOWFLAKES; i++) {
                int x = (int)((i * 61 + 7) % SCREEN_W + 5 * sinf(t * 0.6f + i));
                int y = (int)fmodf(i * 37 + 11 + t * SNOW_SPEED * (0.7f + 0.3f * (i % 3)), (float)SCREEN_H);
                fillRect(x, y, 1, 1, paletteColor('E'));
            }
        } else if (game.level == 4) {
            for (int i = 0; i < EMBERS; i++) {
                if (sinf(t * 6 + i * 2.3f) < -0.3f) continue;   // flicker
                int x = (int)((i * 83 + 29) % SCREEN_W + 3 * sinf(t * 1.3f + i));
                int y = SCREEN_H - 1 - (int)fmodf(i * 41 + 17 + t * EMBER_RISE_SPEED, (float)SCREEN_H);
                fillRect(x, y, 1, 1, paletteColor('C'));
            }
        } else {
            float drift = -SEA_CURRENT / CURRENT_RATE * cosf(CURRENT_RATE * game.time);   // where the current has carried them
            for (int i = 0; i < SEA_BUBBLES; i++) {
                int x = (int)(fmodf(i * 113 + 41 + drift + 2 * sinf(t * 2 + i) + 4 * SCREEN_W, (float)SCREEN_W));
                int y = SCREEN_H - 1 - (int)fmodf(i * 67 + 5 + t * SEA_BUBBLE_SPEED, (float)SCREEN_H);
                fillRect(x - 1, y, 1, 1, paletteColor('5'));
                fillRect(x + 1, y, 1, 1, paletteColor('5'));
                fillRect(x, y - 1, 1, 1, paletteColor('5'));
                fillRect(x, y + 1, 1, 1, paletteColor('5'));
            }
        }
        return;
    }
    fillRect(0, 0, SCREEN_W, SCREEN_H, paletteColor('k'));
    for (const Star& star : game.stars) fillRect((int)star.x, (int)star.y, 1, 1, paletteColor(STAR_COLORS[star.layer]));
}

void drawGame(const Game& game) {
    drawBackground(game);

    int frame = (int)(game.time / ANIMATION_FRAME) % 2;
    for (const Mist& mist : game.mists) {   // a dithered sickly-green circle that grows, shimmers and thins out
        float r = MIST_RADIUS * std::min(mist.age / MIST_GROW, 1.0f);
        int shimmer = (int)(game.time * 4) % 2;
        bool fading = mist.age > MIST_TIME - MIST_FADE;
        for (int dy = -(int)r; dy <= (int)r; dy++)
            for (int dx = -(int)r; dx <= (int)r; dx++) {
                float d2 = (float)(dx * dx + dy * dy);
                int px = (int)mist.x + dx, py = (int)mist.y + dy;
                if (d2 > r * r || px < 0 || py < 0) continue;
                bool edge = d2 > r * r * 0.55f;
                if ((edge || fading) ? (px + 2 * py + shimmer) % 4 == 0 : (px + py + shimmer) % 2 == 0)
                    fillRect(px, py, 1, 1, paletteColor('M'));
            }
    }
    for (const Peel& peel : game.peels) {
        if (peel.life <= PEEL_BLINK && fmodf(peel.life, 0.2f) < 0.1f) continue;
        drawSprite(PEEL, (int)peel.x, (int)peel.y);
    }
    for (const Alien& alien : game.aliens) {
        if (!alien.alive) continue;
        const Sprite& sprite = *ALIEN_SPRITES[game.level - 1][alien.type][frame];
        if (alien.flash > 0) drawSpriteSolid(sprite, (int)alien.x, (int)alien.y, paletteColor('w'));
        else drawSprite(sprite, (int)alien.x, (int)alien.y);
    }
    for (const Boss& boss : game.bosses) {
        if (!boss.alive) continue;
        const Sprite* sprite = frame ? &BOSS_B : &BOSS_A;
        switch (boss.type) {
            case BOSS_CLASSIC: break;
            case BOSS_MAGE: sprite = frame ? &MAGE_B : &MAGE_A; break;
            case BOSS_MONKEY: sprite = frame ? &MONKEY_B : &MONKEY_A; break;
            case BOSS_GORILLA:
                sprite = boss.laughTimer > 0 ? &GORILLA_LAUGH
                       : boss.windupTimer > 0 ? ((int)(boss.windupTimer * 10) % 2 ? &GORILLA_POUND : &GORILLA_A)
                       : frame ? &GORILLA_B : &GORILLA_A;
                break;
            case BOSS_YETI: sprite = boss.windupTimer > 0 ? &YETI_SNEEZE : frame ? &YETI_B : &YETI_A; break;
            case BOSS_GOLEM: sprite = frame ? &GOLEM_B : &GOLEM_A; break;
            case BOSS_IMP: sprite = frame ? &IMP_B : &IMP_A; break;
            case BOSS_FIRELORD: sprite = frame ? &FIRELORD_B : &FIRELORD_A; break;
            case BOSS_ANGLER: sprite = frame ? &ANGLER_B : &ANGLER_A; break;
            case BOSS_FISHERMAN: sprite = frame ? &FISHERMAN_B : &FISHERMAN_A; break;
        }
        if (boss.flash > 0) drawSpriteSolid(*sprite, (int)boss.x, (int)boss.y, paletteColor('w'));
        else drawSprite(*sprite, (int)boss.x, (int)boss.y);
    }
    for (const Pickup& pickup : game.pickups)
        drawSprite(pickup.type == PICKUP_ROCKET ? ROCKET_PICKUP : UPGRADE_PICKUP, (int)pickup.x, (int)pickup.y);
    for (const Shot& bullet : game.bullets) {
        const Player& owner = game.players[bullet.owner];
        if (owner.inMist) {
            fillRect((int)bullet.x, (int)bullet.y, 2, 4, paletteColor('l'));   // heals gorillas
        } else if (owner.scorchTimer > 0) {   // fire bolts: double damage
            fillRect((int)bullet.x, (int)bullet.y, 2, 4, paletteColor('O'));
            fillRect((int)bullet.x, (int)bullet.y, 2, 1, paletteColor('N'));
        } else {
            drawSprite(BULLET, (int)bullet.x, (int)bullet.y);
        }
    }
    for (const Rocket& rocket : game.rockets) drawSprite(ROCKET, (int)rocket.x, (int)rocket.y);
    for (size_t i = 0; i < game.players.size(); i++) {
        const Player& player = game.players[i];
        bool blinkOff = player.invincibleTimer > 0 && fmodf(player.invincibleTimer, 2 * BLINK_PERIOD) < BLINK_PERIOD;
        const Sprite& ship = i == 0 ? (frame ? SHIP_B : SHIP_A) : (frame ? SHIP2_B : SHIP2_A);
        bool shaking = player.slideTimer > 0 || player.stunTimer > 0 || player.hookedTimer > 0;
        if (player.alive && !blinkOff) drawSprite(ship, (int)player.x + (shaking && (int)(game.feelTime * 30) % 2 ? 1 : 0), (int)player.y);
        if (player.alive && player.frozenTimer > 0) drawSprite(ICE_CUBE, (int)player.x - 1, (int)player.y - 1);
        if (player.alive && player.scorchTimer > SCORCH_TIME - HOT_TEXT_TIME)
            drawText((int)player.x + SHIP_W / 2 - textWidth("HOT!") / 2, (int)player.y - 8, "HOT!", paletteColor('O'));
        if (player.alive && (player.stunTimer > 0 || player.hookedTimer > 0))
            for (int star = 0; star < 3; star++) {   // dizzy stars circle above the ship
                float a = game.feelTime * 8 + star * 2.09f;
                fillRect((int)(player.x + 8 + 6 * cosf(a)), (int)(player.y - 3 + 2 * sinf(a)), 1, 1, paletteColor('Y'));
            }
    }
    for (const Shot& shot : game.enemyShots) drawSprite(ENEMY_SHOT, (int)shot.x, (int)shot.y);
    for (const Thrown& t : game.thrown) {
        int shake = t.delay > 0 && (int)(game.time * 20) % 2 ? 1 : 0;   // a hanging icicle trembles
        drawSprite(*THROWN_SPRITES[t.kind][(int)(game.time * 10) % 2], (int)t.x + shake, (int)t.y);
    }
    for (const Lob& lob : game.lobs) {
        if (lob.age < 0) continue;
        float t = lob.age / LOB_TIME;
        float x = lob.fromX + (lob.toX - lob.fromX) * t, y = lob.fromY + (lob.toY - lob.fromY) * t - LOB_ARC * 4 * t * (1 - t);
        const Sprite& sprite = lob.poop ? POOP : PEEL;
        drawSprite(sprite, (int)x - sprite.w / 2, (int)y - sprite.h / 2);
        uint32_t mark = paletteColor(lob.poop ? 'M' : 'H');   // landing mark
        fillRect((int)lob.toX - 3, (int)lob.toY, 1, 1, mark);
        fillRect((int)lob.toX + 3, (int)lob.toY, 1, 1, mark);
        fillRect((int)lob.toX, (int)lob.toY - 2, 1, 1, mark);
        fillRect((int)lob.toX, (int)lob.toY + 2, 1, 1, mark);
    }
    for (const Boss& boss : game.bosses) {
        if (!boss.alive) continue;
        int bx = (int)boss.ballX, by = (int)boss.ballY;
        if (boss.laughTimer > 0) drawText((int)boss.x + 6, (int)boss.y + BOSS_H + 3, "HA HA", paletteColor('M'));
        if (boss.sayTimer > 0 && boss.say) {   // beside the head, clear of the attack below; on the side with room
            int sayX = boss.x + BOSS_W + 2 + textWidth(boss.say) <= SCREEN_W ? (int)boss.x + BOSS_W + 2 : (int)boss.x - 2 - textWidth(boss.say);
            drawText(sayX, (int)boss.y + 3, boss.say, paletteColor('w'));
        }
        if (boss.hookState != HOOK_NONE) {   // the fishing line from the rod tip, then the hook
            float rodX = boss.x + ROD_TIP_X, rodY = boss.y + ROD_TIP_Y, dx = boss.hookX - rodX, dy = boss.hookY - rodY;
            int steps = (int)std::max(fabsf(dx), fabsf(dy));
            for (int i = 0; i <= steps; i++)
                fillRect((int)(rodX + dx * i / std::max(steps, 1)), (int)(rodY + dy * i / std::max(steps, 1)), 1, 1, paletteColor('s'));
            drawSprite(HOOK, (int)boss.hookX - 2, (int)boss.hookY);
        }
        if (boss.type == BOSS_FIRELORD) {   // the eruption: a bubbling pool and a guide line, then a column of fire
            if (boss.ballTimer > 0) {
                for (int y = SCREEN_H - 8; y > 0; y -= 4) fillRect(bx, y, 1, 2, paletteColor('m'));
                fillRect(bx - ERUPTION_W / 2, SCREEN_H - 3, ERUPTION_W, 3, paletteColor('O'));
                for (int i = 0; i < 4; i++)
                    fillRect(bx - 6 + i * 4 + (int)(game.time * 12 + i) % 3, SCREEN_H - 4 - (int)(game.time * 9 + i * 2) % 3, 1, 1, paletteColor('N'));
            }
            if (boss.beamTimer > 0) {
                fillRect(bx - ERUPTION_W / 2, 0, ERUPTION_W, SCREEN_H, paletteColor('m'));
                fillRect(bx - ERUPTION_W / 2 + 1, 0, ERUPTION_W - 2, SCREEN_H, paletteColor('O'));
                fillRect(bx - 3, 0, 6, SCREEN_H, paletteColor('N'));
            }
            continue;
        }
        if (boss.ballTimer > 0) {
            for (int y = by + BALL_MAX_RADIUS + 2; y < SCREEN_H; y += 4) fillRect(bx, y, 1, 2, paletteColor('m'));   // guide line
            int radius = 1 + (int)((1 - boss.ballTimer / BALL_CHARGE_TIME) * (BALL_MAX_RADIUS - 1));
            fillCircle(bx, by, radius, paletteColor('m'));
            if (radius >= 3) fillCircle(bx, by, radius - 2, paletteColor('w'));
        }
        if (boss.beamTimer > 0) {
            fillRect(bx - BEAM_W / 2, 0, BEAM_W, SCREEN_H, paletteColor('m'));
            fillRect(bx - BEAM_CORE_W / 2, 0, BEAM_CORE_W, SCREEN_H, paletteColor('w'));
        }
    }
    for (const Explosion& explosion : game.explosions) drawExplosion(explosion);
    for (const Particle& p : game.particles) fillRect((int)p.x, (int)p.y, 1, 1, p.color);

    // Screen flash: the play field halfway to white; reduce flashing turns it off.
    if (game.flashTimer > 0 && !game.reduceFlashing)
        for (uint32_t& pixel : screenPixels) pixel = 0xFF000000 | (((pixel >> 1) & 0x7F7F7F) + 0x808080);

    // Shake: shift the play field picture by a whole pixel offset; the HUD below is drawn after, so it stays still.
    float shake = game.trauma * game.trauma * game.shakeStrength;
    int dx = (int)lroundf(SHAKE_X * shake * sinf(game.feelTime * 41)), dy = (int)lroundf(SHAKE_Y * shake * sinf(game.feelTime * 57));
    if (dx != 0 || dy != 0) {
        for (int i = 0; i < SCREEN_H; i++) {
            int y = dy > 0 ? SCREEN_H - 1 - i : i, sourceY = y - dy;   // copy order: a row is read before it is overwritten
            uint32_t* row = screenPixels + y * SCREEN_W;
            if (sourceY < 0 || sourceY >= SCREEN_H) {
                std::fill(row, row + SCREEN_W, paletteColor('k'));
                continue;
            }
            memmove(row + std::max(dx, 0), screenPixels + sourceY * SCREEN_W + std::max(-dx, 0), (SCREEN_W - abs(dx)) * sizeof(uint32_t));
            if (dx > 0) std::fill(row, row + dx, paletteColor('k'));
            if (dx < 0) std::fill(row + SCREEN_W + dx, row + SCREEN_W, paletteColor('k'));
        }
    }

    // HUD: the wave at the top centre; each player's score, spare lives and rockets on their side (1P left, 2P right).
    char text[48];
    const char* label = levelLabel(game.level);
    snprintf(text, sizeof text, "%s %d", label, game.wave);
    drawCentredText(2, text, paletteColor('w'), 1);
    snprintf(text, sizeof text, "HI %07d", game.highScore);
    drawCentredText(9, text, paletteColor('s'), 1);
    bool coOp = game.players.size() > 1;
    for (size_t p = 0; p < game.players.size(); p++) {
        const Player& player = game.players[p];
        bool right = p == 1;
        snprintf(text, sizeof text, "%s %07d", coOp ? (right ? "2P" : "1P") : "SCORE", player.score);
        drawText(right ? SCREEN_W - 2 - textWidth(text) : 2, 2, text, paletteColor('w'));
        for (int i = 0; i < player.lives - 1; i++) drawSprite(LIFE_ICON, right ? SCREEN_W - 9 - i * 9 : 2 + i * 9, 9);
        for (int i = 0; i < player.rockets; i++) drawSprite(ROCKET_ICON, right ? SCREEN_W - 5 - i * 5 : 2 + i * 5, SCREEN_H - 7);
    }
    int bossHp = 0, bossMaxHp = 0;
    bool anyBossAlive = false, anyLaughing = false;
    for (const Boss& boss : game.bosses) {
        anyLaughing |= boss.laughTimer > 0;
        bossHp += std::max(boss.hp, 0);
        bossMaxHp += boss.maxHp;
        anyBossAlive |= boss.alive;
    }
    if (anyBossAlive) {
        int barX = (SCREEN_W - BOSS_HEALTH_BAR_W) / 2;
        fillRect(barX, BOSS_HEALTH_BAR_Y, BOSS_HEALTH_BAR_W, 3, paletteColor('R'));
        fillRect(barX, BOSS_HEALTH_BAR_Y, BOSS_HEALTH_BAR_W * bossHp / bossMaxHp, 3, paletteColor(anyLaughing ? 'l' : 'r'));
    }
    if (!coOp && !game.over && !game.levelClear) drawText(SCREEN_W - 2 - textWidth("2P PRESS START"), 2, "2P PRESS START", paletteColor('s'));

    if (game.over) {
        fillRect(SCREEN_W / 2 - 72, 58, 144, 62, paletteColor('n'));   // panel so passing aliens don't hide the text
        fillRect(SCREEN_W / 2 - 71, 59, 142, 60, paletteColor('k'));
        snprintf(text, sizeof text, "%s CLEAR", levelName(game.level));
        drawCentredText(66, game.won ? text : "GAME OVER", paletteColor(game.won ? 'y' : 'r'), 2);
        if (coOp) snprintf(text, sizeof text, "1P %d   2P %d   %s %d", game.players[0].score, game.players[1].score, label, game.wave);
        else snprintf(text, sizeof text, "SCORE %d   %s %d", game.players[0].score, label, game.wave);
        drawCentredText(86, text, paletteColor('w'), 1);
        if (game.newHighScore) drawCentredText(96, "NEW HIGH SCORE", paletteColor('y'), 1);
        if (game.time - game.overTime > GAME_OVER_INPUT_LOCK) drawCentredText(108, "X PLAY AGAIN   O MENU", paletteColor('y'), 1);
    } else if (game.levelClear) {
        snprintf(text, sizeof text, "%s CLEAR", levelName(game.level));
        drawCentredText(84, text, paletteColor('y'), 2);
    } else if (game.bannerTimer > 0) {
        const char* banner = game.wave == 1 && game.bannerTimer > BANNER_TIME ? levelName(game.level) : isBossWave(game.wave) ? "BOSS" : "WAVE %d";
        snprintf(text, sizeof text, banner, game.wave);
        drawCentredText(84, text, paletteColor('y'), 2);
    }
}
