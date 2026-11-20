// NAME QUEST drawing: the slot, class and intro screens, and play: background, tiles, objects, hero, HUD (spec §13, §15, §21).
#include "quest.h"
#include "quest_sprites.h"
#include "screen.h"
#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const float SHAKE_X = 4, SHAKE_Y = 3;   // px at trauma 1 and full strength, like the shooter
const float WALK_FRAME = 0.15f, IDLE_FRAME = 0.5f, CLIMB_FRAME = 0.2f, SHIMMER_FRAME = 0.25f, FLAME_FRAME = 0.15f;
const char TRIM_BY_JOB[4] = {'h', 'b', 'Y', 'N'};
enum HeroFrame { FRAME_IDLE_A, FRAME_IDLE_B, FRAME_WALK_A, FRAME_WALK_B, FRAME_JUMP, FRAME_ATTACK, FRAME_CLIMB, FRAME_ROLL };
static const Sprite* const HERO_FRAMES[CLASS_COUNT][8] = {
    {&HERO_KNIGHT_IDLE_A, &HERO_KNIGHT_IDLE_B, &HERO_KNIGHT_WALK_A, &HERO_KNIGHT_WALK_B, &HERO_KNIGHT_JUMP, &HERO_KNIGHT_ATTACK,
     &HERO_KNIGHT_CLIMB, &HERO_KNIGHT_ROLL},
    {&HERO_ARCHER_IDLE_A, &HERO_ARCHER_IDLE_B, &HERO_ARCHER_WALK_A, &HERO_ARCHER_WALK_B, &HERO_ARCHER_JUMP, &HERO_ARCHER_ATTACK,
     &HERO_ARCHER_CLIMB, &HERO_ARCHER_ROLL},
    {&HERO_MAGICIAN_IDLE_A, &HERO_MAGICIAN_IDLE_B, &HERO_MAGICIAN_WALK_A, &HERO_MAGICIAN_WALK_B, &HERO_MAGICIAN_JUMP,
     &HERO_MAGICIAN_ATTACK, &HERO_MAGICIAN_CLIMB, &HERO_MAGICIAN_ROLL},
    {&HERO_THIEF_IDLE_A, &HERO_THIEF_IDLE_B, &HERO_THIEF_WALK_A, &HERO_THIEF_WALK_B, &HERO_THIEF_JUMP, &HERO_THIEF_ATTACK,
     &HERO_THIEF_CLIMB, &HERO_THIEF_ROLL},
};
// Class select: POWER, TOUGHNESS, REACH out of 5, and two lines about the class.
const int CLASS_BARS[CLASS_COUNT][3] = {{3, 5, 1}, {3, 2, 5}, {5, 1, 4}, {2, 3, 2}};
const char* const CLASS_TEXT[CLASS_COUNT][2] = {
    {"HEAVY ARMOUR,", "A WIDE SWORD."}, {"ARROWS FROM", "FAR AWAY."}, {"BOLTS THAT HIT", "HARD. FRAGILE."}, {"FAST DAGGERS,", "QUICK FEET."}};
const char* const JOB_NAMES[CLASS_COUNT][4] = {{"KNIGHT", "VANGUARD", "WARDEN", "LIONHEART"},
                                               {"ARCHER", "RANGER", "SHARPSHOOTER", "STORMBOW"},
                                               {"MAGICIAN", "ADEPT", "ELEMENTALIST", "ARCHMAGE"},
                                               {"THIEF", "ROGUE", "SHADOW", "NIGHTBLADE"}};

static void drawCentred(int y, const char* text, uint32_t color, int scale = 1) {
    drawText((SCREEN_W - textWidth(text, scale)) / 2, y, text, color, scale);
}

static void drawPanel(int x, int y, int w, int h, char border) {
    fillRect(x, y, w, h, paletteColor(border));
    fillRect(x + 1, y + 1, w - 2, h - 2, paletteColor('k'));
}

