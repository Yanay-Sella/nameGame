# Phase 1: NAME QUEST engine (menu, slots, classes, platformer, areas, saves, validator)

Read first: [`../name-quest-plan.md`](../name-quest-plan.md) (how to work), then spec §2, §3, §4, §5, §15 (slots,
class select, HUD's LV/HP lines only), §16 (the sound table), §17, §18, §20, §21. Every design choice is final.

## What the player gets at the end of this phase
- Main menu: `CLASSIC`, `LEVEL PICKER`, `NAME QUEST`, `SETTINGS`. NAME QUEST opens the slot screen.
- Slots → class select → the 4 intro pages → the hero stands in Namula (3 areas, no NPCs yet) and can walk, jump,
  double-jump (Thief), climb, drop through platforms, go between areas through edges and portals.
- Falling into a pit or the sea dies (`YOU FADED`, 2.5 s) and returns to the hearth.
- RETURN TO MENU saves; relaunching the game and picking the slot puts the hero back at the hearth with the same class.
- Dev: `NAMEGAME_QUEST` starts straight into play; Select warps; the TEST GROUNDS world (4 areas) exists for tests.
- `make test` runs a physics-based reachability check over every area.

## Step 1: input (`source/game.h`, `source/input.cpp`, `host/input_keyboard.cpp`)
- `game.h` › `struct Actions`, add after `cycle`:
  ```cpp
      bool jump;                    // X pressed (NAME QUEST: jump and confirm; Start is not a confirm there)
      bool jumpHeld;                // X held: a full-height jump
      bool item;                    // Triangle pressed (NAME QUEST: select the next item)
  ```
- `input.cpp` › `inputRead`: `a.jump = pressed[player][BTN_CROSS]; a.jumpHeld = SDL_JoystickGetButton(joystick, BTN_CROSS); a.item = pressed[player][BTN_TRIANGLE];`
- `host/input_keyboard.cpp` › `inputRead`: the same three with `pressed[player][BTN_CROSS]`, `down(BTN_CROSS)`, `pressed[player][BTN_TRIANGLE]`.
- Update the key comment at the top of `input_keyboard.cpp`: `Z cross (jump)`, `V triangle (item)`, `X circle (dodge)`, `C square (potion)`.

## Step 2: palette, font and a flipping sprite draw (`source/sprites.h`, `source/screen.h/.cpp`)
- `paletteColor`: add the 10 characters of spec §21 with their comments.
- `fontGlyph`: add `case ',': return 000024; case '?': return 071202; case '\'': return 022000; case '<': return 012421;
  case '(': return 012221; case ')': return 042224; case '*': return 005250;` (rows top to bottom, 3 bits each).
- `screen.h/.cpp`:
  ```cpp
  // Like drawSprite, mirrored when flip, with every swapFrom pixel drawn as swapTo (swapFrom 0: no swap).
  void drawSpriteEx(const Sprite& sprite, int x, int y, bool flip, char swapFrom = 0, uint32_t swapTo = 0);
  ```
  Same clipping and `~` mixing as `drawSprite`; the column read is `flip ? sprite.w - 1 - col : col`.
- Don't rerun `make_xmb_art.py` (the XMB art doesn't use these).

## Step 3: sounds (`source/game.h`, `source/audio.cpp`)
- Append every `SOUND_Q_*` of spec §16 to `enum Sound` before `SOUND_COUNT`, in the table's order
  (`SOUND_Q_STEP_STONE, SOUND_Q_STEP_METAL, SOUND_Q_STEP_LEAVES, SOUND_Q_STEP_SNOW, SOUND_Q_STEP_ROCK, SOUND_Q_STEP_SAND,
  SOUND_Q_STEP_MARBLE, SOUND_Q_STEP_PAPER, SOUND_Q_JUMP, SOUND_Q_LAND, SOUND_Q_SWING, SOUND_Q_STAB, SOUND_Q_ARROW,
  SOUND_Q_SPELL, SOUND_Q_HIT, SOUND_Q_CRIT, SOUND_Q_HURT, SOUND_Q_DEATH, SOUND_Q_NAMES, SOUND_Q_KINDLE, SOUND_Q_LEVEL_UP,
  SOUND_Q_PORTAL, SOUND_Q_BLIP, SOUND_Q_TALK, SOUND_Q_DRINK, SOUND_Q_DODGE, SOUND_Q_SPLASH, SOUND_Q_ZAP, SOUND_Q_ROAR,
  SOUND_Q_GATE, SOUND_Q_CHEST`).
