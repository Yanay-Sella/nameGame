// NAME QUEST's art system (spec §22) and Namula's world art (spec §13 row 0): layers, props, lights, vignette.
#include "quest_art.h"
#include "quest_sprites_0.h"
#include "quest_sprites_1.h"
#include "quest_sprites_2.h"
#include "quest_sprites_3.h"
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
    nullptr,
    nullptr, nullptr,
    nullptr, {},
};

// --- World 1, The Drifting Station: deep space, a dim sun and a ringed planet, station spars, near pipes (spec §13 row 1) ---
const int STATION_SPAR_Y[3] = {150, 236, 330};   // L2 rows of the long truss beams
const int STATION_DISH_X = 820, STATION_DISH_Y = 132;   // L2: the radar dish's hub

// A truss beam along L2 row y: two rails with cross-bracing.
static void paintTruss(int x0, int x1, int y, const WorldPalette& p) {
    paintRect(x0, y, x1 - x0, 3, p.c[1]);
    paintRect(x0, y + 15, x1 - x0, 3, p.c[1]);
    for (int x = x0; x < x1; x += 18) {
        paintLine((float)x, (float)y + 2, (float)x + 18, (float)y + 15, 1.5f, p.c[9]);
        paintRect(x, y, 2, 18, p.c[9]);
    }
}

static void paintStation(uint32_t* out[4], int, int) {
    const WorldPalette& p = STATION_PALETTE;
    seed = 1977;
    // L0: black space with a dithered violet-red nebula band and three depths of stars.
    canvas = out[0];
    for (int y = 0; y < LAYER_H; y++)
        for (int x = 0; x < LAYER_W; x++) {
            float band = 150 + 60 * sinf(TAU * x / LAYER_W * 2 + 0.6f) + 20 * sinf(TAU * x / LAYER_W * 5);
            float v = fmaxf(0, 1 - fabsf(y - band) / 90) * 2.2f;
            int step = (int)v + ((v - (int)v) * 16 > bayer4(x, y));
            float red = 0.5f + 0.5f * sinf(TAU * x / LAYER_W * 3 + y * 0.02f);   // a slow red tint along the band
            bool tint = step >= 2 && red * 16 > bayer4(x + 1, y + 2) + 9;
            uint32_t c = tint ? p.c[4] : step >= 2 ? p.c[10] : step == 1 ? p.c[9] : p.c[8];
            canvas[y * LAYER_W + x] = c;
        }
    for (int i = 0; i < 260; i++) {
        int x = randomInt(LAYER_W), y = randomInt(LAYER_H), kind = randomInt(12);
        plot(x, y, p.c[kind < 7 ? 10 : 11]);
        if (kind == 11) {
            plot(x - 1, y, p.c[10]);
            plot(x + 1, y, p.c[10]);
            plot(x, y - 1, p.c[10]);
            plot(x, y + 1, p.c[10]);
        }
    }

    // L1: a dim red sun with a dithered corona, a ringed planet; nothing else (space is empty).
    canvas = out[1];
    for (int y = 30; y < 170; y++)
        for (int x = 160; x < 300; x++) {
            float d = hypotf(x - 230.0f, y - 100.0f);
            if (d <= 34) plot(x, y, d > 30 ? p.c[5] : p.c[4]);
            else if (d <= 62 && (62 - d) / 28 * 16 > bayer4(x, y) + 4) plot(x, y, p.c[4]);
        }
    for (int y = 150; y < 330; y++)
        for (int x = 700; x < 1060; x++) {
            float dx = x - 880.0f, dy = y - 240.0f, d = hypotf(dx, dy);
            float ring = hypotf(dx, dy * 3.6f + dx * 0.9f);   // a tilted ellipse
            bool ringFront = ring > 120 && ring < 136 && (d > 58 || dy > -dx * 0.25f);
            if (d <= 58) {
                float lit = (-dx - dy) / 58;   // lit from the sun's side, upper left
                int step = lit > 0.35f ? 2 : lit > -0.3f ? 1 : 0;
                bool stripe = (int)(dy + 0.2f * dx + 60) % 17 < 4;
                plot(x, y, p.c[9 + std::max(0, step - (stripe ? 1 : 0))]);
            }
            if (ringFront) plot(x, y, ring < 128 ? p.c[11] : p.c[10]);
        }

    // L2: three long truss beams with solar panel wings, a far docked ship, the radar dish's mast.
    canvas = out[2];
    for (int k = 0; k < 3; k++) {
        int y = STATION_SPAR_Y[k], x0 = k * 300 + 40, x1 = x0 + 520 + k * 120;
        paintTruss(x0, x1, y, p);
        for (int x = x0 + 60; x < x1 - 80; x += 150) {   // solar wings above and below the beam
            for (int side = -1; side <= 1; side += 2) {
                int py = side < 0 ? y - 46 : y + 22;
                paintRect(x + 18, side < 0 ? py + 40 : y + 18, 3, side < 0 ? 6 : 4, p.c[1]);
                paintRect(x, py, 40, 24, p.c[9]);
                for (int cy = py + 1; cy < py + 24; cy += 6)
                    for (int cx = x + 1; cx < x + 40; cx += 8) {   // dark cells with a teal glint on their top edge
                        paintRect(cx, cy, 7, 5, p.c[10]);
                        paintRect(cx, cy, 7, 1, p.c[12]);
                    }
            }
        }
    }
    // The docked ship: a long hull with an engine block and a few lit windows.
    paintRect(1000, 268, 170, 26, p.c[1]);
    paintRect(1010, 262, 60, 6, p.c[1]);
    paintRect(1160, 260, 40, 42, p.c[2]);
    for (int x = 1020; x < 1150; x += 14) paintRect(x, 276, 4, 3, x % 28 ? p.c[12] : p.c[5]);
    paintLine(1000, 281, 940, 281, 3, p.c[1]);   // the docking arm
    paintRect(STATION_DISH_X - 2, STATION_DISH_Y, 5, STATION_SPAR_Y[1] - STATION_DISH_Y, p.c[1]);

    // L3: near and dark, under 25 % of the view: vertical pipes with clamps, a cable bundle, a broken window frame.
    canvas = out[3];
    const int pipes[4][2] = {{120, 14}, {150, 8}, {760, 18}, {1110, 10}};
    for (const auto& pipe : pipes) {
        paintRect(pipe[0], 0, pipe[1], LAYER_H, p.c[0]);
        for (int y = 40; y < LAYER_H; y += 90) paintRect(pipe[0] - 3, y, pipe[1] + 6, 8, p.c[1]);
    }
    for (int x = 150; x <= 760; x++) {   // a sagging cable bundle between two pipes
        float t = (x - 150) / 610.0f;
        int y = 60 + (int)(70 * sinf(t * 3.14159f));
        paintRect(x, y, 1, 5, p.c[0]);
    }
    paintRect(420, 0, 10, 300, p.c[0]);   // the broken window: two struts and a jagged sill
    paintRect(560, 0, 10, 210, p.c[0]);
    paintLine(430, 300, 470, 270, 6, p.c[0]);
    paintLine(560, 210, 530, 240, 5, p.c[0]);
}

