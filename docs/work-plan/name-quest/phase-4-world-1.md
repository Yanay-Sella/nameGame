# Phase 4: world 1 (The Drifting Station), the boss framework, The Overseer, The Star Mage

Read first: [`../name-quest-plan.md`](../name-quest-plan.md), then spec §4 (layout rules), §8 (world 1 roster), §9
(world 1), §12 (rules, world 1 row), §13 (world 1 row, ramp), §14 (Ilsa and Lio after a Great Name), §15 (boss bar),
§10 (world 1 chests). Phases 1–3 done.

This phase also writes the **world authoring recipe** below; phases 5, 6, 8 and 9 follow it for their worlds.

## What the player gets
World 1 from its entry to its boss: 20 areas that start calm and end intense, electric panels and vents, five enemy
types, the Overseer guarding the gate, a second checkpoint, two surveyors, six chests, a shortcut, and the Star Mage.
Killing it lights world 2's entry (world 2's areas come in phase 5; until then travelling there shows the entry
checkpoint in an empty one-area placeholder room: author `WORLD2_AREAS` with one wide area holding a `C`, removed in phase 5).

## World authoring recipe (phases 4, 5, 6, 8, 9)
1. **Plan the graph before any tiles.** At the top of `source/quest_world<N>.cpp`, a comment block: an ASCII sketch of
   the world map grid (area indices in their cells), then one line per area: index, name, wide/tall, band, purpose
   (`teaches panels`, `dead end: VIAL SHARD`, `loop back to 4`, ...). Check it against spec §4 layout rules 1–10 and
   the world's chest list before drawing tiles.
2. **Teach, then test.** Each new hazard or enemy first appears where a mistake costs little (no pit beside the first
   panel, the first gunner with cover nearby), then again where it matters. Band rules (§8 counts, §9 hazards) are hard
   limits.
3. **Fair placement.** No enemy within 6 tiles of an arrival tile or a checkpoint; no hazard on an arrival tile; a
   shooter always has a place to stand out of its line within 5 tiles; elites stand on flat ground at least 10 tiles
   wide; pits are visible from the jump's start (no blind jumps off-screen: a gap's far side is within 300 px).
