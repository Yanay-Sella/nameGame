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
    bool autopilot;               // Select pressed: dev autopilot toggle (main.cpp)
    bool screenshot;              // L1 pressed: dev screenshot (main.cpp)
    bool godMode;                 // L2+L3 pressed: dev test mode, no ship can die (main.cpp)
    bool cycle;                  // Square pressed: arm the next held strong attack (level buffs)
    bool jump;                    // X pressed (NAME QUEST: jump and confirm; Start is not a confirm there)
    bool jumpHeld;                // X held: a full-height jump
    bool selectHeld;              // Select held (NAME QUEST dev: held 2 s → TEST GROUNDS)
    bool item;                    // Triangle pressed (NAME QUEST: select the next item)
    bool connected;               // this player's controller is open
};

// Sounds the game asks for during a step; main.cpp plays each requested one once.
enum Sound {
    SOUND_SHOOT, SOUND_ALIEN_HIT, SOUND_ALIEN_DEATH, SOUND_ROCKET_LAUNCH, SOUND_EXPLOSION, SOUND_PICKUP,
    SOUND_PLAYER_DEATH, SOUND_ROCKET_EMPTY, SOUND_WAVE_START, SOUND_EXTRA_LIFE,
    SOUND_BANANA_THROW, SOUND_SCREECH, SOUND_SLIP, SOUND_STUN, SOUND_SPLAT, SOUND_LAUGH, SOUND_POUND,
    SOUND_SNEEZE, SOUND_FREEZE, SOUND_CRACK, SOUND_SIZZLE, SOUND_BUBBLE, SOUND_CAST, SOUND_HOOKED,
    // NAME QUEST (spec §16)
    SOUND_Q_STEP_STONE, SOUND_Q_STEP_METAL, SOUND_Q_STEP_LEAVES, SOUND_Q_STEP_SNOW, SOUND_Q_STEP_ROCK, SOUND_Q_STEP_SAND,
    SOUND_Q_STEP_MARBLE, SOUND_Q_STEP_PAPER, SOUND_Q_JUMP, SOUND_Q_LAND, SOUND_Q_SWING, SOUND_Q_STAB, SOUND_Q_ARROW,
    SOUND_Q_SPELL, SOUND_Q_HIT, SOUND_Q_CRIT, SOUND_Q_HURT, SOUND_Q_DEATH, SOUND_Q_NAMES, SOUND_Q_KINDLE, SOUND_Q_LEVEL_UP,
    SOUND_Q_PORTAL, SOUND_Q_BLIP, SOUND_Q_TALK, SOUND_Q_DRINK, SOUND_Q_DODGE, SOUND_Q_SPLASH, SOUND_Q_ZAP, SOUND_Q_ROAR,
    SOUND_Q_GATE, SOUND_Q_CHEST, SOUND_COUNT
};

// Level buffs: bit (level - 1) of Player::buffs (docs/features/level-buffs).
const int BUFF_SPACE = 1, BUFF_JUNGLE = 2, BUFF_GLACIER = 4, BUFF_VOLCANO = 8, BUFF_SEA = 16, BUFF_ALL = 31;
const int STANDARD_PLAYERS = 2, MAX_PLAYERS = 4, MAX_BOSSES = 3;
enum ClassicVariant { CLASSIC_STANDARD, CLASSIC_EXPANDED };

