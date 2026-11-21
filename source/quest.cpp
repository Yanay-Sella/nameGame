// NAME QUEST rules: tiles and hero movement (spec §4, §5) and the mode machine (slots, class, intro, play, death).
#include "quest.h"
#include "screen.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

const char* const CLASS_NAMES[CLASS_COUNT] = {"KNIGHT", "ARCHER", "MAGICIAN", "THIEF"};

// Movement (spec §5), in one block so world 5's underwater numbers can scale it later.
const float GRAVITY = 700 * DP, JUMP_SPEED = 225 * DP, JUMP_CUT = 80 * DP, MAX_FALL = 260 * DP;
const float GROUND_ACCEL = 900 * DP, AIR_ACCEL = 500 * DP, ICE_ACCEL = 120 * DP;
const float COYOTE_TIME = 0.08f, JUMP_BUFFER = 0.10f, DOUBLE_JUMP_SPEED = 200 * DP, CLIMB_SPEED = 50 * DP;
const float DROP_TIME = 0.2f;                       // s one-way tops are ignored after down + X
const float CURRENT_PUSH = 50 * DP;                 // px/s added to x inside < and >
const float VENT_LIFT = 900 * DP, VENT_MAX_RISE = 160 * DP;   // net px/s² upward inside a vent's column, up to this rise speed
const int VENT_REACH = 6;                           // tiles above a vent that still lift
// Flow
const float DEATH_TIME = 2.5f, STEP_TIME = 0.28f, LAND_SOUND_SPEED = 120 * DP;
const float BANNER_TIME = 2;
const float TRAUMA_DECAY = 1.5f, TRAUMA_DEATH = 0.6f;
const float ECHO_SIZE = 8 * DP, NAMES_TEXT_TIME = 1.2f, DEATH_PARTICLES = 20;
const float SELECT_HOLD_TIME = 2;                   // dev: Select held this long → TEST GROUNDS area 0 (spec §20)
const float CAMERA_LEAD = 32 * DP, CAMERA_EASE_X = 5, CAMERA_EASE_Y = 6;
// RPG systems (spec §10, §11, §14): how near Up must be (dp from the hero's centre, same floor), and the timings.
const float NPC_REACH = 14 * DP, CHEST_REACH = 10 * DP, CHECKPOINT_REACH = 12 * DP, SURVEYOR_REACH = 20 * DP;
const float KINDLE_BANNER_TIME = 1.5f, LID_TIME = 0.3f, POPUP_TIME = 1.5f, REST_FADE = 0.3f, GATE_ANIMATION = 1.0f;
const float DRINK_TIME = 0.5f, BUFF_TIME = 120;
const float POTION_HEAL[3] = {0.35f, 0.70f, 0.50f};   // RED and BIG: of max HP; MANA: of max MP

const char* const INTRO_PAGES[4] = {
    "LONG AGO, EVERY THING HAD A NAME,\nAND EVERY NAME KEPT ITS THING ALIVE.",
    "THEN CAME THE UNNAMING: A GREY HUNGER\nTHAT EATS NAMES. WHAT LOSES ITS NAME, FADES.",
    "ONE FIRE STILL BURNS: THE HEARTH OF NAMES,\nIN NAMULA, AT THE EDGE OF THE WORLD.",
    "YOU WAKE BESIDE IT.\nYOU DO NOT REMEMBER YOUR NAME.",
};

bool tileSolid(char t) {
    return t == '#' || t == 'i' || t == 'c' || t == 'g' || t == 'k' || t == 'f' || t == 'u';
}

char tileAt(const AreaState& area, int tx, int ty) {
    const AreaDef& def = *area.def;
    if (tx < 0) return def.left.area >= 0 ? '.' : '#';
    if (tx >= def.cols) return def.right.area >= 0 ? '.' : '#';
    if (ty < 0) return def.up.area >= 0 ? '.' : '#';
    if (ty >= def.rows) return '.';
    return area.tiles[ty * def.cols + tx];
}

void loadArea(AreaState& area, int world, int index, uint32_t gates, uint32_t shortcuts) {
    const AreaDef& def = worldDef(world).areas[index];
    area.world = world;
    area.index = index;
    area.def = &def;
    area.tiles.assign(def.tiles, def.tiles + def.cols * def.rows);
    bool gateOpen = gates & (1u << world);
    bool doorOpen = def.shortcut >= 0 && (shortcuts & (1u << def.shortcut));
    for (char& t : area.tiles)
        if ((t == 'g' && gateOpen) || (t == 'k' && doorOpen)) t = '.';
}

// Tile rows and columns the hitbox covers.
static int firstCol(const Hero& h) { return (int)floorf(h.x / TILE); }
static int lastCol(const Hero& h) { return (int)floorf((h.x + HERO_W - 0.001f) / TILE); }
static int firstRow(const Hero& h) { return (int)floorf(h.y / TILE); }
static int lastRow(const Hero& h) { return (int)floorf((h.y + HERO_H - 0.001f) / TILE); }

// The ladder column the hitbox overlaps (the centre's first), looking `below` rows under the feet too; -1 when none.
static int ladderColumn(const Hero& h, const AreaState& area, int below) {
    int columns[3] = {(int)floorf((h.x + HERO_W / 2) / TILE), firstCol(h), lastCol(h)};
    for (int c : columns)
        for (int r = firstRow(h); r <= lastRow(h) + below; r++)
            if (tileAt(area, c, r) == '|') return c;
    return -1;
}

