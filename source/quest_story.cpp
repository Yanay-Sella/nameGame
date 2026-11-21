// NAME QUEST's people and economy (spec §10, §14): NPC names and lines, Ilsa's levels, the mentors' jobs, both shops, chests.
#include "quest.h"
#include "screen.h"
#include <math.h>
#include <stdio.h>

const char* const ITEM_NAMES[ITEM_COUNT] = {"RED POTION", "BIG POTION", "MANA POTION", "WHETSTONE", "IRON TONIC", "SWIFT FEATHER"};
const char* const KEY_NAMES[KEY_COUNT] = {"STAR IRON", "FROZEN FEATHER", "GARRICK'S PICKAXE", "HEART OF MAGMA", "THE STRANGER'S LETTER",
                                          "CALDER'S RING", "TOBIN'S CHARM", "RING OF THE STRANGER", "SELA'S LOCKET", "GARRICK'S GIFT",
                                          "MAP PAGE 1", "MAP PAGE 2", "MAP PAGE 3", "MAP PAGE 4", "MAP PAGE 5", "TRIAL OF ECHOES",
                                          "THE UNWRITTEN", "THE CREDITS"};
// Gear names by class and tier 1..7 (tier 7: the class's secret weapon, spec §10); armor is shared, tiers 1..6.
const char* const WEAPON_NAMES[CLASS_COUNT][7] = {
    {"IRON SWORD", "STEEL SWORD", "KNIGHT'S BLADE", "RUNE SWORD", "EMBER BRAND", "OATHKEEPER", "DAWNBREAKER"},
    {"ASH BOW", "YEW BOW", "HUNTER'S BOW", "RUNE BOW", "EMBER BOW", "SKYPIERCER", "FIRST LIGHT"},
    {"OAK STAFF", "ACOLYTE ROD", "SAGE STAFF", "RUNE STAFF", "EMBER ROD", "STARWEAVE", "THE OPEN BOOK"},
    {"IRON DAGGER", "STEEL DIRK", "QUICK KNIFE", "RUNE DAGGER", "EMBER FANG", "WHISPER", "NAMELESS EDGE"}};
const char* const ARMOR_NAMES[6] = {"PADDED COAT", "LEATHER VEST", "CHAIN SHIRT", "SCALE MAIL", "PLATE MAIL", "NAMEWOVEN MAIL"};

static const char* const NPC_NAMES[NPC_COUNT] = {
    "ILSA, KEEPER OF NAMES", "MARRA THE HERBALIST", "BRANN THE SMITH", "LIO THE BARD", "PELL THE CARTOGRAPHER", "OLD SELA",
    "SIR ODO", "WREN", "YSOLDE", "KIT", "THE GATEKEEPER", "TOBIN", "GARRICK", "THE STRANGER", "SER CALDER, THE DROWNED KNIGHT",
    "CLAUDE", "SURVEYOR ANDO", "SURVEYOR BETT", "SURVEYOR CORIN", "SURVEYOR DESS", "SURVEYOR EMRYS", "SURVEYOR FAYE",
    "SURVEYOR GILD", "SURVEYOR HOLLIS", "SURVEYOR IVO", "SURVEYOR JUNE", "SURVEYOR KESTREL", "SURVEYOR LORNE"};
