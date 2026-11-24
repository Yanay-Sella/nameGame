// NAME QUEST drawing at 640x360 (D-053): the slot, class and intro screens, and play: layers, tiles, props, landmarks,
// objects, hero, lights, HUD (spec §13, §15, §21, §22). Layout numbers are in dp, multiplied by DP.
#include "quest.h"
#include "quest_art.h"
#include "quest_travel_previews.h"
#include "quest_sprites.h"
#include "quest_sprites_1.h"
#include "quest_sprites_2.h"
#include "quest_sprites_3.h"
#include "quest_sprites_hero.h"
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
const float SHIMMER_FRAME = 0.25f, FLAME_FRAME = 0.15f, LIQUID_FRAME = 0.2f;
const float ENEMY_FRAME = 0.25f, TELL_FLASH = 0.1f, IFRAME_BLINK = 0.1f;
const float NUMBER_RISE = 12 * DP, ECHO_BOB = 2 * DP, PARTNER_BEHIND = 10 * DP;
const char TRIM_BY_JOB[4] = {'h', 'b', 'Y', 'N'};
const uint32_t HEARTH_LIGHT = 0xFFA05018, BRAZIER_LIGHT = 0xFF804010, LANTERN_LIGHT = 0xFF704018;
const int HEARTH_LIGHT_RADIUS = 48 * DP, BRAZIER_LIGHT_RADIUS = 28 * DP, LANTERN_LIGHT_RADIUS = 24 * DP;
const float NPC_FRAME = 0.7f, POPUP_SHOWN = 1.5f, POPUP_RISE = 6 * DP, REST_FADE = 0.3f, FADE_DEPTH = 60 * DP, GATE_SHAKE = 0.5f;
const int MAP_CELL_W = 12 * DP, MAP_CELL_H = 8 * DP;
static_assert(sizeof NPC_FRAMES / sizeof NPC_FRAMES[0] == NPC_DEV_TESTER, "Every story NPC needs two idle frames");
// Boss frames: idle 1-2, tell, attack, hurt, phase 2 (spec §22); bosses of later phases have none yet.
#define BOSS_ROW(N) {&BOSS_##N##_IDLE_1, &BOSS_##N##_IDLE_2, &BOSS_##N##_TELL, &BOSS_##N##_ATTACK, &BOSS_##N##_HURT, &BOSS_##N##_PHASE2}
static const Sprite* const BOSS_FRAMES[6][6] = {BOSS_ROW(OVERSEER), BOSS_ROW(STAR_MAGE), BOSS_ROW(MONKEY_KING), BOSS_ROW(SILVERBACK),
                                                BOSS_ROW(YETI), BOSS_ROW(TITAN)};
const float BOSS_WHITE = 0.12f, MARKER_BLINK = 4, MARKER_BLINK_LAST = 12, MARKER_LAST = 0.3f, EDGE_WALL = 4 * DP;
static const Sprite* const ITEM_ICONS[ITEM_COUNT] = {&ICON_RED, &ICON_BIG, &ICON_MANA, &ICON_WHETSTONE, &ICON_TONIC, &ICON_FEATHER};
// Class select: POWER, TOUGHNESS, REACH out of 5, and two lines about the class.
const int CLASS_BARS[CLASS_COUNT][3] = {{3, 5, 1}, {3, 2, 5}, {5, 1, 4}, {2, 3, 2}};
const char* const CLASS_TEXT[CLASS_COUNT][2] = {
    {"HEAVY ARMOUR,", "A WIDE SWORD."}, {"ARROWS FROM", "FAR AWAY."}, {"BOLTS THAT HIT", "HARD. FRAGILE."}, {"FAST DAGGERS,", "QUICK FEET."}};
static void drawCentred(int y, const char* text, uint32_t color, int scale = 1) {
    drawText((QUEST_W - textWidth(text, scale * DP)) / 2, y, text, color, scale * DP);
}

// A 1x shooter sprite (banana, coconut, peel, poop) drawn as 2x2 blocks, the size the spec gives it in dp; see-through
// ice '~' mixes a quarter of its colour into what is under it, as drawSprite does.
static void drawDoubled(const Sprite& sprite, int x, int y, bool flip) {
    for (int row = 0; row < sprite.h; row++)
        for (int col = 0; col < sprite.w; col++) {
            char c = sprite.pixels[row * sprite.w + (flip ? sprite.w - 1 - col : col)];
            if (c == '~') {
                for (int py = y + row * DP; py < y + (row + 1) * DP; py++)
                    for (int px = x + col * DP; px < x + (col + 1) * DP; px++)
                        if (px >= 0 && px < screenW && py >= 0 && py < screenH) {
                            uint32_t& pixel = screenTarget[py * screenW + px];
                            pixel = 0xFF000000 | (((pixel >> 1) & 0x7F7F7F) + ((pixel >> 2) & 0x3F3F3F) + ((paletteColor('~') >> 2) & 0x3F3F3F));
                        }
            } else if (c != '.') {
                fillRect(x + col * DP, y + row * DP, DP, DP, paletteColor(c));
            }
        }
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
    case 'z': {   // a floor plate (spec §9): dark while off, flickering 0.4 s before on, arcs up the tile while on
        float cycle = q.area.def->band >= 2 ? 3.0f : 3.5f, t = fmodf(q.time, cycle);
        bool flicker = t > cycle - 0.4f && (int)(q.time * 20) % 2, on = panelOn(q);
        fillRect(sx, sy + 6 * DP, TILE, 2 * DP, paletteColor('d'));
        fillRect(sx + DP, sy + 6 * DP, TILE - 2 * DP, DP, paletteColor(on || flicker ? 'c' : 'n'));
        fillRect(sx, sy + 7 * DP, TILE, DP, paletteColor('k'));
        if (flicker) fillRect(sx + ((tx * 3) % 6) * DP, sy + 4 * DP, DP, 2 * DP, paletteColor('c'));
        if (on) {   // two jagged arcs, re-drawn 15 times a second
            int beat = (int)(q.time * 15) + tx * 7;
            for (int k = 0; k < 2; k++) {
                int x = sx + ((beat * (k + 3) + k * 4) % 7) * DP;
                for (int y = 0; y < 6; y++) fillRect(x + ((beat + y * (k + 1)) % 3 - 1) * DP, sy + y * DP, DP, DP, paletteColor(y % 2 ? 'c' : 'w'));
            }
        }
        break;
    }
    case 'f':   // sparks while warning, then a flame 3 tiles tall
        fillRect(sx, sy, TILE, TILE, paletteColor('K'));
        fillRect(sx + 2 * DP, sy, 4 * DP, 2 * DP, paletteColor('O'));
        if (jetWarning(q) > 0 && (int)(q.time * 16) % 2) fillRect(sx + (2 + (int)(q.time * 30) % 4) * DP, sy - 2 * DP, DP, DP, paletteColor('o'));
        if (jetBurning(q)) {
            fillRect(sx + DP, sy - 3 * TILE, TILE - 2 * DP, 3 * TILE, paletteColor('o'));
            fillRect(sx + 3 * DP, sy - 3 * TILE + 4 * DP, 2 * DP, 3 * TILE - 4 * DP, paletteColor('N'));
        }
        break;
    case '<':
    case '>': {   // world 3 and 6 wind: two snow streaks blowing the way the zone pushes now
        bool wind = q.area.world == 3 || q.area.world == 6, right = (t == '>') != (wind && q.area.windFlipped);
        for (int k = 0; k < (wind ? 2 : 1); k++) {
            int shift = (int)(q.time * 50 + k * 4 + tx * 3) % 8;
            int x = right ? shift : 7 - shift;
            fillRect(sx + std::min(x, 5) * DP, sy + (2 + (tx + ty + k * 3) % 6) * DP, 3 * DP, DP, paletteColor(wind ? 'w' : 'n'));
        }
        break;
    }
    case 'v': {   // a floor grille and pale streaks rising through the 6 tiles it lifts (spec §4)
        fillRect(sx, sy + 4 * DP, TILE, 4 * DP, paletteColor('d'));
        for (int k = 0; k < 3; k++) fillRect(sx + (1 + 3 * k) * DP, sy + 5 * DP, DP, 3 * DP, paletteColor('k'));
        for (int k = 0; k < 3; k++) {
            float rise = fmodf(q.time * 90 * DP + k * 37 * DP + tx * 11 * DP, 6 * TILE);
            fillRect(sx + (1 + 3 * k) * DP, sy - (int)rise, DP, 3 * DP, paletteColor('s'));
        }
        break;
    }
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
        if (t == 'i' && art.ice) {
            drawSpritePal(*art.ice, sx, sy, false, pal);
            break;
        }
        if (t == 'c' && art.crumble[0]) {   // the crack spreads over the shake; the block jitters 1 dp
            const TileTrap* trap = nullptr;
            for (const TileTrap& k : q.traps) if (k.tile == ty * area.def->cols + tx) trap = &k;
            int stage = trap && trap->state == 1 ? std::min(2, (int)((0.6f - trap->timer) / 0.2f)) : 0;
            int shake = trap && trap->state == 1 ? ((int)(q.time * 40) % 2 ? DP : -DP) : 0;
            drawSpritePal(*art.crumble[stage], sx + shake, sy, false, pal);
            break;
        }
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
    case '^':
        if (art.spikes) drawSpritePal(*art.spikes, sx, sy, hash & 1, pal);
        else drawTileFlat(q, t, tx, ty, sx, sy);
        break;
    case 'g':
        if (art.gate) drawSpritePal(*art.gate, sx, sy, false, pal);
        else drawTileFlat(q, t, tx, ty, sx, sy);
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
    case '.': {   // a pit's last 2 rows open to space (world 1): black with stars, so it never reads as floor
        bool pit = art.pit && ty >= area.def->rows - 2;
        for (int r = ty; r < area.def->rows && pit; r++) pit = !tileSolid(tileAt(area, tx, r)) && tileAt(area, tx, r) != '-';
        if (pit) drawSpritePal(*art.pit, sx, sy, (tx + ty) % 2, pal);
        break;
    }
    default:
        drawTileFlat(q, t, tx, ty, sx, sy);
        break;
    }
}