// Blinking lights on the spars, slow drifting debris, sparking cables near the pipes, the turning radar dish.
static void stationDetails(const Quest& q, int camX, int camY) {
    const WorldPalette& p = STATION_PALETTE;
    float t = q.time;
    auto put = [](int x, int y, uint32_t c) {
        if (x >= 0 && x < screenW && y >= 0 && y < screenH) screenTarget[y * screenW + x] = c;
    };
    auto wrapX = [](int x) { return ((x % LAYER_W) + LAYER_W) % LAYER_W; };
    int off2X = (int)(camX * LAYER_FACTOR_X[2]), off2Y = (int)(camY * LAYER_FACTOR_Y[2]);
    for (int k = 0; k < 3; k++)   // red and teal beacons along the beams, each on its own beat
        for (int i = 0; i < 6; i++) {
            if (fmodf(t * 0.8f + i * 0.37f + k * 0.21f, 1) > 0.25f) continue;
            int lx = wrapX(k * 300 + 70 + i * 96 - off2X), ly = STATION_SPAR_Y[k] - 3 - off2Y;
            uint32_t c = p.c[i % 2 ? 5 : 13];
            for (int d = 0; d < 4; d++) put(lx + d % 2, ly + d / 2, c);
        }
    // The radar dish: a short arc turning on its mast (0.6 rad/s).
    float a = t * 0.6f;
    int hx = wrapX(STATION_DISH_X - off2X), hy = STATION_DISH_Y - off2Y;
    for (int k = -14; k <= 14; k++) {
        float u = k / 14.0f;
        int x = hx + (int)(cosf(a) * k), y = hy - 6 + (int)(u * u * 8 * fabsf(sinf(a)) - 2);
        put(x, y, p.c[2]);
        put(x, y + 1, p.c[1]);
    }
    // Debris: small dark shards drifting across L1 at 4-9 px/s.
    int off1X = (int)(camX * LAYER_FACTOR_X[1]), off1Y = (int)(camY * LAYER_FACTOR_Y[1]);
    for (int i = 0; i < 9; i++) {
        int x = wrapX((int)(i * 151 + t * (4 + i % 5)) - off1X), y = 40 + (i * 89) % 300 + (int)(6 * sinf(t * 0.2f + i)) - off1Y;
        for (int d = 0; d < 3 + i % 3; d++) put(x + d, y + (d * (i % 2)) / 2, p.c[i % 3 ? 10 : 2]);
    }
    // Sparks where the cable bundle meets the near pipes: a few pixels flashing now and then.
    int off3X = (int)(camX * LAYER_FACTOR_X[3]), off3Y = (int)(camY * LAYER_FACTOR_Y[3]);
    const int sparks[2][2] = {{156, 62}, {754, 62}};
    for (int s = 0; s < 2; s++) {
        if (fmodf(t * 1.3f + s * 0.5f, 2.1f) > 0.18f) continue;
        for (int d = 0; d < 5; d++)
            put(wrapX(sparks[s][0] + (d * 7 + (int)(t * 50)) % 9 - 4 - off3X), sparks[s][1] + (d * 5) % 7 - 3 - off3Y, p.c[d % 2 ? 7 : 14]);
    }
}