// One line each, then the world map opens (spec §11): flavour about getting lost.
static const char* const SURVEYOR_LINES[12] = {
    "I'VE MAPPED THIS PLACE THREE TIMES.\nIT DISAGREES WITH ME EVERY TIME. HERE, TAKE A LOOK.",
    "LOST? GOOD. THAT MEANS YOU'VE GONE SOMEWHERE NEW.",
    "THE PATHS HERE GROW BACK WHEN NO ONE WATCHES.\nI WATCH. MOSTLY.",
    "EVERY DOOR I DRAW, SOMEONE ELSE FINDS A WAY AROUND.\nMAYBE THAT SOMEONE IS YOU.",
    "DON'T TRUST THE PORTALS. THEY GO WHERE THEY LIKE.\nTRUST THE MAP. IT ONLY LIES A LITTLE.",
    "I CAME HERE TO MEASURE THE SNOW. THE SNOW MEASURED ME.",
    "HOT ROCK, COLD FEET. KEEP THE MAP AWAY FROM THE LAVA.",
    "THE CAVES HERE CHANGE THEIR MINDS. I WRITE THEM DOWN ANYWAY.",
    "THE WATER CARRIES SOUND AND NOTHING ELSE.\nNOT EVEN MY OLD MAPS.",
    "I MAPPED THE HALLS BY CANDLE. THE CANDLE IS GONE NOW.",
    "THE CITADEL HAS NO NAME, SO IT HAS NO SHAPE.\nI DREW IT ANYWAY. YOU'RE WELCOME.",
    "THE LAST ROOMS ARE THE QUIETEST. I DON'T GO IN.",
};
// Ilsa's three intro talks (spec §14: she is a little lost too); talk 1's first line is given by the spec.
static const char* const ILSA_INTRO[3][2] = {
    {"OH. YOU'RE AWAKE. THE FIRE CALLED YOU HERE, I THINK. IT DOES THAT.",
     "I AM ILSA. I KEEP THE NAMES THE FIRE IS GIVEN.\nYOURS IS NOT AMONG THEM. NOT YET."},
    {"YOU WANT TO KNOW WHERE YOU ARE. SO DO I, SOME DAYS.\nTHIS IS NAMULA, THE LAST VILLAGE, AT THE EDGE OF EVERYTHING.",
     "OUT THERE THE MONSTERS ARE FULL OF STOLEN NAMES.\nWHEN THEY FALL, THE NAMES ARE FREE. CARRY THEM HOME."},
    {"I USED TO REMEMBER WHO LIT THIS FIRE.\nNOW I ONLY REMEMBER TO KEEP IT. IS THAT SAD? IT FEELS LIKE WORK.",
     "BRING ME WHAT YOU CARRY. THE HEARTH WILL TURN IT INTO YOU."},
};
// Ilsa's first line once the Great Name of world n is taken (spec §14: author 5); worlds 4-5 come with their phases.
static const char* const ILSA_GREAT_NAME[5] = {
    "YOU CAME BACK WITH A GREAT NAME. THE FIRE SAT UP WHEN YOU WALKED IN.\nIT HAS NOT DONE THAT IN A LONG TIME.",
    "YOU SMELL OF RAIN AND OLD LEAVES. THE FIRE LIKES IT.\nLOOK, IT BURNS A LITTLE GREENER TONIGHT.",
    "YOUR HANDS ARE SO COLD. HOLD THEM TO THE FIRE A WHILE.\nSOMETHING OLD AND QUIET GAVE YOU ITS NAME. KEEP IT WARM.",
    nullptr, nullptr};
// Lio's verse about the latest Great Name, after the song (spec §14: the song changes after each); worlds 4-5 later.
static const char* const LIO_GREAT_NAME[5] = {
    nullptr,
    "I WROTE A NEW VERSE ABOUT THE SILVERBACK.\nIT'S MOSTLY DRUMMING. HE'D HAVE LIKED THAT.",
    "THEY SAY THE TITAN HUMMED AS IT FELL. I'VE TRIED TO FIND THE NOTE.\nIT'S LOWER THAN ANY STRING I OWN.",
    nullptr, nullptr};
