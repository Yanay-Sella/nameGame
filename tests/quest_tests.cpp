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
#include "quest_art.h"
#include "quest_sprites.h"
#include <algorithm>

static int failures = 0;

#define CHECK(condition)                                                     \
    do {                                                                     \
        if (!(condition)) {                                                  \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition);      \
            failures++;                                                      \
        }                                                                    \
    } while (0)

const float DT = 1.0f / 60;
const char* const LEGEND = ".#-|^~icgkzf<>vuSCMNPTKABDEFQWRYXLO";
const char* const OBJECTS_TO_REACH = "CMNTKQW";
const int REACH_WORLDS[] = {WORLD_NAMULA, 1, 2, 3, WORLD_TEST};   // phases 5-9 add their worlds

static const Sprite* const ALL_QUEST_SPRITES[] = {
    &HERO_KNIGHT_IDLE_1, &HERO_KNIGHT_IDLE_2, &HERO_KNIGHT_WALK_1, &HERO_KNIGHT_WALK_2, &HERO_KNIGHT_WALK_3,
    &HERO_KNIGHT_WALK_4, &HERO_KNIGHT_JUMP, &HERO_KNIGHT_FALL, &HERO_KNIGHT_ATTACK_1, &HERO_KNIGHT_ATTACK_2,
    &HERO_KNIGHT_ATTACK_3, &HERO_KNIGHT_CLIMB_1, &HERO_KNIGHT_CLIMB_2, &HERO_KNIGHT_ROLL_1, &HERO_KNIGHT_ROLL_2,
    &HERO_KNIGHT_HURT, &HERO_ARCHER_IDLE_1, &HERO_ARCHER_IDLE_2, &HERO_ARCHER_WALK_1, &HERO_ARCHER_WALK_2,
    &HERO_ARCHER_WALK_3, &HERO_ARCHER_WALK_4, &HERO_ARCHER_JUMP, &HERO_ARCHER_FALL, &HERO_ARCHER_ATTACK_1,
    &HERO_ARCHER_ATTACK_2, &HERO_ARCHER_ATTACK_3, &HERO_ARCHER_CLIMB_1, &HERO_ARCHER_CLIMB_2, &HERO_ARCHER_ROLL_1,
    &HERO_ARCHER_ROLL_2, &HERO_ARCHER_HURT, &HERO_MAGICIAN_IDLE_1, &HERO_MAGICIAN_IDLE_2, &HERO_MAGICIAN_WALK_1,
    &HERO_MAGICIAN_WALK_2, &HERO_MAGICIAN_WALK_3, &HERO_MAGICIAN_WALK_4, &HERO_MAGICIAN_JUMP, &HERO_MAGICIAN_FALL,
    &HERO_MAGICIAN_ATTACK_1, &HERO_MAGICIAN_ATTACK_2, &HERO_MAGICIAN_ATTACK_3, &HERO_MAGICIAN_CLIMB_1,
    &HERO_MAGICIAN_CLIMB_2, &HERO_MAGICIAN_ROLL_1, &HERO_MAGICIAN_ROLL_2, &HERO_MAGICIAN_HURT, &HERO_THIEF_IDLE_1,
    &HERO_THIEF_IDLE_2, &HERO_THIEF_WALK_1, &HERO_THIEF_WALK_2, &HERO_THIEF_WALK_3, &HERO_THIEF_WALK_4,
    &HERO_THIEF_JUMP, &HERO_THIEF_FALL, &HERO_THIEF_ATTACK_1, &HERO_THIEF_ATTACK_2, &HERO_THIEF_ATTACK_3,
    &HERO_THIEF_CLIMB_1, &HERO_THIEF_CLIMB_2, &HERO_THIEF_ROLL_1, &HERO_THIEF_ROLL_2, &HERO_THIEF_HURT,
    &CHECKPOINT_UNLIT, &CHECKPOINT_LIT_A, &CHECKPOINT_LIT_B, &CHECKPOINT_LIT_C, &HEARTH_A, &HEARTH_B, &HEARTH_C,
    &PORTAL_A, &PORTAL_B, &ARCH_CLOSED, &CHEST_CLOSED, &CHEST_OPEN, &CHEST_AJAR, &LEVER_OFF, &LEVER_ON, &NPC_ILSA_A,
    &NPC_ILSA_B, &NPC_MARRA_A, &NPC_MARRA_B, &NPC_BRANN_A, &NPC_BRANN_B, &NPC_LIO_A, &NPC_LIO_B, &NPC_PELL_A, &NPC_PELL_B,
    &NPC_SELA_A, &NPC_SELA_B, &NPC_ODO_A, &NPC_ODO_B, &NPC_WREN_A, &NPC_WREN_B, &NPC_YSOLDE_A, &NPC_YSOLDE_B, &NPC_KIT_A,
    &NPC_KIT_B, &NPC_TOBIN_A, &NPC_TOBIN_B, &NPC_GATEKEEPER_A, &NPC_GATEKEEPER_B, &NPC_SURVEYOR_A, &NPC_SURVEYOR_B,
    &ICON_RED, &ICON_BIG, &ICON_MANA, &ICON_WHETSTONE, &ICON_TONIC, &ICON_FEATHER};

// Every palette sprite of a world's art, for the size and character checks.
static std::vector<const Sprite*> artSprites(const WorldArt& art) {
    std::vector<const Sprite*> all;
    for (const auto& variants : art.ground)
        for (const Sprite* s : variants) if (s) all.push_back(s);
    const Sprite* single[] = {art.fringe[0], art.fringe[1], art.fringe[2], art.drapeLeft, art.drapeRight, art.platformLeft,
                              art.platformMid, art.platformRight, art.bracket, art.rope, art.post, art.ladder[0], art.ladder[1],
                              art.liquid[0], art.liquid[1], art.liquid[2]};
    all.insert(all.end(), single, single + sizeof single / sizeof single[0]);
    for (int i = 0; i < art.propCount; i++)
        for (int f = 0; f < art.props[i].frameCount; f++) all.push_back(art.props[i].frames[f]);
    for (int i = 0; i < art.landmarkCount; i++)
        for (int f = 0; f < art.landmarks[i].frameCount; f++) all.push_back(art.landmarks[i].frames[f]);
    return all;
}

static void questSpritesHaveTheirSize() {
    for (const Sprite* s : ALL_QUEST_SPRITES) CHECK((int)strlen(s->pixels) == s->w * s->h);
    for (int w = 0; w < WORLD_COUNT; w++)
        if (worldArt(w))
            for (const Sprite* s : artSprites(*worldArt(w))) CHECK(s && (int)strlen(s->pixels) == s->w * s->h);
}

static void palSpritesUseOnlyPaletteChars() {
    for (int w = 0; w < WORLD_COUNT; w++)
        if (worldArt(w))
            for (const Sprite* s : artSprites(*worldArt(w)))
                for (const char* c = s->pixels; *c; c++) CHECK(strchr("0123456789abcdef.", *c) != NULL);
}

static void everyGroundShapeHasArt() {
    for (int w : {WORLD_NAMULA, 1, 2, 3}) {
        const WorldArt& art = *worldArt(w);
        for (int shape = 0; shape < TILE_SHAPE_COUNT; shape++) CHECK(art.ground[shape][0] != nullptr);
        CHECK(art.propCount >= (w == WORLD_NAMULA ? 12 : 10) && art.landmarkCount >= 6);   // spec §22, §13
    }
}

static void propsNeverBlockSpots() {
    for (int w : {WORLD_NAMULA, 1, 2, 3})
    for (int i = 0; i < worldDef(w).count; i++) {
        AreaState area;
        loadArea(area, w, i, 0, 0);
        std::vector<PlacedProp> props;
        placeProps(area, props);
        CHECK(!props.empty());
        for (const PlacedProp& p : props) {
            CHECK(!propTileExcluded(area, p.tx, p.ty));
            for (int dy = -1; dy <= 1; dy++)
                for (int dx = -1; dx <= 1; dx++) CHECK(!strchr("SCMNPTKQWXLO", tileAt(area, p.tx + dx, p.ty + dy)));
        }
    }
}

static void propPlacementIsStable() {
    AreaState area;
    loadArea(area, WORLD_NAMULA, 0, 0, 0);
    std::vector<PlacedProp> a, b;
    placeProps(area, a);
    placeProps(area, b);
    CHECK(a.size() == b.size());
    for (size_t i = 0; i < a.size() && i < b.size(); i++) CHECK(a[i].kind == b[i].kind && a[i].x == b[i].x && a[i].y == b[i].y);
}

static float luminance(uint32_t c) {
    return (0.2126f * ((c >> 16) & 0xFF) + 0.7152f * ((c >> 8) & 0xFF) + 0.0722f * (c & 0xFF)) / 255;
}

// Spec §22: L0-L1 at most 30 % luminance, L2 40 %, L3 50 % and at most 25 % of the view; never m or y.
static void layersObeyTheValueRulesIn(int world) {
    prepareWorldArt(world);
    const float caps[4] = {0.30f, 0.30f, 0.40f, 0.50f};
    for (int l = 0; l < 4; l++) {
        const uint32_t* pixels = layerPixels(l);
        CHECK(pixels != nullptr);
        if (!pixels) continue;
        float brightest = 0;
        int opaque = 0;
        for (int i = 0; i < LAYER_W * LAYER_H; i++) {
            if (!pixels[i]) continue;
            brightest = std::max(brightest, luminance(pixels[i]));
            CHECK(pixels[i] != paletteColor('m') && pixels[i] != paletteColor('y'));
            opaque += i / LAYER_W < QUEST_H;
        }
        if (brightest > caps[l]) printf("  world %d layer %d is %.0f %% bright (cap %.0f %%)\n", world, l, brightest * 100, caps[l] * 100);
        CHECK(brightest <= caps[l]);
        if (l == 3) CHECK(opaque <= LAYER_W * QUEST_H / 4);
    }
}

static void layersObeyTheValueRules() {
    for (int w : {WORLD_NAMULA, 1, 2, 3}) layersObeyTheValueRulesIn(w);
}

static void outlineSurroundsTheSprite() {
    static uint32_t pixels[64 * 64];
    screenSetTarget(pixels, 64, 64);
    std::fill(pixels, pixels + 64 * 64, 0u);
    const Sprite plus = {3, 3, ".w.www.w."};
    drawSpriteOutlined(plus, 10, 10, false, 0xFF000001);
    CHECK(pixels[11 * 64 + 11] == paletteColor('w'));
    CHECK(pixels[9 * 64 + 11] == 0xFF000001 && pixels[13 * 64 + 11] == 0xFF000001);   // above and below the plus
    CHECK(pixels[11 * 64 + 9] == 0xFF000001 && pixels[11 * 64 + 13] == 0xFF000001);
    CHECK(pixels[10 * 64 + 10] == 0xFF000001);   // the corner cell beside two arms
    CHECK(pixels[8 * 64 + 8] == 0);
    screenSetTarget(screenPixels, SCREEN_W, SCREEN_H);
}

