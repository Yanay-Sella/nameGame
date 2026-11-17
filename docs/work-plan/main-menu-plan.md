# Plan: main menu, pause menu, settings (music on/off)

For the implementer: follow the steps in order. Every design choice below is final, so don't redesign anything.
Read only the files each step names. Code style: follow `CLAUDE.md` and the existing code (plain C++14, no new
libraries, short one-line comments, no helpers for one-off logic). Commit straight to `main` when done.

## What the player gets
```
Boot → MAIN MENU ─ CLASSIC ──→ PLAYING ⇄ (Start) PAUSED ─ RETURN TO MENU ─→ MAIN MENU
           └─ SETTINGS ⇄           │                  └─ SETTINGS ⇄
                                   └─ game over: X/Start = play again, Circle = MAIN MENU
```
- **Main menu**: title "NAMEGAME" and the items `CLASSIC`, `SETTINGS`. Starfield scrolls behind it. No music.
  `CLASSIC` is the only game mode for now: it is today's game, started with 1 player (player 2 still drops in with Start).
- **Settings**: items `MUSIC: ON` / `MUSIC: OFF`, then `BACK`. X (or D-pad left/right) on the music item toggles it.
  Circle or `BACK` returns to whichever menu opened settings. The value is saved to a file and survives a reboot.
- **Pause menu** (Start during a run): title "PAUSED", items `RESUME`, `SETTINGS`, `RETURN TO MENU`, drawn over the frozen game.
  Start or Circle also resume. `RETURN TO MENU` ends the run (no confirm) and stops the music.
- **Music off** means the song never plays; sound effects are unaffected. Toggling it on while paused starts the song
  from the beginning; toggling it off stops it at once.

