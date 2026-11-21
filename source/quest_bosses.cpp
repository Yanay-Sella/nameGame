// NAME QUEST bosses (spec §12): the arena lock, phases, attack choice and deaths, then one state machine per boss.
#include "quest.h"
#include "quest_sprites_1.h"
#include "quest_sprites_2.h"
#include "quest_sprites_3.h"
#include <math.h>
#include <string.h>

// Rules for every boss (spec §7, §12).
const float CONTACT_FRACTION = 0.20f, ATTACK_FRACTION = 0.25f, BIG_FRACTION = 0.35f;
const float MINI_HITS = 90, BOSS_HITS = 200, MINI_NAMES = 40, BOSS_NAMES = 150;
const float PHASE_SHIFT = 0.8f, PHASE_STOP = 0.15f, PHASE_TRAUMA = 0.6f, PHASE2_GAP = 0.75f;
const float DEATH_STOP = 0.15f, DEATH_TRAUMA = 1.0f, BANNER_TIME = 2, NAMES_TEXT = 1.2f;
const int DEATH_PARTICLES = 60;
const float TELL = 0.5f, RECOVER = 0.6f;               // a plain attack's telegraph; the open time after every attack
const float BOSS_SHOT_SPEED = 100 * DP;                // aimed and fanned boss shots (4 x 4, magenta)
const int BOSS_SHOT = (int)(4 * DP), MAX_SUMMONS = 4;
// Which boss guards which world: the mini-boss and the big boss (worlds 1-6; TEST GROUNDS borrows world 1's).
const int MINI_BOSS[7] = {-1, QBOSS_OVERSEER, QBOSS_MONKEY_KING, QBOSS_YETI, QBOSS_IMP, QBOSS_ANGLER, QBOSS_TWIN_SWORD};
const int BIG_BOSS[7] = {-1, QBOSS_STAR_MAGE, QBOSS_SILVERBACK, QBOSS_TITAN, QBOSS_FIRE_LORD, QBOSS_FISHERMAN, QBOSS_NAME_EATER};
// Gap between attacks and the attacks of phase 1 and 2 (built in phases 4-9; the rest have none yet).
const float GAP[QBOSS_COUNT] = {1.8f, 1.6f, 1.6f, 1.8f, 1.7f, 1.8f, 1.5f, 1.7f, 1.6f, 1.8f, 2.0f, 2.0f, 1.6f, 1.6f, 1.4f};
const int ATTACKS[QBOSS_COUNT][2] = {{3, 4}, {3, 3}, {3, 4}, {3, 4}, {3, 3}, {3, 3}};
// The Overseer (spec §12 world 1): hovers 50 above the floor sweeping at 40; P2 x1.3 and SUMMON.
enum { OVERSEER_BURST, OVERSEER_FAN, OVERSEER_SLAM, OVERSEER_SUMMON };
const float OVERSEER_HOVER = 50 * DP, OVERSEER_SWEEP = 40 * DP, OVERSEER_P2_SPEED = 1.3f;
const float BURST_SPACING = 0.15f, SLAM_MARK = 0.8f, SLAM_SPEED = 300 * DP, SLAM_SIT = 1.5f, RISE_SPEED = 100 * DP;
const float SLAM_SHAKE_RANGE = 64 * DP, SLAM_TRAUMA = 0.5f;
// The Star Mage: teleports (fade 0.3 out, 0.3 in) to one of 4 anchors before each attack; P2 two beams and the floor.
enum { MAGE_BEAM, MAGE_FAN, MAGE_SUMMON };
const float FADE = 0.3f, BALL_TIME = 1.2f, BEAM_TIME = 0.8f, BEAM_WIDTH = 24 * DP, SECOND_BEAM = 64 * DP;
const float FAN_SPREAD = 120 * 3.14159265f / 180;
const float FLOOR_PERIOD = 8, FLOOR_FLICKER = 0.6f, FLOOR_ON = 1.5f;
const int FLOOR_TILES = 6;
// Lobbed throws (bananas, peels, poop) fly in an arc under this pull and land after LOB_TIME.
const float LOB_GRAVITY = 300 * DP, LOB_TIME = 0.9f;
const int LOB_SIZE = (int)(5 * DP);
// The Monkey King (spec §12 world 2): leaps between its two trees (X anchors 0-1); bananas, peels; P2 coconut rain.
enum { MONKEY_LEAP, MONKEY_BANANAS, MONKEY_PEELS, MONKEY_COCONUTS };
const float LEAP_MARK = 0.5f, LEAP_TIME = 0.8f, LEAP_HEIGHT = 48 * DP, BANANA_SPREAD = 24 * DP, PEEL_SPREAD = 32 * DP;
const float COCONUT_MARK = 0.8f, COCONUT_FALL = 200 * DP, COCONUT_SPACING = 32 * DP;
const int RAIN_COCONUTS = 5, BOSS_COCONUT = (int)(6 * DP);
// The Silverback: POUND sends floor shockwaves (12 tall, 140 px/s; P2 two pairs 0.4 s apart), LEAP, STINK; P2 SUMMON stags.
enum { SILVER_POUND, SILVER_LEAP, SILVER_STINK, SILVER_SUMMON };
const float POUND_TELL = 0.6f, WAVE_SPEED = 140 * DP, WAVE_GAP = 0.4f, SILVER_MARK = 0.7f, SILVER_LEAP_TIME = 0.6f;
const float SILVER_LEAP_HEIGHT = 60 * DP, LEAP_TRAUMA = 0.5f, LEAP_SHAKE_RANGE = 64 * DP;
const int WAVE_W = (int)(16 * DP), WAVE_H = (int)(12 * DP), MAX_STAGS = 2;
// The Yeti (spec §12 world 3): SNEEZE (AH... AH... 0.5 s, 7 snowballs in a 100° fan; P2 twice in a row), BELLY SLIDE
// (crouch 0.6 s, across the arena at 180, big), ICE BLOCK (a lobbed cube that freezes the hero 1.2 s, no damage).
enum { YETI_SNEEZING, YETI_SLIDE, YETI_ICE_BLOCK };   // (YETI_SNEEZE is the shooter's sprite)
const float SNEEZE_TELL = 0.5f, SNEEZE_SHOW = 0.3f, SNEEZE_SPREAD = 100 * 3.14159265f / 180, SLIDE_CROUCH = 0.6f, SLIDE_SPEED = 180 * DP;
const int SNEEZE_BALLS = 7, SNOWBALL_SIZE = (int)(6 * DP), ICE_CUBE_SIZE = (int)(8 * DP);
// The Glacier Titan: ICICLE RAIN (6 marked 0.8 s on the ceiling), VOLLEY (3 aimed shards), SPIKE LINE (cracks run 0.6 s
// ahead of 16 px ice spikes erupting outward, big); P2 the middle 10 floor tiles turn to crumbling ice and a blizzard
// pushes 40 px/s, flipping every 4 s.
enum { TITAN_ICICLES, TITAN_VOLLEY, TITAN_SPIKES };
const float RAIN_MARK = 0.8f, RAIN_SPACING = 32 * DP, ICICLE_FALL = 280 * DP, VOLLEY_SPACING = 0.2f;
const float SPIKE_TELL = 0.8f, CRACK_LEAD = 0.6f, SPIKE_STEP = 0.05f, SPIKE_UP = 0.4f, SPIKE_HEIGHT = 16 * DP;
const float BLIZZARD = 40 * DP, BLIZZARD_FLIP = 4;
const int RAIN_ICICLES = 6, CRUMBLE_TILES = 10, SHARD_SIZE = (int)(6 * DP);

