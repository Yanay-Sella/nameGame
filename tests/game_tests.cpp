// Game-rule tests that run on the Mac (make test); no PS3, SDL or test framework needed.
#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "game.h"
#include "formations.h"
#include "input.h"
#include "screen.h"
#include "settings.h"
#include "sprites.h"

static int failures = 0;

#define CHECK(condition)                                                     \
    do {                                                                     \
        if (!(condition)) {                                                  \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);      \
            failures++;                                                      \
        }                                                                    \
    } while (0)

const float DT = 1.0f / 60;
const Actions NO_INPUT = {};

// A run whose ship can't be hit, for tests that aren't about dying.
static void startSafeGame(Game& game, int level = 1) {
    startGame(game, 1, 7, level);
    game.players[0].invincibleTimer = 1e9f;
}

static void run(Game& game, const Actions& a, float seconds) {
    for (int step = 0; step < (int)(seconds * 60 + 0.5f); step++) updateGame(game, &a, DT);
}

// One alien parked in the formation at (x, y); the sway is frozen at 0.
static Alien parkedAlien(float x, float y) {
    Alien alien = {};
    alien.type = ALIEN_GREEN;
    alien.state = ALIEN_IN_FORMATION;
    alien.slotX = alien.x = x;
    alien.slotY = alien.y = y;
    alien.hp = 1;
    alien.alive = true;
    return alien;
}

static const Sprite* const ALL_SPRITES[] = {&SHIP_A, &SHIP_B, &SHIP2_A, &SHIP2_B, &GREEN_ALIEN_A, &GREEN_ALIEN_B, &BLUE_ALIEN_A, &BLUE_ALIEN_B,
                                            &TANK_SHIP, &ASSASSIN_SHIP, &PHASED_SHIP, &MAGE_SHIP, &HEAVY_ROCKET, &MAGE_ORB,
                                            &RED_ALIEN_A, &RED_ALIEN_B, &BOSS_A, &BOSS_B, &MAGE_A, &MAGE_B, &BULLET, &ENEMY_SHOT, &ROCKET, &ROCKET_PICKUP,
                                            &UPGRADE_PICKUP, &LIFE_ICON, &ROCKET_ICON, &LEAFHOPPER_A, &LEAFHOPPER_B, &BEETLE_A, &BEETLE_B,
                                            &STAG_A, &STAG_B, &RHINO_A, &RHINO_B, &MONKEY_A, &MONKEY_B, &GORILLA_A, &GORILLA_B,
                                            &GORILLA_LAUGH, &GORILLA_POUND, &BANANA_A, &BANANA_B, &COCONUT, &PEEL, &POOP,
                                            &FLAKE_A, &FLAKE_B, &ICEBAT_A, &ICEBAT_B, &FROSTSKULL_A, &FROSTSKULL_B, &YETICLOUD_A,
                                            &YETICLOUD_B, &YETI_A, &YETI_B, &YETI_SNEEZE, &GOLEM_A, &GOLEM_B, &EMBER_A, &EMBER_B,
                                            &FIREBAT_A, &FIREBAT_B, &MAGMASKULL_A, &MAGMASKULL_B, &PHOENIX_A, &PHOENIX_B, &IMP_A,
                                            &IMP_B, &FIRELORD_A, &FIRELORD_B, &PIRANHA_A, &PIRANHA_B, &PUFFER_A, &PUFFER_B,
                                            &LIONFISH_A, &LIONFISH_B, &SHARK_A, &SHARK_B, &ANGLER_A, &ANGLER_B, &FISHERMAN_A,
                                            &FISHERMAN_B, &SNOWBALL, &ICICLE, &FIREBALL_A, &FIREBALL_B, &BUBBLE, &BIG_BUBBLE, &BOOT,
                                            &ICE_BLOCK, &ICE_CUBE, &LAVA_DROP, &HOOK, &BIG_LAVA_DROP, &LIVE_FISH_L_A, &LIVE_FISH_L_B,
                                            &LIVE_FISH_R_A, &LIVE_FISH_R_B,
                                            &SHIP_JUNGLE_A, &SHIP_JUNGLE_B, &SHIP2_JUNGLE_A, &SHIP2_JUNGLE_B, &SHIP_GLACIER_A, &SHIP_GLACIER_B, &SHIP2_GLACIER_A, &SHIP2_GLACIER_B, &SHIP_VOLCANO_A, &SHIP_VOLCANO_B, &SHIP2_VOLCANO_A, &SHIP2_VOLCANO_B, &SHIP_SEA_A, &SHIP_SEA_B, &SHIP2_SEA_A, &SHIP2_SEA_B, &WAND, &WAND_ICE, &WAND_BUBBLE, &PLAYER_BANANA, &FIRE_BANANA, &FIRE_BULLET, &SHIELD_BUBBLE_3, &SHIELD_BUBBLE_2, &SHIELD_BUBBLE_1, &BUFF_ICON_SPACE, &BUFF_ICON_JUNGLE, &BUFF_ICON_GLACIER, &BUFF_ICON_VOLCANO, &BUFF_ICON_SEA};

static void spritesHaveTheirSize() {
    for (const Sprite* s : ALL_SPRITES) CHECK((int)strlen(s->pixels) == s->w * s->h);
}

static void onlyDeadlyShotsUseMagenta() {
    for (const Sprite* s : ALL_SPRITES)
        CHECK((s == &ENEMY_SHOT || s == &BANANA_A || s == &BANANA_B || s == &COCONUT || s == &SNOWBALL || s == &ICICLE ||
               s == &FIREBALL_A || s == &FIREBALL_B || s == &BUBBLE || s == &BIG_BUBBLE || s == &BOOT ||
               s == &LIVE_FISH_L_A || s == &LIVE_FISH_L_B || s == &LIVE_FISH_R_A || s == &LIVE_FISH_R_B) == (strchr(s->pixels, 'm') != NULL));
}

static void deadzoneIsRadialAndRescaled() {
    float x = 0.1f, y = 0.1f;   // length 0.14, inside 0.2
    applyDeadzone(x, y, 0.2f);
    CHECK(x == 0 && y == 0);
    x = 0.2001f, y = 0;   // just past the edge: almost no movement
    applyDeadzone(x, y, 0.2f);
    CHECK(x > 0 && x < 0.01f);
    x = 1, y = 1;   // full diagonal: capped at length 1, not faster than straight
    applyDeadzone(x, y, 0.2f);
    CHECK(fabsf(sqrtf(x * x + y * y) - 1) < 0.001f);
    x = 0, y = -1;
    applyDeadzone(x, y, 0.2f);
    CHECK(x == 0 && fabsf(y + 1) < 0.001f);
}

static void holdingFireShootsEvery012Seconds() {
    Game game;
    startSafeGame(game);
    Actions a = NO_INPUT;
    a.fire = true;
    int bullets = 0;
    for (int step = 0; step < 60; step++) {   // one second; early bullets leave the screen meanwhile, so count shots
        game.soundRequested[SOUND_SHOOT] = false;
        updateGame(game, &a, 1.0f / 60);
        if (game.soundRequested[SOUND_SHOOT]) bullets++;
    }
    CHECK(bullets == 9);   // at 0, 0.12, ... 0.96 s
}

static void shipStaysInsideMovementZone() {
    Game game;
    startSafeGame(game);
    Actions a = NO_INPUT;
    a.moveX = -1, a.moveY = -1;
    for (int step = 0; step < 300; step++) updateGame(game, &a, 1.0f / 60);
    CHECK(game.players[0].x == 0 && game.players[0].y == 100);
    a.moveX = 1, a.moveY = 1;
    for (int step = 0; step < 300; step++) updateGame(game, &a, 1.0f / 60);
    CHECK(game.players[0].x == SCREEN_W - 16 && game.players[0].y == SCREEN_H - 14);
}

static void formationGrowsToFiveRows() {
    CHECK(formationRows(1) == 2);
    CHECK(formationRows(2) == 3);
    CHECK(formationRows(4) == 4);
    CHECK(formationRows(6) == 5);
    CHECK(formationRows(19) == 5);
    Game game;
    startGame(game, 1, 7);
    CHECK(game.wave == 1 && game.aliens.size() == 16);
}

static void clearingAWaveStartsTheNextAfterThePause() {
    Game game;
    startSafeGame(game);
    for (Alien& alien : game.aliens) alien.alive = false;
    Actions a = NO_INPUT;
    updateGame(game, &a, 1.0f / 60);
    CHECK(game.players[0].score == 1000);   // wave 1 clear bonus
    for (int step = 0; step < 119; step++) updateGame(game, &a, 1.0f / 60);
    CHECK(game.wave == 1);
    updateGame(game, &a, 1.0f / 60);
    updateGame(game, &a, 1.0f / 60);
    CHECK(game.wave == 2 && game.aliens.size() == 24);
}

static void enemyFormulasFollowTheDesign() {
    CHECK(fabsf(shotChance(1) - 0.025f) < 1e-6f);
    CHECK(fabsf(shotChance(12) - 0.08f) < 1e-6f);
    CHECK(fabsf(diveInterval(3) - 3.4f) < 1e-5f);
    CHECK(diveInterval(15) == 1.0f && diveInterval(40) == 1.0f);
}

// Classic difficulty (enemy-waves rule 9): more HP and more movement up to wave 40.
static void lateDifficultyAddsHpAndMovement() {
    CHECK(maxDivers(14) == 1 && maxDivers(15) == 2 && maxDivers(30) == 3 && maxDivers(40) == 3);
    CHECK(alienHpBonus(14) == 0 && alienHpBonus(15) == 1 && alienHpBonus(25) == 2 && alienHpBonus(35) == 3 && alienHpBonus(40) == 3);
    CHECK(swaySpeed(1) == 22 && swaySpeed(15) == 50 && swaySpeed(30) == 80 && swaySpeed(40) == 80);
    CHECK(bobAmplitude(15) == 0 && bobAmplitude(16) == 1 && bobAmplitude(40) == 4);

    Game game;   // level 5 wave 1 plays as wave 21: every alien has 1 more HP, and the formation bobs
    startSafeGame(game, 5);
    for (const Alien& alien : game.aliens) CHECK(alien.hp == (alien.type == ALIEN_RED ? 5 : alien.type == ALIEN_BLUE ? 3 : 2));
    run(game, NO_INPUT, 6);
    float minOffset = 0, maxOffset = 0;
    for (int step = 0; step < 120; step++) {
        updateGame(game, &NO_INPUT, DT);
        for (const Alien& a : game.aliens) {   // any alien may be off diving at the time, so look at them all
            if (a.state != ALIEN_IN_FORMATION) continue;
            minOffset = std::min(minOffset, a.y - a.slotY);
            maxOffset = std::max(maxOffset, a.y - a.slotY);
        }
    }
    CHECK(minOffset < -1.2f && maxOffset > 1.2f && maxOffset <= bobAmplitude(21) + 0.01f);
}

// Wave patterns (enemy-waves rule 8): shapes and entries vary, fit the screen and settle into formation.
static void wavesUseRandomPatterns() {
    bool shapes[SHAPE_COUNT] = {}, entries[ENTRY_COUNT] = {};
    for (int level : {1, 5}) {   // 16 aliens, then 40
        for (uint32_t seed = 1; seed <= 150; seed++) {
            Game game;
            startGame(game, 1, seed, level);
            game.players[0].invincibleTimer = 1e9f;
            shapes[game.formationShape] = entries[game.entryStyle] = true;
            for (size_t i = 0; i < game.aliens.size(); i++) {
                const Alien& a = game.aliens[i];
                CHECK(a.type == (level == 1 ? ALIEN_GREEN : i < 16 ? ALIEN_RED : i < 32 ? ALIEN_BLUE : ALIEN_GREEN));   // the rows keep their types
                CHECK(a.slotX - 40 >= 0 && a.slotX + 40 + 12 <= SCREEN_W && a.slotY >= 18 && a.slotY + 10 + 4 <= 100);
            }
            run(game, NO_INPUT, 5);
            for (const Alien& a : game.aliens) CHECK(a.state != ALIEN_ENTERING);
        }
    }
    int seen = 0;
    for (bool shape : shapes) seen += shape;
    CHECK(seen >= 24);
    for (bool seen : entries) CHECK(seen);
}

// Every traced shape draws clearly with some alien count: slots in the box, apart, in rows of 8 from the top.
static void tracedShapesFit() {
    for (int shape = SHAPE_FIRST_TRACED; shape < SHAPE_COUNT; shape++) {
        bool fitsSomewhere = false;
        for (int count = 16; count <= 40; count += 8) {
            std::vector<Slot> slots;
            if (!traceFormation(shape, shape == SHAPE_WORD ? "NAME" : "ABC", count, slots)) continue;
            fitsSomewhere = true;
            CHECK((int)slots.size() == count);
            for (int i = 0; i < count; i++) {
                CHECK(slots[i].x >= 40 && slots[i].x + 12 <= SCREEN_W - 40 && slots[i].y >= 18 && slots[i].y + 10 + 4 <= 100);
                if (i % 8) CHECK(slots[i].x >= slots[i - 1].x);
                if (i >= 8 && i % 8 == 0) CHECK(slots[i].y >= slots[i - 1].y);   // a row starts below the one before it
                for (int j = 0; j < i; j++) CHECK(fabsf(slots[i].x - slots[j].x) >= 12 || fabsf(slots[i].y - slots[j].y) >= 10);
            }
        }
        CHECK(fitsSomewhere);
    }
}

static void aliensFlyInBeforeTheyCanShoot() {
    Game game;
    startSafeGame(game);
    CHECK(game.aliens[0].state == ALIEN_ENTERING && game.aliens[0].y < 0);
    run(game, NO_INPUT, 2.5f);
    for (const Alien& alien : game.aliens) CHECK(alien.state == ALIEN_IN_FORMATION);

    // Held in the entering state for 30 s at the highest shot chance: ~38 shots if entering aliens could fire.
    startSafeGame(game);
    game.wave = 12;
    for (Alien& alien : game.aliens) alien.entryDelay = 1e9f;
    bool shot = false;
    for (int step = 0; step < 30 * 60; step++) {
        updateGame(game, &NO_INPUT, DT);
        shot |= !game.enemyShots.empty();
    }
    CHECK(!shot);
}