- `audio.cpp` › `audioOpen`: synthesize each from the §16 tones (same `{start, seconds, startHz, endHz, volume, noise}` form).
  Ambience is phase 3; don't add `audioSetAmbient` yet.

## Step 4: data types and the area table (`source/quest.h`, `source/quest_areas.cpp`, `source/quest_sprites.h`)
`quest.h` includes only `game.h` (for `Actions`, `Sound`), `<stdint.h>`, `<string>`, `<vector>`. Declare:
```cpp
const int TILE = 8, WIDE_COLS = 80, WIDE_ROWS = 23, TALL_COLS = 40, TALL_ROWS = 45;
const int WORLD_NAMULA = 0, WORLD_SECRET = 7, WORLD_TEST = 8, WORLD_COUNT = 9, SAVED_WORLDS = 8;   // 1-6 the worlds
const int SLOT_COUNT = 3, MAX_LEVEL = 40;
enum HeroClass { CLASS_KNIGHT, CLASS_ARCHER, CLASS_MAGICIAN, CLASS_THIEF, CLASS_COUNT };
enum AreaFlag { AREA_MINIBOSS = 1, AREA_BOSS = 2, AREA_DARK = 4, AREA_RISING_LAVA = 8 };
enum ItemType { ITEM_RED, ITEM_BIG, ITEM_MANA, ITEM_WHETSTONE, ITEM_TONIC, ITEM_FEATHER, ITEM_COUNT };
enum QuestId { QUEST_JOB, QUEST_TOBIN, QUEST_PELL, QUEST_STRANGER, QUEST_SMITH, QUEST_SELA, QUEST_SKY, QUEST_COUNT };
enum KeyItem { KEY_STAR_IRON, KEY_FROZEN_FEATHER, KEY_PICKAXE, KEY_HEART_OF_MAGMA, KEY_LETTER, KEY_CALDER_RING,
               KEY_TOBIN_CHARM, KEY_STRANGER_RING, KEY_LOCKET, KEY_GARRICK_GIFT, KEY_PAGE_1, KEY_PAGE_2, KEY_PAGE_3,
               KEY_PAGE_4, KEY_PAGE_5, KEY_TRIAL_DONE, KEY_SECRET_DONE, KEY_CREDITS_SEEN, KEY_COUNT };   // bits of SaveData::keys
```
Then `AreaLink`, `AreaDef` exactly as spec §4 "Data", and:
```cpp
// How a world looks and sounds; tile drawing and the map read it (spec §13).
struct WorldStyle { char top, fill, platform, ladder, liquidA, liquidB, mapColor; Sound step; };
struct WorldDef { const char* name; const AreaDef* areas; int count; WorldStyle style; };
const WorldDef& worldDef(int world);   // 0..WORLD_COUNT-1; worlds with no areas yet have count 0
```
`SaveData` with every §17 field (`bool used; int heroClass, level, job, names; float playtime; int hp, mp;
int checkpointWorld, checkpointId; uint32_t lit; int echoNames, echoWorld, echoArea; float echoX, echoY;
uint32_t visited[SAVED_WORLDS], chests[SAVED_WORLDS], shortcuts[SAVED_WORLDS]; uint32_t gates, bosses;
int highest, ngPlus, deaths, kills; int items[ITEM_COUNT]; int weapon, armor, refine, shards; uint32_t keys;
int quests[QUEST_COUNT]; int ilsaTalks;`), with in-class defaults: level 1, highest 1, echoNames 0, lit 1 (the hearth),
items {5, 0, 0, 0, 0, 0} (a new hero starts with 5 RED POTION). Then:
```cpp
struct SlotSummary { bool used; int heroClass, level, job, highest, ngPlus; float playtime; bool secretDone; };

// The hero's body; combat fields come in phase 2.
struct Hero {
    float x, y, vx, vy;          // top-left of the 8x14 hitbox
    bool facingLeft, onGround, climbing, doubleJumpUsed;
    float coyote, jumpBuffer;    // s left of each grace window
    float lastSafeX, lastSafeY;  // where the hero last stood on safe ground (pit deaths drop names there)
    float stepTimer;             // s until the next footstep sound
};

// The current area's tiles as they are now (gates, crumbles, shortcut doors change them).
struct AreaState { int world, index; const AreaDef* def; std::vector<char> tiles; };

enum QuestMode { QMODE_SLOTS, QMODE_DELETE, QMODE_CLASS, QMODE_INTRO, QMODE_PLAY, QMODE_DEAD };   // later phases add more

struct Quest {
    QuestMode mode;
    std::string folder;                 // folder of quest1..3.txt
    SlotSummary slots[SLOT_COUNT];
    int slot, cursor;
    SaveData save;
    Hero hero;
    AreaState area;
    float camX, camY;
    float time, modeTimer;              // s since questOpen; s in the current mode
    int introPage;
    const char* banner; int bannerScale; float bannerTimer;
    bool soundRequested[SOUND_COUNT];
    bool wantsMainMenu;                 // main.cpp goes back to the main menu and clears it
    bool devAids, godMode;
    float trauma;
    float shakeStrength = 1; bool reduceFlashing;   // draw-only, copied from Settings by main.cpp
    std::vector<std::string> logLines;  // spec §18 lines for main.cpp to print
};

void questOpen(Quest& q, const char* folder);             // reads the slot summaries, shows the slot screen
void questUpdate(Quest& q, const Actions& a, float dt);
void questDraw(const Quest& q);
bool questCanPause(const Quest& q);                       // true in PLAY (and, later, its menus)
bool questSave(Quest& q);                                 // writes the current slot (spec §17); false on failure
void questStartDev(Quest& q, const char* spec);           // NAMEGAME_QUEST (spec §20)
// Movement and tiles, also used by the reachability test:
char tileAt(const AreaState& area, int tx, int ty);       // outside: '#' past a closed edge, '.' past an open one; '.' below (a pit)
void loadArea(AreaState& area, int world, int index, uint32_t gates, uint32_t shortcuts);   // copies the tiles, opens gates and doors
void moveHero(Hero& hero, const Actions& a, const AreaState& area, HeroClass heroClass, float dt);
bool heroInLiquid(const Hero& hero, const AreaState& area);
// Save files (quest_save.cpp; stdio only):
bool saveWrite(const char* folder, int slot, const SaveData& save);
bool saveRead(const char* folder, int slot, SaveData& save);    // false when missing
bool saveDelete(const char* folder, int slot);
```
- `quest_areas.cpp`: `worldDef` returns from a `static const WorldDef WORLDS[WORLD_COUNT]`. Worlds 1–7 get their name and
  style now with `areas = nullptr, count = 0` (later phases fill them; world files will be `quest_world<N>.cpp`, each
  defining `extern const AreaDef WORLD<N>_AREAS[]`). Styles from spec §13 (step sounds: Namula STONE, 1 METAL, 2 LEAVES,
  3 SNOW, 4 ROCK, 5 SAND, 6 MARBLE, 7 PAPER, test STONE).