const char* bossName(int id) {
    static const char* const NAMES[QBOSS_COUNT] = {"THE OVERSEER", "THE STAR MAGE", "MONKEY KING", "SILVERBACK", "YETI",
        "THE GLACIER TITAN", "THE IMP", "THE FIRE LORD", "THE ANGLER", "THE FISHERMAN", "SWORD TWIN", "SPEAR TWIN",
        "THE NAME-EATER", "CLAUDE", "THE FIRST NAME"};
    return id >= 0 && id < QBOSS_COUNT ? NAMES[id] : "?";
}

static bool overlaps(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) {
    return ax < bx + bw && bx < ax + aw && ay < by + bh && by < ay + ah;
}

// The bottom-most floor of the arena: the y of the first floor top found going up from the bottom in the middle column.
static float arenaFloor(const AreaState& area) {
    const AreaDef& def = *area.def;
    int col = def.cols / 2;
    for (int r = def.rows - 1; r > 0; r--)
        if (tileSolid(tileAt(area, col, r)) && !tileSolid(tileAt(area, col, r - 1))) return (float)(r * TILE);
    return (float)(def.rows * TILE);
}

// The n-th X anchor of the area (bottom-centre of its tile), false when there are fewer.
static bool anchor(const AreaState& area, int n, float* x, float* y) {
    const AreaDef& def = *area.def;
    for (int t = 0; t < def.cols * def.rows; t++)
        if (def.tiles[t] == 'X' && n-- == 0) {
            *x = (t % def.cols) * TILE + TILE / 2.0f;
            *y = (float)((t / def.cols + 1) * TILE);
            return true;
        }
    return false;
}

static void bossShot(Quest& q, const QuestBoss& b, float x, float y, float angle, int kind = PROJ_ENEMY, int size = BOSS_SHOT) {
    Projectile p = {};
    p.x = x - size / 2;
    p.y = y - size / 2;
    p.vx = cosf(angle) * BOSS_SHOT_SPEED;
    p.vy = sinf(angle) * BOSS_SHOT_SPEED;
    p.w = p.h = size;
    p.damage = b.atk;
    p.range = BOSS_SHOT_SPEED * 4;
    p.kind = kind;
    q.projectiles.push_back(p);
}

// A throw lobbed from (x, y) to land on (tx, ty) after LOB_TIME; damage 0 for harmless ones (peels, poop).
static void bossLob(Quest& q, float x, float y, float tx, float ty, int kind, int damage) {
    Projectile p = {};
    p.x = x - LOB_SIZE / 2;
    p.y = y - LOB_SIZE / 2;
    p.vx = (tx - x) / LOB_TIME;
    p.vy = (ty - LOB_SIZE / 2 - y - 0.5f * LOB_GRAVITY * LOB_TIME * LOB_TIME) / LOB_TIME;
    p.w = p.h = LOB_SIZE;
    p.damage = damage;
    p.range = 2000 * DP;
    p.kind = kind;
    p.gravity = LOB_GRAVITY;
    q.projectiles.push_back(p);
}

static float angleToHero(const Quest& q, float x, float y) {
    return atan2f(q.hero.y + HERO_H / 2 - y, q.hero.x + HERO_W / 2 - x);
}

static int livingSummons(const Quest& q) {
    int n = 0;
    for (const Enemy& e : q.enemies) n += e.alive && e.spawn == 64;
    return n;
}

static void endAttack(QuestBoss& b) {
    b.lastAttack = b.attack;
    b.attack = -1;
    b.gap = GAP[b.id] * (b.phase >= 2 ? PHASE2_GAP : 1);
}

void startBoss(Quest& q, int id, float x, float y) {
    const Sprite* const LOOKS[6] = {&BOSS_OVERSEER_IDLE_1, &BOSS_STAR_MAGE_IDLE_1, &BOSS_MONKEY_KING_IDLE_1, &BOSS_SILVERBACK_IDLE_1,
                                    &BOSS_YETI_IDLE_1, &BOSS_TITAN_IDLE_1};
    const Sprite& look = *LOOKS[id < 6 ? id : 0];
    bool big = q.area.def->flags & AREA_BOSS;
    int world = statsWorld(q.area.world), ng = q.save.ngPlus;
    QuestBoss b = {};
    b.id = id;
    b.big = big;
    b.w = look.w - (int)(4 * DP);
    b.h = look.h - (int)(4 * DP);
    b.x = x - b.w / 2.0f;
    b.y = y - b.h;
    b.hp = b.maxHp = enemyHealth(world, big ? BOSS_HITS : MINI_HITS, ng);
    b.atk = enemyAttack(world, ATTACK_FRACTION, ng);
    b.bigAtk = enemyAttack(world, BIG_FRACTION, ng);
    b.contactAtk = enemyAttack(world, CONTACT_FRACTION, ng);
    b.names = enemyNames(world, big ? BOSS_NAMES : MINI_NAMES, ng, q.save.keys >> KEY_STRANGER_RING & 1);
    b.phase = 1;
    b.attack = b.lastAttack = -1;
    b.gap = GAP[id];
    b.alive = true;
    b.started = q.time;
    b.lastSwing = b.lastPartnerSwing = -1;
    b.facingLeft = q.hero.x < b.x;
    if (id == QBOSS_OVERSEER) {   // data: 0 sweep's left limit, 1 its right limit, 2 hover y, 3 floor y, 4 BURST shots fired
        b.data[3] = arenaFloor(q.area);
        b.data[2] = b.y = b.data[3] - OVERSEER_HOVER - b.h / 2;   // its centre 50 above the floor: a jump reaches it
        b.data[0] = 0;
        b.data[1] = (float)(q.area.def->cols * TILE - b.w);
        for (float sx = b.x; sx > 0; sx -= TILE)   // the walls at hover height bound the sweep
            if (tileSolid(tileAt(q.area, (int)(sx / TILE), (int)((b.y + b.h / 2) / TILE)))) { b.data[0] = sx + TILE; break; }
        for (float sx = b.x + b.w; sx < q.area.def->cols * TILE; sx += TILE)
            if (tileSolid(tileAt(q.area, (int)(sx / TILE), (int)((b.y + b.h / 2) / TILE)))) { b.data[1] = sx - TILE - b.w; break; }
        b.vx = OVERSEER_SWEEP;
    }
    if (id == QBOSS_STAR_MAGE) {   // data: 0 anchor index (-1 the W spot), 1 s into the floor's 8 s cycle, 2 visibility,
        b.data[0] = -1;            // 3 and 4 the beam columns' centres, 5 floor y
        b.data[2] = 1;
        b.data[5] = arenaFloor(q.area);
    }
    if (id == QBOSS_MONKEY_KING) b.data[0] = -1;   // data: 0 the tree it sits in (-1 the Q spot), 1-2 leap start, 3-4 leap end
    if (id == QBOSS_SILVERBACK) b.data[0] = arenaFloor(q.area);   // data: 0 floor y, 1 leap start x, 2 leap end x, 3 waves sent
    // Yeti data: 0 floor y, 1 sneezes done, 2 slide direction, 3 standing height. Titan data: 0 floor y, 1 shards fired,
    // 2-7 the icicle columns.
    if (id == QBOSS_YETI || id == QBOSS_TITAN) b.data[0] = arenaFloor(q.area);
    q.bosses.push_back(b);
    q.arenaLocked = true;
    q.banner = bossName(id);
    q.bannerScale = 2;
    q.bannerColor = 'w';
    q.bannerTimer = BANNER_TIME;
    logLine(q, "quest: boss %s start", bossName(id));
}

