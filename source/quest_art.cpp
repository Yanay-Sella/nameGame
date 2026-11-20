// NAME QUEST's art system (spec §22) and Namula's world art (spec §13 row 0): layers, props, lights, vignette.
#include "quest_art.h"
#include "quest_sprites_0.h"
#include <algorithm>
#include <math.h>
#include <string.h>

// --- Layers: pre-rendered once per world, drawn as whole rows (L0) and opaque runs (L1-L3) ---
struct Run { uint16_t x, len; };
static std::vector<uint32_t> layers[4];
static std::vector<Run> runs[4];
static std::vector<int> rowRuns[4];   // runs of row y: runs[l][rowRuns[l][y] .. rowRuns[l][y + 1])
static int paintedWorld = -1;
const float LAYER_FACTOR_X[4] = {0, 0.15f, 0.35f, 0.6f}, LAYER_FACTOR_Y[4] = {0.05f, 0.1f, 0.2f, 0.35f};

void prepareWorldArt(int world) {
    const WorldArt* art = worldArt(world);
    if (!art || world == paintedWorld) return;
    paintedWorld = world;
    uint32_t* pixels[4];
    for (int l = 0; l < 4; l++) {
        layers[l].assign(LAYER_W * LAYER_H, 0);
        pixels[l] = layers[l].data();
    }
    art->paintLayers(pixels, LAYER_W, LAYER_H);
    for (int l = 1; l < 4; l++) {
        runs[l].clear();
        rowRuns[l].assign(LAYER_H + 1, 0);
        for (int y = 0; y < LAYER_H; y++) {
            rowRuns[l][y] = (int)runs[l].size();
            const uint32_t* row = &layers[l][y * LAYER_W];
            for (int x = 0; x < LAYER_W;) {
                if (!row[x]) { x++; continue; }
                int start = x;
                while (x < LAYER_W && row[x]) x++;
                runs[l].push_back({(uint16_t)start, (uint16_t)(x - start)});
            }
        }
        rowRuns[l][LAYER_H] = (int)runs[l].size();
    }
}

const uint32_t* layerPixels(int layer) {
    return layers[layer].empty() ? nullptr : layers[layer].data();
}

void drawLayers(int world, int camX, int camY) {
    prepareWorldArt(world);
    int offY0 = std::min((int)(camY * LAYER_FACTOR_Y[0]), LAYER_H - screenH);
    for (int y = 0; y < screenH; y++) memcpy(screenTarget + y * screenW, &layers[0][(y + offY0) * LAYER_W], screenW * sizeof(uint32_t));
    for (int l = 1; l < 4; l++) {
        int offX = (int)(camX * LAYER_FACTOR_X[l]) % LAYER_W, offY = (int)(camY * LAYER_FACTOR_Y[l]);
        for (int y = 0; y < screenH && y + offY < LAYER_H; y++) {
            const uint32_t* src = &layers[l][(y + offY) * LAYER_W];
            uint32_t* dst = screenTarget + y * screenW;
            for (int i = rowRuns[l][y + offY]; i < rowRuns[l][y + offY + 1]; i++) {
                const Run& run = runs[l][i];
                // The layer repeats every LAYER_W: draw the copies of this run that reach the screen.
                for (int sx = run.x - offX - (run.x - offX + run.len <= 0 ? -LAYER_W : 0); sx < screenW; sx += LAYER_W) {
                    int x0 = std::max(sx, 0), x1 = std::min(sx + (int)run.len, screenW);
                    if (x1 > x0) memcpy(dst + x0, src + run.x + (x0 - sx), (x1 - x0) * sizeof(uint32_t));
                }
            }
        }
    }
}