void moveHero(Hero& h, const Actions& a, const AreaState& area, HeroClass heroClass, float dt) {
    h.jumped = false;
    h.coyote = h.onGround ? COYOTE_TIME : fmaxf(0, h.coyote - dt);
    h.jumpBuffer = a.jump ? JUMP_BUFFER : fmaxf(0, h.jumpBuffer - dt);
    h.dropTimer = fmaxf(0, h.dropTimer - dt);
    if (h.onGround || h.climbing) h.doubleJumpUsed = false;
    int col = (int)floorf((h.x + HERO_W / 2) / TILE), feetRow = lastRow(h) + 1;

    // Ladders: up or down on a ladder column grabs it (down also from its top); the hero hangs centred on it.
    int ladder = a.moveY < -0.5f ? ladderColumn(h, area, 0) : a.moveY > 0.5f ? ladderColumn(h, area, h.onGround ? 1 : 0) : -1;
    if (!h.climbing && ladder >= 0) {
        h.climbing = true;
        h.x = (float)(ladder * TILE);
        h.vx = h.vy = 0;
        col = ladder;
    }

    // Walking eases toward the stick's speed; ice is slow to start and to stop.
    float walk = CLASSES[heroClass].walk * DP;
    if (h.climbing) {
        h.vx = 0;
        h.vy = a.moveY * CLIMB_SPEED;
    } else {
        bool onIce = h.onGround && tileAt(area, col, feetRow) == 'i';
        float accel = !h.onGround ? AIR_ACCEL : onIce ? ICE_ACCEL : GROUND_ACCEL;
        float target = a.moveX * walk;
        h.vx = target > h.vx ? fminf(h.vx + accel * dt, target) : fmaxf(h.vx - accel * dt, target);
    }
    if (a.moveX < -0.1f) h.facingLeft = true;
    else if (a.moveX > 0.1f) h.facingLeft = false;

    // Jump (buffered, with coyote time, or off a ladder sideways); down + X on a one-way platform drops through instead.
    if (h.jumpBuffer > 0 && (h.coyote > 0 || h.climbing)) {
        bool onOneWayOnly = h.onGround && a.moveY > 0.5f;
        for (int c = firstCol(h); c <= lastCol(h); c++) onOneWayOnly &= !tileSolid(tileAt(area, c, feetRow));
        if (onOneWayOnly) {
            h.dropTimer = DROP_TIME;
        } else {
            h.vy = -JUMP_SPEED;
            if (h.climbing) h.vx = a.moveX * walk;
            h.climbing = false;
            h.jumped = true;
        }
        h.jumpBuffer = h.coyote = 0;
    } else if (heroClass == CLASS_THIEF && a.jump && !h.onGround && !h.climbing && !h.doubleJumpUsed) {
        h.vy = -DOUBLE_JUMP_SPEED;
        h.doubleJumpUsed = true;
        h.jumpBuffer = 0;
        h.jumped = true;
    }
    if (!a.jumpHeld && h.vy < -JUMP_CUT) h.vy = -JUMP_CUT;

    // Gravity, currents and vents; y moves by the step's average speed, so a jump peaks where the formula says.
    float vyBefore = h.vy;
    float dx = h.vx * dt;
    if (!h.climbing) {
        h.vy = fminf(h.vy + GRAVITY * dt, MAX_FALL);
        float push = 0;
        bool inVent = false;
        for (int c = firstCol(h); c <= lastCol(h); c++) {
            for (int r = firstRow(h); r <= lastRow(h); r++) {
                char t = tileAt(area, c, r);
                if (t == '<') push = -CURRENT_PUSH;
                if (t == '>') push = CURRENT_PUSH;
            }
            for (int r = firstRow(h); r <= lastRow(h) + VENT_REACH; r++) inVent |= tileAt(area, c, r) == 'v';
        }
        dx += push * dt;
        if (inVent && h.vy > -VENT_MAX_RISE) h.vy = fmaxf(h.vy - (GRAVITY + VENT_LIFT) * dt, -VENT_MAX_RISE);
    }
    float dy = (vyBefore + h.vy) / 2 * dt;

    // Move x, then y, each against the solid tiles; falling also lands on one-way tops it was above.
    h.x += dx;
    if (dx > 0) {
        int c = lastCol(h);
        for (int r = firstRow(h); r <= lastRow(h); r++)
            if (tileSolid(tileAt(area, c, r))) { h.x = (float)(c * TILE) - HERO_W; h.vx = 0; break; }
    } else if (dx < 0) {
        int c = firstCol(h);
        for (int r = firstRow(h); r <= lastRow(h); r++)
            if (tileSolid(tileAt(area, c, r))) { h.x = (float)((c + 1) * TILE); h.vx = 0; break; }
    }
    float previousBottom = h.y + HERO_H;
    h.y += dy;
    if (dy > 0) {
        int r = lastRow(h);
        bool wasAbove = previousBottom <= r * TILE + 0.01f && h.dropTimer <= 0;
        for (int c = firstCol(h); c <= lastCol(h); c++) {
            char t = tileAt(area, c, r);
            bool ladderTop = t == '|' && !h.climbing && tileAt(area, c, r - 1) != '|';
            if (tileSolid(t) || (wasAbove && (t == '-' || ladderTop))) {
                h.y = (float)(r * TILE) - HERO_H;
                h.vy = 0;
                break;
            }
        }
    } else if (dy < 0) {
        int r = firstRow(h);
        for (int c = firstCol(h); c <= lastCol(h); c++)
            if (tileSolid(tileAt(area, c, r))) { h.y = (float)((r + 1) * TILE); h.vy = 0; break; }
    }

    // Standing: the feet exactly on a tile top that holds them.
    float feet = h.y + HERO_H;
    feetRow = (int)lroundf(feet / TILE);
    h.onGround = false;
    if (h.vy >= 0 && fabsf(feet - feetRow * TILE) < 0.01f) {
        for (int c = firstCol(h); c <= lastCol(h); c++) {
            char t = tileAt(area, c, feetRow);
            bool ladderTop = t == '|' && !h.climbing && tileAt(area, c, feetRow - 1) != '|';
            h.onGround |= tileSolid(t) || (h.dropTimer <= 0 && (t == '-' || ladderTop));
        }
    }
    if (h.climbing && (ladderColumn(h, area, 0) < 0 || (h.onGround && a.moveY >= 0))) h.climbing = false;

    // Safe ground for pit returns: plain ground or ice, with no spikes or liquid within 2 tiles of the feet.
    col = (int)floorf((h.x + HERO_W / 2) / TILE);
    char under = tileAt(area, col, feetRow);
    if (h.onGround && (under == '#' || under == 'i')) {
        bool hazard = false;
        for (int r = feetRow - 2; r <= feetRow + 1; r++)
            for (int c = col - 2; c <= col + 2; c++) {
                char t = tileAt(area, c, r);
                hazard |= t == '^' || t == '~';
            }
        if (!hazard) {
            h.lastSafeX = h.x;
            h.lastSafeY = h.y;
        }
    }
}