void startArenaFight(Quest& q) {
    const AreaDef& def = *q.area.def;
    int world = statsWorld(q.area.world);
    bool big = def.flags & AREA_BOSS, mini = def.flags & AREA_MINIBOSS;
    if ((!big && !mini) || world < 1 || world > 6) return;
    std::vector<uint64_t>& killed = q.killed[q.area.world];
    if (big && (q.save.bosses >> q.area.world & 1)) return;   // big bosses stay dead
    if (mini && q.area.index < (int)killed.size() && (killed[q.area.index] >> 63 & 1)) return;   // until the next rest or death
    int id = big ? BIG_BOSS[world] : MINI_BOSS[world];
    if (id > QBOSS_TITAN) return;   // the other worlds' bosses come in their phases
    const char* spot = strchr(def.tiles, big ? 'W' : 'Q');
    if (!spot) return;
    int t = (int)(spot - def.tiles);
    startBoss(q, id, (t % def.cols) * TILE + TILE / 2.0f, (float)((t / def.cols + 1) * TILE));
}

bool bossHittable(const QuestBoss& b) {
    return b.alive && b.hp > 0 && b.shift <= 0 && bossVisibility(b) >= 1;
}

float bossVisibility(const QuestBoss& b) {
    if (b.id != QBOSS_STAR_MAGE || b.attack < 0) return 1;
    if (b.step == 0) return 1 - b.timer / FADE;   // fading out
    if (b.step == 1) return b.timer / FADE;       // fading in at the new anchor
    return 1;
}

int bossPose(const QuestBoss& b) {
    if (b.attack < 0) return POSE_IDLE;
    if (b.id == QBOSS_OVERSEER) return b.step == 0 ? POSE_TELL : b.step == 1 ? POSE_ATTACK : POSE_OPEN;
    if (b.id == QBOSS_STAR_MAGE) return b.step <= 1 ? POSE_IDLE : b.step == 2 ? POSE_TELL : b.step == 3 ? POSE_ATTACK : POSE_OPEN;
    return b.step == 0 ? POSE_TELL : b.step == 1 ? POSE_ATTACK : POSE_OPEN;
}

int electricFloor(const Quest& q, int* col0, int* row) {
    for (const QuestBoss& b : q.bosses) {
        if (!b.alive || b.id != QBOSS_STAR_MAGE || b.phase < 2) continue;
        *col0 = q.area.def->cols / 2 - FLOOR_TILES / 2;
        *row = (int)(b.data[5] / TILE);
        float t = b.data[1];
        return t >= FLOOR_PERIOD - FLOOR_ON ? 2 : t >= FLOOR_PERIOD - FLOOR_ON - FLOOR_FLICKER ? 1 : 0;
    }
    return 0;
}

// The Overseer: sweeps at hover height; BURST, FAN, SLAM, and SUMMON from phase 2 (spec §12 world 1).
static void updateOverseer(Quest& q, QuestBoss& b, float dt) {
    float speed = b.phase >= 2 ? OVERSEER_P2_SPEED : 1;
    float cx = b.x + b.w / 2, mouthY = b.y + b.h * 0.65f;
    b.facingLeft = q.hero.x + HERO_W / 2 < cx;
    if (b.attack != OVERSEER_SLAM) {   // the sweep, back and forth between the walls
        b.x += b.vx * speed * dt;
        if (b.x < b.data[0]) b.x = b.data[0], b.vx = OVERSEER_SWEEP;
        if (b.x > b.data[1]) b.x = b.data[1], b.vx = -OVERSEER_SWEEP;
    }
    if (b.attack < 0) return;
    switch (b.attack) {
    case OVERSEER_BURST:   // tell 0.5, then 3 aimed shots 0.15 s apart
        if (b.step == 0 && b.timer >= TELL) b.step = 1, b.timer = 0, b.data[4] = 0;
        if (b.step == 1 && b.timer >= b.data[4] * BURST_SPACING) {
            bossShot(q, b, cx, mouthY, angleToHero(q, cx, mouthY));
            q.soundRequested[SOUND_Q_ZAP] = true;
            if (++b.data[4] >= 3) b.step = 2, b.timer = 0;
        }
        if (b.step == 2 && b.timer >= RECOVER) endAttack(b);
        break;
    case OVERSEER_FAN:   // tell 0.5, then 5 shots fanned downward
        if (b.step == 0 && b.timer >= TELL) {
            for (int k = -2; k <= 2; k++) bossShot(q, b, cx, mouthY, 3.14159265f / 2 + k * 0.35f);
            q.soundRequested[SOUND_Q_ZAP] = true;
            b.step = 2;
            b.timer = 0;
        }
        if (b.step == 2 && b.timer >= RECOVER) endAttack(b);
        break;
    case OVERSEER_SLAM:   // its shadow marks the floor 0.8 s, it drops at 300, sits 1.5 s (open), rises
        if (b.step == 0 && b.timer <= 0) q.markers.push_back({b.x, b.data[3] - 4 * DP, (float)b.w, 4 * DP, SLAM_MARK, 'm'});
        if (b.step == 0 && b.timer >= SLAM_MARK) b.step = 1, b.timer = 0;
        if (b.step == 1) {
            b.y += SLAM_SPEED * dt;
            if (overlaps(b.x, b.y, (float)b.w, (float)b.h, q.hero.x, q.hero.y, HERO_W, HERO_H))
                hurtHero(q, damageToHero(b.bigAtk, q.stats.def), cx);
            if (b.y + b.h >= b.data[3]) {
                b.y = b.data[3] - b.h;
                b.step = 2;
                b.timer = 0;
                q.soundRequested[SOUND_POUND] = true;
                if (fabsf(q.hero.x + HERO_W / 2 - cx) <= b.w / 2 + SLAM_SHAKE_RANGE) q.trauma = fminf(1, q.trauma + SLAM_TRAUMA);
            }
        }
        if (b.step == 2 && b.timer >= SLAM_SIT) b.step = 3, b.timer = 0;
        if (b.step == 3) {
            b.y -= RISE_SPEED * dt;
            if (b.y <= b.data[2]) b.y = b.data[2], endAttack(b);
        }
        break;
    case OVERSEER_SUMMON:   // tell 0.5, then 2 floaters beside it (at most 4 alive)
        if (b.step == 0 && b.timer >= TELL) {
            for (int k = 0; k < 2 && livingSummons(q) < MAX_SUMMONS; k++) spawnSummon(q, 1, cx + (k ? 1 : -1) * b.w * 0.7f, b.y + b.h);
            q.soundRequested[SOUND_SCREECH] = true;
            b.step = 2;
            b.timer = 0;
        }
        if (b.step == 2 && b.timer >= RECOVER) endAttack(b);
        break;
    }
}