// --- Painting into a layer buffer (x wraps at LAYER_W) ---
static uint32_t* canvas;
static void plot(int x, int y, uint32_t c) {
    if (y < 0 || y >= LAYER_H) return;
    canvas[y * LAYER_W + ((x % LAYER_W) + LAYER_W) % LAYER_W] = c;
}
static void paintRect(int x, int y, int w, int h, uint32_t c) {
    for (int yy = y; yy < y + h; yy++)
        for (int xx = x; xx < x + w; xx++) plot(xx, yy, c);
}
static void paintDisc(float cx, float cy, float r, uint32_t c) {
    for (int y = (int)(cy - r) - 1; y <= (int)(cy + r) + 1; y++)
        for (int x = (int)(cx - r) - 1; x <= (int)(cx + r) + 1; x++)
            if ((x + 0.5f - cx) * (x + 0.5f - cx) + (y + 0.5f - cy) * (y + 0.5f - cy) <= r * r) plot(x, y, c);
}
// A line `thick` px wide.
static void paintLine(float x0, float y0, float x1, float y1, float thick, uint32_t c) {
    int steps = (int)(fmaxf(fabsf(x1 - x0), fabsf(y1 - y0))) + 1;
    for (int i = 0; i <= steps; i++) {
        float t = (float)i / steps;
        paintDisc(x0 + (x1 - x0) * t, y0 + (y1 - y0) * t, thick / 2, c);
    }
}
static void paintSprite(const Sprite& s, int x, int y, const WorldPalette& pal) {
    for (int row = 0; row < s.h; row++)
        for (int col = 0; col < s.w; col++) {
            char ch = s.pixels[row * s.w + col];
            int i = ch >= '0' && ch <= '9' ? ch - '0' : ch >= 'a' && ch <= 'f' ? ch - 'a' + 10 : -1;
            if (i >= 0) plot(x + col, y + row, pal.c[i]);
        }
}
static uint32_t seed = 1;
static int randomInt(int n) {   // a fixed-seed LCG, so the layers come out the same every time
    seed = seed * 1103515245u + 12345u;
    return (int)((seed >> 8) % (uint32_t)n);
}

// --- Namula: dusk sky, sea cliffs and a lighthouse, the village's rooftops, near fences and a tree ---
static const Sprite* const NAMULA_HOUSES[NAMULA_HOUSE_COUNT] = {&NAMULA_HOUSE_1, &NAMULA_HOUSE_2, &NAMULA_HOUSE_3, &NAMULA_HOUSE_4,
    &NAMULA_HOUSE_5, &NAMULA_HOUSE_6, &NAMULA_HOUSE_7, &NAMULA_HOUSE_8, &NAMULA_HOUSE_9, &NAMULA_HOUSE_10};
const int NAMULA_STREET = 316;   // L2 row the houses stand on
const int NAMULA_SEA = 268;      // L1 row of the horizon
const float TAU = 6.2831853f;

static int namulaHouseX(int i) { return i * 128 + (i * 37) % 11; }
// Where a far cliff's top is at L1 column x, or LAYER_H where there is open sea.
static int namulaCliffTop(int x) {
    float u = TAU * x / LAYER_W;
    if (sinf(u * 3 + 0.5f) < -0.35f) return LAYER_H;
    return (int)(226 + 26 * sinf(u * 2 + 1) + 12 * sinf(u * 5) + 5 * sinf(u * 13));
}
// Lantern k (0-2) on the string between house i and the next, in L2 px.
static void namulaLantern(int i, int k, int& x, int& y) {
    int x0 = namulaHouseX(i) + 50, x1 = namulaHouseX(i + 1) + 50;
    float t = (k + 1) / 4.0f;
    x = (int)(x0 + (x1 - x0) * t);
    y = 206 + (int)(14 * sinf(t * 3.14159f)) + 3;
}

