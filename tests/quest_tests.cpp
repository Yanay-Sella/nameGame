// NAME QUEST tests that run on the Mac (make test): area data, movement feel, saves, and a physics-based reachability check.
#include <chrono>
#include <map>
#include <math.h>
#include <set>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include "quest.h"
#include "quest_sprites.h"

static int failures = 0;

#define CHECK(condition)                                                     \
    do {                                                                     \
        if (!(condition)) {                                                  \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);      \
            failures++;                                                      \
        }                                                                    \
    } while (0)

const float DT = 1.0f / 60;
const char* const LEGEND = ".#-|^~icgkzf<>vuSCMNPTKABDEFQWRYXO";
const char* const OBJECTS_TO_REACH = "CMNTKQW";
const int REACH_WORLDS[] = {WORLD_NAMULA, WORLD_TEST};   // phases 4-9 add their worlds

static const Sprite* const ALL_QUEST_SPRITES[] = {
    &HERO_KNIGHT_IDLE_A, &HERO_KNIGHT_IDLE_B, &HERO_KNIGHT_WALK_A, &HERO_KNIGHT_WALK_B, &HERO_KNIGHT_JUMP, &HERO_KNIGHT_ATTACK,
    &HERO_KNIGHT_CLIMB, &HERO_KNIGHT_ROLL, &HERO_ARCHER_IDLE_A, &HERO_ARCHER_IDLE_B, &HERO_ARCHER_WALK_A, &HERO_ARCHER_WALK_B,
    &HERO_ARCHER_JUMP, &HERO_ARCHER_ATTACK, &HERO_ARCHER_CLIMB, &HERO_ARCHER_ROLL, &HERO_MAGICIAN_IDLE_A, &HERO_MAGICIAN_IDLE_B,
    &HERO_MAGICIAN_WALK_A, &HERO_MAGICIAN_WALK_B, &HERO_MAGICIAN_JUMP, &HERO_MAGICIAN_ATTACK, &HERO_MAGICIAN_CLIMB,
    &HERO_MAGICIAN_ROLL, &HERO_THIEF_IDLE_A, &HERO_THIEF_IDLE_B, &HERO_THIEF_WALK_A, &HERO_THIEF_WALK_B, &HERO_THIEF_JUMP,
    &HERO_THIEF_ATTACK, &HERO_THIEF_CLIMB, &HERO_THIEF_ROLL, &CHECKPOINT_UNLIT, &CHECKPOINT_LIT_A, &CHECKPOINT_LIT_B,
    &CHECKPOINT_LIT_C, &HEARTH_A, &HEARTH_B, &HEARTH_C, &PORTAL_A, &PORTAL_B, &ARCH_CLOSED, &CHEST_CLOSED, &CHEST_OPEN};

static void questSpritesHaveTheirSize() {
    for (const Sprite* s : ALL_QUEST_SPRITES) CHECK((int)strlen(s->pixels) == s->w * s->h);
}

static bool isWide(const AreaDef& def) { return def.cols == WIDE_COLS && def.rows == WIDE_ROWS; }
static bool isTall(const AreaDef& def) { return def.cols == TALL_COLS && def.rows == TALL_ROWS; }
static int countOf(const AreaDef& def, char letter) {
    int n = 0;
    for (const char* t = def.tiles; *t; t++) n += *t == letter;
    return n;
}

static void areaSizesAndObjectsAreValid() {
    for (int w = 0; w < WORLD_COUNT; w++) {
        const WorldDef& world = worldDef(w);
        for (int i = 0; i < world.count; i++) {
            const AreaDef& def = world.areas[i];
            CHECK((int)strlen(def.tiles) == def.cols * def.rows);
            CHECK(isWide(def) || isTall(def));
            CHECK(strlen(def.name) <= 20);
            for (const char* t = def.tiles; *t; t++) CHECK(strchr(LEGEND, *t) != NULL);
            int portals = countOf(def, 'P');
            CHECK(portals <= 4);
            for (int p = 0; p < 4; p++)
                CHECK(p < portals ? def.portals[p].area >= -1 && def.portals[p].area < world.count : def.portals[p].area == -1);
            int chests = 0, npcs = 0;
            for (int c : def.chests) chests += c >= 0;
            for (int n : def.npcs) npcs += n >= 0;
            CHECK(countOf(def, 'T') == chests);
            if (w != WORLD_NAMULA) CHECK(countOf(def, 'N') == npcs);   // Namula's NPCs outnumber npcs[2]; code places them (phase 3)
        }
    }
}