bool heroInLiquid(const Hero& h, const AreaState& area) {
    for (int r = firstRow(h); r <= lastRow(h); r++)
        for (int c = firstCol(h); c <= lastCol(h); c++)
            if (tileAt(area, c, r) == '~') return true;
    return false;
}

// Appends one spec §18 line for main.cpp to print.
void logLine(Quest& q, const char* format, ...) {
    char text[160];
    va_list args;
    va_start(args, format);
    vsnprintf(text, sizeof text, format, args);
    va_end(args);
    q.logLines.push_back(text);
}

static SlotSummary summaryOf(const SaveData& s) {
    return {s.used, s.heroClass, s.level, s.job, s.highest, s.ngPlus, s.playtime, (s.keys >> KEY_SECRET_DONE & 1) != 0};
}

bool questSave(Quest& q) {
    bool renameFailed = false;
    bool ok = saveWrite(q.folder.c_str(), q.slot, q.save, &renameFailed);
    logLine(q, "quest: save slot %d %s", q.slot + 1, !ok ? "FAILED" : renameFailed ? "rename failed, wrote directly" : "ok");
    if (ok) q.slots[q.slot] = summaryOf(q.save);
    return ok;
}

// The hero's standing spot: air there and above, something to stand on below, no spikes or liquid.
static bool standable(const AreaState& area, int tx, int ty) {
    char t = tileAt(area, tx, ty), below = tileAt(area, tx, ty + 1);
    bool holds = tileSolid(below) || below == '-' || (below == '|' && t != '|');
    return !tileSolid(t) && !tileSolid(tileAt(area, tx, ty - 1)) && t != '~' && t != '^' && ty + 1 < area.def->rows && holds;
}

// Moves the camera toward the hero (snap: at once); wide areas are bottom-aligned, tall ones follow in y.
static void updateCamera(Quest& q, float dt, bool snap) {
    const AreaDef& def = *q.area.def;
    float targetX = q.hero.x + HERO_W / 2 - QUEST_W / 2 + (q.hero.facingLeft ? -CAMERA_LEAD : CAMERA_LEAD);
    float targetY = def.rows == TALL_ROWS ? q.hero.y + HERO_H / 2 - QUEST_H / 2 : (float)(def.rows * TILE - QUEST_H);
    q.camX = snap ? targetX : q.camX + (targetX - q.camX) * fminf(1, CAMERA_EASE_X * dt);
    q.camY = snap || def.rows != TALL_ROWS ? targetY : q.camY + (targetY - q.camY) * fminf(1, CAMERA_EASE_Y * dt);
    q.camX = fmaxf(0, fminf(q.camX, (float)(def.cols * TILE - QUEST_W)));
    q.camY = fmaxf(0, fminf(q.camY, (float)(def.rows * TILE - QUEST_H)));
}

// Loads an area and stands the hero on tile (tx, ty), or on its first standable tile from the left when tx < 0.
static void enterArea(Quest& q, int world, int index, int tx, int ty) {
    bool newWorld = q.area.def == nullptr || q.area.world != world;
    loadArea(q.area, world, index, q.save.gates, world < SAVED_WORLDS ? q.save.shortcuts[world] : 0);
    const AreaDef& def = *q.area.def;
    for (int c = 0; c < def.cols && tx < 0; c++)
        for (int r = 0; r < def.rows && tx < 0; r++)
            if (standable(q.area, c, r)) { tx = c; ty = r; }
    q.hero.x = (float)(tx * TILE);
    q.hero.y = (float)((ty + 1) * TILE) - HERO_H;
    q.hero.climbing = tileAt(q.area, tx, ty) == '|' && q.hero.climbing;
    q.hero.lastSafeX = q.hero.x;
    q.hero.lastSafeY = q.hero.y;
    updateCamera(q, 0, true);
    spawnEnemies(q);
    if (index == 0 && world >= 1 && world <= 6 && world > q.save.highest) q.save.highest = world;   // spec §11 highestWorld
    q.ambient = world == WORLD_TEST ? 0 : world + 1;   // 1 Namula, 2-8 worlds 1-7 (spec §16)
    bool firstVisit = world < SAVED_WORLDS && !(q.save.visited[world] >> index & 1);
    if (world < SAVED_WORLDS) q.save.visited[world] |= 1u << index;
    if (newWorld || firstVisit) {
        q.banner = newWorld ? worldDef(world).name : def.name;
        q.bannerScale = newWorld ? 2 : 1;
        q.bannerTimer = BANNER_TIME;
        q.bannerColor = newWorld ? 'w' : 'b';
    }
    logLine(q, "quest: area %d.%d %s", world, index, def.name);
}

// Starts play at the respawn checkpoint: id n is the n-th C of the world, areas in order, rows top to bottom.
static void enterAtCheckpoint(Quest& q) {
    int world = q.save.checkpointWorld >= 0 && q.save.checkpointWorld < WORLD_COUNT ? q.save.checkpointWorld : WORLD_NAMULA;
    const WorldDef& w = worldDef(world);
    if (w.count == 0) world = WORLD_NAMULA;
    int seen = 0;
    for (int i = 0; i < worldDef(world).count; i++) {
        const AreaDef& def = worldDef(world).areas[i];
        for (int t = 0; t < def.cols * def.rows; t++) {
            if (def.tiles[t] != 'C' || seen++ != q.save.checkpointId) continue;
            q.hero = Hero();
            questRefreshStats(q, true);
            enterArea(q, world, i, t % def.cols, t / def.cols);
            q.mode = QMODE_PLAY;
            q.modeTimer = 0;
            return;
        }
    }
    q.hero = Hero();
    questRefreshStats(q, true);
    enterArea(q, world, 0, -1, -1);
    q.mode = QMODE_PLAY;
    q.modeTimer = 0;
}