static const PropDef STATION_PROPS[] = {
    {{&STATION_CABLE_COIL}, 1, 1, PROP_TOP, false, 5},
    {{&STATION_BOLTS}, 1, 1, PROP_TOP, false, 6},
    {{&STATION_SCREEN_1, &STATION_SCREEN_2}, 2, 0.4f, PROP_TOP, false, 2},
    {{&STATION_CRATE}, 1, 1, PROP_TOP, false, 3},
    {{&STATION_SIGN}, 1, 1, PROP_TOP, false, 1},
    {{&STATION_TANK}, 1, 1, PROP_TOP, false, 2},
    {{&STATION_HELMET}, 1, 1, PROP_TOP, false, 1},
    {{&STATION_ANTENNA_1, &STATION_ANTENNA_2}, 2, 0.8f, PROP_TOP, false, 2},
    {{&STATION_CONSOLE_1, &STATION_CONSOLE_2}, 2, 0.5f, PROP_TOP, true, 2},
    {{&STATION_HANGING_1, &STATION_HANGING_2}, 2, 1.1f, PROP_CEILING, false, 3},
    {{&STATION_LAMP_1, &STATION_LAMP_2}, 2, 0.7f, PROP_CEILING, true, 1},
    {{&STATION_WALL_PIPE}, 1, 1, PROP_WALL, false, 2},
    {{&STATION_WALL_SCREEN_1, &STATION_WALL_SCREEN_2}, 2, 0.5f, PROP_WALL, true, 1},
};
// Ids 1-6 for AreaDef::landmarks: airlock, reactor, observation window, wrecked shuttle, server racks, crew mural.
static const LandmarkDef STATION_LANDMARKS[] = {
    {{&STATION_AIRLOCK}, 1, 1, 64, 8, 32, 0xFF501010},
    {{&STATION_REACTOR_1, &STATION_REACTOR_2}, 2, 0.6f, 48, 80, 80, 0xFF104850},
    {{&STATION_WINDOW}, 1, 1, 0, 0, 0, 0},
    {{&STATION_SHUTTLE}, 1, 1, 0, 0, 0, 0},
    {{&STATION_RACKS_1, &STATION_RACKS_2}, 2, 0.4f, 40, 56, 40, 0xFF0C3438},
    {{&STATION_MURAL}, 1, 1, 0, 0, 0, 0},
};
static const WorldArt STATION_ART = {
    STATION_PALETTE,
    {{&STATION_TOP_1, &STATION_TOP_2, &STATION_TOP_3}, {&STATION_TOP_LEFT}, {&STATION_TOP_RIGHT}, {&STATION_LEFT}, {&STATION_RIGHT},
     {&STATION_FILL_1, &STATION_FILL_2, &STATION_FILL_3}, {&STATION_BOTTOM}, {&STATION_BOTTOM_LEFT}, {&STATION_BOTTOM_RIGHT},
     {&STATION_COLUMN_TOP}, {&STATION_COLUMN}, {&STATION_SINGLE}},
    {&STATION_FRINGE_1, &STATION_FRINGE_2, &STATION_FRINGE_3},
    &STATION_DRAPE_LEFT, &STATION_DRAPE_RIGHT,
    &STATION_CATWALK_LEFT, &STATION_CATWALK_MID, &STATION_CATWALK_RIGHT,
    &STATION_BRACKET, &STATION_CABLE, &STATION_STRUT,
    {&STATION_LADDER, &STATION_LADDER_TOP},
    {&STATION_COOLANT_1, &STATION_COOLANT_2, &STATION_COOLANT_3},
    STATION_PROPS, (int)(sizeof STATION_PROPS / sizeof STATION_PROPS[0]),
    STATION_LANDMARKS, (int)(sizeof STATION_LANDMARKS / sizeof STATION_LANDMARKS[0]),
    paintStation, stationDetails,
    0.35f, 0.25f, 0.10f,
    {0xFF0C3438, 24},
    &STATION_PIT,
    nullptr, nullptr,
    nullptr, {},
};