// A world's area with every gate open and shortcut doors closed.
static AreaState openArea(int world, int index) {
    AreaState area;
    loadArea(area, world, index, ~0u, 0);
    return area;
}

static bool holdsHero(char below) { return tileSolid(below) || below == '-' || below == '|'; }

static void linksArriveOnStandableTiles() {
    for (int w = 0; w < WORLD_COUNT; w++) {
        const WorldDef& world = worldDef(w);
        for (int i = 0; i < world.count; i++) {
            const AreaDef& def = world.areas[i];
            const AreaLink links[8] = {def.left, def.right, def.up, def.down, def.portals[0], def.portals[1], def.portals[2], def.portals[3]};
            for (const AreaLink& link : links) {
                if (link.area < 0) continue;
                CHECK(link.area < world.count);
                AreaState target = openArea(w, link.area);
                char t = tileAt(target, link.tileX, link.tileY);
                bool fits = !tileSolid(t) && !tileSolid(tileAt(target, link.tileX, link.tileY - 1));
                bool ok = t == '|' || (fits && holdsHero(tileAt(target, link.tileX, link.tileY + 1)));
                if (!ok) printf("  link from %d.%d to %d.%d (%d,%d) isn't standable\n", w, i, w, link.area, link.tileX, link.tileY);
                CHECK(ok);
            }
        }
    }
}

// Spec §4: areas joined by an edge touch on the world map on that side (wide 2x1 cells, tall 1x2).
static void edgeLinkedAreasAreMapNeighbours() {
    for (int w = 0; w < WORLD_COUNT; w++) {
        const WorldDef& world = worldDef(w);
        for (int i = 0; i < world.count; i++) {
            const AreaDef& a = world.areas[i];
            int aw = isWide(a) ? 2 : 1, ah = isWide(a) ? 1 : 2;
            const AreaLink* sides[4] = {&a.left, &a.right, &a.up, &a.down};
            for (int s = 0; s < 4; s++) {
                if (sides[s]->area < 0) continue;
                const AreaDef& b = world.areas[sides[s]->area];
                int bw = isWide(b) ? 2 : 1, bh = isWide(b) ? 1 : 2;
                bool rowsOverlap = a.mapY < b.mapY + bh && b.mapY < a.mapY + ah;
                bool colsOverlap = a.mapX < b.mapX + bw && b.mapX < a.mapX + aw;
                bool ok = s == 0 ? b.mapX + bw == a.mapX && rowsOverlap
                        : s == 1 ? b.mapX == a.mapX + aw && rowsOverlap
                        : s == 2 ? b.mapY + bh == a.mapY && colsOverlap
                                 : b.mapY == a.mapY + ah && colsOverlap;
                if (!ok) printf("  areas %d.%d and %d.%d are linked but not map neighbours\n", w, i, w, sides[s]->area);
                CHECK(ok);
            }
        }
    }
}

// A hero standing still at tile (tx, ty)'s bottom-centre, or hanging on the ladder there.
static Hero heroAt(const AreaState& area, int tx, int ty) {
    Hero h = {};
    h.x = (float)(tx * TILE);
    h.y = (float)((ty + 1) * TILE) - HERO_H;
    h.onGround = holdsHero(tileAt(area, tx, ty + 1)) && tileAt(area, tx, ty) != '|';
    h.climbing = !h.onGround && tileAt(area, tx, ty) == '|';
    return h;
}

// --- Reachability (spec §4 rule 8): where the weakest hero can get to, by simulating the real moveHero. ---
// Approximations: c, u and closed g/k count as solid (moveHero treats them so); O platforms (none yet) would count as a
// one-way strip over their whole travel; hazards that only hurt (^, z, f) don't block; every spot starts the hero at rest.
enum Script { WALK_LEFT, WALK_RIGHT, OFF_LEFT, OFF_RIGHT, JUMP_UP, JUMP_LEFT, JUMP_RIGHT, TAP_LEFT, TAP_RIGHT, DROP, CLIMB_UP,
              CLIMB_DOWN, SCRIPT_COUNT };