// Lio's song, one per highest world 1..7 (spec §14).
static const char* const LIO_SONGS[7] = {
    "SING WITH ME: THE STATION DRIFTS, THE STATION SLEEPS,\nAND NO ONE WAKES THE THING IT KEEPS...",
    "SING WITH ME: GREEN GROWS DEEP AND DEEP GROWS GREEN,\nAND NOTHING DOWN THERE WANTS TO BE SEEN...",
    "SING WITH ME: WHITE IS THE SILENCE, WHITE IS THE SNOW,\nWHERE DID THE NAMES OF THE SNOWMEN GO...",
    "SING WITH ME: THE MAW IS HUNGRY, THE MAW IS HOT,\nIT EATS WHAT IT'S GIVEN AND WHAT IT'S NOT...",
    "SING WITH ME: DOWN IN THE HALLS WHERE THE WATER SPEAKS,\nTHE QUIET HAS BEEN DROWNING FOR WEEKS...",
    "SING WITH ME: UP PAST THE CLOUDS THE CITADEL STANDS,\nWITH NO NAMES ON ITS DOORS AND NO NAMES ON ITS HANDS...",
    "SING WITH ME: A PAGE THAT NO ONE WROTE,\nA SONG THAT NO ONE SANG... HOW DOES IT GO?",
};
// A mentor of another class talks about their craft (spec §14); by class.
static const char* const MENTOR_FLAVOUR[CLASS_COUNT] = {
    "A SHIELD IS A PROMISE. HOLD IT HIGH AND KEEP IT,\nAND IT WILL KEEP YOU.",
    "WATCH THE WIND, NOT THE TARGET. THE TARGET ALWAYS LIES.",
    "EVERY SPELL IS A NAME SAID VERY LOUDLY. MIND YOUR VOICE.",
    "FAST FEET, QUIET HANDS. IF THEY SAW YOU, YOU WERE SLOW.",
};

// Chests by world and ChestId (spec §10 table); world 0 has none.
static const ChestContent CHESTS[WORLD_COUNT][6] = {
    {},
    {{CHEST_SHARD, 0, 1}, {CHEST_PAGE, 1, 1}, {CHEST_KEY, KEY_STAR_IRON, 1}, {CHEST_NAMES, 0, 30}, {CHEST_ITEM, ITEM_RED, 2},
     {CHEST_ITEM, ITEM_WHETSTONE, 1}},
    {{CHEST_SHARD, 0, 1}, {CHEST_PAGE, 2, 1}, {CHEST_NAMES, 0, 30}, {CHEST_ITEM, ITEM_RED, 2}, {CHEST_ITEM, ITEM_TONIC, 1},
     {CHEST_ITEM, ITEM_BIG, 1}},
    {{CHEST_SHARD, 0, 1}, {CHEST_PAGE, 3, 1}, {CHEST_KEY, KEY_FROZEN_FEATHER, 1}, {CHEST_NAMES, 0, 30}, {CHEST_ITEM, ITEM_RED, 2},
     {CHEST_ITEM, ITEM_FEATHER, 1}},
    {{CHEST_SHARD, 0, 1}, {CHEST_PAGE, 4, 1}, {CHEST_KEY, KEY_PICKAXE, 1}, {CHEST_KEY, KEY_HEART_OF_MAGMA, 1}, {CHEST_NAMES, 0, 30},
     {CHEST_ITEM, ITEM_BIG, 1}},
    {{CHEST_SHARD, 0, 1}, {CHEST_PAGE, 5, 1}, {CHEST_NAMES, 0, 30}, {CHEST_ITEM, ITEM_BIG, 2}, {CHEST_ITEM, ITEM_MANA, 2}},
    {{CHEST_SHARD, 0, 1}, {CHEST_NAMES, 0, 30}, {CHEST_NAMES, 0, 30}, {CHEST_ITEM, ITEM_BIG, 2},
     {CHEST_ITEM, ITEM_WHETSTONE, 1}},   // spec: WHETSTONE + IRON TONIC, which one ChestContent can't hold (phase 8 decides)
    {{CHEST_NAMES, 0, 50}, {CHEST_NAMES, 0, 50}},
    {{CHEST_ITEM, ITEM_RED, 2}, {CHEST_NAMES, 0, 30}},   // TEST GROUNDS
};