static void die(Quest& q, const char* cause) {
    if (q.godMode) {   // test mode: back on the last safe ground instead
        q.hero.x = q.hero.lastSafeX;
        q.hero.y = q.hero.lastSafeY;
        q.hero.vx = q.hero.vy = 0;
        return;
    }
    // Every carried name drops as an echo where the hero last stood on safe ground; an older echo is lost (spec §7).
    Hero& h = q.hero;
    int dropped = q.save.names;
    q.save.echoNames = dropped;
    q.save.echoWorld = q.area.world;
    q.save.echoArea = q.area.index;
    q.save.echoX = h.lastSafeX + HERO_W / 2 - ECHO_SIZE / 2;
    q.save.echoY = h.lastSafeY + HERO_H - ECHO_SIZE - 2 * DP;
    q.save.names = 0;
    for (int i = 0; i < DEATH_PARTICLES; i++) {
        float angle = i * 6.2832f / DEATH_PARTICLES, speed = (40 + 20 * (i % 3)) * DP;
        q.particles.push_back({h.x + HERO_W / 2, h.y + HERO_H / 2, cosf(angle) * speed, sinf(angle) * speed, 0.8f, paletteColor('w'), 1});
    }
    q.mode = QMODE_DEAD;
    q.modeTimer = 0;
    q.soundRequested[SOUND_Q_DEATH] = true;
    if (!strcmp(cause, "liquid")) q.soundRequested[SOUND_Q_SPLASH] = true;
    q.trauma = fminf(1, q.trauma + TRAUMA_DEATH);
    logLine(q, "quest: hero died in %d.%d (%s), dropped %d names", q.area.world, q.area.index, cause, dropped);
}

void questOpen(Quest& q, const char* folder) {
    q = Quest();
    q.folder = folder;
    for (int i = 0; i < SLOT_COUNT; i++) {
        SaveData s;
        saveRead(folder, i, s);
        q.slots[i] = summaryOf(s);
    }
    q.mode = QMODE_SLOTS;
    q.cursor = 0;
}

bool questCanPause(const Quest& q) {
    return q.mode == QMODE_PLAY || q.mode == QMODE_DEAD || q.mode == QMODE_DIALOGUE || q.mode == QMODE_MENU ||
           q.mode == QMODE_SHOP || q.mode == QMODE_MAP;
}

int objectIndex(int world, int area, int tile, char letter) {
    int n = 0;
    for (int i = 0; i <= area && i < worldDef(world).count; i++) {
        const AreaDef& def = worldDef(world).areas[i];
        int end = i == area ? tile : def.cols * def.rows;
        for (int t = 0; t < end; t++) n += def.tiles[t] == letter;
    }
    return n;
}

std::vector<int> travelCheckpoints(const SaveData& save) {
    std::vector<int> list = {0};   // the hearth, always lit
    for (int world = 1; world < WORLD_COUNT; world++) {
        int count = objectIndex(world, worldDef(world).count, 0, 'C');   // every C of the world
        for (int id = 0; id < count && id < 2; id++)
            if (save.lit >> (world * 2 + id) & 1) list.push_back(world * 2 + id);
    }
    return list;
}

std::string checkpointName(int bit) {
    if (bit == 0) return "HEARTH OF NAMES";
    return std::string(worldDef(bit / 2).name) + (bit % 2 ? " DEPTHS" : " ENTRY");
}

// Rest resets the crumbled blocks (phase 4) and dropped icicles (phase 5) by reloading the area's tiles.
static void resetHazards(Quest& q) {
    loadArea(q.area, q.area.world, q.area.index, q.save.gates, q.save.shortcuts[q.area.world]);
}

// REST (spec §11): HP and MP full, every enemy and mini-boss back, hazards reset, the respawn point, a white fade, autosave.
static void rest(Quest& q, int bit) {
    for (auto& killed : q.killed) killed.assign(killed.size(), 0);
    resetHazards(q);
    spawnEnemies(q);
    questRefreshStats(q, true);
    q.save.checkpointWorld = bit / 2;
    q.save.checkpointId = bit % 2;
    q.fadeTimer = q.reduceFlashing ? 0 : REST_FADE;
    logLine(q, "quest: rest at %d.%d", bit / 2, bit % 2);
    questSave(q);
}

void openGate(Quest& q, int world) {
    if (q.save.gates >> world & 1) return;
    q.save.gates |= 1u << world;
    q.banner = "THE GATE OPENS";
    q.bannerScale = 2;
    q.bannerColor = 'w';
    q.bannerTimer = BANNER_TIME;
    q.soundRequested[SOUND_Q_GATE] = true;
    if (q.area.def && q.area.world == world) {   // solid no more at once; the tiles shake, then vanish top to bottom
        for (char& t : q.area.tiles)
            if (t == 'g') t = '.';
        q.gateTimer = GATE_ANIMATION;
    }
    logLine(q, "quest: gate %d open", world);
    questSave(q);
}

void openShortcut(Quest& q) {
    const AreaDef& def = *q.area.def;
    uint32_t& open = q.save.shortcuts[q.area.world];
    if (def.shortcut < 0 || (open >> def.shortcut & 1)) return;
    open |= 1u << def.shortcut;
    for (char& t : q.area.tiles)
        if (t == 'k') t = '.';
    q.soundRequested[SOUND_Q_GATE] = true;
    logLine(q, "quest: shortcut %d.%d open", q.area.world, def.shortcut);
    questSave(q);
}

