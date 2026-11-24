// Renders the NAME QUEST guide's pictures and facts with the game's own code (make_quest_guide.py builds and runs it).
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string>
#include <algorithm>
#include <vector>
#include "quest.h"
#include "screen.h"
#include "quest_sprites.h"
#include "quest_sprites_hero.h"
#include "quest_sprites_0.h"
#include "quest_sprites_1.h"
#include "quest_sprites_2.h"
#include "quest_sprites_3.h"

static const char* OUT;
static const uint32_t KEY = 0xFF01FE02;   // see-through in the PNGs (make_quest_guide.py)
static const char TRIM_BY_JOB[4] = {'h', 'b', 'Y', 'N'};   // as quest_draw.cpp

static void save(const char* name) {
    char path[512];
    snprintf(path, sizeof path, "%s/%s.bmp", OUT, name);
    if (!screenSaveBmp(path)) printf("could not write %s\n", path);
}

// One sprite alone on the key colour, `outline` around it when not 0.
static void saveSprite(const Sprite& s, const char* name, uint32_t outline) {
    int w = (s.w + 7) & ~3;   // screenSaveBmp writes unpadded rows, so keep the width a multiple of 4
    std::vector<uint32_t> pixels(w * (s.h + 4), KEY);
    screenSetTarget(pixels.data(), w, s.h + 4);
    if (outline) drawSpriteOutlined(s, 2, 2, false, outline);
    else drawSprite(s, 2, 2);
    save(name);
    screenSetTarget(questPixels, QUEST_W, QUEST_H);
}

// A hero sprite with its job's trim and details, as drawHeroSprite in quest_draw.cpp.
static void saveHero(const Sprite& s, int job, const char* name) {
    int w = (s.w + 7) & ~3;
    std::vector<uint32_t> pixels(w * (s.h + 4), KEY);
    for (int row = 0; row < s.h; row++)
        for (int col = 0; col < s.w; col++) {
            char c = s.pixels[row * s.w + col];
            if (c == '.' || (c >= '1' && c <= '3' && job < c - '0')) continue;
            pixels[(row + 2) * w + col + 2] = c == 'h' || (c >= '1' && c <= '3') ? paletteColor(TRIM_BY_JOB[job]) : paletteColor(c);
        }
    screenSetTarget(pixels.data(), w, s.h + 4);
    save(name);
    screenSetTarget(questPixels, QUEST_W, QUEST_H);
}

static int countTiles(const AreaDef& a, char letter) {
    int n = 0;
    for (int i = 0; i < a.cols * a.rows; i++) n += a.tiles[i] == letter;
    return n;
}

static void startDev(Quest& q, int heroClass, int world, int area, int level, int job) {
    q = Quest();
    q.folder = OUT;
    char spec[128];
    snprintf(spec, sizeof spec, "slot=3 class=%d world=%d area=%d level=%d job=%d gear=%d", heroClass, world, area, level, job, world);
    questStartDev(q, spec);
    q.godMode = true;
    q.bannerTimer = 0;
}

static void step(Quest& q, const Actions& a, float seconds) {
    for (int i = 0; i < (int)(seconds * 60 + 0.5f); i++) questUpdate(q, a, 1.0f / 60);
}