const char* npcName(int id) {
    return id >= 0 && id < NPC_COUNT ? NPC_NAMES[id] : "";
}

bool npcPresent(const SaveData& save, int id, int world, int area) {
    (void)area;
    switch (id) {
    case NPC_STRANGER: return save.bosses >> 3 & 1;                                   // from world 3's boss death
    case NPC_GARRICK: return (world == WORLD_NAMULA) == (save.quests[QUEST_TOBIN] >= 2);   // the Greendeep, then the Shore
    case NPC_CLAUDE: return save.keys >> KEY_CREDITS_SEEN & 1;
    default: return true;
    }
}

void talkTo(Quest& q, int id) {
    SaveData& s = q.save;
    DialogueState& d = q.dialogue;
    d = DialogueState();
    d.speaker = npcName(id);
    d.after = AFTER_NONE;
    std::vector<std::string>& p = d.pages;
    int cls = s.heroClass >= 0 && s.heroClass < CLASS_COUNT ? s.heroClass : 0;
    char line[96];
    switch (id) {
    case NPC_ILSA:   // after a Great Name her first line changes (spec §14)
        s.ilsaTalks++;
        if (s.ilsaTalks <= 3) {
            p = {ILSA_INTRO[s.ilsaTalks - 1][0], ILSA_INTRO[s.ilsaTalks - 1][1]};
        } else {
            p = {"GIVE THE FIRE WHAT YOU CARRY, AND IT WILL GIVE YOU MORE OF YOURSELF."};
            for (int w = 5; w >= 1; w--)   // the latest Great Name's line comes first
                if ((s.bosses >> w & 1) && ILSA_GREAT_NAME[w - 1]) {
                    p.insert(p.begin(), ILSA_GREAT_NAME[w - 1]);
                    break;
                }
            d.after = AFTER_LEVEL_UP;
        }
        break;
    case NPC_MARRA:
        p = {"HERBS FROM THE CLIFF, ROOTS FROM THE SHORE. EVERYTHING HEALS SOMETHING.\nTAKE WHAT YOU NEED. PAY WHAT IT COSTS."};
        d.after = AFTER_POTION_SHOP;
        break;
    case NPC_BRANN:   // phase 7: the Smith quest's refines
        p = {"THE FORGE STILL REMEMBERS HOW TO MAKE A BLADE. SO DO I, MOSTLY.\nSHOW ME YOUR NAMES AND I'LL SHOW YOU STEEL."};
        d.after = AFTER_GEAR_SHOP;
        break;
    case NPC_LIO:   // the song of the highest world, then a verse about the latest Great Name
        p = {LIO_SONGS[(s.highest < 1 ? 1 : s.highest > 7 ? 7 : s.highest) - 1]};
        for (int w = 5; w >= 1; w--)
            if (s.bosses >> w & 1) {
                if (LIO_GREAT_NAME[w - 1]) p.push_back(LIO_GREAT_NAME[w - 1]);
                break;
            }
        break;
    case NPC_PELL: p = {"MY PAGES! THE WIND TOOK MY MAPS OF THE FIVE WORLDS."}; break;   // phase 7: the map pages quest
    case NPC_SELA: p = {"..."}; break;                                                     // phase 7: lore at 20, the locket at 35
    case NPC_ODO:
    case NPC_WREN:
    case NPC_YSOLDE:
    case NPC_KIT: {   // the class mentor gives the jobs (spec §14 JOB quest); the others talk about their craft
        if (id - NPC_ODO != cls) {
            p = {MENTOR_FLAVOUR[id - NPC_ODO]};
            break;
        }
        int stage = s.quests[QUEST_JOB];
        snprintf(line, sizeof line, "FROM TODAY YOUR CRAFT HAS A NEW NAME: %s.", JOB_NAMES[cls][stage < 2 ? stage + 1 : 2]);
        if (stage == 0 && s.level < 5) {
            p = {"COME BACK AT LEVEL 5."};
        } else if (stage == 0 && !(s.gates >> 1 & 1)) {
            p = {"YOU'RE NEARLY READY. BUT THE GATE IN THE DRIFTING STATION STILL STANDS.\nOPEN IT, THEN COME BACK TO ME."};
        } else if (stage == 0) {
            p = {"THE GATE IS OPEN AND YOU CAME BACK. THAT IS MOST OF THE LESSON.", line};
            d.after = AFTER_JOB;
        } else if (stage == 1 && s.level < 15) {
            p = {"COME BACK AT LEVEL 15."};
        } else if (stage == 1 && !(s.keys >> KEY_FROZEN_FEATHER & 1)) {
            p = {"BRING ME A FROZEN FEATHER FROM THE WHITE SILENCE.\nIT STILL REMEMBERS HOW TO FLY. YOU'LL NEED THAT."};
        } else if (stage == 1) {
            p = {"A FROZEN FEATHER. COLD AS A PROMISE. GIVE IT HERE.", line};
            d.after = AFTER_JOB;
        } else {
            p = {"COME BACK AT LEVEL 30."};   // phase 7: the Trial of Echoes and tier 3
        }
        break;
    }
    case NPC_GATEKEEPER: {   // phase 7: the Sky Stair opens at 5
        int sigils = 0;
        for (int w = 1; w <= 5; w++) sigils += s.bosses >> w & 1;
        snprintf(line, sizeof line, "%d OF 5 GREAT NAMES.", sigils);
        p = {line};
        break;
    }
    case NPC_TOBIN: p = {"MY FATHER GARRICK WENT INTO THE GREENDEEP AND NEVER CAME BACK."}; break;   // phase 7: the Tobin quest
    case NPC_GARRICK:   // hurt in a Greendeep dead end (spec §14 Tobin quest stage 1)
        if (q.area.world != 2) {
            p = {"..."};   // phase 7: Garrick on the Shore
        } else if (s.items[ITEM_RED] > 0) {   // phase 7: the potion is used and the quest moves to stage 2
            p = {"A RED POTION... FOR AN OLD DIGGER LIKE ME? BLESS YOU, FRIEND.",
                 "TELL MY BOY TOBIN I'M COMING HOME.\nSLOWLY. BUT I'M COMING."};
        } else {
            p = {"I... I CAN'T WALK. DO YOU HAVE A RED POTION?"};
        }
        break;
    case NPC_STRANGER:
    case NPC_CALDER: p = {"..."}; break;   // phase 7: their quests
    case NPC_CLAUDE:   // phase 8: New Game+
        p = {"ANOTHER ROUND? I'LL MAKE IT HARDER. I PROMISE."};
        d.after = AFTER_YES_NO;
        break;
    default:   // the surveyors: one line, then the world map
        p = {SURVEYOR_LINES[(id - NPC_SURVEYOR_FIRST) % 12]};
        d.after = AFTER_MAP;
        break;
    }
    q.mode = QMODE_DIALOGUE;
    q.modeTimer = 0;
}