// --- World 2, The Greendeep: green-gold canopy light, giant trees and a temple in mist, vines and pillars, near fronds (spec §13 row 2) ---
const int GREEN_TEMPLE_X = 700;   // L1: the temple top in the mist
const int GREEN_PILLARS[4] = {180, 470, 860, 1130};   // L2: temple pillars

static void paintGreendeep(uint32_t* out[4], int, int) {
    const WorldPalette& p = GREEN_PALETTE;
    seed = 4242;
    // L0: canopy light, dark at the top, a green-gold glow lower down, with dithered sun beams slanting down-right.
    canvas = out[0];
    for (int y = 0; y < LAYER_H; y++)
        for (int x = 0; x < LAYER_W; x++) {
            float v = fminf(fmaxf((y - 30) / 260.0f, 0), 1) * 2;
            int band = (int)v + ((v - (int)v) * 16 > bayer4(x, y));
            uint32_t c = p.c[9 + std::min(band, 2)];
            float beam = fmodf(x - y * 0.45f + 2000, 230);   // a beam 26 px wide every 230 px
            if (beam < 26 && y > 20) {
                float strength = (1 - fabsf(beam - 13) / 13) * 12 * (1 - y / (float)LAYER_H);
                if (strength > bayer4(x + 2, y)) c = band >= 1 ? p.c[2] : p.c[11];
            }
            canvas[y * LAYER_W + x] = c;
        }
    // L1: giant tree trunks with crowns of leaves, the temple's stepped top in mist between them.
    canvas = out[1];
    for (int i = 0; i < 6; i++) {
        int x = i * 213 + randomInt(60), w = 26 + randomInt(22);
        for (int y = 60; y < LAYER_H; y++) {
            int flare = y > 400 ? (y - 400) / 3 : 0;   // the roots spread at the foot
            paintRect(x - flare, y, w + 2 * flare, 1, p.c[9]);
        }
        for (int k = 0; k < 7; k++) paintDisc((float)(x + w / 2 - 70 + randomInt(140)), (float)(30 + randomInt(70)), 26.0f + randomInt(24), p.c[9]);
    }
    for (int k = 0; k < 5; k++) paintRect(GREEN_TEMPLE_X - 90 + k * 18, 210 + k * 22 - 110, 180 - k * 36, 24, p.c[10]);
    paintRect(GREEN_TEMPLE_X - 12, 70, 24, 30, p.c[10]);
    for (int y = 230; y < 330; y++)   // mist: a dithered band thinning upward
        for (int x = 0; x < LAYER_W; x++)
            if ((1 - fabsf(y - 290.0f) / 60) * 10 > bayer4(x, y) + 3) plot(x, y, p.c[10]);
    // L2: temple pillars with moss caps, hanging vines, broad leaves.
    canvas = out[2];
    for (int x : GREEN_PILLARS) {
        paintRect(x, 150, 34, LAYER_H - 150, p.c[10]);
        paintRect(x - 4, 140, 42, 12, p.c[11]);
        for (int y = 170; y < LAYER_H; y += 40) paintRect(x, y, 34, 2, p.c[9]);
        paintRect(x - 4, 136, 42, 5, p.c[4]);
    }
    for (int i = 0; i < 22; i++) {   // vines from the top, swaying a little along their length
        int x = randomInt(LAYER_W), length = 80 + randomInt(220);
        for (int y = 0; y < length; y++) {
            int sx = x + (int)(4 * sinf(y * 0.05f + i));
            plot(sx, y, p.c[5]);
            if (y % 14 == 0) paintDisc((float)sx + (y % 28 ? 3 : -3), (float)y, 2.5f, p.c[4]);
        }
    }
    for (int i = 0; i < 14; i++) {   // broad leaves: long ellipses with a dark rib
        float cx = (float)randomInt(LAYER_W), cy = 260.0f + randomInt(200), a = (randomInt(100) - 50) / 100.0f;
        for (int k = -16; k <= 16; k++) {
            float r = 6 * (1 - (k * k) / 300.0f);
            paintDisc(cx + cosf(a) * k * 2, cy + sinf(a) * k * 2, r, p.c[5]);
        }
        paintLine(cx - cosf(a) * 30, cy - sinf(a) * 30, cx + cosf(a) * 30, cy + sinf(a) * 30, 1, p.c[4]);
    }
    // L3: near fronds hanging from the top and roots rising at the bottom, dark and sparse.
    canvas = out[3];
    const int fronds[3] = {90, 560, 1010};
    for (int f = 0; f < 3; f++)
        for (int k = 0; k < 5; k++) {
            float a = 0.6f + k * 0.45f, x0 = (float)fronds[f], y0 = -10;
            for (int i = 0; i < 60; i++) {
                float t = i / 60.0f, x = x0 + cosf(a) * 120 * t, y = y0 + sinf(a) * 60 * t + 90 * t * t;
                paintDisc(x, y, 5 * (1 - t) + 1, p.c[8]);
                if (i % 6 == 3) paintLine(x, y, x + 6, y + 12, 2, p.c[8]);
            }
        }
    const int roots[2] = {330, 820};
    for (int r = 0; r < 2; r++)
        for (int k = 0; k < 4; k++)
            for (int y = LAYER_H; y > 330; y--) {
                float t = (LAYER_H - y) / 150.0f;
                paintRect(roots[r] + k * 22 + (int)(30 * t * t * (k - 1.5f)), y, 9 - (int)(5 * t), 1, p.c[0]);
            }
}

