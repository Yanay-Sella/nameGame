// NAME QUEST stats and formulas (spec §6, §7, §10): pure functions and tables, no state.
#include "quest.h"
#include <math.h>

const ClassInfo CLASSES[CLASS_COUNT] = {
    // name        hp  +   mp  +    atk +    def  +    walk regen crit critMult
    {"KNIGHT",   120, 14, 30, 2,   10, 2.0f, 8, 1.0f, 66, 1.0f, 0.05f, 1.5f},
    {"ARCHER",    85, 10, 40, 3,    9, 2.0f, 4, 0.6f, 72, 1.2f, 0.05f, 1.5f},
    {"MAGICIAN",  70,  8, 80, 6,   11, 2.3f, 3, 0.5f, 68, 2.5f, 0.05f, 1.5f},
    {"THIEF",     90, 11, 35, 2.5f, 8, 1.9f, 5, 0.7f, 80, 1.0f, 0.10f, 1.8f},
};
const char* const JOB_NAMES[CLASS_COUNT][4] = {{"KNIGHT", "VANGUARD", "WARDEN", "LIONHEART"},
                                               {"ARCHER", "RANGER", "SHARPSHOOTER", "STORMBOW"},
                                               {"MAGICIAN", "ADEPT", "ELEMENTALIST", "ARCHMAGE"},
                                               {"THIEF", "ROGUE", "SHADOW", "NIGHTBLADE"}};
const char* const SKILL_NAMES[CLASS_COUNT][4] = {{"POWER SLASH", "RISING SLASH", "GROUND QUAKE", "LION'S ROAR"},
                                                 {"DOUBLE SHOT", "ARROW RAIN", "PIERCING BOLT", "STORM VOLLEY"},
                                                 {"ARCANE BURST", "FROST ORB", "CHAIN LIGHTNING", "METEOR"},
                                                 {"STAR TOSS", "SHADOW DASH", "BLADE FURY", "SHADOW PARTNER"}};

// Gear by tier 1..7 (index tier - 1), spec §10.
const int WEAPON_ATK[7] = {3, 8, 15, 24, 35, 48, 62};
const int ARMOR_DEF[6] = {3, 7, 12, 18, 25, 33};
const int ARMOR_HP[6] = {10, 30, 60, 100, 150, 210};
// Reference hero per world 1..7 (index 0 unused), spec §7.
const int REF_LEVEL[8] = {0, 4, 11, 17, 23, 29, 35, 40};
const int REF_GEAR[8] = {0, 0, 1, 2, 3, 4, 5, 6};
const int WORLD_NAMES[8] = {0, 4, 12, 35, 100, 300, 900, 1500};
// Shops (spec §10): item base prices, the price scale by highest world, gear prices by tier 1..6, item carry limits.
const int ITEM_PRICE[ITEM_COUNT] = {15, 45, 20, 60, 60, 40};
const int PRICE_SCALE[8] = {1, 1, 3, 9, 26, 75, 220, 360};   // index 0 unused (highest starts at 1)
const int WEAPON_PRICE[6] = {150, 1400, 4500, 14000, 42000, 130000};
const int ARMOR_PRICE[6] = {120, 1100, 3600, 11000, 34000, 105000};
const int CARRY_START[ITEM_COUNT] = {5, 2, 3, 3, 3, 3};

float jobMultiplier(int job) {
    return 1 + 0.1f * (job < 0 ? 0 : job > 3 ? 3 : job);
}

