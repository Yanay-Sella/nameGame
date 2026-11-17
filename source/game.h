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
    int nextExtraLife;       // score that gives the next extra life
};

struct Shot { float x, y, vy; int owner; };   // owner: player index for bullets, -1 for enemy shots

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

struct Rocket { float x, y, speed; int owner; };
struct Explosion { float x, y, age, radius; };
struct Pickup { float x, y; };
struct Particle { float x, y, vx, vy, life; uint32_t color; };
struct Star { float x, y; int layer; };

struct Game {
    std::vector<Player> players;
    std::vector<Shot> bullets, enemyShots;
    std::vector<Alien> aliens;
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
    float overTime;        // game time when the run ended
    bool soundRequested[SOUND_COUNT];
    uint32_t random;       // xorshift state
};

const float GAME_OVER_INPUT_LOCK = 1.5f;   // s before X or Start can start a new run

void startGame(Game& game, int playerCount, uint32_t seed);
void updateGame(Game& game, const Actions* actions, float dt);   // actions: one per player
void drawGame(const Game& game);

int formationRows(int wave);        // rows of 8 aliens in a wave
float shotChance(int wave);         // enemy shots per alien per second
float diveInterval(int wave);       // s between dives; dives start at wave 3
