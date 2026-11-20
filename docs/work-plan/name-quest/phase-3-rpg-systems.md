# Phase 3: RPG systems (checkpoints, Namula, levelling, shops, items, jobs, chests, gates, map, ambience)

Read first: [`../name-quest-plan.md`](../name-quest-plan.md), then spec §2 (Up, Square, Triangle), §6 "Jobs", §10, §11,
§14 (dialogue box, NPC table, the JOB quest, Ilsa's lines), §15, §16 (ambience), §17 "Autosave", §18, §22 (characters, lights). Phases 1, 1b and 2 done.

## What the player gets
- Braziers light on touch; Up opens REST / TRAVEL / LEAVE; rest heals and respawns enemies; travel goes between lit
  checkpoints and Namula.
- Namula is alive: Ilsa (levels, after three intro talks), Marra (potions and buffs), Brann (gear), Lio, Pell, Sela,
  the four mentors and the Gatekeeper stand in their areas with their sprites and baseline lines; the class mentor
  gives the tier-1 job at level 5 once world 1's gate is open, and tier 2 at level 15 with the FROZEN FEATHER.
- Square drinks or uses the selected item, Triangle selects the next owned one; buffs run 120 s.
- Chests, the gate, shortcut levers and surveyors (with the world map screen) work; ambience loops per world.
- Everything autosaves at the spec §17 moments.

## Step 1: interaction and modes (`source/quest.h`, `source/quest.cpp`)
- Add modes `QMODE_DIALOGUE, QMODE_MENU, QMODE_SHOP, QMODE_MAP` (and `QMODE_CREDITS` now, unused until phase 8).
- Up pressed on the ground: the first touched of NPC (within 14 px of the hero's centre horizontally, same floor) →
  chest → checkpoint → surveyor → portal; climbing a ladder only if none of them is touched (clear `moveY` for that step).
- `QMODE_MENU` is one generic list menu: `struct MenuState { int kind; std::vector<std::string> items; int cursor, scroll; };`
  kinds: MENU_CHECKPOINT, MENU_TRAVEL, MENU_LEVEL_UP, MENU_JOB_CONFIRM, MENU_YES_NO. One `switch (kind)` on confirm.
  Drawn with `drawMenu` (lists ≤ 8 rows) or the level-up panel (spec §15). Circle closes every menu.
- `QMODE_DIALOGUE`: `std::vector<std::string> pages; int page; float typed; std::string speaker; int after;` —
  `after` is what opens when the last page closes (`AFTER_NONE, AFTER_POTION_SHOP, AFTER_GEAR_SHOP, AFTER_LEVEL_UP,
  AFTER_JOB, AFTER_MAP, AFTER_YES_NO`). Typing, X and Circle per spec §14.
- The game is frozen (no enemy, hero or projectile update) in every mode but PLAY; playtime keeps counting.
  `questCanPause` is true in PLAY, DEAD, DIALOGUE, MENU, SHOP and MAP.

## Step 2: NPCs (new `source/quest_story.cpp`, `source/quest_sprites.h`)
```cpp
enum NpcId { NPC_ILSA, NPC_MARRA, NPC_BRANN, NPC_LIO, NPC_PELL, NPC_SELA, NPC_ODO, NPC_WREN, NPC_YSOLDE, NPC_KIT,
             NPC_GATEKEEPER, NPC_TOBIN, NPC_GARRICK, NPC_STRANGER, NPC_CALDER, NPC_CLAUDE, NPC_SURVEYOR_FIRST,
             NPC_COUNT = NPC_SURVEYOR_FIRST + 12 };
const char* npcName(int id);
bool npcPresent(const SaveData& save, int id, int world, int area);   // the Stranger after world 3, Garrick moving, Claude after the credits
void talkTo(Quest& q, int id);                                        // fills the dialogue pages and `after`, applies quest effects
```
- Place `N` spots in Namula's 3 areas and fill their `npcs[]` with the spec §14 locations. Garrick has two spots
  (world 2 later, the Shore now): `npcPresent` picks by `quests[QUEST_TOBIN]`. Claude's Shore spot is absent until
  KEY_CREDITS_SEEN.
- `talkTo` is one `switch (id)`. Write in this phase: Ilsa (talks 1–3 authored intro, the §14 key lines, from talk 4
  `after = AFTER_LEVEL_UP`; `ilsaTalks` saved), Marra and Brann (a greeting page, then their shop), the class mentor
  (JOB stages 0 → 1 → 2 per spec §14; tier 3 says `COME BACK AT LEVEL 30.` until phase 7), other mentors' flavour line,
  Lio, Pell, Sela (`...`), the Gatekeeper (`<n> OF 5 GREAT NAMES.` only), Tobin (stage 0 line only). Quest effects and
  the remaining stages come in phase 7; leave a one-line `// phase 7` at each spot that will grow.
- Sprites (16 x 20, 2–4 idle frames each, outlined, lanterns and the hearth as lights; author in the spec §1 tone): Ilsa (green-grey hooded keeper with a lantern),
  Marra, Brann (apron, hammer), Lio (lute), Pell (huge rolled maps), Sela (shawl, by the well), Sir Odo, Wren, Ysolde,
  Kit, Tobin (a small boy, 12 x 12), the Gatekeeper (a 16 x 24 stone statue), the surveyor (telescope).
  Draw NPCs facing the hero.

## Step 3: levelling, jobs, shops (`source/quest_story.cpp`, `source/quest_stats.cpp`)
- Level up menu (spec §15): LEVEL UP spends `levelCost(level)` if affordable and level < 40, recomputes stats, full
  HP/MP, SOUND_Q_LEVEL_UP, log `quest: level %d (spent %d, names left %d)`, autosave; the menu stays open.
- Job: the mentor's last page with `after = AFTER_JOB` grants the tier (spec §6 "Jobs"): banner, particles, log
  `quest: job %d %s`, autosave.
- Potion shop (Marra): RED POTION, BIG POTION, MANA POTION, WHETSTONE, IRON TONIC, SWIFT FEATHER at spec §10 prices
  (`PRICE_SCALE[highest]`), carry limits from §10 (`carryLimit(save, item)` in quest_stats.cpp, with shards, locket,
  Garrick's gift). Gear shop (Brann): the next weapon tier (the class's name for it) and the next armor tier, each only if
  `highest >= tier`; tier 7 never sold. Buying: names taken, SOUND_PICKUP, log
  `quest: bought %s for %d`, autosave. Rows that can't be bought are grey; X on them plays SOUND_ROCKET_EMPTY.
- Weapon and armor names: `WEAPON_NAMES[CLASS_COUNT][7]`, `ARMOR_NAMES[6]` (author; tier 7 names from spec §10).

## Step 4: items and buffs (`source/quest.cpp`)
- Triangle cycles `selectedItem` through owned items (count > 0) in ItemType order; Square uses it: potions start a
  0.5 s drink (walk x0.4, SOUND_Q_DRINK) and heal/restore at its end; buffs start or restart their 120 s timer at once.
  Nothing owned: SOUND_ROCKET_EMPTY. Using an item doesn't autosave; the counts go out with the next autosave
  (deaths, rests and quitting all save).
- Buff effects: WHETSTONE ATK x1.2, IRON TONIC damage taken x0.8, SWIFT FEATHER walk x1.15. Death clears them.

## Step 5: checkpoints, travel, Namula's hearth (`source/quest.cpp`)
- Spec §11 exactly: lighting on touch (log `quest: checkpoint %d.%d lit`), the menu, REST (log `rest at`), TRAVEL
  (log `travel to`; the list: `NAMULA` then every lit checkpoint as `<WORLD NAME> ENTRY` / `DEPTHS`), the respawn point.
- Rest clears every `killed` list, resets crumbled `c` tiles and dropped icicles (phase 5 adds icicles; keep a
  `resetHazards` hook), heals, autosaves. The white fade: 0.3 s of a `w` frame border growing in from the edges.
- Entering any area 0 of world w sets `highest = max(highest, w)` for w in 1..6.
- Namula's hearth (`C` in Namula area 0) is checkpoint (0, 0) and always lit.

## Step 6: chests, gates, shortcuts (`source/quest.h`, `source/quest_areas.cpp`, `source/quest.cpp`)
```cpp
enum ChestKind { CHEST_ITEM, CHEST_NAMES, CHEST_KEY, CHEST_SHARD, CHEST_PAGE };
struct ChestContent { ChestKind kind; int item; int count; };   // item: ItemType, KeyItem or page number
const ChestContent& chestContent(int world, int chestId);        // spec §10 chest table; world 8 has 2 test chests
```
- Opening (spec §10): Up, lid 0.3 s, pop-up text 1.5 s (`VIAL SHARD`, `2 RED POTION`, `+120 NAMES`...), SOUND_Q_CHEST,
  bit in `chests[world]`, log `quest: chest %d.%d %s`, autosave. Bag full → `YOUR BAG IS FULL` and the chest stays shut.
  A chest whose bit is set is drawn open.
- `void openGate(Quest& q, int world)` (spec §11 visuals, log, autosave); the mini-bosses call it in phase 4. Shortcut
  levers `K`: any hero attack hitbox or hero projectile touching the lever opens the area's `k` tiles (log, autosave).
- TEST GROUNDS: add a gate wall in area 2 (opened in tests and by `NAMEGAME_QUEST ... gate=1`), a lever `K` with `k`
  door tiles in area 3 so it opens a way back to area 0's checkpoint, a second `C` in area 2, a surveyor `M` in area 1,
  and 2 chests (`chestContent(8, 0)` = 2 RED POTION, `(8, 1)` = names x30). Update the reachability test's world list
  (closed-gate check comes in phase 4).

## Step 7: surveyors and the map (`source/quest.cpp`, `source/quest_draw.cpp`)
- Up at a surveyor: one authored line (12 surveyor lines and names: `SURVEYOR ANDO` etc.; TEST GROUNDS uses the first),
  then QMODE_MAP. Map drawing per spec §11 exactly. Circle or X closes.

## Step 8: ambience (`source/audio.h`, `source/audio.cpp`, `host/audio_silent.cpp`, `source/main.cpp`)
- `Tone` gets `float fade = 1;` (1: today's linear fade-out; 0: flat). `synthesize` multiplies by `(1 - progress * t.fade)`.
- `void audioSetAmbient(int id);   // 0 off, 1 Namula, 2-8 worlds 1-7: loops on the reserved channel 1`. `Mix_ReserveChannels(2)`
  (0 shoot, 1 ambience). Build the 8 chunks at `audioOpen` per spec §16 (author the tones; keep each under the stated volume).
  Add the stub to `host/audio_silent.cpp`.
- `Quest::ambient` (int) set on area entry from the world (TEST GROUNDS: 0). main.cpp calls `audioSetAmbient(quest.ambient)`
  while the scene is QUEST or paused/settings from the quest, else `audioSetAmbient(0)`; the function returns early when
  the id is unchanged.

## Step 9: tests (`tests/quest_tests.cpp`)
`levelUpSpendsAndRaisesStats`, `cannotLevelPastForty`, `shopPricesScaleWithHighestWorld`, `carryLimitsStopBuying`,
`gearTierNeedsTheWorld`, `restHealsAndRespawnsEnemies`, `travelListsLitCheckpoints`, `chestOpensOnceAndSaves`,
`fullBagKeepsChestShut`, `gateWallOpens`, `leverOpensDoorsForGood`, `jobNeedsLevelAndGate`, `secondJobNeedsTheFeather`,
`buffsEndOnDeath`, `drinkHealsAfterHalfASecond`, `npcPresenceFollowsQuestStages`. All through public functions or by
driving `questUpdate` with scripted `Actions` on TEST GROUNDS and Namula.

## Step 10: docs
design.md rules for this phase's systems; implementation.md files; testing.md this phase's PS3 checks. GDD change log: the next version. `check_docs.py` clean.

## Step 11: Mac self-check
- `NAMEGAME_QUEST='slot=3 class=1 world=0 area=0 names=2000'`: talk to Ilsa four times, level up twice, buy two potions
  from Marra; screenshot the level-up panel and the shop and read them.
- TEST GROUNDS: light the area-2 checkpoint, travel to Namula and back, open both chests, the lever, the map. Read the
  map screenshot: visited areas filled, current blinking.

## Step 12: PS3 check
One locked command that loads slot 1, Select-holds into TEST GROUNDS, rests at its checkpoint (Up, X), opens the map at
the surveyor, travels to Namula, talks to Ilsa, and screenshots the map and Namula. Expect `quest: checkpoint 8.0 lit`,
`quest: rest at 8.0`, `quest: travel to 0.0`, ambience audible (ask the user to listen: hearth crackle in Namula, none in
TEST GROUNDS), fps ≈ 60.

## Done when
- `make test` passes, `make ps3` builds, `check_docs.py` is clean.
- On the PS3: rest, travel, level up, buy, open a chest, read the map; the ambience plays in Namula.