static void paintNamula(uint32_t* out[4], int, int) {
    const WorldPalette& p = NAMULA_PALETTE;
    seed = 2024;
    // L0: a dithered dusk gradient (deep blue above, violet at the horizon), stars, a thin moon.
    canvas = out[0];
    for (int y = 0; y < LAYER_H; y++)
        for (int x = 0; x < LAYER_W; x++) {
            float v = fminf(fmaxf((y - 20) / 300.0f, 0), 1) * 3;
            int band = (int)v + ((v - (int)v) * 16 > bayer4(x, y));
            canvas[y * LAYER_W + x] = p.c[8 + std::min(band, 3)];
        }
    for (int i = 0; i < 120; i++) {
        int x = randomInt(LAYER_W), y = randomInt(randomInt(200) + 20), kind = randomInt(10);
        plot(x, y, p.c[kind < 6 ? 10 : 11]);
        if (kind == 9) {
            plot(x - 1, y, p.c[10]);
            plot(x + 1, y, p.c[10]);
            plot(x, y - 1, p.c[10]);
            plot(x, y + 1, p.c[10]);
        }
    }
    for (int y = 30; y < 90; y++)   // the moon: a disc minus a slightly shifted disc
        for (int x = 870; x < 930; x++) {
            float a = hypotf(x - 900.0f, y - 60.0f), b = hypotf(x - 907.0f, y - 55.0f);
            if (a <= 16 && b > 15) plot(x, y, p.c[11]);
        }

    // L1: the sea at the horizon with wave rows, far cliffs (2-3 values), the lighthouse with its lit window.
    canvas = out[1];
    for (int y = NAMULA_SEA; y < LAYER_H; y++)
        for (int x = 0; x < LAYER_W; x++)
            plot(x, y, (y - NAMULA_SEA) % 11 < 2 && (x / 7 + y) % 5 < 3 ? p.c[10] : p.c[9]);
    for (int x = 0; x < LAYER_W; x++) {
        int top = namulaCliffTop(x);
        for (int y = top; y < LAYER_H; y++) {
            bool stratum = (y + (int)(4 * sinf(x * 0.05f))) % 14 == 0;
            plot(x, y, y < top + 2 ? p.c[10] : stratum || y > NAMULA_SEA + 20 ? p.c[8] : p.c[9]);
        }
    }
    paintSprite(NAMULA_LIGHTHOUSE, 402, namulaCliffTop(420) - NAMULA_LIGHTHOUSE.h + 6, p);

    // L2: the village street, ten houses with chimneys and windows, lantern strings between them.
    canvas = out[2];
    paintRect(0, NAMULA_STREET, LAYER_W, LAYER_H - NAMULA_STREET, p.c[8]);
    for (int i = 0; i < NAMULA_HOUSE_COUNT; i++)
        paintSprite(*NAMULA_HOUSES[i], namulaHouseX(i), NAMULA_STREET - NAMULA_HOUSES[i]->h + 2, p);
    for (int i = 0; i < NAMULA_HOUSE_COUNT; i++) {
        int x0 = namulaHouseX(i) + 50, x1 = namulaHouseX(i + 1) + 50;
        for (int x = x0; x <= x1; x++) {
            float t = (float)(x - x0) / (x1 - x0);
            plot(x, 206 + (int)(14 * sinf(t * 3.14159f)), p.c[1]);
        }
        for (int k = 0; k < 3; k++) {
            int lx, ly;
            namulaLantern(i, k, lx, ly);
            paintRect(lx - 1, ly - 2, 3, 2, p.c[0]);
            paintRect(lx - 2, ly, 5, 5, p.c[12]);
        }
    }

    // L3: near and dark, under 25 % of the view: a tree with a laundry line, fence posts.
    canvas = out[3];
    for (int t = 0; t < 2; t++) {
        float tx = t ? 900 : 180, height = t ? 250 : 330;
        paintLine(tx, LAYER_H, tx + 8, LAYER_H - height, t ? 12 : 18, p.c[0]);
        for (int b = 0; b < 6; b++) {   // branches, each with a forked tip and a few leaf clumps
            float by = LAYER_H - height * (0.45f + b * 0.1f), dir = b % 2 ? 1 : -1, len = 40 + randomInt(40);
            float ex = tx + 8 + dir * len, ey = by - 20 - randomInt(30);
            paintLine(tx + 6, by, ex, ey, 5 - b * 0.5f, p.c[0]);
            paintLine(ex, ey, ex + dir * 18, ey - 14, 2, p.c[0]);
            for (int k = 0; k < 3; k++) paintDisc(ex + dir * randomInt(24), ey - 6 - randomInt(16), 5 + randomInt(5), p.c[4]);
        }
    }
    for (int x = 228; x <= 470; x++) {   // laundry line from the first tree to a pole, with cloths
        float t = (x - 228) / 242.0f;
        plot(x, 236 + (int)(26 * sinf(t * 3.14159f)), p.c[1]);
        if (x % 34 == 10 && x < 460) {   // a cloth pegged to the line, its hem uneven
            int y = 237 + (int)(26 * sinf(t * 3.14159f)), h = 9 + x % 3 * 3;
            paintRect(x, y, 10, h, x % 68 == 10 ? p.c[9] : p.c[1]);
            paintRect(x + 3, y + h, 4, 2, x % 68 == 10 ? p.c[9] : p.c[1]);
        }
    }
    paintRect(470, 230, 5, LAYER_H - 230, p.c[0]);
    for (int x = 560; x < 800; x += 28) paintRect(x, 300, 6, LAYER_H - 300, p.c[0]);   // fence
    for (int x = 1040; x < 1240; x += 28) paintRect(x, 310, 6, LAYER_H - 310, p.c[0]);
    paintRect(560, 312, 240, 4, p.c[0]);
    paintRect(560, 336, 240, 4, p.c[0]);
    paintRect(1040, 322, 200, 4, p.c[0]);
}