- Author **Namula** (world 0, 3 wide areas, spec §11 table): area 0 with `S` near the middle and the hearth `C` 24 px
  right of it; NPC spots `N` (npcs -1 for now, so nothing spawns), a cliff on the right edge going to area 1, stairs down
  on the left to area 2. Area 1: higher cliff paths, ladders, the well, a `P` Sky Stair arch (portals[0] closed: area -1)
  at the top right. Area 2: the shore, the sea `~` at the bottom-left (the only way to die in Namula), 3 `P` arches
  (Trial, Unwritten, and a decorative one — all closed). Namula must be calm to walk: wide floors, small steps.
- Author **TEST GROUNDS** (world 8, name `TEST GROUNDS`, 4 areas) to exercise every phase-1 tile: area 0 (wide): `C`,
  steps of 1–4 tiles, a 4-tile gap over a pit, a one-way platform stack, a ladder, right edge → area 1. Area 1 (tall):
  ladders and platforms up to a top exit → area 2, bottom-left exit back to area 0. Area 2 (wide): ice `i`, spikes `^`,
  a liquid pool `~`, currents `<`, a vent `v`, crumble `c`, a `P` → area 3. Area 3 (wide): a dead end with a `T`
  and a `P` back to area 2. Every tile reachable (the step 8 test proves it).