bool levelUp(Quest& q) {
    SaveData& s = q.save;
    int cost = levelCost(s.level);
    if (s.level >= MAX_LEVEL || s.names < cost) {
        q.soundRequested[SOUND_ROCKET_EMPTY] = true;
        return false;
    }
    s.names -= cost;
    s.level++;
    questRefreshStats(q, true);
    q.soundRequested[SOUND_Q_LEVEL_UP] = true;
    logLine(q, "quest: level %d (spent %d, names left %d)", s.level, cost, s.names);
    questSave(q);
    return true;
}

void grantJob(Quest& q) {
    SaveData& s = q.save;
    if (s.job >= 3) return;
    s.job++;
    s.quests[QUEST_JOB] = s.job;   // JOB stage n gave tier n (tier 3 waits for the trial, phase 7)
    if (s.job == 2) s.keys &= ~(1u << KEY_FROZEN_FEATHER);   // the mentor takes the feather
    questRefreshStats(q, true);
    int cls = s.heroClass >= 0 && s.heroClass < CLASS_COUNT ? s.heroClass : 0;
    q.banner = JOB_NAMES[cls][s.job];
    q.bannerScale = 2;
    q.bannerColor = 'z';
    q.bannerTimer = 2;
    const Hero& h = q.hero;
    for (int i = 0; i < 30 && q.particles.size() < PARTICLE_CAP; i++) {
        float angle = i * 6.2832f / 30, speed = (30 + 15 * (i % 4)) * DP;
        q.particles.push_back({h.x + HERO_W / 2, h.y + HERO_H / 2, cosf(angle) * speed, sinf(angle) * speed - 20 * DP, 0.9f,
                               paletteColor(i % 2 ? 'z' : 'N'), 1});
    }
    q.soundRequested[SOUND_Q_LEVEL_UP] = true;
    logLine(q, "quest: job %d %s", s.job, JOB_NAMES[cls][s.job]);
    logLine(q, "quest: quest %s stage %d", "JOB", s.quests[QUEST_JOB]);
    questSave(q);
}