static int spotKey(int area, int tx, int ty) { return area * 10000 + (ty + 1) * 100 + tx + 1; }   // tiles -1 allowed
static int keyArea(int key) { return key / 10000; }
static int keyX(int key) { return key % 100 - 1; }
static int keyY(int key) { return key / 100 % 100 - 1; }

struct Reach {
    int world;
    std::vector<AreaState> areas;
    std::map<int, std::vector<int>> next;   // spot → spots one script or portal away
    std::vector<double> areaSeconds;        // simulation time spent per area
};

// The spot the hero now occupies: the tile under the hitbox's centre, or under the other column if only that one holds.
static int spotOf(const AreaState& area, const Hero& h) {
    int ty = (int)floorf((h.y + HERO_H - 0.01f) / TILE);
    int tx = (int)floorf((h.x + HERO_W / 2) / TILE);
    if (h.onGround && !holdsHero(tileAt(area, tx, ty + 1))) {
        int left = (int)floorf(h.x / TILE), right = (int)floorf((h.x + HERO_W - 0.01f) / TILE);
        tx = holdsHero(tileAt(area, left, ty + 1)) ? left : right;
    }
    return spotKey(area.index, tx, ty);
}

// Plays one script from a spot; returns the spot it ends on (maybe in another area), or -1 when the hero dies.
static int simulate(const Reach& reach, int from, Script script) {
    const AreaState& area = reach.areas[keyArea(from)];
    const AreaDef& def = *area.def;
    Hero h = heroAt(area, keyX(from), keyY(from));
    float x0 = h.x, y0 = h.y;
    bool airborne = false;
    for (int i = 0; i < 150; i++) {
        float t = i * DT;
        Actions a = {};
        bool walk = script == WALK_LEFT || script == WALK_RIGHT, climb = script == CLIMB_UP || script == CLIMB_DOWN;
        bool inputDone = walk ? fabsf(h.x - x0) >= 8 : climb ? fabsf(h.y - y0) >= 8 : true;
        if (script == WALK_LEFT || script == OFF_LEFT || script == JUMP_LEFT || script == TAP_LEFT) a.moveX = -1;
        if (script == WALK_RIGHT || script == OFF_RIGHT || script == JUMP_RIGHT || script == TAP_RIGHT) a.moveX = 1;
        if (walk && inputDone) a.moveX = 0;
        if (script >= JUMP_UP && script <= DROP) a.jump = i == 0;
        if (script >= JUMP_UP && script <= JUMP_RIGHT) a.jumpHeld = t < 0.3f;
        if (script == TAP_LEFT || script == TAP_RIGHT) a.jumpHeld = t < 0.05f;
        if (script == DROP) a.moveY = i == 0 ? 1 : 0;
        if (climb && !inputDone) a.moveY = script == CLIMB_UP ? -1 : 1;
        moveHero(h, a, area, CLASS_KNIGHT, DT);

        float cx = h.x + HERO_W / 2, cy = h.y + HERO_H / 2;
        const AreaLink* exit = cx < 0 ? &def.left : cx > def.cols * TILE ? &def.right : cy < 0 ? &def.up
                             : cy > def.rows * TILE ? &def.down : nullptr;
        if (exit && exit->area >= 0) return spotKey(exit->area, exit->tileX, exit->tileY);
        if (exit == &def.down || heroInLiquid(h, area)) return -1;
        if (!h.onGround && !h.climbing) airborne = true;
        bool held = h.onGround || h.climbing;
        if (held && airborne) return spotOf(area, h);
        if (held && (walk || climb) && fabsf(h.x - x0) + fabsf(h.y - y0) >= 8 && fabsf(h.vx) < 1 && fabsf(h.vy) < 1) return spotOf(area, h);
    }
    return h.onGround || h.climbing ? spotOf(area, h) : -1;
}