static void divesStartAtWaveThree() {
    Game game;
    startSafeGame(game);
    run(game, NO_INPUT, 8);
    for (const Alien& alien : game.aliens) CHECK(alien.state != ALIEN_DIVING);
    game.wave = 3;
    game.diveTimer = 0.01f;
    run(game, NO_INPUT, 0.05f);
    int divers = 0;
    for (const Alien& alien : game.aliens) divers += alien.state == ALIEN_DIVING;
    CHECK(divers == 1);
}

static void enemyShotsSpeedUpAfterWaveTen() {
    CHECK(enemyShotSpeed(1) == 70);
    CHECK(enemyShotSpeed(10) == 70);
    CHECK(enemyShotSpeed(11) == 72);
    CHECK(enemyShotSpeed(20) == 90);
    CHECK(enemyShotSpeed(30) == 90);

    Game game;
    startSafeGame(game);
    run(game, NO_INPUT, 8);
    game.wave = 20;
    game.enemyShots.clear();
    for (int step = 0; step < 600 && game.enemyShots.empty(); step++) updateGame(game, &NO_INPUT, DT);
    CHECK(!game.enemyShots.empty());
    if (!game.enemyShots.empty()) CHECK(game.enemyShots[0].vy == 90);
}

static void lateWavesAddArmouredRows() {
    for (int wave : {13, 7}) {
        Game game;
        startSafeGame(game);
        game.wave = wave - 1;
        for (Alien& alien : game.aliens) alien.alive = false;
        run(game, NO_INPUT, 2.2f);
        CHECK(game.wave == wave);
        int redRows = wave == 13 ? 2 : 1, blueEnd = wave == 13 ? 4 : 2;
        CHECK(game.aliens.size() == 40);
        for (size_t i = 0; i < game.aliens.size() && i < 40; i++) {
            int row = (int)i / 8;
            AlienType expected = row < redRows ? ALIEN_RED : row < blueEnd ? ALIEN_BLUE : ALIEN_GREEN;
            CHECK(game.aliens[i].type == expected);
        }
    }
}

static void twoAliensDiveFromWaveFifteen() {
    Game game;
    startSafeGame(game);
    run(game, NO_INPUT, 8);
    for (int wave : {14, 16}) {
        game.wave = wave;
        for (int i = 0; i < 3; i++) {
            game.diveTimer = 0.01f;
            run(game, NO_INPUT, 0.05f);
        }
        int divers = 0;
        for (const Alien& alien : game.aliens) divers += alien.alive && alien.state == ALIEN_DIVING;
        CHECK(divers == (wave == 14 ? 1 : 2));
    }
}

static void rocketBufferFiresAnEarlyPress() {
    Game game;
    startSafeGame(game);
    Actions press = NO_INPUT;
    press.rocket = true;
    game.players[0].rocketCooldown = 0.1f;   // pressed 0.1 s early: inside the 0.15 s buffer
    updateGame(game, &press, DT);
    run(game, NO_INPUT, 0.2f);
    CHECK(game.players[0].rockets == 2);
    game.players[0].rocketCooldown = 0.3f;   // pressed 0.3 s early: the press expires
    updateGame(game, &press, DT);
    run(game, NO_INPUT, 0.5f);
    CHECK(game.players[0].rockets == 2);
}

static void emptyRocketClicks() {
    Game game;
    startSafeGame(game);
    game.players[0].rockets = 0;
    Actions press = NO_INPUT;
    press.rocket = true;
    updateGame(game, &press, DT);
    CHECK(game.soundRequested[SOUND_ROCKET_EMPTY] && game.rockets.empty());
}

static void blastKillsOnlyInsideItsRadius() {
    Game game;
    startSafeGame(game);
    game.aliens.clear();
    game.aliens.push_back(parkedAlien(100, 40));   // hit directly
    game.aliens.push_back(parkedAlien(130, 40));   // 30 px away: inside the 40 px blast
    game.aliens.push_back(parkedAlien(160, 40));   // 60 px away: outside
    game.swayDirection = 0;
    Rocket rocket = {104, 60, 60, 0};
    game.rockets.push_back(rocket);
    run(game, NO_INPUT, 0.5f);
    CHECK(!game.aliens[0].alive && !game.aliens[1].alive && game.aliens[2].alive);
    CHECK(game.players[0].score == 200);
}

static void extraLivesAtEvery50000() {
    Game game;
    startSafeGame(game);
    game.players[0].score = 99500;   // the wave 1 bonus (1,000) crosses 50,000 and 100,000 at once
    for (Alien& alien : game.aliens) alien.alive = false;
    updateGame(game, &NO_INPUT, DT);
    CHECK(game.players[0].lives == 5 && game.players[0].nextExtraLife == 150000);
}

static void threeDeathsEndTheRun() {
    Game game;
    startGame(game, 1, 7);
    game.aliens.clear();
    game.aliens.push_back(parkedAlien(10, 20));   // keeps the wave from ending
    game.swayDirection = 0;
    for (int death = 1; death <= 3; death++) {
        Player& p = game.players[0];
        Shot shot = {p.x + 7, p.y + 5, 0, -1, 0};   // parked on the hitbox
        game.enemyShots.push_back(shot);
        updateGame(game, &NO_INPUT, DT);
        CHECK(!game.players[0].alive && game.players[0].lives == 3 - death);
        run(game, NO_INPUT, 0.12f + 1.5f + 2.1f);   // hit-stop, respawn delay, then invincibility
        CHECK(game.players[0].alive == (death < 3));
    }
    CHECK(game.over);
}

static void nextWaveWaitsForALivingShip() {
    Game game;
    startGame(game, 1, 7);
    for (Alien& alien : game.aliens) alien.alive = false;
    Player& p = game.players[0];
    Shot shot = {p.x + 7, p.y + 5, 0, -1, 0};
    game.enemyShots.push_back(shot);
    updateGame(game, &NO_INPUT, DT);
    CHECK(!game.players[0].alive);
    game.players[0].respawnTimer = 4;   // longer than the 2 s wave pause
    run(game, NO_INPUT, 3);
    CHECK(game.wave == 1);              // the 2 s pause only counts while a ship is alive
    run(game, NO_INPUT, 1.1f + 0.2f);   // the death's hit-stop (0.12 s) comes first
    CHECK(game.players[0].alive && game.wave == 1);
    run(game, NO_INPUT, 2.2f);
    CHECK(game.wave == 2);
}

static void alienBodyKillsTheShip() {
    Game game;
    startGame(game, 1, 7);
    game.swayDirection = 0;
    Player& p = game.players[0];
    game.aliens.push_back(parkedAlien(p.x + 2, p.y + 2));
    updateGame(game, &NO_INPUT, DT);
    CHECK(!game.players[0].alive && game.players[0].lives == 2);
}

static void pickupAddsARocketUpToNine() {
    Game game;
    startSafeGame(game);
    Player& p = game.players[0];
    Pickup pickup = {p.x + 4, p.y, PICKUP_ROCKET, 0};
    game.pickups.push_back(pickup);
    updateGame(game, &NO_INPUT, DT);
    CHECK(game.players[0].rockets == 4 && game.pickups.empty() && game.soundRequested[SOUND_PICKUP]);
    game.players[0].rockets = 9;
    game.pickups.push_back(pickup);
    updateGame(game, &NO_INPUT, DT);
    CHECK(game.players[0].rockets == 9);
}

static void secondPlayerJoinsBlinkingWithAFreshRun() {
    Game game;
    startSafeGame(game);
    run(game, NO_INPUT, 5);
    CHECK(game.players.size() == 1);   // one player until someone joins
    for (bool& s : game.soundRequested) s = false;
    joinGame(game);
    CHECK(game.players.size() == 2 && game.soundRequested[SOUND_EXTRA_LIFE]);
    const Player& p2 = game.players[1];
    CHECK(p2.alive && p2.lives == 3 && p2.rockets == 3 && p2.score == 0 && p2.invincibleTimer == 2);
    CHECK(p2.x == (SCREEN_W - 16) / 2.0f + 40);   // 40 px right of player 1's start
    Actions actions[2] = {};
    actions[1].fire = true;
    updateGame(game, actions, DT);
    CHECK(game.bullets.size() == 1 && game.bullets[0].owner == 1);   // player 2's bullets score for player 2
}

static void coOpRunEndsWhenBothPlayersAreOut() {
    Game game;
    startGame(game, 1, 7);
    joinGame(game);
    game.aliens.clear();
    game.aliens.push_back(parkedAlien(10, 20));   // keeps the wave from ending
    game.swayDirection = 0;
    Actions actions[2] = {};
    game.players[0].lives = 1;
    Shot shot = {game.players[0].x + 7, game.players[0].y + 5, 0, -1, 0};
    game.enemyShots.push_back(shot);
    for (int step = 0; step < 3 * 60; step++) updateGame(game, actions, DT);
    CHECK(game.players[0].lives == 0 && game.players[1].alive && !game.over);
    game.players[1].lives = 1;
    game.players[1].invincibleTimer = 0;
    Shot shot2 = {game.players[1].x + 7, game.players[1].y + 5, 0, -1, 0};
    game.enemyShots.push_back(shot2);
    for (int step = 0; step < 2 * 60; step++) updateGame(game, actions, DT);
    CHECK(game.over);
}

static void waveBonusSkipsAPlayerWhoIsOut() {
    Game game;
    startSafeGame(game);
    joinGame(game);
    game.players[1].alive = false;
    game.players[1].lives = 0;
    for (Alien& alien : game.aliens) alien.alive = false;
    Actions actions[2] = {};
    updateGame(game, actions, DT);
    CHECK(game.players[0].score == 1000 && game.players[1].score == 0);
}

static void screenshotIsAValidBmp() {
    fillRect(0, 0, SCREEN_W, SCREEN_H, 0xFF000000);
    fillRect(0, 0, 1, 1, 0xFF112233);
    CHECK(screenSaveBmp("build/tests/shot.bmp"));
    static uint8_t data[54 + SCREEN_W * SCREEN_H * 3 + 1];
    FILE* file = fopen("build/tests/shot.bmp", "rb");
    size_t size = file ? fread(data, 1, sizeof data, file) : 0;
    if (file) fclose(file);
    CHECK(size == 172854);
    CHECK(data[0] == 'B' && data[1] == 'M');
    CHECK(data[18] + data[19] * 256 == 320 && data[22] == 180);
    CHECK(data[size - 960] == 0x33 && data[size - 959] == 0x22 && data[size - 958] == 0x11);   // top-left pixel starts the last row; B, G, R
    CHECK(data[54] == 0 && data[55] == 0 && data[56] == 0);                              // bottom-left pixel
    CHECK(!screenSaveBmp("build/tests/no-such-folder/shot.bmp"));
}

static void weaponLevelsFireTheirPattern() {
    Game game;
    startSafeGame(game);
    Actions a = NO_INPUT;
    a.fire = true;
    size_t expected[4] = {1, 2, 3, 3};
    for (int level = 1; level <= 3; level++) {
        game.bullets.clear();
        game.players[0].weaponLevel = level;
        game.players[0].fireTimer = 0;
        updateGame(game, &a, DT);
        CHECK(game.bullets.size() == expected[level - 1]);
        if (level == 2) CHECK(game.bullets[0].x < game.bullets[1].x && game.bullets[0].vx == 0 && game.bullets[1].vx == 0);
        if (level == 3) CHECK(game.bullets[0].vx == -70 && game.bullets[1].vx == 0 && game.bullets[2].vx == 70);
    }
    game.bullets.clear();
    game.players[0].weaponLevel = 4;
    game.players[0].fireTimer = 0;
    int volleys = 0;
    for (int step = 0; step < 60; step++) {   // one second at 0.08 s: shots at 0, 0.08, ... 0.96 s
        game.soundRequested[SOUND_SHOOT] = false;
        updateGame(game, &a, DT);
        if (game.soundRequested[SOUND_SHOOT]) volleys++;
    }
    CHECK(volleys == 13);
}

static void killDropsFollowTheRoll(ClassicVariant variant) {
    Game game;
    startGame(game, 1, 7, 1, false, variant);
    game.players[0].invincibleTimer = 1e9f;
    game.swayDirection = 0;
    int rockets = 0, upgrades = 0;
    const int KILLS = 20000;
    for (int kill = 0; kill < KILLS; kill++) {
        game.aliens.clear();
        game.aliens.push_back(parkedAlien(10, 20));    // keeps the wave going
        game.aliens.push_back(parkedAlien(150, 40));   // the victim
        Shot bullet = {150, 40, 0, 0, 0};
        game.bullets.push_back(bullet);
        updateGame(game, &NO_INPUT, DT);
        for (const Pickup& pickup : game.pickups) {
            rockets += pickup.type == PICKUP_ROCKET;
            upgrades += pickup.type == PICKUP_UPGRADE;
        }
        game.pickups.clear();
    }
    CHECK(rockets > KILLS * 0.015 && rockets < KILLS * 0.025);    // about 2 %
    if (variant == CLASSIC_EXPANDED) CHECK(upgrades > KILLS * 0.016 && upgrades < KILLS * 0.024);
    else CHECK(upgrades > KILLS * 0.007 && upgrades < KILLS * 0.013);
}

static void upgradePickupRaisesLevelUpToFour() {
    Game game;
    startSafeGame(game);
    Player& p = game.players[0];
    Pickup pickup = {p.x + 4, p.y, PICKUP_UPGRADE, 0};
    for (int level = 1; level <= 3; level++)   // level n takes n pickups: 1, then 2, then 3
        for (int part = 1; part <= level; part++) {
            game.pickups.push_back(pickup);
            updateGame(game, &NO_INPUT, DT);
            bool levelUp = part == level;
            CHECK(game.players[0].weaponLevel == (levelUp ? level + 1 : level) && game.pickups.empty());
            CHECK(game.players[0].upgradeParts == (levelUp ? 0 : part) && game.players[0].upgradeLevelUp == levelUp);
            CHECK(game.players[0].upgradeTimer > 0);
        }
    int score = game.players[0].score;
    game.pickups.push_back(pickup);
    updateGame(game, &NO_INPUT, DT);
    CHECK(game.players[0].weaponLevel == 4 && game.players[0].score == score + 1000);
    CHECK(game.players[0].rockets == 3);   // an upgrade is not a rocket
}