// Far and near layers behind the tiles (far x0.25, near x0.5 of the camera).
static void drawBackground(const Quest& q, int camX, int camY) {
    if (q.area.world != WORLD_NAMULA) {   // TEST GROUNDS and worlds not built yet: plain with a 32 px grid
        fillRect(0, 0, SCREEN_W, SCREEN_H, paletteColor('k'));
        for (int x = (32 - camX % 32) % 32; x < SCREEN_W; x += 32) fillRect(x, 0, 1, SCREEN_H, paletteColor('n'));
        for (int y = (32 - camY % 32) % 32; y < SCREEN_H; y += 32) fillRect(0, y, SCREEN_W, 1, paletteColor('n'));
        return;
    }
    // Namula: night sky with stars, the sea at the horizon, far cliffs, darker near walls.
    fillRect(0, 0, SCREEN_W, 120, paletteColor('='));
    for (int i = 0; i < 40; i++) {
        int sx = ((i * 97 + 13) % 400 - camX / 8 % 400 + 400) % 400, sy = (i * 53 + 7) % 110;
        if (sx < SCREEN_W) fillRect(sx, sy, 1, 1, paletteColor(i % 3 ? 'n' : 'd'));
    }
    fillRect(0, 120, SCREEN_W, 60, paletteColor('2'));
    for (int x = 0; x < SCREEN_W; x += 2) {
        float wx = x + camX * 0.25f;
        int height = 16 + (int)(10 * sinf(wx * 0.021f) + 6 * sinf(wx * 0.067f + 1));
        if (height > 8) fillRect(x, 122 - height, 2, height + 6, paletteColor('*'));
        float nx = x + camX * 0.5f;
        int wall = (int)(12 * sinf(nx * 0.013f) + 6 * sinf(nx * 0.041f));
        if (wall > 6) fillRect(x, 170 - wall * 4, 2, wall * 4 + 10, paletteColor('v'));   // no edge line: it would read as a platform
    }
}

