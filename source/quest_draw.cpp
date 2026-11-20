// NAME QUEST drawing at 640x360 (D-053): the slot, class and intro screens, and play: layers, tiles, props, landmarks,
// objects, hero, lights, HUD (spec §13, §15, §21, §22). Layout numbers are in dp, multiplied by DP.
#include "quest.h"
#include "quest_art.h"
#include "quest_sprites.h"
#include "screen.h"
#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

double (*questClock)() = nullptr;
float questDrawMs[3];

const float SHAKE_X = 4 * DP, SHAKE_Y = 3 * DP;   // at trauma 1 and full strength, like the shooter
const float WALK_FRAME = 0.1f, IDLE_FRAME = 0.6f, CLIMB_FRAME = 0.2f, SHIMMER_FRAME = 0.25f, FLAME_FRAME = 0.15f;
const float LIQUID_FRAME = 0.2f, FALL_SPEED = 40 * DP;
const char TRIM_BY_JOB[4] = {'h', 'b', 'Y', 'N'};
const uint32_t HEARTH_LIGHT = 0xFFA05018, BRAZIER_LIGHT = 0xFF804010;
const int HEARTH_LIGHT_RADIUS = 48 * DP, BRAZIER_LIGHT_RADIUS = 28 * DP;
enum HeroFrame { FRAME_IDLE_1, FRAME_IDLE_2, FRAME_WALK_1, FRAME_WALK_2, FRAME_WALK_3, FRAME_WALK_4, FRAME_JUMP, FRAME_FALL,
                 FRAME_ATTACK_1, FRAME_ATTACK_2, FRAME_ATTACK_3, FRAME_CLIMB_1, FRAME_CLIMB_2, FRAME_ROLL_1, FRAME_ROLL_2,
                 FRAME_HURT, HERO_FRAME_COUNT };
#define HERO_ROW(C) {&HERO_##C##_IDLE_1, &HERO_##C##_IDLE_2, &HERO_##C##_WALK_1, &HERO_##C##_WALK_2, &HERO_##C##_WALK_3, \
    &HERO_##C##_WALK_4, &HERO_##C##_JUMP, &HERO_##C##_FALL, &HERO_##C##_ATTACK_1, &HERO_##C##_ATTACK_2, &HERO_##C##_ATTACK_3, \
    &HERO_##C##_CLIMB_1, &HERO_##C##_CLIMB_2, &HERO_##C##_ROLL_1, &HERO_##C##_ROLL_2, &HERO_##C##_HURT}
static const Sprite* const HERO_FRAMES[CLASS_COUNT][HERO_FRAME_COUNT] = {HERO_ROW(KNIGHT), HERO_ROW(ARCHER), HERO_ROW(MAGICIAN),
                                                                          HERO_ROW(THIEF)};
// Class select: POWER, TOUGHNESS, REACH out of 5, and two lines about the class.
const int CLASS_BARS[CLASS_COUNT][3] = {{3, 5, 1}, {3, 2, 5}, {5, 1, 4}, {2, 3, 2}};
const char* const CLASS_TEXT[CLASS_COUNT][2] = {
    {"HEAVY ARMOUR,", "A WIDE SWORD."}, {"ARROWS FROM", "FAR AWAY."}, {"BOLTS THAT HIT", "HARD. FRAGILE."}, {"FAST DAGGERS,", "QUICK FEET."}};
const char* const JOB_NAMES[CLASS_COUNT][4] = {{"KNIGHT", "VANGUARD", "WARDEN", "LIONHEART"},
                                               {"ARCHER", "RANGER", "SHARPSHOOTER", "STORMBOW"},
                                               {"MAGICIAN", "ADEPT", "ELEMENTALIST", "ARCHMAGE"},
                                               {"THIEF", "ROGUE", "SHADOW", "NIGHTBLADE"}};

static void drawCentred(int y, const char* text, uint32_t color, int scale = 1) {
    drawText((QUEST_W - textWidth(text, scale * DP)) / 2, y, text, color, scale * DP);
}

static void drawPanel(int x, int y, int w, int h, char border) {
    fillRect(x, y, w, h, paletteColor(border));
    fillRect(x + DP, y + DP, w - 2 * DP, h - 2 * DP, paletteColor('k'));
}