// Fireflies blinking in L2, leaves falling slowly in L1, butterflies fluttering in L2, drips from the vines.
static void greendeepDetails(const Quest& q, int camX, int camY) {
    const WorldPalette& p = GREEN_PALETTE;
    float t = q.time;
    auto put = [](int x, int y, uint32_t c) {
        if (x >= 0 && x < screenW && y >= 0 && y < screenH) screenTarget[y * screenW + x] = c;
    };
    auto wrapX = [](int x) { return ((x % LAYER_W) + LAYER_W) % LAYER_W; };
    int off1X = (int)(camX * LAYER_FACTOR_X[1]), off1Y = (int)(camY * LAYER_FACTOR_Y[1]);
    int off2X = (int)(camX * LAYER_FACTOR_X[2]), off2Y = (int)(camY * LAYER_FACTOR_Y[2]);
    for (int i = 0; i < 24; i++) {   // fireflies: 1 px drifting slowly, glowing up every few seconds
        float glow = sinf(t * (1.1f + i % 4 * 0.3f) + i * 1.7f);
        if (glow < 0.3f) continue;
        int x = wrapX((int)(i * 97 + 20 * sinf(t * 0.3f + i)) - off2X), y = 120 + (i * 61) % 240 + (int)(10 * sinf(t * 0.4f + i * 2)) - off2Y;
        put(x, y, p.c[glow > 0.8f ? 13 : 12]);
        if (glow > 0.8f) put(x + 1, y, p.c[12]);
    }
    for (int i = 0; i < 10; i++) {   // leaves falling at 10-16 px/s, swaying
        float fall = fmodf(t * (10 + i % 4 * 2) + i * 53, 420);
        int x = wrapX((int)(i * 131 + 14 * sinf(t * 1.3f + i)) - off1X), y = (int)fall - 30 - off1Y;
        uint32_t c = p.c[i % 2 ? 2 : 5];
        put(x, y, c);
        put(x + ((int)(t * 2 + i) % 2 ? 1 : -1), y + 1, c);
    }
    for (int i = 0; i < 3; i++) {   // butterflies: two wings opening and closing
        int x = wrapX((int)(i * 400 + t * (6 + i * 2) + 20 * sinf(t * 0.7f + i)) - off2X);
        int y = 200 + i * 50 + (int)(12 * sinf(t * 0.9f + i * 3)) - off2Y;
        bool open = (int)(t * 6 + i) % 2;
        put(x, y, p.c[1]);
        put(x - 1, y - (open ? 1 : 0), p.c[i == 1 ? 12 : 3]);
        put(x + 1, y - (open ? 1 : 0), p.c[i == 1 ? 12 : 3]);
    }
    for (int i = 0; i < 6; i++) {   // drips falling from the vines' tips in L2
        float fall = fmodf(t * 40 + i * 37, 90);
        int x = wrapX(i * 211 + 40 - off2X), y = 150 + (i * 47) % 100 + (int)fall - off2Y;
        if (fall < 60) put(x, y, p.c[11]);
    }
}

