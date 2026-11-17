// The rules and drawing of one run. No SDL or PS3 headers, so it also builds on the Mac for `make test`.
#pragma once
#include <stdint.h>
#include <vector>

// One player's named actions for one game step (input.cpp builds them from the controller).
struct Actions {
    float moveX, moveY;   // -1..1, after the stick deadzone
    bool fire;            // R1 held
    bool rocket;          // R2 pressed
    bool pause;           // Start pressed
    bool confirm;         // X or Start pressed
    bool stress;          // Triangle pressed: dev stress test (main.cpp)
    bool up, down, left, right;   // D-pad pressed or left stick flicked (menus)
    bool back;                    // Circle pressed
};

// Sounds the game asks for during a step; main.cpp plays each requested one once.
enum Sound {
    SOUND_SHOOT, SOUND_ALIEN_HIT, SOUND_ALIEN_DEATH, SOUND_ROCKET_LAUNCH, SOUND_EXPLOSION, SOUND_PICKUP,
    SOUND_PLAYER_DEATH, SOUND_ROCKET_EMPTY, SOUND_WAVE_START, SOUND_EXTRA_LIFE, SOUND_COUNT
};

// One ship and its owner's run state; players are an array so co-op only adds entries (GDD D-007).
struct Player {
    float x, y;              // top-left of the ship sprite
    bool alive;              // false between a death and the respawn
    float fireTimer;         // s until the next bullet while R1 is held
    float rocketCooldown;    // s until the next rocket may launch
    float rocketPressedAt;   // game time of a buffered R2 press, or -1
    float respawnTimer;      // s until the ship returns after a death
    float invincibleTimer;   // s of blinking invincibility left
    int lives, rockets, score;
    int weaponLevel;         // 1..4: single, double, spread, fast spread
    int nextExtraLife;       // score that gives the next extra life
};

struct Shot { float x, y, vy; int owner; float vx; };   // owner: player index for bullets, -1 for enemy shots

enum AlienType { ALIEN_GREEN, ALIEN_BLUE, ALIEN_RED };
enum AlienState { ALIEN_ENTERING, ALIEN_IN_FORMATION, ALIEN_DIVING, ALIEN_RETURNING };

struct Alien {
    AlienType type;
    AlienState state;
    float slotX, slotY;   // place in the formation, before the sway
    float x, y;           // where it is drawn and hit
    float vx;             // sideways speed while diving
    float entryDelay;     // s before it starts flying in
    int hp;
    float flash;          // s of white hit flash left
    bool alive;
};

// The wave's boss; separate from Alien because the 12x10 alien size is used in every collision.
struct Boss {
    float x, y, vx;          // top-left of the sprite; vx is the signed sway speed
    float attackTimer;       // s until the next attack
    float burstTimer;        // s until the next shot of a burst
    int hp, maxHp;
    int burstShotsLeft;
    int nextAttack;          // 0: aimed burst, 1: fan
    float flash;             // s of white hit flash left
    bool alive;
};

struct Rocket { float x, y, speed; int owner; };
struct Explosion { float x, y, age, radius; };
enum PickupType { PICKUP_ROCKET, PICKUP_UPGRADE };
struct Pickup { float x, y; PickupType type; };
struct Particle { float x, y, vx, vy, life; uint32_t color; };
struct Star { float x, y; int layer; };

struct Game {
    std::vector<Player> players;
    std::vector<Shot> bullets, enemyShots;
    std::vector<Alien> aliens;
    Boss boss;
    std::vector<Rocket> rockets;
    std::vector<Explosion> explosions;
    std::vector<Pickup> pickups;
    std::vector<Particle> particles;
    std::vector<Star> stars;
    int wave;
    float time;            // game time, s
    float swayOffset, swayDirection;
    float diveTimer;       // s until the next alien dives (from wave 3)
    float bannerTimer;     // s the "WAVE n" banner stays up
    float nextWaveTimer;   // s until the next wave once this one is cleared; 0 while a wave is on
    bool over;             // the last life is gone
    int highScore;         // draw-only: the saved best, copied in by main.cpp
    bool newHighScore;     // draw-only: this run beat it, set by main.cpp at game over
    float overTime;        // game time when the run ended
    bool soundRequested[SOUND_COUNT];
    uint32_t random;       // xorshift state
    float trauma;          // 0..1 screen shake, decays in real time
    float hitStop;         // s the game step is still frozen
    float flashTimer;      // s of screen flash left
    float lastFlashTime = -1;   // feelTime of the last screen flash, for the rate limit
    float feelTime;        // real seconds, advances during hit-stop
    float shakeStrength = 1;    // draw-only: 0..1, copied from Settings by main.cpp
    bool reduceFlashing;   // draw-only: copied from Settings by main.cpp
};

const float GAME_OVER_INPUT_LOCK = 1.5f;   // s before X or Start can start a new run
const int MAX_SCORE = 9999999;             // the score display and the saved high score stop here
const int MAX_PLAYERS = 2;                 // player 2 drops in from a second controller (co-op)

void startGame(Game& game, int playerCount, uint32_t seed);
void joinGame(Game& game);   // adds the next player to a running game, arriving like a respawn
void updateGame(Game& game, const Actions* actions, float dt);   // actions: one per player
void drawGame(const Game& game);
void updateBackground(Game& game, float dt);   // scrolls the starfield
void drawBackground(const Game& game);         // navy fill and stars

int formationRows(int wave);        // rows of 8 aliens in a wave
float shotChance(int wave);         // enemy shots per alien per second
float diveInterval(int wave);       // s between dives; dives start at wave 3
bool isBossWave(int wave);          // every 5th wave is a boss alone
int bossHealth(int wave);           // 100 + 50 per earlier appearance