static void areaLandmarksExist() {
    for (int w = 0; w < WORLD_COUNT; w++) {
        const WorldArt* art = worldArt(w);
        for (int i = 0; i < worldDef(w).count; i++) {
            const AreaDef& def = worldDef(w).areas[i];
            int n = 0;
            for (const char* t = def.tiles; *t; t++) {
                if (*t != 'L') continue;
                CHECK(n < 4 && art && def.landmarks[n] >= 1 && def.landmarks[n] <= art->landmarkCount);
                n++;
            }
            for (int k = n; k < 4; k++) CHECK(def.landmarks[k] == 0);
            if (w == WORLD_NAMULA) CHECK(n >= 1);
        }
    }
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
            CHECK(countOf(def, 'N') == npcs);   // unused npcs[] entries are -1 (0 would be Ilsa)
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
        bool inputDone = walk ? fabsf(h.x - x0) >= 8 * DP : climb ? fabsf(h.y - y0) >= 8 * DP : true;
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
        if (held && (walk || climb) && fabsf(h.x - x0) + fabsf(h.y - y0) >= 8 * DP && fabsf(h.vx) < 1 * DP && fabsf(h.vy) < 1 * DP) return spotOf(area, h);
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
        bool touching = h.x < px + 8 * DP && h.x + HERO_W > px - 8 * DP && h.y < bottom && h.y + HERO_H > bottom - 24 * DP;
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

// --- World layout (spec §4 layout rules, §8 counts, §10 chests), phase 4 on. ---
// A world's spot graph with the given gates (shortcut doors closed), for the rules below.
static Reach worldReach(int world, uint32_t gates) {
    Reach reach;
    reach.world = world;
    reach.areaSeconds.assign(worldDef(world).count, 0);
    for (int i = 0; i < worldDef(world).count; i++) {
        AreaState area;
        loadArea(area, world, i, gates, 0);
        reach.areas.push_back(area);
    }
    return reach;
}

static int firstSpotOf(int world, int area, char letter) {
    const AreaDef& def = worldDef(world).areas[area];
    for (int t = 0; t < def.cols * def.rows; t++)
        if (def.tiles[t] == letter) return spotKey(area, t % def.cols, t / def.cols);
    return -1;
}

// The areas a hero walking from area 0's checkpoint can move between, as directed area-to-area steps.
static std::map<int, std::set<int>> areaSteps(Reach& reach) {
    std::map<int, std::set<int>> steps;
    for (int spot : reachableFrom(reach, firstSpotOf(reach.world, 0, 'C')))
        for (int to : neighbours(reach, spot))
            if (keyArea(to) != keyArea(spot)) steps[keyArea(spot)].insert(keyArea(to));
    return steps;
}

// Fewest areas from 0 to `last` (both included), by breadth-first search over the area steps.
static std::vector<int> mainPath(const std::map<int, std::set<int>>& steps, int last) {
    std::map<int, int> from = {{0, -1}};
    std::vector<int> queue = {0};
    for (size_t i = 0; i < queue.size(); i++) {
        auto found = steps.find(queue[i]);
        if (found == steps.end()) continue;
        for (int to : found->second)
            if (!from.count(to)) from[to] = queue[i], queue.push_back(to);
    }
    std::vector<int> path;
    for (int a = from.count(last) ? last : -1; a >= 0; a = from[a]) path.insert(path.begin(), a);
    return path;
}

// Every area joined to `area` by an edge link or a portal, either way.
static std::set<int> linkedAreas(int world, int area) {
    std::set<int> out;
    const WorldDef& w = worldDef(world);
    for (int i = 0; i < w.count; i++) {
        const AreaDef& d = w.areas[i];
        const AreaLink links[8] = {d.left, d.right, d.up, d.down, d.portals[0], d.portals[1], d.portals[2], d.portals[3]};
        for (const AreaLink& l : links) {
            if (l.area < 0) continue;
            if (i == area && l.area != area) out.insert(l.area);
            if (l.area == area && i != area) out.insert(i);
        }
    }
    return out;
}

static int areaWithLetter(int world, char letter, int skip = 0) {
    for (int i = 0; i < worldDef(world).count; i++)
        if (countOf(worldDef(world).areas[i], letter) > 0 && skip-- == 0) return i;
    return -1;
}

// With the gate closed, the boss arena and the second checkpoint can't be reached from area 0; the mini-boss can.
static void gateGuardsTheSecondHalf(int world) {
    Reach reach = worldReach(world, 0);
    std::set<int> reached = reachableFrom(reach, firstSpotOf(world, 0, 'C'));
    std::set<int> areas;
    for (int spot : reached) areas.insert(keyArea(spot));
    int miniBoss = areaWithLetter(world, 'Q'), depths = areaWithLetter(world, 'C', 1);
    CHECK(miniBoss >= 0 && reached.count(firstSpotOf(world, miniBoss, 'Q')));
    CHECK(!areas.count(19));
    CHECK(depths > 0 && !reached.count(firstSpotOf(world, depths, 'C')));
}

// Spec §4 rules 1-6 and 9, and §8's enemy counts per band.
static void worldLayoutRulesHold(int world) {
    const WorldDef& w = worldDef(world);
    CHECK(w.count == 20);
    if (w.count != 20) return;
    Reach reach = worldReach(world, ~0u);
    std::vector<int> path = mainPath(areaSteps(reach), 19);
    printf("world %d main path:", world);
    for (int a : path) printf(" %d", a);
    printf("\n");
    // 1, 2: the entry, the boss arena last, the mini-boss arena 6th to 8th of 12-14.
    CHECK(countOf(w.areas[0], 'C') == 1 && countOf(w.areas[19], 'W') == 1 && (w.areas[19].flags & AREA_BOSS));
    CHECK(path.size() >= 12 && path.size() <= 14);
    int mini = -1, gate = -1, depths = -1;
    for (int k = 0; k < (int)path.size(); k++) {
        const AreaDef& d = w.areas[path[k]];
        if (d.flags & AREA_MINIBOSS) mini = k;
        if (countOf(d, 'g') > 0 && gate < 0) gate = k;
        if (countOf(d, 'C') > 0 && path[k] != 0) depths = k;
    }
    CHECK(mini >= 5 && mini <= 7 && countOf(w.areas[path[mini]], 'Q') == 1);
    // 3: the gate is the area right after the mini-boss arena; 4: the second C 2+ after it and 3+ before the boss.
    CHECK(gate == mini + 1);
    CHECK(depths >= gate + 2 && (int)path.size() - 1 - depths >= 3);
    // 5: 3 dead ends with a chest or NPC, a loop, a shortcut, 2 portals, 2-4 tall areas.
    int deadEnds = 0, edges = 0, portals = 0, tall = 0, levers = 0, doors = 0;
    for (int i = 0; i < w.count; i++) {
        const AreaDef& d = w.areas[i];
        std::set<int> linked = linkedAreas(world, i);
        edges += (int)linked.size();
        if (linked.size() == 1 && countOf(d, 'T') + countOf(d, 'N') > 0) deadEnds++;
        for (int k = 0; k < 4; k++) portals += d.portals[k].area >= 0;
        tall += isTall(d);
        levers += countOf(d, 'K') > 0 && d.shortcut >= 0;
        doors += countOf(d, 'k') > 0;
    }
    CHECK(deadEnds >= 3);
    CHECK(edges / 2 >= w.count);   // a connected graph with as many links as areas has a cycle
    CHECK(levers >= 1 && doors >= 1);
    CHECK(portals >= 2);
    CHECK(tall >= 2 && tall <= 4);
    // 6: a surveyor in the first 4 main-path areas and one after the gate.
    bool early = false, late = false;
    for (int k = 0; k < (int)path.size(); k++) {
        early |= k < 4 && countOf(w.areas[path[k]], 'M') > 0;
        late |= k > gate && countOf(w.areas[path[k]], 'M') > 0;
    }
    CHECK(early && late);
    // §8 counts by band; arenas have only their boss; 9: area 18 is quiet.
    for (int i = 0; i < w.count; i++) {
        const AreaDef& d = w.areas[i];
        int a = countOf(d, 'A') + countOf(d, 'B'), soldiers = countOf(d, 'D') + countOf(d, 'E'), elites = countOf(d, 'F');
        int all = a + soldiers + elites;
        bool ok = (d.flags & (AREA_MINIBOSS | AREA_BOSS)) || i == 18 ? all == 0
                : d.band == 0 ? all >= 3 && all <= 5 && soldiers + elites == 0
                : d.band == 1 ? all >= 5 && all <= 7 && elites <= 1
                              : all >= 7 && all <= 10 && elites <= 3;
        if (!ok) printf("  %d.%d (band %d) has %d enemies (%d soldiers, %d elites)\n", world, i, d.band, all, soldiers, elites);
        CHECK(ok);
    }
}

// Spec §10: every chest of the world's list appears once; the first `deadEnds` (shard, map page, key item) in dead ends.
static void chestsMatchTheWorldList(int world, int deadEnds) {
    int seen[6] = {};
    for (int i = 0; i < worldDef(world).count; i++) {
        const AreaDef& d = worldDef(world).areas[i];
        for (int c : d.chests) {
            if (c < 0) continue;
            CHECK(c < 6);
            if (c >= 6) continue;
            seen[c]++;
            if (c < deadEnds) CHECK(linkedAreas(world, i).size() == 1);
        }
    }
    for (int c = 0; c < 6; c++) CHECK(seen[c] == 1);
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
    t.def = {"TEST", WIDE_COLS, WIDE_ROWS, t.tiles.c_str(), none, none, none, none, {none, none, none, none}, {-1, -1, -1, -1}, {-1, -1, -1, -1, -1, -1, -1, -1}, 0, 0, 0, 0, -1, {}};
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
    CHECK(fabsf(startY - top - 36 * DP) <= 1 * DP);
    CHECK(h.onGround);

    h = standing(t, 10);
    top = h.y;
    Actions tap = {};
    tap.jump = tap.jumpHeld = true;
    step(h, t, tap);
    for (int i = 0; i < 120; i++) { step(h, t, Actions()); top = fminf(top, h.y); }
    CHECK(startY - top <= 12 * DP);
    CHECK(startY - top > 2 * DP);
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
        if (late < 0.08f) CHECK(h.vy < -150 * DP);   // inside coyote time: still a full jump
        else CHECK(h.vy > 0);                   // too late: keeps falling
    }
    // Buffer: X pressed 0.05 s before landing still jumps on landing.
    Hero h = standing(t, 40);
    h.y -= 30 * DP;
    h.onGround = false;
    Actions none = {};
    float landY = (float)(20 * TILE) - HERO_H;
    while (h.y + h.vy * 0.05f < landY - 1 * DP) step(h, t, none);
    Actions press = {};
    press.jump = press.jumpHeld = true;
    step(h, t, press);
    Actions hold = {};
    hold.jumpHeld = true;
    bool jumped = false;
    for (int i = 0; i < 10; i++) { step(h, t, hold); jumped |= h.vy < -150 * DP; }
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
    CHECK(fabsf(y - h.y - 25 * DP) < 1.5f * DP);   // 50 dp/s
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
    CHECK(!h.climbing && h.vy < -150 * DP && h.vx < 0);
}