// Checkpoints, portals, chests, levers, NPCs and surveyors standing on the tile below their letter; the hearth, lit
// braziers and Ilsa's lantern add a light. NPCs face the hero.
static void drawObject(const Quest& q, char letter, int tx, int ty, int camX, int camY, std::vector<Light>& lights) {
    const AreaDef& def = *q.area.def;
    int cx = tx * TILE + TILE / 2 - camX, bottom = (ty + 1) * TILE - camY;
    int flame = (int)(q.time / FLAME_FRAME) % 3;
    const Sprite* sprite = nullptr;
    if ((letter == 'b' || letter == 'p') && !checkpointLit(q.save, q.area.world, BOSS_CHECKPOINT)) return;
    if (letter == 'C' && q.area.world == WORLD_NAMULA) {
        const Sprite* hearth[3] = {&HEARTH_A, &HEARTH_B, &HEARTH_C};
        sprite = hearth[flame];
        lights.push_back({cx, bottom - (int)(24 * DP), HEARTH_LIGHT_RADIUS, HEARTH_LIGHT});
    } else if (letter == 'C' || letter == 'b') {
        int id = letter == 'b' ? BOSS_CHECKPOINT : objectIndex(q.area.world, q.area.index, ty * def.cols + tx, 'C');
        const Sprite* lit[3] = {&CHECKPOINT_LIT_A, &CHECKPOINT_LIT_B, &CHECKPOINT_LIT_C};
        bool isLit = checkpointLit(q.save, q.area.world, id);
        sprite = isLit ? lit[flame] : &CHECKPOINT_UNLIT;
        if (isLit) lights.push_back({cx, bottom - (int)(14 * DP), BRAZIER_LIGHT_RADIUS, BRAZIER_LIGHT});
    } else if (letter == 'p') {
        const WorldDef& next = worldDef(q.area.world + 1);
        sprite = !next.count ? &ARCH_CLOSED : (int)(q.time / SHIMMER_FRAME) % 2 ? &PORTAL_B : &PORTAL_A;
        int width = textWidth(next.name, DP);
        drawPanel(cx - width / 2 - 3 * DP, bottom - sprite->h - 12 * DP, width + 6 * DP, 10 * DP, 'n');
        drawText(cx - width / 2, bottom - sprite->h - 10 * DP, next.name, paletteColor('w'), DP);
    } else if (letter == 'P') {
        int index = 0;
        for (int t = 0; t < ty * def.cols + tx; t++) index += def.tiles[t] == 'P';
        bool open = index < 4 && def.portals[index].area >= 0;
        sprite = !open ? &ARCH_CLOSED : (int)(q.time / SHIMMER_FRAME) % 2 ? &PORTAL_B : &PORTAL_A;
    } else if (letter == 'T') {
        int index = 0;
        for (int t = 0; t < ty * def.cols + tx; t++) index += def.tiles[t] == 'T';
        int id = index < 4 ? def.chests[index] : -1;
        bool opened = id >= 0 && (q.save.chests[q.area.world] >> id & 1);
        sprite = q.chestTimer > 0 && q.chestTile == ty * def.cols + tx ? &CHEST_AJAR : opened ? &CHEST_OPEN : &CHEST_CLOSED;
    } else if (letter == 'K') {
        bool pulled = def.shortcut >= 0 && (q.save.shortcuts[q.area.world] >> def.shortcut & 1);
        sprite = pulled ? &LEVER_ON : &LEVER_OFF;
    } else if (letter == 'R' && q.area.world == 2) {   // a coconut palm: its trunk down to the ground, the crown, its coconut
        const WorldArt& art = *worldArt(2);
        int state = 0;
        for (const TileTrap& trap : q.traps) if (trap.tile == ty * def.cols + tx) state = trap.state;
        for (int r = ty + 1; r < def.rows && !tileSolid(tileAt(q.area, tx, r)); r++)
            drawSpritePal(GREEN_PALM_TRUNK, cx - GREEN_PALM_TRUNK.w / 2, r * TILE - camY, false, art.palette);
        const Sprite& crown = state == 2 ? GREEN_PALM_BARE : GREEN_PALM;
        drawSpritePal(crown, cx - crown.w / 2, bottom - crown.h + 6 * DP, false, art.palette);
        int shake = state == 1 ? ((int)(q.time * 40) % 2 ? DP : -DP) : 0;
        if (state != 2) drawDoubled(COCONUT, cx - 3 * DP + shake, bottom - 6 * DP, false);
        return;
    } else if (letter == 'Y' && q.area.world == 3) {   // an icicle hanging from the ceiling above its letter; it shakes, then falls
        int state = 0;
        for (const TileTrap& trap : q.traps) if (trap.tile == ty * def.cols + tx) state = trap.state;
        int shake = state == 1 ? ((int)(q.time * 40) % 2 ? DP : -DP) : 0;
        if (state != 2) drawDoubled(ICICLE, cx - 2 * DP + shake, ty * TILE - camY, false);
        return;
    } else if (letter == 'N' || letter == 'M') {
        int id;
        if (letter == 'M') {
            id = surveyorId(q.area.world, q.area.index, ty * def.cols + tx);
        } else {
            int index = 0;
            for (int t = 0; t < ty * def.cols + tx; t++) index += def.tiles[t] == 'N';
            id = index < 8 ? def.npcs[index] : -1;
            if (id < 0 || id >= NPC_COUNT || !npcPresent(q.save, id, q.area.world, q.area.index, q.devAids)) return;
        }
        int idle = (int)(q.time / NPC_FRAME + tx * 0.3f) % 2;
        const Sprite& frame = *NPC_FRAMES[id == NPC_DEV_TESTER ? NPC_SURVEYOR_FIRST : id][idle];
        bool flip = q.hero.x + HERO_W / 2 < tx * TILE + TILE / 2;
        drawSpriteEx(frame, cx - frame.w / 2, bottom - frame.h, flip);
        if (id == NPC_DEV_TESTER)
            drawText(cx - textWidth("DEV", DP) / 2, bottom - frame.h - 8 * DP, "DEV", paletteColor('y'), DP);
        if (id == NPC_ILSA)
            lights.push_back({cx + (flip ? -1 : 1) * (int)(8 * DP), bottom - (int)((13 - idle) * DP), LANTERN_LIGHT_RADIUS, LANTERN_LIGHT});
        return;
    }
    if (sprite) drawSpriteOutlined(*sprite, cx - sprite->w / 2, bottom - sprite->h, false, paletteColor('x'));
}

// A hero sprite: trim 'h' in the tier colour, tier details '1'-'3' only from their tier on (spec §21), clipped.
static void drawHeroSprite(const Sprite& s, int x, int y, bool flip, int job) {
    uint32_t tier = paletteColor(TRIM_BY_JOB[job]);
    for (int row = 0; row < s.h; row++) {
        int py = y + row;
        if (py < 0 || py >= QUEST_H) continue;
        for (int col = 0; col < s.w; col++) {
            char c = s.pixels[row * s.w + col];
            if (c == '.' || (c >= '1' && c <= '3' && job < c - '0')) continue;
            int px = x + (flip ? s.w - 1 - col : col);
            if (px < 0 || px >= QUEST_W) continue;
            questPixels[py * QUEST_W + px] = c == 'h' || (c >= '1' && c <= '3') ? tier : paletteColor(c);
        }
    }
}

static void drawHero(const Quest& q, int camX, int camY) {
    const Hero& h = q.hero;
    HeroPose p = heroPose(q);
    int cls = q.save.heroClass, job = q.save.job >= 0 && q.save.job < 4 ? q.save.job : 0;
    const Sprite& sprite = p.arms == ARMS_NONE ? *HERO_BODY[cls][p.body] : *HERO_ARMS[cls][p.arms][p.legs];
    int x = (int)floorf(h.x - 12 * DP) - camX, y = (int)floorf(h.y + HERO_H - 28 * DP) - camY;   // feet on the hitbox bottom
    if (h.partnerTimer > 0) {   // the Shadow Partner: the hero's silhouette in x, 50 % dithered so the hero in front stays clear
        int px0 = x + (int)(h.facingLeft ? PARTNER_BEHIND : -PARTNER_BEHIND);
        for (int row = 0; row < sprite.h; row++)
            for (int col = 0; col < sprite.w; col++) {
                int px = px0 + (h.facingLeft ? sprite.w - 1 - col : col), py = y + row;
                if (sprite.pixels[row * sprite.w + col] != '.' && px >= 0 && px < QUEST_W && py >= 0 && py < QUEST_H && bayer4(px, py) < 8)
                    screenTarget[py * QUEST_W + px] = paletteColor('x');
            }
    }
    if (h.iframes > 0 && q.mode != QMODE_DEAD && (int)(h.iframes / IFRAME_BLINK) % 2) return;   // blinks during i-frames
    drawHeroSprite(sprite, x, y, h.facingLeft, job);
    if (h.frozenTimer > 0)   // the Yeti's ice block around the hero
        drawDoubled(ICE_CUBE, (int)(h.x + HERO_W / 2) - ICE_CUBE.w * DP / 2 - camX, (int)(h.y + HERO_H) - ICE_CUBE.h * DP - camY, false);
}

// A crescent filling the box: the outer half-ellipse centred on the box's side nearest the hero (sword arcs).
static void drawArc(int x, int y, int w, int h, bool flipped, uint32_t color) {
    for (int row = 0; row < h; row += DP)
        for (int col = 0; col < w; col += DP) {
            float nx = (col + DP / 2.0f) / w, ny = (row + DP / 2.0f) / h * 2 - 1;
            if (flipped) nx = 1 - nx;
            float d = nx * nx + ny * ny;
            if (d <= 1 && d >= 0.45f) fillRect(x + col, y + row, DP, DP, color);
        }
}