// One ship and its owner's run state; players are an array so co-op only adds entries (GDD D-007).
struct Player {
    float x, y;              // top-left of the ship sprite
    bool alive;              // false between a death and the respawn
    float fireTimer;         // s until the next bullet while R1 is held
    float muzzleTimer;       // s of muzzle flash left at the nose
    float rocketCooldown;    // s until the next rocket may launch
    float rocketPressedAt;   // game time of a buffered R2 press, or -1
    float respawnTimer;      // s until the ship returns after a death
    float invincibleTimer;   // s of blinking invincibility left
    int lives, rockets, score;
    int weaponLevel;         // 1..4: single, double, spread, fast spread
    int upgradeParts;        // upgrade pickups collected toward the next level; it takes weaponLevel of them
    float upgradeTimer;      // s left of the pickup's pop-up text and HUD pulse
    bool upgradeLevelUp;     // the last pickup raised the level: "POWER UP!" instead of "n/m"
    int nextExtraLife;       // score that gives the next extra life
    float slideTimer;        // s left of a banana-peel slide (jungle)
    float slideVX, slideVY;  // slide direction, a unit vector
    float stunTimer;         // s the ship can't move, shoot or fire rockets after a slide
    bool inMist;             // the ship's centre is in a stink mist: this player's bullets heal gorillas
    float vx, vy;            // current speed, px/s: on the glacier it eases toward the stick's
    float frozenTimer;       // s left frozen in an ice block: no moving, shooting or rockets
    bool fireWasHeld;        // R1 was held last step: a fresh press chips at the ice
    float scorchTimer;       // s left scorched by a lava drop: slower, bullets do double damage
    float hookedTimer;       // s left dangling on the fisherman's hook: no moving, shooting or rockets
    float hookLiftTo;        // y the hook lifts the ship to
    float trailTimer;        // s until the next bubble of the deep-sea trail
    int buffs;               // level buffs held: bit (level - 1); all lost on death
    int strongAttack;        // what R2 fires: 0 rocket, or the buff's level: 1 magic beam, 3 frost wave, 5 bubble shield
    bool shield;             // deep-sea bubble shield up: the next deadly hit pops it instead of killing
    float beamCharge;        // s left of the magic ball growing at the wand; the beam fires at 0
    float beamTimer;         // s the magic beam stays drawn
    float beamX, beamBottom; // the beam's centre x and the y it rises from
};

// owner: player index for bullets, -1 for enemy shots. Jungle bullets pierce: pierced hits so far, damage owed, enemies already hit.
struct Shot {
    float x, y, vy; int owner; float vx; int pierced = 0; float owed = 0; std::vector<const void*> hitList;
    Shot(float x, float y, float vy, int owner, float vx) : x(x), y(y), vy(vy), owner(owner), vx(vx) {}
};

enum AlienType { ALIEN_GREEN, ALIEN_BLUE, ALIEN_RED, ALIEN_HEAVY };   // roles; each level draws them its own way
enum AlienState { ALIEN_ENTERING, ALIEN_IN_FORMATION, ALIEN_DIVING, ALIEN_RETURNING, ALIEN_ROAMING };

// Random per formation wave (enemy-waves rule 8): the slots' layout and where the aliens fly in from.
// The first six bend the 8-column grid; the rest are traced by formations.cpp (and need a fitting alien count).
enum FormationShape {
    SHAPE_BLOCK, SHAPE_STAGGER, SHAPE_V, SHAPE_ARROW, SHAPE_WAVE, SHAPE_SPLIT,
    SHAPE_DIAGONALS_R, SHAPE_DIAGONALS_L, SHAPE_ZIGZAG, SHAPE_ZIGZAGS, SHAPE_X, SHAPE_X3, SHAPE_HEART, SHAPE_HEARTS,
    SHAPE_DIAMOND, SHAPE_DIAMONDS, SHAPE_CHEVRONS, SHAPE_TRIANGLE, SHAPE_TRIANGLE_DOWN, SHAPE_BOX, SHAPE_CIRCLES, SHAPE_TWINS,
    SHAPE_OVAL, SHAPE_OVALS, SHAPE_WAVES, SHAPE_SPIRAL, SHAPE_PLUSES, SHAPE_POINTER,
    SHAPE_BOWTIE, SHAPE_CROWN, SHAPE_RAINBOW, SHAPE_BOWL, SHAPE_PYRAMID, SHAPE_LETTERS, SHAPE_WORD, SHAPE_COUNT
};
const int SHAPE_FIRST_TRACED = SHAPE_DIAGONALS_R;
enum EntryStyle { ENTRY_TOP, ENTRY_LEFT, ENTRY_RIGHT, ENTRY_PINCER, ENTRY_SCATTER, ENTRY_COUNT };

struct Alien {
    AlienType type;
    AlienState state;
    float slotX, slotY;   // place in the formation, before the sway and bob
    float x, y;           // where it is drawn and hit
    float vx;             // sideways speed while diving
    float vy;             // roaming heavy: vertical speed
    float entryDelay;     // s before it starts flying in from where it was placed
    int hp, maxHp;
    float flash;          // s of white hit flash left
    bool alive;
};