// Up at a chest (spec §10): the contents go to the bag, or "YOUR BAG IS FULL" and the chest stays shut.
static void openChest(Quest& q, int tile, int chestId) {
    SaveData& s = q.save;
    int world = q.area.world;
    const ChestContent& c = chestContent(world, chestId);
    char text[40];
    bool full = false;
    switch (c.kind) {
    case CHEST_ITEM:
        full = s.items[c.item] + c.count > carryLimit(s, c.item);
        if (!full) s.items[c.item] += c.count;
        if (c.count > 1) snprintf(text, sizeof text, "%d %s", c.count, ITEM_NAMES[c.item]);
        else snprintf(text, sizeof text, "%s", ITEM_NAMES[c.item]);
        break;
    case CHEST_NAMES: {
        int names = enemyNames(statsWorld(world), (float)c.count, 0, false);   // WORLD_NAMES[world] x count
        s.names = s.names > MAX_NAMES - names ? MAX_NAMES : s.names + names;
        snprintf(text, sizeof text, "+%d NAMES", names);
        break;
    }
    case CHEST_KEY:
        s.keys |= 1u << c.item;
        snprintf(text, sizeof text, "%s", KEY_NAMES[c.item]);
        break;
    case CHEST_SHARD:
        s.shards++;
        snprintf(text, sizeof text, "VIAL SHARD");
        break;
    case CHEST_PAGE:
        s.keys |= 1u << (KEY_PAGE_1 + c.item - 1);
        snprintf(text, sizeof text, "MAP PAGE %d", c.item);
        break;
    }
    const AreaDef& def = *q.area.def;
    q.popupX = (tile % def.cols) * TILE + TILE / 2.0f;
    q.popupY = (float)((tile / def.cols + 1) * TILE) - 16 * DP;
    if (full) {
        q.popup = "YOUR BAG IS FULL";
        q.popupTimer = POPUP_TIME;
        q.soundRequested[SOUND_ROCKET_EMPTY] = true;
        return;
    }
    q.popup = text;
    q.popupTimer = LID_TIME + POPUP_TIME;   // shown once the lid is up
    q.chestTile = tile;
    q.chestTimer = LID_TIME;
    s.chests[world] |= 1u << chestId;
    q.soundRequested[SOUND_Q_CHEST] = true;
    logLine(q, "quest: chest %d.%d %s", world, chestId, text);
    questSave(q);
}

static void openMenu(Quest& q, int kind, std::vector<std::string> items) {
    q.menu.kind = kind;
    q.menu.items = items;
    q.menu.cursor = q.menu.scroll = 0;
    q.mode = QMODE_MENU;
    q.modeTimer = 0;
}

void questStartDev(Quest& q, const char* spec) {
    SaveData s;
    s.used = true;
    int slot = 3, world = 0, area = 0, gate = 0;
    char text[256];
    snprintf(text, sizeof text, "%s", spec);
    for (char* token = strtok(text, " "); token; token = strtok(NULL, " ")) {
        char* value = strchr(token, '=');
        if (!value) continue;
        *value++ = 0;
        int id;
        if (!strcmp(token, "slot")) slot = atoi(value);
        else if (!strcmp(token, "class")) s.heroClass = atoi(value);
        else if (!strcmp(token, "world")) world = atoi(value);
        else if (!strcmp(token, "area")) area = atoi(value);
        else if (!strcmp(token, "level")) s.level = atoi(value);
        else if (!strcmp(token, "job")) s.job = atoi(value);
        else if (!strcmp(token, "names")) s.names = atoi(value);
        else if (!strcmp(token, "gear")) s.weapon = s.armor = atoi(value);
        else if (!strcmp(token, "items"))
            sscanf(value, "%d,%d,%d,%d,%d,%d", &s.items[0], &s.items[1], &s.items[2], &s.items[3], &s.items[4], &s.items[5]);
        else if (!strcmp(token, "gate")) gate = atoi(value);
        else if (!strcmp(token, "keys")) s.keys = (uint32_t)strtoul(value, NULL, 16);
        else if (!strcmp(token, "bosses")) s.bosses = (uint32_t)strtoul(value, NULL, 16);
        else if (sscanf(token, "quest%d", &id) == 1 && id >= 0 && id < QUEST_COUNT) s.quests[id] = atoi(value);
    }
    if (world < 0 || world >= WORLD_COUNT || worldDef(world).count == 0) world = WORLD_NAMULA;
    if (area < 0 || area >= worldDef(world).count) area = 0;
    s.heroClass = s.heroClass >= 0 && s.heroClass < CLASS_COUNT ? s.heroClass : 0;
    if (gate) s.gates |= 1u << world;
    s.highest = world > 1 ? world : 1;
    q.slot = slot >= 1 && slot <= SLOT_COUNT ? slot - 1 : SLOT_COUNT - 1;
    q.save = s;
    logLine(q, "quest: slot %d new (class %s, level %d)", q.slot + 1, CLASS_NAMES[s.heroClass], s.level);
    questSave(q);
    q.hero = Hero();
    questRefreshStats(q, true);
    enterArea(q, world, area, -1, -1);
    q.mode = QMODE_PLAY;
    q.modeTimer = 0;
}