## Input rules (all menus)
- Every connected controller drives the menus (OR of all controllers' actions).
- Up/down: D-pad press, or a left-stick flick (see step 2). The cursor wraps around.
- X selects. Start selects too, except in the pause menu, where Start resumes.
- Circle goes back: settings → its opener, pause → resume, main menu → nothing.
- Circle is button 10, known only from the SDL port's source. If it turns out wrong on the PS3, every screen is still
  reachable without it (`BACK`, `RESUME`, `RETURN TO MENU` items).

## Step 1: `source/settings.h` + `source/settings.cpp` (new)
Stdio only (no SDL, no debugnet), so the Mac tests can build it.
```cpp
// Player settings saved next to EBOOT.BIN as "key=value" lines (docs/features/screens).
struct Settings {
    bool music = true;   // the song plays during runs
};
Settings settingsLoad(const char* path);                     // defaults when the file is missing or unreadable
bool settingsSave(const char* path, const Settings& settings);   // false when the file couldn't be written
```
- File format: one line `music=1` or `music=0`. Load reads lines with `fgets` and parses `sscanf(line, "music=%d", &v)`;
  unknown lines are ignored (future settings just add lines).
- Save writes the whole file with `fopen(path, "w")`, `fprintf`, `fclose`; return whether all three succeeded.

## Step 2: input (`source/game.h`, `source/input.cpp`)
Add to `struct Actions` in `game.h`:
```cpp
    bool up, down, left, right;   // D-pad pressed or left stick flicked (menus)
    bool back;                    // Circle pressed
```
In `input.cpp`:
- Add `BTN_CIRCLE = 10` to the button enum.
- `a.up/down/left/right` = `pressed[player][BTN_UP/DOWN/LEFT/RIGHT]` OR a stick flick. `a.back = pressed[player][BTN_CIRCLE]`.
- Stick flick with hysteresis, per player and axis, using the raw axes (not the deadzoned move values):
```cpp
static int stickFlick[MAX_PLAYERS][2];   // last flick direction of the left stick per axis: -1, 0 or 1

// in inputRead, after the movement code:
for (int axis = 0; axis < 2; axis++) {
    float v = SDL_JoystickGetAxis(joystick, axis) / 32768.0f;
    int dir = v < -0.6f ? -1 : v > 0.6f ? 1 : fabsf(v) < 0.3f ? 0 : stickFlick[player][axis];
    if (dir != 0 && dir != stickFlick[player][axis]) {
        if (axis == 0) (dir < 0 ? a.left : a.right) = true;
        else (dir < 0 ? a.up : a.down) = true;
    }
    stickFlick[player][axis] = dir;
}
```

## Step 3: drawing (`source/sprites.h`, `source/screen.h/.cpp`, `source/game.h/.cpp`)
- `sprites.h` › `fontGlyph`: add `case '>': return 042124;` (cursor). Font text is not used by the XMB art, so don't rerun `make_xmb_art.py`.
- `screen.h/.cpp`: add
  ```cpp
  void drawMenu(const char* title, const char* const* items, int count, int selected, const char* hint);   // centred panel
  ```
  Layout (all px, 320×180 screen): panel 150 wide, centred horizontally and vertically, 1 px border `paletteColor('n')`
  filled `paletteColor('k')` (same look as the game-over panel). Inside, from the panel top: title at +8, scale 2, white;
  first item at +28, then one item every 12, scale 1, centred; hint 8 below the last item in `paletteColor('s')`;
  panel bottom 8 below the hint. Unselected items white, the selected one `paletteColor('y')` with `>` drawn 8 px left of it.
- `game.h/.cpp`: split the starfield out of `updateGame`/`drawGame` (they then call these), because the menus reuse it:
  ```cpp
  void updateBackground(Game& game, float dt);   // scrolls the starfield
  void drawBackground(const Game& game);         // navy fill and stars
  ```
- `game.cpp` › `drawGame`, game-over panel: replace `"PRESS X TO PLAY AGAIN"` with `"X PLAY AGAIN   O MENU"`.

## Step 4: scenes in `source/main.cpp`
Replace the `paused` bool with a scene. Keep the flow in `main()` as one readable switch (no scene classes or tables).
```cpp
enum Scene { SCENE_MAIN_MENU, SCENE_SETTINGS, SCENE_PLAYING, SCENE_PAUSED };
const char* const MAIN_ITEMS[] = {"CLASSIC", "SETTINGS"};          // game modes are added above SETTINGS
const char* const PAUSE_ITEMS[] = {"RESUME", "SETTINGS", "RETURN TO MENU"};
```
Startup:
- `settingsPath = <folder of EBOOT.BIN> + "/settings.txt"` (the folder `audioOpen` already gets). `settings = settingsLoad(...)`;
  log `settings: music on|off`.
- Keep `startGame(game, 1, seed)` at boot (it gives the menu its stars); remove the boot `audioStartMusic()`.
- `Scene scene = SCENE_MAIN_MENU, settingsBack = SCENE_MAIN_MENU; int selected = 0;`

Each frame, after reading `actions[p]`, OR all players' actions into one `Actions menu`, then:
- Up/down in any menu scene: `selected = (selected + count ± 1) % count`, where `count` is that scene's item count.
- **SCENE_MAIN_MENU**: confirm on `CLASSIC` → `startGame(game, 1, seed)`, `if (settings.music) audioStartMusic()`, scene PLAYING.
  Confirm on `SETTINGS` → `settingsBack = SCENE_MAIN_MENU`, scene SETTINGS, `selected = 0`.
- **SCENE_SETTINGS**: back, or confirm on `BACK` → scene = `settingsBack`, `selected = 1` (the `SETTINGS` item in both menus).
  Confirm or left/right on the music item → flip `settings.music`; if now off `audioStopMusic()`, else if
  `settingsBack == SCENE_PAUSED` `audioStartMusic()`; then `settingsSave` and log `settings: music on|off (saved|SAVE FAILED)`.
- **SCENE_PAUSED**: check `menu.pause || menu.back` first → scene PLAYING. Otherwise confirm on `RESUME` → PLAYING;
  on `SETTINGS` → `settingsBack = SCENE_PAUSED`, scene SETTINGS, `selected = 0`; on `RETURN TO MENU` → `audioStopMusic()`,
  scene MAIN_MENU, `selected = 0`.
- **SCENE_PLAYING**: today's per-player loop, with these changes:
  - Join (X/Start on a controller without a ship) only here; drop the `!paused` checks.
  - `actions[p].pause && !game.over` → clear every player's `rocketPressedAt`, scene PAUSED, `selected = 0`, then `break`
    out of the player loop (so a second Start the same frame can't act).
  - Game over after `GAME_OVER_INPUT_LOCK`: confirm → new run as today, but `if (settings.music) audioStartMusic()`;
    `actions[p].back` → scene MAIN_MENU, `selected = 0` (music already stopped at game over).
  - Triangle stress test only here.
- Log every scene change: `scene: main menu|settings|playing|paused`.

Fixed-step loop: `updateGame` only in PLAYING; `updateBackground` in MAIN_MENU and in SETTINGS opened from the main menu;
nothing in PAUSED or SETTINGS opened from pause (the run stays frozen). Play and clear `game.soundRequested` only in PLAYING.
The game-over detection (`game.over && !wasOver` → stop music) stays as is.

Drawing:
- PLAYING: `drawGame`. PAUSED: `drawGame` + `drawMenu("PAUSED", PAUSE_ITEMS, 3, selected, "X SELECT  O RESUME")`.
- MAIN_MENU: `drawBackground` + `drawMenu("NAMEGAME", MAIN_ITEMS, 2, selected, "X SELECT")`.
- SETTINGS: `drawGame` if opened from pause, else `drawBackground`; then `drawMenu("SETTINGS", {musicLabel, "BACK"}, 2,
  selected, "X CHANGE  O BACK")` with `musicLabel` = `"MUSIC: ON"` / `"MUSIC: OFF"`.
- Remove the old `"PAUSED"` text line.

## Step 5: tests (`tests/game_tests.cpp`, `Makefile`)
- `Makefile` › `test`: add `source/settings.cpp` to the `c++` line.
- New test functions, called from `main()`:
  - `settingsDefaultToMusicOn`: `settingsLoad("build/tests/missing.txt").music == true`.
  - `settingsSurviveASave`: save `music = false` to `build/tests/settings.txt`, load it back → false; save true → true.
  - `menuCursorHasAGlyph`: `fontGlyph('>') != 0`.
- Run `make test`: all must pass.

## Step 6: docs (game-docs conventions; copy the shape of `docs/features/co-op/*.md`)
- New `docs/features/screens/design.md`, `implementation.md`, `testing.md`. Design = the "What the player gets" and
  "Input rules" sections above as numbered rules; edge cases: Circle unconfirmed, music toggled while paused, save failure
  (game keeps the in-memory value). Testing = the PS3 checks in step 7 as unchecked criteria plus the three Mac tests.
- `docs/GDD.md`:
  - Feature index › Screens: status `in progress`, summary "Main menu (Classic, Settings), pause menu, settings with music
    on/off, game over", docs link to `features/screens/design.md`. The high score on the title stays with High score save.
  - Rewrite "Screens and flow" to the diagram above; Controls line: add "Circle backs out in menus"; Accessibility line:
    "in the settings screen" instead of "in the pause menu".
  - Decision `D-010 | 2026-11-17 | A main menu with game modes (only Classic for now) and a Settings screen reachable from
    the main and pause menus; music on/off saved to settings.txt | title screen with "press X" only | the user wanted
    room for more modes and settings | screens, music and sound, co-op`.
  - Change log `0.7`: main menu, pause menu and settings.
- Update rules that change: `co-op/design.md` rule 6 and `lives-and-score/design.md` rule 4 (Circle → main menu; drop
  "the M3 screens replace this"); `music-and-sound/design.md` rule 1 (music only if enabled in settings; the main menu is
  silent).
- Run `python3 .claude/skills/game-docs/scripts/check_docs.py docs` and fix what it reports.
- Don't touch `docs/work-plan/make_work_plan.py` or the PDF.

## Step 7: PS3 check (needs the user)
1. `make ps3` must build without warnings in the new code. Then start `make log` in the background and run `make deploy`
   (it refuses if a game is running; ask the user to quit to the XMB).
2. Ask the user to launch nameGame and try, in order: menu up/down with D-pad and stick; Settings → music off → Back →
   Classic (silent run, SFX still play); Start → Settings → music on (song starts) → Circle → Resume; Start → Return to
   menu; quit and relaunch → music setting kept; lose a run → Circle at game over → main menu.
3. Confirm from the log: `scene:` lines for each step, `settings: music off (saved)`, `button 10` for Circle, fps ≈ 60.
   If Circle shows a different number, fix `BTN_CIRCLE`. If it is 10, add 10 to the confirmed list in `CLAUDE.md`.
4. Tick the testing.md criteria that the log proves; leave the rest unchecked with a note.

## Done when
- `make test` passes, `make ps3` builds, `check_docs.py` is clean.
- On the PS3: boot → main menu → Classic → pause → settings → return to menu works, and the music setting survives a relaunch.