static void iceSlides() {
    std::string ice(80, 'i');
    TestArea t;
    makeArea(t, {{20, ice}});
    Hero h = standing(t, 10);
    Actions run = {};
    run.moveX = 1;
    step(h, t, run, CLASS_KNIGHT, 120);
    CHECK(fabsf(h.vx - 66 * DP) < 0.5f * DP);
    int steps = 0;
    while (h.vx > 0.5f * DP && steps < 600) { step(h, t, Actions()); steps++; }
    CHECK(steps * DT >= 0.4f);

    TestArea floor;
    makeArea(floor, {});
    h = standing(floor, 10);
    step(h, floor, run, CLASS_KNIGHT, 120);
    steps = 0;
    while (h.vx > 0.5f * DP && steps < 600) { step(h, floor, Actions()); steps++; }
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
        bool doubled = h.vy < -150 * DP;
        CHECK(doubled == (c == CLASS_THIEF));
        step(h, t, hold, (HeroClass)c, 10);
        step(h, t, jump, (HeroClass)c);
        CHECK(h.vy > -150 * DP);   // only once per airtime
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
    CHECK(!s.used && s.level == 1 && s.items[ITEM_RED] == 5 && s.lit == (1 | 1u << 2));   // the hearth and world 1's entry
}

static void deleteRemovesTheSlot() {
    SaveData s;
    s.used = true;
    CHECK(saveWrite(SAVE_FOLDER, 2, s));
    CHECK(saveRead(SAVE_FOLDER, 2, s));
    CHECK(saveDelete(SAVE_FOLDER, 2));
    CHECK(!saveRead(SAVE_FOLDER, 2, s));
}

// --- Combat (spec §6-§8): formulas, attacks, enemies, death and the name echo. ---
static void levelCostMatchesSpec() {
    CHECK(levelCost(1) == 100);
    CHECK(levelCost(4) == 173);
    CHECK(levelCost(14) == 1070);
    CHECK(levelCost(39) == 102067);   // round(100 * 1.2^38); the spec's example says 102,064
}

// Integer damage can't be within 1 % of a small HP share, so the tolerance is 1 % or 1 HP, whichever is larger.
static void referenceHitIsTwentyPercent() {
    const float FRACTIONS[3] = {0.15f, 0.20f, 0.25f};
    for (int w = 1; w <= 7; w++) {
        RefStats ref = referenceHero(w);
        for (float f : FRACTIONS) {
            int hit = damageToHero(enemyAttack(w, f, 0), (int)lroundf(ref.def));
            float want = f * ref.hp;
            bool ok = fabsf(hit - want) <= fmaxf(0.01f * want, 1);
            if (!ok) printf("  world %d fraction %.2f: hit %d, want %.1f\n", w, f, hit, want);
            CHECK(ok);
        }
    }
}

static void classesDifferAsDesigned() {
    HeroStats st[CLASS_COUNT];
    for (int c = 0; c < CLASS_COUNT; c++) {
        SaveData s;
        s.heroClass = c;
        s.level = 10;
        st[c] = heroStats(s);
        CHECK(SKILL_NAMES[c][3] && SKILL_NAMES[c][3][0]);
    }
    for (int c = 0; c < CLASS_COUNT; c++) {
        if (c != CLASS_KNIGHT) CHECK(st[CLASS_KNIGHT].maxHp > st[c].maxHp && st[CLASS_KNIGHT].def > st[c].def);
        if (c != CLASS_MAGICIAN) CHECK(st[CLASS_MAGICIAN].maxMp > st[c].maxMp && st[CLASS_MAGICIAN].atk > st[c].atk);
        if (c != CLASS_THIEF) CHECK(st[CLASS_THIEF].walk > st[c].walk);
    }
}

// A quest in play in test area `t` (world `world`), the hero standing at column tx facing right, no crits.
static void startQuest(Quest& q, TestArea& t, int heroClass, int level, int job, int tx, int world = WORLD_TEST) {
    q = Quest();
    q.folder = SAVE_FOLDER;
    q.slot = 2;
    q.save.used = true;
    q.save.heroClass = heroClass;
    q.save.level = level;
    q.save.job = job;
    t.state.world = world;
    q.area = t.state;
    q.hero = standing(t, tx);
    questRefreshStats(q, true);
    q.stats.crit = 0;
    spawnEnemies(q);
    q.mode = QMODE_PLAY;
}

// Presses happen on the first step only; held inputs (fire, stick, X held) stay for every step.
static void play(Quest& q, Actions a, int steps) {
    for (int i = 0; i < steps; i++) {
        questUpdate(q, a, DT);
        a.jump = a.rocket = a.back = a.up = a.down = a.left = a.right = a.autopilot = a.godMode = false;
    }
}

static std::string row(int length, int from, int to, char c) {
    std::string r(length, '.');
    for (int i = from; i <= to; i++) r[i] = c;
    return r;
}

static std::string withLetter(int col, char letter) {
    std::string r(WIDE_COLS, '.');
    r[col] = letter;
    return r;
}

// Bolts of a level-4 Magician (world 1's reference level): x1.1 at 0.9-1.1 always takes 3 to kill world-1 fodder.
static void basicAttackKillsFodderInThreeHits() {
    TestArea t;
    makeArea(t, {{19, withLetter(24, 'A')}});
    Quest q;
    startQuest(q, t, CLASS_MAGICIAN, 4, 0, 14);
    q.godMode = true;
    CHECK(q.enemies.size() == 1 && q.enemies[0].hp == enemyHealth(1, 3, 0));
    Actions a = {};
    a.fire = true;
    int hits = 0, last = q.enemies[0].hp;
    for (int i = 0; i < 600 && q.enemies[0].alive; i++) {
        play(q, a, 1);
        if (q.enemies[0].hp < last) hits++;
        last = q.enemies[0].hp;
    }
    CHECK(!q.enemies[0].alive && hits == 3);
    CHECK(q.save.names == enemyNames(1, 1, 0, false) && q.save.kills == 1);
    CHECK(q.killed[WORLD_TEST][0] & 1);
}

// Every redrawn ground enemy (worlds 1-2) stands tall enough for a basic arrow, flying at the bow's height, to hit it (spec §6, §22).
static void arrowsHitEveryGroundEnemy() {
    for (int world = 1; world <= 3; world++)
    for (int slot = 0; slot < ROSTER_SLOTS; slot++) {
        if (rosterType(world, slot).behaviour == BEH_FLYER) continue;
        TestArea t;
        makeArea(t, {{19, withLetter(24, ROSTER_LETTERS[slot])}});
        Quest q;
        startQuest(q, t, CLASS_ARCHER, 4, 0, 14, world);
        q.godMode = true;
        if (q.enemies[0].state == ES_HANG) {   // a dropper is shot at once it has dropped onto the floor
            q.enemies[0].state = ES_PATROL;
            q.enemies[0].y = 20 * TILE - q.enemies[0].h;
        }
        Actions a = {};
        a.fire = true;
        int hits = 0, last = q.enemies[0].hp;
        for (int i = 0; i < 120; i++) {
            q.hero.facingLeft = q.enemies[0].x < q.hero.x;
            play(q, a, 1);
            if (q.enemies[0].hp < last) hits++;
            last = q.enemies[0].hp;
        }
        if (hits == 0) printf("  the archer's arrows never hit %s\n", rosterType(world, slot).name);
        CHECK(hits > 0);
    }
}

static void knightSwingHitsOnlyInItsWindow() {
    TestArea t;
    makeArea(t, {{19, withLetter(16, 'D')}});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 10, 0, 14);
    q.godMode = true;
    q.enemies[0].x = q.hero.x + HERO_W + 2 * DP;   // just in front, inside the 22 x 18 box
    Actions a = {};
    a.fire = true;
    play(q, a, 1);
    a.fire = false;
    float hitAt = -1;
    for (int i = 1; i < 30 && hitAt < 0; i++) {
        play(q, a, 1);
        if (q.enemies[0].hp < q.enemies[0].maxHp) hitAt = i * DT;
    }
    CHECK(hitAt >= 0.08f - DT && hitAt <= 0.18f + DT);
    // Out of reach (30 dp ahead): never hit.
    startQuest(q, t, CLASS_KNIGHT, 10, 0, 14);
    q.godMode = true;
    q.enemies[0].x = q.hero.x + HERO_W + 30 * DP;
    a.fire = true;
    play(q, a, 30);
    CHECK(q.enemies[0].hp == q.enemies[0].maxHp);
}

// Thief, R1 with up held: the stab hits an enemy right above the head; without up it misses it.
static void thiefStabsUp() {
    TestArea t;
    makeArea(t, {{19, withLetter(16, 'D')}});
    for (int up = 1; up >= 0; up--) {
        Quest q;
        startQuest(q, t, CLASS_THIEF, 10, 0, 14);
        q.godMode = true;
        Enemy& e = q.enemies[0];
        Actions a = {};
        a.fire = true;
        a.moveY = up ? -1 : 0;
        for (int i = 0; i < 10; i++) {   // the enemy held just above the head through the stab's window
            e.x = q.hero.x + HERO_W / 2 - e.w / 2;
            e.y = q.hero.y - e.h - 2 * DP;
            play(q, a, 1);
        }
        CHECK(q.hero.attackUp == (up == 1));
        CHECK((e.hp < e.maxHp) == (up == 1));
    }
}

// R2 just after R1: the Power Slash (x2.4) still lands on the enemy the basic swing would have hit.
static void skillCastMidSwingStillHits() {
    TestArea t;
    makeArea(t, {{19, withLetter(16, 'D')}});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 10, 0, 14);
    q.godMode = true;
    q.enemies[0].x = q.hero.x + HERO_W + 2 * DP;
    Actions a = {};
    a.fire = true;
    play(q, a, 1);
    a.fire = false;
    play(q, a, 2);
    a.rocket = true;
    play(q, a, 40);
    CHECK(q.hero.skill == -1 && q.enemies[0].maxHp - q.enemies[0].hp >= (int)(q.stats.atk * 2.4f * 0.9f));
}

static void spikesHurtAndBounce() {
    TestArea t;
    makeArea(t, {{19, withLetter(10, '^')}});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 10, 0, 10);
    int hp = q.hero.hp;
    updateCombat(q, Actions(), DT);
    CHECK(q.hero.hp == hp - (int)lroundf(q.stats.maxHp * 0.2f) && q.hero.vy == -180 * DP && q.hero.iframes > 0.9f);
}

static void arrowsStopAtWalls() {
    TestArea t;
    std::map<int, std::string> rows;
    for (int r = 0; r < 20; r++) rows[r] = withLetter(18, '#');
    rows[19][24] = 'D';
    makeArea(t, rows);
    Quest q;
    startQuest(q, t, CLASS_ARCHER, 10, 0, 14);
    q.godMode = true;
    Actions a = {};
    a.fire = true;
    play(q, a, 1);
    CHECK(q.projectiles.size() == 1);
    play(q, Actions(), 60);
    bool heroShots = false;
    for (const Projectile& p : q.projectiles) heroShots |= p.fromHero;
    CHECK(!heroShots && q.enemies[0].hp == q.enemies[0].maxHp);
}