// Worlds without art (TEST GROUNDS and worlds not built yet): plain with a 32 dp grid.
static void drawPlainBackground(int camX, int camY) {
    const int grid = 32 * DP;
    fillRect(0, 0, QUEST_W, QUEST_H, paletteColor('k'));
    for (int x = (grid - camX % grid) % grid; x < QUEST_W; x += grid) fillRect(x, 0, DP, QUEST_H, paletteColor('n'));
    for (int y = (grid - camY % grid) % grid; y < QUEST_H; y += grid) fillRect(0, y, QUEST_W, DP, paletteColor('n'));
}

// Phase 1's flat look: every tile in worlds without art, and the hazards and doors that have no art yet.
static void drawTileFlat(const Quest& q, char t, int tx, int ty, int sx, int sy) {
    const WorldStyle& style = worldDef(q.area.world).style;
    bool topOpen = !tileSolid(tileAt(q.area, tx, ty - 1));
    switch (t) {
    case '#':
        fillRect(sx, sy, TILE, TILE, paletteColor(style.fill));
        if (topOpen) fillRect(sx, sy, TILE, 2 * DP, paletteColor(style.top));
        if ((tx * 7 + ty * 3) % 5 == 0) fillRect(sx + (2 + tx % 4) * DP, sy + 4 * DP, DP, DP, paletteColor(style.top));
        break;
    case 'i':
        fillRect(sx, sy, TILE, TILE, paletteColor('I'));
        if (topOpen) fillRect(sx, sy, TILE, 2 * DP, paletteColor('i'));
        fillRect(sx + (1 + tx % 5) * DP, sy + 3 * DP, 2 * DP, DP, paletteColor('i'));
        break;
    case 'c':
        fillRect(sx, sy, TILE, TILE, paletteColor(style.fill));
        if (topOpen) fillRect(sx, sy, TILE, 2 * DP, paletteColor(style.top));
        for (int i = 0; i < 4; i++) fillRect(sx + (2 + i) * DP, sy + (2 + i + (i > 1)) * DP, DP, DP, paletteColor('k'));
        fillRect(sx + 5 * DP, sy + 6 * DP, 2 * DP, DP, paletteColor('k'));
        break;
    case '-':
        fillRect(sx, sy, TILE, 2 * DP, paletteColor(style.platform));
        fillRect(sx + DP, sy + 2 * DP, DP, 2 * DP, paletteColor(style.fill));
        fillRect(sx + 6 * DP, sy + 2 * DP, DP, 2 * DP, paletteColor(style.fill));
        break;
    case '|':
        fillRect(sx + DP, sy, DP, TILE, paletteColor(style.ladder));
        fillRect(sx + 6 * DP, sy, DP, TILE, paletteColor(style.ladder));
        fillRect(sx + DP, sy + 2 * DP, 6 * DP, DP, paletteColor(style.ladder));
        fillRect(sx + DP, sy + 6 * DP, 6 * DP, DP, paletteColor(style.ladder));
        break;
    case '^':
        for (int k = 0; k < 2; k++) {
            fillRect(sx + (k * 4 + 1) * DP, sy + 2 * DP, 2 * DP, 2 * DP, paletteColor('w'));
            fillRect(sx + (k * 4 + 1) * DP, sy + 4 * DP, 2 * DP, 2 * DP, paletteColor('b'));
            fillRect(sx + k * 4 * DP, sy + 6 * DP, 4 * DP, 2 * DP, paletteColor('s'));
        }
        break;
    case '~': {
        int frame = (int)(q.time / SHIMMER_FRAME) % 2;
        fillRect(sx, sy, TILE, TILE, paletteColor(style.liquidA));
        if (tileAt(q.area, tx, ty - 1) != '~') fillRect(sx, sy, TILE, DP, paletteColor(style.liquidB));
        fillRect(sx + (frame ? 1 : 5) * DP, sy + 3 * DP, 2 * DP, DP, paletteColor(style.liquidB));
        fillRect(sx + (frame ? 5 : 1) * DP, sy + 6 * DP, 2 * DP, DP, paletteColor(style.liquidB));
        break;
    }
    case 'g':
        fillRect(sx, sy + 3 * DP, TILE, DP, paletteColor('d'));
        for (int k = 0; k < 3; k++) fillRect(sx + (1 + 3 * k) * DP, sy, DP, TILE, paletteColor('s'));
        break;
    case 'k':
        fillRect(sx, sy, TILE, TILE, paletteColor('&'));
        fillRect(sx + DP, sy + DP, TILE - 2 * DP, TILE - 2 * DP, paletteColor('h'));
        break;
    case 'z':
        fillRect(sx, sy, TILE, TILE, paletteColor('n'));
        for (int i = 0; i < 8; i++) fillRect(sx + i * DP, sy + (3 + i % 2) * DP, DP, DP, paletteColor('c'));
        break;
    case 'f':
        fillRect(sx, sy, TILE, TILE, paletteColor('K'));
        fillRect(sx + 2 * DP, sy, 4 * DP, 2 * DP, paletteColor('O'));
        break;
    case '<':
    case '>': {
        int shift = (int)(q.time * 50) % 8;
        int x = t == '>' ? shift : 7 - shift;
        fillRect(sx + std::min(x, 5) * DP, sy + (2 + (tx + ty) % 4) * DP, 3 * DP, DP, paletteColor('n'));
        break;
    }
    case 'v':
        fillRect(sx, sy + 4 * DP, TILE, 4 * DP, paletteColor('d'));
        for (int k = 0; k < 3; k++) fillRect(sx + (1 + 3 * k) * DP, sy + 5 * DP, DP, 3 * DP, paletteColor('k'));
        break;
    case 'u':
        fillRect(sx, sy, TILE, DP, paletteColor('w'));
        fillRect(sx, sy + TILE - DP, TILE, DP, paletteColor('w'));
        fillRect(sx, sy, DP, TILE, paletteColor('w'));
        fillRect(sx + TILE - DP, sy, DP, TILE, paletteColor('w'));
        break;
    }
}