void questUpdate(Quest& q, const Actions& a, float dt) {
    q.time += dt;
    q.modeTimer += dt;
    q.bannerTimer = fmaxf(0, q.bannerTimer - dt);
    q.trauma = fmaxf(0, q.trauma - TRAUMA_DECAY * dt);
    q.popupTimer = fmaxf(0, q.popupTimer - dt);
    q.chestTimer = fmaxf(0, q.chestTimer - dt);
    q.gateTimer = fmaxf(0, q.gateTimer - dt);
    q.fadeTimer = fmaxf(0, q.fadeTimer - dt);
    switch (q.mode) {
    case QMODE_SLOTS:
        if (a.up || a.down) {
            q.cursor = (q.cursor + (a.up ? SLOT_COUNT - 1 : 1)) % SLOT_COUNT;
            q.soundRequested[SOUND_Q_BLIP] = true;
        }
        if (a.jump && !q.slots[q.cursor].used) {
            q.slot = q.cursor;
            q.cursor = 0;
            q.mode = QMODE_CLASS;
        } else if (a.jump) {
            q.slot = q.cursor;
            saveRead(q.folder.c_str(), q.slot, q.save);
            q.save.heroClass = q.save.heroClass >= 0 && q.save.heroClass < CLASS_COUNT ? q.save.heroClass : 0;
            q.save.lit |= 1;   // the hearth is always lit
            logLine(q, "quest: slot %d loaded (class %s, level %d)", q.slot + 1, CLASS_NAMES[q.save.heroClass], q.save.level);
            q.area.def = nullptr;
            enterAtCheckpoint(q);
        } else if (a.item && q.slots[q.cursor].used) {
            q.slot = q.cursor;
            q.cursor = 0;
            q.mode = QMODE_DELETE;
        } else if (a.back) {
            q.wantsMainMenu = true;
        }
        break;
    case QMODE_DELETE:
        if (a.up || a.down) q.cursor = 1 - q.cursor;
        if (a.jump && q.cursor == 1) {
            const SlotSummary& gone = q.slots[q.slot];
            logLine(q, "quest: slot %d deleted (class %s, level %d)", q.slot + 1, CLASS_NAMES[gone.heroClass % CLASS_COUNT], gone.level);
            saveDelete(q.folder.c_str(), q.slot);
            q.slots[q.slot] = SlotSummary();
        }
        if (a.jump || a.back) {
            q.cursor = q.slot;
            q.mode = QMODE_SLOTS;
        }
        break;
    case QMODE_CLASS:
        if (a.left || a.right) {
            q.cursor = (q.cursor + (a.left ? CLASS_COUNT - 1 : 1)) % CLASS_COUNT;
            q.soundRequested[SOUND_Q_BLIP] = true;
        }
        if (a.jump) {   // the slot is written at once, so a crash after the pick keeps the class
            q.save = SaveData();
            q.save.used = true;
            q.save.heroClass = q.cursor;
            logLine(q, "quest: slot %d new (class %s, level %d)", q.slot + 1, CLASS_NAMES[q.cursor], q.save.level);
            questSave(q);
            q.introPage = 0;
            q.modeTimer = 0;
            q.mode = QMODE_INTRO;
        } else if (a.back) {
            q.cursor = q.slot;
            q.mode = QMODE_SLOTS;
        }
        break;
    case QMODE_INTRO:
        if (a.jump && q.modeTimer * INTRO_CHARS_PER_S < strlen(INTRO_PAGES[q.introPage])) {
            q.modeTimer = 100;   // completes the page
        } else if (a.jump && ++q.introPage < 4) {
            q.modeTimer = 0;
        } else if (a.jump) {   // wake beside the hearth, at Namula's S
            const AreaDef& namula = worldDef(WORLD_NAMULA).areas[0];
            const char* start = strchr(namula.tiles, 'S');
            q.hero = Hero();
            questRefreshStats(q, true);
            q.area.def = nullptr;
            enterArea(q, WORLD_NAMULA, 0, (int)(start - namula.tiles) % namula.cols, (int)(start - namula.tiles) / namula.cols);
            q.mode = QMODE_PLAY;
            q.modeTimer = 0;
        }
        break;
    case QMODE_PLAY: {
        q.save.playtime += dt;
        if (q.devAids && a.godMode) {
            q.godMode = !q.godMode;
            logLine(q, "quest: test mode %s", q.godMode ? "on" : "off");
        }
        updateEffects(q, dt);
        q.selectHeldTime = a.selectHeld ? q.selectHeldTime + dt : 0;
        if (q.devAids && q.selectHeldTime >= SELECT_HOLD_TIME && q.selectHeldTime - dt < SELECT_HOLD_TIME) {   // Select held 2 s
            logLine(q, "quest: warp %d.%d", WORLD_TEST, 0);
            enterArea(q, WORLD_TEST, 0, -1, -1);
            break;
        }
        if (q.devAids && a.autopilot) {   // Select: warp to the next area of this world
            int next = (q.area.index + 1) % worldDef(q.area.world).count;
            logLine(q, "quest: warp %d.%d", q.area.world, next);
            enterArea(q, q.area.world, next, -1, -1);
            break;
        }
        Hero& h = q.hero;
        const AreaDef& def = *q.area.def;
        SaveData& s = q.save;
        int world = q.area.world;
        // What the hero touches: objects stand on the tile below their letter; Up uses the first of NPC, chest,
        // checkpoint, surveyor, portal (spec §2), and never grabs a ladder while one is touched.
        float heroCx = h.x + HERO_W / 2, heroBottom = h.y + HERO_H;
        int npc = -1, chestTile = -1, chestId = -1, checkpoint = -1, surveyor = -1, portal = -1;
        int npcs = 0, chests = 0, portals = 0;
        for (int t = 0; t < def.cols * def.rows; t++) {
            char letter = def.tiles[t];
            if (!strchr("NTCMP", letter)) continue;
            float cx = (t % def.cols) * TILE + TILE / 2.0f, bottom = (float)((t / def.cols + 1) * TILE);
            float dx = fabsf(cx - heroCx);
            bool sameFloor = fabsf(bottom - heroBottom) < TILE / 2.0f;
            if (letter == 'N') {
                int id = npcs < 8 ? def.npcs[npcs] : -1;
                npcs++;
                if (npc < 0 && id >= 0 && sameFloor && dx <= NPC_REACH && npcPresent(s, id, world, q.area.index)) npc = id;
            } else if (letter == 'T') {
                int id = chests < 4 ? def.chests[chests] : -1;
                chests++;
                if (chestTile < 0 && id >= 0 && sameFloor && dx <= CHEST_REACH && !(s.chests[world] >> id & 1)) chestTile = t, chestId = id;
            } else if (letter == 'C' && sameFloor && dx <= CHECKPOINT_REACH) {
                int bit = world == WORLD_NAMULA ? 0 : world * 2 + objectIndex(world, q.area.index, t, 'C');
                if (bit < 32 && !(s.lit >> bit & 1)) {   // touching an unlit brazier lights it (spec §11)
                    s.lit |= 1u << bit;
                    q.banner = "NAME KINDLED";
                    q.bannerScale = 2;
                    q.bannerColor = 'o';
                    q.bannerTimer = KINDLE_BANNER_TIME;
                    q.soundRequested[SOUND_Q_KINDLE] = true;
                    logLine(q, "quest: checkpoint %d.%d lit", bit / 2, bit % 2);
                    questSave(q);
                }
                if (checkpoint < 0) checkpoint = bit;
            } else if (letter == 'M' && surveyor < 0 && sameFloor && dx <= SURVEYOR_REACH) {
                int m = objectIndex(world, q.area.index, t, 'M');   // 2 per world; TEST GROUNDS uses the first
                surveyor = world >= 1 && world <= 6 ? NPC_SURVEYOR_FIRST + (world - 1) * 2 + m % 2 : NPC_SURVEYOR_FIRST;
            } else if (letter == 'P') {
                if (h.x < cx + 8 * DP && h.x + HERO_W > cx - 8 * DP && h.y < bottom && h.y + HERO_H > bottom - 24 * DP) portal = portals;
                portals++;
            }
        }
        bool touching = npc >= 0 || chestTile >= 0 || checkpoint >= 0 || surveyor >= 0 || portal >= 0;
        Actions move = a;
        if (touching && h.onGround && move.moveY < 0) move.moveY = 0;
        if (h.onGround && a.up && touching) {
            if (npc >= 0) {
                talkTo(q, npc);
            } else if (chestTile >= 0) {
                openChest(q, chestTile, chestId);
            } else if (checkpoint >= 0) {
                q.atCheckpoint = checkpoint;
                openMenu(q, MENU_CHECKPOINT, {"REST", "TRAVEL", "LEAVE"});
            } else if (surveyor >= 0) {
                talkTo(q, surveyor);
            } else if (def.portals[portal].area >= 0) {
                AreaLink link = def.portals[portal];
                q.soundRequested[SOUND_Q_PORTAL] = true;
                h.vx = 0;
                enterArea(q, q.area.world, link.area, link.tileX, link.tileY);
                break;
            }
            if (q.mode != QMODE_PLAY) break;
        }

        // Items (spec §10): Triangle selects the next owned one, Square drinks a potion (healing at the drink's end)
        // or starts a buff's 120 s at once.
        if (a.item) {
            int next = -1;
            for (int k = 1; k <= ITEM_COUNT && next < 0; k++)
                if (s.items[(q.selectedItem + k) % ITEM_COUNT] > 0) next = (q.selectedItem + k) % ITEM_COUNT;
            if (next >= 0) q.selectedItem = next, q.soundRequested[SOUND_Q_BLIP] = true;
            else q.soundRequested[SOUND_ROCKET_EMPTY] = true;
        }
        if (a.cycle && h.drinkTimer <= 0) {
            int item = q.selectedItem;
            if (s.items[item] <= 0) {
                q.soundRequested[SOUND_ROCKET_EMPTY] = true;
            } else {
                s.items[item]--;
                if (item <= ITEM_MANA) {
                    h.drinkTimer = DRINK_TIME;
                    h.drinkItem = item;
                    q.soundRequested[SOUND_Q_DRINK] = true;
                } else {
                    h.buffTimers[item - ITEM_WHETSTONE] = BUFF_TIME;
                    q.soundRequested[SOUND_PICKUP] = true;
                }
                for (int k = 0; k < ITEM_COUNT && s.items[q.selectedItem] == 0; k++)   // the last one used: select the next owned
                    if (s.items[(item + k) % ITEM_COUNT] > 0) q.selectedItem = (item + k) % ITEM_COUNT;
            }
        }
        if (h.drinkTimer > 0 && (h.drinkTimer -= dt) <= 0) {
            if (h.drinkItem == ITEM_MANA) h.mp = std::min(q.stats.maxMp, h.mp + (int)(q.stats.maxMp * POTION_HEAL[2]));
            else h.hp = std::min(q.stats.maxHp, h.hp + (int)(q.stats.maxHp * POTION_HEAL[h.drinkItem]));
            h.drinkTimer = 0;
        }

        bool wasOnGround = h.onGround;
        float fallSpeed = h.vy;
        updateCombat(q, move, dt);
        if (h.jumped) q.soundRequested[SOUND_Q_JUMP] = true;
        if (!wasOnGround && h.onGround && fallSpeed > LAND_SOUND_SPEED) q.soundRequested[SOUND_Q_LAND] = true;
        if (h.onGround && fabsf(h.vx) > 10 * DP && move.moveX != 0) {
            h.stepTimer -= dt;
            if (h.stepTimer <= 0) {
                q.soundRequested[worldDef(q.area.world).style.step] = true;
                h.stepTimer = STEP_TIME;
            }
        } else {
            h.stepTimer = 0;
        }

        // Edges: the hero's centre past an open edge goes through its link; below a closed bottom is a pit.
        float cx = h.x + HERO_W / 2, cy = h.y + HERO_H / 2;
        const AreaLink* exit = cx < 0 ? &def.left : cx > def.cols * TILE ? &def.right : cy < 0 ? &def.up
                             : cy > def.rows * TILE ? &def.down : nullptr;
        if (exit && exit->area >= 0) {
            bool sideways = exit == &def.left || exit == &def.right;
            if (sideways) {
                h.vy = 0;
                h.facingLeft = exit == &def.left;
            }
            enterArea(q, q.area.world, exit->area, exit->tileX, exit->tileY);
            break;
        }
        if (exit == &def.down) die(q, "pit");
        else if (heroInLiquid(h, q.area)) die(q, "liquid");
        else if (h.hp <= 0) die(q, "hp");

        // Touching the name echo gives the names back.
        if (q.mode == QMODE_PLAY && s.echoNames > 0 && s.echoWorld == q.area.world && s.echoArea == q.area.index &&
            h.x < s.echoX + ECHO_SIZE && s.echoX < h.x + HERO_W && h.y < s.echoY + ECHO_SIZE && s.echoY < h.y + HERO_H) {
            s.names = s.names > MAX_NAMES - s.echoNames ? MAX_NAMES : s.names + s.echoNames;
            q.numbers.push_back({h.x + HERO_W / 2, h.y, 0, s.echoNames, paletteColor('y'), NAMES_TEXT_TIME, true});
            q.soundRequested[SOUND_Q_NAMES] = true;
            logLine(q, "quest: names recovered %d", s.echoNames);
            s.echoNames = 0;
            questSave(q);
        }
        if (q.mode == QMODE_PLAY) updateCamera(q, dt, false);
        break;
    }
    case QMODE_DIALOGUE: {   // spec §14: types at 50 chars/s; X completes the page or turns it; Circle closes
        q.save.playtime += dt;
        updateEffects(q, dt);
        DialogueState& d = q.dialogue;
        float length = (float)d.pages[d.page].size();
        int before = (int)d.typed;
        d.typed = fminf(length, d.typed + TALK_CHARS_PER_S * dt);
        if ((int)d.typed / 3 > before / 3) q.soundRequested[SOUND_Q_TALK] = true;
        if (a.back) {
            q.mode = QMODE_PLAY;
        } else if (a.jump && d.typed < length) {
            d.typed = length;
        } else if (a.jump && d.page + 1 < (int)d.pages.size()) {
            d.page++;
            d.typed = 0;
        } else if (a.jump) {
            q.mode = QMODE_PLAY;
            switch (d.after) {
            case AFTER_POTION_SHOP:
            case AFTER_GEAR_SHOP:
                q.shopKind = d.after;
                q.shopCursor = 0;
                q.mode = QMODE_SHOP;
                break;
            case AFTER_LEVEL_UP: openMenu(q, MENU_LEVEL_UP, {"LEVEL UP", "LEAVE"}); break;
            case AFTER_JOB: openMenu(q, MENU_JOB_CONFIRM, {"YES", "NOT YET"}); break;
            case AFTER_MAP: q.mode = QMODE_MAP; break;
            case AFTER_YES_NO: openMenu(q, MENU_YES_NO, {"YES", "NOT YET"}); break;
            }
        }
        break;
    }
    case QMODE_MENU: {
        q.save.playtime += dt;
        updateEffects(q, dt);
        MenuState& m = q.menu;
        int count = (int)m.items.size();
        if (a.up || a.down) {
            m.cursor = (m.cursor + (a.up ? count - 1 : 1)) % count;
            m.scroll = std::max(std::min(m.scroll, m.cursor), m.cursor - MENU_ROWS + 1);
            q.soundRequested[SOUND_Q_BLIP] = true;
        }
        if (a.back) {
            q.mode = QMODE_PLAY;
        } else if (a.jump) {
            switch (m.kind) {
            case MENU_CHECKPOINT:
                if (m.cursor == 0) {
                    rest(q, q.atCheckpoint);
                    q.mode = QMODE_PLAY;
                } else if (m.cursor == 1) {
                    std::vector<std::string> names;
                    for (int bit : travelCheckpoints(q.save)) names.push_back(bit == 0 ? "NAMULA" : checkpointName(bit));
                    openMenu(q, MENU_TRAVEL, names);
                } else {
                    q.mode = QMODE_PLAY;
                }
                break;
            case MENU_TRAVEL: {   // moves the hero to that checkpoint and rests there; buffs keep running
                int bit = travelCheckpoints(q.save)[m.cursor];
                float buffs[3] = {q.hero.buffTimers[0], q.hero.buffTimers[1], q.hero.buffTimers[2]};
                q.save.checkpointWorld = bit / 2;
                q.save.checkpointId = bit % 2;
                logLine(q, "quest: travel to %d.%d", bit / 2, bit % 2);
                enterAtCheckpoint(q);
                for (int k = 0; k < 3; k++) q.hero.buffTimers[k] = buffs[k];
                rest(q, bit);
                q.soundRequested[SOUND_Q_PORTAL] = true;
                break;
            }
            case MENU_LEVEL_UP:   // the menu stays open, so several levels can be bought in one visit
                if (m.cursor == 0) levelUp(q);
                else q.mode = QMODE_PLAY;
                break;
            case MENU_JOB_CONFIRM:
                if (m.cursor == 0) grantJob(q);
                q.mode = QMODE_PLAY;
                break;
            case MENU_YES_NO: q.mode = QMODE_PLAY; break;   // phase 8: Claude's New Game+
            }
        }
        break;
    }
    case QMODE_SHOP: {
        q.save.playtime += dt;
        int rows = (int)shopRows(q.save, q.shopKind).size();
        if ((a.up || a.down) && rows > 0) {
            q.shopCursor = (q.shopCursor + (a.up ? rows - 1 : 1)) % rows;
            q.soundRequested[SOUND_Q_BLIP] = true;
        }
        if (a.back) q.mode = QMODE_PLAY;
        else if (a.jump) shopBuy(q, q.shopCursor);
        break;
    }
    case QMODE_MAP:   // the game stays frozen while the map is open
        q.save.playtime += dt;
        if (a.back || a.jump) q.mode = QMODE_PLAY;
        break;
    case QMODE_CREDITS: break;   // phase 8
    case QMODE_DEAD:
        updateEffects(q, dt);
        if (q.modeTimer >= DEATH_TIME) {   // respawn: every enemy back, full HP and MP, buffs off
            for (auto& killed : q.killed) killed.assign(killed.size(), 0);
            q.save.deaths++;
            enterAtCheckpoint(q);
            questSave(q);
        }
        break;
    }
}