static void chargerTellsThenCharges() {
    TestArea t;
    makeArea(t, {{19, withLetter(30, 'D')}});   // world 2's D: the STAG, a charger
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 10, 0, 24, 2);
    q.godMode = true;
    CHECK(rosterType(2, 2).behaviour == BEH_CHARGER);
    play(q, Actions(), 1);
    float x0 = q.enemies[0].x;
    CHECK(q.enemies[0].state == ES_TELL);
    play(q, Actions(), 18);
    CHECK(q.enemies[0].state == ES_TELL && q.enemies[0].x == x0);
    play(q, Actions(), 18);
    CHECK(q.enemies[0].state == ES_ACT && q.enemies[0].x < x0);
}

static void armoredGuardsItsFront() {
    TestArea t;
    makeArea(t, {{19, withLetter(40, 'F')}});
    int damage[2];
    for (int back = 0; back < 2; back++) {
        Quest q;
        startQuest(q, t, CLASS_ARCHER, 1, 0, 22);   // 18 tiles = 144 dp away: outside its 120 px sight
        q.godMode = true;
        q.enemies[0].facingLeft = !back;
        Actions a = {};
        a.fire = true;
        play(q, a, 1);
        a.fire = false;
        for (int i = 0; i < 90 && q.enemies[0].hp == q.enemies[0].maxHp; i++) play(q, a, 1);
        damage[back] = q.enemies[0].maxHp - q.enemies[0].hp;
    }
    CHECK(damage[0] >= 1 && damage[0] <= 3);   // 9 ATK x0.25
    CHECK(damage[1] >= 8 && damage[1] <= 10);
}

static void dropperFallsWhenHeroBelow() {
    TestArea t;
    makeArea(t, {{5, row(WIDE_COLS, 25, 35, '#')}, {6, withLetter(30, 'E')}});   // world 2's E: the TREE MONKEY
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 10, 0, 10, 2);
    q.godMode = true;
    float hangY = q.enemies[0].y;
    CHECK(q.enemies[0].state == ES_HANG && hangY == 6 * TILE);
    play(q, Actions(), 60);
    CHECK(q.enemies[0].state == ES_HANG && q.enemies[0].y == hangY);
    q.hero.x = q.enemies[0].x;
    play(q, Actions(), 1);
    CHECK(q.enemies[0].state == ES_ACT);
    q.hero.x = 10 * TILE;   // step aside so the banana it throws on landing stays in the air
    bool banana = false;
    for (int i = 0; i < 120 && q.enemies[0].state == ES_ACT; i++) play(q, Actions(), 1);
    for (const Projectile& p : q.projectiles) banana |= !p.fromHero;
    CHECK(q.enemies[0].state == ES_PATROL && q.enemies[0].onGround && q.enemies[0].y > hangY);
    CHECK(banana);
}

static void walkersNeverWalkIntoPits() {
    TestArea t;
    std::string island = row(WIDE_COLS, 20, 30, '#');
    makeArea(t, {{19, withLetter(25, 'A')}, {20, island}, {21, island}, {22, island}});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 10, 0, 22);
    q.hero.x = 70 * TILE;   // far away, on no ground: keep it out of the way
    q.hero.y = 0;
    q.godMode = true;
    float y0 = q.enemies[0].y;
    bool stayed = true;
    for (int i = 0; i < 600; i++) {
        updateCombat(q, Actions(), DT);
        q.hero.x = 70 * TILE;
        q.hero.y = 0;
        stayed &= q.enemies[0].y == y0 && q.enemies[0].x >= 20 * TILE && q.enemies[0].x + q.enemies[0].w <= 31 * TILE;
    }
    CHECK(stayed);
}

static void iframesBlockDamage() {
    TestArea t;
    makeArea(t, {{19, withLetter(16, 'D')}});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 10, 0, 16);
    q.enemies[0].x = q.hero.x;
    int hp = q.hero.hp;
    q.hero.iframes = 0.5f;
    for (int i = 0; i < 20; i++) {
        q.enemies[0].x = q.hero.x;
        updateCombat(q, Actions(), DT);
    }
    CHECK(q.hero.hp == hp);
    for (int i = 0; i < 30; i++) {
        q.enemies[0].x = q.hero.x;
        updateCombat(q, Actions(), DT);
    }
    CHECK(q.hero.hp < hp && q.hero.iframes > 0.5f);
}

static void rollGivesIframes() {
    TestArea t;
    makeArea(t, {});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 10, 0, 10);
    float x0 = q.hero.x;
    Actions a = {};
    a.back = true;
    play(q, a, 1);
    CHECK(q.hero.rollTimer > 0 && q.hero.iframes > 0.2f && q.hero.iframes <= 0.25f);
    play(q, Actions(), 20);
    CHECK(q.hero.x - x0 >= 140 * DP * 0.3f);
}

static void blinkStopsBeforeWalls() {
    TestArea t;
    std::map<int, std::string> rows;
    for (int r = 0; r < 20; r++) rows[r] = withLetter(13, '#');
    makeArea(t, rows);
    Quest q;
    startQuest(q, t, CLASS_MAGICIAN, 10, 0, 10);
    float x0 = q.hero.x;
    Actions a = {};
    a.back = true;
    play(q, a, 1);
    CHECK(q.hero.x > x0 && q.hero.x + HERO_W <= 13 * TILE && q.hero.x + HERO_W >= 13 * TILE - DP);
    CHECK(q.hero.iframes > 0.15f && q.hero.iframes <= 0.2f);
}

static void strongAttackNeedsMp() {
    TestArea t;
    makeArea(t, {});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 10, 0, 10);
    q.hero.mp = 5;
    Actions a = {};
    a.rocket = true;
    play(q, a, 1);
    CHECK(q.hero.mp == 5 && q.hero.skill == -1 && q.soundRequested[SOUND_ROCKET_EMPTY]);
    q.hero.mp = 30;
    play(q, a, 1);
    CHECK(q.hero.mp == 20 && q.hero.skill == 0);
}

static bool logged(const Quest& q, const char* text) {
    for (const std::string& line : q.logLines)
        if (line.find(text) != std::string::npos) return true;
    return false;
}

static void deathDropsNamesAtLastSafeGround() {
    TestArea t;
    std::string floor = row(WIDE_COLS, 0, WIDE_COLS - 1, '#');
    for (int c = 30; c <= 33; c++) floor[c] = '.';
    makeArea(t, {{20, floor}, {21, floor}, {22, floor}});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 10, 0, 24);
    q.save.names = 500;
    Actions a = {};
    a.moveX = 1;
    for (int i = 0; i < 600 && q.mode == QMODE_PLAY; i++) play(q, a, 1);
    CHECK(q.mode == QMODE_DEAD && q.save.names == 0 && q.save.echoNames == 500);
    CHECK(q.save.echoWorld == WORLD_TEST && q.save.echoArea == 0);
    CHECK(q.save.echoX + 8 * DP <= 30 * TILE + HERO_W && q.save.echoX >= 26 * TILE);
    CHECK(logged(q, "(pit), dropped 500 names"));
}

static void secondDeathLosesTheOldEcho() {
    TestArea t;
    makeArea(t, {});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 10, 0, 24);
    q.save.echoNames = 500;
    q.save.echoX = 60 * TILE;
    q.save.names = 200;
    q.hero.hp = 0;
    play(q, Actions(), 1);
    CHECK(q.mode == QMODE_DEAD && q.save.echoNames == 200 && q.save.names == 0);
    CHECK(logged(q, "(hp), dropped 200 names"));
}

static void echoReturnsNames() {
    TestArea t;
    makeArea(t, {});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 10, 0, 24);
    q.save.names = 7;
    q.save.echoNames = 300;
    q.save.echoWorld = WORLD_TEST;
    q.save.echoArea = 0;
    q.save.echoX = q.hero.x;
    q.save.echoY = q.hero.y;
    play(q, Actions(), 1);
    CHECK(q.save.names == 307 && q.save.echoNames == 0 && logged(q, "quest: names recovered 300"));
    CHECK(q.soundRequested[SOUND_Q_NAMES] && !q.numbers.empty() && q.numbers.back().names);
    saveDelete(SAVE_FOLDER, 2);
}

static void deathRespawnsEnemies() {
    TestArea t;
    makeArea(t, {{19, withLetter(40, 'A')}});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 10, 0, 24);
    q.killed[WORLD_TEST][0] = 1;
    q.hero.hp = 0;
    play(q, Actions(), 1);
    CHECK(q.mode == QMODE_DEAD);
    play(q, Actions(), (int)(2.6f / DT));
    CHECK(q.mode == QMODE_PLAY && q.killed[WORLD_TEST][0] == 0 && q.hero.hp == q.stats.maxHp && q.hero.mp == q.stats.maxMp);
    saveDelete(SAVE_FOLDER, 2);
}

// --- RPG systems (phase 3): levels, shops, checkpoints, chests, gates, levers, jobs, items, NPCs. ---
// A dev start (NAMEGAME_QUEST's format) in slot 3 of the test folder.
static void startDev(Quest& q, const char* spec) {
    q = Quest();
    q.folder = SAVE_FOLDER;
    questStartDev(q, spec);
}

// Stands the hero on tile (tx, ty) and lets it settle.
static void standAt(Quest& q, int tx, int ty) {
    q.hero.x = (float)(tx * TILE);
    q.hero.y = (float)((ty + 1) * TILE) - HERO_H;
    q.hero.vx = q.hero.vy = 0;
    play(q, Actions(), 1);
}

static void pressUp(Quest& q) { Actions a = {}; a.up = true; play(q, a, 1); }
static void pressX(Quest& q) { Actions a = {}; a.jump = true; play(q, a, 1); }
static void pressDown(Quest& q) { Actions a = {}; a.down = true; play(q, a, 1); }

// X through every page of the open dialogue (completing each page first).
static void readDialogue(Quest& q) {
    for (int i = 0; i < 20 && q.mode == QMODE_DIALOGUE; i++) pressX(q);
}

static void levelUpSpendsAndRaisesStats() {
    Quest q;
    startDev(q, "slot=3 class=0 world=0 area=0 names=1000");
    q.save.ilsaTalks = 3;
    standAt(q, 45, 17);   // Ilsa, by the hearth
    int hpBefore = q.stats.maxHp;
    pressUp(q);
    CHECK(q.mode == QMODE_DIALOGUE && q.dialogue.speaker == "ILSA, KEEPER OF NAMES" && q.dialogue.after == AFTER_LEVEL_UP);
    readDialogue(q);
    CHECK(q.mode == QMODE_MENU && q.menu.kind == MENU_LEVEL_UP);
    pressX(q);
    CHECK(q.save.level == 2 && q.save.names == 900 && q.stats.maxHp > hpBefore && q.hero.hp == q.stats.maxHp);
    CHECK(logged(q, "quest: level 2 (spent 100, names left 900)") && q.soundRequested[SOUND_Q_LEVEL_UP]);
    CHECK(q.mode == QMODE_MENU);   // stays open for the next level
    pressX(q);
    CHECK(q.save.level == 3 && q.save.names == 780);
    SaveData saved;
    CHECK(saveRead(SAVE_FOLDER, 2, saved) && saved.level == 3);
    saveDelete(SAVE_FOLDER, 2);
}