static uint32_t tileHash(int tx, int ty) {
    uint32_t h = (uint32_t)tx * 73856093u ^ (uint32_t)ty * 19349663u;
    h ^= h >> 13;
    h *= 0x5bd1e995u;
    return h ^ (h >> 15);
}

// Autotile: the shape from which of the 4 neighbours are solid.
static TileShape shapeOf(bool up, bool right, bool down, bool left) {
    if (!up) return !left && !right ? (down ? SHAPE_COLUMN_TOP : SHAPE_SINGLE) : !left ? SHAPE_TOP_LEFT : !right ? SHAPE_TOP_RIGHT : SHAPE_TOP;
    if (!left && !right) return SHAPE_COLUMN;
    if (!down) return !left ? SHAPE_BOTTOM_LEFT : !right ? SHAPE_BOTTOM_RIGHT : SHAPE_BOTTOM;
    return !left ? SHAPE_LEFT : !right ? SHAPE_RIGHT : SHAPE_FILL;
}

// A tile in its world's art (spec §22): autotiled ground with caps, fringes and drapes; platforms with supports; ladders; liquids.
static void drawTile(const Quest& q, const WorldArt& art, int tx, int ty, int sx, int sy) {
    const AreaState& area = q.area;
    char t = tileAt(area, tx, ty);
    const WorldPalette& pal = art.palette;
    uint32_t hash = tileHash(tx, ty);
    switch (t) {
    case '#':
    case 'c':
    case 'i': {
        bool up = tileSolid(tileAt(area, tx, ty - 1)), right = tileSolid(tileAt(area, tx + 1, ty));
        bool down = tileSolid(tileAt(area, tx, ty + 1)), left = tileSolid(tileAt(area, tx - 1, ty));
        const Sprite* const* variants = art.ground[shapeOf(up, right, down, left)];
        const Sprite* sprite = variants[hash % 3] ? variants[hash % 3] : variants[0];
        drawSpritePal(*sprite, sx, sy, false, pal);
        if (t != '#') drawTileFlat(q, t, tx, ty, sx, sy);   // crumble cracks and ice keep phase 1's marks over the ground
        if (!up) {
            drawSpritePal(*art.fringe[(hash >> 8) % 3], sx, sy - 3 * DP, false, pal);
            if (!left) drawSpritePal(*art.drapeLeft, sx - art.drapeLeft->w, sy, false, pal);
            if (!right) drawSpritePal(*art.drapeRight, sx + TILE, sy, false, pal);
        }
        break;
    }
    case '-': {
        bool left = tileAt(area, tx - 1, ty) == '-', right = tileAt(area, tx + 1, ty) == '-';
        drawSpritePal(!left ? *art.platformLeft : !right ? *art.platformRight : *art.platformMid, sx, sy, false, pal);
        if (left && right) break;
        // Supports at each end: a bracket into a wall within 2 tiles, else ropes to a ceiling within 6, else posts to the ground within 3.
        int side = !left ? -1 : 1;
        for (int k = 1; k <= 2; k++) {
            if (!tileSolid(tileAt(area, tx + side * k, ty))) continue;
            for (int g = 0; g < k; g++)
                drawSpritePal(*art.bracket, sx + side * g * TILE, sy + 6 * DP, side > 0, pal);
            return;
        }
        for (int k = 1; k <= 6; k++) {
            if (!tileSolid(tileAt(area, tx, ty - k))) continue;
            for (int y = sy - (k - 1) * TILE; y < sy; y += art.rope->h) drawSpritePal(*art.rope, sx + TILE / 2 - art.rope->w / 2, y, false, pal);
            return;
        }
        for (int k = 1; k <= 3; k++) {
            if (!tileSolid(tileAt(area, tx, ty + k))) continue;
            for (int y = sy + 5 * DP; y < sy + k * TILE; y += art.post->h)
                drawSpritePal(*art.post, sx + (side < 0 ? 3 * DP : TILE - 3 * DP - art.post->w), y, false, pal);
            drawSpritePal(!left ? *art.platformLeft : *art.platformRight, sx, sy, false, pal);   // the plank over its posts
            return;
        }
        break;
    }
    case '|':
        drawSpritePal(*art.ladder[tileAt(area, tx, ty - 1) == '|' ? 0 : 1], sx, sy, false, pal);
        break;
    case '~':
        if (tileAt(area, tx, ty - 1) == '~') {
            fillRect(sx, sy, TILE, TILE, pal.c[8]);
            break;
        }
        // Depth: a dithered gradient from the background ramp's light end to its dark end, then the surface line.
        for (int y = 0; y < TILE; y++)
            for (int x = 0; x < TILE; x++) {
                float v = y / (float)TILE * 2;
                int step = (int)v + ((v - (int)v) * 16 > bayer4(sx + x, sy + y));
                if (sx + x >= 0 && sx + x < QUEST_W && sy + y >= 0 && sy + y < QUEST_H)
                    screenTarget[(sy + y) * QUEST_W + sx + x] = pal.c[10 - std::min(step, 2)];
            }
        drawSpritePal(*art.liquid[(int)(q.time / LIQUID_FRAME) % 3], sx, sy, false, pal);
        break;
    case '.':
        break;
    default:
        drawTileFlat(q, t, tx, ty, sx, sy);
        break;
    }
}

