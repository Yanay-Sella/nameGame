// Game-rule tests that run on the Mac (make test); no PS3, SDL or test framework needed.
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "input.h"
#include "screen.h"
#include "sprites.h"

static int failures = 0;

#define CHECK(condition)                                                     \
    do {                                                                     \
        if (!(condition)) {                                                  \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);      \
            failures++;                                                      \
        }                                                                    \
    } while (0)

static Actions noInput() {
    Actions a = {};
    return a;
}

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

static void spritesHaveTheirSize() {
    const Sprite* sprites[] = {&SHIP_A, &SHIP_B, &SHIP2_A, &SHIP2_B, &GREEN_ALIEN_A, &GREEN_ALIEN_B, &BLUE_ALIEN_A, &BLUE_ALIEN_B,
                               &RED_ALIEN_A, &RED_ALIEN_B, &BULLET, &ENEMY_SHOT, &ROCKET, &ROCKET_PICKUP,
                               &LIFE_ICON, &ROCKET_ICON};
    for (const Sprite* s : sprites) CHECK((int)strlen(s->pixels) == s->w * s->h);
}

static void onlyEnemyShotsUseMagenta() {
    const Sprite* sprites[] = {&SHIP_A, &SHIP_B, &SHIP2_A, &SHIP2_B, &GREEN_ALIEN_A, &GREEN_ALIEN_B, &BLUE_ALIEN_A, &BLUE_ALIEN_B,
                               &RED_ALIEN_A, &RED_ALIEN_B, &BULLET, &ROCKET, &ROCKET_PICKUP, &LIFE_ICON, &ROCKET_ICON};
    for (const Sprite* s : sprites) CHECK(strchr(s->pixels, 'm') == NULL);
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
    Actions a = noInput();
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
    Actions a = noInput();
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
    Actions a = noInput();
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
    run(game, noInput(), 2.5f);
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
    run(game, noInput(), 8);
    for (const Alien& alien : game.aliens) CHECK(alien.state != ALIEN_DIVING);
    game.wave = 3;
    game.diveTimer = 0.01f;
    run(game, noInput(), 0.05f);
    int divers = 0;
    for (const Alien& alien : game.aliens) divers += alien.state == ALIEN_DIVING;
    CHECK(divers == 1);
}

static void rocketBufferFiresAnEarlyPress() {
    Game game;
    startSafeGame(game);
    Actions press = noInput();
    press.rocket = true;
    game.players[0].rocketCooldown = 0.1f;   // pressed 0.1 s early: inside the 0.15 s buffer
    updateGame(game, &press, DT);
    run(game, noInput(), 0.2f);
    CHECK(game.players[0].rockets == 2);
    game.players[0].rocketCooldown = 0.3f;   // pressed 0.3 s early: the press expires
    updateGame(game, &press, DT);
    run(game, noInput(), 0.5f);
    CHECK(game.players[0].rockets == 2);
}

static void emptyRocketClicks() {
    Game game;
    startSafeGame(game);
    game.players[0].rockets = 0;
    Actions press = noInput();
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
    run(game, noInput(), 0.5f);
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
        Shot shot = {p.x + 7, p.y + 5, 0, -1};   // parked on the hitbox
        game.enemyShots.push_back(shot);
        updateGame(game, &NO_INPUT, DT);
        CHECK(!game.players[0].alive && game.players[0].lives == 3 - death);
        run(game, noInput(), 1.5f + 2.1f);   // respawn delay, then invincibility
        CHECK(game.players[0].alive == (death < 3));
    }
    CHECK(game.over);
}

static void nextWaveWaitsForALivingShip() {
    Game game;
    startGame(game, 1, 7);
    for (Alien& alien : game.aliens) alien.alive = false;
    Player& p = game.players[0];
    Shot shot = {p.x + 7, p.y + 5, 0, -1};
    game.enemyShots.push_back(shot);
    updateGame(game, &NO_INPUT, DT);
    CHECK(!game.players[0].alive);
    game.players[0].respawnTimer = 4;   // longer than the 2 s wave pause
    run(game, noInput(), 3);
    CHECK(game.wave == 1);              // the 2 s pause only counts while a ship is alive
    run(game, noInput(), 1.1f);
    CHECK(game.players[0].alive && game.wave == 1);
    run(game, noInput(), 2);
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
    Pickup pickup = {p.x + 4, p.y};
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
    run(game, noInput(), 5);
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
    Shot shot = {game.players[0].x + 7, game.players[0].y + 5, 0, -1};
    game.enemyShots.push_back(shot);
    for (int step = 0; step < 3 * 60; step++) updateGame(game, actions, DT);
    CHECK(game.players[0].lives == 0 && game.players[1].alive && !game.over);
    game.players[1].lives = 1;
    game.players[1].invincibleTimer = 0;
    Shot shot2 = {game.players[1].x + 7, game.players[1].y + 5, 0, -1};
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

int main() {
    spritesHaveTheirSize();
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
    secondPlayerJoinsBlinkingWithAFreshRun();
    coOpRunEndsWhenBothPlayersAreOut();
    waveBonusSkipsAPlayerWhoIsOut();
    printf(failures ? "%d check(s) failed\n" : "all tests passed\n", failures);
    return failures ? 1 : 0;
}