// The Star Mage: every attack starts with a teleport to another anchor; BEAM, STAR FAN, SUMMON; P2 two beams and the floor.
static void updateStarMage(Quest& q, QuestBoss& b, float dt) {
    if (b.phase >= 2) {   // the electric floor's 8 s cycle; its middle 6 tiles hurt while on
        b.data[1] = fmodf(b.data[1] + dt, FLOOR_PERIOD);
        int col0, row;
        int state = electricFloor(q, &col0, &row);
        if (state == 2 && b.data[1] - dt < FLOOR_PERIOD - FLOOR_ON) q.soundRequested[SOUND_Q_ZAP] = true;
        const Hero& h = q.hero;
        int heroCol = (int)floorf((h.x + HERO_W / 2) / TILE);
        if (state == 2 && h.onGround && fabsf(h.y + HERO_H - row * TILE) < 1 && heroCol >= col0 && heroCol < col0 + FLOOR_TILES)
            hurtHero(q, damageToHero(b.atk, q.stats.def), h.x + HERO_W / 2 + (h.facingLeft ? 1 : -1));
    }
    const Sprite& frame = BOSS_STAR_MAGE_IDLE_1;
    float starX = b.x - 2 * DP + (b.facingLeft ? frame.w - STAR_MAGE_STAR[0][0] : STAR_MAGE_STAR[0][0]);
    float starY = b.y - 2 * DP + STAR_MAGE_STAR[0][1];
    if (b.attack < 0) {
        b.facingLeft = q.hero.x + HERO_W / 2 < b.x + b.w / 2;
        return;
    }
    if (b.step == 0 && b.timer >= FADE) {   // gone: reappear on another anchor, facing the hero
        int current = (int)b.data[0], count = 0;
        float ax, ay;
        while (count < 4 && anchor(q.area, count, &ax, &ay)) count++;
        int next = count > 1 ? (int)(random01(q) * (count - 1)) : 0;
        if (count > 1 && next >= current && current >= 0) next++;
        if (count > 0 && anchor(q.area, next, &ax, &ay)) {
            b.x = ax - b.w / 2.0f;
            b.y = ay - b.h;
            b.data[0] = (float)next;
        }
        b.facingLeft = q.hero.x + HERO_W / 2 < b.x + b.w / 2;
        b.step = 1;
        b.timer = 0;
        q.soundRequested[SOUND_Q_PORTAL] = true;
        return;
    }
    if (b.step == 1 && b.timer >= FADE) {
        b.step = 2;
        b.timer = 0;
        if (b.attack == MAGE_BEAM) {   // mark the column(s) below the star for the ball's 1.2 s
            float second = starX + (q.hero.x + HERO_W / 2 < starX ? -SECOND_BEAM : SECOND_BEAM);
            if (second < BEAM_WIDTH / 2 || second > q.area.def->cols * TILE - BEAM_WIDTH / 2) second = 2 * starX - second;
            b.data[3] = starX;
            b.data[4] = b.phase >= 2 ? second : -1;
            for (int k = 0; k < (b.phase >= 2 ? 2 : 1); k++) {
                float x = b.data[3 + k] - BEAM_WIDTH / 2;
                q.markers.push_back({x, starY, BEAM_WIDTH, b.data[5] - starY, BALL_TIME, 'm'});
            }
            q.effects.push_back({FX_BALL, starX, starY, 6 * DP, 6 * DP, 0, 0, 0, BALL_TIME, false, 'm'});
        }
        return;
    }
    switch (b.attack) {
    case MAGE_BEAM:   // the ball grows 1.2 s, then the beam burns down its column(s) for 0.8 s (big)
        if (b.step == 2 && b.timer >= BALL_TIME) {
            b.step = 3;
            b.timer = 0;
            q.soundRequested[SOUND_Q_SPELL] = true;
            for (int k = 0; k < (b.data[4] >= 0 ? 2 : 1); k++)
                q.effects.push_back({FX_BEAM, b.data[3 + k] - BEAM_WIDTH / 2, starY, BEAM_WIDTH, b.data[5] - starY, 0, 0, 0, BEAM_TIME, false, 'm'});
        }
        if (b.step == 3) {
            for (int k = 0; k < (b.data[4] >= 0 ? 2 : 1); k++)
                if (overlaps(b.data[3 + k] - BEAM_WIDTH / 2, starY, BEAM_WIDTH, b.data[5] - starY, q.hero.x, q.hero.y, HERO_W, HERO_H))
                    hurtHero(q, damageToHero(b.bigAtk, q.stats.def), b.data[3 + k]);
            if (b.timer >= BEAM_TIME) b.step = 4, b.timer = 0;
        }
        break;
    case MAGE_FAN:   // tell 0.5, then 7 shots spread over 120 degrees toward the hero
        if (b.step == 2 && b.timer >= TELL) {
            float aim = angleToHero(q, starX, starY);
            for (int k = -3; k <= 3; k++) bossShot(q, b, starX, starY, aim + k * FAN_SPREAD / 6);
            q.soundRequested[SOUND_Q_SPELL] = true;
            b.step = 4;
            b.timer = 0;
        }
        break;
    case MAGE_SUMMON:   // tell 0.5, then 2 crawlers on the floor, a quarter of the arena in from each side
        if (b.step == 2 && b.timer >= TELL) {
            float width = q.area.def->cols * TILE;
            for (int k = 0; k < 2 && livingSummons(q) < MAX_SUMMONS; k++) spawnSummon(q, 0, width * (k ? 0.75f : 0.25f), b.data[5]);
            q.soundRequested[SOUND_SCREECH] = true;
            b.step = 4;
            b.timer = 0;
        }
        break;
    }
    if (b.step == 4 && b.timer >= RECOVER) endAttack(b);
}