static void cannotLevelPastForty() {
    Quest q;
    startDev(q, "slot=3 class=1 world=0 area=0 level=40 names=99999999");
    CHECK(!levelUp(q) && q.save.level == 40 && q.save.names == 99999999 && q.soundRequested[SOUND_ROCKET_EMPTY]);
    q.save.level = 39;
    CHECK(levelUp(q) && q.save.level == 40 && q.save.names == 99999999 - levelCost(39));
    saveDelete(SAVE_FOLDER, 2);
}

static void shopPricesScaleWithHighestWorld() {
    SaveData s;
    CHECK(itemPrice(s, ITEM_RED) == 15 && itemPrice(s, ITEM_WHETSTONE) == 60);
    s.highest = 3;
    CHECK(itemPrice(s, ITEM_RED) == 135 && shopRows(s, AFTER_POTION_SHOP)[ITEM_FEATHER].price == 360);
    s.highest = 7;
    CHECK(itemPrice(s, ITEM_BIG) == 45 * 360);
}

static void carryLimitsStopBuying() {
    Quest q;
    startDev(q, "slot=3 class=0 world=0 area=0 names=1000");
    q.shopKind = AFTER_POTION_SHOP;
    q.save.items[ITEM_RED] = 5;
    shopBuy(q, ITEM_RED);
    CHECK(q.save.items[ITEM_RED] == 5 && q.save.names == 1000 && q.soundRequested[SOUND_ROCKET_EMPTY]);
    CHECK(!shopRows(q.save, AFTER_POTION_SHOP)[ITEM_RED].canBuy);
    q.save.shards = 1;   // a Vial Shard carries one more
    shopBuy(q, ITEM_RED);
    CHECK(q.save.items[ITEM_RED] == 6 && q.save.names == 985 && logged(q, "quest: bought RED POTION for 15"));
    q.save.keys |= 1u << KEY_LOCKET;
    CHECK(carryLimit(q.save, ITEM_RED) == 7 && carryLimit(q.save, ITEM_BIG) == 3 && carryLimit(q.save, ITEM_MANA) == 4);
    CHECK(carryLimit(q.save, ITEM_TONIC) == 3);
    saveDelete(SAVE_FOLDER, 2);
}

static void gearTierNeedsTheWorld() {
    SaveData s;
    s.names = 1000000;
    std::vector<ShopRow> rows = shopRows(s, AFTER_GEAR_SHOP);
    CHECK(rows.size() == 2 && rows[0].goods == GOODS_WEAPON && rows[0].name == "IRON SWORD" && rows[0].price == 150);
    CHECK(rows[0].detail == "ATK +3" && rows[1].goods == GOODS_ARMOR && rows[1].price == 120);
    s.weapon = 1;
    rows = shopRows(s, AFTER_GEAR_SHOP);
    CHECK(rows.size() == 1 && rows[0].goods == GOODS_ARMOR);   // tier 2 waits for world 2
    s.highest = 2;
    rows = shopRows(s, AFTER_GEAR_SHOP);
    CHECK(rows.size() == 2 && rows[0].price == 1400);
    s.weapon = s.armor = 6;
    s.highest = 6;
    CHECK(shopRows(s, AFTER_GEAR_SHOP).empty());   // tier 7 is never sold
}

static void restHealsAndRespawnsEnemies() {
    Quest q;
    startDev(q, "slot=3 class=0 world=8 area=0");
    q.godMode = true;
    for (Enemy& e : q.enemies) e.alive = false;
    q.killed[WORLD_TEST][0] = ~0ull;
    q.hero.hp = 1;
    standAt(q, 3, 19);   // touching the brazier lights it
    CHECK(logged(q, "quest: checkpoint 8.0 lit") && (q.save.lit >> 16 & 1));
    pressUp(q);
    CHECK(q.mode == QMODE_MENU && q.menu.kind == MENU_CHECKPOINT && q.atCheckpoint == 16);
    pressX(q);   // REST
    CHECK(q.mode == QMODE_PLAY && q.hero.hp == q.stats.maxHp && q.killed[WORLD_TEST][0] == 0);
    int alive = 0;
    for (const Enemy& e : q.enemies) alive += e.alive;
    CHECK(alive == 3 && q.save.checkpointWorld == WORLD_TEST && q.save.checkpointId == 0 && logged(q, "quest: rest at 8.0"));
    saveDelete(SAVE_FOLDER, 2);
}

static void travelListsLitCheckpoints() {
    SaveData s;
    CHECK(travelCheckpoints(s) == std::vector<int>({0, 2}));   // world 1's entry is lit from the start (spec §11)
    s.lit |= 1u << 17 | 1u << 16;
    CHECK(travelCheckpoints(s) == std::vector<int>({0, 2, 16, 17}));
    Quest q;
    startDev(q, "slot=3 class=0 world=8 area=0");
    standAt(q, 3, 19);
    pressUp(q);
    pressDown(q);
    pressX(q);   // TRAVEL
    CHECK(q.mode == QMODE_MENU && q.menu.kind == MENU_TRAVEL && q.menu.items.size() == 3 && q.menu.items[0] == "NAMULA");
    CHECK(q.menu.items[1] == "THE DRIFTING STATION ENTRY" && q.menu.items[2] == "TEST GROUNDS ENTRY");
    pressX(q);
    CHECK(q.mode == QMODE_PLAY && q.area.world == WORLD_NAMULA && q.area.index == 0 && logged(q, "quest: travel to 0.0"));
    CHECK(q.save.checkpointWorld == 0 && q.save.checkpointId == 0);
    saveDelete(SAVE_FOLDER, 2);
}

static void chestOpensOnceAndSaves() {
    Quest q;
    startDev(q, "slot=3 class=0 world=8 area=3 items=0,0,0,0,0,0");
    q.godMode = true;
    standAt(q, 42, 14);
    pressUp(q);
    CHECK(q.save.items[ITEM_RED] == 2 && (q.save.chests[WORLD_TEST] & 1) && logged(q, "quest: chest 8.0 2 RED POTION"));
    CHECK(q.popup == "2 RED POTION" && q.soundRequested[SOUND_Q_CHEST]);
    SaveData saved;
    CHECK(saveRead(SAVE_FOLDER, 2, saved) && (saved.chests[WORLD_TEST] & 1) && saved.items[ITEM_RED] == 2);
    pressUp(q);
    CHECK(q.save.items[ITEM_RED] == 2);   // an open chest stays empty
    standAt(q, 25, 18);
    pressUp(q);
    CHECK(q.save.names == 120 && logged(q, "quest: chest 8.1 +120 NAMES"));
    saveDelete(SAVE_FOLDER, 2);
}

static void fullBagKeepsChestShut() {
    Quest q;
    startDev(q, "slot=3 class=0 world=8 area=3 items=5,0,0,0,0,0");
    q.godMode = true;
    standAt(q, 42, 14);
    pressUp(q);
    CHECK(q.save.items[ITEM_RED] == 5 && !(q.save.chests[WORLD_TEST] & 1) && q.popup == "YOUR BAG IS FULL");
    saveDelete(SAVE_FOLDER, 2);
}

static void gateWallOpens() {
    Quest q;
    startDev(q, "slot=3 class=0 world=8 area=2");
    CHECK(tileAt(q.area, 66, 10) == 'g' && tileSolid('g'));
    openGate(q, WORLD_TEST);
    CHECK(tileAt(q.area, 66, 10) == '.' && (q.save.gates >> WORLD_TEST & 1) && logged(q, "quest: gate 8 open"));
    CHECK(q.soundRequested[SOUND_Q_GATE] && q.gateTimer > 0);
    AreaState again;
    loadArea(again, WORLD_TEST, 2, q.save.gates, 0);
    CHECK(tileAt(again, 66, 10) == '.');
    Quest open;
    startDev(open, "slot=3 class=0 world=8 area=2 gate=1");
    CHECK(tileAt(open.area, 66, 18) == '.');
    saveDelete(SAVE_FOLDER, 2);
}

static void leverOpensDoorsForGood() {
    Quest q;
    startDev(q, "slot=3 class=0 world=8 area=3");
    q.godMode = true;
    for (Enemy& e : q.enemies) e.alive = false;
    CHECK(tileAt(q.area, 77, 10) == 'k');
    standAt(q, 74, 18);
    q.hero.facingLeft = false;
    Actions a = {};
    a.fire = true;
    play(q, a, 30);
    CHECK(tileAt(q.area, 77, 10) == '.' && (q.save.shortcuts[WORLD_TEST] & 1) && logged(q, "quest: shortcut 8.0 open"));
    AreaState again;
    loadArea(again, WORLD_TEST, 3, 0, q.save.shortcuts[WORLD_TEST]);
    CHECK(tileAt(again, 77, 18) == '.');
    saveDelete(SAVE_FOLDER, 2);
}

static void jobNeedsLevelAndGate() {
    Quest q;
    startDev(q, "slot=3 class=0 world=0 area=1 level=4");
    talkTo(q, NPC_ODO);
    CHECK(q.dialogue.pages[0] == "COME BACK AT LEVEL 5." && q.dialogue.after == AFTER_NONE);
    q.save.level = 5;
    talkTo(q, NPC_ODO);
    CHECK(q.dialogue.after == AFTER_NONE);   // world 1's gate still stands
    talkTo(q, NPC_WREN);
    CHECK(q.dialogue.after == AFTER_NONE && q.dialogue.pages.size() == 1);   // another class's mentor
    q.save.gates |= 1u << 1;
    talkTo(q, NPC_ODO);
    CHECK(q.dialogue.after == AFTER_JOB);
    readDialogue(q);
    CHECK(q.mode == QMODE_MENU && q.menu.kind == MENU_JOB_CONFIRM);
    pressX(q);
    CHECK(q.save.job == 1 && q.save.quests[QUEST_JOB] == 1 && logged(q, "quest: job 1 VANGUARD"));
    CHECK(q.hero.hp == q.stats.maxHp && q.banner && !strcmp(q.banner, "VANGUARD"));
    saveDelete(SAVE_FOLDER, 2);
}