static void deathLosesOneWeaponLevel() {
    Game game;
    startGame(game, 1, 7);
    game.swayDirection = 0;
    for (int level = 3; level >= 1; level -= 2) {   // 3 -> 2, then 1 -> 1
        Player& p = game.players[0];
        p.weaponLevel = level;
        p.upgradeParts = level - 1;
        p.invincibleTimer = 0;
        Shot shot = {p.x + 7, p.y + 5, 0, -1, 0};
        game.enemyShots.push_back(shot);
        updateGame(game, &NO_INPUT, DT);
        CHECK(!game.players[0].alive && game.players[0].weaponLevel == (level == 3 ? 2 : 1) && game.players[0].upgradeParts == 0);
        run(game, NO_INPUT, 1.6f);
    }
}

// A safe run at the start of a boss wave: the boss has flown in and sways, nothing has fired yet.
static void startBossGame(Game& game, int wave, int level = 1) {
    startSafeGame(game, level);
    game.wave = wave - 1;
    for (Alien& alien : game.aliens) alien.alive = false;
    run(game, NO_INPUT, 2.5f);   // wave clear pause, then the boss enters
    run(game, NO_INPUT, 0.5f);
    for (Boss& boss : game.bosses) boss.attackTimer = 100;
}

static void everyFifthWaveIsABoss() {
    Game game;
    startSafeGame(game);
    game.wave = 4;
    for (Alien& alien : game.aliens) alien.alive = false;
    run(game, NO_INPUT, 2.2f);
    CHECK(game.wave == 5 && game.bosses.size() == 1 && game.bosses[0].alive && game.aliens.empty() && game.bannerTimer > 0);
    game.bosses[0].alive = false;
    run(game, NO_INPUT, 2.2f);
    CHECK(game.wave == 6 && game.bosses.empty() && game.aliens.size() == 40);
}

static void bossWavesFollowTheTable() {
    CHECK(bossHealth(1, 5) == 100 && bossHealth(1, 10) == 100 && bossHealth(1, 15) == 200 && bossHealth(1, 20) == 200);
    CHECK(bossCount(5) == 1 && bossCount(10) == 2 && bossCount(15) == 1 && bossCount(20) == 2);
    CHECK(bossType(1, 5) == BOSS_CLASSIC && bossType(1, 10) == BOSS_CLASSIC && bossType(1, 15) == BOSS_MAGE && bossType(1, 20) == BOSS_MAGE);
    CHECK(isBossWave(5) && isBossWave(10) && isBossWave(15));
    CHECK(!isBossWave(0) && !isBossWave(4) && !isBossWave(6));
}

static void bulletsAndBlastsDamageTheBoss() {
    Game game;
    startBossGame(game, 5);
    Boss& boss = game.bosses[0];
    CHECK(boss.y == 20);
    Shot bullet = {boss.x + 10, boss.y + 5, 0, 0, 0};
    game.bullets.push_back(bullet);
    updateGame(game, &NO_INPUT, DT);
    CHECK(boss.hp == 99 && boss.flash > 0 && game.bullets.empty());
    Rocket rocket = {boss.x + 14, boss.y + 10, 60, 0};
    game.rockets.push_back(rocket);
    updateGame(game, &NO_INPUT, DT);
    CHECK(boss.hp == 79 && game.rockets.empty());
    game.rockets.push_back({boss.x - 80, 3, 60, 0});   // bursts at the top, 80 px beside the boss: out of reach
    run(game, NO_INPUT, 0.2f);
    CHECK(boss.hp == 79);
    game.rockets.push_back({boss.x + 14, 3, 60, 0});   // bursts at the top, 20 px above the boss: the blast reaches it
    run(game, NO_INPUT, 0.2f);
    CHECK(boss.hp == 59);
}

static void killingTheBossScoresAndDropsBothPickups() {
    Game game;
    startBossGame(game, 5);
    game.bosses[0].hp = 1;
    int before = game.players[0].score;
    Shot bullet = {game.bosses[0].x + 10, game.bosses[0].y + 5, 0, 0, 0};
    game.bullets.push_back(bullet);
    updateGame(game, &NO_INPUT, DT);
    CHECK(!game.bosses[0].alive && game.players[0].score == before + 5000 + 5 * 1000);   // 5,000 x 1 and the wave 5 clear bonus
    int rockets = 0, upgrades = 0;
    for (const Pickup& pickup : game.pickups) {
        rockets += pickup.type == PICKUP_ROCKET;
        upgrades += pickup.type == PICKUP_UPGRADE;
    }
    CHECK(rockets == 1 && upgrades == 2 && !game.explosions.empty());
    run(game, NO_INPUT, 0.1f);
    CHECK(game.nextWaveTimer > 0);   // the normal wave-clear pause starts
}

static void bossBodyKillsTheShip() {
    Game game;
    startBossGame(game, 5);
    game.bosses[0].y = 110;   // down in the ship's movement zone
    Player& p = game.players[0];
    p.invincibleTimer = 0;
    p.x = game.bosses[0].x + 10;
    p.y = 112;
    updateGame(game, &NO_INPUT, DT);
    CHECK(!game.players[0].alive && game.players[0].lives == 2);
}

static void bossFiresBurstThenFan() {
    Game game;
    startBossGame(game, 5);
    game.bosses[0].attackTimer = 0.01f;
    run(game, NO_INPUT, 0.5f);
    CHECK(game.enemyShots.size() == 3);   // burst: 0, 0.15, 0.3 s
    for (const Shot& shot : game.enemyShots) CHECK(fabsf(sqrtf(shot.vx * shot.vx + shot.vy * shot.vy) - 90) < 0.5f && shot.vy > 0);
    game.enemyShots.clear();
    game.bosses[0].attackTimer = 0.01f;
    run(game, NO_INPUT, 0.05f);
    CHECK(game.enemyShots.size() == 5);   // fan
    CHECK(game.enemyShots[0].vx == -60 && game.enemyShots[2].vx == 0 && game.enemyShots[4].vx == 60 && game.enemyShots[2].vy == 70);
}

static void bossWaitsForALivingShip() {
    Game game;
    startBossGame(game, 5);
    game.players[0].alive = false;
    game.players[0].respawnTimer = 1e9f;
    game.bosses[0].attackTimer = 0.01f;
    run(game, NO_INPUT, 3);
    CHECK(game.enemyShots.empty());
}

static void twinBossesSwayInTheirOwnHalves() {
    Game game;
    startBossGame(game, 10);
    const Boss& left = game.bosses[0];
    const Boss& right = game.bosses[1];
    CHECK(game.bosses.size() == 2);
    CHECK(left.minX == 0 && left.maxX == 128 && right.minX == 160 && right.maxX == 288 && fabsf(fabsf(left.vx) - 50) < 0.01f);
    bool inside = true;
    for (int step = 0; step < 600; step++) {
        updateGame(game, &NO_INPUT, DT);
        inside &= left.x >= 0 && left.x <= 128 && right.x >= 160 && right.x <= 288;
    }
    CHECK(inside);
}

static void twinBossesBothMustDie() {
    Game game;
    startBossGame(game, 10);
    int before = game.players[0].score;
    game.bosses[0].hp = 1;
    Shot bullet = {game.bosses[0].x + 10, game.bosses[0].y + 5, 0, 0, 0};
    game.bullets.push_back(bullet);
    updateGame(game, &NO_INPUT, DT);
    CHECK(!game.bosses[0].alive && game.bosses[1].alive && game.players[0].score == before + 10000);
    run(game, NO_INPUT, 0.3f);
    CHECK(game.nextWaveTimer == 0 && game.wave == 10);
    game.bosses[1].hp = 1;
    bullet = {game.bosses[1].x + 10, game.bosses[1].y + 5, 0, 0, 0};
    game.bullets.push_back(bullet);
    updateGame(game, &NO_INPUT, DT);
    CHECK(game.players[0].score == before + 10000 + 10000 + 10 * 1000);
    int rockets = 0, upgrades = 0;
    for (const Pickup& pickup : game.pickups) {
        rockets += pickup.type == PICKUP_ROCKET;
        upgrades += pickup.type == PICKUP_UPGRADE;
    }
    CHECK(rockets == 2 && upgrades == 4);   // two twins, 2 upgrades each
    run(game, NO_INPUT, 0.1f);
    CHECK(game.nextWaveTimer > 0);
}

static void mageBallBeamsItsColumnAfterOneAndAHalfSeconds() {
    for (int offset : {0, 20}) {
        Game game;
        startBossGame(game, 15);
        Boss& mage = game.bosses[0];
        Player& p = game.players[0];
        CHECK(game.bosses.size() == 1 && mage.type == BOSS_MAGE && mage.hp == 200);
        p.invincibleTimer = 0;
        mage.ballX = p.x + 8 + offset;
        mage.ballY = 21;
        mage.ballTimer = 1.5f;
        run(game, NO_INPUT, 1.4f);
        CHECK(p.alive && mage.beamTimer == 0);
        run(game, NO_INPUT, 0.3f);
        CHECK(mage.beamTimer > 0 && p.alive == (offset == 20));
        run(game, NO_INPUT, 1);
        CHECK(mage.beamTimer == 0);
    }
}

static void deadMageCastsNoBeam() {
    Game game;
    startBossGame(game, 15);
    Boss& mage = game.bosses[0];
    Player& p = game.players[0];
    p.invincibleTimer = 0;
    mage.ballX = p.x + 8;
    mage.ballY = 21;
    mage.ballTimer = 1;
    mage.hp = 1;
    Shot bullet = {mage.x + 10, mage.y + 5, 0, 0, 0};
    game.bullets.push_back(bullet);
    run(game, NO_INPUT, 1.5f);
    CHECK(game.wave == 15 && !mage.alive && mage.beamTimer == 0 && p.alive);
}

static void mageAttacksInRandomOrderAndGaps() {
    Game game;
    startBossGame(game, 15);
    const Boss& mage = game.bosses[0];
    game.bosses[0].attackTimer = 0.01f;
    float lastAttack = -1, minGap = 1e9f, maxGap = 0;
    int casts = 0;
    bool shotsSeen = false, ballBefore = false;
    for (int step = 0; step < 7200; step++) {
        float timerBefore = mage.attackTimer;
        updateGame(game, &NO_INPUT, DT);
        if (mage.attackTimer > timerBefore) {
            if (lastAttack >= 0) {
                minGap = std::min(minGap, game.time - lastAttack);
                maxGap = std::max(maxGap, game.time - lastAttack);
            }
            lastAttack = game.time;
        }
        if (mage.ballTimer > 0 && !ballBefore) casts++;
        ballBefore = mage.ballTimer > 0;
        if (!game.enemyShots.empty()) shotsSeen = true;
    }
    CHECK(casts >= 5 && shotsSeen);
    CHECK(minGap >= 0.6f * 1.9f - 0.02f && maxGap <= 1.4f * 1.9f + 0.02f);
    CHECK(minGap < 1.5f && maxGap > 2.3f);
}

static void levelBannerComesBeforeWaveOne() {
    Game game;
    startSafeGame(game);
    CHECK(fabsf(game.bannerTimer - 3.5f) < 1e-4f);
    run(game, NO_INPUT, 2.1f);
    CHECK(game.wave == 1 && game.bannerTimer > 0 && game.bannerTimer < 1.5f);
    for (Alien& alien : game.aliens) alien.alive = false;
    run(game, NO_INPUT, 2.2f);
    CHECK(game.wave == 2 && game.bannerTimer <= 1.5f);
}

static void killingTheLevelBossClearsTheLevel() {
    Game game;
    startBossGame(game, 20, 5);
    CHECK(game.bosses.size() == 2 && game.bosses[0].type == BOSS_FISHERMAN && game.bosses[0].hp == 420 && game.bosses[1].hp == 420);
    game.bosses[1].alive = false;   // its twin is already down
    game.bosses[0].hp = 1;
    Shot enemyShot = {10, 40, 0, -1, 0};   // parked away from the ship
    game.enemyShots.push_back(enemyShot);
    Shot bullet = {game.bosses[0].x + 10, game.bosses[0].y + 5, 0, 0, 0};
    game.bullets.push_back(bullet);
    int before = game.players[0].score;
    updateGame(game, &NO_INPUT, DT);
    CHECK(!game.bosses[0].alive && game.levelClear && game.clearTimer == 3 && !game.over);
    CHECK(game.players[0].score == before + 100000 + 20 * 1000);   // 5,000 x (4 + 16) and the wave 20 clear bonus
    CHECK(game.enemyShots.size() == 1);   // shots in the air stay
    run(game, NO_INPUT, 2.9f);
    CHECK(!game.over && game.wave == 20);   // the boss death's 0.15 s hit-stop comes first
    run(game, NO_INPUT, 0.5f);
    CHECK(game.over && game.won && game.wave == 20);
    run(game, NO_INPUT, 3);
    CHECK(game.wave == 20 && !game.bosses[0].alive && game.aliens.empty());
}

static void deathDuringTheClearOnlyLosesOnTheLastLife() {
    for (int lives : {1, 2}) {
        Game game;
        startBossGame(game, 20, 5);
        game.bosses[1].alive = false;
        game.players[0].lives = lives;
        game.players[0].nextExtraLife = MAX_SCORE;   // the boss reward must not grant a life
        game.bosses[0].hp = 1;
        Shot bullet = {game.bosses[0].x + 10, game.bosses[0].y + 5, 0, 0, 0};
        game.bullets.push_back(bullet);
        updateGame(game, &NO_INPUT, DT);
        CHECK(game.levelClear);
        run(game, NO_INPUT, 0.2f);   // past the boss death's hit-stop, which skips collisions
        Player& p = game.players[0];
        p.invincibleTimer = 0;
        Shot shot = {p.x + 7, p.y + 5, 0, -1, 0};   // parked on the hitbox, as in threeDeathsEndTheRun
        game.enemyShots.push_back(shot);
        updateGame(game, &NO_INPUT, DT);
        CHECK(!game.players[0].alive && game.players[0].lives == lives - 1);
        run(game, NO_INPUT, 4);
        CHECK(game.over && game.won == (lives == 2));
    }
}