// The y of the first open row from the top in column col (where a coconut hangs before it falls).
static float ceilingY(const AreaState& area, float x) {
    int col = (int)floorf(x / TILE), r = 0;
    while (r < area.def->rows - 1 && tileSolid(tileAt(area, col, r))) r++;
    return (float)(r * TILE);
}

// The Monkey King: LEAP to its other tree, BANANA ARC, PEEL TOSS; P2 adds COCONUT RAIN (spec §12 world 2).
static void updateMonkeyKing(Quest& q, QuestBoss& b) {
    float cx = b.x + b.w / 2, heroX = q.hero.x + HERO_W / 2, heroFeet = q.hero.y + HERO_H;
    if (b.attack < 0 || b.step == 2) b.facingLeft = heroX < cx;
    if (b.attack < 0) return;
    switch (b.attack) {
    case MONKEY_LEAP:   // the landing is marked 0.5 s, then an 0.8 s arc onto the other tree
        if (b.step == 0 && b.timer <= 0) {
            float ax0, ay0, ax1, ay1;
            bool two = anchor(q.area, 0, &ax0, &ay0) && anchor(q.area, 1, &ax1, &ay1);
            int next = !two ? -1 : b.data[0] == 0 ? 1 : b.data[0] == 1 ? 0 : fabsf(ax0 - cx) > fabsf(ax1 - cx) ? 0 : 1;
            if (next < 0) { endAttack(b); break; }
            b.data[0] = (float)next;
            b.data[1] = b.x;
            b.data[2] = b.y;
            b.data[3] = (next ? ax1 : ax0) - b.w / 2.0f;
            b.data[4] = (next ? ay1 : ay0) - b.h;
            q.markers.push_back({b.data[3], b.data[4] + b.h - 4 * DP, (float)b.w, 4 * DP, LEAP_MARK, 'm'});
        }
        if (b.step == 0 && b.timer >= LEAP_MARK) b.step = 1, b.timer = 0, q.soundRequested[SOUND_Q_JUMP] = true;
        if (b.step == 1) {
            float t = fminf(b.timer / LEAP_TIME, 1);
            b.x = b.data[1] + (b.data[3] - b.data[1]) * t;
            b.y = b.data[2] + (b.data[4] - b.data[2]) * t - LEAP_HEIGHT * 4 * t * (1 - t);
            b.facingLeft = b.data[3] < b.data[1];
            if (t >= 1) b.step = 2, b.timer = 0, q.soundRequested[SOUND_Q_LAND] = true;
        }
        if (b.step == 2 && b.timer >= RECOVER) endAttack(b);
        break;
    case MONKEY_BANANAS:   // tell 0.5 (a banana raised), then 3 bananas lobbed at the hero
        if (b.step == 0 && b.timer >= TELL) {
            for (int k = -1; k <= 1; k++) bossLob(q, cx, b.y + b.h * 0.3f, heroX + k * BANANA_SPREAD, heroFeet, PROJ_BANANA, b.atk);
            q.soundRequested[SOUND_BANANA_THROW] = true;
            b.step = 2;
            b.timer = 0;
        }
        if (b.step == 2 && b.timer >= RECOVER) endAttack(b);
        break;
    case MONKEY_PEELS: {   // tell 0.5, then 2 peels lobbed onto the floor on either side of the hero
        if (b.step == 0 && b.timer >= TELL) {
            float floorY = arenaFloor(q.area);
            for (int k = -1; k <= 1; k += 2) bossLob(q, cx, b.y + b.h * 0.3f, heroX + k * PEEL_SPREAD, floorY, PROJ_PEEL, 0);
            q.soundRequested[SOUND_BANANA_THROW] = true;
            b.step = 2;
            b.timer = 0;
        }
        if (b.step == 2 && b.timer >= RECOVER) endAttack(b);
        break;
    }
    case MONKEY_COCONUTS:   // 5 coconuts marked 0.8 s on the ceiling over the hero, then they fall
        if (b.step == 0 && b.timer <= 0)
            for (int k = 0; k < RAIN_COCONUTS; k++) {
                float x = heroX + (k - RAIN_COCONUTS / 2) * COCONUT_SPACING;
                b.data[3 + k] = x;   // data 3-7: the coconuts' columns
                q.markers.push_back({x - BOSS_COCONUT / 2, ceilingY(q.area, x), (float)BOSS_COCONUT, 4 * DP, COCONUT_MARK, 'm'});
            }
        if (b.step == 0 && b.timer >= COCONUT_MARK) {
            for (int k = 0; k < RAIN_COCONUTS; k++) {
                Projectile p = {};
                p.x = b.data[3 + k] - BOSS_COCONUT / 2;
                p.y = ceilingY(q.area, b.data[3 + k]);
                p.vy = COCONUT_FALL;
                p.w = p.h = BOSS_COCONUT;
                p.damage = b.atk;
                p.range = (float)(q.area.def->rows * TILE);
                p.kind = PROJ_COCONUT;
                q.projectiles.push_back(p);
            }
            q.soundRequested[SOUND_BANANA_THROW] = true;
            b.step = 2;
            b.timer = 0;
        }
        if (b.step == 2 && b.timer >= RECOVER) endAttack(b);
        break;
    }
}

