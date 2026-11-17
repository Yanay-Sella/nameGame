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
const int ALIEN_HP[3] = {1, 2, 4};
const int ALIEN_SCORE[3] = {100, 200, 400};
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
const int BOSS_BASE_HP = 100, BOSS_HP_PER_APPEARANCE = 50;
const float BOSS_BASE_SPEED = 40, BOSS_SPEED_PER_APPEARANCE = 10, BOSS_MAX_SPEED = 80;   // px/s
const float BOSS_BASE_INTERVAL = 2.5f, BOSS_INTERVAL_STEP = 0.3f, BOSS_MIN_INTERVAL = 1.2f;   // s
const int BOSS_BURST_SHOTS = 3, BOSS_FAN_SHOTS = 5;
const float BOSS_BURST_GAP = 0.15f;    // s
const float BOSS_AIMED_SPEED = 90, BOSS_FAN_DOWN_SPEED = 70, BOSS_FAN_SIDEWAYS = 60;   // px/s
const int BOSS_REWARD = 5000;          // points x appearance
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
// Pixel art (pixel-art/design.md)
const float ANIMATION_FRAME = 0.25f;   // s
const float HIT_FLASH = 0.06f;         // s
const int STAR_COUNTS[3] = {30, 20, 10};
const float STAR_SPEEDS[3] = {6, 14, 30};   // px/s
const char STAR_COLORS[3] = {'n', 'd', 's'};

const int SHIP_W = 16, SHIP_H = 14, ALIEN_W = 12, ALIEN_H = 10;
const int HITBOX = 6, HITBOX_X = 5, HITBOX_Y = 4;   // ship hitbox: 6x6, centred on the sprite
const Sprite* const ALIEN_SPRITES[3][2] = {{&GREEN_ALIEN_A, &GREEN_ALIEN_B}, {&BLUE_ALIEN_A, &BLUE_ALIEN_B},
                                           {&RED_ALIEN_A, &RED_ALIEN_B}};
const char ALIEN_COLORS[3] = {'g', 'u', 'r'};

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

int bossHealth(int wave) {
    return BOSS_BASE_HP + BOSS_HP_PER_APPEARANCE * (wave / 5 - 1);
}

static float bossAttackInterval(int wave) {
    return std::max(BOSS_BASE_INTERVAL - BOSS_INTERVAL_STEP * (wave / 5 - 1), BOSS_MIN_INTERVAL);
}