// A frozen copy of the picture drawGame just drew.
static std::vector<uint32_t> pictureOf(const Game& game) {
    drawGame(game);
    return std::vector<uint32_t>(screenPixels, screenPixels + SCREEN_W * SCREEN_H);
}

static void beamDrawsAMagentaColumn() {
    Game game;
    startBossGame(game, 15);
    game.bosses[0].ballX = 100;
    game.bosses[0].ballY = 21;
    game.bosses[0].beamTimer = 0.5f;
    std::vector<uint32_t> picture = pictureOf(game);
    CHECK(picture[170 * SCREEN_W + 95] == paletteColor('m'));
    CHECK(picture[170 * SCREEN_W + 100] == paletteColor('w'));
}

// A rocket 0.5 px under the top of the screen: it bursts on the next step.
static void rocketAboutToBurst(Game& game, float x) {
    Rocket rocket = {x, 0.5f, 60, 0};
    game.rockets.push_back(rocket);
}

static void overlappingHitStopsTakeTheLongest() {
    Game game;
    startSafeGame(game);
    game.aliens.clear();
    game.aliens.push_back(parkedAlien(10, 20));   // keeps the wave going
    game.swayDirection = 0;
    game.players[0].invincibleTimer = 0;
    Player& p = game.players[0];
    Shot shot = {p.x + 7, p.y + 5, 0, -1, 0};
    game.enemyShots.push_back(shot);   // the ship dies (0.12 s) on the same step as a blast (0.06 s)
    rocketAboutToBurst(game, 200);
    updateGame(game, &NO_INPUT, DT);
    CHECK(!game.players[0].alive && game.explosions.size() == 2);
    float frozenAt = game.time;
    int frozenSteps = 0;
    while (frozenSteps < 30) {
        updateGame(game, &NO_INPUT, DT);
        if (game.time != frozenAt) break;
        frozenSteps++;
    }
    CHECK(frozenSteps >= 6 && frozenSteps <= 8);   // 0.12 s is 7.2 steps; the sum 0.18 s would be 10.8
    CHECK(game.time > frozenAt);                   // and the game runs again
}

static void shootingAndKillsNeverShake() {
    Game game;
    startSafeGame(game);
    game.aliens.clear();
    for (int i = 0; i < 4; i++) game.aliens.push_back(parkedAlien(153, 30 + i * 12));   // stacked right above the ship
    game.swayDirection = 0;
    Actions a = NO_INPUT;
    a.fire = true;
    float worst = 0;
    for (int step = 0; step < 120; step++) {
        updateGame(game, &a, DT);
        worst = std::max(worst, game.trauma);
    }
    CHECK(game.players[0].score >= 400 && worst == 0);   // aliens died, nothing shook
}

static void traumaDecaysToZero() {
    Game game;
    startSafeGame(game);
    game.trauma = 1;
    run(game, NO_INPUT, 0.3f);
    CHECK(game.trauma > 0.4f && game.trauma < 0.6f);   // 1 - 1.5 x 0.3
    run(game, NO_INPUT, 0.4f);
    CHECK(game.trauma == 0);                           // inside 1 / 1.5 s
}

static void inputDuringHitStopStillFires() {
    Game game;
    startSafeGame(game);
    game.hitStop = 0.1f;
    Actions press = NO_INPUT;
    press.rocket = true;
    updateGame(game, &press, DT);   // pressed while frozen
    CHECK(game.rockets.empty() && game.hitStop > 0);
    run(game, NO_INPUT, 0.2f);
    CHECK(game.players[0].rockets == 2);   // the buffered press launched once the stop ended
}

static void flashesAreRateLimited() {
    Game game;
    startSafeGame(game);
    game.aliens.clear();
    game.aliens.push_back(parkedAlien(10, 20));
    game.swayDirection = 0;
    int flashes = 0;
    float before = 0;
    for (int step = 0; step < 12; step++) {   // three blasts in about 0.2 s
        if (step % 4 == 0) rocketAboutToBurst(game, 200);
        updateGame(game, &NO_INPUT, DT);
        if (game.flashTimer > before) flashes++;
        before = game.flashTimer;
    }
    CHECK(flashes == 1);
    run(game, NO_INPUT, 0.5f);
    rocketAboutToBurst(game, 200);
    updateGame(game, &NO_INPUT, DT);
    CHECK(game.flashTimer > 0);   // a later blast flashes again
}

static void reducedFlashingDrawsNoFlash() {
    Game game;
    startSafeGame(game);
    std::vector<uint32_t> calm = pictureOf(game);
    game.flashTimer = 0.05f;
    CHECK(pictureOf(game) != calm);   // a flash changes the picture
    game.reduceFlashing = true;
    CHECK(pictureOf(game) == calm);   // reduced: no pixel differs
}

static void zeroShakeDrawsNoOffset() {
    Game game;
    startSafeGame(game);
    std::vector<uint32_t> calm = pictureOf(game);
    game.trauma = 1;
    game.feelTime = 0.1f;
    CHECK(pictureOf(game) != calm);   // full strength moves the picture
    game.shakeStrength = 0;
    CHECK(pictureOf(game) == calm);
}

static void settingsDefaultToMizrahit() {
    CHECK(settingsLoad("build/tests/missing.txt").music == MUSIC_MIZRAHIT);
}

static void settingsSurviveASave() {
    Settings settings;
    for (Music song : {MUSIC_OFF, MUSIC_DUBSTEP, MUSIC_MIZRAHIT}) {
        settings.music = song;
        CHECK(settingsSave("build/tests/settings.txt", settings));
        CHECK(settingsLoad("build/tests/settings.txt").music == song);
    }
}

static void writeTextFile(const char* path, const char* text) {
    FILE* file = fopen(path, "w");
    fputs(text, file);
    fclose(file);
}

static void oldMusicSettingsStillLoad() {
    writeTextFile("build/tests/old-settings.txt", "music=0\n");
    CHECK(settingsLoad("build/tests/old-settings.txt").music == MUSIC_OFF);
    writeTextFile("build/tests/old-settings.txt", "music=1\n");
    CHECK(settingsLoad("build/tests/old-settings.txt").music == MUSIC_MIZRAHIT);
    writeTextFile("build/tests/old-settings.txt", "music=7\n");
    CHECK(settingsLoad("build/tests/old-settings.txt").music == MUSIC_MIZRAHIT);
}

static void highScoreDefaultsToZero() {
    CHECK(highScoreLoad("build/tests/no-such-highscore.txt") == 0);
    writeTextFile("build/tests/garbage-highscore.txt", "this is not a score\nbest=\n");
    CHECK(highScoreLoad("build/tests/garbage-highscore.txt") == 0);
}

static void highScoreSurvivesASave() {
    CHECK(highScoreSave("build/tests/highscore.txt", 123456));
    CHECK(highScoreLoad("build/tests/highscore.txt") == 123456);
    CHECK(highScoreSave("build/tests/highscore.txt", 0));
    CHECK(highScoreLoad("build/tests/highscore.txt") == 0);
    CHECK(!highScoreSave("build/tests/no-such-folder/highscore.txt", 5));
}

static void highScoreIsClamped() {
    writeTextFile("build/tests/clamp-highscore.txt", "best=-500\n");
    CHECK(highScoreLoad("build/tests/clamp-highscore.txt") == 0);
    writeTextFile("build/tests/clamp-highscore.txt", "best=99999999999\n");
    CHECK(highScoreLoad("build/tests/clamp-highscore.txt") == 9999999);
}

static void shakeAndFlashSettingsSurviveASave() {
    Settings defaults = settingsLoad("build/tests/missing.txt");
    CHECK(defaults.shake == 100 && !defaults.reduceFlashing);
    Settings settings;
    settings.shake = 50;
    settings.reduceFlashing = true;
    CHECK(settingsSave("build/tests/shake-settings.txt", settings));
    Settings loaded = settingsLoad("build/tests/shake-settings.txt");
    CHECK(loaded.shake == 50 && loaded.reduceFlashing && loaded.music == MUSIC_MIZRAHIT);
    writeTextFile("build/tests/shake-settings.txt", "shake=900\n");
    CHECK(settingsLoad("build/tests/shake-settings.txt").shake == 100);   // clamped
}

static void jungleDifficultyIsFiveWavesAhead() {
    CHECK(difficultyWave(1, 7) == 7 && difficultyWave(2, 1) == 6);
    Game game;
    startSafeGame(game, 2);
    CHECK(game.level == 2 && fabsf(game.bannerTimer - 3.5f) < 1e-4f && game.aliens.size() == 40);
    for (size_t i = 0; i < game.aliens.size(); i++) CHECK(game.aliens[i].type == (i < 16 ? ALIEN_BLUE : ALIEN_GREEN));
}

// Jungle wave 8 after a cleared wave 7: 40 aliens and one heavy beetle.
static void startHeavyWave(Game& game) {
    startSafeGame(game, 2);
    game.wave = 7;
    for (Alien& alien : game.aliens) alien.alive = false;
    run(game, NO_INPUT, 2.2f);
}

static void heavyBeetlesComeEveryEightWaves() {
    CHECK(heavyCount(2, 7) == 0 && heavyCount(2, 8) == 1 && heavyCount(2, 14) == 1 && heavyCount(2, 15) == 0);
    CHECK(heavyCount(2, 16) == 2 && heavyCount(2, 19) == 2 && heavyCount(1, 16) == 0);
    Game game;
    startHeavyWave(game);
    int heavies = 0;
    for (const Alien& alien : game.aliens)
        if (alien.type == ALIEN_HEAVY) {
            heavies++;
            CHECK(alien.hp == 12);
        }
    CHECK(game.wave == 8 && game.aliens.size() == 41 && heavies == 1);
}

static void heavyBeetleRoamsTheUpperScreen() {
    Game game;
    startHeavyWave(game);
    run(game, NO_INPUT, 3);
    float minX = 1000, maxX = -1000;
    for (int step = 0; step < 360; step++) {
        updateGame(game, &NO_INPUT, DT);
        for (const Alien& alien : game.aliens)
            if (alien.type == ALIEN_HEAVY) {
                CHECK(alien.state == ALIEN_ROAMING && alien.y >= 18 && alien.y <= 84);
                minX = std::min(minX, alien.x);
                maxX = std::max(maxX, alien.x);
            }
    }
    CHECK(maxX - minX > 100);
}

static void levelOneClearContinuesIntoTheJungle() {
    Game game;
    startBossGame(game, 20);
    game.bosses[1].alive = false;
    game.bosses[0].hp = 1;
    Shot bullet = {game.bosses[0].x + 10, game.bosses[0].y + 5, 0, 0, 0};
    game.bullets.push_back(bullet);
    updateGame(game, &NO_INPUT, DT);
    CHECK(game.levelClear);
    int scoreAtKill = game.players[0].score;
    run(game, NO_INPUT, 3.4f);
    CHECK(!game.over && game.level == 2 && game.wave == 1 && !game.levelClear && game.aliens.size() == 40);
    CHECK(game.players[0].score == scoreAtKill);
}

static void pickedLevelRunEndsOnItsClear() {
    Game game;
    startGame(game, 1, 7, 2, true);   // the level picker's jungle
    game.players[0].invincibleTimer = 1e9f;
    game.wave = 19;
    for (Alien& alien : game.aliens) alien.alive = false;
    run(game, NO_INPUT, 3);
    for (Boss& boss : game.bosses) boss.alive = false;
    run(game, NO_INPUT, 0.1f);
    CHECK(game.levelClear);
    run(game, NO_INPUT, 3.4f);
    CHECK(game.over && game.won && game.level == 2);
}

static void jungleBossesFollowTheTable() {
    const int hp[4] = {150, 150, 300, 300};
    const BossType type[4] = {BOSS_MONKEY, BOSS_MONKEY, BOSS_GORILLA, BOSS_GORILLA};
    for (int i = 0; i < 4; i++) CHECK(bossHealth(2, 5 * (i + 1)) == hp[i] && bossType(2, 5 * (i + 1)) == type[i]);
    CHECK(bossCount(5) == 1 && bossCount(10) == 2 && bossCount(15) == 1 && bossCount(20) == 2);
}

static void monkeyThrowsBananasStraightDownThenAPeel() {
    Game game;
    startBossGame(game, 5, 2);
    game.bosses[0].attackTimer = 0.001f;
    updateGame(game, &NO_INPUT, DT);
    CHECK(game.soundRequested[SOUND_SCREECH]);
    run(game, NO_INPUT, 0.5f);
    CHECK(game.thrown.size() == 3);
    for (const Thrown& t : game.thrown) CHECK(t.kind == THROWN_BANANA && t.vx == 0 && t.vy == 85);
    CHECK(game.lobs.size() == 1 && !game.lobs[0].poop);
    CHECK(game.lobs[0].toX >= 8 && game.lobs[0].toX <= 312 && game.lobs[0].toY >= 107 && game.lobs[0].toY <= 173);
    run(game, NO_INPUT, 0.9f);
    const Player& p = game.players[0];
    CHECK(game.lobs.empty() && (game.peels.size() == 1 || p.slideTimer > 0 || p.stunTimer > 0));
}

static void monkeyAlternatesWithABananaBunch() {
    Game game;
    startBossGame(game, 5, 2);
    game.bosses[0].attackTimer = 0.001f;
    run(game, NO_INPUT, 1.4f);
    size_t before = game.thrown.size();
    game.bosses[0].attackTimer = 0.001f;
    updateGame(game, &NO_INPUT, DT);
    CHECK(game.thrown.size() == before + 5);
    const float sideways[5] = {-50, -25, 0, 25, 50};
    for (int i = 0; i < 5; i++) CHECK(game.thrown[before + i].vx == sideways[i]);
}

static void peelSlidesTheShipThenStunsIt() {
    Game game;
    startSafeGame(game);
    Player& p = game.players[0];
    game.peels.push_back({p.x + 5, p.y + 4, 5});
    Actions right = {};
    right.moveX = 1;
    updateGame(game, &right, DT);
    CHECK(p.slideTimer > 0 && game.peels.empty());
    float x = p.x;
    Actions left = {};
    left.moveX = -1;
    run(game, left, 0.4f);
    CHECK(p.x > x);
    run(game, left, 0.2f);
    CHECK(p.stunTimer > 0);
    x = p.x;
    left.fire = true;
    run(game, left, 0.5f);
    CHECK(p.x == x && game.bullets.empty());
    left.fire = false;
    run(game, left, 0.6f);
    CHECK(p.x < x);
}