// A crescent in a box: each dp cell gets its 0..1 place across the band (0 inner edge, 1 outer edge) and the colour rule
// picks its colour (0: none). upright: bulging up and forward from the box's lower near corner, else forward from the
// near side; scale shrinks the crescent toward the hero; clip skips cells in solid tiles (Lion's Roar).
template <typename Rule>
static void drawCrescent(const Quest& q, int x, int y, int w, int h, bool flip, bool upright, float scale, bool clip, int camX, int camY,
                         Rule rule) {
    const float INNER = 0.67f;   // drawArc's inner edge (d >= 0.45)
    for (int row = 0; row < h; row += DP)
        for (int col = 0; col < w; col += DP) {
            float nx = (col + DP / 2.0f) / w, ny = (row + DP / 2.0f) / h * 2 - 1;
            if (flip) nx = 1 - nx;
            if (upright) ny = 1 - (row + DP / 2.0f) / h;
            float r = sqrtf(nx * nx + ny * ny) / scale;
            if (r > 1 || r < INNER) continue;
            int px = x + col, py = y + row;
            if (clip && tileSolid(tileAt(q.area, (px + camX) / TILE, (py + camY) / TILE))) continue;
            float across = (r - INNER) / (1 - INNER), radius = scale * (upright ? (w + h) / 2.0f : (w + h / 2.0f) / 2.0f);
            uint32_t color = rule(across, (1 - r) * radius, (r - INNER) * radius, px, py);
            if (color) fillRect(px, py, DP, DP, color);
        }
}

// The hero's melee slashes, drawn from heroPose (spec §22): the brightest frame is exactly the hit window.
static void drawHeroSlash(const Quest& q, const HeroPose& p, int camX, int camY) {
    const Hero& h = q.hero;
    int cls = q.save.heroClass, job = q.save.job >= 0 && q.save.job < 4 ? q.save.job : 0;
    bool left = h.facingLeft;
    uint32_t core = paletteColor(q.reduceFlashing ? 'b' : 'w'), edge = paletteColor(job == 0 ? 'b' : TRIM_BY_JOB[job]);
    float dir = left ? -1 : 1;
    for (int copy = 0; copy < (h.partnerTimer > 0 ? 2 : 1); copy++) {   // the Shadow Partner's copy 10 dp behind, in x and p
        uint32_t c = copy ? paletteColor('x') : core, e = copy ? paletteColor('p') : edge;
        int shift = copy ? (int)(-dir * PARTNER_BEHIND) : 0;
        bool recovering = p.arms >= ARMS_BASIC && (p.arms - ARMS_BASIC) % BASIC_STRIKE_FRAMES >= 6;
        if (cls == CLASS_KNIGHT && p.attackT >= 0) {
            int w = p.slashW, bh = p.slashH;
            int x = (int)(left ? h.x - w : h.x + HERO_W) - camX + shift, y = (int)(h.y + HERO_H / 2 - bh / 2) - camY;
            if (p.slashLit || (recovering && p.attackT <= 0.18f + 0.06f)) {
                float sweep = fmaxf(0, fminf(1, (p.attackT - 0.08f) / 0.10f));
                drawCrescent(q, x, y, w, bh, left, false, 1, true, camX, camY, [&](float, float out, float in, int px, int py) -> uint32_t {
                    float row = (py - y) / (float)bh;
                    float along = h.basicStep == 1 ? 1 - row : row;
                    if (h.basicStep != 0 && (along > sweep + 0.60f || along < sweep - 0.55f)) return 0;
                    if (!p.slashLit) return out < 2 * DP && bayer4(px, py) < 8 ? e : 0;
                    return out < 2 * DP ? e : in < 2 * DP ? c : bayer4(px, py) < 8 ? c : 0;
                });
                if (h.basicStep == 0 && p.attackT < 0.08f + 1.0f / 60) fillRect(x, y + bh / 2, w, DP, c);
            }
        }
        if (cls == CLASS_THIEF && p.attackT >= 0 && (p.slashLit || (recovering && p.attackT <= 0.10f + 0.04f))) {
            int length = p.slashW, from = p.slashLit ? 0 : length * 2 / 3;
            for (int k = from; k < length; k += DP) {
                int tall = 1 + (2 + 2 * job) * k / length;
                int tilt = h.basicStep == 0 ? 0 : (h.basicStep == 1 ? -1 : 1) * 4 * k / length;
                for (int t = -tall / 2; t <= tall / 2; t++) {
                    uint32_t color = t == 0 ? c : e;
                    if (h.attackUp)
                        fillRect((int)(h.x + HERO_W / 2) - camX + shift + (t + dir * tilt) * DP, (int)h.y - camY - k - DP, DP, DP, color);
                    else
                        fillRect((int)(left ? h.x - k - DP : h.x + HERO_W + k) - camX + shift,
                                 (int)(h.y + HERO_H / 2) - camY + (t + tilt) * DP, DP, DP, color);
                }
            }
        }
    }
    if (cls != CLASS_KNIGHT || h.skill < 0) return;
    float t = p.skillT;
    int sx = (int)(left ? h.x - 12 * DP : h.x - 12 * DP) - camX, sy = (int)(h.y + HERO_H - 28 * DP) - camY;
    auto art = [&](int col, int row, uint32_t color) {   // a point of the hero's 32 x 28 art grid, mirrored with the sprite
        fillRect(sx + (left ? 31 - col : col) * (int)DP, sy + row * (int)DP, DP, DP, color);
    };
    if (h.skill == 0) {   // Power Slash: a glint on the raised blade, the big crescent in its window, then its outer band fading
        int w = (int)(30 * DP), bh = (int)(20 * DP);
        int x = (int)(left ? h.x - w : h.x + HERO_W) - camX, y = (int)(h.y + HERO_H / 2 - bh / 2) - camY;
        if (t < 0.1f) {
            if ((int)(q.time * 20) % 2) art(16, 0, core), art(19, 2, core), art(14, 3, core);
        } else if (t <= 0.25f) {
            drawCrescent(q, x, y, w, bh, left, false, 1, false, camX, camY, [&](float, float out, float in, int px, int py) -> uint32_t {
                return out < 2 * DP ? edge : in < 3 * DP ? core : bayer4(px, py) < 8 ? core : 0;
            });
            for (int k = 0; k < 6; k++) {   // speed lines behind the crescent
                int len = (int)((6 + 3 * (k % 3)) * DP), ly = y + (int)((2 + k * 3) * DP);
                fillRect(left ? x + w : x - len, ly, len, DP, edge);
            }
        } else if (t <= 0.35f) {
            bool thin = t > 0.30f;
            drawCrescent(q, x, y, w, bh, left, false, 1, false, camX, camY, [&](float, float out, float, int px, int py) -> uint32_t {
                return out < 2 * DP && (thin ? ((px + py) / (int)DP) % 2 == 0 : bayer4(px, py) < 8) ? edge : 0;
            });
        }
    } else if (h.skill == 1 && t <= 0.15f + 0.08f) {   // Rising Slash: an upright crescent in the 24 x 36 box above-front
        int w = (int)(24 * DP), bh = (int)(36 * DP);
        int x = (int)(left ? h.x - w : h.x + HERO_W) - camX, y = (int)(h.y + HERO_H - bh) - camY;
        bool fading = t > 0.15f;
        drawCrescent(q, x, y, w, bh, left, true, 1, false, camX, camY, [&](float, float out, float in, int px, int py) -> uint32_t {
            if (fading) return out < 2 * DP && bayer4(px, py) < 8 ? edge : 0;
            return out < 2 * DP ? edge : in < 2 * DP ? core : bayer4(px, py) < 8 ? core : 0;
        });
    } else if (h.skill == 3 && t <= 0.15f + 0.1f) {   // Lion's Roar: three golden crescents growing in the 72 x 40 box, a mane
        int w = (int)(72 * DP), bh = (int)(40 * DP);
        int x = (int)(left ? h.x - w : h.x + HERO_W) - camX, y = (int)(h.y + HERO_H / 2 - bh / 2) - camY;
        bool fading = t > 0.15f;
        float grow = 0.4f + 0.6f * fminf(1, t / 0.15f);
        uint32_t gold[3] = {paletteColor('N'), paletteColor('Y'), paletteColor('o')};
        drawCrescent(q, x, y, w, bh, left, false, grow, true, camX, camY, [&](float across, float out, float, int px, int py) -> uint32_t {
            if (fading) return out < 2 * DP && bayer4(px, py) < 8 ? gold[0] : 0;
            for (int k = 0; k < 3; k++)   // bands at the outer edge, two thirds and one third across
                if (fabsf(across - (1 - k / 3.0f)) < 0.08f) return gold[k];
            return bayer4(px, py) < 8 ? gold[1] : 0;
        });
        if (!fading)
            for (int k = 0; k < 8; k++) {   // the mane: 8 short rays around the hero
                float a = k * 0.785f;
                for (int d = 6; d < 10; d++)
                    fillRect((int)(h.x + HERO_W / 2 + cosf(a) * d * DP) - camX, (int)(h.y + HERO_H / 2 + sinf(a) * d * DP) - camY, DP, DP, gold[0]);
            }
    }
}

// Promoted R1 hits open into a directional burst, with staggered arcs on the higher jobs.
static void drawBasicImpact(const Effect& f, int x, int y, uint32_t core) {
    float t = f.age / f.life, decay = 1 - t;
    float radius = 0.24f + 0.76f * (1 - decay * decay);
    uint32_t color = paletteColor(f.color);
    int rays = 6 + 2 * f.basicJob;
    for (int k = 0; k < rays; ++k) {
        float angle = (k + 0.25f) * 6.2831853f / rays;
        float along = cosf(angle) * (cosf(angle) < 0 ? f.w * 0.7f : f.w), across = sinf(angle) * f.h;
        float dx = along * f.x2 - across * f.y2, dy = along * f.y2 + across * f.x2;
        for (int j = 0; j < 4; ++j) {
            float r = radius - j * (0.035f + 0.055f * decay);
            int px = x + (int)(dx * r), py = y + (int)(dy * r);
            if (t < 0.58f || bayer4(k, j) < decay * 24)
                fillRect(px, py, DP, DP, j == 0 && t < 0.42f ? core : color);
        }
    }
    for (int ring = 1; ring < f.basicJob; ++ring) {
        float delay = (ring - 1) * 0.22f, ringT = t - delay;
        if (ringT < 0 || ringT >= 0.72f) continue;
        float r = 0.24f + 0.85f * ringT;
        for (int k = 0; k < 36; ++k) {
            if ((k + 2 * ring) % 9 >= 6) continue;
            float angle = k * 6.2831853f / 36;
            float along = f.w * r * cosf(angle), across = f.h * r * sinf(angle);
            int px = x + (int)(along * f.x2 - across * f.y2);
            int py = y + (int)(along * f.y2 + across * f.x2);
            if (ringT < 0.43f || bayer4(k, ring) < (0.72f - ringT) * 32)
                fillRect(px, py, DP, DP, color);
        }
    }
}