HeroStats heroStats(const SaveData& save) {
    const ClassInfo& c = CLASSES[save.heroClass >= 0 && save.heroClass < CLASS_COUNT ? save.heroClass : 0];
    int level = save.level < 1 ? 1 : save.level > MAX_LEVEL ? MAX_LEVEL : save.level;
    int job = save.job < 0 ? 0 : save.job > 3 ? 3 : save.job;
    HeroStats s;
    s.maxHp = (int)floorf(c.hp + c.hpG * (level - 1));
    s.maxMp = (int)floorf(c.mp + c.mpG * (level - 1));
    s.atk = (int)floorf(c.atk + c.atkG * (level - 1));
    s.def = (int)floorf(c.def + c.defG * (level - 1));
    s.maxHp = (int)floorf(s.maxHp * jobMultiplier(job));
    s.atk = (int)floorf(s.atk * jobMultiplier(job));
    if (save.weapon >= 1 && save.weapon <= 7) s.atk += (int)floorf(WEAPON_ATK[save.weapon - 1] * powf(1.1f, (float)save.refine));
    if (save.armor >= 1 && save.armor <= 6) {
        s.def += ARMOR_DEF[save.armor - 1];
        s.maxHp += ARMOR_HP[save.armor - 1];
    }
    if (save.keys >> KEY_TOBIN_CHARM & 1) s.maxHp = (int)floorf(s.maxHp * 1.05f);
    s.walk = c.walk;
    s.mpRegen = c.mpRegen + 0.05f * (level - 1);
    s.crit = c.crit + (save.heroClass == CLASS_THIEF ? 0.05f * job : 0);
    s.critMult = c.critMult;
    return s;
}

RefStats referenceHero(int world) {
    world = world < 1 ? 1 : world > 7 ? 7 : world;
    RefStats r = {0, 0, 0};
    for (int c = 0; c < CLASS_COUNT; c++) {
        SaveData s;
        s.heroClass = c;
        s.level = REF_LEVEL[world];
        s.job = s.level >= 30 ? 3 : s.level >= 15 ? 2 : s.level >= 5 ? 1 : 0;
        s.weapon = s.armor = REF_GEAR[world];
        HeroStats h = heroStats(s);
        r.hp += h.maxHp / 4.0f;
        r.atk += h.atk / 4.0f;
        r.def += h.def / 4.0f;
    }
    return r;
}

static float ngFactor(int ngPlus) {
    return 1 + 0.5f * (ngPlus < 0 ? 0 : ngPlus > 3 ? 3 : ngPlus);
}

int enemyAttack(int world, float hitFraction, int ngPlus) {
    RefStats r = referenceHero(world);
    return (int)lroundf(hitFraction * r.hp * (100 + r.def) / 100 * ngFactor(ngPlus));
}

int enemyHealth(int world, float hitsToKill, int ngPlus) {
    return (int)lroundf(hitsToKill * referenceHero(world).atk * ngFactor(ngPlus));
}

int enemyNames(int world, float namesMult, int ngPlus, bool strangerRing) {
    world = world < 1 ? 1 : world > 7 ? 7 : world;
    return (int)lroundf(WORLD_NAMES[world] * namesMult * ngFactor(ngPlus) * (strangerRing ? 1.15f : 1));
}

int levelCost(int level) {
    return (int)lround(100 * pow(1.2, level - 1));
}

int damageToHero(int enemyAtk, int heroDef) {
    int d = (int)lroundf(enemyAtk * 100.0f / (100 + heroDef));
    return d < 1 ? 1 : d;
}

int damageToEnemy(float heroAtk, float mult, float roll, bool crit, float critMult, float enemyMod) {
    int d = (int)lroundf(heroAtk * mult * roll * (crit ? critMult : 1) * enemyMod);
    return d < 1 ? 1 : d;
}

int itemPrice(const SaveData& save, int item) {
    int highest = save.highest < 1 ? 1 : save.highest > 7 ? 7 : save.highest;
    return ITEM_PRICE[item] * PRICE_SCALE[highest];
}

int carryLimit(const SaveData& save, int item) {
    bool locket = save.keys >> KEY_LOCKET & 1;
    int limit = CARRY_START[item];
    if (item == ITEM_RED) limit += save.shards + locket;
    if (item == ITEM_BIG) limit += (int)(save.keys >> KEY_GARRICK_GIFT & 1) + locket;
    if (item == ITEM_MANA) limit += locket;
    return limit;
}

int gearPrice(int tier, bool weapon) {
    tier = tier < 1 ? 1 : tier > 6 ? 6 : tier;
    return weapon ? WEAPON_PRICE[tier - 1] : ARMOR_PRICE[tier - 1];
}