// Every spot one move away: each script, plus Up at an open portal.
static const std::vector<int>& neighbours(Reach& reach, int from) {
    auto found = reach.next.find(from);
    if (found != reach.next.end()) return found->second;
    auto start = std::chrono::steady_clock::now();
    std::vector<int> spots;
    for (int s = 0; s < SCRIPT_COUNT; s++) {
        int to = simulate(reach, from, (Script)s);
        if (to >= 0 && to != from) spots.push_back(to);
    }
    const AreaState& area = reach.areas[keyArea(from)];
    const AreaDef& def = *area.def;
    Hero h = heroAt(area, keyX(from), keyY(from));
    int seen = 0;
    for (int t = 0; t < def.cols * def.rows && h.onGround; t++) {
        if (def.tiles[t] != 'P') continue;
        float px = (t % def.cols) * TILE + TILE / 2.0f, bottom = (float)((t / def.cols + 1) * TILE);
        bool touching = h.x < px + 8 && h.x + HERO_W > px - 8 && h.y < bottom && h.y + HERO_H > bottom - 24;
        if (touching && def.portals[seen].area >= 0)
            spots.push_back(spotKey(def.portals[seen].area, def.portals[seen].tileX, def.portals[seen].tileY));
        seen++;
    }
    reach.areaSeconds[keyArea(from)] += std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    return reach.next[from] = spots;
}

static std::set<int> reachableFrom(Reach& reach, int start) {
    std::set<int> seen = {start};
    std::vector<int> queue = {start};
    while (!queue.empty()) {
        int spot = queue.back();
        queue.pop_back();
        for (int to : neighbours(reach, spot))
            if (seen.insert(to).second) queue.push_back(to);
    }
    return seen;
}

static void everythingIsReachable() {
    double slowest = 0;
    int slowestWorld = 0, slowestArea = 0;
    for (int w : REACH_WORLDS) {
        const WorldDef& world = worldDef(w);
        Reach reach;
        reach.world = w;
        reach.areaSeconds.assign(world.count, 0);
        for (int i = 0; i < world.count; i++) reach.areas.push_back(openArea(w, i));
        // Arrival points: link targets, the start and every checkpoint.
        std::vector<int> arrivals, checkpoints;
        int start = -1;
        for (int i = 0; i < world.count; i++) {
            const AreaDef& def = world.areas[i];
            const AreaLink links[8] = {def.left, def.right, def.up, def.down, def.portals[0], def.portals[1], def.portals[2], def.portals[3]};
            for (const AreaLink& link : links)
                if (link.area >= 0) arrivals.push_back(spotKey(link.area, link.tileX, link.tileY));
            for (int t = 0; t < def.cols * def.rows; t++) {
                int key = spotKey(i, t % def.cols, t / def.cols);
                if (def.tiles[t] == 'C') checkpoints.push_back(key);
                if (def.tiles[t] == 'S' || (def.tiles[t] == 'C' && start < 0 && w != WORLD_NAMULA)) start = key;
            }
        }
        arrivals.insert(arrivals.end(), checkpoints.begin(), checkpoints.end());
        CHECK(start >= 0);
        if (start < 0) continue;
        arrivals.push_back(start);

        std::set<int> reached = reachableFrom(reach, start);
        for (int key : arrivals) {
            if (!reached.count(key)) printf("  %d.%d: arrival point (%d,%d) is unreachable\n", w, keyArea(key), keyX(key), keyY(key));
            CHECK(reached.count(key));
        }
        for (int i = 0; i < world.count; i++) {
            const AreaDef& def = world.areas[i];
            for (int t = 0; t < def.cols * def.rows; t++) {
                if (!strchr(OBJECTS_TO_REACH, def.tiles[t])) continue;
                bool ok = reached.count(spotKey(i, t % def.cols, t / def.cols)) > 0;
                if (!ok) printf("  %d.%d: '%c' at (%d,%d) is unreachable\n", w, i, def.tiles[t], t % def.cols, t / def.cols);
                CHECK(ok);
            }
        }
        // No soft lock: from every arrival point some checkpoint can be reached.
        for (int key : arrivals) {
            std::set<int> from = reachableFrom(reach, key);
            bool ok = false;
            for (int c : checkpoints) ok |= from.count(c) > 0;
            if (!ok) printf("  %d.%d: no checkpoint reachable from (%d,%d)\n", w, keyArea(key), keyX(key), keyY(key));
            CHECK(ok);
        }
        for (int i = 0; i < world.count; i++)
            if (reach.areaSeconds[i] > slowest) { slowest = reach.areaSeconds[i]; slowestWorld = w; slowestArea = i; }
    }
    printf("reachability: slowest area %d.%d took %.2f s\n", slowestWorld, slowestArea, slowest);
}