// Checkpoint id n: the n-th C of the world, areas in order, rows top to bottom.
static int checkpointId(int world, int index, int tile) {
    int id = 0;
    for (int i = 0; i <= index; i++) {
        const AreaDef& def = worldDef(world).areas[i];
        int end = i == index ? tile : def.cols * def.rows;
        for (int t = 0; t < end; t++) id += def.tiles[t] == 'C';
    }
    return id;
}

// Checkpoints, portals and chests standing on the tile below their letter; the hearth and lit braziers add a light.
static void drawObject(const Quest& q, char letter, int tx, int ty, int camX, int camY, std::vector<Light>& lights) {
    const AreaDef& def = *q.area.def;
    int cx = tx * TILE + TILE / 2 - camX, bottom = (ty + 1) * TILE - camY;
    int flame = (int)(q.time / FLAME_FRAME) % 3;
    const Sprite* sprite = nullptr;
    if (letter == 'C' && q.area.world == WORLD_NAMULA) {
        const Sprite* hearth[3] = {&HEARTH_A, &HEARTH_B, &HEARTH_C};
        sprite = hearth[flame];
        lights.push_back({cx, bottom - (int)(24 * DP), HEARTH_LIGHT_RADIUS, HEARTH_LIGHT});
    } else if (letter == 'C') {
        int bit = q.area.world * 2 + checkpointId(q.area.world, q.area.index, ty * def.cols + tx);
        const Sprite* lit[3] = {&CHECKPOINT_LIT_A, &CHECKPOINT_LIT_B, &CHECKPOINT_LIT_C};
        bool isLit = bit < 32 && (q.save.lit >> bit & 1);
        sprite = isLit ? lit[flame] : &CHECKPOINT_UNLIT;
        if (isLit) lights.push_back({cx, bottom - (int)(14 * DP), BRAZIER_LIGHT_RADIUS, BRAZIER_LIGHT});
    } else if (letter == 'P') {
        int index = 0;
        for (int t = 0; t < ty * def.cols + tx; t++) index += def.tiles[t] == 'P';
        bool open = index < 4 && def.portals[index].area >= 0;
        sprite = !open ? &ARCH_CLOSED : (int)(q.time / SHIMMER_FRAME) % 2 ? &PORTAL_B : &PORTAL_A;
    } else if (letter == 'T') {
        sprite = &CHEST_CLOSED;
    }
    if (sprite) drawSpriteOutlined(*sprite, cx - sprite->w / 2, bottom - sprite->h, false, paletteColor('x'));
}