4. **Getting lost on purpose.** Areas look alike within a world (that's the point of surveyors), but each dead end has
   one landmark (a broken statue, a big window) so players can describe it. Portals lead away from the map's logic.
5. **Shortcuts** open toward a checkpoint and are opened from the far side (the lever beyond the door).
6. **Validate:** `make test` (structure, links, map adjacency, reachability, gate rules). Then `make quest-maps`
   (step 2) and read every area image: fix unreadable spots (a shot colour lost on a background, tiles that look the
   same but behave differently).
7. **Play it on the Mac** with `NAMEGAME_QUEST` at the world's reference level and gear (spec §7) through the main
   path; fix what feels unfair (not what feels hard).

## Step 1: the boss framework (`source/quest.h`, new `source/quest_bosses.cpp`, `source/quest.cpp`, `source/quest_draw.cpp`)
```cpp
enum QBossId { QBOSS_OVERSEER, QBOSS_STAR_MAGE, QBOSS_MONKEY_KING, QBOSS_SILVERBACK, QBOSS_YETI, QBOSS_TITAN, QBOSS_IMP,
               QBOSS_FIRE_LORD, QBOSS_ANGLER, QBOSS_FISHERMAN, QBOSS_TWIN_SWORD, QBOSS_TWIN_SPEAR, QBOSS_NAME_EATER,
               QBOSS_CLAUDE, QBOSS_FIRST_NAME, QBOSS_COUNT };
struct Boss {
    int id; float x, y, vx, vy; int w, h;
    int hp, maxHp, atk, bigAtk, contactAtk;
    int phase;               // 1, 2 (Claude 3)
    int attack, lastAttack;  // the attack running, the one before
    int step; float timer;   // the attack's own step machine: telegraph, act, recover
    float gap;               // s until the next attack
    float shift;             // s of the invulnerable phase-change pose
    float flash; bool facingLeft, alive;
    float started;           // q.time when the fight began (the log's duration)
    float data[8];           // per-boss scratch (anchors chosen, wave counts...); name each use in a comment
};
struct Marker { float x, y, w, h, timer; char color; };   // a telegraph drawn as dashes until timer runs out
const char* bossName(int id);
void startBoss(Quest& q, int id, float x, float y);       // stats from §7, banner, bar, exits closed, log "start"
void updateBosses(Quest& q, float dt);                    // phases, gaps, the per-boss switch, deaths
```
- Add `std::vector<Boss> bosses; std::vector<Marker> markers; bool arenaLocked;` to `Quest`.
- Entering an AREA_MINIBOSS / AREA_BOSS area whose boss is due (mini-boss: always after a rest or death; big boss:
  `bosses` bit clear) spawns it at its `Q` / `W` spot and locks the arena (spec §12 rules). Leaving is impossible while
  locked; the hero's death unlocks and removes the boss.
- `updateBosses`: phase change at the thresholds (§12), `gap` countdown, attack choice (random, never the last one),
  then `switch (id)` into one function per boss (`updateOverseer`, `updateStarMage`, ...) — each boss is its own function
  because each is a long, distinct state machine. Boss hits on the hero use `contactAtk`, `atk`, `bigAtk` from spec §7.
- Death (§12): mini-boss → names, `openGate`; big boss → names, bit, next world's entry lit, banner, autosave; logs
  `quest: boss %s down after %.0f s`. Phase log: `quest: boss %s phase %d`.
- Boss bar and name (spec §15); markers drawn as dashed rectangles in their colour, blinking faster in their last 0.3 s.

## Step 2: map renderer tool (`Makefile`, `tests/quest_tests.cpp`)
`make quest-maps`: builds `quest_tests` and runs it with `--maps`, which, instead of testing, renders every area of
every world: for each area, it sets up a `Quest` in PLAY there (no enemies moving, hero hidden), draws with the camera
at each 320x180 screen of the area (2 per area), saves `build/maps/w<N>_a<NN>_<k>.bmp` with `screenSaveBmp`, then the
Makefile converts them with `sips -s format png` into `shots/maps/` (gitignored with `shots/`). Enemy spawns are drawn
at their spots. This is your eyes on the level design.

## Step 3: world 1 hazards and enemies (`source/quest.cpp`, `source/quest_combat.cpp`, `source/quest_sprites.h`)
- Electric panels `z` (spec §9): a world clock `q.time` drives the cycle (all panels in sync per area; band 2 uses the
  shorter off time); flicker frames before on; damage when on (§7 trap 15 %); SOUND_Q_ZAP when they turn on and are on screen.
- Vents `v` already lift (phase 1); draw their rising streaks.
- Sprites: HOPPER DRONE (12x10, 2 frames), SENTINEL (16x16, 2 frames + a wind-up frame), world 1 tiles (plate floor
  with rivets, girders), the world 1 background (stars, a far planet, girders; spec §13), the space pit (black with stars).
- The shooter's aliens face the camera; that's fine for this world (they're aliens).

## Step 4: The Overseer and The Star Mage (`source/quest_bosses.cpp`)
Exactly spec §12's world 1 row: telegraph times, speeds, counts, phase-2 changes. The Star Mage's 4 teleport spots are
the arena's `X` anchors 0–3; its electric floor is the arena's middle 6 tiles of the bottom-most floor row (draw them as
panels). Summons use the world's roster slot A (crawlers) and B (floaters) with normal enemy stats and give no names.

## Step 5: author world 1 (new `source/quest_world1.cpp`)
Follow the recipe. World 1's personality: a dead space station drifting near a dim sun; corridors, airlocks, hangars,
a broken observatory. Chests: spec §10 world 1. Its gate is a bulkhead. The second checkpoint is in a med bay.
Arena 9-ish (mini-boss) is a hangar; area 19 the observatory dome. Surveyors: author 2 names and lines.

## Step 6: the closed-gate rule and world 1 in the tests (`tests/quest_tests.cpp`)
- `everythingIsReachable` covers world 1.
- `gateGuardsTheSecondHalf`: with the gate closed, area 19 and the second `C` are unreachable from area 0; the
  mini-boss arena is reachable.
- `worldLayoutRulesHold(world)` (call for world 1): spec §4 rules 2, 4, 5, 6 (main-path length via BFS over the area
  graph, the mini-boss position, the second `C` distances, ≥3 dead ends, ≥1 loop, ≥1 `K`, ≥2 portals, 2–4 tall areas,
  surveyor positions), and §8 spawn counts per band.
- `chestsMatchTheWorldList`: every chest id 0–5 of world 1 appears exactly once.
- `overseerTellsBeforeEveryAttack`, `starMageBeamMarksItsColumn`, `bossFightLocksAndUnlocksExits`,
  `bigBossStaysDeadMiniBossReturns`, `bossDeathLightsTheNextWorld`.

## Step 7: story touches (`source/quest_story.cpp`)
Ilsa's first line after world 1's Great Name (author, §14), Lio's world 1 and 2 lines. The world 2 placeholder entry.

## Step 8: Mac self-check
- `make quest-maps`; read all 40 world-1 images.
- Play world 1 on the Mac at the reference (`level=4 gear=0`) with each class for the boss fights at least:
  `NAMEGAME_QUEST='slot=3 class=2 world=1 area=19 level=4'` etc. Note fight lengths from the log.

## Step 9: docs
design.md: world 1 and the boss rules; implementation.md; testing.md (PS3 checks below + a user playtest checklist:
"a first-time run of world 1 takes at least 2 hours", "each boss took more than one try", "never stuck"). GDD change
log `0.32`. `check_docs.py`.

## Step 10: PS3 check and the user's playtest
1. Locked command: load slot 1, travel from the hearth to world 1's entry (Up at the hearth, TRAVEL, the second row), walk
   right a few seconds, screenshot. Expect `quest: travel to 1.0`, `quest: area 1.0 <name>`, fps ≈ 60.
2. The PS3 has no `NAMEGAME_QUEST`, so use a dev save: add `pkgfiles/USRDIR/quest*.txt` to `.gitignore`, copy the
   `quest3.txt` the Mac wrote (`build/mac/quest3.txt`, made with `NAMEGAME_QUEST='slot=3 class=0 world=1 area=19 level=8 gear=1'`)
   to `pkgfiles/USRDIR/quest3.txt`, `make ps3`, and delete the copy after the check. In one locked command: load slot 3, fight the Star Mage in god mode (L2+L3) and screenshot its beam. Expect
   `quest: boss THE STAR MAGE start`, `phase 2`, `down after`, fps ≥ 59 during beams.
3. Ask the user to play world 1 from a new slot on the PS3 (any class) and tell you: time to the Overseer, deaths, where
   they got lost, what felt unfair. Record their notes in testing.md. Don't retune now (phase 9), unless something is
   broken or impossible.

## Done when
- `make test` passes, `make ps3` builds, `check_docs.py` is clean.
- On the PS3: world 1 can be played from entry to the Star Mage's death, which lights world 2's entry.