// Smoke from the chimneys, flickering lanterns, waves on the far sea, fireflies (all slower than 20 px/s).
static void namulaDetails(const Quest& q, int camX, int camY) {
    const WorldPalette& p = NAMULA_PALETTE;
    float t = q.time;
    auto put = [](int x, int y, uint32_t c) {
        if (x >= 0 && x < screenW && y >= 0 && y < screenH) screenTarget[y * screenW + x] = c;
    };
    auto wrapX = [](int x) { return ((x % LAYER_W) + LAYER_W) % LAYER_W; };
    // Waves: dashes drifting along the sea between the far cliffs.
    int off1X = (int)(camX * LAYER_FACTOR_X[1]), off1Y = (int)(camY * LAYER_FACTOR_Y[1]);
    for (int k = 0; k < 28; k++) {
        int lx = wrapX((int)(k * 263 + t * 6 * (k % 2 ? 1 : -1))), ly = NAMULA_SEA + 4 + (k * 7) % 40;
        if (namulaCliffTop(lx) <= ly) continue;
        int sx = wrapX(lx - off1X), sy = ly - off1Y;
        for (int i = 0; i < 5; i++) put(sx + i, sy, p.c[11]);
    }
    // Smoke: four puffs per chimney rising 12 px/s, growing and fading by dither.
    int off2X = (int)(camX * LAYER_FACTOR_X[2]), off2Y = (int)(camY * LAYER_FACTOR_Y[2]);
    for (int i = 0; i < NAMULA_HOUSE_COUNT; i++) {
        if (NAMULA_CHIMNEYS[i][0] < 0) continue;
        int cx = namulaHouseX(i) + NAMULA_CHIMNEYS[i][0], cy = NAMULA_STREET - NAMULA_HOUSES[i]->h + 2 + NAMULA_CHIMNEYS[i][1];
        for (int k = 0; k < 4; k++) {
            float age = fmodf(t * 0.25f + i * 0.37f + k * 0.25f, 1);
            int px = wrapX(cx + (int)(age * 10 + 3 * sinf(t + k)) - off2X), py = cy - (int)(age * 48) - off2Y;
            int r = 2 + (int)(age * 4);
            for (int dy = -r; dy <= r; dy++)
                for (int dx = -r; dx <= r; dx++)
                    if (dx * dx + dy * dy <= r * r && bayer4(px + dx, py + dy) < (1 - age) * 10) put(px + dx, py + dy, p.c[10]);
        }
    }
    // Lanterns on the strings flicker between amber shades.
    for (int i = 0; i < NAMULA_HOUSE_COUNT; i++)
        for (int k = 0; k < 3; k++) {
            int lx, ly;
            namulaLantern(i, k, lx, ly);
            int sx = wrapX(lx - off2X), sy = ly - off2Y;
            uint32_t c = p.c[((int)(t * 7) + i * 3 + k * 5) % 7 == 0 ? 13 : 14];
            for (int dy = 1; dy < 4; dy++)
                for (int dx = -1; dx < 2; dx++) put(sx + dx, sy + dy, c);
        }
    // Fireflies: single pixels wandering slowly, blinking.
    for (int i = 0; i < 10; i++) {
        if (sinf(t * 1.3f + i * 2.7f) < 0.2f) continue;
        int x = ((int)(i * 97 + 24 * sinf(t * 0.31f + i * 2.1f) - camX * 0.5f) % screenW + screenW) % screenW;
        int y = 60 + (i * 53) % 200 + (int)(18 * sinf(t * 0.23f + i));
        put(x, y, p.c[7]);
    }
}