static void secondJobNeedsTheFeather() {
    Quest q;
    startDev(q, "slot=3 class=2 world=0 area=1 level=15 job=1 quest0=1");
    talkTo(q, NPC_YSOLDE);
    CHECK(q.dialogue.after == AFTER_NONE);
    q.save.keys |= 1u << KEY_FROZEN_FEATHER;
    q.save.level = 14;
    talkTo(q, NPC_YSOLDE);
    CHECK(q.dialogue.pages[0] == "COME BACK AT LEVEL 15.");
    q.save.level = 15;
    talkTo(q, NPC_YSOLDE);
    CHECK(q.dialogue.after == AFTER_JOB);
    readDialogue(q);
    pressX(q);
    CHECK(q.save.job == 2 && !(q.save.keys >> KEY_FROZEN_FEATHER & 1) && logged(q, "quest: job 2 ELEMENTALIST"));
    talkTo(q, NPC_YSOLDE);
    CHECK(q.dialogue.pages[0] == "COME BACK AT LEVEL 30.");
    saveDelete(SAVE_FOLDER, 2);
}

static void buffsEndOnDeath() {
    TestArea t;
    makeArea(t, {});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 10, 0, 24);
    q.save.items[ITEM_RED] = 0;
    q.save.items[ITEM_WHETSTONE] = 1;
    Actions a = {};
    a.item = true;   // Triangle: the next owned item
    play(q, a, 1);
    CHECK(q.selectedItem == ITEM_WHETSTONE);
    a = Actions();
    a.cycle = true;
    play(q, a, 1);
    CHECK(q.hero.buffTimers[0] > 119 && q.save.items[ITEM_WHETSTONE] == 0);
    q.hero.hp = 0;
    play(q, Actions(), (int)(2.7f / DT));
    CHECK(q.mode == QMODE_PLAY && q.hero.buffTimers[0] == 0);
    saveDelete(SAVE_FOLDER, 2);
}

static void drinkHealsAfterHalfASecond() {
    TestArea t;
    makeArea(t, {});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 10, 0, 24);
    q.save.items[ITEM_RED] = 1;
    q.hero.hp = 10;
    Actions a = {};
    a.cycle = true;
    play(q, a, 1);
    CHECK(q.save.items[ITEM_RED] == 0 && q.soundRequested[SOUND_Q_DRINK]);
    play(q, Actions(), (int)(0.3f / DT));
    CHECK(q.hero.hp == 10);
    play(q, Actions(), (int)(0.3f / DT));
    CHECK(q.hero.hp == 10 + (int)(q.stats.maxHp * 0.35f));
    a.cycle = true;
    play(q, a, 1);
    CHECK(q.soundRequested[SOUND_ROCKET_EMPTY]);   // nothing left
}

static void npcPresenceFollowsQuestStages() {
    SaveData s;
    CHECK(npcPresent(s, NPC_ILSA, 0, 0) && !npcPresent(s, NPC_STRANGER, 0, 2) && !npcPresent(s, NPC_CLAUDE, 0, 2));
    CHECK(!npcPresent(s, NPC_GARRICK, 0, 2) && npcPresent(s, NPC_GARRICK, 2, 5));
    s.quests[QUEST_TOBIN] = 2;
    CHECK(npcPresent(s, NPC_GARRICK, 0, 2) && !npcPresent(s, NPC_GARRICK, 2, 5));
    s.bosses |= 1u << 3;
    s.keys |= 1u << KEY_CREDITS_SEEN;
    CHECK(npcPresent(s, NPC_STRANGER, 0, 2) && npcPresent(s, NPC_CLAUDE, 0, 2));
}

// Every line anyone says fits the dialogue box: at most 4 lines of 72 characters (spec §14).
static void dialogueFitsTheBox() {
    Quest q;
    for (int talk = 0; talk < 5; talk++)
        for (int id = 0; id < NPC_COUNT; id++)
            for (int highest = 1; highest <= 7; highest += 6) {
                q.save = SaveData();
                q.save.highest = highest;
                q.save.ilsaTalks = talk;
                q.save.level = talk * 5;
                q.save.gates = talk >= 2 ? 2 : 0;
                q.save.quests[QUEST_JOB] = talk >= 3 ? talk - 2 : 0;
                talkTo(q, id);
                CHECK(!q.dialogue.pages.empty());
                for (const std::string& page : q.dialogue.pages) {
                    int lines = 1, length = 0;
                    for (char c : page) {
                        length = c == '\n' ? 0 : length + 1;
                        lines += c == '\n';
                        CHECK(length <= 72);
                    }
                    CHECK(lines <= 4);
                }
            }
}

// --- Bosses (spec §12). TEST GROUNDS area 4 holds the Overseer (mini-boss), area 5 the Star Mage (big boss). ---
static int bossShots(const Quest& q) {
    int n = 0;
    for (const Projectile& p : q.projectiles) n += !p.fromHero;
    return n;
}

// Every shot, slam drop and summon comes at least 0.4 s after its attack began (0.8 s and a marker for the slam).
static void overseerTellsBeforeEveryAttack() {
    Quest q;
    startDev(q, "slot=3 class=0 world=8 area=4 level=4");
    q.godMode = true;
    CHECK(q.bosses.size() == 1 && q.bosses[0].id == QBOSS_OVERSEER && q.arenaLocked);
    if (q.bosses.empty()) return;
    bool seen[4] = {};
    float began = 0;
    int last = -1, shots = 0, enemies = (int)q.enemies.size(), slamStep = 0;
    for (int i = 0; i < 60 * 60; i++) {
        if (i == 30 * 60) q.bosses[0].hp = q.bosses[0].maxHp / 2;   // phase 2 for the second half
        play(q, Actions(), 1);
        const QuestBoss& b = q.bosses[0];
        if (b.attack >= 0 && last < 0) began = q.time, seen[b.attack] = true;
        float since = q.time - began;
        if (bossShots(q) > shots) CHECK(b.attack >= 0 && since >= 0.4f);
        if ((int)q.enemies.size() > enemies) CHECK(b.attack == 3 && since >= 0.4f);
        if (b.attack == 2 && b.step == 1 && slamStep == 0) CHECK(since >= 0.8f);
        if (b.attack == 2 && b.step == 0) CHECK(!q.markers.empty() && q.markers[0].color == 'm');
        last = b.attack;
        slamStep = b.attack == 2 ? b.step : 0;
        shots = bossShots(q);
        enemies = (int)q.enemies.size();
    }
    CHECK(seen[0] && seen[1] && seen[2] && seen[3]);
    CHECK(q.bosses[0].phase == 2 && logged(q, "quest: boss THE OVERSEER phase 2"));
}

// The beam's column is marked for the ball's whole 1.2 s, and the beam burns exactly there; in phase 2 two columns 64 apart.
static void starMageBeamMarksItsColumn() {
    Quest q;
    startDev(q, "slot=3 class=0 world=8 area=5 level=4");
    q.godMode = true;
    CHECK(q.bosses.size() == 1 && q.bosses[0].id == QBOSS_STAR_MAGE);
    if (q.bosses.empty()) return;
    for (int round = 0; round < 2; round++) {
        if (round == 1) {
            q.bosses[0].hp = q.bosses[0].maxHp / 2;
            play(q, Actions(), 2);
            CHECK(q.bosses[0].phase == 2);
        }
        int columns = round + 1, beams = 0;
        for (int i = 0; i < 60 * 60 && beams == 0; i++) {
            play(q, Actions(), 1);
            const QuestBoss& b = q.bosses[0];
            if (b.attack != 0 || b.step < 2) continue;
            if (b.step == 2) {
                CHECK((int)q.markers.size() == columns);
                for (int k = 0; k < (int)q.markers.size() && k < 2; k++)
                    CHECK(fabsf(q.markers[k].x + q.markers[k].w / 2 - b.data[3 + k]) < 0.5f && q.markers[k].w == 24 * DP);
                if (columns == 2) CHECK(fabsf(fabsf(b.data[4] - b.data[3]) - 64 * DP) < 0.5f);
            }
            if (b.step == 3)   // the first beam frame: its effects stand on the marked columns
                for (const Effect& f : q.effects) {
                    if (f.kind != FX_BEAM) continue;
                    beams++;
                    CHECK(fabsf(f.x + f.w / 2 - b.data[3]) < 0.5f || fabsf(f.x + f.w / 2 - b.data[4]) < 0.5f);
                }
        }
        CHECK(beams == columns);
    }
}

// A world 1 mini-boss arena with a right edge into world 1's area 0: closed while the boss lives, open after its death.
static void bossFightLocksAndUnlocksExits() {
    TestArea t;
    makeArea(t, {{19, withLetter(40, 'Q')}});
    t.def.right = {0, 2, 18};
    t.def.flags = AREA_MINIBOSS;
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 4, 0, 70, 1);
    q.godMode = true;
    startArenaFight(q);
    CHECK(q.arenaLocked && q.bosses.size() == 1 && logged(q, "quest: boss THE OVERSEER start"));
    Actions right = {};
    right.moveX = 1;
    play(q, right, 120);
    CHECK(q.area.world == 1 && q.area.def == &t.def && q.hero.x <= WIDE_COLS * TILE - HERO_W);
    q.bosses[0].hp = 0;
    play(q, Actions(), 1);
    CHECK(!q.arenaLocked && q.save.gates >> 1 & 1 && logged(q, "quest: boss THE OVERSEER down after"));
    play(q, right, 60);
    CHECK(q.area.def != &t.def && q.area.world == 1 && q.area.index == 0);
    // The hangar's ladder down to the bulkhead is closed too: climbing down it holds the hero at the bottom.
    startDev(q, "slot=3 class=0 world=1 area=9 level=4");
    CHECK(q.arenaLocked);
    standAt(q, 76, 18);
    Actions down = {};
    down.moveY = 1;
    play(q, down, 180);
    CHECK(q.mode == QMODE_PLAY && q.area.world == 1 && q.area.index == 9 && q.arenaLocked);
    // The hero's death also ends the fight (the boss is back at full HP next time).
    startQuest(q, t, CLASS_KNIGHT, 4, 0, 70, 1);
    startArenaFight(q);
    CHECK(q.arenaLocked);
    q.hero.hp = 0;
    play(q, Actions(), 1);
    CHECK(q.mode == QMODE_DEAD && !q.arenaLocked && q.bosses.empty());
    saveDelete(SAVE_FOLDER, 2);
}

// Leaving and coming back: the mini-boss stays down until a rest or death, the big boss for good.
static void bigBossStaysDeadMiniBossReturns() {
    Quest q;
    startDev(q, "slot=3 class=0 world=8 area=4 level=4");
    q.bosses[0].hp = 0;
    play(q, Actions(), 30);   // past the death's hit-stop
    standAt(q, 75, 18);
    pressUp(q);   // the portal to the dome
    CHECK(q.area.index == 5 && q.bosses.size() == 1 && q.bosses[0].id == QBOSS_STAR_MAGE);
    q.bosses[0].hp = 0;
    play(q, Actions(), 30);
    CHECK(q.save.bosses >> WORLD_TEST & 1);
    standAt(q, 4, 18);
    pressUp(q);   // back to the arena: the Overseer is still down
    CHECK(q.area.index == 4 && q.bosses.empty() && !q.arenaLocked);
    standAt(q, 75, 18);
    pressUp(q);   // and the dome stays empty
    CHECK(q.area.index == 5 && q.bosses.empty());
    q.hero.hp = 0;   // a death clears the killed lists: the mini-boss is due again, the big boss is not
    play(q, Actions(), 1);
    play(q, Actions(), 160);
    CHECK(q.mode == QMODE_PLAY);
    loadArea(q.area, WORLD_TEST, 4, q.save.gates, 0);
    spawnEnemies(q);
    startArenaFight(q);
    CHECK(q.bosses.size() == 1 && q.bosses[0].hp == q.bosses[0].maxHp);
    q.bosses.clear();
    loadArea(q.area, WORLD_TEST, 5, q.save.gates, 0);
    spawnEnemies(q);
    startArenaFight(q);
    CHECK(q.bosses.empty());
    saveDelete(SAVE_FOLDER, 2);
}