enum BossType { BOSS_CLASSIC, BOSS_MAGE, BOSS_MONKEY, BOSS_GORILLA, BOSS_YETI, BOSS_GOLEM, BOSS_IMP, BOSS_FIRELORD,
                BOSS_ANGLER, BOSS_FISHERMAN };
enum HookState { HOOK_NONE, HOOK_DOWN, HOOK_WAIT, HOOK_HOLD, HOOK_REEL };
// The wave's boss; separate from Alien because the 12x10 alien size is used in every collision.
struct Boss {
    BossType type;
    float x, y, vx;          // top-left of the sprite; vx is the signed sway speed
    float minX, maxX;        // sway range within this boss's lane
    float attackTimer;       // s until the next attack
    float burstTimer;        // s until the next shot of a burst
    int hp, maxHp;
    int burstShotsLeft;
    int nextAttack;          // small bosses: the next attack of their fixed cycle
    float flash;             // s of white hit flash left
    float ballX, ballY;      // mage: centre of the power ball; fire lord: x of the eruption
    float ballTimer;         // s until the ball fires its beam (or the eruption bursts); 0 when none
    float beamTimer;         // s of beam (or eruption) left; 0 when none
    float laughTimer;        // gorilla: s of laughing left after a heal
    float windupTimer;       // s left of a gorilla's chest pound or a yeti's sneeze; the attack comes at 0
    float sayTimer;          // s the speech under the boss stays up
    const char* say;         // ACHOO!, GOT ONE!, OLD BOOT
    HookState hookState;     // fisherman: its hook
    float hookX, hookY;      // top centre of the hook
    float hookTimer;         // s the hook waits at the bottom
    int hookPlayer;          // the ship on the hook while HOOK_HOLD
    bool hookCaught;         // this cast caught a ship (no boot on the way back)
    bool alive;
};

struct Rocket { float x, y, speed; int owner; };
struct Explosion { float x, y, age, radius; };
enum PickupType { PICKUP_ROCKET, PICKUP_UPGRADE, PICKUP_BUFF };
struct Pickup { float x, y; PickupType type; int level; };   // level: the buff's level, for PICKUP_BUFF
enum ThrownKind { THROWN_BANANA, THROWN_COCONUT, THROWN_SNOWBALL, THROWN_ICE_BLOCK, THROWN_ICICLE, THROWN_FIREBALL,
                  THROWN_LAVA_DROP, THROWN_BUBBLE, THROWN_BIG_BUBBLE, THROWN_BOOT, THROWN_BIG_LAVA };
// A thrown hazard, top-left of its sprite; all kill except the ice block (freezes) and the lava drops (scorch).
struct Thrown { float x, y, vx, vy; ThrownKind kind; float delay, age; };   // delay: s it hangs first; age: s moving
struct Lob { float fromX, fromY, toX, toY, age; bool poop, fish; };   // a harmless peel or poop, or a live fish, in flight; waits while age < 0
struct Peel { float x, y, life; };                               // a peel lying in the ship zone; top-left of its 8x6 sprite
struct Mist { float x, y, age; };                                // a stink mist centred at x, y
struct Fish { float x, y, life; bool left; };   // a live fish swimming at the nearest ship; top-left of its 9x6 sprite
// A bubble from a popped shield, centred at x, y; size 3, 2, 1; homes on alien index `alien` or boss index `boss` (-1: none).
struct ShieldBubble { float x, y, vx, vy, age; int size, owner, alien, boss; };
struct Particle { float x, y, vx, vy, life; uint32_t color; int h; };   // h: px tall, 1 or 2 (beam shard)
struct Star { float x, y; int layer; };