// Two shockwaves running both ways along the floor from the Silverback's fists (big).
static void silverbackWaves(Quest& q, const QuestBoss& b) {
    for (int dir = -1; dir <= 1; dir += 2) {
        Projectile p = {};
        p.x = b.x + b.w / 2 + dir * b.w / 2 - (dir < 0 ? WAVE_W : 0);
        p.y = b.data[0] - WAVE_H;
        p.vx = dir * WAVE_SPEED;
        p.w = WAVE_W;
        p.h = WAVE_H;
        p.damage = b.bigAtk;
        p.range = (float)(q.area.def->cols * TILE);
        p.kind = PROJ_WAVE;
        q.projectiles.push_back(p);
    }
    q.soundRequested[SOUND_POUND] = true;
}

// The Silverback: POUND, LEAP, STINK; P2 adds SUMMON 2 STAG and a second pair of waves (spec §12 world 2).
static void updateSilverback(Quest& q, QuestBoss& b) {
    float cx = b.x + b.w / 2, heroX = q.hero.x + HERO_W / 2;
    if (b.attack < 0 || b.step == 2) b.facingLeft = heroX < cx;
    if (b.attack < 0) return;
    switch (b.attack) {
    case SILVER_POUND:   // pounds its chest 0.6 s, then the waves (P2: a second pair 0.4 s later)
        if (b.step == 0 && b.timer >= POUND_TELL) {
            silverbackWaves(q, b);
            b.data[3] = 1;
            b.step = 1;
            b.timer = 0;
        }
        if (b.step == 1 && (b.phase < 2 || b.data[3] >= 2)) b.step = 2, b.timer = 0;
        else if (b.step == 1 && b.timer >= WAVE_GAP) silverbackWaves(q, b), b.data[3] = 2;
        if (b.step == 2 && b.timer >= RECOVER) endAttack(b);
        break;
    case SILVER_LEAP: {   // its shadow marks the hero's spot 0.7 s, then it lands there (big), trauma 0.5
        float minX = 2.0f * TILE, maxX = (float)(q.area.def->cols * TILE - 2 * TILE - b.w);
        if (b.step == 0 && b.timer <= 0) {
            b.data[1] = b.x;
            b.data[2] = fmaxf(minX, fminf(heroX - b.w / 2.0f, maxX));
            q.markers.push_back({b.data[2], b.data[0] - 4 * DP, (float)b.w, 4 * DP, SILVER_MARK, 'm'});
        }
        if (b.step == 0 && b.timer >= SILVER_MARK) b.step = 1, b.timer = 0, q.soundRequested[SOUND_Q_JUMP] = true;
        if (b.step == 1) {
            float t = fminf(b.timer / SILVER_LEAP_TIME, 1);
            b.x = b.data[1] + (b.data[2] - b.data[1]) * t;
            b.y = b.data[0] - b.h - SILVER_LEAP_HEIGHT * 4 * t * (1 - t);
            if (t >= 1) {
                b.step = 2;
                b.timer = 0;
                q.soundRequested[SOUND_POUND] = true;
                if (overlaps(b.x, b.y, (float)b.w, (float)b.h, q.hero.x, q.hero.y, HERO_W, HERO_H))
                    hurtHero(q, damageToHero(b.bigAtk, q.stats.def), b.x + b.w / 2);
                if (fabsf(heroX - (b.x + b.w / 2)) <= b.w / 2 + LEAP_SHAKE_RANGE) q.trauma = fminf(1, q.trauma + LEAP_TRAUMA);
            }
        }
        if (b.step == 2 && b.timer >= RECOVER) endAttack(b);
        break;
    }
    case SILVER_STINK:   // tell 0.5, then a lobbed poop that leaves a 40 px mist for 6 s where it lands
        if (b.step == 0 && b.timer >= TELL) {
            bossLob(q, cx, b.y + b.h * 0.3f, heroX, q.hero.y + HERO_H, PROJ_POOP, 0);
            q.soundRequested[SOUND_BANANA_THROW] = true;
            b.step = 2;
            b.timer = 0;
        }
        if (b.step == 2 && b.timer >= RECOVER) endAttack(b);
        break;
    case SILVER_SUMMON:   // tell 0.5, then 2 stags beside it (at most 2 alive)
        if (b.step == 0 && b.timer >= TELL) {
            for (int k = 0; k < 2 && livingSummons(q) < MAX_STAGS; k++) spawnSummon(q, 2, cx + (k ? 1 : -1) * b.w * 0.8f, b.data[0]);
            q.soundRequested[SOUND_Q_ROAR] = true;
            b.step = 2;
            b.timer = 0;
        }
        if (b.step == 2 && b.timer >= RECOVER) endAttack(b);
        break;
    }
}

// The Yeti's box: standing, or flat on its belly for the slide (same bottom).
static void yetiLies(QuestBoss& b, bool flat) {
    float bottom = b.y + b.h;
    if (flat && b.data[3] == 0) b.data[3] = (float)b.h, b.h = BOSS_YETI_SLIDE.h - (int)(4 * DP);
    if (!flat && b.data[3] > 0) b.h = (int)b.data[3], b.data[3] = 0;
    b.y = bottom - b.h;
}