static const PropDef NAMULA_PROPS[] = {
    {{&NAMULA_TUFT_A_1, &NAMULA_TUFT_A_2}, 2, 0.7f, PROP_TOP, false, 8},
    {{&NAMULA_TUFT_B_1, &NAMULA_TUFT_B_2}, 2, 0.6f, PROP_TOP, false, 8},
    {{&NAMULA_FLOWER_A_1, &NAMULA_FLOWER_A_2}, 2, 0.9f, PROP_TOP, false, 4},
    {{&NAMULA_FLOWER_B_1, &NAMULA_FLOWER_B_2}, 2, 1.1f, PROP_TOP, false, 3},
    {{&NAMULA_PEBBLES_A}, 1, 1, PROP_TOP, false, 5},
    {{&NAMULA_PEBBLES_B}, 1, 1, PROP_TOP, false, 4},
    {{&NAMULA_CRATE}, 1, 1, PROP_TOP, false, 2},
    {{&NAMULA_BARREL}, 1, 1, PROP_TOP, false, 2},
    {{&NAMULA_BUCKET}, 1, 1, PROP_TOP, false, 2},
    {{&NAMULA_STOOL}, 1, 1, PROP_TOP, false, 1},
    {{&NAMULA_CAT_1, &NAMULA_CAT_2, &NAMULA_CAT_3, &NAMULA_CAT_2}, 4, 0.5f, PROP_TOP, false, 1},
    {{&NAMULA_BASKET}, 1, 1, PROP_TOP, false, 1},
    {{&NAMULA_ROOTS_1, &NAMULA_ROOTS_2}, 2, 0.8f, PROP_CEILING, false, 1},
    {{&NAMULA_WALL_LANTERN_1, &NAMULA_WALL_LANTERN_2}, 2, 0.15f, PROP_WALL, true, 1},
};
// Ids 1-8 for AreaDef::landmarks: stone ring, well, stall 1, stall 2, signpost, statue, boats, the hut.
static const LandmarkDef NAMULA_LANDMARKS[] = {
    {{&NAMULA_STONE_RING}, 1, 1, 0, 0, 0, 0},
    {{&NAMULA_WELL}, 1, 1, 0, 0, 0, 0},
    {{&NAMULA_STALL_1}, 1, 1, 79, 33, 40, 0xFF6A3410},
    {{&NAMULA_STALL_2}, 1, 1, 71, 33, 40, 0xFF6A3410},
    {{&NAMULA_SIGNPOST}, 1, 1, 0, 0, 0, 0},
    {{&NAMULA_STATUE}, 1, 1, 0, 0, 0, 0},
    {{&NAMULA_BOATS}, 1, 1, 0, 0, 0, 0},
    {{&NAMULA_HUT}, 1, 1, 89, 72, 56, 0xFF5A3010},
};
static const WorldArt NAMULA_ART = {
    NAMULA_PALETTE,
    {{&NAMULA_TOP_1, &NAMULA_TOP_2, &NAMULA_TOP_3}, {&NAMULA_TOP_LEFT}, {&NAMULA_TOP_RIGHT}, {&NAMULA_LEFT}, {&NAMULA_RIGHT},
     {&NAMULA_FILL_1, &NAMULA_FILL_2, &NAMULA_FILL_3}, {&NAMULA_BOTTOM}, {&NAMULA_BOTTOM_LEFT}, {&NAMULA_BOTTOM_RIGHT},
     {&NAMULA_COLUMN_TOP}, {&NAMULA_COLUMN}, {&NAMULA_SINGLE}},
    {&NAMULA_FRINGE_1, &NAMULA_FRINGE_2, &NAMULA_FRINGE_3},
    &NAMULA_DRAPE_LEFT, &NAMULA_DRAPE_RIGHT,
    &NAMULA_PLANK_LEFT, &NAMULA_PLANK_MID, &NAMULA_PLANK_RIGHT,
    &NAMULA_BRACKET, &NAMULA_ROPE, &NAMULA_POST,
    {&NAMULA_LADDER, &NAMULA_LADDER_TOP},
    {&NAMULA_SEA_1, &NAMULA_SEA_2, &NAMULA_SEA_3},
    NAMULA_PROPS, (int)(sizeof NAMULA_PROPS / sizeof NAMULA_PROPS[0]),
    NAMULA_LANDMARKS, (int)(sizeof NAMULA_LANDMARKS / sizeof NAMULA_LANDMARKS[0]),
    paintNamula, namulaDetails,
    0.45f, 0.25f, 0.10f,
    {0xFF6A3410, 32},
};