static void drawHero(const Quest& q, int camX, int camY) {
    const Hero& h = q.hero;
    HeroFrame frame;
    if (q.mode == QMODE_DEAD) frame = FRAME_HURT;
    else if (h.climbing) frame = h.vy != 0 && (int)(q.time / CLIMB_FRAME) % 2 ? FRAME_CLIMB_2 : FRAME_CLIMB_1;
    else if (!h.onGround) frame = h.vy > FALL_SPEED ? FRAME_FALL : FRAME_JUMP;
    else if (fabsf(h.vx) > 10 * DP) frame = (HeroFrame)(FRAME_WALK_1 + (int)(q.time / WALK_FRAME) % 4);
    else frame = (int)(q.time / IDLE_FRAME) % 2 ? FRAME_IDLE_2 : FRAME_IDLE_1;
    int job = q.save.job >= 0 && q.save.job < 4 ? q.save.job : 0;
    const Sprite& sprite = *HERO_FRAMES[q.save.heroClass][frame];
    // Small old-school heroes (the user's choice): the outline is part of the sprite.
    drawSpriteEx(sprite, (int)floorf(h.x - 3 * DP) - camX, (int)floorf(h.y - 3 * DP) - camY, h.facingLeft, job ? 'h' : 0,
                 paletteColor(TRIM_BY_JOB[job]));
}

// Shake: shift the play picture by whole pixels; the HUD drawn after stays still.
static void shakePicture(const Quest& q) {
    float shake = q.trauma * q.trauma * q.shakeStrength;
    int dx = (int)lroundf(SHAKE_X * shake * sinf(q.time * 41)), dy = (int)lroundf(SHAKE_Y * shake * sinf(q.time * 57));
    if (dx == 0 && dy == 0) return;
    for (int i = 0; i < QUEST_H; i++) {
        int y = dy > 0 ? QUEST_H - 1 - i : i, sourceY = y - dy;   // copy order: a row is read before it is overwritten
        uint32_t* row = screenTarget + y * QUEST_W;
        if (sourceY < 0 || sourceY >= QUEST_H) {
            std::fill(row, row + QUEST_W, paletteColor('k'));
            continue;
        }
        memmove(row + std::max(dx, 0), screenTarget + sourceY * QUEST_W + std::max(-dx, 0), (QUEST_W - abs(dx)) * sizeof(uint32_t));
        if (dx > 0) std::fill(row, row + dx, paletteColor('k'));
        if (dx < 0) std::fill(row + QUEST_W + dx, row + QUEST_W, paletteColor('k'));
    }
}