static void peelLiesFiveSeconds() {
    Game game;
    startSafeGame(game);
    game.peels.push_back({20, 110, 5});
    run(game, NO_INPUT, 4.9f);
    CHECK(game.peels.size() == 1);
    run(game, NO_INPUT, 0.2f);
    CHECK(game.peels.empty());
}

static void bananaKillsTheShip() {
    Game game;
    startSafeGame(game);
    Player& p = game.players[0];
    p.invincibleTimer = 0;
    game.thrown.push_back({p.x + 5, p.y + 4, 0, 0, THROWN_BANANA, 0, 0});
    updateGame(game, &NO_INPUT, DT);
    CHECK(!p.alive);
}

static void mistShotsHealTheGorilla() {
    Game game;
    startBossGame(game, 15, 2);
    Boss& boss = game.bosses[0];
    boss.hp = 100;
    const Player& p = game.players[0];
    game.mists.push_back({p.x + 8, p.y + 7, 1});
    game.soundRequested[SOUND_LAUGH] = false;
    Shot bullet = {boss.x + 10, boss.y + 5, 0, 0, 0};
    game.bullets.push_back(bullet);
    updateGame(game, &NO_INPUT, DT);
    CHECK(boss.hp == 101 && boss.laughTimer > 0 && game.soundRequested[SOUND_LAUGH]);
    game.mists.clear();
    game.bullets.push_back(bullet);
    updateGame(game, &NO_INPUT, DT);
    CHECK(boss.hp == 100);
}

static void poopSplatsIntoAMistThatFades() {
    Game game;
    startSafeGame(game, 2);
    game.lobs.push_back({100, 40, 100, 140, 0, true, false});
    run(game, NO_INPUT, 0.85f);
    CHECK(game.mists.size() == 1 && game.lobs.empty());
    run(game, NO_INPUT, 6);
    CHECK(game.mists.empty());
}

static void gorillaPoundDropsFiveCoconuts() {
    Game game;
    startBossGame(game, 15, 2);
    game.bosses[0].windupTimer = 0.01f;
    run(game, NO_INPUT, 0.05f);
    CHECK(game.thrown.size() == 5);
    for (size_t i = 0; i < game.thrown.size(); i++)
        CHECK(game.thrown[i].kind == THROWN_COCONUT && game.thrown[i].x >= i * 62.8f && game.thrown[i].x < (i + 1) * 62.8f);
}

static void gorillaNeverPoopsPastThreeMists() {
    Game game;
    startBossGame(game, 15, 2);
    for (int i = 0; i < 3; i++) game.mists.push_back({20.0f + i * 40, 30, 1});
    bool pounded = false, peeled = false;
    for (int i = 0; i < 40; i++) {
        game.bosses[0].attackTimer = 0.001f;
        updateGame(game, &NO_INPUT, DT);
        pounded |= game.bosses[0].windupTimer > 0;
        for (const Lob& lob : game.lobs) {
            CHECK(!lob.poop);
            peeled |= !lob.poop;
        }
    }
    CHECK(pounded && peeled);
}

static void deadGorillaDropsNoCoconuts() {
    Game game;
    startBossGame(game, 15, 2);
    Boss& boss = game.bosses[0];
    boss.hp = 1;
    boss.windupTimer = 0.3f;
    Shot bullet = {boss.x + 10, boss.y + 5, 0, 0, 0};
    game.bullets.push_back(bullet);
    updateGame(game, &NO_INPUT, DT);
    CHECK(!boss.alive);
    run(game, NO_INPUT, 1);
    CHECK(game.thrown.empty());
}

static void levelBackgroundsStayDarkerThanTheStars() {
    const char feature[LEVEL_COUNT] = {0, 't', 'A', 'C', '3'};   // jungle edge trunks, ice walls, lava rivers, kelp
    for (int level = 2; level <= LEVEL_COUNT; level++) {
        Game game;
        startGame(game, 1, 7, level);
        uint32_t star = paletteColor('s');
        uint32_t starLight = (299 * (star >> 16 & 255) + 587 * (star >> 8 & 255) + 114 * (star & 255)) / 1000;
        for (float time : {0.0f, 13.7f}) {
            game.sceneryTime = game.time = time;
            drawBackground(game);
            int count = 0;
            for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
                uint32_t c = screenPixels[i];
                CHECK((299 * (c >> 16 & 255) + 587 * (c >> 8 & 255) + 114 * (c & 255)) / 1000 <= starLight);
                count += c == paletteColor(feature[level - 1]);
            }
            CHECK(count >= 500);
        }
    }
}

// Three more levels (three-levels-plan.md)
static void levelLadderGetsHarder() {
    // Each new level drops 15 below the last level's wave 20, then climbs 20 more: 1-20, 6-25, 11-30, 16-35, 21-40.
    CHECK(difficultyWave(1, 20) == 20 && difficultyWave(2, 1) == 6 && difficultyWave(2, 20) == 25);
    CHECK(difficultyWave(3, 1) == 11 && difficultyWave(4, 1) == 16 && difficultyWave(5, 1) == 21 && difficultyWave(5, 20) == 40);
    CHECK(heavyCount(3, 5) == 0 && heavyCount(3, 6) == 1 && heavyCount(3, 12) == 2 && heavyCount(3, 18) == 3);
    CHECK(heavyCount(4, 4) == 0 && heavyCount(4, 6) == 1 && heavyCount(4, 11) == 2 && heavyCount(4, 19) == 3);
    CHECK(heavyCount(5, 4) == 1 && heavyCount(5, 8) == 2 && heavyCount(5, 16) == 3 && heavyCount(5, 15) == 0);
    CHECK(heavyCount(2, 16) == 2 && heavyCount(1, 19) == 0);
    Game game;
    startSafeGame(game, 5);
    CHECK(game.level == 5 && game.aliens.size() == 40);
    for (size_t i = 0; i < game.aliens.size(); i++) CHECK(game.aliens[i].type == (i < 16 ? ALIEN_RED : i < 32 ? ALIEN_BLUE : ALIEN_GREEN));
    for (int step = 0; step < 600 && game.enemyShots.empty(); step++) updateGame(game, &NO_INPUT, DT);
    CHECK(!game.enemyShots.empty() && game.enemyShots[0].vy == 105);   // wave 21's 90 px/s plus the deep sea's 15
}

static void heaviesFlyInAtThreePlaces() {
    Game game;
    startSafeGame(game, 5);
    game.wave = 15;
    for (Alien& alien : game.aliens) alien.alive = false;
    run(game, NO_INPUT, 2.2f);
    std::vector<float> slots;
    for (const Alien& alien : game.aliens)
        if (alien.type == ALIEN_HEAVY) slots.push_back(alien.slotX);
    CHECK(game.wave == 16 && slots.size() == 3 && slots[0] == 60 && slots[1] == 244 && slots[2] == 152);
}

static void levelNamesFollowTheOrder() {
    CHECK(strcmp(levelName(1), "LEVEL 1") == 0 && strcmp(levelName(3), "GLACIER") == 0 && strcmp(levelName(5), "DEEP SEA") == 0);
    CHECK(strcmp(levelLabel(1), "WAVE") == 0 && strcmp(levelLabel(4), "VOLCANO") == 0);
}

static void eachLevelClearContinuesToTheNext() {
    for (int level = 2; level <= 4; level++) {
        Game game;
        startBossGame(game, 20, level);
        game.bosses[1].alive = false;
        game.bosses[0].hp = 1;
        Shot bullet = {game.bosses[0].x + 10, game.bosses[0].y + 5, 0, 0, 0};
        game.bullets.push_back(bullet);
        updateGame(game, &NO_INPUT, DT);
        CHECK(game.levelClear);
        game.peels.push_back({20, 120, 5});   // hazards don't follow the run into the next level
        game.thrown.push_back({20, 40, 0, 0, THROWN_ICE_BLOCK, 0, 0});
        run(game, NO_INPUT, 3.4f);
        CHECK(!game.over && game.level == level + 1 && game.wave == 1 && game.peels.empty() && game.thrown.empty());
    }
}

static void newBossesFollowTheTable() {
    const BossType types[3][2] = {{BOSS_YETI, BOSS_GOLEM}, {BOSS_IMP, BOSS_FIRELORD}, {BOSS_ANGLER, BOSS_FISHERMAN}};
    const int hp[3][2] = {{180, 340}, {200, 380}, {220, 420}};
    for (int level = 3; level <= 5; level++)
        for (int wave = 5; wave <= 20; wave += 5) {
            int big = wave >= 15;
            CHECK(bossType(level, wave) == types[level - 3][big] && bossHealth(level, wave) == hp[level - 3][big]);
        }
}

static float thrownSpeed(const Thrown& t) {
    return sqrtf(t.vx * t.vx + t.vy * t.vy);
}

static int countThrown(const Game& game, ThrownKind kind) {
    return (int)std::count_if(game.thrown.begin(), game.thrown.end(), [kind](const Thrown& t) { return t.kind == kind; });
}

static void shipGlidesOnTheGlacier() {
    Actions right = {};
    right.moveX = 1;
    Game game;
    startSafeGame(game);
    updateGame(game, &right, DT);
    CHECK(game.players[0].vx == 150);   // level 1: instant
    startSafeGame(game, 3);
    updateGame(game, &right, DT);
    CHECK(game.players[0].vx > 10 && game.players[0].vx < 15);   // 150 x 5 / 60
    run(game, right, 1);
    CHECK(game.players[0].vx > 140);
    float x = game.players[0].x;
    run(game, NO_INPUT, 0.2f);
    CHECK(game.players[0].x > x + 10);   // still gliding after the stick is let go
}

static void iceBlockFreezesAndMashingBreaksOut() {
    for (int mash = 0; mash < 2; mash++) {
        Game game;
        startSafeGame(game, 3);
        Player& p = game.players[0];
        game.thrown.push_back({p.x + 4, p.y + 3, 0, 0, THROWN_ICE_BLOCK, 0, 0});
        Actions hold = {};
        hold.fire = !mash;   // held from before the freeze: no fresh press
        updateGame(game, &hold, DT);
        CHECK(p.frozenTimer > 1.5f && p.alive && game.thrown.empty() && game.soundRequested[SOUND_FREEZE]);
        game.bullets.clear();   // fired in the step before the block landed
        Actions push = {};
        push.moveX = 1;
        float x = p.x;
        int steps = 0;
        while (p.frozenTimer > 0 && steps < 200) {
            push.fire = mash ? steps % 2 == 0 : true;   // a fresh press every other step, or R1 held
            updateGame(game, &push, DT);
            steps++;
        }
        CHECK(p.x == x && game.bullets.empty());
        if (mash) CHECK(steps < 40);   // 0.15 s off per press
        else CHECK(steps >= 95 && steps <= 97);   // 1.6 s
    }
}

static void yetiCyclesVolleyIceBlockAndSneeze() {
    Game game;
    startBossGame(game, 5, 3);
    Boss& yeti = game.bosses[0];
    yeti.attackTimer = 0.001f;
    run(game, NO_INPUT, 0.4f);
    CHECK(countThrown(game, THROWN_SNOWBALL) == 3);
    for (const Thrown& t : game.thrown) CHECK(fabsf(thrownSpeed(t) - 90) < 0.5f && t.vy > 0);
    game.thrown.clear();
    yeti.attackTimer = 0.001f;
    updateGame(game, &NO_INPUT, DT);
    CHECK(countThrown(game, THROWN_ICE_BLOCK) == 1 && fabsf(thrownSpeed(game.thrown[0]) - 80) < 0.5f && game.soundRequested[SOUND_CRACK]);
    game.thrown.clear();
    yeti.attackTimer = 0.001f;
    updateGame(game, &NO_INPUT, DT);
    CHECK(yeti.windupTimer > 0 && strcmp(yeti.say, "AH...") == 0 && game.thrown.empty());
    run(game, NO_INPUT, 0.5f);
    CHECK(countThrown(game, THROWN_SNOWBALL) == 7 && strcmp(yeti.say, "ACHOO!") == 0 && yeti.sayTimer > 0);
    CHECK(game.thrown[0].vx == -60 && game.thrown[3].vx == 0 && game.thrown[6].vx == 60 && game.thrown[3].vy == 80);
}

static void golemIciclesHangThenFall() {
    Game game;
    startBossGame(game, 15, 3);
    bool rained = false, blocks = false;
    for (int i = 0; i < 60; i++) {
        game.bosses[0].attackTimer = 0.001f;
        updateGame(game, &NO_INPUT, DT);
        blocks |= countThrown(game, THROWN_ICE_BLOCK) == 2;
        rained |= countThrown(game, THROWN_ICICLE) == 6;
        game.thrown.clear();
    }
    CHECK(rained && blocks);
    for (int i = 0; i < 60 && countThrown(game, THROWN_ICICLE) == 0; i++) {
        game.thrown.clear();
        game.bosses[0].attackTimer = 0.001f;
        updateGame(game, &NO_INPUT, DT);
    }
    CHECK(countThrown(game, THROWN_ICICLE) == 6);
    int lane = 0;
    for (const Thrown& t : game.thrown) {
        if (t.kind != THROWN_ICICLE) continue;
        CHECK(t.y == 0 && t.delay > 0.7f && t.x >= lane * 52.66f && t.x < (lane + 1) * 52.67f);
        lane++;
    }
    game.bosses[0].attackTimer = 100;
    run(game, NO_INPUT, 0.7f);
    for (const Thrown& t : game.thrown) if (t.kind == THROWN_ICICLE) CHECK(t.y == 0);
    run(game, NO_INPUT, 0.3f);
    for (const Thrown& t : game.thrown) if (t.kind == THROWN_ICICLE) CHECK(t.y > 0);
}