static const PropDef GREEN_PROPS[] = {
    {{&GREEN_FERN_1, &GREEN_FERN_2}, 2, 0.9f, PROP_TOP, false, 6},
    {{&GREEN_MUSHROOMS_BROWN}, 1, 1, PROP_TOP, false, 3},
    {{&GREEN_MUSHROOM_TALL}, 1, 1, PROP_TOP, false, 2},
    {{&GREEN_MUSHROOMS_GLOW_1, &GREEN_MUSHROOMS_GLOW_2}, 2, 0.8f, PROP_TOP, true, 2},
    {{&GREEN_FLOWERS}, 1, 1, PROP_TOP, false, 3},
    {{&GREEN_LEAVES}, 1, 1, PROP_TOP, false, 5},
    {{&GREEN_ROOT}, 1, 1, PROP_TOP, false, 3},
    {{&GREEN_SNAIL}, 1, 1, PROP_TOP, false, 1},
    {{&GREEN_STONE}, 1, 1, PROP_TOP, false, 2},
    {{&GREEN_BONE}, 1, 1, PROP_TOP, false, 1},
    {{&GREEN_CEILING_VINES_1, &GREEN_CEILING_VINES_2}, 2, 1.2f, PROP_CEILING, false, 4},
    {{&GREEN_DRIP_1, &GREEN_DRIP_2}, 2, 0.6f, PROP_CEILING, false, 1},
    {{&GREEN_WALL_FERN}, 1, 1, PROP_WALL, false, 2},
};
// Ids 1-6 for AreaDef::landmarks: fallen statue head, temple stairs, waterfall, giant flower, rope bridge, idol.
static const LandmarkDef GREEN_LANDMARKS[] = {
    {{&GREEN_STATUE_HEAD}, 1, 1, 0, 0, 0, 0},
    {{&GREEN_TEMPLE_STAIRS}, 1, 1, 0, 0, 0, 0},
    {{&GREEN_WATERFALL_1, &GREEN_WATERFALL_2}, 2, 0.15f, 20, 64, 40, 0xFF0C3020},
    {{&GREEN_FLOWER_1, &GREEN_FLOWER_2}, 2, 0.9f, 0, 0, 0, 0},
    {{&GREEN_ROPE_BRIDGE}, 1, 1, 0, 0, 0, 0},
    {{&GREEN_IDOL_1, &GREEN_IDOL_2}, 2, 0.5f, 20, 17, 36, 0xFF104838},
};
static const WorldArt GREEN_ART = {
    GREEN_PALETTE,
    {{&GREEN_TOP_1, &GREEN_TOP_2, &GREEN_TOP_3}, {&GREEN_TOP_LEFT}, {&GREEN_TOP_RIGHT}, {&GREEN_LEFT}, {&GREEN_RIGHT},
     {&GREEN_FILL_1, &GREEN_FILL_2, &GREEN_FILL_3}, {&GREEN_BOTTOM}, {&GREEN_BOTTOM_LEFT}, {&GREEN_BOTTOM_RIGHT},
     {&GREEN_COLUMN_TOP}, {&GREEN_COLUMN}, {&GREEN_SINGLE}},
    {&GREEN_FRINGE_1, &GREEN_FRINGE_2, &GREEN_FRINGE_3},
    &GREEN_DRAPE_LEFT, &GREEN_DRAPE_RIGHT,
    &GREEN_BRANCH_LEFT, &GREEN_BRANCH_MID, &GREEN_BRANCH_RIGHT,
    &GREEN_BRACKET, &GREEN_HANGING_VINE, &GREEN_POST,
    {&GREEN_VINE, &GREEN_VINE_TOP},
    {&GREEN_WATER_1, &GREEN_WATER_2, &GREEN_WATER_3},
    GREEN_PROPS, (int)(sizeof GREEN_PROPS / sizeof GREEN_PROPS[0]),
    GREEN_LANDMARKS, (int)(sizeof GREEN_LANDMARKS / sizeof GREEN_LANDMARKS[0]),
    paintGreendeep, greendeepDetails,
    0.35f, 0.25f, 0.10f,
    {0xFF0C3428, 24},
    nullptr,
    &GREEN_THORNS, &GREEN_THORN_GATE,
    nullptr, {},
};

// --- World 3, The White Silence: a dim blue cave glow, far ice cliffs and a frozen waterfall, crystals and frozen trees, near ice pillars (spec §13 row 3) ---
const int FROST_FALL_X = 640;   // L1: the frozen waterfall between the cliffs