static void drawPlay(const Quest& q) {
    int camX = (int)floorf(q.camX), camY = (int)floorf(q.camY);
    const AreaDef& def = *q.area.def;
    const WorldArt* art = worldArt(q.area.world);
    int col0 = camX / TILE, row0 = camY / TILE;
    std::vector<Light> lights;
    double t0 = questClock ? questClock() : 0;

    // Layers and their animated details.
    if (art) drawLayers(q.area.world, camX, camY);
    else drawPlainBackground(camX, camY);
    if (art) art->addDetails(q, camX, camY);
    double t1 = questClock ? questClock() : 0;

    // Tiles (a column to each side, for drapes), then the scattered props behind everything else.
    for (int ty = std::max(row0 - 1, 0); ty <= row0 + QUEST_H / TILE + 1 && ty < def.rows; ty++)
        for (int tx = std::max(col0 - 1, 0); tx <= col0 + QUEST_W / TILE + 1 && tx < def.cols; tx++) {
            if (art) drawTile(q, *art, tx, ty, tx * TILE - camX, ty * TILE - camY);
            else drawTileFlat(q, tileAt(q.area, tx, ty), tx, ty, tx * TILE - camX, ty * TILE - camY);
        }
    if (art) {
        static std::vector<PlacedProp> props;   // the current area's props, placed once per area
        static const AreaDef* propsArea = nullptr;
        if (propsArea != q.area.def) {
            placeProps(q.area, props);
            propsArea = q.area.def;
        }
        for (const PlacedProp& p : props) {
            const PropDef& d = art->props[p.kind];
            const Sprite& s = *d.frames[(int)(q.time / d.frameTime + p.tx * 0.37f) % d.frameCount];
            int sx = p.x - camX, sy = p.y - camY;
            if (sx + s.w < 0 || sx >= QUEST_W || sy + s.h < 0 || sy >= QUEST_H) continue;
            drawSpritePal(s, sx, sy, p.flip, art->palette);
            if (d.light) lights.push_back({sx + s.w / 2, sy + s.h / 2, art->propLight.radius, art->propLight.color});
        }
    }
    double t2 = questClock ? questClock() : 0;

    // Landmarks (L, by order of appearance), then objects; both reach well above their tile, so look below the view too.
    int landmark = 0;
    for (int t = 0; t < def.cols * def.rows && art; t++) {
        if (def.tiles[t] != 'L') continue;
        int id = landmark < 4 ? def.landmarks[landmark] : 0;
        landmark++;
        if (id < 1 || id > art->landmarkCount) continue;
        const LandmarkDef& d = art->landmarks[id - 1];
        const Sprite& s = *d.frames[(int)(q.time / d.frameTime) % d.frameCount];
        int sx = (t % def.cols) * TILE + TILE / 2 - s.w / 2 - camX, sy = (t / def.cols + 1) * TILE - s.h - camY;
        if (sx + s.w < 0 || sx >= QUEST_W || sy + s.h < 0 || sy >= QUEST_H) continue;
        drawSpritePal(s, sx, sy, false, art->palette);
        if (d.lightRadius > 0) lights.push_back({sx + d.lightX, sy + d.lightY, d.lightRadius, d.lightColor});
    }
    for (int ty = row0; ty <= row0 + QUEST_H / TILE + 4 && ty < def.rows; ty++)
        for (int tx = std::max(col0 - 2, 0); tx <= col0 + QUEST_W / TILE + 2 && tx < def.cols; tx++) {
            char letter = def.tiles[ty * def.cols + tx];
            if (letter == 'C' || letter == 'P' || letter == 'T') drawObject(q, letter, tx, ty, camX, camY, lights);
        }

    drawHero(q, camX, camY);
    // Grass fringe in front of the hero's feet: only its lowest 2 dp above the top line.
    if (art) {
        int heroCol0 = (int)floorf(q.hero.x / TILE) - 1, heroCol1 = (int)floorf((q.hero.x + HERO_W) / TILE) + 1;
        int feetRow = (int)floorf((q.hero.y + HERO_H) / TILE);
        for (int ty = feetRow - 1; ty <= feetRow + 1; ty++)
            for (int tx = heroCol0; tx <= heroCol1; tx++) {
                if (!tileSolid(tileAt(q.area, tx, ty)) || tileSolid(tileAt(q.area, tx, ty - 1)) || tx < 0 || tx >= def.cols) continue;
                const Sprite& f = *art->fringe[(tileHash(tx, ty) >> 8) % 3];
                Sprite low = {f.w, f.h - (int)DP, f.pixels + f.w * (int)DP};
                drawSpritePal(low, tx * TILE - camX, ty * TILE - 2 * DP - camY, false, art->palette);
            }
    }
    double t3 = questClock ? questClock() : 0;
    if (art) applyLights(lights, (int)(q.hero.x + HERO_W / 2) - camX, (int)(q.hero.y + HERO_H / 2) - camY);
    shakePicture(q);
    if (q.area.world != WORLD_SECRET) applyVignette();
    if (questClock) {
        double t4 = questClock();
        questDrawMs[0] = std::max(questDrawMs[0], (float)((t1 - t0) * 1000));
        questDrawMs[1] = std::max(questDrawMs[1], (float)((t2 - t1) * 1000));
        questDrawMs[2] = std::max(questDrawMs[2], (float)((t4 - t3) * 1000));
    }

    // HUD: level and the HP bar (full until combat comes in phase 2), then the banner.
    char text[32];
    snprintf(text, sizeof text, "LV %d", q.save.level);
    drawText(4 * DP, 4 * DP, text, paletteColor('w'), DP);
    fillRect(4 * DP, 11 * DP, 64 * DP, 4 * DP, paletteColor('R'));
    fillRect(4 * DP, 11 * DP, 64 * DP, 4 * DP, paletteColor('r'));
    if (q.bannerTimer > 0 && q.banner)
        drawCentred((q.bannerScale == 2 ? 40 : 30) * DP, q.banner, paletteColor(q.bannerScale == 2 ? 'w' : 'b'), q.bannerScale);
    if (q.mode == QMODE_DEAD) drawCentred(80 * DP, "YOU FADED", paletteColor('r'), 2);
}

