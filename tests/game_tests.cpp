// Game-rule tests that run on the Mac (make test); no PS3, SDL or test framework needed.
#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "game.h"
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
static void startSafeGame(Game& game) {
    startGame(game, 1, 7);
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
                                            &RED_ALIEN_A, &RED_ALIEN_B, &BOSS_A, &BOSS_B, &BULLET, &ENEMY_SHOT, &ROCKET, &ROCKET_PICKUP,
                                            &UPGRADE_PICKUP, &LIFE_ICON, &ROCKET_ICON};

static void spritesHaveTheirSize() {
    for (const Sprite* s : ALL_SPRITES) CHECK((int)strlen(s->pixels) == s->w * s->h);
}

static void onlyEnemyShotsUseMagenta() {
    for (const Sprite* s : ALL_SPRITES) CHECK((s == &ENEMY_SHOT) == (strchr(s->pixels, 'm') != NULL));
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
    CHECK(diveInterval(20) == 1.5f);
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
    Pickup pickup = {p.x + 4, p.y, PICKUP_ROCKET};
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

static void killDropsFollowTheRoll() {
    Game game;
    startSafeGame(game);
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
    CHECK(upgrades > KILLS * 0.024 && upgrades < KILLS * 0.036);  // about 3 %
}

static void upgradePickupRaisesLevelUpToFour() {
    Game game;
    startSafeGame(game);
    Player& p = game.players[0];
    Pickup pickup = {p.x + 4, p.y, PICKUP_UPGRADE};
    for (int expected = 2; expected <= 4; expected++) {
        game.pickups.push_back(pickup);
        updateGame(game, &NO_INPUT, DT);
        CHECK(game.players[0].weaponLevel == expected && game.pickups.empty());
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
        p.invincibleTimer = 0;
        Shot shot = {p.x + 7, p.y + 5, 0, -1, 0};
        game.enemyShots.push_back(shot);
        updateGame(game, &NO_INPUT, DT);
        CHECK(!game.players[0].alive && game.players[0].weaponLevel == (level == 3 ? 2 : 1));
        run(game, NO_INPUT, 1.6f);
    }
}

// A safe run at the start of a boss wave: the boss has flown in and sways, nothing has fired yet.
static void startBossGame(Game& game, int wave) {
    startSafeGame(game);
    game.wave = wave - 1;
    for (Alien& alien : game.aliens) alien.alive = false;
    run(game, NO_INPUT, 2.5f);   // wave clear pause, then the boss enters
    run(game, NO_INPUT, 0.5f);
    game.boss.attackTimer = 100;
}

static void everyFifthWaveIsABoss() {
    Game game;
    startSafeGame(game);
    game.wave = 4;
    for (Alien& alien : game.aliens) alien.alive = false;
    run(game, NO_INPUT, 2.2f);
    CHECK(game.wave == 5 && game.boss.alive && game.aliens.empty() && game.bannerTimer > 0);
    game.boss.alive = false;
    run(game, NO_INPUT, 2.2f);
    CHECK(game.wave == 6 && !game.boss.alive && game.aliens.size() == 40);
}

static void bossHealthGrowsEachAppearance() {
    CHECK(bossHealth(5) == 100 && bossHealth(10) == 150 && bossHealth(15) == 200);
    CHECK(isBossWave(5) && isBossWave(10) && isBossWave(15));
    CHECK(!isBossWave(0) && !isBossWave(4) && !isBossWave(6));
    Game game;
    startBossGame(game, 10);
    CHECK(game.boss.hp == 150 && game.boss.maxHp == 150 && fabsf(fabsf(game.boss.vx) - 50) < 0.01f);
}

static void bulletsAndBlastsDamageTheBoss() {
    Game game;
    startBossGame(game, 5);
    Boss& boss = game.boss;
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
    startBossGame(game, 10);
    game.boss.hp = 1;
    int before = game.players[0].score;
    Shot bullet = {game.boss.x + 10, game.boss.y + 5, 0, 0, 0};
    game.bullets.push_back(bullet);
    updateGame(game, &NO_INPUT, DT);
    CHECK(!game.boss.alive && game.players[0].score == before + 10000 + 10 * 1000);   // 5,000 x 2 and the wave 10 clear bonus
    int rockets = 0, upgrades = 0;
    for (const Pickup& pickup : game.pickups) {
        rockets += pickup.type == PICKUP_ROCKET;
        upgrades += pickup.type == PICKUP_UPGRADE;
    }
    CHECK(rockets == 1 && upgrades == 1 && !game.explosions.empty());
    run(game, NO_INPUT, 0.1f);
    CHECK(game.nextWaveTimer > 0);   // the normal wave-clear pause starts
}

static void bossBodyKillsTheShip() {
    Game game;
    startBossGame(game, 5);
    game.boss.y = 110;   // down in the ship's movement zone
    Player& p = game.players[0];
    p.invincibleTimer = 0;
    p.x = game.boss.x + 10;
    p.y = 112;
    updateGame(game, &NO_INPUT, DT);
    CHECK(!game.players[0].alive && game.players[0].lives == 2);
}

static void bossFiresBurstThenFan() {
    Game game;
    startBossGame(game, 5);
    game.boss.attackTimer = 0.01f;
    run(game, NO_INPUT, 0.5f);
    CHECK(game.enemyShots.size() == 3);   // burst: 0, 0.15, 0.3 s
    for (const Shot& shot : game.enemyShots) CHECK(fabsf(sqrtf(shot.vx * shot.vx + shot.vy * shot.vy) - 90) < 0.5f && shot.vy > 0);
    game.enemyShots.clear();
    game.boss.attackTimer = 0.01f;
    run(game, NO_INPUT, 0.05f);
    CHECK(game.enemyShots.size() == 5);   // fan
    CHECK(game.enemyShots[0].vx == -60 && game.enemyShots[2].vx == 0 && game.enemyShots[4].vx == 60 && game.enemyShots[2].vy == 70);
}

static void bossWaitsForALivingShip() {
    Game game;
    startBossGame(game, 5);
    game.players[0].alive = false;
    game.players[0].respawnTimer = 1e9f;
    game.boss.attackTimer = 0.01f;
    run(game, NO_INPUT, 3);
    CHECK(game.enemyShots.empty());
}

// A frozen copy of the picture drawGame just drew.
static std::vector<uint32_t> pictureOf(const Game& game) {
    drawGame(game);
    return std::vector<uint32_t>(screenPixels, screenPixels + SCREEN_W * SCREEN_H);
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

static void settingsDefaultToMusicOn() {
    CHECK(settingsLoad("build/tests/missing.txt").music == true);
}

static void settingsSurviveASave() {
    Settings settings;
    settings.music = false;
    CHECK(settingsSave("build/tests/settings.txt", settings));
    CHECK(settingsLoad("build/tests/settings.txt").music == false);
    settings.music = true;
    CHECK(settingsSave("build/tests/settings.txt", settings));
    CHECK(settingsLoad("build/tests/settings.txt").music == true);
}

static void writeTextFile(const char* path, const char* text) {
    FILE* file = fopen(path, "w");
    fputs(text, file);
    fclose(file);
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
    CHECK(loaded.shake == 50 && loaded.reduceFlashing && loaded.music);
    writeTextFile("build/tests/shake-settings.txt", "shake=900\n");
    CHECK(settingsLoad("build/tests/shake-settings.txt").shake == 100);   // clamped
}

static void menuCursorHasAGlyph() {
    CHECK(fontGlyph('>') != 0);
}

int main() {
    spritesHaveTheirSize();
    settingsDefaultToMusicOn();
    settingsSurviveASave();
    shakeAndFlashSettingsSurviveASave();
    highScoreDefaultsToZero();
    highScoreSurvivesASave();
    highScoreIsClamped();
    menuCursorHasAGlyph();
    onlyEnemyShotsUseMagenta();
    deadzoneIsRadialAndRescaled();
    holdingFireShootsEvery012Seconds();
    shipStaysInsideMovementZone();
    formationGrowsToFiveRows();
    clearingAWaveStartsTheNextAfterThePause();
    enemyFormulasFollowTheDesign();
    aliensFlyInBeforeTheyCanShoot();
    divesStartAtWaveThree();
    rocketBufferFiresAnEarlyPress();
    emptyRocketClicks();
    blastKillsOnlyInsideItsRadius();
    extraLivesAtEvery50000();
    threeDeathsEndTheRun();
    nextWaveWaitsForALivingShip();
    alienBodyKillsTheShip();
    pickupAddsARocketUpToNine();
    weaponLevelsFireTheirPattern();
    killDropsFollowTheRoll();
    upgradePickupRaisesLevelUpToFour();
    deathLosesOneWeaponLevel();
    everyFifthWaveIsABoss();
    bossHealthGrowsEachAppearance();
    bulletsAndBlastsDamageTheBoss();
    killingTheBossScoresAndDropsBothPickups();
    bossBodyKillsTheShip();
    bossFiresBurstThenFan();
    bossWaitsForALivingShip();
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
    printf(failures ? "%d check(s) failed\n" : "all tests passed\n", failures);
    return failures ? 1 : 0;
}