static void bossDeathLightsTheNextWorld() {
    TestArea t;
    makeArea(t, {{19, withLetter(40, 'W')}});
    t.def.flags = AREA_BOSS;
    Quest q;
    startQuest(q, t, CLASS_ARCHER, 4, 0, 10, 1);
    startArenaFight(q);
    CHECK(q.bosses.size() == 1 && q.bosses[0].id == QBOSS_STAR_MAGE && q.bosses[0].big);
    if (q.bosses.empty()) return;
    CHECK(q.bosses[0].maxHp == enemyHealth(1, 200, 0));
    int names = q.save.names;
    q.bosses[0].hp = 0;
    play(q, Actions(), 1);
    CHECK(q.save.bosses >> 1 & 1);
    CHECK(q.save.lit >> 4 & 1);   // world 2's entry
    CHECK(q.save.names == names + enemyNames(1, 150, 0, false));
    CHECK(q.banner && !strcmp(q.banner, "GREAT NAME TAKEN") && logged(q, "quest: boss THE STAR MAGE down after"));
    startArenaFight(q);
    CHECK(q.bosses.size() == 1 && !q.bosses[0].alive);   // the old one, dead: no new fight
    saveDelete(SAVE_FOLDER, 2);
}

// make quest-maps (phase 4 step 2): every screen of every area, enemies and bosses at their spots, the hero hidden.
// --- World 2 (phase 5) ---
static int countProjectiles(const Quest& q, int kind) {
    int n = 0;
    for (const Projectile& p : q.projectiles) n += p.kind == kind;
    return n;
}

// A palm R drops its coconut 0.3 s after the hero's centre comes within 8 px below it, for 20 % of max HP; it regrows in 4 s.
static void coconutDropsWhenHeroPassesUnder() {
    TestArea t;
    makeArea(t, {{12, withLetter(30, 'R')}});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 11, 1, 20, 2);
    CHECK(q.traps.size() == 1);
    Actions a = {};
    a.moveX = 1;
    float under = -1, dropped = -1;
    for (int i = 0; i < 240 && dropped < 0; i++) {
        play(q, a, 1);
        if (under < 0 && fabsf(q.hero.x + HERO_W / 2 - (30 * TILE + TILE / 2)) <= 8 * DP) under = q.time;
        if (countProjectiles(q, PROJ_COCONUT) > 0) dropped = q.time;
    }
    CHECK(under >= 0 && dropped >= 0 && dropped - under >= 0.3f - 0.001f && dropped - under <= 0.3f + 2 * DT);
    if (dropped < 0) return;
    for (const Projectile& p : q.projectiles)
        if (p.kind == PROJ_COCONUT) CHECK(p.trap && p.damage == (int)lroundf(q.stats.maxHp * 0.2f) && p.vy == 200 * DP);
    play(q, Actions(), (int)(3.9f / DT));
    CHECK(q.traps[0].state == 2);
    play(q, Actions(), (int)(0.2f / DT));
    CHECK(q.traps[0].state == 0);
}

// A tree monkey hanging from the ceiling drops on the hero below and throws one banana as it lands.
static void treeMonkeyThrowsOnLanding() {
    TestArea t;
    makeArea(t, {{4, std::string(WIDE_COLS, '#')}, {19, withLetter(30, 'E')}});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 11, 1, 31, 2);   // a tile aside, so the banana flies a little before it hits
    q.godMode = true;
    CHECK(q.enemies.size() == 1 && q.enemies[0].state == ES_HANG);
    int bananas = 0;
    for (int i = 0; i < 180; i++) {
        play(q, Actions(), 1);
        bananas = std::max(bananas, countProjectiles(q, PROJ_BANANA));
    }
    CHECK(q.enemies[0].onGround && bananas == 1);
}

// A peel slides the hero 0.5 s the way it faced, at 100 px/s or more, whatever the stick says.
static void peelSlidesTheHero() {
    TestArea t;
    makeArea(t, {});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 11, 1, 20, 2);
    q.hazards.push_back({HAZARD_PEEL, 22.0f * TILE, 20.0f * TILE, 6});
    Actions a = {};
    a.moveX = 1;
    for (int i = 0; i < 120 && q.hero.slideTimer <= 0; i++) play(q, a, 1);
    CHECK(q.hero.slideTimer > 0);
    float x0 = q.hero.x;
    a.moveX = -1;   // pushing back changes nothing while sliding
    play(q, a, (int)(0.4f / DT));
    CHECK(q.hero.x - x0 >= 100 * DP * 0.4f - DP);
    play(q, a, (int)(0.3f / DT));
    CHECK(q.hero.slideTimer <= 0);
}

// Inside the stink mist the hero loses 3 % of max HP per second (no i-frames, no knockback).
static void mistHurtsOverTime() {
    TestArea t;
    makeArea(t, {});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 11, 1, 20, 2);
    q.hazards.push_back({HAZARD_MIST, q.hero.x + HERO_W / 2, q.hero.y + HERO_H / 2, 6});
    int hp = q.hero.hp;
    play(q, Actions(), 60);
    float expected = q.stats.maxHp * 0.03f;
    CHECK(fabsf(hp - q.hero.hp - expected) <= 1.5f && q.hero.iframes == 0);
}

// The Silverback's POUND wave (12 px tall) is cleared by a jump at the right moment and hurts a hero who stays down.
static void silverbackWaveCanBeJumped() {
    for (int jump = 0; jump < 2; jump++) {
        TestArea t;
        makeArea(t, {});
        Quest q;
        startQuest(q, t, CLASS_KNIGHT, 11, 1, 20, 2);
        startBoss(q, QBOSS_SILVERBACK, 60 * TILE, 20.0f * TILE);
        QuestBoss& b = q.bosses[0];
        b.attack = 0;   // POUND
        b.step = 0;
        b.timer = 0;
        int hp = q.hero.hp;
        bool jumped = false, passed = false;
        for (int i = 0; i < 6 * 60 && !passed; i++) {
            Actions a = {};
            a.jumpHeld = true;
            for (const Projectile& p : q.projectiles)
                if (p.kind == PROJ_WAVE && p.vx < 0 && jump && !jumped && p.x - (q.hero.x + HERO_W) < 22 * DP) a.jump = jumped = true;
            play(q, a, 1);
            if (q.bosses[0].attack != 0) q.bosses[0].gap = 99;   // only the one pound
            passed = q.hero.hp < hp;
            bool waveLeft = false;
            for (const Projectile& p : q.projectiles) waveLeft |= p.kind == PROJ_WAVE && p.vx < 0 && p.x > q.hero.x;
            if (i > 60 && !waveLeft) break;
        }
        CHECK(jump ? q.hero.hp == hp && jumped : q.hero.hp < hp);
    }
}

// Both world 2 bosses use every attack of both phases within a minute each, and the Monkey King only lands on its trees.
static void worldTwoBossesUseEveryAttack() {
    for (int id : {QBOSS_MONKEY_KING, QBOSS_SILVERBACK}) {
        std::string anchors = withLetter(13, 'X'), trees = row(WIDE_COLS, 10, 16, '-');
        anchors[63] = 'X';
        trees.replace(60, 7, "-------");
        TestArea t;
        makeArea(t, {{11, anchors}, {12, trees}});
        Quest q;
        startQuest(q, t, CLASS_ARCHER, 11, 1, 38, 2);
        q.godMode = true;
        startBoss(q, id, id == QBOSS_MONKEY_KING ? 13 * TILE + TILE / 2.0f : 40.0f * TILE, id == QBOSS_MONKEY_KING ? 12.0f * TILE : 20.0f * TILE);
        bool seen[2][4] = {};
        for (int i = 0; i < 120 * 60; i++) {
            if (i == 60 * 60) q.bosses[0].hp = q.bosses[0].maxHp / 2;
            play(q, Actions(), 1);
            const QuestBoss& b = q.bosses[0];
            if (b.attack >= 0 && b.attack < 4) seen[b.phase - 1][b.attack] = true;
            if (id == QBOSS_MONKEY_KING && b.attack < 0 && b.shift <= 0 && i > 0)   // between attacks it sits on a tree
                CHECK(fabsf(b.y + b.h - 12 * TILE) < 1);
        }
        CHECK(seen[0][0] && seen[0][1] && seen[0][2] && seen[1][0] && seen[1][1] && seen[1][2] && seen[1][3] && !seen[0][3]);
        CHECK(q.bosses[0].phase == 2);
    }
}

// --- World 3 (spec §9, §12): icicles, crumbling ice, wind, the frozen status, the Yeti and the Titan. ---
// An icicle falls 0.25 s after the hero comes within 12 px below it (20 % trap, 280 px/s); it stays gone until a rest.
static void icicleFallsOnceUntilRest() {
    TestArea t;
    makeArea(t, {{4, std::string(WIDE_COLS, '#')}, {5, withLetter(30, 'Y')}});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 17, 1, 20, 3);
    CHECK(q.traps.size() == 1 && q.traps[0].kind == 'Y');
    Actions a = {};
    a.moveX = 1;
    float near = -1, dropped = -1;
    for (int i = 0; i < 240 && dropped < 0; i++) {
        play(q, a, 1);
        if (near < 0 && fabsf(q.hero.x + HERO_W / 2 - (30 * TILE + TILE / 2)) <= 12 * DP) near = q.time;
        if (countProjectiles(q, PROJ_ICICLE) > 0) dropped = q.time;
    }
    CHECK(near >= 0 && dropped >= 0 && dropped - near >= 0.25f - 0.001f && dropped - near <= 0.25f + 2 * DT);
    for (const Projectile& p : q.projectiles)
        if (p.kind == PROJ_ICICLE) CHECK(p.trap && p.damage == (int)lroundf(q.stats.maxHp * 0.2f) && p.vy == 280 * DP);
    spawnEnemies(q);   // coming back without a rest: still gone
    CHECK(q.traps[0].state == 2);
    for (auto& killed : q.killed) killed.assign(killed.size(), 0);   // a rest
    spawnEnemies(q);
    CHECK(q.traps[0].state == 0);
}