// --- Movement feel (spec §5), in small hand-made areas. ---
struct TestArea {
    std::string tiles;
    AreaDef def;
    AreaState state;
};

// A closed wide area: a floor at row 20, with `rows` overriding some rows (row index → 80 chars).
static void makeArea(TestArea& t, const std::map<int, std::string>& rows) {
    t.tiles.clear();
    for (int r = 0; r < WIDE_ROWS; r++) {
        auto found = rows.find(r);
        t.tiles += found != rows.end() ? found->second : std::string(WIDE_COLS, r >= 20 ? '#' : '.');
    }
    AreaLink none = {-1, 0, 0};
    t.def = {"TEST", WIDE_COLS, WIDE_ROWS, t.tiles.c_str(), none, none, none, none, {none, none, none, none}, {-1, -1, -1, -1}, {-1, -1}, 0, 0, 0, 0, -1};
    t.state.world = WORLD_TEST;
    t.state.index = 0;
    t.state.def = &t.def;
    t.state.tiles.assign(t.tiles.begin(), t.tiles.end());
}

static void step(Hero& h, const TestArea& t, const Actions& a, HeroClass c = CLASS_KNIGHT, int steps = 1) {
    for (int i = 0; i < steps; i++) moveHero(h, a, t.state, c, DT);
}

// A hero settled on the floor at tile column tx (standing on row 20).
static Hero standing(const TestArea& t, int tx, int floorRow = 20) {
    Hero h = {};
    h.x = (float)(tx * TILE);
    h.y = (float)(floorRow * TILE) - HERO_H;
    step(h, t, Actions());
    return h;
}

static void jumpPeakIsFourAndAHalfTiles() {
    TestArea t;
    makeArea(t, {});
    Hero h = standing(t, 10);
    CHECK(h.onGround);
    float startY = h.y, top = h.y;
    Actions a = {};
    a.jump = a.jumpHeld = true;
    step(h, t, a);
    a.jump = false;
    for (int i = 0; i < 120; i++) { step(h, t, a); top = fminf(top, h.y); }
    CHECK(fabsf(startY - top - 36) <= 1);
    CHECK(h.onGround);

    h = standing(t, 10);
    top = h.y;
    Actions tap = {};
    tap.jump = tap.jumpHeld = true;
    step(h, t, tap);
    for (int i = 0; i < 120; i++) { step(h, t, Actions()); top = fminf(top, h.y); }
    CHECK(startY - top <= 12);
    CHECK(startY - top > 2);
}

static void coyoteAndBufferWork() {
    // A ledge at row 10 over cols 0-19; the hero walks off its right end.
    std::string ledge = std::string(20, '#') + std::string(60, '.');
    TestArea t;
    makeArea(t, {{10, ledge}});
    for (float late : {0.05f, 0.15f}) {
        Hero h = standing(t, 18, 10);
        Actions walk = {};
        walk.moveX = 1;
        while (h.onGround) step(h, t, walk);
        step(h, t, walk, CLASS_KNIGHT, (int)(late * 60));
        Actions jump = walk;
        jump.jump = jump.jumpHeld = true;
        step(h, t, jump);
        if (late < 0.08f) CHECK(h.vy < -150);   // inside coyote time: still a full jump
        else CHECK(h.vy > 0);                   // too late: keeps falling
    }
    // Buffer: X pressed 0.05 s before landing still jumps on landing.
    Hero h = standing(t, 40);
    h.y -= 30;
    h.onGround = false;
    Actions none = {};
    float landY = (float)(20 * TILE) - HERO_H;
    while (h.y + h.vy * 0.05f < landY - 1) step(h, t, none);
    Actions press = {};
    press.jump = press.jumpHeld = true;
    step(h, t, press);
    Actions hold = {};
    hold.jumpHeld = true;
    bool jumped = false;
    for (int i = 0; i < 10; i++) { step(h, t, hold); jumped |= h.vy < -150; }
    CHECK(jumped);
}

