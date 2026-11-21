// NAME QUEST bosses (spec §12): the arena lock, phases, attack choice and deaths, then one state machine per boss.
#include "quest.h"
#include "quest_sprites_1.h"
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
const int ATTACKS[QBOSS_COUNT][2] = {{3, 4}, {3, 3}};
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

static void bossShot(Quest& q, const QuestBoss& b, float x, float y, float angle) {
    Projectile p = {};
    p.x = x - BOSS_SHOT / 2;
    p.y = y - BOSS_SHOT / 2;
    p.vx = cosf(angle) * BOSS_SHOT_SPEED;
    p.vy = sinf(angle) * BOSS_SHOT_SPEED;
    p.w = p.h = BOSS_SHOT;
    p.damage = b.atk;
    p.range = BOSS_SHOT_SPEED * 4;
    p.kind = PROJ_ENEMY;
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
    const Sprite& look = id == QBOSS_OVERSEER ? BOSS_OVERSEER_IDLE_1 : BOSS_STAR_MAGE_IDLE_1;
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
    if (id != QBOSS_OVERSEER && id != QBOSS_STAR_MAGE) return;   // the other worlds' bosses come in their phases
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
    return POSE_IDLE;
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

// Big boss: names, the bit, the next world's entry lit, banner, autosave. Mini-boss: names, the gate (spec §12).
static void bossDown(Quest& q, QuestBoss& b) {
    int world = q.area.world;
    b.alive = false;
    q.hitStop = fmaxf(q.hitStop, DEATH_STOP);
    q.trauma = DEATH_TRAUMA;
    float cx = b.x + b.w / 2, cy = b.y + b.h / 2;
    for (int i = 0; i < DEATH_PARTICLES; i++) {
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
        }
        if (bossVisibility(b) >= 1 && overlaps(b.x, b.y, (float)b.w, (float)b.h, q.hero.x, q.hero.y, HERO_W, HERO_H))
            hurtHero(q, damageToHero(b.contactAtk, q.stats.def), b.x + b.w / 2);
    }
}