static void drawSlots(const Quest& q) {
    fillRect(0, 0, QUEST_W, QUEST_H, paletteColor('k'));
    drawCentred(12 * DP, "NAME QUEST", paletteColor('w'), 2);
    for (int i = 0; i < SLOT_COUNT; i++) {
        const SlotSummary& s = q.slots[i];
        int y = (34 + 46 * i) * DP;
        drawPanel(20 * DP, y, 280 * DP, 40 * DP, q.mode == QMODE_SLOTS && q.cursor == i ? 'y' : 'n');
        char text[48];
        if (!s.used) {
            snprintf(text, sizeof text, "SLOT %d  - EMPTY -", i + 1);
            drawText(28 * DP, y + 17 * DP, text, paletteColor('s'), DP);
            continue;
        }
        int heroClass = s.heroClass % CLASS_COUNT, job = s.job >= 0 && s.job < 4 ? s.job : 0;
        snprintf(text, sizeof text, "SLOT %d", i + 1);
        drawText(28 * DP, y + 6 * DP, text, paletteColor('w'), DP);
        if (job) snprintf(text, sizeof text, "%s - %s  LV %d", CLASS_NAMES[heroClass], JOB_NAMES[heroClass][job], s.level);
        else snprintf(text, sizeof text, "%s  LV %d", CLASS_NAMES[heroClass], s.level);
        drawText(28 * DP, y + 16 * DP, text, paletteColor('b'), DP);
        int world = s.highest >= 0 && s.highest < WORLD_COUNT ? s.highest : 1;
        drawText(28 * DP, y + 26 * DP, worldDef(world).name, paletteColor('s'), DP);
        int seconds = (int)s.playtime;
        snprintf(text, sizeof text, "%d:%02d:%02d", seconds / 3600, seconds / 60 % 60, seconds % 60);
        drawText(292 * DP - textWidth(text, DP), y + 6 * DP, text, paletteColor('b'), DP);
        if (s.ngPlus > 0) {
            snprintf(text, sizeof text, "NG+%d", s.ngPlus);
            drawText(292 * DP - textWidth(text, DP), y + 16 * DP, text, paletteColor('o'), DP);
        }
        if (s.secretDone) drawText(292 * DP - textWidth("*", DP), y + 26 * DP, "*", paletteColor('z'), DP);
    }
    drawCentred(172 * DP, "X PLAY  TRIANGLE DELETE  O BACK", paletteColor('s'));
    if (q.mode == QMODE_DELETE) {
        char title[24];
        snprintf(title, sizeof title, "DELETE SLOT %d?", q.slot + 1);
        const char* const items[2] = {"NO", "YES"};
        drawMenu(title, items, 2, q.cursor, "X CHOOSE  O BACK");
    }
}