static void writeFacts(FILE* f) {
    fprintf(f, "{\n\"classes\": [");
    for (int c = 0; c < CLASS_COUNT; c++) {
        const ClassInfo& k = CLASSES[c];
        fprintf(f, "%s\n {\"name\": \"%s\", \"hp\": [%g, %g], \"mp\": [%g, %g], \"atk\": [%g, %g], \"def\": [%g, %g], \"walk\": %g, "
                   "\"mpRegen\": %g, \"crit\": %g, \"critMult\": %g, \"jobs\": [", c ? "," : "", k.name, k.hp, k.hpG, k.mp, k.mpG,
                k.atk, k.atkG, k.def, k.defG, k.walk, k.mpRegen, k.crit, k.critMult);
        for (int j = 0; j < 4; j++) fprintf(f, "%s\"%s\"", j ? ", " : "", JOB_NAMES[c][j]);
        fprintf(f, "], \"skills\": [");
        for (int j = 0; j < 4; j++) fprintf(f, "%s\"%s\"", j ? ", " : "", SKILL_NAMES[c][j]);
        fprintf(f, "], \"weapons\": [");
        for (int j = 0; j < 7; j++) fprintf(f, "%s\"%s\"", j ? ", " : "", WEAPON_NAMES[c][j]);
        fprintf(f, "]}");
    }
    fprintf(f, "],\n\"armor\": [");
    for (int j = 0; j < 6; j++) fprintf(f, "%s\"%s\"", j ? ", " : "", ARMOR_NAMES[j]);
    fprintf(f, "],\n\"npcs\": [");
    for (int i = 0; i < NPC_COUNT; i++) fprintf(f, "%s\"%s\"", i ? ", " : "", npcName(i));
    fprintf(f, "],\n\"levelCost\": [");
    for (int l = 1; l < MAX_LEVEL; l++) fprintf(f, "%s%d", l > 1 ? ", " : "", levelCost(l));
    fprintf(f, "],\n\"worlds\": [");
    for (int w = 0; w <= 7; w++) {
        const WorldDef& wd = worldDef(w);
        RefStats ref = referenceHero(w < 1 ? 1 : w);
        fprintf(f, "%s\n {\"name\": \"%s\", \"ref\": [%g, %g, %g], \"areas\": [", w ? "," : "", wd.name, ref.hp, ref.atk, ref.def);
        for (int i = 0; i < wd.count; i++) {
            const AreaDef& a = wd.areas[i];
            fprintf(f, "%s\n  {\"name\": \"%s\", \"wide\": %s, \"mapX\": %d, \"mapY\": %d, \"band\": %d, \"flags\": %d, \"links\": [%d, %d, %d, %d], "
                       "\"portals\": [%d, %d, %d, %d], \"npcs\": [", i ? "," : "", a.name, a.cols == WIDE_COLS ? "true" : "false",
                    a.mapX, a.mapY, a.band, a.flags, a.left.area, a.right.area, a.up.area, a.down.area,
                    a.portals[0].area, a.portals[1].area, a.portals[2].area, a.portals[3].area);
            for (int k = 0, n = 0; k < 8; k++)
                if (a.npcs[k] >= 0) fprintf(f, "%s%d", n++ ? ", " : "", a.npcs[k]);
            fprintf(f, "], \"chests\": [");
            for (int k = 0, n = 0; k < 4; k++) {
                if (a.chests[k] < 0) continue;
                const ChestContent& c = chestContent(w, a.chests[k]);
                std::string what = c.kind == CHEST_ITEM ? ITEM_NAMES[c.item] : c.kind == CHEST_NAMES ? "NAMES"
                                 : c.kind == CHEST_KEY ? KEY_NAMES[c.item] : c.kind == CHEST_SHARD ? "VIAL SHARD" : "MAP PAGE";
                fprintf(f, "%s[\"%s\", %d]", n++ ? ", " : "", what.c_str(), c.count);
            }
            fprintf(f, "], \"tiles\": {");
            const char* letters = "ABDEFCTMgKPNb^zv~|icYR<>";
            for (int k = 0; letters[k]; k++) {
                char key[3] = {letters[k], 0, 0};
                if (letters[k] == '\\' || letters[k] == '"') key[0] = '\\', key[1] = letters[k];
                fprintf(f, "%s\"%s\": %d", k ? ", " : "", key, countTiles(a, letters[k]));
            }
            fprintf(f, "}}");
        }
        fprintf(f, "], \"roster\": [");
        if (w >= 1 && w <= 3) {
            // Stats from real spawns: the first of each roster slot met in the world's areas.
            for (int slot = 0; slot < ROSTER_SLOTS; slot++) {
                const EnemyType& t = rosterType(w, slot);
                int hp = 0, atk = 0, names = 0;
                for (int i = 0; i < wd.count && !hp; i++) {
                    Quest q = Quest();
                    q.save.used = true;
                    loadArea(q.area, w, i, 0, 0);
                    questRefreshStats(q, true);
                    q.hero.x = q.hero.y = -1000;
                    spawnEnemies(q);
                    for (const Enemy& e : q.enemies)
                        if (e.type == slot && !hp) hp = e.maxHp, atk = e.atk, names = e.names;
                }
                fprintf(f, "%s\n  {\"name\": \"%s\", \"behaviour\": %d, \"kind\": %d, \"trait\": %d, \"hp\": %d, \"atk\": %d, \"names\": %d, "
                           "\"speed\": %g}", slot ? "," : "", t.name, t.behaviour, t.kind, t.trait, hp, atk, names, t.speedScale);
            }
        }
        fprintf(f, "], \"bosses\": [");
        int n = 0;
        for (int i = 0; i < wd.count && w >= 1 && w <= 3; i++) {
            if (!(wd.areas[i].flags & (AREA_MINIBOSS | AREA_BOSS))) continue;
            Quest q = Quest();
            q.save.used = true;
            loadArea(q.area, w, i, 0, 0);
            questRefreshStats(q, true);
            q.hero.x = q.hero.y = -1000;
            startArenaFight(q);
            for (const QuestBoss& b : q.bosses)
                fprintf(f, "%s\n  {\"id\": %d, \"name\": \"%s\", \"area\": %d, \"big\": %s, \"hp\": %d, \"atk\": %d, \"bigAtk\": %d, \"contact\": %d, "
                           "\"names\": %d, \"gap\": %g}", n++ ? "," : "", b.id, bossName(b.id), i, b.big ? "true" : "false", b.maxHp,
                        b.atk, b.bigAtk, b.contactAtk, b.names, b.gap);
        }
        fprintf(f, "]}");
    }
    fprintf(f, "]\n}\n");
}