static void paintWhiteSilence(uint32_t* out[4], int, int) {
    const WorldPalette& p = FROST_PALETTE;
    seed = 3131;
    // L0: dark at the top and bottom, a cold glow band in the middle, dithered between the steps.
    canvas = out[0];
    for (int y = 0; y < LAYER_H; y++)
        for (int x = 0; x < LAYER_W; x++) {
            float v = (1 - fabsf(y - 250.0f) / 230) * 3;
            int step = std::min(std::max((int)v + ((v - (int)v) * 16 > bayer4(x, y)), 0), 3);
            canvas[y * LAYER_W + x] = p.c[8 + step];
        }
    // L1: jagged ice cliffs along the bottom with a pale rim on their peaks, the frozen waterfall pouring from a notch.
    canvas = out[1];
    int peaks[12];
    for (int i = 0; i < 12; i++) peaks[i] = 190 + randomInt(90);
    for (int x = 0; x < LAYER_W; x++) {
        int i = x * 12 / LAYER_W, x0 = i * LAYER_W / 12, span = LAYER_W / 12;
        float t = (x - x0) / (float)span;   // a sawtooth ridge between two peaks
        int next = peaks[(i + 1) % 12], top = (int)(peaks[i] + (next - peaks[i]) * t + 40 * sinf(t * 3.14159f) + 6 * sinf(x * 0.3f));
        if (abs(x - FROST_FALL_X) < 70) top += 60 - abs(x - FROST_FALL_X) * 60 / 70;
        paintRect(x, top, 1, LAYER_H - top, p.c[10]);
        paintRect(x, top, 1, 2, p.c[11]);
    }
    // L2: crystal clusters on the floor, blunt blue-white icicles from the top, frozen trees.
    canvas = out[2];
    for (int i = 0; i < 9; i++) {
        float cx = (float)(i * 142 + randomInt(60)), base = 400.0f + randomInt(50);
        for (int k = 0; k < 4; k++) {
            float lean = (k - 1.5f) * 9, h = 30.0f + randomInt(50);
            for (int j = 0; j < (int)h; j++) {
                float t = j / h, w = 7 * (1 - t);
                paintRect((int)(cx + k * 10 + lean * t - w), (int)(base - j), (int)(2 * w) + 1, 1, p.c[t > 0.1f && k % 2 ? 2 : 11]);
            }
        }
    }
    for (int i = 0; i < 40; i++) {   // decorative icicles: short, rounded at the tip, never the hazard's sharp shape
        int x = randomInt(LAYER_W), length = 20 + randomInt(70);
        for (int y = 0; y < length; y++) paintRect(x - 3 + y * 3 / length, y, 6 - y * 5 / length, 1, p.c[y < length / 2 ? 11 : 2]);
        paintDisc((float)x, (float)length, 2, p.c[2]);
    }
    for (int i = 0; i < 4; i++) {   // frozen trees: a trunk and bare branches, rimed
        int x = 150 + i * 330 + randomInt(40);
        paintLine((float)x, 470, (float)x, 250, 5, p.c[10]);
        for (int k = 0; k < 5; k++) {
            float y = 290.0f + k * 30, dir = k % 2 ? 1.0f : -1.0f;
            paintLine((float)x, y, x + dir * (40 + randomInt(30)), y - 40 - randomInt(30), 2, p.c[10]);
        }
        for (int k = 0; k < 8; k++) paintDisc((float)(x - 50 + randomInt(100)), (float)(220 + randomInt(100)), 2, p.c[11]);
    }
    // L3: near ice pillars, dark and sparse, with a pale rim.
    canvas = out[3];
    const int pillars[3] = {60, 520, 980};
    for (int x : pillars)
        for (int y = 0; y < LAYER_H; y++) {
            int w = 26 + (int)(10 * sinf(y * 0.02f + x));
            paintRect(x, y, w, 1, p.c[9]);
            plot(x + w - 1, y, p.c[11]);
        }
}

// Snow falling at three depths, crystals glinting in L2, breath-like mist drifting, a shimmer running down the far waterfall.
static void whiteSilenceDetails(const Quest& q, int camX, int camY) {
    const WorldPalette& p = FROST_PALETTE;
    float t = q.time;
    auto put = [](int x, int y, uint32_t c) {
        if (x >= 0 && x < screenW && y >= 0 && y < screenH) screenTarget[y * screenW + x] = c;
    };
    auto wrapX = [](int x) { return ((x % LAYER_W) + LAYER_W) % LAYER_W; };
    for (int depth = 0; depth < 3; depth++) {   // far flakes slow and dim, near flakes fast, bright and 2 px
        float factor = depth == 0 ? LAYER_FACTOR_X[1] : depth == 1 ? LAYER_FACTOR_X[2] : 0.8f;
        int offX = (int)(camX * factor), offY = (int)(camY * factor * 0.5f);
        for (int i = 0; i < 30 - depth * 8; i++) {
            float fall = fmodf(t * (8 + depth * 10 + i % 3 * 2) + i * 41, 400);
            int x = wrapX((int)(i * 89 + depth * 300 + 10 * sinf(t * 0.8f + i)) - offX) % screenW, y = (int)fall - 20 - offY % 400;
            uint32_t c = p.c[depth == 0 ? 3 : depth == 1 ? 5 : 7];
            put(x, y, c);
            if (depth == 2) put(x + 1, y, c), put(x, y + 1, c), put(x + 1, y + 1, c);
        }
    }
    int off2X = (int)(camX * LAYER_FACTOR_X[2]), off2Y = (int)(camY * LAYER_FACTOR_Y[2]);
    for (int i = 0; i < 9; i++) {   // a glint on each crystal cluster, every few seconds
        float glint = sinf(t * 0.9f + i * 2.3f);
        if (glint < 0.85f) continue;
        int x = wrapX(i * 142 + 30 - off2X), y = 380 - off2Y;
        put(x, y, p.c[13]);
        put(x - 1, y, p.c[12]);
        put(x + 1, y, p.c[12]);
        put(x, y - 1, p.c[12]);
    }
    for (int i = 0; i < 4; i++) {   // mist puffs drifting low across the floor
        int cx = wrapX((int)(i * 330 + t * 6) - off2X), cy = 300 + i * 13 - off2Y;
        for (int y = -6; y <= 6; y++)
            for (int x = -24; x <= 24; x++)
                if ((1 - (x * x / 576.0f + y * y / 36.0f)) * 16 > bayer4(cx + x, cy + y) + 10) put(cx + x, cy + y, p.c[11]);
    }
    int off1X = (int)(camX * LAYER_FACTOR_X[1]), off1Y = (int)(camY * LAYER_FACTOR_Y[1]);
    int shimmer = (int)fmodf(t * 40, 300);   // a pale line sliding down the frozen waterfall
    for (int k = 0; k < 5; k++) put(wrapX(FROST_FALL_X - 34 + k * 17 - off1X), 160 + shimmer - off1Y, p.c[5]);
}