// The Yeti: SNEEZE, BELLY SLIDE, ICE BLOCK; P2 sneezes twice in a row (spec §12 world 3).
static void updateYeti(Quest& q, QuestBoss& b, float dt) {
    float cx = b.x + b.w / 2, heroX = q.hero.x + HERO_W / 2, mouthY = b.y + b.h * 0.3f;
    if (b.attack < 0 || b.step == 0) b.facingLeft = heroX < cx;
    if (b.attack < 0) return;
    float mouthX = cx + (b.facingLeft ? -1 : 1) * b.w * 0.4f;
    switch (b.attack) {
    case YETI_SNEEZING:   // AH... AH... for 0.5 s, then 7 snowballs fanned at the hero; P2 a second sneeze
        if (b.step == 0 && b.timer <= 0 && b.data[1] == 0) q.soundRequested[SOUND_SNEEZE] = true;
        if (b.step == 0 && b.timer >= SNEEZE_TELL) {
            float aim = angleToHero(q, mouthX, mouthY);
            for (int k = 0; k < SNEEZE_BALLS; k++)
                bossShot(q, b, mouthX, mouthY, aim - SNEEZE_SPREAD / 2 + k * SNEEZE_SPREAD / (SNEEZE_BALLS - 1), PROJ_SNOWBALL, SNOWBALL_SIZE);
            b.data[1]++;
            b.step = 1;
            b.timer = 0;
        }
        if (b.step == 1 && b.timer >= SNEEZE_SHOW) {
            b.step = b.phase >= 2 && b.data[1] < 2 ? 0 : 2;
            b.timer = 0;
            if (b.step == 0) q.soundRequested[SOUND_SNEEZE] = true;
        }
        if (b.step == 2 && b.timer >= RECOVER) b.data[1] = 0, endAttack(b);
        break;
    case YETI_SLIDE: {   // crouches 0.6 s, then slides on its belly toward the hero's side until a wall (big)
        if (b.step == 0 && b.timer >= SLIDE_CROUCH) {
            b.data[2] = heroX < cx ? -1 : 1;
            yetiLies(b, true);
            b.step = 1;
            b.timer = 0;
            q.soundRequested[SOUND_Q_DODGE] = true;
        }
        if (b.step == 1) {
            b.x += b.data[2] * SLIDE_SPEED * dt;
            float lead = b.data[2] > 0 ? b.x + b.w : b.x;
            bool wall = tileSolid(tileAt(q.area, (int)floorf(lead / TILE), (int)floorf((b.y + b.h - 1) / TILE)));
            if (overlaps(b.x, b.y, (float)b.w, (float)b.h, q.hero.x, q.hero.y, HERO_W, HERO_H))
                hurtHero(q, damageToHero(b.bigAtk, q.stats.def), b.x + b.w / 2);
            if (wall || lead < TILE || lead > (q.area.def->cols - 1) * TILE) {
                b.x = b.data[2] > 0 ? floorf(lead / TILE) * TILE - b.w : (floorf(lead / TILE) + 1) * TILE;
                yetiLies(b, false);
                b.step = 2;
                b.timer = 0;
                q.soundRequested[SOUND_POUND] = true;
            }
        }
        if (b.step == 2 && b.timer >= RECOVER) endAttack(b);
        break;
    }
    case YETI_ICE_BLOCK:   // tell 0.5, then an ice cube lobbed at the hero; a hit freezes them 1.2 s
        if (b.step == 0 && b.timer >= TELL) {
            bossLob(q, mouthX, mouthY, heroX, q.hero.y + HERO_H / 2, PROJ_ICE_CUBE, 0);
            Projectile& cube = q.projectiles.back();
            cube.w = cube.h = ICE_CUBE_SIZE;
            q.soundRequested[SOUND_BANANA_THROW] = true;
            b.step = 1;
            b.timer = 0;
        }
        if (b.step == 1 && b.timer >= SNEEZE_SHOW) b.step = 2, b.timer = 0;
        if (b.step == 2 && b.timer >= RECOVER) endAttack(b);
        break;
    }
}

// The Titan's phase 2: the middle 10 tiles of the arena floor turn to crumbling ice in the live area (the fight's reset
// reloads the area's tiles).
static void titanCracksTheFloor(Quest& q, const QuestBoss& b) {
    const AreaDef& def = *q.area.def;
    int row = (int)(b.data[0] / TILE);
    for (int col = def.cols / 2 - CRUMBLE_TILES / 2; col < def.cols / 2 + CRUMBLE_TILES / 2; col++) {
        int t = row * def.cols + col;
        if (q.area.tiles[t] != '#') continue;
        q.area.tiles[t] = 'c';
        q.traps.push_back({t, 'c', 0, 0});
    }
    q.soundRequested[SOUND_CRACK] = true;
}

int titanSpikeLine(const Quest& q, int col, int* row) {
    for (const QuestBoss& b : q.bosses) {
        if (!b.alive || b.id != QBOSS_TITAN || b.attack != TITAN_SPIKES || b.step != 1) continue;
        *row = (int)(b.data[0] / TILE);
        int centre = (int)floorf((b.x + b.w / 2) / TILE), half = (int)ceilf(b.w / 2.0f / TILE);
        int n = abs(col - centre) - half;
        if (n < 0) return 0;
        float start = n * SPIKE_STEP;
        return b.timer < start ? 0 : b.timer < start + CRACK_LEAD ? 1 : b.timer < start + CRACK_LEAD + SPIKE_UP ? 2 : 0;
    }
    return 0;
}

// The Glacier Titan: ICICLE RAIN, VOLLEY, SPIKE LINE; P2 the blizzard (spec §12 world 3).
static void updateTitan(Quest& q, QuestBoss& b) {
    float cx = b.x + b.w / 2, heroX = q.hero.x + HERO_W / 2;
    b.facingLeft = heroX < cx;
    if (b.phase >= 2) q.area.wind = fmodf(q.time - b.started, 2 * BLIZZARD_FLIP) < BLIZZARD_FLIP ? BLIZZARD : -BLIZZARD;
    if (b.attack < 0) return;
    switch (b.attack) {
    case TITAN_ICICLES:   // 6 columns around the hero marked 0.8 s on the ceiling, then icicles fall at 280
        if (b.step == 0 && b.timer <= 0)
            for (int k = 0; k < RAIN_ICICLES; k++) {
                float x = heroX + (k - (RAIN_ICICLES - 1) / 2.0f) * RAIN_SPACING;
                x = fmaxf(2.0f * TILE, fminf(x, (q.area.def->cols - 2) * TILE - 1.0f));
                b.data[2 + k] = x;
                q.markers.push_back({x - 2 * DP, ceilingY(q.area, x), 4 * DP, 4 * DP, RAIN_MARK, 'c'});
            }
        if (b.step == 0 && b.timer >= RAIN_MARK) {
            for (int k = 0; k < RAIN_ICICLES; k++) {
                Projectile p = {};
                p.x = b.data[2 + k] - 2 * DP;
                p.y = ceilingY(q.area, b.data[2 + k]);
                p.vy = ICICLE_FALL;
                p.w = (int)(4 * DP);
                p.h = (int)(9 * DP);
                p.damage = b.atk;
                p.range = (float)(q.area.def->rows * TILE);
                p.kind = PROJ_ICICLE;
                q.projectiles.push_back(p);
            }
            q.soundRequested[SOUND_CRACK] = true;
            b.step = 1;
            b.timer = 0;
        }
        if (b.step == 1 && b.timer >= SNEEZE_SHOW) b.step = 2, b.timer = 0;
        if (b.step == 2 && b.timer >= RECOVER) endAttack(b);
        break;
    case TITAN_VOLLEY:   // tell 0.5, then 3 aimed ice shards 0.2 s apart
        if (b.step == 0 && b.timer >= TELL) b.step = 1, b.timer = 0, b.data[1] = 0;
        if (b.step == 1 && b.timer >= b.data[1] * VOLLEY_SPACING) {
            float y = b.y + b.h * 0.4f;
            bossShot(q, b, cx, y, angleToHero(q, cx, y), PROJ_SHARD, SHARD_SIZE);
            q.soundRequested[SOUND_FREEZE] = true;
            if (++b.data[1] >= 3) b.step = 2, b.timer = 0;
        }
        if (b.step == 2 && b.timer >= RECOVER) endAttack(b);
        break;
    case TITAN_SPIKES: {   // raises its fists 0.8 s, slams; cracks run out both ways 0.6 s ahead of the spikes (big)
        if (b.step == 0 && b.timer >= SPIKE_TELL) {
            b.step = 1;
            b.timer = 0;
            q.soundRequested[SOUND_POUND] = true;
            q.trauma = fminf(1, q.trauma + LEAP_TRAUMA);
        }
        if (b.step == 1) {
            const Hero& h = q.hero;
            int row;
            for (int c = (int)floorf(h.x / TILE); c <= (int)floorf((h.x + HERO_W - 0.001f) / TILE); c++)
                if (titanSpikeLine(q, c, &row) == 2 && h.y + HERO_H > row * TILE - SPIKE_HEIGHT)
                    hurtHero(q, damageToHero(b.bigAtk, q.stats.def), h.x + HERO_W / 2 + (heroX < cx ? -1 : 1));
            if (b.timer >= q.area.def->cols * SPIKE_STEP + CRACK_LEAD + SPIKE_UP) b.step = 2, b.timer = 0;
        }
        if (b.step == 2 && b.timer >= RECOVER) endAttack(b);
        break;
    }
    }
}