static void drawClassSelect(const Quest& q) {
    fillRect(0, 0, QUEST_W, QUEST_H, paletteColor('k'));
    drawCentred(10 * DP, "CHOOSE YOUR CLASS", paletteColor('w'), 2);
    const char* const bars[3] = {"POWER", "TOUGHNESS", "REACH"};
    for (int i = 0; i < CLASS_COUNT; i++) {
        int x = (12 + 76 * i) * DP, y = 30 * DP;
        drawPanel(x, y, 72 * DP, 120 * DP, q.cursor == i ? 'y' : 'n');
        const Sprite& hero = *HERO_FRAMES[i][FRAME_IDLE_1];
        drawSprite(hero, x + 36 * DP - hero.w / 2, y + 4 * DP);
        drawText(x + (72 * DP - textWidth(CLASS_NAMES[i], DP)) / 2, y + 26 * DP, CLASS_NAMES[i], paletteColor(q.cursor == i ? 'y' : 'w'), DP);
        for (int b = 0; b < 3; b++) {
            int by = y + (38 + 13 * b) * DP;
            drawText(x + 5 * DP, by, bars[b], paletteColor('s'), DP);
            for (int step = 0; step < 5; step++)
                fillRect(x + (5 + 12 * step) * DP, by + 7 * DP, 10 * DP, 3 * DP, paletteColor(step < CLASS_BARS[i][b] ? 'b' : 'n'));
        }
        if (i == CLASS_THIEF) drawText(x + 5 * DP, y + 80 * DP, "DOUBLE JUMP", paletteColor('o'), DP);
        drawText(x + 5 * DP, y + 94 * DP, CLASS_TEXT[i][0], paletteColor('b'), DP);
        drawText(x + 5 * DP, y + 102 * DP, CLASS_TEXT[i][1], paletteColor('b'), DP);
    }
    drawCentred(172 * DP, "X CHOOSE  O BACK", paletteColor('s'));
}

// The intro page typed out, line by line.
static void drawIntro(const Quest& q) {
    fillRect(0, 0, QUEST_W, QUEST_H, paletteColor('k'));
    const char* page = INTRO_PAGES[q.introPage % 4];
    int shown = (int)(q.modeTimer * INTRO_CHARS_PER_S), length = (int)strlen(page);
    const char* newline = strchr(page, '\n');
    int firstLength = (int)(newline - page);
    char line[64];
    snprintf(line, sizeof line, "%.*s", std::min(shown, firstLength), page);
    int width = textWidth(std::string(page, firstLength).c_str(), DP);
    drawText((QUEST_W - width) / 2, 78 * DP, line, paletteColor('b'), DP);
    if (shown > firstLength + 1) {
        snprintf(line, sizeof line, "%.*s", std::min(shown - firstLength - 1, length - firstLength - 1), newline + 1);
        drawText((QUEST_W - textWidth(newline + 1, DP)) / 2, 90 * DP, line, paletteColor('b'), DP);
    }
    if (shown >= length) drawCentred(160 * DP, "X", paletteColor((int)(q.time * 2) % 2 ? 's' : 'n'));
}

void questDraw(const Quest& q) {
    switch (q.mode) {
    case QMODE_SLOTS:
    case QMODE_DELETE: drawSlots(q); break;
    case QMODE_CLASS: drawClassSelect(q); break;
    case QMODE_INTRO: drawIntro(q); break;
    case QMODE_PLAY:
    case QMODE_DEAD: drawPlay(q); break;
    }
}