const WorldArt* worldArt(int world) {
    return world == WORLD_NAMULA ? &NAMULA_ART : nullptr;
}

// --- Props (spec §22: deterministic, never near spots, objects or hazards) ---
bool propTileExcluded(const AreaState& area, int tx, int ty) {
    const WorldDef& w = worldDef(area.world);
    for (int i = 0; i < w.count; i++) {   // arrival tiles of every link into this area
        const AreaDef& a = w.areas[i];
        const AreaLink links[8] = {a.left, a.right, a.up, a.down, a.portals[0], a.portals[1], a.portals[2], a.portals[3]};
        for (const AreaLink& link : links)
            if (link.area == area.index && abs(link.tileX - tx) <= 1 && abs(link.tileY - ty) <= 1) return true;
    }
    for (int dy = -2; dy <= 2; dy++)
        for (int dx = -2; dx <= 2; dx++) {
            char t = tileAt(area, tx + dx, ty + dy);
            if (strchr("^~zf", t)) return true;
            if (abs(dx) <= 1 && abs(dy) <= 1 && strchr("SCMNPTKQWXLO", t)) return true;
        }
    return false;
}

void placeProps(const AreaState& area, std::vector<PlacedProp>& out) {
    out.clear();
    const WorldArt* art = worldArt(area.world);
    if (!art) return;
    const AreaDef& def = *area.def;
    for (int ty = 0; ty < def.rows; ty++)
        for (int tx = 0; tx < def.cols; tx++) {
            if (tileAt(area, tx, ty) != '.') continue;
            char below = tileAt(area, tx, ty + 1);
            bool insideX = tx > 0 && tx < def.cols - 1;
            int where = ty + 1 < def.rows && (tileSolid(below) || below == '-') ? PROP_TOP
                      : ty > 0 && tileSolid(tileAt(area, tx, ty - 1)) ? PROP_CEILING
                      : insideX && (tileSolid(tileAt(area, tx - 1, ty)) || tileSolid(tileAt(area, tx + 1, ty))) ? PROP_WALL : -1;
            if (where < 0 || propTileExcluded(area, tx, ty)) continue;
            uint32_t h = (uint32_t)(area.world * 73856093) ^ (uint32_t)(area.index * 19349663) ^ (uint32_t)(tx * 83492791) ^
                         (uint32_t)ty * 2654435761u;
            h ^= h << 13;
            h ^= h >> 17;
            h ^= h << 5;
            float density = where == PROP_TOP ? art->propDensityTop : where == PROP_CEILING ? art->propDensityCeiling : art->propDensityWall;
            if (h % 1000 >= (uint32_t)(density * 1000)) continue;
            int total = 0;
            for (int k = 0; k < art->propCount; k++) total += art->props[k].where == where ? art->props[k].weight : 0;
            if (total == 0) continue;
            int pick = (int)((h >> 10) % (uint32_t)total), kind = 0;
            for (; kind < art->propCount; kind++) {
                if (art->props[kind].where != where) continue;
                if (pick < art->props[kind].weight) break;
                pick -= art->props[kind].weight;
            }
            const Sprite& s = *art->props[kind].frames[0];
            PlacedProp prop = {kind, tx, ty, tx * TILE + TILE / 2 - s.w / 2, ty * TILE, false};
            if (where == PROP_TOP) {
                prop.x += (int)((h >> 24) % 7) - 3;
                prop.y = (ty + 1) * TILE - s.h;
                prop.flip = (h >> 20) & 1;
            } else if (where == PROP_WALL) {
                bool wallLeft = tileSolid(tileAt(area, tx - 1, ty));
                prop.x = wallLeft ? tx * TILE : (tx + 1) * TILE - s.w;
                prop.flip = !wallLeft;
            }
            out.push_back(prop);
        }
}