int main(int argc, char** argv) {
    OUT = argv[1];
    char name[128];
    char path[512];
    snprintf(path, sizeof path, "%s/facts.json", OUT);
    FILE* f = fopen(path, "w");
    writeFacts(f);
    fclose(f);

    // Heroes: idle at every job tier, plus a mid-strike pose and a jump.
    const char* const CLASS_FILES[CLASS_COUNT] = {"knight", "archer", "magician", "thief"};
    for (int c = 0; c < CLASS_COUNT; c++) {
        for (int job = 0; job < 4; job++) {
            snprintf(name, sizeof name, "hero_%s_%d", CLASS_FILES[c], job);
            saveHero(*HERO_BODY[c][BODY_IDLE_1], job, name);
            snprintf(name, sizeof name, "hero_%s_%d_strike", CLASS_FILES[c], job);
            saveHero(*HERO_ARMS[c][ARMS_BASIC + 4][LEGS_STAND], job, name);
            snprintf(name, sizeof name, "hero_%s_%d_run", CLASS_FILES[c], job);
            saveHero(*HERO_BODY[c][BODY_RUN_2], job, name);
        }
    }
    // Enemies (worlds 1-3) with the in-game outline, and the bosses' six frames.
    for (int w = 1; w <= 3; w++)
        for (int slot = 0; slot < ROSTER_SLOTS; slot++) {
            const EnemyType& t = rosterType(w, slot);
            snprintf(name, sizeof name, "enemy_%d_%d", w, slot);
            saveSprite(*t.frames[0], name, paletteColor('x'));
            if (t.tell) { snprintf(name, sizeof name, "enemy_%d_%d_tell", w, slot); saveSprite(*t.tell, name, paletteColor('x')); }
            if (t.attack) { snprintf(name, sizeof name, "enemy_%d_%d_attack", w, slot); saveSprite(*t.attack, name, paletteColor('x')); }
        }
#define BOSS_SPRITES(N, I) \
    saveSprite(BOSS_##N##_IDLE_1, "boss_" #I "_idle", 0); saveSprite(BOSS_##N##_TELL, "boss_" #I "_tell", 0); \
    saveSprite(BOSS_##N##_ATTACK, "boss_" #I "_attack", 0); saveSprite(BOSS_##N##_PHASE2, "boss_" #I "_phase2", 0);
    BOSS_SPRITES(OVERSEER, 0) BOSS_SPRITES(STAR_MAGE, 1) BOSS_SPRITES(MONKEY_KING, 2)
    BOSS_SPRITES(SILVERBACK, 3) BOSS_SPRITES(YETI, 4) BOSS_SPRITES(TITAN, 5)
    // The people met so far (Calder is in world 5; the last story NPC stays unseen).
    for (int id = NPC_ILSA; id <= NPC_STRANGER; id++) {
        snprintf(name, sizeof name, "npc_%d", id);
        saveSprite(*NPC_FRAMES[id][0], name, 0);
    }
    saveSprite(*NPC_FRAMES[NPC_SURVEYOR_FIRST][0], "npc_surveyor", 0);
    saveSprite(CHECKPOINT_LIT_A, "checkpoint", 0);

    // Every area: its two screens, enemies at their spots, no hero (as `make quest-maps`).
    screenSetTarget(questPixels, QUEST_W, QUEST_H);
    for (int w = 0; w <= 3; w++)
        for (int i = 0; i < worldDef(w).count; i++) {
            Quest q = Quest();
            q.folder = OUT;
            q.save.used = true;
            q.save.quests[QUEST_TOBIN] = w == 0 ? 2 : 0;   // Garrick on the Shore, not in the Greendeep
            q.save.bosses = w == 0 ? 1u << 3 : 0;         // and the Stranger arrived
            loadArea(q.area, w, i, 0, 0);
            questRefreshStats(q, true);
            q.hero.x = q.hero.y = -1000;
            spawnEnemies(q);
            startArenaFight(q);
            q.bannerTimer = 0;
            q.time = 1;
            q.mode = QMODE_PLAY;
            const AreaDef& def = *q.area.def;
            for (int k = 0; k < 2; k++) {
                q.camX = def.cols == WIDE_COLS ? (float)(k * (def.cols * TILE - QUEST_W)) : 0;
                q.camY = def.cols == WIDE_COLS ? (float)(def.rows * TILE - QUEST_H) : (float)(k * (def.rows * TILE - QUEST_H));
                // No HUD: the top and bottom bands come from two more draws with the camera 40 dp up and down.
                static uint32_t clean[QUEST_W * QUEST_H];
                const int BAND = 40 * DP;
                float camY = q.camY;
                q.camY = camY - BAND;
                questDraw(q);
                memcpy(clean, questPixels + BAND * QUEST_W, BAND * QUEST_W * sizeof(uint32_t));
                q.camY = camY + BAND;
                questDraw(q);
                memcpy(clean + (QUEST_H - BAND) * QUEST_W, questPixels + (QUEST_H - 2 * BAND) * QUEST_W, BAND * QUEST_W * sizeof(uint32_t));
                q.camY = camY;
                questDraw(q);
                memcpy(questPixels, clean, BAND * QUEST_W * sizeof(uint32_t));
                memcpy(questPixels + (QUEST_H - BAND) * QUEST_W, clean + (QUEST_H - BAND) * QUEST_W, BAND * QUEST_W * sizeof(uint32_t));
                snprintf(name, sizeof name, "area_%d_%02d_%d", w, i, k);
                save(name);
            }
        }

    // Strong attacks: each class and tier casting R2 beside the first ground enemy of world 1's cargo hold.
    for (int c = 0; c < CLASS_COUNT; c++)
        for (int job = 0; job < 4; job++) {
            Quest q;
            int world = job < 2 ? 1 : 2, area = job == 0 ? 1 : job == 1 ? 2 : job == 2 ? 4 : 2;   // a different place per tier
            startDev(q, c, world, area, 30, job);
            q.selectedSkill = job;
            const Enemy* target = nullptr;
            for (const Enemy& e : q.enemies)
                if (e.alive && !target && rosterType(e.world, e.type).behaviour != BEH_FLYER &&
                    rosterType(e.world, e.type).behaviour != BEH_DROPPER) target = &e;
            if (target) {
                q.hero.x = target->x - 46 * DP;
                q.hero.y = target->y + target->h - HERO_H;
            }
            q.hero.facingLeft = false;
            Actions none = {};
            step(q, none, 0.4f);
            for (int k = 0; k < 7; k++) {
                for (Enemy& e : q.enemies) e.hp = e.maxHp;
                Actions cast = {};
                cast.rocket = k == 0;
                cast.firePressed = c == CLASS_THIEF && job == 3 && k % 2 == 1;   // the Shadow Partner repeats strikes
                q.hero.mp = q.stats.maxMp;
                step(q, cast, k == 0 ? 1.0f / 60 : 0.1f);
                questDraw(q);
                // A 400 x 225 crop with the hero left of centre, where the skill goes.
                static uint32_t crop[400 * 225];
                int x0 = std::max(0, std::min(QUEST_W - 400, (int)(q.hero.x - q.camX) - 70));
                int y0 = std::max(0, std::min(QUEST_H - 225, (int)(q.hero.y - q.camY) - 150));
                for (int y = 0; y < 225; y++) memcpy(&crop[y * 400], &questPixels[(y0 + y) * QUEST_W + x0], 400 * sizeof(uint32_t));
                screenSetTarget(crop, 400, 225);
                snprintf(name, sizeof name, "skill_%s_%d_%d", CLASS_FILES[c], job, k);
                save(name);
                screenSetTarget(questPixels, QUEST_W, QUEST_H);
            }
        }

    // Boss fights: the hero walks in and stands; a frame every 0.4 s, with phase 2 forced half way.
    for (int w = 1; w <= 3; w++)
        for (int i = 0; i < worldDef(w).count; i++) {
            const AreaDef& def = worldDef(w).areas[i];
            if (!(def.flags & (AREA_MINIBOSS | AREA_BOSS))) continue;
            Quest q;
            startDev(q, (w + i) % CLASS_COUNT, w, i, 12, 1);
            if (q.bosses.empty()) continue;
            int id = q.bosses[0].id;
            for (int k = 0; k < 48; k++) {
                if (k == 24)
                    for (QuestBoss& b : q.bosses) b.hp = b.maxHp / 2 - 1;
                for (QuestBoss& b : q.bosses) if (k < 24) b.hp = b.maxHp;
                for (int t = 0; t < 24; t++) {   // 0.4 s, keeping about 110 dp from the boss
                    const QuestBoss& b = q.bosses[0];
                    float dx = b.x + b.w / 2 - q.hero.x;
                    Actions a = {};
                    a.moveX = fabsf(dx) > 130 * DP ? (dx > 0 ? 1 : -1) : fabsf(dx) < 90 * DP ? (dx > 0 ? -1 : 1) : 0;
                    questUpdate(q, a, 1.0f / 60);
                }
                // The camera between hero and boss (on the boss when they are too far apart).
                const QuestBoss& b = q.bosses[0];
                float bx = b.x + b.w / 2, hx = q.hero.x + HERO_W / 2;
                float cx = fabsf(bx - hx) < QUEST_W - 80 * DP ? (bx + hx) / 2 : bx;
                q.camX = std::max(0.0f, std::min((float)(def.cols * TILE - QUEST_W), cx - QUEST_W / 2));
                q.bannerTimer = 0;
                questDraw(q);
                snprintf(name, sizeof name, "fight_%d_%02d", id, k);
                save(name);
            }
        }
    return 0;
}