std::vector<ShopRow> shopRows(const SaveData& s, int shopKind) {
    std::vector<ShopRow> rows;
    char detail[32];
    if (shopKind == AFTER_POTION_SHOP) {
        for (int i = 0; i < ITEM_COUNT; i++) {
            int price = itemPrice(s, i), limit = carryLimit(s, i);
            snprintf(detail, sizeof detail, "%d/%d", s.items[i], limit);
            rows.push_back({i, ITEM_NAMES[i], detail, price, s.names >= price && s.items[i] < limit});
        }
        return rows;
    }
    // Brann: only the next tier of each, once the hero has reached that world; tier 7 is never sold.
    int cls = s.heroClass >= 0 && s.heroClass < CLASS_COUNT ? s.heroClass : 0;
    HeroStats now = heroStats(s);
    SaveData next = s;
    next.weapon = s.weapon + 1;
    if (next.weapon <= 6 && s.highest >= next.weapon) {
        snprintf(detail, sizeof detail, "ATK +%d", heroStats(next).atk - now.atk);
        int price = gearPrice(next.weapon, true);
        rows.push_back({GOODS_WEAPON, WEAPON_NAMES[cls][next.weapon - 1], detail, price, s.names >= price});
    }
    next = s;
    next.armor = s.armor + 1;
    if (next.armor <= 6 && s.highest >= next.armor) {
        HeroStats more = heroStats(next);
        snprintf(detail, sizeof detail, "DEF +%d HP +%d", more.def - now.def, more.maxHp - now.maxHp);
        int price = gearPrice(next.armor, false);
        rows.push_back({GOODS_ARMOR, ARMOR_NAMES[next.armor - 1], detail, price, s.names >= price});
    }
    return rows;
}

void shopBuy(Quest& q, int row) {
    std::vector<ShopRow> rows = shopRows(q.save, q.shopKind);
    if (row < 0 || row >= (int)rows.size() || !rows[row].canBuy) {
        q.soundRequested[SOUND_ROCKET_EMPTY] = true;
        return;
    }
    const ShopRow& r = rows[row];
    SaveData& s = q.save;
    s.names -= r.price;
    if (r.goods == GOODS_WEAPON) s.weapon++;
    else if (r.goods == GOODS_ARMOR) s.armor++;
    else s.items[r.goods]++;
    questRefreshStats(q, false);
    q.soundRequested[SOUND_PICKUP] = true;
    logLine(q, "quest: bought %s for %d", r.name.c_str(), r.price);
    questSave(q);
}

const ChestContent& chestContent(int world, int chestId) {
    static const ChestContent EMPTY = {CHEST_NAMES, 0, 0};
    if (world < 0 || world >= WORLD_COUNT || chestId < 0 || chestId >= 6) return EMPTY;
    return CHESTS[world][chestId];
}