- `quest_sprites.h`: the 4 classes' 8 hero sprites each (spec §21 looks; trim in `h` only), the checkpoint brazier
  unlit + 3 flame frames (12 x 16), the hearth (16 x 20, 3 frames), a portal (16 x 24, 2 frames) and a closed arch
  (16 x 24), a chest closed/open (10 x 8). Name them `HERO_<CLASS>_<FRAME>`, `CHECKPOINT_UNLIT`, `CHECKPOINT_LIT_A/B/C`, etc.
  Add every sprite to an `ALL_QUEST_SPRITES` list in `tests/quest_tests.cpp` (step 8) so their sizes are checked.

## Step 5: movement and areas (`source/quest.cpp`)
Implement `tileAt`, `loadArea`, `moveHero`, `heroInLiquid` to spec §4 and §5. The subtle parts:
- **Solidity per axis.** Move x, resolve against solid tiles (`#`, `i`, `c`, closed `g`/`k`, `f`, closed `u` is solid
  in phase 1); then move y, resolve against solid tiles and, when falling, one-way tops (`-`, a ladder top) the hero's
  previous bottom was at or above. `onGround` = something solid or one-way directly under the hitbox after the y move.
- **Order inside `moveHero`:** timers (coyote set to COYOTE_TIME while grounded, else counts down; jumpBuffer set on
  `a.jump`) → ladder (start climbing when `a.moveY < -0.5` or `> 0.5` overlapping a ladder column and not on a portal or
  NPC — those checks are the caller's, pass `a` with moveY cleared when Up was used to interact) → horizontal accel
  toward `moveX * walk` (GROUND/AIR/ICE accel; ice when the tile under the hero's centre is `i`) → jump (buffer > 0 and
  (coyote > 0 or climbing) → vy = -JUMP_SPEED, climbing off; down held + jump on a one-way → drop: ignore one-way tops
  for 0.2 s instead) → Thief double jump (in the air, `a.jump`, not used yet → vy = -DOUBLE_JUMP_SPEED) → jump cut
  (`!a.jumpHeld && vy < -JUMP_CUT` → vy = -JUMP_CUT) → gravity, MAX_FALL → currents (`<` `>` add ±50 px/s to x while
  overlapped) → vents → move and collide.
- World 5's underwater numbers come in phase 6; keep the physics constants in one block so a per-world multiplier fits later.
- `lastSafeX/Y`: set whenever grounded on `#`/`i` (not `c`), with no `^` or `~` within 2 tiles of the feet.
- `heroInLiquid`: any `~` tile overlapping the hitbox.

## Step 6: the quest flow (`source/quest.cpp`, `source/quest_save.cpp`)
- `quest_save.cpp`: spec §17 format and the tmp-then-rename write; `saveRead` resets to defaults first. Hex fields with
  `%x`. `saveDelete` = `remove`. No logging here (it has no debugnet; return values only).
- **Quest files never call `debugNetPrintf`** (they have no debugnet). They append §18 lines (built with `snprintf`)
  to `q.logLines`; main.cpp prints and clears them every frame.
- `questOpen`: `folder`, read the 3 summaries via `saveRead`, mode SLOTS, cursor 0.
- Modes in `questUpdate` (one `switch`):
  - SLOTS: up/down move the cursor (wrapping); `a.jump` on empty → CLASS (cursor = 0); on used → load, enter play;
    `a.item` on used → DELETE (cursor 0 = NO); `a.back` → `wantsMainMenu`.
  - DELETE: up/down; `a.jump` on YES → `saveDelete`, summary cleared, log `deleted`, back to SLOTS; NO or `a.back` → SLOTS.
  - CLASS: left/right (wrapping); `a.jump` → new `SaveData` (used, class), `saveWrite`, log `new`, INTRO page 0;
    `a.back` → SLOTS.
  - INTRO: 4 pages of spec §1, typed at 50 chars/s; `a.jump` completes the page or goes on; after page 4 → play at
    Namula's `S`.
  - PLAY: `moveHero`, footsteps every 0.28 s on the ground (the world style's step sound), SOUND_Q_JUMP on a jump,
    SOUND_Q_LAND when landing after falling faster than 120, edge/portal/pit/liquid handling (spec §4 "Edges"), the
    area banner on a first visit (`visited` bit, log `quest: area`), playtime += dt, camera (below), dev aids.
    Up pressed (`a.up`) on the ground while overlapping a portal whose link is open → SOUND_Q_PORTAL, go there.
  - DEAD: 2.5 s, then respawn at the checkpoint (`save.checkpointWorld/Id`; id n = the n-th `C` scanning areas 0..19
    in order, rows top to bottom), deaths += 1, autosave.
- **Camera:** `targetX = hero centre x - 160 + (facingLeft ? -32 : 32)`; `camX += (targetX - camX) * min(1, 5 * dt)`;
  tall areas: `targetY = hero centre y - 90`, eased at 6/s; wide areas: `camY = area height - 180` (bottom-aligned).
  Clamp both to the area (wide: x 0..320; tall: y 0..180). Snap (no easing) when entering an area.
- Entering an area: `loadArea`, place the hero at the link tile's bottom-centre (`x = tx*8`, `y = (ty+1)*8 - 14`),
  facing away from the entry edge, snap the camera.
- `questCanPause`: mode PLAY or DEAD.
- `questStartDev`: parse `key=value` tokens (spec §20), fill a fresh SaveData (level, class, job, names, weapon=armor=gear,
  items, gate → set the world's gate bit, highest = max(world, 1)), write it to the slot, enter play at the world's area
  (`area`) on its first standable tile from the left.
- Dev aids (only when `devAids`): `a.godMode` toggles `godMode` (log `quest: test mode on|off`); `a.autopilot`
  (Select) warps (§20).

## Step 7: drawing (`source/quest_draw.cpp`)
- PLAY / DEAD: background (Namula: spec §13; test world: plain `k` with a `n` grid every 32 px), then tiles in view
  (`#` top/fill by the tile above, `-` a 2 px platform with posts, `|` rails and rungs, `^`, `~` with 2 shimmer frames
  every 0.25 s, `i`, `c` with cracks, `g` bars, `k` a door, `z`, `f`, `<` `>` faint streaks, `v` grille, `u`), objects
  (checkpoint/hearth, portal or closed arch, chest), the hero (frame from state: climbing → CLIMB alternating every 0.2 s
  of movement, air → JUMP, walking → WALK_A/B every 0.15 s, else IDLE_A/B every 0.5 s; flipped when facing left; trim
  recoloured by job), HUD (`LV n` and the HP bar with full HP for now), the banner. Screen shake from `trauma` like the
  shooter's (`shakeStrength`, `reduceFlashing`).
- DEAD: the frozen play picture + `YOU FADED`.
- SLOTS, DELETE, CLASS, INTRO: spec §15 layouts (DELETE uses `drawMenu("DELETE SLOT n?", {"NO","YES"}, ...)` over the slots).
  Class cards' sprite is the IDLE_A frame at 1x.
- Draw only the tiles inside the camera rectangle (41 x 24 columns/rows at most); no full-screen clears beyond the
  background fill.

## Step 8: tests (`tests/quest_tests.cpp`, `Makefile`)
- `Makefile`: `test` builds and runs both binaries; the second line is
  `c++ -std=c++14 -O2 -Wall -Wextra -Isource tests/quest_tests.cpp $(wildcard source/quest*.cpp) source/screen.cpp -o build/tests/quest_tests && build/tests/quest_tests`.
  `mac`: add `$(wildcard source/quest*.cpp)` to the source list. (ps3.mk already compiles every `source/*.cpp`.)
- Tests (named functions called from `main`, `CHECK` macro copied from `game_tests.cpp`):
  - `areaSizesAndObjectsAreValid`: every area of every world with areas: `strlen(tiles) == cols * rows`, size is wide or
    tall, only legend characters, `P` count ≤ 4 and each used portal index has a link, `T`/`N` counts match their lists.
  - `linksArriveOnStandableTiles`: every open link's target tile: hitbox fits there and the tile below is solid,
    one-way or a ladder (or the target is a ladder tile).
  - `edgeLinkedAreasAreMapNeighbours` (spec §4 map adjacency).
  - `everythingIsReachable`: the physics BFS below, for Namula and TEST GROUNDS now; phases 4–9 add their worlds to the list.
  - `jumpPeakIsFourAndAHalfTiles`: a standing jump with X held peaks 36 ± 1 px above the start; a tap (released at once) peaks ≤ 12 px.
  - `coyoteAndBufferWork`, `oneWayDropThrough`, `ladderClimbAndJumpOff`, `iceSlides` (braking on ice takes ≥ 0.4 s
    from full speed), `thiefDoubleJumpsOthersDont`.
  - `saveRoundTrip`: every field written and read back equal; `missingSaveIsUnused`; `deleteRemovesTheSlot`.
  - `questSpritesHaveTheirSize`: `strlen(pixels) == w * h` for `ALL_QUEST_SPRITES`.
- **The reachability BFS** (a test helper, not game code; keep it in `quest_tests.cpp`):
  - Spots = tile positions where the 8x14 hitbox stands (`x = tx*8`, bottom on the tile top) or is on a ladder.
  - From each spot, simulate `moveHero` at 60 Hz with these scripts, each until grounded again (max 2.5 s) or an exit:
    walk left / right for 8 px; walk off left / right; jump straight; jump + hold left / right (X held 0.3 s);
    tap-jump + hold left / right (X released after 0.05 s); drop through; climb up / down 8 px.
    Simulate as CLASS_KNIGHT (walk 66, the slowest, no double jump), so every layout works for every class.
  - A landing spot is the tile under the hitbox's centre. Hitting `~` or a pit ends the script with no result. Crossing
    an open edge or using a portal (Up on a portal spot) records the exit.
  - Per world: a graph of arrival points (link targets, `S`, every `C`); BFS from area 0's first `C` (Namula: `S`) with
    gates open; assert every arrival point, `C`, `M`, `N`, `T`, `K`, `Q`, `W` spot is reached, and that from every
    arrival point some checkpoint is reachable (no soft lock). Phase 4 adds the closed-gate assertions (§4 rule 3).
  - Approximations (write them in a comment): `c`, `u`, closed `g`/`k` count as solid; `O` platforms count as a one-way
    strip over their whole travel; hazards that only hurt (`^`, `z`, `f`) don't block.
  - Print the slowest area's time; the whole test should stay under 20 s on the Mac.

## Step 9: wiring (`source/main.cpp`)
- `MAIN_ITEMS = {"CLASSIC", "LEVEL PICKER", "NAME QUEST", "SETTINGS"}`, `PICKER_ITEM = 1, QUEST_ITEM = 2, SETTINGS_ITEM = 3`.
  `itemCount`: settings 4, picker LEVEL_COUNT, main menu 4, pause 3.
- `enum Scene` + `SCENE_QUEST`, `SCENE_NAMES` + `"quest"`. A `Quest quest;` next to `game`, `bool questPaused = false;`.
- Main menu confirm on QUEST_ITEM: `audioStopMusic()`, `questOpen(quest, exeFolder.c_str())`, `quest.devAids = DEV_AIDS`,
  scene QUEST.
- At startup, after the settings: if `DEV_AIDS && getenv("NAMEGAME_QUEST")` → the same, then `questStartDev`.
- `case SCENE_QUEST`: `if (quest.wantsMainMenu) { quest.wantsMainMenu = false; selected = QUEST_ITEM; setScene(SCENE_MAIN_MENU); }`
  else `if ((actions[0].pause || !actions[0].connected) && questCanPause(quest)) { questPaused = true; selected = 0; setScene(SCENE_PAUSED); }`.
- `SCENE_PAUSED`: resuming goes to `questPaused ? SCENE_QUEST : SCENE_PLAYING` (and clears `questPaused` when it goes
  to the quest); the lost-controller check uses `!actions[0].connected` when `questPaused`. RETURN TO MENU with
  `questPaused`: `questSave(quest)`, `questPaused = false`, then as today.
- Settings opened from pause draws `questPaused ? questDraw(quest) : drawGame(game)`; PAUSED draws the same way.
  The music item there must not start a song while `questPaused` (NAME QUEST has no music: skip `audioStartMusic`).
- Update loop: `if (scene == SCENE_QUEST) questUpdate(quest, actions[0], STEP);` and after each step, clear every press
  field of `actions[0]` (`jump, item, rocket, cycle, back, up, down, left, right, confirm, pause, godMode, autopilot`).
- After the update: in SCENE_QUEST play and clear `quest.soundRequested`, print and clear `quest.logLines`
  (`debugNetPrintf(INFO, "%s\n", ...)`), set `quest.shakeStrength`, `quest.reduceFlashing`.
- The 5-second fps line: when the scene is QUEST (or paused from it) print `quest %d.%d` (world.area) instead of the wave and score.
- Draw: `case SCENE_QUEST: questDraw(quest)`.
- The Triangle stress test and the shooter's autopilot stay in SCENE_PLAYING only (already true).

## Step 10: docs
- New `docs/features/name-quest/design.md` (status `in progress`), `implementation.md`, `testing.md`, shaped like
  `docs/features/co-op/`. design.md: a one-paragraph summary, then "Rules and numbers: see the work-plan spec
  [`spec.md`](../../work-plan/name-quest/spec.md) until phase 9 folds the final numbers in here", then the phase-1
  player-facing rules as a numbered list. implementation.md: files and how the quest is wired into main.cpp.
  testing.md: the phase-1 PS3 checks below as unchecked criteria plus the Mac tests.
- `docs/GDD.md`: Summary gets one sentence about NAME QUEST; a feature-index row `NAME QUEST | in progress | MVP |
  RPG platformer mode: 4 classes, 6 worlds of 20 areas, Namula hub, souls-like death |` and a docs cell linking
  `features/name-quest/design.md` like the other rows;
  the flow diagram gets the NAME QUEST branch; pillar 3 gets "(Classic and the level picker)"; decisions D-035 to D-051
  from the master plan's table (date 2026-11-20, "Alternatives considered" = the options the user didn't pick, "Why" =
  "user's choice"); change log `0.29 (2026-11-20): NAME QUEST designed (D-035 to D-051); phase 1 engine`. Bump
  `**Version:**` to 0.29.
- `docs/features/screens/design.md` rule 1: the main menu items now include NAME QUEST.
- `CLAUDE.md` › Commands: one line under `make mac-run`: `NAMEGAME_QUEST='slot=3 class=0 world=8 area=0' make mac-run BUTTONS=...`
  starts NAME QUEST in play (spec §20).
- Run `python3 .claude/skills/game-docs/scripts/check_docs.py docs`; fix what it reports.

## Step 11: Mac self-check (you, no user needed)
```
NAMEGAME_QUEST='slot=3 class=3 world=0 area=0' make mac-run BUTTONS='2|right:1.5|cross|1|l1|1|quit'
```
Read `shots/shot.png`: the Thief in Namula, HUD at top-left. Then a script that walks into the sea on Namula's shore
(`world=0 area=2`) and confirm `quest: hero died in 0.2 (liquid)` in stdout and the respawn at the hearth. Then one that
climbs the TEST GROUNDS tall area (`world=8 area=1`). Fix anything wrong before the PS3.

## Step 12: PS3 check (needs the PS3; the user only watches)
1. `make ps3` (no new warnings in quest files).
2. One locked command (fill the button script to reach the slot screen: main menu down twice, X):
   ```
   scripts/ps3-lock.sh bash -c 'make install launch && (make log > /tmp/nq-log.txt &) && make pad BUTTONS="down|down|cross|1|cross|1|cross|1|right|cross|2|cross|1|cross|1|cross|1|cross|2|right:2|cross|1|l1" && make shot; sleep 1; make pad BUTTONS="start|1|down|down|cross|3"; make quit'
   ```
   (empty slot 1 → class Archer → 4 intro pages → walk and jump → screenshot → pause → RETURN TO MENU.)
   Kill the background `make log` afterwards (`lsof -nP -iUDP:18195`).
3. Expect in the log: `scene: quest`, `quest: slot 1 new (class ARCHER, level 1)`, `quest: area 0.0 NAMULA`,
   `quest: save slot 1 ok` (or `rename failed, wrote directly` — then note it; it still counts), and `fps` ≈ 60 with
   worst work under 10 ms. The screenshot shows the Archer in Namula.
4. Relaunch, open NAME QUEST, X on slot 1: `quest: slot 1 loaded (class ARCHER, level 1)`.
5. Tick the testing.md criteria the log proves.

## Done when
- `make test` passes (both binaries), `make ps3` builds, `check_docs.py` is clean.
- On the PS3: main menu → NAME QUEST → new slot → class → intro → walking and jumping in Namula at 60 fps; the slot
  loads again after a relaunch.