// Hero effects and shots first, then enemies and bosses, then enemy shots and telegraphs on top (spec §7, §8, §21; phase 5b).
static void drawCombat(const Quest& q, int camX, int camY) {
    const SaveData& s = q.save;
    if (s.echoNames > 0 && s.echoWorld == q.area.world && s.echoArea == q.area.index) {   // 8 x 8 wisp, bobbing 2 px
        int ex = (int)s.echoX - camX, ey = (int)(s.echoY + ECHO_BOB * sinf(q.time * 3)) - camY;
        fillRect(ex + 2 * DP, ey + 2 * DP, 4 * DP, 5 * DP, paletteColor('y'));
        fillRect(ex + DP, ey + 4 * DP, 6 * DP, 3 * DP, paletteColor('y'));
        fillRect(ex + 3 * DP, ey, 2 * DP, 2 * DP, paletteColor('y'));
        fillRect(ex + 3 * DP, ey + 4 * DP, 2 * DP, 2 * DP, paletteColor('w'));
    }
    for (const FloorHazard& z : q.hazards) {
        int zx = (int)z.x - camX, zy = (int)z.y - camY;
        if (z.kind == HAZARD_PEEL) {   // blinks in its last second
            if (z.timer > 1 || fmodf(z.timer, 0.2f) >= 0.1f) drawDoubled(PEEL, zx - 4 * DP, zy - 6 * DP, false);
            continue;
        }
        // The stink mist: a dithered sickly-green circle that shimmers and thins out in its last second (the shooter's).
        int r = (int)(20 * DP), shimmer = (int)(q.time * 4) % 2;
        bool fading = z.timer < 1;
        for (int dy = -r; dy <= r; dy += DP)
            for (int dx = -r; dx <= r; dx += DP) {
                int d2 = dx * dx + dy * dy, gx = dx / (int)DP, gy = dy / (int)DP;
                if (d2 > r * r) continue;
                bool edge = d2 > r * r * 0.55f;
                if ((edge || fading) ? ((gx + 2 * gy + shimmer) & 3) == 0 : ((gx + gy + shimmer) & 1) == 0)
                    fillRect(zx + dx, zy + dy, DP, DP, paletteColor('M'));
            }
    }
    const Hero& h = q.hero;
    const uint32_t white = paletteColor(q.reduceFlashing ? 'b' : 'w');   // the white cores and sparks (reduce flashing: grey)
    for (const Effect& f : q.effects) {
        if (f.kind == FX_BASIC_IMPACT) continue;
        int x = (int)f.x - camX, y = (int)f.y - camY;
        uint32_t color = paletteColor(f.color);
        switch (f.kind) {
        case FX_SPARK: {   // a hit spark: 3 frames over its life, centred, in the hit's colour; a Shadow Dash hit adds a slash line
            const Sprite& sp = f.age < f.life / 3 ? SPARK_1 : f.age < f.life * 2 / 3 ? SPARK_2 : SPARK_3;
            uint32_t c = f.color == 'w' ? white : color;
            drawSpriteEx(sp, x - sp.w / 2, y - sp.h / 2, false, 'w', c);
            if (f.w > 0) fillRect(x - (int)f.w / 2, y, (int)f.w, DP, white);
            break;
        }
        case FX_ZONE: {   // Arrow Rain's zone: dashes along the floor, blinking faster in its last 0.15 s
            if ((int)((f.life - f.age) * (f.life - f.age < 0.15f ? MARKER_BLINK_LAST : MARKER_BLINK)) % 2) break;
            for (int k = -(int)f.w / 2; k < (int)f.w / 2; k += 4 * DP) fillRect(x + k, y - DP, 2 * DP, DP, white);
            break;
        }
        case FX_GHOST: {   // a Shadow Dash afterimage: the hero's silhouette in x, 50 % dither fading to 25 %
            HeroPose pose = heroPose(q);
            const Sprite& sp = pose.arms == ARMS_NONE ? *HERO_BODY[q.save.heroClass][pose.body] : *HERO_ARMS[q.save.heroClass][pose.arms][pose.legs];
            int gx = (int)floorf(f.x - 12 * DP) - camX, gy = (int)floorf(f.y + HERO_H - 28 * DP) - camY;
            int level = f.age < f.life / 2 ? 8 : 4;
            for (int row = 0; row < sp.h; row++)
                for (int col = 0; col < sp.w; col++) {
                    int px = gx + (f.flip ? sp.w - 1 - col : col), py = gy + row;
                    if (sp.pixels[row * sp.w + col] != '.' && px >= 0 && px < QUEST_W && py >= 0 && py < QUEST_H && bayer4(px, py) < level)
                        screenTarget[py * QUEST_W + px] = color;
                }
            break;
        }
        case FX_BLINK_MIST: {
            const struct { int x, y, rx, ry; } wisps[] = {{-4, 5, 9, 4}, {0, -1, 11, 6}, {6, -7, 7, 4}};
            const uint32_t light = paletteColor('I');
            float t = f.age / f.life, drift = t * (2 - t), dir = f.flip ? -1 : 1;
            float density = fmaxf(0, 1 - t / 0.65f);
            if (density > 0) {
                for (const auto& w : wisps) {
                    int cx = x + (int)(dir * (w.x + 3 * drift) * DP);
                    int cy = y + (int)((w.y - 5 * drift) * DP);
                    int rx = (int)((w.rx + 3 * drift) * DP), ry = w.ry * DP;
                    for (int dy = -ry; dy <= ry; dy++)
                        for (int dx = -rx; dx <= rx; dx++) {
                            float edge = 1 - (float)(dx * dx) / (rx * rx) - (float)(dy * dy) / (ry * ry);
                            float coverage = 16 * density * edge;
                            if (coverage > bayer4(dx, dy))
                                fillRect(cx + dx, cy + dy, 1, 1, coverage > 9 && dy < 0 ? light : color);
                        }
                }
            }
            for (int i = 0; i < 14; i++) {
                if (t >= 0.58f + 0.03f * i) continue;
                int ox = i * 7 % 19 - 9, oy = i * 11 % 15 - 7;
                int px = x + (int)(dir * (ox * (1 + drift) + 4 * drift) * DP);
                int py = y + (int)((oy - (8 + i % 5) * drift) * DP);
                int size = t < 0.48f && i % 3 == 0 ? DP : 1;
                fillRect(px, py, size, size, i % 3 == 0 ? light : i % 3 == 1 ? color : paletteColor('c'));
            }
            break;
        }
        case FX_ARC: drawArc(x, y, (int)f.w, (int)f.h, f.flip, color); break;
        case FX_FROST_BURST:
        case FX_BURST: {   // a flash, then a ring growing to its reach (f.w)
            bool frost = f.kind == FX_FROST_BURST;
            float t = f.age / f.life;
            float flashEnd = frost ? 0.12f : 0.3f;
            if (t < flashEnd) {
                fillCircle(x, y, (int)(f.w * 0.4f), white);
                break;
            }
            int r = (int)(f.w * (0.4f + 0.6f * fminf(1, (t - flashEnd) / (0.7f - flashEnd))));
            for (int dy = -r; dy <= r; dy += DP)
                for (int dx = -r; dx <= r; dx += DP) {
                    int d2 = dx * dx + dy * dy, inner = r - 2 * (int)DP;
                    if (d2 > r * r) continue;
                    bool ring = d2 >= inner * inner;
                    if (ring && (t < 0.7f || ((dx + dy) / (int)DP) % 2 == 0)) {
                        fillRect(x + dx, y + dy, DP, DP, frost && d2 >= (r - DP) * (r - DP) && t < 0.7f ? white : color);
                    } else if (!ring && t < 0.7f) {
                        if (frost) {
                            if (bayer4(dx / DP, dy / DP) < 4) fillRect(x + dx, y + dy, 1, 1, color);
                        } else if (bayer4(x + dx, y + dy) < 8) {
                            fillRect(x + dx, y + dy, DP, DP, color);
                        }
                    }
                }
            break;
        }
        case FX_SPIN: {   // Blade Fury: a dithered ring at the hit radius around the hero, a bright quarter turning once per hit
            int job = q.save.job >= 0 && q.save.job < 4 ? q.save.job : 0;
            uint32_t ring = paletteColor(job == 0 ? 'b' : TRIM_BY_JOB[job]);
            int cx = (int)(h.x + HERO_W / 2) - camX, cy = (int)(h.y + HERO_H / 2) - camY, r = (int)f.w;
            bool pulse = fmodf(f.age, 0.1f) < 1.0f / 60;   // the 5 hit ticks, one every 0.1 s
            float turn = fmodf(f.age, 0.1f) / 0.1f * 6.2832f;
            for (int a = 0; a < 96; a++) {
                float angle = a * 6.2832f / 96;
                int px = cx + (int)(cosf(angle) * r), py = cy + (int)(sinf(angle) * r);
                float off = fmodf(angle - turn + 12.5664f, 6.2832f);
                if (off < 1.5708f) fillRect(px, py, DP, DP, white);
                else if (pulse || bayer4(px, py) < 8) fillRect(px, py, DP, DP, pulse ? white : ring);
            }
            break;
        }
        case FX_CHAIN: {   // Chain Lightning: a jagged line (white core, cyan sides), re-jagged every 0.05 s; a glow at the target
            int x2 = (int)f.x2 - camX, y2 = (int)f.y2 - camY, seed = (int)(q.time * 20);
            int steps = std::max(4, (int)(hypotf((float)(x2 - x), (float)(y2 - y)) / (4 * DP)));
            int lastX = x, lastY = y;
            for (int k = 1; k <= steps; k++) {
                int jag = k == steps ? 0 : (int)((((k * 7 + seed * 13) % 5) - 2) * DP);
                int nx = x + (x2 - x) * k / steps, ny = y + (y2 - y) * k / steps + jag;
                for (int i = 0; i <= 4; i++) {
                    int px = lastX + (nx - lastX) * i / 4, py = lastY + (ny - lastY) * i / 4;
                    fillRect(px, py - DP, DP, 3 * DP, paletteColor('c'));
                    fillRect(px, py, DP, DP, white);
                }
                lastX = nx, lastY = ny;
            }
            if (f.age < 2.0f / 60) fillCircle(x2, y2, 4 * DP, paletteColor('c'));
            break;
        }
        case FX_BEAM: {   // a boss's beam: magenta with a flickering white core
            fillRect(x, y, (int)f.w, (int)f.h, color);
            int core = (int)f.w / 3 + ((int)(q.time * 30) % 2) * (int)DP;
            fillRect(x + ((int)f.w - core) / 2, y, core, (int)f.h, paletteColor('w'));
            break;
        }
        case FX_BALL: {   // a growing ball of power at a wand's star
            int r = (int)(2 * DP + (f.w - 2 * DP) * f.age / f.life);
            fillCircle(x, y, r + (int)DP, color);
            fillCircle(x, y, r / 2 + (int)DP, paletteColor('w'));
            break;
        }
        case FX_METEOR: {   // the target: an ellipse on the floor blinking faster at the end; the rock falls onto it on a slant
            float left = f.life - f.age;
            if (!((int)(left * (left < 0.2f ? 12 : 4)) % 2))
                for (int a = 0; a < 64; a++) {
                    float angle = a * 6.2832f / 64;
                    fillRect(x + (int)(cosf(angle) * f.w), y - (int)(3 * DP) + (int)(sinf(angle) * 3 * DP), DP, DP, color);
                }
            float fall = 1 - f.age / f.life, dir = f.flip ? -1 : 1;
            int rx = x - (int)(dir * 30 * DP * fall), ry = y - (int)(120 * DP * fall);
            drawSpriteEx(METEOR_ROCK, rx - METEOR_ROCK.w / 2, ry - METEOR_ROCK.h, f.flip);
            break;
        }
        }
    }
    drawHeroSlash(q, heroPose(q), camX, camY);
    for (const Projectile& p : q.projectiles) {   // hero shots: sprites in their class colours, centred on the hitbox (D-063)
        if (!p.fromHero) continue;
        int cx = (int)(p.x + p.w / 2) - camX, cy = (int)(p.y + p.h / 2) - camY;
        bool flip = p.vx < 0, b = (int)(q.time / 0.08f) % 2;
        float speed = hypotf(p.vx, p.vy), ux = speed > 0 ? p.vx / speed : 0, uy = speed > 0 ? p.vy / speed : 0;
        const Sprite* sp = nullptr;
        if (p.kind == PROJ_ARROW) {
            bool big = p.strong && p.w >= 10 * DP;
            sp = big ? &ARROW_BIG : p.vy < -0.12f * fabsf(p.vx) ? &ARROW_UP : p.vy > 0.12f * fabsf(p.vx) ? &ARROW_DOWN : &ARROW_FLAT;
            for (int k = 0; k < 6 + 2 * p.basicJob && !big; k++) {
                float d = (sp->w + p.basicJob * DP) / 2 + k * DP;
                int px = cx - (int)(ux * d), py = cy - (int)(uy * d);
                if (bayer4(px, py) < 8) fillRect(px, py, DP, DP, p.basicJob > 0 ? paletteColor('g') : white);
            }
            const char trail[3] = {'w', 'c', 'I'};
            for (int k = 0; k < 3 && big; k++)   // the Piercing Bolt's glowing streak
                fillRect(cx - (int)(ux * (sp->w / 2 + (k + 1) * 4 * DP)), cy, 4 * DP, DP, paletteColor(trail[k]));
            if (p.strong && p.mult < 2) {   // Double Shot and Storm Volley arrows glint cyan
                drawSpriteEx(*sp, cx - sp->w / 2, cy - sp->h / 2, flip, 'w', paletteColor('c'));
                continue;
            }
        } else if (p.kind == PROJ_RAIN) {
            sp = &ARROW_FALL;
        } else if (p.kind == PROJ_BOLT || p.kind == PROJ_ORB) {   // a trail of 3 dots behind, at 3, 6 and 9 dp
            bool bolt = p.kind == PROJ_BOLT;
            sp = bolt ? (b ? &BOLT_B : &BOLT_A) : (b ? &ORB_B : &ORB_A);
            const char* colors = bolt ? "cuU" : "iIi";
            for (int k = 0; k < 3 + p.basicJob; k++) {
                int d = (int)((k + 1) * 3 * DP + (sp->w + p.basicJob * DP) / 2), size = k < 2 ? 2 * DP : DP;
                fillRect(cx - (int)(ux * d) - size / 2, cy - (int)(uy * d) - size / 2, size, size, paletteColor(colors[k % 3]));
            }
        } else if (p.kind == PROJ_STAR) {
            sp = b ? &STAR_B : &STAR_A;
        } else if (p.kind == PROJ_WAVE) {
            sp = b ? &QUAKE_B : &QUAKE_A;
            cy = (int)(p.y + p.h) - camY - sp->h / 2;   // the crest stands on the floor
        }
        if (!sp) continue;
        if (p.basicJob > 0) {
            int w = sp->w + p.basicJob * DP, h = sp->h + 2 * p.basicJob * DP;
            for (int row = 0; row < h; row += DP) {
                int sy = row * sp->h / h / (int)DP * (int)DP;
                for (int col = 0; col < w; col += DP) {
                    int sx = col * sp->w / w / (int)DP * (int)DP;
                    char ink = sp->pixels[sy * sp->w + (flip ? sp->w - 1 - sx : sx)];
                    if (ink != '.') fillRect(cx - w / 2 + col, cy - h / 2 + row, DP, DP, ink == 'w' ? white : paletteColor(ink));
                }
            }
        } else {
            drawSpriteEx(*sp, cx - sp->w / 2, cy - sp->h / 2, flip);
        }
    }
    for (const Enemy& e : q.enemies) {
        if (!e.alive) continue;
        const EnemyType& type = rosterType(e.world, e.type);
        // The tell and attack poses (§22) where the type has them, else the 4 (or 2) movement frames.
        const Sprite* pose = type.frames[(int)(q.time / ENEMY_FRAME) % (type.frames[2] ? 4 : 2)];
        bool telling = e.state == ES_TELL || (type.behaviour == BEH_JUMPER && e.onGround && e.stateTimer >= 1.0f);
        bool attacking = e.state == ES_ACT || (type.behaviour == BEH_SHOOTER && e.state == ES_PATROL && e.stateTimer < 0.2f) ||
                         (type.behaviour == BEH_JUMPER && !e.onGround);
        if ((telling || (type.behaviour == BEH_DROPPER && e.state == ES_HANG)) && type.tell) pose = type.tell;   // a dropper hangs in its tell pose
        else if (attacking && type.attack) pose = type.attack;
        const Sprite& sprite = *pose;
        int x = (int)floorf(e.x - 2 * DP) - camX, y = (int)floorf(e.y - 2 * DP) - camY;
        if (x + sprite.w < 0 || x >= QUEST_W || y + sprite.h < 0 || y >= QUEST_H) continue;
        if (e.state == ES_EMBER) {   // the phoenix's ember pulses before it revives
            int r = (int)((2 + 2 * fabsf(sinf(q.time * 8))) * DP);
            fillCircle(x + sprite.w / 2, y + sprite.h / 2, r, paletteColor('o'));
            fillCircle(x + sprite.w / 2, y + sprite.h / 2, r / 2, paletteColor('N'));
            continue;
        }
        bool tell = e.state == ES_TELL && (int)(e.stateTimer / TELL_FLASH) % 2 == 0;
        if (e.state == ES_TELL && type.behaviour == BEH_CHARGER) x += (int)(q.time * 60) % 2 ? DP : -DP;   // the charger shakes
        if (e.flash > 0 || (tell && type.behaviour != BEH_SHOOTER)) drawSpriteSolid(sprite, x, y, paletteColor('w'), e.facingLeft);
        else if (tell) drawSpriteSolid(sprite, x, y, paletteColor('o'), e.facingLeft);   // the shooter glows
        else drawSpriteOutlined(sprite, x, y, e.facingLeft, paletteColor(type.trait == TRAIT_INFLATE && e.state == ES_ACT ? 'w' : 'x'));
        for (int k = 0; k < 3 && e.slowTimer > 0; k++)   // slowed by the Frost Orb: frost on its top edge
            fillRect(x + sprite.w / 4 + k * sprite.w / 4 - DP / 2, y + 2 * DP, DP, DP, paletteColor('i'));
        if (type.behaviour == BEH_ARMORED && e.state == ES_ACT)   // the swing, 28 x 16 in front
            drawArc((int)(e.facingLeft ? e.x - 28 * DP : e.x + e.w) - camX, (int)(e.y + e.h - 16 * DP) - camY, (int)(28 * DP), (int)(16 * DP),
                    e.facingLeft, paletteColor('m'));
    }
    // Bosses: white on a hit, then the hurt frame; the phase-2 look while shifting and after; the Star Mage fades by dither.
    for (const QuestBoss& b : q.bosses) {
        if (!b.alive || b.id > QBOSS_TITAN) continue;
        int pose = bossPose(b), frame = b.phase >= 2 ? 5 : (int)(q.time / 0.4f) % 2;
        if (pose == POSE_TELL) frame = 2;
        else if (pose == POSE_ATTACK) frame = 3;
        if (b.shift > 0) frame = 5;
        if (b.flash > 0) frame = 4;
        bool sliding = b.id == QBOSS_YETI && b.attack == 1 && b.step == 1;   // the Yeti's BELLY SLIDE
        const Sprite& sprite = sliding && b.flash <= 0 ? BOSS_YETI_SLIDE : *BOSS_FRAMES[b.id][frame];
        int x = (int)floorf(b.x + b.w / 2.0f - sprite.w / 2.0f) - camX, y = (int)floorf(b.y + b.h + 2 * DP - sprite.h) - camY;
        if (b.id == QBOSS_YETI && b.attack == 0 && b.step == 0)   // SNEEZE's tell
            drawText(x + sprite.w / 2 - textWidth("AH... AH...", DP) / 2, y - 6 * DP, "AH... AH...", paletteColor('w'), DP);
        float shown = bossVisibility(b);
        if (b.flash > BOSS_WHITE) {
            drawSpriteSolid(sprite, x, y, paletteColor('w'), b.facingLeft);
        } else if (shown < 1) {
            for (int row = 0; row < sprite.h; row++)
                for (int col = 0; col < sprite.w; col++) {
                    char c = sprite.pixels[row * sprite.w + col];
                    int px = x + (b.facingLeft ? sprite.w - 1 - col : col), py = y + row;
                    if (c != '.' && px >= 0 && px < QUEST_W && py >= 0 && py < QUEST_H && bayer4(px, py) < shown * 16)
                        screenTarget[py * QUEST_W + px] = paletteColor(c);
                }
        } else {
            drawSpriteEx(sprite, x, y, b.facingLeft);
        }
        for (int k = 0; k < 3 && b.slowTimer > 0 && shown >= 1; k++)
            fillRect(x + (k + 1) * sprite.w / 4, y - (k % 2 + 1) * DP, DP, DP, paletteColor('i'));
    }
    for (const Effect& f : q.effects) {
        if (f.kind == FX_BASIC_IMPACT) drawBasicImpact(f, (int)f.x - camX, (int)f.y - camY, white);
    }
    for (const Projectile& p : q.projectiles) {   // enemy shots, over everything but the hero
        if (p.fromHero) continue;
        int x = (int)p.x - camX, y = (int)p.y - camY;
        if ((p.kind == PROJ_BANANA || p.kind == PROJ_COCONUT || p.kind == PROJ_PEEL || p.kind == PROJ_POOP)) {
            const Sprite& sprite = p.kind == PROJ_BANANA ? ((int)(q.time * 8) % 2 ? BANANA_B : BANANA_A)
                                 : p.kind == PROJ_COCONUT ? COCONUT : p.kind == PROJ_PEEL ? PEEL : POOP;
            drawDoubled(sprite, x + p.w / 2 - sprite.w * DP / 2, y + p.h / 2 - sprite.h * DP / 2, p.vx < 0);
        } else if ((p.kind == PROJ_ICICLE || p.kind == PROJ_SNOWBALL || p.kind == PROJ_ICE_CUBE || p.kind == PROJ_SHARD)) {
            const Sprite& sprite = p.kind == PROJ_ICICLE ? ICICLE : p.kind == PROJ_SNOWBALL ? SNOWBALL : p.kind == PROJ_ICE_CUBE ? ICE_CUBE : ICE_SHARD;
            drawDoubled(sprite, x + p.w / 2 - sprite.w * DP / 2, y + p.h / 2 - sprite.h * DP / 2, p.kind == PROJ_SHARD && p.vx < 0);
        } else if (p.kind == PROJ_WAVE) {   // a boss's shockwave along the floor: a magenta crest
            for (int k = 0; k < p.w; k += DP) {
                int hgt = p.h * (k < p.w / 2 ? k + DP : p.w - k) / (p.w / 2);
                fillRect(x + k, y + p.h - hgt, DP, hgt, paletteColor('m'));
            }
            fillRect(x + p.w / 4, y + p.h - 2 * DP, p.w / 2, 2 * DP, paletteColor('w'));
        } else {
            fillRect(x, y, p.w, p.h, paletteColor('m'));
        }
    }
    // Telegraph markers: dashed outlines in their colour, blinking faster in their last 0.3 s.
    for (const Marker& m : q.markers) {
        if ((int)(m.timer * (m.timer < MARKER_LAST ? MARKER_BLINK_LAST : MARKER_BLINK)) % 2) continue;
        int x = (int)m.x - camX, y = (int)m.y - camY, w = (int)m.w, h = (int)m.h;
        uint32_t color = paletteColor(m.color);
        for (int k = 0; k < w; k += 4 * DP) {
            fillRect(x + k, y, std::min(2 * (int)DP, w - k), DP, color);
            fillRect(x + k, y + h - DP, std::min(2 * (int)DP, w - k), DP, color);
        }
        for (int k = 0; k < h; k += 4 * DP) {
            fillRect(x, y + k, DP, std::min(2 * (int)DP, h - k), color);
            fillRect(x + w - DP, y + k, DP, std::min(2 * (int)DP, h - k), color);
        }
    }
}