static void oneWayDropThrough() {
    std::string platform = std::string(10, '.') + std::string(10, '-') + std::string(60, '.');
    TestArea t;
    makeArea(t, {{16, platform}});
    // Jumping from the floor passes up through it and lands on it.
    Hero h = standing(t, 12);
    Actions jump = {};
    jump.jump = jump.jumpHeld = true;
    step(h, t, jump);
    jump.jump = false;
    step(h, t, jump, CLASS_KNIGHT, 60);
    CHECK(h.onGround && fabsf(h.y + HERO_H - 16 * TILE) < 0.01f);
    // Down + X drops through to the floor.
    Actions drop = {};
    drop.moveY = 1;
    drop.jump = drop.jumpHeld = true;
    step(h, t, drop);
    step(h, t, Actions(), CLASS_KNIGHT, 60);
    CHECK(h.onGround && fabsf(h.y + HERO_H - 20 * TILE) < 0.01f);
}

static void ladderClimbAndJumpOff() {
    // A ladder at col 10 from row 12 down to row 19, next to a ledge at row 12 (cols 11-20).
    TestArea t;
    std::map<int, std::string> rows;
    for (int r = 12; r < 20; r++) {
        std::string row(80, '.');
        row[10] = '|';
        if (r == 12) for (int c = 11; c <= 20; c++) row[c] = '#';
        rows[r] = row;
    }
    makeArea(t, rows);
    Hero h = standing(t, 10);
    Actions up = {};
    up.moveY = -1;
    step(h, t, up);
    CHECK(h.climbing);
    float y = h.y;
    step(h, t, up, CLASS_KNIGHT, 30);
    CHECK(fabsf(y - h.y - 25) < 1.5f);   // 50 px/s
    step(h, t, up, CLASS_KNIGHT, 120);   // over the top: stands on the ladder's top
    CHECK(!h.climbing && h.onGround && fabsf(h.y + HERO_H - 12 * TILE) < 0.01f);
    // Down from the top climbs back down; X with the stick sideways jumps off.
    Actions down = {};
    down.moveY = 1;
    step(h, t, down, CLASS_KNIGHT, 30);
    CHECK(h.climbing && h.y + HERO_H > 12 * TILE);
    Actions off = {};
    off.moveX = -1;
    off.jump = off.jumpHeld = true;
    step(h, t, off);
    CHECK(!h.climbing && h.vy < -150 && h.vx < 0);
}

static void iceSlides() {
    std::string ice(80, 'i');
    TestArea t;
    makeArea(t, {{20, ice}});
    Hero h = standing(t, 10);
    Actions run = {};
    run.moveX = 1;
    step(h, t, run, CLASS_KNIGHT, 120);
    CHECK(fabsf(h.vx - 66) < 0.5f);
    int steps = 0;
    while (h.vx > 0.5f && steps < 600) { step(h, t, Actions()); steps++; }
    CHECK(steps * DT >= 0.4f);

    TestArea floor;
    makeArea(floor, {});
    h = standing(floor, 10);
    step(h, floor, run, CLASS_KNIGHT, 120);
    steps = 0;
    while (h.vx > 0.5f && steps < 600) { step(h, floor, Actions()); steps++; }
    CHECK(steps * DT < 0.1f);
}

static void thiefDoubleJumpsOthersDont() {
    TestArea t;
    makeArea(t, {});
    for (int c = 0; c < CLASS_COUNT; c++) {
        Hero h = standing(t, 10);
        Actions jump = {};
        jump.jump = jump.jumpHeld = true;
        step(h, t, jump, (HeroClass)c);
        Actions hold = {};
        hold.jumpHeld = true;
        step(h, t, hold, (HeroClass)c, 15);
        step(h, t, jump, (HeroClass)c);
        bool doubled = h.vy < -150;
        CHECK(doubled == (c == CLASS_THIEF));
        step(h, t, hold, (HeroClass)c, 10);
        step(h, t, jump, (HeroClass)c);
        CHECK(h.vy > -150);   // only once per airtime
    }
}

// --- Saves (spec §17) ---
const char* const SAVE_FOLDER = "build/tests";