static void lavaDropScorchesForDoubleDamage() {
    Game game;
    startSafeGame(game, 4);
    game.aliens.clear();
    game.aliens.push_back(parkedAlien(100, 20));
    game.aliens[0].type = ALIEN_RED;
    game.aliens[0].hp = 4;
    game.swayDirection = 0;
    game.diveTimer = 1e9f;   // it stays parked
    Player& p = game.players[0];
    game.thrown.push_back({p.x + 5, p.y + 4, 0, 0, THROWN_LAVA_DROP, 0, 0});
    updateGame(game, &NO_INPUT, DT);
    CHECK(p.scorchTimer > 2.9f && p.alive && game.thrown.empty() && game.soundRequested[SOUND_SIZZLE]);
    Actions right = {};
    right.moveX = 1;
    float x = p.x;
    run(game, right, 0.5f);
    CHECK(fabsf(p.x - x - 45) < 1.5f);   // 0.6 x 150 px/s
    Shot bullet = {104, 25, 0, 0, 0};
    game.bullets.push_back(bullet);
    updateGame(game, &NO_INPUT, DT);
    CHECK(game.aliens[0].hp == 2);
    run(game, NO_INPUT, 2.6f);
    CHECK(p.scorchTimer == 0);
    game.bullets.push_back(bullet);
    updateGame(game, &NO_INPUT, DT);
    CHECK(game.aliens[0].hp == 1);
}

static void impCyclesFireballsLavaAndFan() {
    Game game;
    startBossGame(game, 5, 4);
    Boss& imp = game.bosses[0];
    imp.attackTimer = 0.001f;
    run(game, NO_INPUT, 0.4f);
    CHECK(countThrown(game, THROWN_FIREBALL) == 3);
    for (const Thrown& t : game.thrown) CHECK(fabsf(thrownSpeed(t) - 95) < 0.5f);
    game.thrown.clear();
    imp.attackTimer = 0.001f;
    updateGame(game, &NO_INPUT, DT);
    CHECK(countThrown(game, THROWN_BIG_LAVA) == 1 && countThrown(game, THROWN_LAVA_DROP) == 0 && game.thrown[0].vx == 0 && game.thrown[0].vy == 60);
    game.thrown.clear();
    imp.attackTimer = 0.001f;
    updateGame(game, &NO_INPUT, DT);
    CHECK(countThrown(game, THROWN_FIREBALL) == 5 && game.thrown[0].vx == -60 && game.thrown[4].vx == 60);
}

static void eruptionKillsOnlyInItsColumn() {
    for (int offset : {0, 30}) {
        Game game;
        startBossGame(game, 15, 4);
        Player& p = game.players[0];
        p.invincibleTimer = 0;
        Boss& lord = game.bosses[0];
        lord.ballX = p.x + 8 + offset;   // the ship's centre
        lord.ballTimer = 0.5f;
        run(game, NO_INPUT, 0.45f);
        CHECK(p.alive && lord.beamTimer == 0);   // the warning doesn't hurt
        run(game, NO_INPUT, 0.3f);
        CHECK(p.alive == (offset != 0));
    }
}

static void fireLordRollsAllFourAttacks() {
    Game game;
    startBossGame(game, 15, 4);
    bool volley = false, rain = false, erupt = false;
    for (int i = 0; i < 60; i++) {
        game.thrown.clear();
        game.bosses[0].ballTimer = game.bosses[0].beamTimer = 0;
        game.bosses[0].attackTimer = 0.001f;
        updateGame(game, &NO_INPUT, DT);
        volley |= game.bosses[0].burstShotsLeft > 0;
        rain |= countThrown(game, THROWN_LAVA_DROP) == 5;
        erupt |= game.bosses[0].ballTimer > 0;
    }
    CHECK(volley && rain && erupt);
}

static void currentPushesShipsInTheDeepSea() {
    for (int level : {1, 5}) {
        Game game;
        startSafeGame(game, level);
        float x = game.players[0].x;
        run(game, NO_INPUT, 3);
        float drift = game.players[0].x - x;
        if (level == 1) CHECK(drift == 0);
        else CHECK(drift > 27 && drift < 35);   // 18 / 0.45 x (1 - cos 1.35)
    }
}

static void anglerCyclesStreamBigBubbleAndFan() {
    Game game;
    startBossGame(game, 5, 5);
    Boss& angler = game.bosses[0];
    angler.attackTimer = 0.001f;
    run(game, NO_INPUT, 0.6f);
    CHECK(countThrown(game, THROWN_BUBBLE) == 5);
    game.thrown.clear();
    angler.attackTimer = 0.001f;
    updateGame(game, &NO_INPUT, DT);
    CHECK(countThrown(game, THROWN_BIG_BUBBLE) == 1);
    run(game, NO_INPUT, 1.5f);
    CHECK(countThrown(game, THROWN_BIG_BUBBLE) == 0 && countThrown(game, THROWN_BUBBLE) == 8 && game.soundRequested[SOUND_BUBBLE]);
    game.thrown.clear();
    angler.attackTimer = 0.001f;
    updateGame(game, &NO_INPUT, DT);
    CHECK(countThrown(game, THROWN_BUBBLE) == 7 && game.thrown[0].vx == -70 && game.thrown[6].vx == 70);
}

// A fisherman whose hook drops at the ship; its other attacks held.
static void castAtTheShip(Game& game) {
    startBossGame(game, 15, 5);
    Boss& boss = game.bosses[0];
    boss.hookState = HOOK_DOWN;
    boss.hookX = game.players[0].x + 8;   // the ship's centre
    boss.hookY = boss.y + 1;
}

static void hookLiftsAndStunsTheShip() {
    Game game;
    castAtTheShip(game);
    Player& p = game.players[0];
    p.invincibleTimer = 0;
    float y = p.y;
    Boss& boss = game.bosses[0];
    for (int step = 0; step < 78; step++) {   // the current drifts the ship: the hook follows it down
        if (boss.hookState == HOOK_DOWN) boss.hookX = p.x + 8;
        updateGame(game, &NO_INPUT, DT);
    }
    CHECK(p.alive && p.hookedTimer > 1.4f && boss.hookState == HOOK_HOLD && strcmp(boss.say, "GOT ONE!") == 0);
    CHECK(game.soundRequested[SOUND_HOOKED]);
    Actions push = {};
    push.moveX = 1;
    push.fire = true;
    float x = p.x;
    run(game, push, 0.6f);
    CHECK(p.x == x && fabsf(p.y - (y - 24)) < 0.5f && game.bullets.empty());   // lifted 24 px, can't move or shoot
    run(game, push, 1.1f);
    CHECK(p.hookedTimer == 0 && boss.hookState == HOOK_REEL && p.x > x && !game.bullets.empty());
}

static void invincibleShipsSlipOffTheHook() {
    Game game;
    castAtTheShip(game);
    run(game, NO_INPUT, 3.5f);
    CHECK(game.players[0].hookedTimer == 0 && game.bosses[0].hookState == HOOK_NONE);
}

static void emptyCastsSometimesBringABoot() {
    Game game;
    startBossGame(game, 15, 5);
    Boss& boss = game.bosses[0];
    int boots = 0;
    for (int cast = 0; cast < 40; cast++) {
        game.thrown.clear();
        boss.hookState = HOOK_DOWN;
        boss.hookX = 4;   // far from the ship
        boss.hookY = boss.y + 1;
        boss.hookCaught = false;
        boss.attackTimer = 100;
        while (boss.hookState != HOOK_NONE) updateGame(game, &NO_INPUT, DT);
        if (countThrown(game, THROWN_BOOT) == 1) {
            boots++;
            CHECK(strcmp(boss.say, "OLD BOOT") == 0 && game.thrown.size() == 1 && fabsf(thrownSpeed(game.thrown[0]) - 85) < 0.5f);
        }
    }
    CHECK(boots >= 6 && boots <= 24);   // 35 % of 40
}

static void playerTwoShipIsGold() {
    CHECK(strchr(SHIP2_A.pixels, 'Y') && !strchr(SHIP2_A.pixels, 'b'));
    CHECK(strchr(SHIP_A.pixels, 'b') && !strchr(SHIP_A.pixels, 'Y'));
}

static void mageBallFormsAtTheWand() {
    Game game;
    startBossGame(game, 15);
    Boss& mage = game.bosses[0];
    bool formed = false;
    for (int i = 0; i < 60 && !formed; i++) {
        mage.ballTimer = mage.beamTimer = 0;
        mage.attackTimer = 0.001f;
        updateGame(game, &NO_INPUT, DT);
        if (mage.ballTimer > 0) {
            CHECK(mage.ballX == mage.x + 30 && mage.ballY == mage.y + 8);
            formed = true;
        }
    }
    CHECK(formed);
}

static void bigIceBlockFreezesFromItsCorner() {
    Game game;
    startSafeGame(game, 3);
    Player& p = game.players[0];
    game.thrown.push_back({p.x - 17, p.y - 6, 0, 0, THROWN_ICE_BLOCK, 0, 0});
    updateGame(game, &NO_INPUT, DT);
    CHECK(p.frozenTimer > 1.5f && game.thrown.empty());
}

static void seeThroughIceTintsWhatIsUnder() {
    fillRect(0, 0, 30, 30, 0xFF0C0F1E);
    drawSprite(ICE_BLOCK, 0, 0);
    CHECK(screenPixels[12 * SCREEN_W + 10] == (0xFF000000u | (((0xFF0C0F1Eu >> 1) & 0x7F7F7F) + ((0xFF0C0F1Eu >> 2) & 0x3F3F3F) + ((0xFF9FE0FFu >> 2) & 0x3F3F3F))));
    CHECK(screenPixels[12 * SCREEN_W + 10] != 0xFF0C0F1E);
}

static void bubbleTrailOnlyWhileMovingInTheDeepSea() {
    for (int level = 1; level <= 5; level += 4) {
        Game game;
        startSafeGame(game, level);
        game.particles.clear();
        run(game, NO_INPUT, 0.5f);
        CHECK(game.particles.empty());
        Actions right = NO_INPUT;
        right.moveX = 1;
        run(game, right, 0.5f);
        if (level == 1) {
            CHECK(game.particles.empty());
        } else {
            CHECK(game.particles.size() >= 8);
            for (const Particle& p : game.particles) CHECK(p.vy < 0);
        }
    }
}

static void fishermanKeepsAtMostThreeLiveFishOut() {
    Game game;
    startBossGame(game, 15, 5);
    int most = 0;
    for (int i = 0; i < 200; i++) {
        game.bosses[0].attackTimer = 0.001f;
        game.bosses[0].hookState = HOOK_NONE;
        updateGame(game, &NO_INPUT, DT);
        int out = (int)game.fish.size();
        for (const Lob& lob : game.lobs) {
            if (!lob.fish) continue;
            out++;
            CHECK(lob.toX >= 8 && lob.toX <= 312 && lob.toY >= 107 && lob.toY <= 173);
        }
        CHECK(out <= 3);
        most = std::max(most, out);
    }
    CHECK(most == 3);
}

static void liveFishLandsAndChasesTheShip() {
    Game game;
    startBossGame(game, 15, 5);
    game.bosses[0].attackTimer = 100;
    game.lobs.push_back({100, 40, 200, 120, 0, false, true});
    run(game, NO_INPUT, 0.85f);
    CHECK(game.fish.size() == 1 && game.lobs.empty() && game.soundRequested[SOUND_SPLAT]);
    Player& p = game.players[0];
    game.fish[0] = {p.x + 60, p.y + 4, 6, false};   // level with the ship's centre
    float startX = game.fish[0].x;
    run(game, NO_INPUT, 0.5f);
    CHECK(game.fish[0].left && fabsf(startX - game.fish[0].x - 17.5f) < 1);
}

static void liveFishKillsAndDies() {
    Game game;
    startSafeGame(game, 5);
    Player& p = game.players[0];
    p.invincibleTimer = 0;
    game.fish.push_back({p.x + 4, p.y + 4, 6, true});
    updateGame(game, &NO_INPUT, DT);
    CHECK(!p.alive && game.fish.empty());
}

static void shotsAndTimeEndLiveFish() {
    Game game;
    startSafeGame(game, 5);
    game.fish.push_back({100, 120, 6, true});
    game.bullets.push_back({104, 126, -320, 0, 0});
    updateGame(game, &NO_INPUT, DT);
    CHECK(game.fish.empty() && game.players[0].score == 200);
    game.fish.push_back({20, 170, 6, true});   // the ship is invincible
    run(game, NO_INPUT, 5.9f);
    CHECK(game.fish.size() == 1);
    run(game, NO_INPUT, 0.2f);
    CHECK(game.fish.empty());
}

static void menuCursorHasAGlyph() {
    CHECK(fontGlyph('>') != 0);
}

static void shipSkinsKeepTheBody() {
    const Sprite* ships[] = {&SHIP_JUNGLE_A, &SHIP_JUNGLE_B, &SHIP_GLACIER_A, &SHIP_GLACIER_B, &SHIP_VOLCANO_A, &SHIP_VOLCANO_B, &SHIP_SEA_A, &SHIP_SEA_B};
    const Sprite* ships2[] = {&SHIP2_JUNGLE_A, &SHIP2_JUNGLE_B, &SHIP2_GLACIER_A, &SHIP2_GLACIER_B, &SHIP2_VOLCANO_A, &SHIP2_VOLCANO_B, &SHIP2_SEA_A, &SHIP2_SEA_B};
    for (const Sprite* s : ships) CHECK(s->w == 16 && s->h == 14);
    for (const Sprite* s : ships2) CHECK(s->w == 16 && s->h == 14 && strchr(s->pixels, 'c') == NULL);
}

static void bigBossesDropTheLevelBuff() {
    Game game;
    startBossGame(game, 15, 3);
    game.bosses[0].hp = 1;
    Shot bullet = {game.bosses[0].x + 10, game.bosses[0].y + 5, 0, 0, 0};
    game.bullets.push_back(bullet);
    updateGame(game, &NO_INPUT, DT);
    int buffs = 0, level = 0;
    for (const Pickup& pickup : game.pickups)
        if (pickup.type == PICKUP_BUFF) { buffs++; level = pickup.level; }
    CHECK(buffs == 1 && level == 3);
    int bigUpgrades = 0;
    for (const Pickup& pickup : game.pickups) bigUpgrades += pickup.type == PICKUP_UPGRADE;
    CHECK(bigUpgrades == 4);   // a big boss drops 4 upgrades
    startBossGame(game, 10, 3);
    for (Boss& boss : game.bosses) boss.hp = 1;
    for (const Boss& boss : game.bosses) {
        Shot shot = {boss.x + 10, boss.y + 5, 0, 0, 0};
        game.bullets.push_back(shot);
    }
    updateGame(game, &NO_INPUT, DT);
    buffs = 0;
    for (const Pickup& pickup : game.pickups) buffs += pickup.type == PICKUP_BUFF;
    CHECK(buffs == 0 && !game.bosses[0].alive && !game.bosses[1].alive);
}