// Particles and floating numbers: damage white, crits orange, the hero's red, "+n NAMES" pale yellow.
static void drawNumbers(const Quest& q, int camX, int camY) {
    for (const Particle& p : q.particles) fillRect((int)p.x - camX, (int)p.y - camY, DP, DP * p.h, p.color);
    char text[24];
    for (const DamageNumber& n : q.numbers) {
        if (n.names) snprintf(text, sizeof text, "+%d NAMES", n.value);
        else snprintf(text, sizeof text, "%d", n.value);
        drawText((int)n.x - camX - textWidth(text, DP) / 2, (int)(n.y - 6 * DP - NUMBER_RISE * n.age / n.life) - camY, text, n.color, DP);
    }
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
    if (q.gateTimer > 0) {   // the opened gate's tiles shake, then vanish top to bottom (spec §11)
        int top = def.rows, bottomRow = 0;
        for (int t = 0; t < def.cols * def.rows; t++)
            if (def.tiles[t] == 'g') top = std::min(top, t / def.cols), bottomRow = std::max(bottomRow, t / def.cols);
        float vanished = q.gateTimer < GATE_SHAKE ? (GATE_SHAKE - q.gateTimer) / GATE_SHAKE : 0;
        int shake = q.gateTimer >= GATE_SHAKE ? ((int)(q.time * 40) % 2 ? DP : -DP) : 0;
        for (int t = 0; t < def.cols * def.rows; t++) {
            int tx = t % def.cols, ty = t / def.cols;
            if (def.tiles[t] != 'g' || ty < top + vanished * (bottomRow - top + 1)) continue;
            if (art && art->gate) drawSpritePal(*art->gate, tx * TILE - camX + shake, ty * TILE - camY, false, art->palette);
            else drawTileFlat(q, 'g', tx, ty, tx * TILE - camX + shake, ty * TILE - camY);
        }
    }
    int floorCol0, floorRow, floorState = electricFloor(q, &floorCol0, &floorRow);
    for (int k = 0; k < 6 && floorState > 0; k++) {   // the Star Mage's electrified floor: panels on its top, arcs when on
        int sx = (floorCol0 + k) * TILE - camX, sy = floorRow * TILE - camY;
        bool lit = floorState == 2 || (int)(q.time * 20) % 2;
        fillRect(sx, sy, TILE, 2 * DP, paletteColor('d'));
        fillRect(sx + DP, sy, TILE - 2 * DP, DP, paletteColor(lit ? 'c' : 'n'));
        for (int a = 0; a < 2 && floorState == 2; a++) {
            int beat = (int)(q.time * 15) + k * 5 + a * 3;
            for (int y = 1; y <= 6; y++) fillRect(sx + (2 + (beat + y * (a + 1)) % 5) * DP, sy - y * DP, DP, DP, paletteColor(y % 2 ? 'c' : 'w'));
        }
    }
    for (int col = 0, row; col < q.area.def->cols; col++) {   // the Titan's SPIKE LINE: a crack running ahead, then a spike
        int state = titanSpikeLine(q, col, &row), sx = col * TILE - camX, sy = row * TILE - camY;
        if (state == 1) for (int k = 0; k < 4; k++) fillRect(sx + k * 2 * DP, sy + (k % 2) * DP, 2 * DP, DP, paletteColor('c'));
        for (int k = 0; state == 2 && k < (int)(16 * DP); k += DP) {
            int w = (int)(TILE * (16 * DP - k) / (16 * DP));
            fillRect(sx + (TILE - w) / 2, sy - k - DP, w, DP, paletteColor(k < 2 * DP ? 'I' : 'i'));
            fillRect(sx + (TILE - w) / 2 - DP, sy - k - DP, DP, DP, paletteColor('m'));
            fillRect(sx + (TILE + w) / 2, sy - k - DP, DP, DP, paletteColor('m'));
        }
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
            if (strchr("CPTKNMRYbp", letter)) drawObject(q, letter, tx, ty, camX, camY, lights);
        }

    drawCombat(q, camX, camY);
    drawHero(q, camX, camY);
    bool snowing = false;   // the Yeti's phase 2 (cosmetic snow) and the Titan's blizzard: flakes over the whole screen
    for (const QuestBoss& b : q.bosses) snowing |= b.alive && b.id == QBOSS_YETI && b.phase >= 2;
    for (int i = 0; (snowing || q.area.wind != 0) && i < 48; i++) {
        float drift = q.area.wind != 0 ? q.area.wind * 3 : 10 * sinf(q.time + i);
        int x = (((int)(i * 53 + fmodf(q.time * drift, (float)QUEST_W)) % QUEST_W) + QUEST_W) % QUEST_W, y = (int)(i * 37 + q.time * (30 + i % 5 * 8) * DP) % QUEST_H;
        fillRect(x, y, q.area.wind != 0 ? 3 * DP : DP, DP, paletteColor(i % 3 ? 'w' : 'i'));
    }
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
    drawNumbers(q, camX, camY);
    if (q.popupTimer > 0 && q.popupTimer <= POPUP_SHOWN) {   // the chest's contents, once the lid is up
        int y = (int)(q.popupY - POPUP_RISE * (1 - q.popupTimer / POPUP_SHOWN)) - camY;
        drawText((int)q.popupX - camX - textWidth(q.popup.c_str(), DP) / 2, y, q.popup.c_str(), paletteColor('w'), DP);
    }
    if (q.arenaLocked) {   // closed exits: a grey shimmering wall 4 dp thick at every edge that has a link
        uint32_t color = paletteColor((int)(q.time * 8) % 2 ? 's' : 'b');
        int w = def.cols * TILE, h = def.rows * TILE;
        if (def.left.area >= 0) fillRect(-camX, -camY, (int)EDGE_WALL, h, color);
        if (def.right.area >= 0) fillRect(w - (int)EDGE_WALL - camX, -camY, (int)EDGE_WALL, h, color);
        if (def.up.area >= 0) fillRect(-camX, -camY, w, (int)EDGE_WALL, color);
        if (def.down.area >= 0) fillRect(-camX, h - (int)EDGE_WALL - camY, w, (int)EDGE_WALL, color);
    }
    shakePicture(q);
    if (q.area.world != WORLD_SECRET) applyVignette();
    if (questClock) {
        double t4 = questClock();
        questDrawMs[0] = std::max(questDrawMs[0], (float)((t1 - t0) * 1000));
        questDrawMs[1] = std::max(questDrawMs[1], (float)((t2 - t1) * 1000));
        questDrawMs[2] = std::max(questDrawMs[2], (float)((t4 - t3) * 1000));
    }

    // HUD (spec §15): level, HP and MP bars, HP numbers, names with the wisp icon; then the banner.
    char text[32];
    const Hero& h = q.hero;
    snprintf(text, sizeof text, "LV %d", q.save.level);
    drawText(4 * DP, 4 * DP, text, paletteColor('w'), DP);
    fillRect(4 * DP, 11 * DP, 64 * DP, 4 * DP, paletteColor('R'));
    fillRect(4 * DP, 11 * DP, 64 * DP * std::max(0, h.hp) / std::max(1, q.stats.maxHp), 4 * DP, paletteColor('r'));
    snprintf(text, sizeof text, "HP %d/%d", std::max(0, h.hp), q.stats.maxHp);
    drawText(72 * DP, 10 * DP, text, paletteColor('s'), DP);
    fillRect(4 * DP, 17 * DP, 64 * DP, 3 * DP, paletteColor('U'));
    fillRect(4 * DP, 17 * DP, 64 * DP * std::max(0, h.mp) / std::max(1, q.stats.maxMp), 3 * DP, paletteColor('u'));
    snprintf(text, sizeof text, "%d", q.save.names);
    int namesX = 316 * DP - textWidth(text, DP);
    drawText(namesX, 4 * DP, text, paletteColor('w'), DP);
    fillRect(namesX - 6 * DP + DP, 4 * DP, 3 * DP, 5 * DP, paletteColor('y'));   // the 5 x 5 wisp icon
    fillRect(namesX - 6 * DP, 6 * DP, 5 * DP, 3 * DP, paletteColor('y'));
    fillRect(namesX - 6 * DP + 2 * DP, 6 * DP, DP, 2 * DP, paletteColor('w'));
    if (q.save.job > 0) {
        const char* skill = SKILL_NAMES[q.save.heroClass][selectedSkillTier(q)];
        snprintf(text, sizeof text, "R2 %s", skill);
        drawText(316 * DP - textWidth(text, DP), 12 * DP, text, paletteColor('w'), DP);
        const char* hint = "R3 SWITCH";
        drawText(316 * DP - textWidth(hint, DP), 20 * DP, hint, paletteColor('s'), DP);
    }
    // The selected item and its count, then the running buffs' seconds (spec §15).
    const Sprite& icon = *ITEM_ICONS[q.selectedItem % ITEM_COUNT];
    drawSpriteOutlined(icon, 4 * DP, 168 * DP, false, paletteColor('x'));
    snprintf(text, sizeof text, "X%d", q.save.items[q.selectedItem % ITEM_COUNT]);
    drawText(14 * DP, 170 * DP, text, paletteColor('w'), DP);
    int buffX = 30 * DP;
    const char* const buffNames[3] = {"ATK", "DEF", "SPD"};
    for (int k = 0; k < 3; k++) {
        if (h.buffTimers[k] <= 0) continue;
        snprintf(text, sizeof text, "%s %d", buffNames[k], (int)ceilf(h.buffTimers[k]));
        drawText(buffX, 170 * DP, text, paletteColor('o'), DP);
        buffX += textWidth(text, DP) + 6 * DP;
    }
    for (const QuestBoss& b : q.bosses) {   // the boss bar (spec §15): name, then 160 x 4 at (80, 170)
        if (!b.alive) continue;
        drawCentred(162 * DP, bossName(b.id), paletteColor('w'));
        fillRect(80 * DP, 170 * DP, 160 * DP, 4 * DP, paletteColor('R'));
        fillRect(80 * DP, 170 * DP, (int)(160 * DP) * std::max(0, b.hp) / std::max(1, b.maxHp), 4 * DP, paletteColor('r'));
        break;
    }
    if (q.bannerTimer > 0 && q.banner) {
        char color = q.bannerColor ? q.bannerColor : q.bannerScale == 2 ? 'w' : 'b';
        drawCentred((q.bannerScale == 2 ? 40 : 30) * DP, q.banner, paletteColor(color), q.bannerScale);
    }
    if (q.fadeTimer > 0) {   // the rest's white frame, growing in from the edges
        int depth = (int)(FADE_DEPTH * (1 - q.fadeTimer / REST_FADE));
        fillRect(0, 0, QUEST_W, depth, paletteColor('w'));
        fillRect(0, QUEST_H - depth, QUEST_W, depth, paletteColor('w'));
        fillRect(0, 0, depth, QUEST_H, paletteColor('w'));
        fillRect(QUEST_W - depth, 0, depth, QUEST_H, paletteColor('w'));
    }
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
        const Sprite& hero = *HERO_BODY[i][BODY_IDLE_1];
        drawHeroSprite(hero, x + 36 * DP - hero.w / 2, y - 2 * DP, false, 0);
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

// The dialogue box (spec §14, §15): name, up to 4 typed lines, a blinking > when the page is complete.
static void drawDialogue(const Quest& q) {
    const DialogueState& d = q.dialogue;
    drawPanel(10 * DP, 124 * DP, 300 * DP, 52 * DP, 'n');
    drawText(16 * DP, 128 * DP, d.speaker.c_str(), paletteColor('y'), DP);
    const std::string& page = d.pages[d.page];
    int shown = (int)d.typed, line = 0;
    size_t from = 0;
    while (from <= page.size() && line < 4 && (int)from < shown) {
        size_t end = page.find('\n', from);
        if (end == std::string::npos) end = page.size();
        std::string text = page.substr(from, std::min(end, (size_t)shown) - from);
        drawText(16 * DP, (138 + 9 * line) * DP, text.c_str(), paletteColor('w'), DP);
        from = end + 1;
        line++;
    }
    if (shown >= (int)page.size() && (int)(q.time * 2) % 2) drawText(300 * DP, 168 * DP, ">", paletteColor('w'), DP);
}

// Ilsa's level up panel (spec §15): the level, cost, names and the stats before and after.
static void drawLevelUp(const Quest& q) {
    const SaveData& s = q.save;
    int x = 60 * DP, y = 30 * DP, w = 200 * DP;
    drawPanel(x, y, w, 120 * DP, 'n');
    char text[48];
    bool atMax = s.level >= MAX_LEVEL;
    int cost = levelCost(s.level);
    if (atMax) snprintf(text, sizeof text, "LEVEL %d", s.level);
    else snprintf(text, sizeof text, "LEVEL %d > %d", s.level, s.level + 1);
    drawText(x + (w - textWidth(text, 2 * DP)) / 2, y + 6 * DP, text, paletteColor('w'), 2 * DP);
    if (atMax) snprintf(text, sizeof text, "MAX LEVEL");
    else snprintf(text, sizeof text, "COST %d", cost);
    drawText(x + 12 * DP, y + 22 * DP, text, paletteColor(!atMax && s.names >= cost ? 'w' : 'r'), DP);
    snprintf(text, sizeof text, "NAMES %d", s.names);
    drawText(x + w - 12 * DP - textWidth(text, DP), y + 22 * DP, text, paletteColor('w'), DP);
    SaveData next = s;
    next.level = atMax ? s.level : s.level + 1;
    HeroStats a = heroStats(s), b = heroStats(next);
    const char* const names[4] = {"HP", "MP", "ATK", "DEF"};
    int before[4] = {a.maxHp, a.maxMp, a.atk, a.def}, after[4] = {b.maxHp, b.maxMp, b.atk, b.def};
    for (int k = 0; k < 4; k++) {
        snprintf(text, sizeof text, "%s %d > %d", names[k], before[k], after[k]);
        drawText(x + 12 * DP + (k % 2) * 92 * DP, y + (36 + 10 * (k / 2)) * DP, text, paletteColor('b'), DP);
    }
    const char* const items[2] = {"LEVEL UP", "LEAVE"};
    for (int i = 0; i < 2; i++) {
        bool grey = i == 0 && (atMax || s.names < cost);
        uint32_t color = paletteColor(i == q.menu.cursor ? 'y' : grey ? 's' : 'w');
        int ty = y + (66 + 12 * i) * DP, tx = x + (w - textWidth(items[i], DP)) / 2;
        drawText(tx, ty, items[i], color, DP);
        if (i == q.menu.cursor) drawText(tx - 8 * DP, ty, ">", color, DP);
    }
    drawText(x + (w - textWidth("X CHOOSE  O LEAVE", DP)) / 2, y + 106 * DP, "X CHOOSE  O LEAVE", paletteColor('s'), DP);
}

// A shop (spec §15): 240 x 130 panel, rows `RED POTION  15  2/5`, grey when they can't be bought.
static void drawShop(const Quest& q) {
    int x = 40 * DP, y = 24 * DP, w = 240 * DP;
    drawPanel(x, y, w, 130 * DP, 'n');
    const char* title = q.shopKind == AFTER_POTION_SHOP ? "POTIONS" : "THE FORGE";
    drawText(x + (w - textWidth(title, 2 * DP)) / 2, y + 6 * DP, title, paletteColor('w'), 2 * DP);
    char text[32];
    snprintf(text, sizeof text, "NAMES %d", q.save.names);
    drawText(x + w - 8 * DP - textWidth(text, DP), y + 22 * DP, text, paletteColor('w'), DP);
    std::vector<ShopRow> rows = shopRows(q.save, q.shopKind);
    if (rows.empty()) drawText(x + 16 * DP, y + 36 * DP, "NOTHING NEW TO FORGE YET.", paletteColor('s'), DP);
    for (int i = 0; i < (int)rows.size(); i++) {
        const ShopRow& r = rows[i];
        bool selected = i == q.shopCursor;
        uint32_t color = paletteColor(!r.canBuy ? 's' : selected ? 'y' : 'w');
        int ry = y + (36 + 12 * i) * DP;
        if (selected) drawText(x + 8 * DP, ry, ">", paletteColor(r.canBuy ? 'y' : 's'), DP);
        drawText(x + 16 * DP, ry, r.name.c_str(), color, DP);
        snprintf(text, sizeof text, "%d", r.price);
        drawText(x + 150 * DP - textWidth(text, DP), ry, text, color, DP);
        drawText(x + w - 8 * DP - textWidth(r.detail.c_str(), DP), ry, r.detail.c_str(), color, DP);
    }
    drawText(x + (w - textWidth("X BUY  O LEAVE", DP)) / 2, y + 118 * DP, "X BUY  O LEAVE", paletteColor('s'), DP);
}

// Travel previews use baked thumbnails so browsing keeps the current area’s parallax layers warm.
static void drawTravelMenu(const Quest& q) {
    const MenuState& m = q.menu;
    bool worlds = m.kind == MENU_TRAVEL_WORLDS;
    int world = worlds ? m.travelIds[m.cursor] : m.travelWorld;
    const WorldDef& w = worldDef(world);
    int rows = std::min(MENU_ROWS, (int)m.items.size());
    int width = 164 * DP, height = (54 + rows * 11) * DP;
    int x = (QUEST_W - width) / 2, top = (QUEST_H - height) / 2;
    drawPanel(x, top, width, height, 'n');

    const Sprite& preview = TRAVEL_PREVIEWS[world];
    int px = x + width - 54 * DP, py = top + 5 * DP;
    if (preview.pixels) {
        drawSpritePal(preview, px, py, false, worldArt(world)->palette);
    } else {   // Test Grounds uses the same grid as its playfield.
        for (int col = 0; col < 48; col += 8) fillRect(px + col * DP, py, DP, 20 * DP, paletteColor('n'));
        for (int row = 0; row < 20; row += 8) fillRect(px, py + row * DP, 48 * DP, DP, paletteColor('n'));
    }
    char region[24];
    if (world == WORLD_NAMULA) snprintf(region, sizeof region, "HOME");
    else if (world == WORLD_TEST) snprintf(region, sizeof region, "TEST AREA");
    else snprintf(region, sizeof region, "WORLD %d", world);
    drawText(x + 7 * DP, top + 7 * DP, region, paletteColor('b'), DP);
    drawText(x + 7 * DP, top + 17 * DP, w.name, paletteColor('w'), DP);
    fillRect(x + DP, top + 28 * DP, width - 2 * DP, DP, paletteColor('n'));
    drawText(x + 7 * DP, top + 33 * DP, worlds ? "CHOOSE WORLD" : "CHOOSE CHECKPOINT", paletteColor('b'), DP);
    for (int row = 0; row < rows; row++) {
        int index = m.scroll + row, y = top + (44 + row * 11) * DP;
        bool selected = index == m.cursor;
        if (selected) fillRect(x + 2 * DP, y - 2 * DP, width - 4 * DP, 10 * DP, paletteColor('n'));
        uint32_t color = paletteColor(selected ? 'y' : 'w');
        if (selected) drawText(x + 7 * DP, y, ">", color, DP);
        drawText(x + 16 * DP, y, m.items[index].c_str(), color, DP);
        bool here = worlds ? m.travelIds[index] == q.area.world : world == q.area.world && m.travelIds[index] == q.atCheckpoint;
        if (here) drawText(x + width - 23 * DP, y, "HERE", paletteColor('b'), DP);
    }
    drawText(x + 7 * DP, top + height - 10 * DP, worlds ? "X CHOOSE   O BACK" : "X TRAVEL   O BACK", paletteColor('b'), DP);
}

// The quest menus: plain lists through drawMenu (8 rows, scrolling), Ilsa's panel for levels.
static void drawQuestMenu(const Quest& q) {
    const MenuState& m = q.menu;
    if (m.kind == MENU_LEVEL_UP) {
        drawLevelUp(q);
        return;
    }
    std::string title = "ANOTHER ROUND?";
    if (m.kind == MENU_CHECKPOINT) {
        if (q.area.world == WORLD_NAMULA) title = "HEARTH OF NAMES";
        else if (q.atCheckpoint == BOSS_CHECKPOINT) title = q.area.def->name;
        else title = std::string(worldDef(q.area.world).name) + (q.atCheckpoint == 0 ? " ENTRY" : " DEPTHS");
    }
    if (m.kind == MENU_JOB_CONFIRM) {
        int cls = q.save.heroClass % CLASS_COUNT, tier = std::min(3, q.save.job + 1);
        title = std::string("BECOME ") + JOB_NAMES[cls][tier] + "?";
    }
    switch (m.kind) {
    case MENU_DEV: {
        char text[32];
        snprintf(text, sizeof text, "DEV TESTER - LV %d", q.save.level);
        title = text;
        break;
    }
    case MENU_DEV_CLASS: title = "CHOOSE CLASS"; break;
    case MENU_DEV_JOB: title = "CHOOSE YOUR JOB"; break;
    case MENU_DEV_WORLD: title = "WARP TO WORLD"; break;
    case MENU_DEV_AREA: title = worldDef(m.warpWorld).name; break;
    }
    std::vector<const char*> items;
    for (int i = m.scroll; i < (int)m.items.size() && i < m.scroll + MENU_ROWS; i++) items.push_back(m.items[i].c_str());
    drawMenu(title.c_str(), items.data(), (int)items.size(), m.cursor - m.scroll, "X CHOOSE  O BACK");
}

static uint32_t darker(uint32_t c) {
    return 0xFF000000 | ((c >> 1) & 0x7F7F7F);
}

// The world map (spec §11): visited areas filled, the rest outlined, the current area blinking, lit checkpoints, closed gates.
static void drawMap(const Quest& q) {
    int world = q.area.world;
    const WorldDef& w = worldDef(world);
    fillRect(0, 0, QUEST_W, QUEST_H, paletteColor('n'));
    fillRect(DP, DP, QUEST_W - 2 * DP, QUEST_H - 2 * DP, paletteColor('k'));
    drawCentred(8 * DP, w.name, paletteColor('w'));
    int cols = 1, rows = 1;
    for (int i = 0; i < w.count; i++) {
        bool wide = w.areas[i].cols == WIDE_COLS;
        cols = std::max(cols, w.areas[i].mapX + (wide ? 2 : 1));
        rows = std::max(rows, w.areas[i].mapY + (wide ? 1 : 2));
    }
    int ox = (QUEST_W - cols * MAP_CELL_W) / 2, oy = (QUEST_H - rows * MAP_CELL_H) / 2;
    uint32_t fill = paletteColor(w.style.mapColor);
    for (int i = 0; i < w.count; i++) {
        const AreaDef& a = w.areas[i];
        bool wide = a.cols == WIDE_COLS, visited = q.save.visited[world] >> i & 1;
        int x = ox + a.mapX * MAP_CELL_W, y = oy + a.mapY * MAP_CELL_H;
        int cw = (wide ? 2 : 1) * MAP_CELL_W, ch = (wide ? 1 : 2) * MAP_CELL_H;
        bool blink = i == q.area.index && (int)(q.time * 4) % 2;   // 2 Hz
        uint32_t inside = blink ? paletteColor('y') : visited ? fill : paletteColor('k');
        fillRect(x, y, cw, ch, visited ? darker(fill) : paletteColor('d'));
        fillRect(x + DP, y + DP, cw - 2 * DP, ch - 2 * DP, inside);
        // Edge links: 2 px gaps in the border, in the middle of the side shared with the neighbour.
        const AreaLink* sides[4] = {&a.left, &a.right, &a.up, &a.down};
        for (int k = 0; k < 4; k++) {
            if (sides[k]->area < 0) continue;
            const AreaDef& b = w.areas[sides[k]->area];
            bool bWide = b.cols == WIDE_COLS;
            int bx = ox + b.mapX * MAP_CELL_W, by = oy + b.mapY * MAP_CELL_H;
            int bw = (bWide ? 2 : 1) * MAP_CELL_W, bh = (bWide ? 1 : 2) * MAP_CELL_H;
            if (k < 2) {
                int mid = (std::max(y, by) + std::min(y + ch, by + bh)) / 2;
                fillRect(k == 0 ? x : x + cw - DP, mid - DP, DP, 2 * DP, inside);
            } else {
                int mid = (std::max(x, bx) + std::min(x + cw, bx + bw)) / 2;
                fillRect(mid - DP, k == 2 ? y : y + ch - DP, 2 * DP, DP, inside);
            }
        }
        bool gateDrawn = false;
        for (int t = 0; t < a.cols * a.rows; t++) {
            int px = x + (t % a.cols) * cw / a.cols, py = y + (t / a.cols) * ch / a.rows;
            if (a.tiles[t] == 'C' || a.tiles[t] == 'b') {
                int id = a.tiles[t] == 'b' ? BOSS_CHECKPOINT : objectIndex(world, i, t, 'C');
                if (checkpointLit(q.save, world, id)) fillRect(px - DP, py - DP, 2 * DP, 2 * DP, paletteColor('o'));
            }
            if (a.tiles[t] == 'g' && !(q.save.gates >> world & 1) && !gateDrawn) {   // one bar per gate, on its nearer edge
                gateDrawn = true;
                int edge = t % a.cols < a.cols / 2 ? x : x + cw - DP;
                fillRect(edge, y + ch / 2 - 3 * DP, DP, 6 * DP, paletteColor('r'));
            }
        }
    }
    drawCentred(172 * DP, "X / O CLOSE", paletteColor('s'));
}

void questDraw(const Quest& q) {
    switch (q.mode) {
    case QMODE_SLOTS:
    case QMODE_DELETE: drawSlots(q); break;
    case QMODE_CLASS: drawClassSelect(q); break;
    case QMODE_INTRO: drawIntro(q); break;
    case QMODE_PLAY:
    case QMODE_DEAD: drawPlay(q); break;
    case QMODE_DIALOGUE: drawPlay(q); drawDialogue(q); break;
    case QMODE_MENU:
        drawPlay(q);
        if (q.menu.kind == MENU_TRAVEL_WORLDS || q.menu.kind == MENU_TRAVEL_CHECKPOINTS) drawTravelMenu(q);
        else drawQuestMenu(q);
        break;
    case QMODE_SHOP: drawPlay(q); drawShop(q); break;
    case QMODE_MAP: drawMap(q); break;
    case QMODE_CREDITS: break;   // phase 8
    }
}