static bool sameSave(const SaveData& a, const SaveData& b) {
    bool same = a.used == b.used && a.heroClass == b.heroClass && a.level == b.level && a.job == b.job && a.names == b.names &&
                fabsf(a.playtime - b.playtime) < 0.05f && a.hp == b.hp && a.mp == b.mp && a.checkpointWorld == b.checkpointWorld &&
                a.checkpointId == b.checkpointId && a.lit == b.lit && a.echoNames == b.echoNames && a.echoWorld == b.echoWorld &&
                a.echoArea == b.echoArea && fabsf(a.echoX - b.echoX) < 0.05f && fabsf(a.echoY - b.echoY) < 0.05f &&
                a.gates == b.gates && a.bosses == b.bosses && a.highest == b.highest && a.ngPlus == b.ngPlus &&
                a.deaths == b.deaths && a.kills == b.kills && a.weapon == b.weapon && a.armor == b.armor && a.refine == b.refine &&
                a.shards == b.shards && a.keys == b.keys && a.ilsaTalks == b.ilsaTalks;
    for (int w = 0; w < SAVED_WORLDS; w++)
        same &= a.visited[w] == b.visited[w] && a.chests[w] == b.chests[w] && a.shortcuts[w] == b.shortcuts[w];
    for (int i = 0; i < ITEM_COUNT; i++) same &= a.items[i] == b.items[i];
    for (int i = 0; i < QUEST_COUNT; i++) same &= a.quests[i] == b.quests[i];
    return same;
}

static void saveRoundTrip() {
    SaveData s;
    s.used = true;
    s.heroClass = 3; s.level = 27; s.job = 2; s.names = 123456; s.playtime = 4321.5f; s.hp = 310; s.mp = 77;
    s.checkpointWorld = 4; s.checkpointId = 1; s.lit = 0x1ab; s.echoNames = 999; s.echoWorld = 3; s.echoArea = 17;
    s.echoX = 401.5f; s.echoY = 92; s.gates = 0x1f; s.bosses = 0x70f; s.highest = 5; s.ngPlus = 2; s.deaths = 41; s.kills = 1200;
    s.weapon = 4; s.armor = 3; s.refine = 2; s.shards = 6; s.keys = 0x2a5c3; s.ilsaTalks = 4;
    for (int w = 0; w < SAVED_WORLDS; w++) { s.visited[w] = 0xfffff >> w; s.chests[w] = 0x15u << w; s.shortcuts[w] = (uint32_t)w; }
    for (int i = 0; i < ITEM_COUNT; i++) s.items[i] = i * 3 + 1;
    for (int i = 0; i < QUEST_COUNT; i++) s.quests[i] = i + 2;
    CHECK(saveWrite(SAVE_FOLDER, 0, s));
    SaveData back;
    CHECK(saveRead(SAVE_FOLDER, 0, back));
    CHECK(sameSave(s, back));
    saveDelete(SAVE_FOLDER, 0);
}

static void missingSaveIsUnused() {
    saveDelete(SAVE_FOLDER, 1);
    SaveData s;
    s.level = 9;
    CHECK(!saveRead(SAVE_FOLDER, 1, s));
    CHECK(!s.used && s.level == 1 && s.items[ITEM_RED] == 5 && s.lit == 1);
}

static void deleteRemovesTheSlot() {
    SaveData s;
    s.used = true;
    CHECK(saveWrite(SAVE_FOLDER, 2, s));
    CHECK(saveRead(SAVE_FOLDER, 2, s));
    CHECK(saveDelete(SAVE_FOLDER, 2));
    CHECK(!saveRead(SAVE_FOLDER, 2, s));
}

int main() {
    questSpritesHaveTheirSize();
    areaSizesAndObjectsAreValid();
    linksArriveOnStandableTiles();
    edgeLinkedAreasAreMapNeighbours();
    jumpPeakIsFourAndAHalfTiles();
    coyoteAndBufferWork();
    oneWayDropThrough();
    ladderClimbAndJumpOff();
    iceSlides();
    thiefDoubleJumpsOthersDont();
    saveRoundTrip();
    missingSaveIsUnused();
    deleteRemovesTheSlot();
    everythingIsReachable();
    printf(failures ? "%d quest check(s) failed\n" : "all quest tests passed\n", failures);
    return failures ? 1 : 0;
}