static void startWave(Game& game, int wave) {
    game.wave = wave;
    game.aliens.clear();
    int rows = formationRows(wave);
    game.boss = Boss();
    if (isBossWave(wave)) {
        Boss& boss = game.boss;
        boss.x = (SCREEN_W - BOSS_W) / 2.0f;
        boss.y = -BOSS_H;
        boss.vx = std::min(BOSS_BASE_SPEED + BOSS_SPEED_PER_APPEARANCE * (wave / 5 - 1), BOSS_MAX_SPEED);
        boss.hp = boss.maxHp = bossHealth(wave);
        boss.attackTimer = bossAttackInterval(wave);
        boss.alive = true;
        rows = 0;
    }
    float left = (SCREEN_W - (FORMATION_COLUMNS - 1) * COLUMN_SPACING - ALIEN_W) / 2;
    for (int row = 0; row < rows; row++) {
        // Tougher aliens take the top rows: red from wave 7, blue from wave 4, two of each from wave 13.
        int redRows = wave >= MORE_ARMOUR_WAVE ? 2 : wave >= 7 ? 1 : 0;
        int blueEnd = wave >= MORE_ARMOUR_WAVE ? 4 : wave >= 4 ? 2 : 0;   // rows above this that aren't red are blue
        AlienType type = row < redRows ? ALIEN_RED : row < blueEnd ? ALIEN_BLUE : ALIEN_GREEN;
        for (int col = 0; col < FORMATION_COLUMNS; col++) {
            Alien alien;
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
    game.diveTimer = diveInterval(wave);
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

void startGame(Game& game, int playerCount, uint32_t seed) {
    game = Game();
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
    float cx = alien.x + ALIEN_W / 2.0f, cy = alien.y + ALIEN_H / 2.0f;
    spawnParticles(game, cx, cy, 8, paletteColor(ALIEN_COLORS[alien.type]), 50);
    float roll = randomUnit(game);   // below 2 %: rocket pickup, below 5 %: upgrade pickup
    if (roll < PICKUP_CHANCE + UPGRADE_CHANCE) {
        Pickup pickup = {cx - 4, cy - 4, roll < PICKUP_CHANCE ? PICKUP_ROCKET : PICKUP_UPGRADE};
        game.pickups.push_back(pickup);
    }
    game.soundRequested[SOUND_ALIEN_DEATH] = true;
}

static void damageBoss(Game& game, int damage, int owner) {
    Boss& boss = game.boss;
    boss.hp -= damage;
    boss.flash = HIT_FLASH;
    if (boss.hp > 0) {
        game.soundRequested[SOUND_ALIEN_HIT] = true;
        return;
    }
    boss.alive = false;
    boss.burstShotsLeft = 0;
    feedback(game, TRAUMA_BOSS_DEATH, STOP_BOSS_DEATH, FLASH_BOSS_DEATH);
    if (owner >= 0) addScore(game, game.players[owner], BOSS_REWARD * (game.wave / 5));
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
    float reach = EXPLOSION_RADIUS + ALIEN_W / 2.0f;
    for (Alien& alien : game.aliens) {
        float dx = alien.x + ALIEN_W / 2.0f - x, dy = alien.y + ALIEN_H / 2.0f - y;
        if (alien.alive && dx * dx + dy * dy <= reach * reach) damageAlien(game, alien, EXPLOSION_DAMAGE, owner);
    }
    // The boss is hit when the circle overlaps its rectangle.
    float nearestX = std::max(game.boss.x, std::min(x, game.boss.x + BOSS_W));
    float nearestY = std::max(game.boss.y, std::min(y, game.boss.y + BOSS_H));
    if (game.boss.alive && (nearestX - x) * (nearestX - x) + (nearestY - y) * (nearestY - y) <= EXPLOSION_RADIUS * EXPLOSION_RADIUS)
        damageBoss(game, EXPLOSION_DAMAGE, owner);
    game.soundRequested[SOUND_EXPLOSION] = true;
}

static void killPlayer(Game& game, Player& player) {
    player.alive = false;
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
    player.x = std::max(0.0f, std::min(player.x + a.moveX * SHIP_SPEED * dt, (float)(SCREEN_W - SHIP_W)));
    player.y = std::max(MOVE_ZONE_TOP, std::min(player.y + a.moveY * SHIP_SPEED * dt, (float)(SCREEN_H - SHIP_H)));

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
                alien.state = ALIEN_IN_FORMATION;
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
            float towards = target->x + SHIP_W / 2.0f - (alien.x + ALIEN_W / 2.0f);
            alien.vx += (towards > 0 ? 1 : -1) * DIVE_STEER * dt;
            alien.vx = std::max(-DIVE_MAX_SIDEWAYS, std::min(alien.vx, DIVE_MAX_SIDEWAYS));
            alien.x += alien.vx * dt;
            alien.y += DIVE_SPEED * dt;
            if (alien.y > SCREEN_H) {   // out the bottom, back in from the top
                alien.y = -ALIEN_H;
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
    }
    bool canShoot = alien.state == ALIEN_IN_FORMATION || alien.state == ALIEN_DIVING;
    if (canShoot && !game.over && randomUnit(game) < shotChance(game.wave) * dt) {
        Shot shot = {alien.x + ALIEN_W / 2 - 1, alien.y + ALIEN_H, enemyShotSpeed(game.wave), -1, 0};
        game.enemyShots.push_back(shot);
    }
}

// Flies in, then sways and alternates an aimed burst and a fan; holds fire with no living ship.
static void updateBoss(Game& game, float dt) {
    Boss& boss = game.boss;
    boss.flash = std::max(0.0f, boss.flash - dt);
    if (boss.y < BOSS_STOP_Y) {
        boss.y = std::min(boss.y + ENTRY_SPEED * dt, BOSS_STOP_Y);
        return;
    }
    boss.x += boss.vx * dt;
    if (boss.x < 0 || boss.x > SCREEN_W - BOSS_W) {
        boss.x = std::max(0.0f, std::min(boss.x, (float)(SCREEN_W - BOSS_W)));
        boss.vx = -boss.vx;
    }

    const Player* target = NULL;
    float centreX = boss.x + BOSS_W / 2.0f, bottomY = boss.y + BOSS_H;
    for (const Player& p : game.players)
        if (p.alive && (!target || fabsf(p.x - centreX) < fabsf(target->x - centreX))) target = &p;
    if (!target) {
        boss.burstShotsLeft = 0;
        return;
    }

    boss.attackTimer -= dt;
    if (boss.attackTimer <= 0) {
        boss.attackTimer = bossAttackInterval(game.wave);
        if (boss.nextAttack == 0) {
            boss.burstShotsLeft = BOSS_BURST_SHOTS;
            boss.burstTimer = 0;
        } else {
            for (int i = 0; i < BOSS_FAN_SHOTS; i++) {
                float sideways = (i - (BOSS_FAN_SHOTS - 1) / 2.0f) / ((BOSS_FAN_SHOTS - 1) / 2.0f) * BOSS_FAN_SIDEWAYS;
                game.enemyShots.push_back({centreX - 1, bottomY, BOSS_FAN_DOWN_SPEED, -1, sideways});
            }
        }
        boss.nextAttack = 1 - boss.nextAttack;
    }
    if (boss.burstShotsLeft > 0) {
        boss.burstTimer -= dt;
        if (boss.burstTimer <= 0) {
            float dx = target->x + SHIP_W / 2.0f - centreX, dy = target->y + SHIP_H / 2.0f - bottomY;
            float length = sqrtf(dx * dx + dy * dy) + 0.001f;
            game.enemyShots.push_back({centreX - 1, bottomY, dy / length * BOSS_AIMED_SPEED, -1, dx / length * BOSS_AIMED_SPEED});
            boss.burstShotsLeft--;
            boss.burstTimer = BOSS_BURST_GAP;
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
    if (divers >= (game.wave >= TWO_DIVERS_WAVE ? 2 : 1) || candidates.empty()) return;
    Alien* diver = candidates[(size_t)(randomUnit(game) * candidates.size())];
    diver->state = ALIEN_DIVING;
    diver->vx = 0;
}

void updateBackground(Game& game, float dt) {
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

    float swaySpeed = std::min(20.0f + 2 * game.wave, 50.0f);
    game.swayOffset += game.swayDirection * swaySpeed * dt;
    if (fabsf(game.swayOffset) > SWAY_RANGE) {
        game.swayOffset = game.swayOffset > 0 ? SWAY_RANGE : -SWAY_RANGE;
        game.swayDirection = -game.swayDirection;
    }
    for (Alien& alien : game.aliens)
        if (alien.alive) updateAlien(game, alien, dt);
    if (game.boss.alive) updateBoss(game, dt);
    if (game.wave >= 3 && anyPlayerAlive && !game.over) {
        game.diveTimer -= dt;
        if (game.diveTimer <= 0) {
            game.diveTimer = diveInterval(game.wave);
            startDive(game);
        }
    }

    for (Shot& bullet : game.bullets) {
        bullet.x += bullet.vx * dt;
        bullet.y += bullet.vy * dt;
        for (Alien& alien : game.aliens) {
            if (!alien.alive || alien.y + ALIEN_H < 0 || !overlap(bullet.x, bullet.y, 2, 4, alien.x, alien.y, ALIEN_W, ALIEN_H)) continue;
            damageAlien(game, alien, 1, bullet.owner);
            bullet.y = -100;   // spent; removed below
            break;
        }
        if (bullet.y > -50 && game.boss.alive && game.boss.y + BOSS_H >= 0 &&
            overlap(bullet.x, bullet.y, 2, 4, game.boss.x, game.boss.y, BOSS_W, BOSS_H)) {
            damageBoss(game, 1, bullet.owner);
            bullet.y = -100;   // spent; removed below
        }
    }

    for (Rocket& rocket : game.rockets) {
        rocket.speed = std::min(rocket.speed + ROCKET_ACCEL * dt, ROCKET_TOP_SPEED);
        rocket.y -= rocket.speed * dt;
        bool hit = rocket.y < 0;
        for (const Alien& alien : game.aliens)
            if (alien.alive && alien.y + ALIEN_H >= 0 && overlap(rocket.x, rocket.y, 4, 8, alien.x, alien.y, ALIEN_W, ALIEN_H)) hit = true;
        if (game.boss.alive && game.boss.y + BOSS_H >= 0 && overlap(rocket.x, rocket.y, 4, 8, game.boss.x, game.boss.y, BOSS_W, BOSS_H)) hit = true;
        if (hit) {
            explode(game, rocket.x + 2, std::max(rocket.y, 0.0f) + 4, rocket.owner);
            rocket.y = -1000;   // spent; removed below
        }
    }

    for (Shot& shot : game.enemyShots) {
        shot.x += shot.vx * dt;
        shot.y += shot.vy * dt;
    }
    for (Pickup& pickup : game.pickups) pickup.y += PICKUP_FALL_SPEED * dt;

    for (Player& player : game.players) {
        if (!player.alive) continue;
        float hx = player.x + HITBOX_X, hy = player.y + HITBOX_Y;
        if (player.invincibleTimer == 0) {
            bool hit = false;
            for (Shot& shot : game.enemyShots) {
                if (!hit && overlap(shot.x, shot.y, 2, 4, hx, hy, HITBOX, HITBOX)) {
                    hit = true;
                    shot.y = SCREEN_H + 100;   // spent; removed below
                }
            }
            for (const Alien& alien : game.aliens)
                if (alien.alive && overlap(alien.x, alien.y, ALIEN_W, ALIEN_H, hx, hy, HITBOX, HITBOX)) hit = true;
            if (game.boss.alive && overlap(game.boss.x, game.boss.y, BOSS_W, BOSS_H, hx, hy, HITBOX, HITBOX)) hit = true;
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
    bool anyAlienAlive = game.boss.alive || std::any_of(game.aliens.begin(), game.aliens.end(), [](const Alien& a) { return a.alive; });
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
            game.won = true;
            game.over = true;
            game.overTime = game.time;
            return;
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
            float noise = (((unsigned)(px * 73856093) ^ (unsigned)(py * 19349663)) & 255) / 256.0f;
            if (d > 1 || noise < fade) continue;
            fillRect(px, py, 1, 1, paletteColor(d < 0.35f ? 'w' : d < 0.65f ? 'y' : 'o'));
        }
}

void drawBackground(const Game& game) {
    fillRect(0, 0, SCREEN_W, SCREEN_H, paletteColor('k'));
    for (const Star& star : game.stars) fillRect((int)star.x, (int)star.y, 1, 1, paletteColor(STAR_COLORS[star.layer]));
}

void drawGame(const Game& game) {
    drawBackground(game);

    int frame = (int)(game.time / ANIMATION_FRAME) % 2;
    for (const Alien& alien : game.aliens) {
        if (!alien.alive) continue;
        const Sprite& sprite = *ALIEN_SPRITES[alien.type][frame];
        if (alien.flash > 0) drawSpriteSolid(sprite, (int)alien.x, (int)alien.y, paletteColor('w'));
        else drawSprite(sprite, (int)alien.x, (int)alien.y);
    }
    if (game.boss.alive) {
        const Sprite& sprite = frame ? BOSS_B : BOSS_A;
        if (game.boss.flash > 0) drawSpriteSolid(sprite, (int)game.boss.x, (int)game.boss.y, paletteColor('w'));
        else drawSprite(sprite, (int)game.boss.x, (int)game.boss.y);
    }
    for (const Pickup& pickup : game.pickups)
        drawSprite(pickup.type == PICKUP_ROCKET ? ROCKET_PICKUP : UPGRADE_PICKUP, (int)pickup.x, (int)pickup.y);
    for (const Shot& bullet : game.bullets) drawSprite(BULLET, (int)bullet.x, (int)bullet.y);
    for (const Rocket& rocket : game.rockets) drawSprite(ROCKET, (int)rocket.x, (int)rocket.y);
    for (size_t i = 0; i < game.players.size(); i++) {
        const Player& player = game.players[i];
        bool blinkOff = player.invincibleTimer > 0 && fmodf(player.invincibleTimer, 2 * BLINK_PERIOD) < BLINK_PERIOD;
        const Sprite& ship = i == 0 ? (frame ? SHIP_B : SHIP_A) : (frame ? SHIP2_B : SHIP2_A);
        if (player.alive && !blinkOff) drawSprite(ship, (int)player.x, (int)player.y);
    }
    for (const Shot& shot : game.enemyShots) drawSprite(ENEMY_SHOT, (int)shot.x, (int)shot.y);
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
    snprintf(text, sizeof text, "WAVE %d", game.wave);
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
    if (game.boss.alive) {
        int barX = (SCREEN_W - BOSS_HEALTH_BAR_W) / 2;
        fillRect(barX, BOSS_HEALTH_BAR_Y, BOSS_HEALTH_BAR_W, 3, paletteColor('R'));
        fillRect(barX, BOSS_HEALTH_BAR_Y, BOSS_HEALTH_BAR_W * std::max(game.boss.hp, 0) / game.boss.maxHp, 3, paletteColor('r'));
    }
    if (!coOp && !game.over && !game.levelClear) drawText(SCREEN_W - 2 - textWidth("2P PRESS START"), 2, "2P PRESS START", paletteColor('s'));

    if (game.over) {
        fillRect(SCREEN_W / 2 - 72, 58, 144, 62, paletteColor('n'));   // panel so passing aliens don't hide the text
        fillRect(SCREEN_W / 2 - 71, 59, 142, 60, paletteColor('k'));
        drawCentredText(66, game.won ? "LEVEL 1 CLEAR" : "GAME OVER", paletteColor(game.won ? 'y' : 'r'), 2);
        if (coOp) snprintf(text, sizeof text, "1P %d   2P %d   WAVE %d", game.players[0].score, game.players[1].score, game.wave);
        else snprintf(text, sizeof text, "SCORE %d   WAVE %d", game.players[0].score, game.wave);
        drawCentredText(86, text, paletteColor('w'), 1);
        if (game.newHighScore) drawCentredText(96, "NEW HIGH SCORE", paletteColor('y'), 1);
        if (game.time - game.overTime > GAME_OVER_INPUT_LOCK) drawCentredText(108, "X PLAY AGAIN   O MENU", paletteColor('y'), 1);
    } else if (game.levelClear) {
        drawCentredText(84, "LEVEL 1 CLEAR", paletteColor('y'), 2);
    } else if (game.bannerTimer > 0) {
        const char* banner = game.wave == 1 && game.bannerTimer > BANNER_TIME ? "LEVEL 1" : isBossWave(game.wave) ? "BOSS" : "WAVE %d";
        snprintf(text, sizeof text, banner, game.wave);
        drawCentredText(84, text, paletteColor('y'), 2);
    }
}