static const PropDef FROST_PROPS[] = {
    {{&FROST_SNOW_MOUND}, 1, 1, PROP_TOP, false, 5},
    {{&FROST_SNOW_MOUND_SMALL}, 1, 1, PROP_TOP, false, 4},
    {{&FROST_CRYSTALS_1, &FROST_CRYSTALS_2}, 2, 0.7f, PROP_TOP, true, 3},
    {{&FROST_GRASS}, 1, 1, PROP_TOP, false, 4},
    {{&FROST_BONES}, 1, 1, PROP_TOP, false, 1},
    {{&FROST_LANTERN_1, &FROST_LANTERN_2}, 2, 0.4f, PROP_TOP, true, 1},
    {{&FROST_PEBBLES}, 1, 1, PROP_TOP, false, 3},
    {{&FROST_SHRUB}, 1, 1, PROP_TOP, false, 2},
    {{&FROST_ROCK}, 1, 1, PROP_TOP, false, 2},
    {{&FROST_CEILING_1, &FROST_CEILING_2}, 2, 1.1f, PROP_CEILING, false, 4},
    {{&FROST_WALL_FROST}, 1, 1, PROP_WALL, false, 2},
};
// Ids 1-6 for AreaDef::landmarks: ice cathedral arch, frozen mammoth, frozen waterfall, crystal geode, explorer's camp, frozen tree.
static const LandmarkDef FROST_LANDMARKS[] = {
    {{&FROST_CATHEDRAL}, 1, 1, 0, 0, 0, 0},
    {{&FROST_MAMMOTH}, 1, 1, 0, 0, 0, 0},
    {{&FROST_FROZEN_FALL_1, &FROST_FROZEN_FALL_2}, 2, 0.12f, 0, 0, 0, 0},
    {{&FROST_GEODE_1, &FROST_GEODE_2}, 2, 0.6f, 24, 24, 40, 0xFF103850},
    {{&FROST_CAMP_1, &FROST_CAMP_2}, 2, 0.35f, 49, 31, 36, 0xFF3A2410},
    {{&FROST_TREE}, 1, 1, 0, 0, 0, 0},
};
static const WorldArt FROST_ART = {
    FROST_PALETTE,
    {{&FROST_TOP_1, &FROST_TOP_2, &FROST_TOP_3}, {&FROST_TOP_LEFT}, {&FROST_TOP_RIGHT}, {&FROST_LEFT}, {&FROST_RIGHT},
     {&FROST_FILL_1, &FROST_FILL_2, &FROST_FILL_3}, {&FROST_BOTTOM}, {&FROST_BOTTOM_LEFT}, {&FROST_BOTTOM_RIGHT},
     {&FROST_COLUMN_TOP}, {&FROST_COLUMN}, {&FROST_SINGLE}},
    {&FROST_FRINGE_1, &FROST_FRINGE_2, &FROST_FRINGE_3},
    &FROST_DRAPE_LEFT, &FROST_DRAPE_RIGHT,
    &FROST_SLAB_LEFT, &FROST_SLAB_MID, &FROST_SLAB_RIGHT,
    &FROST_BRACKET, &FROST_ROPE, &FROST_PILLAR,
    {&FROST_LADDER, &FROST_LADDER_TOP},
    {&FROST_WATER_1, &FROST_WATER_2, &FROST_WATER_3},
    FROST_PROPS, (int)(sizeof FROST_PROPS / sizeof FROST_PROPS[0]),
    FROST_LANDMARKS, (int)(sizeof FROST_LANDMARKS / sizeof FROST_LANDMARKS[0]),
    paintWhiteSilence, whiteSilenceDetails,
    0.35f, 0.25f, 0.10f,
    {0xFF0C2838, 24},
    nullptr,
    &FROST_SPIKES, &FROST_ICE_WALL,
    &FROST_ICE, {&FROST_CRACKED_1, &FROST_CRACKED_2, &FROST_CRACKED_3},
};

const WorldArt* worldArt(int world) {
    return world == WORLD_NAMULA ? &NAMULA_ART : world == 1 ? &STATION_ART : world == 2 ? &GREEN_ART : world == 3 ? &FROST_ART : nullptr;
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
            if (abs(dx) <= 1 && abs(dy) <= 1 && strchr("SCMNPTKQWXLORYgbp", t)) return true;
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