static void buffPickupGivesTheBuffOnce() {
    Game game;
    startSafeGame(game, 2);
    Player& p = game.players[0];
    game.pickups.push_back({p.x + 4, p.y, PICKUP_BUFF, 2});
    updateGame(game, &NO_INPUT, DT);
    CHECK(p.buffs == BUFF_JUNGLE && p.strongAttack == 0 && game.soundRequested[SOUND_EXTRA_LIFE]);
    int score = p.score;
    game.pickups.push_back({p.x + 4, p.y, PICKUP_BUFF, 2});
    updateGame(game, &NO_INPUT, DT);
    CHECK(p.buffs == BUFF_JUNGLE && p.score == score + 5000);
    game.pickups.push_back({p.x + 4, p.y, PICKUP_BUFF, 1});
    updateGame(game, &NO_INPUT, DT);
    CHECK(p.buffs == (BUFF_JUNGLE | BUFF_SPACE) && p.strongAttack == 1);
}

static void dyingLosesEveryBuff() {
    Game game;
    startSafeGame(game);
    Player& p = game.players[0];
    p.invincibleTimer = 0;
    p.buffs = BUFF_ALL;
    p.strongAttack = 3;
    Shot shot = {p.x + 7, p.y + 5, 0, -1, 0};
    game.enemyShots.push_back(shot);
    updateGame(game, &NO_INPUT, DT);
    CHECK(!p.alive && p.buffs == 0 && p.strongAttack == 0);
}

static void squareCyclesHeldStrongAttacks() {
    Game game;
    startSafeGame(game);
    Player& p = game.players[0];
    Actions press = {};
    press.cycle = true;
    p.buffs = BUFF_SPACE | BUFF_JUNGLE | BUFF_SEA;
    const int expected[4] = {1, 5, 0, 1};
    for (int i = 0; i < 4; i++) {
        updateGame(game, &press, DT);
        CHECK(p.strongAttack == expected[i]);
    }
    p.buffs = 0;
    p.strongAttack = 0;
    updateGame(game, &press, DT);
    CHECK(p.strongAttack == 0);
}

static void magicBeamHitsItsColumnOnce() {
    Game game;
    startSafeGame(game);
    game.aliens.clear();
    game.swayDirection = 0;
    game.diveTimer = 1e9f;
    const float xs[4] = {158, 158, 158, 100}, ys[4] = {20, 40, 60, 40};
    for (int i = 0; i < 4; i++) {
        game.aliens.push_back(parkedAlien(xs[i], ys[i]));
        game.aliens.back().hp = 40;
    }
    Player& p = game.players[0];
    p.buffs = BUFF_SPACE;
    p.strongAttack = 1;
    Actions r2 = {};
    r2.rocket = true;
    updateGame(game, &r2, DT);
    CHECK(p.rockets == 2 && p.beamCharge > 0);
    p.x = 20;   // moving away doesn't move the column
    run(game, NO_INPUT, 0.5f);
    CHECK(game.aliens[0].hp == 40 && p.beamTimer == 0);   // still charging
    run(game, NO_INPUT, 0.5f);
    CHECK(game.aliens[0].hp == 10 && game.aliens[1].hp == 10 && game.aliens[2].hp == 10 && game.aliens[3].hp == 40 && p.beamTimer > 0);
    run(game, NO_INPUT, 0.5f);
    CHECK(game.aliens[0].hp == 10 && game.aliens[1].hp == 10 && game.aliens[2].hp == 10);
}

static void magicBeamHitsWhenItReachesTheTop() {
    Game game;
    startSafeGame(game);
    game.aliens.clear();
    game.swayDirection = 0;
    game.diveTimer = 1e9f;
    for (int i = 0; i < 3; i++) {
        game.aliens.push_back(parkedAlien(158, 20 + i * 20));
        game.aliens.back().hp = 40;
    }
    Player& p = game.players[0];
    p.buffs = BUFF_SPACE;
    p.strongAttack = 1;
    Actions r2 = {};
    r2.rocket = true;
    updateGame(game, &r2, DT);
    for (int step = 0; step < 120 && p.beamTimer == 0; step++) updateGame(game, &NO_INPUT, DT);
    CHECK(p.beamTimer > 0 && game.aliens[0].hp == 40 && game.aliens[1].hp == 40 && game.aliens[2].hp == 40 && game.trauma > 0.6f);
    run(game, NO_INPUT, 0.15f);
    CHECK(game.aliens[0].hp == 10 && game.aliens[1].hp == 10 && game.aliens[2].hp == 10 && game.trauma >= 0.44f);
    run(game, NO_INPUT, 0.3f);
    int shards = 0;
    for (const Particle& q : game.particles) shards += q.h == 2;
    CHECK(p.beamTimer == 0 && shards >= 20);
}

static void mageBeamKillsOnlyOnceGrown() {
    Game game;
    startBossGame(game, 15);
    Boss& mage = game.bosses[0];
    Player& p = game.players[0];
    p.invincibleTimer = 0;
    mage.ballX = p.x + 8;
    mage.ballY = 21;
    mage.ballTimer = DT / 2;
    updateGame(game, &NO_INPUT, DT);
    CHECK(mage.beamTimer > 0 && p.alive);
    run(game, NO_INPUT, 0.15f);
    CHECK(!p.alive);
}

static void mageBeamRumblesThenShatters() {
    Game game;
    startBossGame(game, 15);
    Boss& mage = game.bosses[0];
    game.players[0].invincibleTimer = 1e9f;
    mage.ballX = 20;
    mage.ballY = 21;
    mage.ballTimer = DT / 2;
    run(game, NO_INPUT, 0.5f);
    CHECK(game.trauma >= 0.44f);
    run(game, NO_INPUT, 0.4f);
    int shards = 0;
    for (const Particle& q : game.particles) shards += q.h == 2;
    CHECK(mage.beamTimer == 0 && shards >= 20);
}

static void eruptionLeavesNoShards() {
    Game game;
    startBossGame(game, 15, 4);
    Boss& lord = game.bosses[0];
    game.players[0].invincibleTimer = 1e9f;
    lord.ballX = 20;
    lord.ballTimer = DT / 2;
    run(game, NO_INPUT, 0.7f);
    int shards = 0;
    for (const Particle& q : game.particles) shards += q.h == 2;
    CHECK(lord.beamTimer == 0 && shards == 0);
}

static void frostWaveFreezesHealsAndShatters() {
    Game game;
    startBossGame(game, 15, 3);
    Boss& boss = game.bosses[0];
    boss.hp = 100;
    Shot shot = {20, 60, 0, -1, 0};
    game.enemyShots.push_back(shot);
    shot.x = 40;
    game.enemyShots.push_back(shot);
    game.thrown.push_back({60, 60, 0, 0, THROWN_SNOWBALL, 0, 0});
    Player& p = game.players[0];
    p.buffs = BUFF_GLACIER;
    p.strongAttack = 3;
    Actions r2 = {};
    r2.rocket = true;
    updateGame(game, &r2, DT);
    CHECK(game.trauma > 0.5f);
    CHECK(game.freezeTimer > 2.9f && boss.hp == 185 && game.enemyShots.empty() && game.thrown.empty() && p.rockets == 2 && game.soundRequested[SOUND_FREEZE]);
    boss.attackTimer = 0.05f;
    float x = boss.x;
    run(game, NO_INPUT, 1);
    CHECK(boss.x == x && game.enemyShots.empty() && game.thrown.empty());
    Shot bullet = {boss.x + 10, boss.y + 5, 0, 0, 0};
    game.bullets.push_back(bullet);
    updateGame(game, &NO_INPUT, DT);
    CHECK(boss.hp == 184);
    run(game, NO_INPUT, 2.1f);
    CHECK(game.freezeTimer == 0);
    run(game, NO_INPUT, 0.5f);
    CHECK(boss.x != x);
}

static void volcanoBuffFiresAQuarterFaster() {
    Actions fire = {};
    fire.fire = true;
    Game game;
    startSafeGame(game);
    updateGame(game, &fire, DT);
    CHECK(fabsf(game.players[0].fireTimer - (0.12f - DT)) < 0.001f);
    startSafeGame(game);
    game.players[0].buffs = BUFF_VOLCANO;
    updateGame(game, &fire, DT);
    CHECK(fabsf(game.players[0].fireTimer - (0.096f - DT)) < 0.001f);
}

static void bubbleShieldTakesAHitAndChains() {
    Game game;
    startSafeGame(game, 5);
    game.aliens.clear();
    game.aliens.push_back(parkedAlien(100, 30));
    game.aliens[0].hp = 200;
    game.swayDirection = 0;
    game.diveTimer = 1e9f;
    Player& p = game.players[0];
    p.buffs = BUFF_SEA;
    p.strongAttack = 5;
    Actions r2 = {};
    r2.rocket = true;
    updateGame(game, &r2, DT);
    CHECK(p.shield && p.rockets == 2);
    run(game, NO_INPUT, 0.6f);
    updateGame(game, &r2, DT);
    CHECK(p.rockets == 2);
    p.invincibleTimer = 0;
    Shot shot = {p.x + 7, p.y + 5, 0, -1, 0};
    game.enemyShots.push_back(shot);
    updateGame(game, &NO_INPUT, DT);
    CHECK(p.alive && !p.shield && p.invincibleTimer > 0.9f && game.shieldBubbles.size() == 3 && game.trauma > 0.5f);
    p.invincibleTimer = 1e9f;
    run(game, NO_INPUT, 4);
    if (game.aliens[0].hp != 200 - 78) printf("bubble chain left hp %g\n", game.aliens[0].hp);
    CHECK(game.aliens[0].hp == 200 - 78 && game.shieldBubbles.empty());
}

static void classicKeepsBuffsIntoTheNextLevel() {
    Game game;
    startBossGame(game, 20, 1);
    game.players[0].buffs = BUFF_SPACE;
    game.bosses[1].alive = false;
    game.bosses[0].hp = 1;
    Shot bullet = {game.bosses[0].x + 10, game.bosses[0].y + 5, 0, 0, 0};
    game.bullets.push_back(bullet);
    updateGame(game, &NO_INPUT, DT);
    run(game, NO_INPUT, 3.4f);
    CHECK(game.level == 2 && (game.players[0].buffs & BUFF_SPACE));
}

static void classicVariantsCapThePartyAndReadPlayerFour() {
    Game game;
    startGame(game, 1, 7);
    CHECK(game.arenaWidth() == 320 && game.arenaHeight() == 180);
    CHECK(joinGame(game));
    CHECK(!joinGame(game) && game.players.size() == 2);

    startGame(game, 1, 7, 1, false, CLASSIC_EXPANDED);
    CHECK(game.arenaWidth() == 384 && game.arenaHeight() == 216);
    CHECK(joinGame(game) && joinGame(game) && joinGame(game));
    if (game.players.size() != 4) return;
    CHECK(!joinGame(game));
    Player& fourth = game.players[3];
    fourth.x = (float)(EXPANDED_W - 17);
    fourth.y = (float)(EXPANDED_H - 15);
    Actions actions[MAX_PLAYERS] = {};
    actions[3].fire = true;
    actions[3].moveX = actions[3].moveY = 1;
    float firstX = game.players[0].x;
    updateGame(game, actions, DT);
    CHECK(game.players[0].x == firstX && fourth.x == EXPANDED_W - 16 && fourth.y == EXPANDED_H - 14);
    CHECK(game.bullets.size() == 1 && game.bullets[0].owner == 3);
}

static void rolesRequireThreeExpandedPlayers() {
    Game game;
    const ShipRole roles[] = {ROLE_TANK, ROLE_MAGE, ROLE_ASSASSIN, ROLE_TANK};
    for (bool singleLevel : {false, true}) {
        startGame(game, 4, 7, 1, singleLevel, singleLevel ? CLASSIC_EXPANDED : CLASSIC_STANDARD, roles);
        CHECK(game.players.size() == 2 && game.players[0].role == ROLE_FIGHTER);
    }
    startGame(game, 2, 7, 1, false, CLASSIC_EXPANDED, roles);
    CHECK(game.players[0].role == ROLE_FIGHTER && game.players[1].role == ROLE_FIGHTER);
    game.players[0].score = 800;
    CHECK(joinGame(game, roles));
    CHECK(game.players[0].role == ROLE_TANK && game.players[1].role == ROLE_MAGE && game.players[2].role == ROLE_ASSASSIN);
    CHECK(game.players[0].score == 800);
    const ShipRole fourthPick[] = {ROLE_FIGHTER, ROLE_FIGHTER, ROLE_FIGHTER, ROLE_MAGE};
    CHECK(joinGame(game, fourthPick));
    CHECK(game.players[0].role == ROLE_TANK && game.players[3].role == ROLE_MAGE);
}

static void rolesApplyMovementFireCadenceAndDamage() {
    Game game;
    const ShipRole roles[] = {ROLE_FIGHTER, ROLE_TANK, ROLE_ASSASSIN, ROLE_MAGE};
    startGame(game, 4, 7, 1, false, CLASSIC_EXPANDED, roles);
    game.aliens.clear();
    game.freezeTimer = 10;
    Actions actions[MAX_PLAYERS] = {};
    float starts[MAX_PLAYERS];
    for (int p = 0; p < MAX_PLAYERS; p++) {
        const Player& player = game.players[p];
        starts[p] = player.x;
        game.aliens.push_back(parkedAlien(player.x + 6, player.y - 14));
        game.aliens.back().hp = 10;
        actions[p].moveX = 1;
        actions[p].fire = true;
    }
    updateGame(game, actions, DT);
    const float expectedHp[] = {9, 8, 9.25f, 8.5f};
    for (int p = 0; p < MAX_PLAYERS; p++) CHECK(game.aliens[p].hp == expectedHp[p]);
    float fighterMove = game.players[0].x - starts[0];
    float fighterFireTimer = game.players[0].fireTimer;
    CHECK(game.players[1].x - starts[1] < fighterMove && game.players[1].fireTimer > fighterFireTimer);
    CHECK(game.players[2].x - starts[2] > fighterMove && game.players[2].fireTimer < fighterFireTimer);
    CHECK(game.players[3].x - starts[3] == fighterMove && game.players[3].fireTimer == fighterFireTimer);
}