// --- Lights and vignette ---
void applyLights(const std::vector<Light>& lights, int nearX, int nearY) {
    // Falloff (1 - d/r)^2 * 64 per offset, cached per radius.
    struct Mask { int radius; std::vector<uint8_t> v; };
    static std::vector<Mask> masks;
    std::vector<const Light*> chosen;
    for (const Light& l : lights) chosen.push_back(&l);
    std::sort(chosen.begin(), chosen.end(), [&](const Light* a, const Light* b) {
        return (a->x - nearX) * (a->x - nearX) + (a->y - nearY) * (a->y - nearY) < (b->x - nearX) * (b->x - nearX) + (b->y - nearY) * (b->y - nearY);
    });
    if (chosen.size() > 16) chosen.resize(16);
    for (const Light* l : chosen) {
        const int r = l->radius, size = 2 * r + 1;
        if (l->x + r < 0 || l->x - r >= screenW || l->y + r < 0 || l->y - r >= screenH) continue;
        const Mask* mask = nullptr;
        for (const Mask& m : masks) if (m.radius == r) mask = &m;
        if (!mask) {
            Mask m = {r, std::vector<uint8_t>(size * size)};
            for (int dy = -r; dy <= r; dy++)
                for (int dx = -r; dx <= r; dx++) {
                    float d = sqrtf((float)(dx * dx + dy * dy)) / r;
                    m.v[(dy + r) * size + dx + r] = d >= 1 ? 0 : (uint8_t)((1 - d) * (1 - d) * 64);
                }
            masks.push_back(m);
            mask = &masks.back();
        }
        uint32_t cr = (l->color >> 16) & 0xFF, cg = (l->color >> 8) & 0xFF, cb = l->color & 0xFF;
        for (int y = std::max(0, l->y - r); y <= std::min(screenH - 1, l->y + r); y++) {
            const uint8_t* row = &mask->v[(y - l->y + r) * size - l->x + r];
            uint32_t* dst = screenTarget + y * screenW;
            for (int x = std::max(0, l->x - r); x <= std::min(screenW - 1, l->x + r); x++) {
                int q = row[x];
                int steps = (q >> 4) + ((q & 15) > bayer4(x, y));   // 0-4 quarter steps, dithered
                if (!steps) continue;
                uint32_t p = dst[x];
                uint32_t pr = std::min(((p >> 16) & 0xFF) + (cr * steps >> 2), 255u);
                uint32_t pg = std::min(((p >> 8) & 0xFF) + (cg * steps >> 2), 255u);
                uint32_t pb = std::min((p & 0xFF) + (cb * steps >> 2), 255u);
                dst[x] = 0xFF000000 | pr << 16 | pg << 8 | pb;
            }
        }
    }
}

void applyVignette() {
    // The darkened pixels of the 640x360 picture, built once: offset << 2 | steps (1-3).
    static std::vector<uint32_t> darkened;
    if (darkened.empty())
        for (int y = 0; y < QUEST_H; y++)
            for (int x = 0; x < QUEST_W; x++) {
                float ex = (x - QUEST_W / 2.0f) / (QUEST_W / 2.0f), ey = (y - QUEST_H / 2.0f) / (QUEST_H / 2.0f);
                float v = (sqrtf(ex * ex + ey * ey) / 1.4142f - 0.5f) / 0.5f * 2.4f;
                if (v <= 0) continue;
                int steps = std::min((int)v + ((v - (int)v) * 16 > bayer4(x, y)), 3);
                if (steps) darkened.push_back((uint32_t)(y * QUEST_W + x) << 2 | steps);
            }
    if (screenW != QUEST_W) return;
    for (uint32_t d : darkened) {
        uint32_t& p = screenTarget[d >> 2];
        p -= (d & 3) * ((p >> 3) & 0x1F1F1F);
    }
}