struct Game {
    std::vector<Player> players;
    std::vector<Shot> bullets, enemyShots;
    std::vector<Alien> aliens;
    std::vector<Boss> bosses;   // this wave's bosses; empty on formation waves
    std::vector<Thrown> thrown;   // deadly bananas and coconuts
    std::vector<Lob> lobs;        // peels and poops in flight
    std::vector<Peel> peels;      // peels lying in the ship zone
    std::vector<Mist> mists;      // stink mists
    std::vector<Fish> fish;      // live fish in the ship zone (deep sea)
    std::vector<ShieldBubble> shieldBubbles;   // homing bubbles from popped shields
    std::vector<Rocket> rockets;
    std::vector<Explosion> explosions;
    std::vector<Pickup> pickups;
    std::vector<Particle> particles;
    std::vector<Star> stars;
    int wave;
    int level;   // 1 space, 2 jungle, 3 glacier, 4 volcano, 5 deep sea
    ClassicVariant variant = CLASSIC_STANDARD;
    int arenaWidth() const;
    int arenaHeight() const;
    int playerLimit() const { return variant == CLASSIC_EXPANDED ? MAX_PLAYERS : STANDARD_PLAYERS; }
    bool singleLevel;      // a level picker run: it ends on this level's clear instead of going on to the next
    float time;            // game time, s
    float swayOffset, swayDirection;
    float sceneryTime;     // s the level background has scrolled
    float freezeTimer;     // s left of a frost wave: no enemy moves, shoots or attacks
    float frostAge = -1;   // s since the frost wave's ring left the ship, or -1
    float frostX, frostY;  // where the ring started
    float diveTimer;       // s until the next alien dives (from wave 3)
    int formationShape;    // this wave's random FormationShape
    int entryStyle;        // this wave's random EntryStyle
    float bannerTimer;     // s the "WAVE n" banner stays up
    float nextWaveTimer;   // s until the next wave once this one is cleared; 0 while a wave is on
    bool over;             // the last life is gone
    int highScore;         // draw-only: the saved best, copied in by main.cpp
    bool newHighScore;     // draw-only: this run beat it, set by main.cpp at game over
    float overTime;        // game time when the run ended
    bool levelClear;       // the level boss is dead: the "<LEVEL> CLEAR" banner runs, then the next level or the end
    float clearTimer;      // s left of the level clear banner
    bool won;              // the run ended on its last level's clear (deep sea, or the picked level), not on the last life
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
const int LEVEL_COUNT = 5;                 // level 1, jungle, glacier, volcano, deep sea; a classic run goes through them in order

void startGame(Game& game, int playerCount, uint32_t seed, int level = 1, bool singleLevel = false,
               ClassicVariant variant = CLASSIC_STANDARD);
bool joinGame(Game& game);   // adds the next player to a running game, arriving like a respawn
void updateGame(Game& game, const Actions* actions, float dt);   // actions: one per player
void drawGame(const Game& game);
void updateBackground(Game& game, float dt);   // scrolls the starfield
void drawBackground(const Game& game);         // navy fill and stars

int formationRows(int wave);        // rows of 8 aliens in a wave
float shotChance(int wave);         // enemy shots per alien per second
float enemyShotSpeed(int wave);     // px/s; rises after wave 10
float diveInterval(int wave);       // s between dives; dives start at wave 3
int maxDivers(int wave);            // aliens diving at once: 1, 2 from wave 15, 3 from wave 30
int alienHpBonus(int wave);         // HP added to every alien: +1 from wave 15, +2 from 25, +3 from 35
float swaySpeed(int wave);          // px/s of the formation's sway
float bobAmplitude(int wave);       // px the formation bobs up and down, from wave 16
bool isBossWave(int wave);          // every 5th wave is a boss alone
int bossHealth(int level, int wave);       // HP of each boss, from the level's table
int bossCount(int wave);            // 2 on waves 10 and 20, else 1
BossType bossType(int level, int wave);    // the level's small boss on waves 5 and 10, its big boss from wave 15
int difficultyWave(int level, int wave);   // the wave whose formation numbers a wave uses: n + 5 per level after the first
int heavyCount(int level, int wave, ClassicVariant variant = CLASSIC_STANDARD);
const char* levelName(int level);          // banner and clear name: LEVEL 1, JUNGLE, GLACIER, VOLCANO, DEEP SEA
const char* levelLabel(int level);         // HUD and end-screen label before the wave: WAVE, JUNGLE, GLACIER ...