// A crumbling block shakes 0.6 s under the hero, breaks, and returns 4 s later once the hero is off its tile.
static void crumblingIceBreaksAndReturns() {
    TestArea t;
    std::string floor(WIDE_COLS, '#');
    floor[30] = 'c';
    makeArea(t, {{20, floor}});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 17, 1, 30, 3);
    play(q, Actions(), (int)(0.55f / DT));
    CHECK(tileAt(q.area, 30, 20) == 'c');
    play(q, Actions(), (int)(0.1f / DT));
    CHECK(tileAt(q.area, 30, 20) == '.');
    Hero moved = standing(t, 10);
    q.hero.x = moved.x;
    q.hero.y = moved.y;
    play(q, Actions(), (int)(4.1f / DT));
    CHECK(tileAt(q.area, 30, 20) == 'c');
}

// World 3's wind zones push 40 px/s and turn around every 3 s.
static void windFlipsEveryThreeSeconds() {
    TestArea t;
    makeArea(t, {{19, row(WIDE_COLS, 2, WIDE_COLS - 3, '>')}});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 17, 1, 40, 3);
    q.godMode = true;
    float x0 = q.hero.x;
    play(q, Actions(), (int)(2.0f / DT));
    CHECK(fabsf(q.hero.x - x0 - 40 * DP * 2) < 3 * DP);
    play(q, Actions(), (int)(1.5f / DT));   // past 3 s
    float x1 = q.hero.x;
    play(q, Actions(), (int)(2.0f / DT));
    CHECK(fabsf(q.hero.x - x1 + 40 * DP * 2) < 3 * DP);
}

// The Yeti's ice cube freezes the hero 1.2 s with no control; each X or R1 press cuts 0.15 s.
static void frozenHeroBreaksFreeSoonerByMashing() {
    float freed[2];
    for (int mash = 0; mash < 2; mash++) {
        TestArea t;
        makeArea(t, {});
        Quest q;
        startQuest(q, t, CLASS_KNIGHT, 17, 1, 20, 3);
        Projectile cube = {};
        cube.x = q.hero.x;
        cube.y = q.hero.y;
        cube.w = cube.h = (int)(8 * DP);
        cube.range = 100;
        cube.kind = PROJ_ICE_CUBE;
        q.projectiles.push_back(cube);
        play(q, Actions(), 1);
        CHECK(q.hero.frozenTimer > 1.1f && q.hero.hp == q.stats.maxHp);
        float x0 = q.hero.x, start = q.time;
        Actions a = {};
        a.moveX = 1;
        for (int i = 0; i < 120 && q.hero.frozenTimer > 0; i++) {
            a.jump = mash && i % 6 == 0;
            play(q, a, 1);
            if (q.hero.frozenTimer > 0) CHECK(q.hero.x == x0);
        }
        freed[mash] = q.time - start;
    }
    CHECK(fabsf(freed[0] - 1.2f) < 3 * DT && freed[1] < 0.7f);
}

// Phase 2 of the Glacier Titan turns the middle 10 floor tiles to crumbling ice and starts the flipping blizzard.
static void titanPhaseTwoCracksTheFloor() {
    TestArea t;
    makeArea(t, {});
    Quest q;
    startQuest(q, t, CLASS_ARCHER, 17, 2, 10, 3);
    q.godMode = true;
    startBoss(q, QBOSS_TITAN, 60.0f * TILE, 20.0f * TILE);
    q.bosses[0].hp = q.bosses[0].maxHp / 2;
    play(q, Actions(), 2);
    int cracked = 0;
    for (int col = 0; col < WIDE_COLS; col++) cracked += tileAt(q.area, col, 20) == 'c';
    CHECK(cracked == 10 && tileAt(q.area, 35, 20) == 'c' && tileAt(q.area, 44, 20) == 'c' && tileAt(q.area, 45, 20) == '#');
    CHECK(q.traps.size() == 10);
    play(q, Actions(), (int)(1.0f / DT));   // past the 0.8 s phase-change pose
    float wind = q.area.wind;
    CHECK(fabsf(wind) == 40 * DP);
    play(q, Actions(), (int)(4.1f / DT));
    CHECK(q.area.wind == -wind);
}

// Both world 3 bosses use every attack of each phase.
static void worldThreeBossesUseEveryAttack() {
    for (int id : {QBOSS_YETI, QBOSS_TITAN}) {
        TestArea t;
        makeArea(t, {});
        Quest q;
        startQuest(q, t, CLASS_ARCHER, 17, 2, 10, 3);
        q.godMode = true;
        startBoss(q, id, 40.0f * TILE, 20.0f * TILE);
        bool seen[2][3] = {};
        for (int i = 0; i < 120 * 60; i++) {
            if (i == 60 * 60) q.bosses[0].hp = q.bosses[0].maxHp / 2;
            play(q, Actions(), 1);
            q.hero.frozenTimer = 0;
            const QuestBoss& b = q.bosses[0];
            if (b.attack >= 0 && b.attack < 3) seen[b.phase - 1][b.attack] = true;
        }
        for (int p = 0; p < 2; p++)
            for (int k = 0; k < 3; k++) CHECK(seen[p][k]);
        CHECK(q.bosses[0].phase == 2 && q.bosses[0].alive);
    }
}

// Phase 5 story (spec §14): Garrick asks for a RED POTION and thanks the hero who has one; Ilsa and Lio speak of the
// latest Great Name of worlds 2 and 3.
static void worldTwoAndThreeStoryLines() {
    TestArea t;
    makeArea(t, {});
    Quest q;
    startQuest(q, t, CLASS_KNIGHT, 11, 1, 20, 2);
    q.save.items[ITEM_RED] = 0;
    talkTo(q, NPC_GARRICK);
    CHECK(q.dialogue.pages.size() == 1 && q.dialogue.pages[0] == "I... I CAN'T WALK. DO YOU HAVE A RED POTION?");
    q.save.items[ITEM_RED] = 1;
    talkTo(q, NPC_GARRICK);
    CHECK(q.dialogue.pages.size() == 2 && q.dialogue.pages[0].find("RED POTION") != std::string::npos);
    q.save.ilsaTalks = 3;
    for (int w = 2; w <= 3; w++) {
        q.save.bosses |= 1u << w;
        talkTo(q, NPC_ILSA);
        CHECK(q.dialogue.pages.size() == 2 && q.dialogue.pages[0].find(w == 2 ? "LEAVES" : "COLD") != std::string::npos);
        talkTo(q, NPC_LIO);
        CHECK(q.dialogue.pages.size() == 2 && q.dialogue.pages[1].find(w == 2 ? "SILVERBACK" : "TITAN") != std::string::npos);
    }
}

static void renderMaps() {
    screenSetTarget(questPixels, QUEST_W, QUEST_H);
    int written = 0;
    for (int w = 0; w < WORLD_COUNT; w++)
        for (int i = 0; i < worldDef(w).count; i++) {
            Quest q = Quest();
            q.folder = SAVE_FOLDER;
            q.save.used = true;
            loadArea(q.area, w, i, 0, 0);
            questRefreshStats(q, true);
            q.hero.x = q.hero.y = -1000;
            spawnEnemies(q);
            startArenaFight(q);
            q.bannerTimer = 0;
            q.mode = QMODE_PLAY;
            const AreaDef& def = *q.area.def;
            for (int k = 0; k < 2; k++) {
                q.camX = def.cols == WIDE_COLS ? (float)(k * (def.cols * TILE - QUEST_W)) : 0;
                q.camY = def.cols == WIDE_COLS ? (float)(def.rows * TILE - QUEST_H) : (float)(k * (def.rows * TILE - QUEST_H));
                questDraw(q);
                char path[64];
                snprintf(path, sizeof path, "build/maps/w%d_a%02d_%d.bmp", w, i, k);
                written += screenSaveBmp(path);
            }
        }
    printf("quest maps: %d screens in build/maps\n", written);
}

int main(int argc, char** argv) {
    if (argc > 1 && !strcmp(argv[1], "--maps")) {
        renderMaps();
        return 0;
    }
    questSpritesHaveTheirSize();
    palSpritesUseOnlyPaletteChars();
    everyGroundShapeHasArt();
    propsNeverBlockSpots();
    propPlacementIsStable();
    layersObeyTheValueRules();
    outlineSurroundsTheSprite();
    areaLandmarksExist();
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
    levelCostMatchesSpec();
    referenceHitIsTwentyPercent();
    classesDifferAsDesigned();
    basicAttackKillsFodderInThreeHits();
    thiefStabsUp();
    arrowsHitEveryGroundEnemy();
    knightSwingHitsOnlyInItsWindow();
    skillCastMidSwingStillHits();
    spikesHurtAndBounce();
    arrowsStopAtWalls();
    chargerTellsThenCharges();
    armoredGuardsItsFront();
    dropperFallsWhenHeroBelow();
    walkersNeverWalkIntoPits();
    iframesBlockDamage();
    rollGivesIframes();
    blinkStopsBeforeWalls();
    strongAttackNeedsMp();
    deathDropsNamesAtLastSafeGround();
    secondDeathLosesTheOldEcho();
    echoReturnsNames();
    deathRespawnsEnemies();
    levelUpSpendsAndRaisesStats();
    cannotLevelPastForty();
    shopPricesScaleWithHighestWorld();
    carryLimitsStopBuying();
    gearTierNeedsTheWorld();
    restHealsAndRespawnsEnemies();
    travelListsLitCheckpoints();
    chestOpensOnceAndSaves();
    fullBagKeepsChestShut();
    gateWallOpens();
    leverOpensDoorsForGood();
    jobNeedsLevelAndGate();
    secondJobNeedsTheFeather();
    buffsEndOnDeath();
    drinkHealsAfterHalfASecond();
    npcPresenceFollowsQuestStages();
    dialogueFitsTheBox();
    overseerTellsBeforeEveryAttack();
    starMageBeamMarksItsColumn();
    bossFightLocksAndUnlocksExits();
    bigBossStaysDeadMiniBossReturns();
    bossDeathLightsTheNextWorld();
    coconutDropsWhenHeroPassesUnder();
    treeMonkeyThrowsOnLanding();
    peelSlidesTheHero();
    mistHurtsOverTime();
    silverbackWaveCanBeJumped();
    worldTwoBossesUseEveryAttack();
    icicleFallsOnceUntilRest();
    crumblingIceBreaksAndReturns();
    windFlipsEveryThreeSeconds();
    frozenHeroBreaksFreeSoonerByMashing();
    titanPhaseTwoCracksTheFloor();
    worldThreeBossesUseEveryAttack();
    worldTwoAndThreeStoryLines();
    everythingIsReachable();
    for (int world : {1, 2, 3}) {
        gateGuardsTheSecondHalf(world);
        worldLayoutRulesHold(world);
    }
    chestsMatchTheWorldList(1, 3);   // VIAL SHARD, MAP PAGE 1, STAR IRON
    chestsMatchTheWorldList(2, 2);   // VIAL SHARD, MAP PAGE 2
    chestsMatchTheWorldList(3, 3);   // VIAL SHARD, MAP PAGE 3, FROZEN FEATHER
    printf(failures ? "%d quest check(s) failed\n" : "all quest tests passed\n", failures);
    return failures ? 1 : 0;
}