static void drawTile(const Quest& q, int tx, int ty, int sx, int sy) {
    const WorldStyle& style = worldDef(q.area.world).style;
    char t = tileAt(q.area, tx, ty);
    bool topOpen = !tileSolid(tileAt(q.area, tx, ty - 1));
    switch (t) {
    case '#':
        fillRect(sx, sy, TILE, TILE, paletteColor(style.fill));
        if (topOpen) fillRect(sx, sy, TILE, 2, paletteColor(style.top));
        if ((tx * 7 + ty * 3) % 5 == 0) fillRect(sx + 2 + tx % 4, sy + 4, 1, 1, paletteColor(style.top));
        break;
    case 'i':
        fillRect(sx, sy, TILE, TILE, paletteColor('I'));
        if (topOpen) fillRect(sx, sy, TILE, 2, paletteColor('i'));
        fillRect(sx + 1 + tx % 5, sy + 3, 2, 1, paletteColor('i'));
        break;
    case 'c':
        fillRect(sx, sy, TILE, TILE, paletteColor(style.fill));
        if (topOpen) fillRect(sx, sy, TILE, 2, paletteColor(style.top));
        for (int i = 0; i < 4; i++) fillRect(sx + 2 + i, sy + 2 + i + (i > 1), 1, 1, paletteColor('k'));
        fillRect(sx + 5, sy + 6, 2, 1, paletteColor('k'));
        break;
    case '-':
        fillRect(sx, sy, TILE, 2, paletteColor(style.platform));
        fillRect(sx + 1, sy + 2, 1, 2, paletteColor(style.fill));
        fillRect(sx + 6, sy + 2, 1, 2, paletteColor(style.fill));
        break;
    case '|':
        fillRect(sx + 1, sy, 1, TILE, paletteColor(style.ladder));
        fillRect(sx + 6, sy, 1, TILE, paletteColor(style.ladder));
        fillRect(sx + 1, sy + 2, 6, 1, paletteColor(style.ladder));
        fillRect(sx + 1, sy + 6, 6, 1, paletteColor(style.ladder));
        break;
    case '^':
        for (int k = 0; k < 2; k++) {
            fillRect(sx + k * 4 + 1, sy + 2, 2, 2, paletteColor('w'));
            fillRect(sx + k * 4 + 1, sy + 4, 2, 2, paletteColor('b'));
            fillRect(sx + k * 4, sy + 6, 4, 2, paletteColor('s'));
        }
        break;
    case '~': {
        int frame = (int)(q.time / SHIMMER_FRAME) % 2;
        fillRect(sx, sy, TILE, TILE, paletteColor(style.liquidA));
        if (tileAt(q.area, tx, ty - 1) != '~') fillRect(sx, sy, TILE, 1, paletteColor(style.liquidB));
        fillRect(sx + (frame ? 1 : 5), sy + 3, 2, 1, paletteColor(style.liquidB));
        fillRect(sx + (frame ? 5 : 1), sy + 6, 2, 1, paletteColor(style.liquidB));
        break;
    }
    case 'g':
        fillRect(sx, sy + 3, TILE, 1, paletteColor('d'));
        for (int k = 0; k < 3; k++) fillRect(sx + 1 + 3 * k, sy, 1, TILE, paletteColor('s'));
        break;
    case 'k':
        fillRect(sx, sy, TILE, TILE, paletteColor('&'));
        fillRect(sx + 1, sy + 1, TILE - 2, TILE - 2, paletteColor('h'));
        break;
    case 'z':
        fillRect(sx, sy, TILE, TILE, paletteColor('n'));
        for (int i = 0; i < 8; i++) fillRect(sx + i, sy + 3 + (i % 2), 1, 1, paletteColor('c'));
        break;
    case 'f':
        fillRect(sx, sy, TILE, TILE, paletteColor('K'));
        fillRect(sx + 2, sy, 4, 2, paletteColor('O'));
        break;
    case '<':
    case '>': {
        int shift = (int)(q.time * 50) % TILE;
        int x = t == '>' ? shift : TILE - 1 - shift;
        fillRect(sx + std::min(x, 5), sy + 2 + (tx + ty) % 4, 3, 1, paletteColor('n'));
        break;
    }
    case 'v':
        fillRect(sx, sy + 4, TILE, 4, paletteColor('d'));
        for (int k = 0; k < 3; k++) fillRect(sx + 1 + 3 * k, sy + 5, 1, 3, paletteColor('k'));
        break;
    case 'u':
        fillRect(sx, sy, TILE, 1, paletteColor('w'));
        fillRect(sx, sy + TILE - 1, TILE, 1, paletteColor('w'));
        fillRect(sx, sy, 1, TILE, paletteColor('w'));
        fillRect(sx + TILE - 1, sy, 1, TILE, paletteColor('w'));
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

// Checkpoints, portals and chests standing on the tile below their letter.
static void drawObject(const Quest& q, char letter, int tx, int ty, int camX, int camY) {
    const AreaDef& def = *q.area.def;
    int cx = tx * TILE + TILE / 2 - camX, bottom = (ty + 1) * TILE - camY;
    int flame = (int)(q.time / FLAME_FRAME) % 3;
    const Sprite* sprite = nullptr;
    if (letter == 'C' && q.area.world == WORLD_NAMULA) {
        const Sprite* hearth[3] = {&HEARTH_A, &HEARTH_B, &HEARTH_C};
        sprite = hearth[flame];
    } else if (letter == 'C') {
        int bit = q.area.world * 2 + checkpointId(q.area.world, q.area.index, ty * def.cols + tx);
        const Sprite* lit[3] = {&CHECKPOINT_LIT_A, &CHECKPOINT_LIT_B, &CHECKPOINT_LIT_C};
        sprite = bit < 32 && (q.save.lit >> bit & 1) ? lit[flame] : &CHECKPOINT_UNLIT;
    } else if (letter == 'P') {
        int index = 0;
        for (int t = 0; t < ty * def.cols + tx; t++) index += def.tiles[t] == 'P';
        bool open = index < 4 && def.portals[index].area >= 0;
        sprite = !open ? &ARCH_CLOSED : (int)(q.time / SHIMMER_FRAME) % 2 ? &PORTAL_B : &PORTAL_A;
    } else if (letter == 'T') {
        sprite = &CHEST_CLOSED;
    }
    if (sprite) drawSprite(*sprite, cx - sprite->w / 2, bottom - sprite->h);
}

static void drawHero(const Quest& q, int camX, int camY) {
    const Hero& h = q.hero;
    HeroFrame frame = FRAME_IDLE_A;
    bool flip = h.facingLeft;
    if (h.climbing) {
        frame = FRAME_CLIMB;
        flip = h.vy != 0 && (int)(q.time / CLIMB_FRAME) % 2;
    } else if (!h.onGround) {
        frame = FRAME_JUMP;
    } else if (fabsf(h.vx) > 10) {
        frame = (int)(q.time / WALK_FRAME) % 2 ? FRAME_WALK_B : FRAME_WALK_A;
    } else {
        frame = (int)(q.time / IDLE_FRAME) % 2 ? FRAME_IDLE_B : FRAME_IDLE_A;
    }
    int job = q.save.job >= 0 && q.save.job < 4 ? q.save.job : 0;
    const Sprite& sprite = *HERO_FRAMES[q.save.heroClass][frame];
    drawSpriteEx(sprite, (int)floorf(h.x) - 2 - camX, (int)floorf(h.y) - 2 - camY, flip, job ? 'h' : 0, paletteColor(TRIM_BY_JOB[job]));
}

static void drawPlay(const Quest& q) {
    int camX = (int)floorf(q.camX), camY = (int)floorf(q.camY);
    drawBackground(q, camX, camY);
    const AreaDef& def = *q.area.def;
    int col0 = camX / TILE, row0 = camY / TILE;
    for (int ty = row0; ty <= row0 + SCREEN_H / TILE && ty < def.rows; ty++)
        for (int tx = col0; tx <= col0 + SCREEN_W / TILE && tx < def.cols; tx++) drawTile(q, tx, ty, tx * TILE - camX, ty * TILE - camY);
    // Objects reach up to 3 tiles above their tile, so look a little below the view too.
    for (int ty = row0; ty <= row0 + SCREEN_H / TILE + 3 && ty < def.rows; ty++)
        for (int tx = std::max(col0 - 1, 0); tx <= col0 + SCREEN_W / TILE + 1 && tx < def.cols; tx++) {
            char letter = def.tiles[ty * def.cols + tx];
            if (letter == 'C' || letter == 'P' || letter == 'T') drawObject(q, letter, tx, ty, camX, camY);
        }
    drawHero(q, camX, camY);

    // Shake: shift the play picture by whole pixels; the HUD drawn after stays still.
    float shake = q.trauma * q.trauma * q.shakeStrength;
    int dx = (int)lroundf(SHAKE_X * shake * sinf(q.time * 41)), dy = (int)lroundf(SHAKE_Y * shake * sinf(q.time * 57));
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

    // HUD: level and the HP bar (full until combat comes in phase 2), then the banner.
    char text[32];
    snprintf(text, sizeof text, "LV %d", q.save.level);
    drawText(4, 4, text, paletteColor('w'));
    fillRect(4, 11, 64, 4, paletteColor('R'));
    fillRect(4, 11, 64, 4, paletteColor('r'));
    if (q.bannerTimer > 0 && q.banner)
        drawCentred(q.bannerScale == 2 ? 40 : 30, q.banner, paletteColor(q.bannerScale == 2 ? 'w' : 'b'), q.bannerScale);
    if (q.mode == QMODE_DEAD) drawCentred(80, "YOU FADED", paletteColor('r'), 2);
}

static void drawSlots(const Quest& q) {
    fillRect(0, 0, SCREEN_W, SCREEN_H, paletteColor('k'));
    drawCentred(12, "NAME QUEST", paletteColor('w'), 2);
    for (int i = 0; i < SLOT_COUNT; i++) {
        const SlotSummary& s = q.slots[i];
        int y = 34 + 46 * i;
        drawPanel(20, y, 280, 40, q.mode == QMODE_SLOTS && q.cursor == i ? 'y' : 'n');
        char text[48];
        if (!s.used) {
            snprintf(text, sizeof text, "SLOT %d  - EMPTY -", i + 1);
            drawText(28, y + 17, text, paletteColor('s'));
            continue;
        }
        int heroClass = s.heroClass % CLASS_COUNT, job = s.job >= 0 && s.job < 4 ? s.job : 0;
        snprintf(text, sizeof text, "SLOT %d", i + 1);
        drawText(28, y + 6, text, paletteColor('w'));
        if (job) snprintf(text, sizeof text, "%s - %s  LV %d", CLASS_NAMES[heroClass], JOB_NAMES[heroClass][job], s.level);
        else snprintf(text, sizeof text, "%s  LV %d", CLASS_NAMES[heroClass], s.level);
        drawText(28, y + 16, text, paletteColor('b'));
        int world = s.highest >= 0 && s.highest < WORLD_COUNT ? s.highest : 1;
        drawText(28, y + 26, worldDef(world).name, paletteColor('s'));
        int seconds = (int)s.playtime;
        snprintf(text, sizeof text, "%d:%02d:%02d", seconds / 3600, seconds / 60 % 60, seconds % 60);
        drawText(292 - textWidth(text), y + 6, text, paletteColor('b'));
        if (s.ngPlus > 0) {
            snprintf(text, sizeof text, "NG+%d", s.ngPlus);
            drawText(292 - textWidth(text), y + 16, text, paletteColor('o'));
        }
        if (s.secretDone) drawText(292 - textWidth("*"), y + 26, "*", paletteColor('z'));
    }
    drawCentred(172, "X PLAY  TRIANGLE DELETE  O BACK", paletteColor('s'));
    if (q.mode == QMODE_DELETE) {
        char title[24];
        snprintf(title, sizeof title, "DELETE SLOT %d?", q.slot + 1);
        const char* const items[2] = {"NO", "YES"};
        drawMenu(title, items, 2, q.cursor, "X CHOOSE  O BACK");
    }
}

static void drawClassSelect(const Quest& q) {
    fillRect(0, 0, SCREEN_W, SCREEN_H, paletteColor('k'));
    drawCentred(10, "CHOOSE YOUR CLASS", paletteColor('w'), 2);
    const char* const bars[3] = {"POWER", "TOUGHNESS", "REACH"};
    for (int i = 0; i < CLASS_COUNT; i++) {
        int x = 12 + 76 * i, y = 30;
        drawPanel(x, y, 72, 120, q.cursor == i ? 'y' : 'n');
        drawSprite(*HERO_FRAMES[i][FRAME_IDLE_A], x + 30, y + 6);
        drawText(x + (72 - textWidth(CLASS_NAMES[i])) / 2, y + 26, CLASS_NAMES[i], paletteColor(q.cursor == i ? 'y' : 'w'));
        for (int b = 0; b < 3; b++) {
            int by = y + 38 + 13 * b;
            drawText(x + 5, by, bars[b], paletteColor('s'));
            for (int step = 0; step < 5; step++)
                fillRect(x + 5 + 12 * step, by + 7, 10, 3, paletteColor(step < CLASS_BARS[i][b] ? 'b' : 'n'));
        }
        if (i == CLASS_THIEF) drawText(x + 5, y + 80, "DOUBLE JUMP", paletteColor('o'));
        drawText(x + 5, y + 94, CLASS_TEXT[i][0], paletteColor('b'));
        drawText(x + 5, y + 102, CLASS_TEXT[i][1], paletteColor('b'));
    }
    drawCentred(172, "X CHOOSE  O BACK", paletteColor('s'));
}

// The intro page typed out, line by line.
static void drawIntro(const Quest& q) {
    fillRect(0, 0, SCREEN_W, SCREEN_H, paletteColor('k'));
    const char* page = INTRO_PAGES[q.introPage % 4];
    int shown = (int)(q.modeTimer * INTRO_CHARS_PER_S), length = (int)strlen(page);
    const char* newline = strchr(page, '\n');
    int firstLength = (int)(newline - page);
    char line[64];
    snprintf(line, sizeof line, "%.*s", std::min(shown, firstLength), page);
    int width = textWidth(std::string(page, firstLength).c_str());
    drawText((SCREEN_W - width) / 2, 78, line, paletteColor('b'));
    if (shown > firstLength + 1) {
        snprintf(line, sizeof line, "%.*s", std::min(shown - firstLength - 1, length - firstLength - 1), newline + 1);
        drawText((SCREEN_W - textWidth(newline + 1)) / 2, 90, line, paletteColor('b'));
    }
    if (shown >= length) drawCentred(160, "X", paletteColor((int)(q.time * 2) % 2 ? 's' : 'n'));
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