static void tankArmorAndHeavyRocket() {
    Game game;
    const ShipRole roles[] = {ROLE_TANK, ROLE_FIGHTER, ROLE_ASSASSIN};
    startGame(game, 3, 7, 1, false, CLASSIC_EXPANDED, roles);
    Player& tank = game.players[0];
    game.freezeTimer = 10;
    Actions actions[MAX_PLAYERS] = {};
    game.enemyShots.emplace_back(tank.x + 7, tank.y + 4, 0, -1, 0);
    updateGame(game, actions, DT);
    CHECK(tank.alive && tank.lives == 3 && tank.armorSpent && tank.invincibleTimer > 0);
    game.enemyShots.emplace_back(tank.x + 7, tank.y + 4, 0, -1, 0);
    updateGame(game, actions, DT);
    CHECK(tank.alive && tank.lives == 3);
    for (int step = 0; step < 48; step++) updateGame(game, actions, DT);
    CHECK(!tank.alive && tank.lives == 2);
    for (int step = 0; step < 120; step++) updateGame(game, actions, DT);
    CHECK(tank.alive && !tank.armorSpent);

    actions[0].rocket = actions[1].rocket = true;
    updateGame(game, actions, DT);
    CHECK(game.rockets.size() == 2 && game.rockets[0].heavy && !game.rockets[1].heavy);
    if (game.rockets.size() != 2) return;
    CHECK(game.rockets[0].speed > 260 && game.rockets[1].speed < 260);
    float rocketX = game.rockets[0].x;
    game.aliens.push_back(parkedAlien(rocketX - 57, 0));
    game.aliens.back().hp = 24;
    game.rockets[0].y = game.rockets[1].y = 0;
    actions[0].rocket = actions[1].rocket = false;
    updateGame(game, actions, DT);
    CHECK(game.rockets.empty() && tank.rockets == 2);
    CHECK(game.aliens.back().hp == 4);   // outside the ordinary blast radius
    CHECK(std::any_of(game.explosions.begin(), game.explosions.end(), [rocketX](const Explosion& e) {
        return e.x == rocketX + 3 && e.radius == 60;
    }));
}

static void assassinPhasesThroughAttacksForFiveSeconds() {
    Game game;
    const ShipRole roles[] = {ROLE_ASSASSIN, ROLE_FIGHTER, ROLE_MAGE};
    startGame(game, 3, 7, 1, false, CLASSIC_EXPANDED, roles);
    Player& assassin = game.players[0];
    game.freezeTimer = 10;
    game.enemyShots.emplace_back(assassin.x + 7, assassin.y + 4, 0, -1, 0);
    game.thrown.push_back({assassin.x + 4, assassin.y + 2, 0, 0, THROWN_ICE_BLOCK, 0, 0});
    Actions actions[MAX_PLAYERS] = {};
    actions[0].rocket = true;
    updateGame(game, actions, DT);
    CHECK(assassin.alive && assassin.phaseTimer == 5 && assassin.rockets == 2 && assassin.frozenTimer == 0);
    CHECK(game.enemyShots.size() == 1 && game.thrown.size() == 1);
    actions[0].rocket = false;
    for (int step = 0; step < 60; step++) updateGame(game, actions, DT);
    actions[0].rocket = true;
    updateGame(game, actions, DT);
    CHECK(assassin.rockets == 2 && assassin.phaseTimer < 4);
    actions[0].rocket = false;
    for (int step = 0; step < 237; step++) updateGame(game, actions, DT);
    CHECK(assassin.alive && assassin.phaseTimer > 0);
    for (int step = 0; step < 3; step++) updateGame(game, actions, DT);
    CHECK(!assassin.alive && assassin.phaseTimer == 0 && assassin.lives == 2);
}

static void mageOrbsPierceEachEnemyOnceAndHalveDamage() {
    Game game;
    const ShipRole roles[] = {ROLE_MAGE, ROLE_TANK, ROLE_FIGHTER};
    startGame(game, 3, 7, 1, false, CLASSIC_EXPANDED, roles);
    game.aliens.clear();
    game.freezeTimer = 10;
    const Player& mage = game.players[0];
    for (int y : {30, 65, 100, 135}) {
        game.aliens.push_back(parkedAlien(mage.x + 2, (float)y));
        game.aliens.back().hp = 10;
    }
    Actions actions[MAX_PLAYERS] = {};
    actions[0].rocket = true;
    updateGame(game, actions, DT);
    CHECK(mage.rockets == 2 && game.rockets.empty() && game.bullets.size() == 7);
    if (game.bullets.size() != 7) return;
    CHECK(game.bullets.front().vx < 0 && game.bullets.back().vx > 0);
    Shot center = game.bullets[3];
    game.bullets.assign(1, center);
    actions[0].rocket = false;
    for (int step = 0; step < 48; step++) updateGame(game, actions, DT);
    const float expectedHp[] = {9.5f, 9, 8, 6};
    for (int i = 0; i < 4; i++) CHECK(game.aliens[i].hp == expectedHp[i]);
}

static void expandedClassicScalesEveryWave() {
    Game standard, expanded;
    startGame(standard, 1, 7);
    startGame(expanded, 4, 7, 1, false, CLASSIC_EXPANDED);
    Actions actions[MAX_PLAYERS] = {};
    for (int level = 1; level <= LEVEL_COUNT; level++) {
        for (int wave = 1; wave <= 20; wave++) {
            CHECK(standard.level == level && expanded.level == level && standard.wave == wave && expanded.wave == wave);
            if (isBossWave(wave)) {
                CHECK(expanded.aliens.empty() && expanded.bosses.size() == standard.bosses.size() + 1);
                float lane = (float)EXPANDED_W / expanded.bosses.size();
                for (size_t i = 0; i < expanded.bosses.size(); i++) {
                    const Boss& boss = expanded.bosses[i];
                    CHECK(boss.type == bossType(level, wave) && boss.hp == (bossHealth(level, wave) * 9 + 3) / 4);
                    CHECK(boss.minX == i * lane && boss.maxX + 32 == (i + 1) * lane);
                }
            } else {
                int standardCounts[4] = {}, expandedCounts[4] = {};
                int standardHp[4] = {0, 0, 0, 12 + alienHpBonus(difficultyWave(level, wave))};
                for (const Alien& a : standard.aliens) {
                    standardCounts[a.type]++;
                    standardHp[a.type] = a.maxHp;
                }
                for (const Alien& a : expanded.aliens) {
                    expandedCounts[a.type]++;
                    CHECK(a.slotX >= 40 && a.slotX + (a.type == ALIEN_HEAVY ? 16 : 12) + 40 <= EXPANDED_W);
                    CHECK(a.hp == (standardHp[a.type] * 9 + 3) / 4 && a.hp == a.maxHp);
                }
                for (int type = ALIEN_GREEN; type <= ALIEN_RED; type++) CHECK(expandedCounts[type] == standardCounts[type] * 3 / 2);
                CHECK(expandedCounts[ALIEN_HEAVY] == standardCounts[ALIEN_HEAVY] + (wave >= 11 ? 2 : 1));
            }
            for (Game* game : {&standard, &expanded}) {
                for (Player& p : game->players) p.invincibleTimer = 1e9f;
                for (Alien& alien : game->aliens) alien.alive = false;
                for (Boss& boss : game->bosses) boss.alive = false;
                updateGame(*game, actions, DT);
                updateGame(*game, actions, 3.05f);
            }
        }
    }
    CHECK(standard.won && expanded.won && expanded.players.size() == 4 && expanded.variant == CLASSIC_EXPANDED);
}

int main() {
    spritesHaveTheirSize();
    settingsDefaultToMizrahit();
    settingsSurviveASave();
    oldMusicSettingsStillLoad();
    shakeAndFlashSettingsSurviveASave();
    highScoreDefaultsToZero();
    highScoreSurvivesASave();
    highScoreIsClamped();
    menuCursorHasAGlyph();
    onlyDeadlyShotsUseMagenta();
    deadzoneIsRadialAndRescaled();
    holdingFireShootsEvery012Seconds();
    shipStaysInsideMovementZone();
    formationGrowsToFiveRows();
    clearingAWaveStartsTheNextAfterThePause();
    enemyFormulasFollowTheDesign();
    aliensFlyInBeforeTheyCanShoot();
    divesStartAtWaveThree();
    enemyShotsSpeedUpAfterWaveTen();
    lateWavesAddArmouredRows();
    twoAliensDiveFromWaveFifteen();
    rocketBufferFiresAnEarlyPress();
    emptyRocketClicks();
    blastKillsOnlyInsideItsRadius();
    extraLivesAtEvery50000();
    threeDeathsEndTheRun();
    nextWaveWaitsForALivingShip();
    alienBodyKillsTheShip();
    pickupAddsARocketUpToNine();
    weaponLevelsFireTheirPattern();
    lateDifficultyAddsHpAndMovement();
    wavesUseRandomPatterns();
    tracedShapesFit();
    killDropsFollowTheRoll(CLASSIC_STANDARD);
    killDropsFollowTheRoll(CLASSIC_EXPANDED);
    upgradePickupRaisesLevelUpToFour();
    deathLosesOneWeaponLevel();
    everyFifthWaveIsABoss();
    bossWavesFollowTheTable();
    bulletsAndBlastsDamageTheBoss();
    killingTheBossScoresAndDropsBothPickups();
    bossBodyKillsTheShip();
    bossFiresBurstThenFan();
    bossWaitsForALivingShip();
    twinBossesSwayInTheirOwnHalves();
    twinBossesBothMustDie();
    mageBallBeamsItsColumnAfterOneAndAHalfSeconds();
    deadMageCastsNoBeam();
    mageAttacksInRandomOrderAndGaps();
    beamDrawsAMagentaColumn();
    levelBannerComesBeforeWaveOne();
    killingTheLevelBossClearsTheLevel();
    deathDuringTheClearOnlyLosesOnTheLastLife();
    jungleDifficultyIsFiveWavesAhead();
    heavyBeetlesComeEveryEightWaves();
    heavyBeetleRoamsTheUpperScreen();
    levelOneClearContinuesIntoTheJungle();
    pickedLevelRunEndsOnItsClear();
    jungleBossesFollowTheTable();
    monkeyThrowsBananasStraightDownThenAPeel();
    monkeyAlternatesWithABananaBunch();
    peelSlidesTheShipThenStunsIt();
    peelLiesFiveSeconds();
    bananaKillsTheShip();
    mistShotsHealTheGorilla();
    poopSplatsIntoAMistThatFades();
    gorillaPoundDropsFiveCoconuts();
    gorillaNeverPoopsPastThreeMists();
    deadGorillaDropsNoCoconuts();
    levelBackgroundsStayDarkerThanTheStars();
    levelLadderGetsHarder();
    heaviesFlyInAtThreePlaces();
    levelNamesFollowTheOrder();
    eachLevelClearContinuesToTheNext();
    newBossesFollowTheTable();
    shipGlidesOnTheGlacier();
    iceBlockFreezesAndMashingBreaksOut();
    yetiCyclesVolleyIceBlockAndSneeze();
    golemIciclesHangThenFall();
    lavaDropScorchesForDoubleDamage();
    impCyclesFireballsLavaAndFan();
    eruptionKillsOnlyInItsColumn();
    fireLordRollsAllFourAttacks();
    currentPushesShipsInTheDeepSea();
    anglerCyclesStreamBigBubbleAndFan();
    hookLiftsAndStunsTheShip();
    invincibleShipsSlipOffTheHook();
    emptyCastsSometimesBringABoot();
    playerTwoShipIsGold();
    mageBallFormsAtTheWand();
    bigIceBlockFreezesFromItsCorner();
    seeThroughIceTintsWhatIsUnder();
    bubbleTrailOnlyWhileMovingInTheDeepSea();
    fishermanKeepsAtMostThreeLiveFishOut();
    liveFishLandsAndChasesTheShip();
    liveFishKillsAndDies();
    shotsAndTimeEndLiveFish();
    overlappingHitStopsTakeTheLongest();
    shootingAndKillsNeverShake();
    traumaDecaysToZero();
    inputDuringHitStopStillFires();
    flashesAreRateLimited();
    reducedFlashingDrawsNoFlash();
    zeroShakeDrawsNoOffset();
    secondPlayerJoinsBlinkingWithAFreshRun();
    coOpRunEndsWhenBothPlayersAreOut();
    waveBonusSkipsAPlayerWhoIsOut();
    screenshotIsAValidBmp();
    shipSkinsKeepTheBody();
    bigBossesDropTheLevelBuff();
    buffPickupGivesTheBuffOnce();
    dyingLosesEveryBuff();
    squareCyclesHeldStrongAttacks();
    magicBeamHitsItsColumnOnce();
    magicBeamHitsWhenItReachesTheTop();
    mageBeamKillsOnlyOnceGrown();
    mageBeamRumblesThenShatters();
    eruptionLeavesNoShards();
    frostWaveFreezesHealsAndShatters();
    volcanoBuffFiresAQuarterFaster();
    bubbleShieldTakesAHitAndChains();
    classicKeepsBuffsIntoTheNextLevel();
    classicVariantsCapThePartyAndReadPlayerFour();
    rolesRequireThreeExpandedPlayers();
    rolesApplyMovementFireCadenceAndDamage();
    tankArmorAndHeavyRocket();
    assassinPhasesThroughAttacksForFiveSeconds();
    mageOrbsPierceEachEnemyOnceAndHalveDamage();
    expandedClassicScalesEveryWave();
    printf(failures ? "%d check(s) failed\n" : "all tests passed\n", failures);
    return failures ? 1 : 0;
}