// Big boss: names, the bit, the next world's entry lit, banner, autosave. Mini-boss: names, the gate (spec §12).
static void bossDown(Quest& q, QuestBoss& b) {
    int world = q.area.world;
    b.alive = false;
    q.area.wind = 0;   // the Titan's blizzard dies with it
    q.hitStop = fmaxf(q.hitStop, DEATH_STOP);
    q.trauma = DEATH_TRAUMA;
    float cx = b.x + b.w / 2, cy = b.y + b.h / 2;
    for (int i = 0; i < DEATH_PARTICLES && q.particles.size() < PARTICLE_CAP; i++) {
        float angle = random01(q) * 6.2832f, speed = (40 + 80 * random01(q)) * DP;
        q.particles.push_back({cx, cy, cosf(angle) * speed, sinf(angle) * speed, 0.6f + 0.6f * random01(q), paletteColor(i % 3 ? 'w' : 'm'), 1});
    }
    q.save.names = q.save.names > MAX_NAMES - b.names ? MAX_NAMES : q.save.names + b.names;
    q.numbers.push_back({cx, b.y, 0, b.names, paletteColor('y'), NAMES_TEXT, true});
    q.save.kills++;
    for (Enemy& e : q.enemies)   // its summons fade with it
        if (e.spawn == 64) e.alive = false;
    q.markers.clear();
    q.arenaLocked = false;
    q.soundRequested[SOUND_EXPLOSION] = true;
    logLine(q, "quest: boss %s down after %.0f s", bossName(b.id), q.time - b.started);
    if (b.big) {
        q.save.bosses |= 1u << world;
        if (world >= 1 && world <= 5) {
            q.save.lit |= 1u << ((world + 1) * 2);
            q.banner = "GREAT NAME TAKEN";
            q.bannerScale = 2;
            q.bannerColor = 'Y';
            q.bannerTimer = BANNER_TIME;
        }
        questSave(q);
        return;
    }
    std::vector<uint64_t>& killed = q.killed[world];
    if (q.area.index < (int)killed.size()) killed[q.area.index] |= 1ull << 63;
    if (q.save.gates >> world & 1) questSave(q);
    else openGate(q, world);   // banner, sound, autosave
}

void updateBosses(Quest& q, float dt) {
    for (size_t i = 0; i < q.markers.size();) {
        q.markers[i].timer -= dt;
        if (q.markers[i].timer <= 0) q.markers.erase(q.markers.begin() + i);
        else i++;
    }
    for (QuestBoss& b : q.bosses) {
        if (!b.alive) continue;
        b.flash = fmaxf(0, b.flash - dt);
        if (b.hp <= 0) {
            bossDown(q, b);
            continue;
        }
        if (b.phase == 1 && b.hp * 2 <= b.maxHp) {   // phase 2 at 50 %: hit-stop, roar, trauma, 0.8 s invulnerable pose
            b.phase = 2;
            b.shift = PHASE_SHIFT;
            if (b.id == QBOSS_MONKEY_KING && b.attack == MONKEY_LEAP && b.step == 1) b.x = b.data[3], b.y = b.data[4];   // a leap cut short: on its tree
            if (b.id == QBOSS_SILVERBACK && b.attack == SILVER_LEAP && b.step == 1) b.x = b.data[2], b.y = b.data[0] - b.h;   // landed
            if (b.id == QBOSS_YETI) yetiLies(b, false), b.data[1] = 0;   // a slide or sneeze cut short: back on its feet
            if (b.id == QBOSS_TITAN) titanCracksTheFloor(q, b);
            b.attack = -1;
            b.gap = 0;
            q.hitStop = fmaxf(q.hitStop, PHASE_STOP);
            q.trauma = fminf(1, q.trauma + PHASE_TRAUMA);
            q.soundRequested[SOUND_Q_ROAR] = true;
            if (b.id == QBOSS_OVERSEER) b.y = b.data[2];   // a slam cut short: back to hover height
            logLine(q, "quest: boss %s phase %d", bossName(b.id), b.phase);
        }
        if (b.shift > 0) {
            b.shift = fmaxf(0, b.shift - dt);
            continue;
        }
        if (b.attack >= 0) b.timer += dt;   // a running attack's clock; a new one starts at 0 on the frame it is chosen
        else if ((b.gap -= dt) <= 0) {   // a random attack of the phase's list, never the last one twice
            int count = ATTACKS[b.id][b.phase >= 2 ? 1 : 0];
            int pick = (int)(random01(q) * (count - (b.lastAttack >= 0 && b.lastAttack < count ? 1 : 0)));
            if (b.lastAttack >= 0 && b.lastAttack < count && pick >= b.lastAttack) pick++;
            b.attack = pick < count ? pick : count - 1;
            b.step = 0;
            b.timer = 0;
        }
        switch (b.id) {
        case QBOSS_OVERSEER: updateOverseer(q, b, dt); break;
        case QBOSS_STAR_MAGE: updateStarMage(q, b, dt); break;
        case QBOSS_MONKEY_KING: updateMonkeyKing(q, b); break;
        case QBOSS_SILVERBACK: updateSilverback(q, b); break;
        case QBOSS_YETI: updateYeti(q, b, dt); break;
        case QBOSS_TITAN: updateTitan(q, b); break;
        }
        if (bossVisibility(b) >= 1 && overlaps(b.x, b.y, (float)b.w, (float)b.h, q.hero.x, q.hero.y, HERO_W, HERO_H))
            hurtHero(q, damageToHero(b.contactAtk, q.stats.def), b.x + b.w / 2);
    }
}
