# Plan: Milestone 4 (release polish)

For the implementer: follow the steps in order. Every design choice below is final, so don't redesign anything.
Read only the files each step names, and don't re-read a file after editing it. Code style: follow `CLAUDE.md` and
the existing code (plain C++14, no new libraries, short one-line comments, no helpers for one-off logic).
Don't commit: the user commits their own work. At each stopping point below, list the changed files (this plan
included) and a one-line commit message in the style of `git log`. Never `git add -A`.
Don't read `docs/work-plan/make_work_plan.py` or the PDF before step 12; the M4 tasks are summarised here.
Pipe long output: `make test 2>&1 | tail -15`, `make deploy 2>&1 | tail -8`.

## What the player gets
Work plan M4 tasks: 1 name and icon, 2 balancing to wave 20, 3 controller unplug, 4 30-minute stability run,
5 install guide and versioned release package, 6 final docs sync. GDD: "M4 done when a fresh player installs the pkg
from the guide and plays a full run without help."
- **Name:** the game is called **NAME game**. That exact string is the XMB title. The pixel fonts are uppercase only,
  so the main menu title and the XMB icon logo read `NAME GAME`. The repo, app ID `NMGM00001`, `build/nameGame.pkg`,
  the Docker image and the log lines keep "nameGame": a new app ID would install a second game and orphan the saved
  high score and settings.
- **Version:** `V1.0` in grey at the bottom right of the main menu. The release file is `dist/NAME-game-1.0.pkg`.
- **Controller unplug:** if a joined player's controller disconnects during a run (not at game over), the game pauses.
  The pause menu shows the hint `CONNECT CONTROLLER n` (n = 1 or 2) instead of `X SELECT  O RESUME`, and Resume does
  nothing until that controller is back. After it reconnects, the pause menu stays up with its normal hint; the player
  resumes with Start, Circle or RESUME as after any pause. Settings and RETURN TO MENU still work while disconnected.
- **Difficulty keeps climbing to wave 20** (it used to stop rising at wave 12):
  - Enemy shot speed: 70 px/s up to wave 10, then +2 per wave, capped at 90 (wave 11: 72 … wave 20: 90).
  - From wave 13: rows 1–2 red, rows 3–4 blue, row 5 green (before: wave 7+ row 1 red, row 2 blue; wave 4+ rows 1–2 blue).
  - From wave 15: up to two aliens dive at the same time (first seen on wave 16, since 15 is a boss).
  - Target: a newcomer's runs end around wave 6–8; a good player reaches wave 15–20.
- **Release build** (`make release`): no dev aids (Triangle stress test, Select autopilot), logs go to 127.0.0.1,
  the song is included.
- **Install guide:** `README.md` at the repo root, copied next to the release pkg.

## Dev aids (dev builds only, never in `make release`)
- Triangle: the existing stress test.
- **Select (button 7, from the SDL port's source, unconfirmed) toggles the autopilot** during a run: player 1 sweeps
  left and right along the bottom, holds fire, launches a rocket every 5 s and stays invincible (the ship blinks the
  whole time, which shows it is on). Turning it off leaves 1 s of invincibility. It exists for the 30-minute soak test.

## Step 1: build flags, version, release target (`Makefile`, `ps3.mk`, `.gitignore`)
- `Makefile`:
  - Add `VERSION  := 1.0` and `RELEASE  ?= 0` under `PKG`.
  - In the `ps3` recipe, add `RELEASE=$(RELEASE) VERSION=$(VERSION)` to the `make -f ps3.mk pkg …` arguments.
  - Add `release` to `.PHONY` and this target after `deploy` (recipe lines start with a tab):
    ```make
    # Release package for other players: no dev aids, logs go nowhere, copied to dist/ with the install guide.
    release:
    	@test -f pkgfiles/USRDIR/music.ogg || { echo "pkgfiles/USRDIR/music.ogg is missing; run make music first."; exit 1; }
    	$(MAKE) ps3 RELEASE=1 LOG_HOST=127.0.0.1
    	mkdir -p dist
    	cp build/nameGame.pkg "dist/NAME-game-$(VERSION).pkg"
    	cp README.md "dist/NAME-game-$(VERSION)-README.md"
    	ls -l dist
    ```
  - `deploy`'s last line: `@echo "Installed. Launch NAME game from the PS3's Game menu."`
- `ps3.mk`: `TITLE := NAME game`. Add `RELEASE ?= 0` and `VERSION ?= 0.0` under `LOG_PORT`, and append
  `-DRELEASE=$(RELEASE) -DGAME_VERSION=\"$(VERSION)\"` to the `CFLAGS` line that has `-DLOG_HOST` (keep the `\`
  continuation layout). Don't change `TARGET`, `APPID` or `CONTENTID`.
  The SDK's default `sfo.xml` sets `APP_VER` 01.00, which matches 1.0; no own sfo.xml now.
- `.gitignore`: add `dist/`.

## Step 2: the name (`source/main.cpp` strings only, `docs/xmb-art/make_xmb_art.py`, `CLAUDE.md`)
- `main.cpp`: `SDL_CreateWindow("NAME game", …)`; main menu `drawMenu("NAME GAME", …)`. Keep the `nameGame started` /
  `nameGame exiting` log lines (testing docs quote them). Step 5 adds the version text.
- `make_xmb_art.py`:
  - `LOGO_FONT`: add a space, `" ": ["...", "...", "...", "...", "...", "...", "..."],`.
  - `logo()`, glint loop: the space has no ink, so replace the `first = next(...)` line with
    `first = next((col for col, c in enumerate(g[1]) if c == "#"), None)` and draw the glint only `if first is not None:`
    (keep `x += len(g[0]) + 1` for every glyph).
  - `make_icon()`: `logo(c, "NAME GAME", 5, 2)`. The logo is 60 units × 2 = 120 px of the 160 px icon canvas, so it fits.
  - Run `python3 docs/xmb-art/make_xmb_art.py`, then `Read` `pkgfiles/ICON0.PNG` and check the logo is centred and
    nothing overlaps it badly. PIC1.PNG has no text; it is redrawn identically.
- `CLAUDE.md`: under the first paragraph add: `The game's title is "NAME game" (GDD D-012); the repo, app ID NMGM00001,
  build/nameGame.pkg and the log lines keep "nameGame".` In Commands add:
  `` - `make release` — release build (no dev aids, logs to 127.0.0.1, song required) → `dist/NAME-game-<VERSION>.pkg` and the README. ``
  Change "The user then launches nameGame" to "The user then launches NAME game".

## Step 3: input (`source/game.h`, `source/input.cpp`)
- `game.h` › `struct Actions`, add at the end:
  ```cpp
      bool autopilot;               // Select pressed: dev autopilot toggle (main.cpp)
      bool connected;               // this player's controller is open
  ```
- `input.cpp`: add `BTN_SELECT = 7` to the enum. In `inputRead`, right after `if (!joystick) return a;` set
  `a.connected = true;`, and next to `a.stress` set `a.autopilot = pressed[player][BTN_SELECT];`.
- Disconnects are already detected: this SDL port re-counts pads every `SDL_PumpEvents` (`SDL_NumJoysticks` = number of
  connected pads), and `inputRead` closes the joystick when its index is no longer counted. Keep that code as is.
  Known limit (write it in co-op's design note, step 8): in co-op, if player 1's controller goes while player 2's stays,
  the port maps by count, so the log names controller 1 and neither pad is read; the game still pauses, and
  reconnecting player 1's controller restores both.

## Step 4: difficulty to wave 20 (`source/game.h`, `source/game.cpp`)
- `game.h`, next to `shotChance`: `float enemyShotSpeed(int wave);   // px/s; rises after wave 10`.
- `game.cpp` constants, Enemy waves group: keep `ENEMY_SHOT_SPEED = 70` and add
  ```cpp
  const int SHOT_SPEEDUP_AFTER_WAVE = 10;
  const float ENEMY_SHOT_SPEED_STEP = 2, ENEMY_SHOT_MAX_SPEED = 90;   // px/s per wave, px/s
  const int MORE_ARMOUR_WAVE = 13;       // two red and two blue rows from here
  const int TWO_DIVERS_WAVE = 15;        // two aliens may dive at once from here
  ```
- `enemyShotSpeed`, next to `shotChance`:
  `return std::min(ENEMY_SHOT_SPEED + ENEMY_SHOT_SPEED_STEP * std::max(wave - SHOT_SPEEDUP_AFTER_WAVE, 0), ENEMY_SHOT_MAX_SPEED);`
- `updateAlien`, the enemy shot: `enemyShotSpeed(game.wave)` instead of `ENEMY_SHOT_SPEED`. Boss shots don't change.
- `startWave`, replace the comment and `AlienType type = …` line with:
  ```cpp
          // Tougher aliens take the top rows: red from wave 7, blue from wave 4, two of each from wave 13.
          int redRows = wave >= MORE_ARMOUR_WAVE ? 2 : wave >= 7 ? 1 : 0;
          int blueEnd = wave >= MORE_ARMOUR_WAVE ? 4 : wave >= 4 ? 2 : 0;   // rows above this that aren't red are blue
          AlienType type = row < redRows ? ALIEN_RED : row < blueEnd ? ALIEN_BLUE : ALIEN_GREEN;
  ```
  Waves 1–12 come out exactly as before.
- `startDive`: replace "one diver at a time" with a count:
  ```cpp
      int divers = 0;
      for (Alien& alien : game.aliens) {
          if (alien.alive && alien.state == ALIEN_DIVING) divers++;
          if (alien.alive && alien.state == ALIEN_IN_FORMATION) candidates.push_back(&alien);
      }
      if (divers >= (game.wave >= TWO_DIVERS_WAVE ? 2 : 1) || candidates.empty()) return;
  ```
  The rest of `startDive` stays.

## Step 5: wiring in `source/main.cpp`
- Includes: add `#include <malloc.h>` and `#include <math.h>`.
- Top constant: `const bool DEV_AIDS = !RELEASE;   // Triangle stress test and Select autopilot; off in make release`.
- Locals next to `selected`: `bool autopilot = false;` and `float bossStart = 0;   // game time the current boss appeared`.
- **Lost controller**, right after the `menu` OR-loop:
  ```cpp
          // The first joined player whose controller is gone, or -1: a run pauses until it is back (D-014).
          int lostController = -1;
          for (int p = (int)game.players.size() - 1; p >= 0; p--)
              if (!actions[p].connected) lostController = p;
  ```
- `SCENE_PAUSED`: resume (`menu.pause || menu.back || (menu.confirm && selected == 0)`) only changes the scene when
  `lostController < 0`; otherwise it does nothing. Keep it the first branch so SETTINGS and RETURN TO MENU behave as before:
  `if (resume) { if (lostController < 0) setScene(SCENE_PLAYING); } else if (…selected == 1…) … else if (…selected == 2…) …`
- `SCENE_PLAYING`, before the per-player loop:
  ```cpp
              if (lostController >= 0 && !game.over) {
                  for (Player& player : game.players) player.rocketPressedAt = -1;   // as for a Start pause
                  selected = 0;
                  debugNetPrintf(INFO, "paused: controller %d disconnected\n", lostController + 1);
                  setScene(SCENE_PAUSED);
                  break;
              }
  ```
- `SCENE_PLAYING` per-player loop: `if (DEV_AIDS && actions[p].stress) addStressLoad(game);` and below it
  `if (DEV_AIDS && actions[p].autopilot) { autopilot = !autopilot; debugNetPrintf(INFO, "autopilot %s\n", autopilot ? "on" : "off"); }`.
- **Autopilot**, after the scene switch and before `bool wasOver`:
  ```cpp
          // Dev aid (Select): player 1 sweeps along the bottom firing, a rocket every 5 s, invincible (30-minute soak test).
          if (autopilot && scene == SCENE_PLAYING) {
              Player& pilot = game.players[0];
              float targetX = 152 + 120 * sinf(game.time * 0.7f);   // 152: the ship centred
              actions[0].moveX = SDL_max(-1.0f, SDL_min((targetX - pilot.x) / 20, 1.0f));
              actions[0].moveY = 1;
              actions[0].fire = true;
              actions[0].rocket = frame % 300 == 0;
              if (pilot.alive) pilot.invincibleTimer = 1;   // runs out 1 s after the autopilot is switched off
          }
  ```
- **Balancing logs**, next to `bool wasOver`: `int waveBefore = game.wave;` and
  `int livesBefore[MAX_PLAYERS] = {}; for (size_t p = 0; p < game.players.size(); p++) livesBefore[p] = game.players[p].lives;`.
  After the update loop, next to the boss lines:
  - On `game.wave != waveBefore`, for each player:
    `"wave %d start at %.0f s: player %d lives %d, rockets %d, weapon %d, score %d\n"` (game.wave, game.time, p + 1, …).
  - For each player whose `lives < livesBefore[p]`: `"player %d died at wave %d (%d lives left)\n"`.
  - Boss lines: set `bossStart = game.time` where `boss wave %d, hp %d` is logged; the other becomes
    `"boss down at wave %d after %.0f s\n"` with `game.time - bossStart`.
- **Heap in the 5 s stats line:** append `, heap %d KB` with `mallinfo().uordblks / 1024` (newlib; bytes the heap
  has handed out).
- **Drawing:**
  - Main menu, after the `HI` line: `drawText(SCREEN_W - 2 - textWidth("V" GAME_VERSION), SCREEN_H - 7, "V" GAME_VERSION, paletteColor('s'));`
  - Paused: hint = `lostController >= 0` ? `"CONNECT CONTROLLER %d"` (lostController + 1, via `snprintf` into
    `char hint[32]`) : `"X SELECT  O RESUME"`.

## Step 6: tests (`tests/game_tests.cpp`)
New functions, called from `main()` next to the enemy-wave tests:
- `enemyShotsSpeedUpAfterWaveTen`: `enemyShotSpeed` 1 → 70, 10 → 70, 11 → 72, 20 → 90, 30 → 90. Then a real shot:
  `startSafeGame`, `run(game, NO_INPUT, 8)` (formation settled), `game.wave = 20; game.enemyShots.clear();`, step with
  `updateGame` until `enemyShots` isn't empty (at most 600 steps); CHECK it isn't empty and `vy == 90`.
- `lateWavesAddArmouredRows`: `startSafeGame`, `game.wave = 12`, kill all aliens, `run(game, NO_INPUT, 2.2f)`;
  CHECK `wave == 13`, 40 aliens, aliens 0–15 `ALIEN_RED`, 16–31 `ALIEN_BLUE`, 32–39 `ALIEN_GREEN` (rows are pushed
  in order, 8 per row). Same for wave 7 (from 6): 0–7 red, 8–15 blue, the rest green.
- `twoAliensDiveFromWaveFifteen`: copy `divesStartAtWaveThree`'s setup (`startSafeGame`, `run 8 s`). Set `game.wave = 14`;
  three times: `game.diveTimer = 0.01f; run(game, NO_INPUT, 0.05f);` → CHECK exactly 1 diver. Then `game.wave = 16`,
  the same three times → CHECK exactly 2 divers.
- Run `make test`: `all tests passed`. Then prove the new tests can fail: set `ENEMY_SHOT_SPEED_STEP` to 3,
  `MORE_ARMOUR_WAVE` to 14 and `TWO_DIVERS_WAVE` to 17 together, run `make test`, check each new test prints `FAIL`,
  undo exactly, rerun to green. Say in the reply which change broke which test.

## Step 7: install guide (`README.md`, new)
Plain Markdown, written for a player, not a developer. Sections in this order:
1. `# NAME game` and one line: an arcade shooter for the PS3: hold R1 to spray bullets at waves of aliens, save the
   R2 rocket for emergencies, beat a boss every 5th wave, chase the high score. One or two players.
2. **What you need:** a PS3 running PS3HEN (tested on firmware 4.93); HEN must be enabled after every PS3 restart.
   A USB stick formatted FAT32. One DualShock 3 (a second one for two players).
3. **Install:** (1) copy `NAME-game-1.0.pkg` to the root of the USB stick (not into a folder); (2) plug it into the PS3
   and enable HEN; (3) Game → Package Manager → Install Package Files → Standard, pick `NAME-game-1.0.pkg`; (4) NAME game
   appears in the Game column. Installing a newer version the same way keeps your high score and settings.
4. **Controls** table: Move: left stick or D-pad · Fire: hold R1 · Rocket: R2 · Pause: Start · Menus: X select,
   Circle back · Player 2: press Start on a second controller during a run.
5. **Saving:** the high score and settings are saved automatically.
6. **Troubleshooting:** not in Package Manager → HEN wasn't enabled, or the file isn't at the stick's root;
   no music → Settings → MUSIC: ON; "CONNECT CONTROLLER" → press the controller's PS button, then Start to resume;
   game frozen → hold the PS3's power button about 10 seconds until it beeps (never pull the plug).
7. **Uninstall:** highlight NAME game, press Triangle, Delete.
8. **For developers:** one line pointing at `CLAUDE.md` and `docs/ps3-new-project-guide.md`.

## Step 8: docs (game-docs conventions; edit in place)
- `docs/features/enemy-waves/design.md`: rule 4 adds "from wave 15 up to two at once"; rule 6 adds "from wave 13 the
  top two rows are red and the next two blue". Numbers: new row `Enemy shot speed | 70, +2 per wave after wave 10, max 90 | px/s | late waves stay tense [ASSUMPTION]`
  (replaces the 70 row); new rows for the armour rows and two divers. Edge case "Wave count past 5 rows" becomes
  "Past wave 15 only the sway keeps rising; everything else is capped." History: `2026-11-17: difficulty extended to wave 20 (D-013).`
  `implementation.md`: add `enemyShotSpeed` to the formulas row and the four new constants to Config.
  `testing.md`: add the three new tests as ticked criteria; add `[ ] A newcomer's runs end around wave 6–8 and a good
  player reaches wave 15–20. *(not proven: step 10)*`.
- `docs/features/ship-controls/design.md`: the edge case becomes "Unplugged controller: the run pauses with
  CONNECT CONTROLLER n; after reconnecting, the player resumes from the pause menu (D-014)." `implementation.md`: replace
  the "not built yet" deviation with "Unplug: `inputRead` sets `Actions.connected`; `main.cpp` pauses the run while a
  joined player's controller is gone and blocks Resume." `testing.md`: the unplug criterion becomes
  `[ ] … *(not proven: step 9)*`.
- `docs/features/co-op/design.md`: the unplug edge case becomes "Either player's controller unplugged: the run pauses
  until it is back (D-014). If player 1's goes while player 2's stays, the SDL port maps pads by count, so neither is
  read until player 1's controller returns; the pause covers it."
- `docs/features/screens/design.md`: rule 1 title `NAME GAME` and `V1.0` at the bottom right; rule 5 adds "A controller
  disconnect also opens it, with the hint CONNECT CONTROLLER n, and RESUME waits for that controller." History line.
- `docs/GDD.md`: version 0.14; title line `# NAME game — Game Design Document`; Main menu line: title "NAME GAME";
  Open questions: drop "working title "nameGame";"; change log `**0.14 (2026-11-17):** M4 built: name, unplug pause,
  difficulty to wave 20, release build and install guide (D-012 to D-015).` Decisions:
  - `| D-012 | 2026-11-17 | The game is called "NAME game" ("NAME GAME" in the uppercase pixel fonts); the repo, app ID NMGM00001 and file names keep nameGame | ROCKET PANIC; STARSWARM; HIVE STORM | user's choice; a new app ID would install a second game and orphan the saved high score and settings | [screens](features/screens/design.md) |`
  - `| D-013 | 2026-11-17 | Difficulty keeps climbing to wave 20: enemy shots 70→90 px/s over waves 11–20, two red and two blue rows from wave 13, two divers at once from wave 15 | keep the wave-12 caps and only retune | every value was capped by wave 12–15, so waves 13–19 played alike | [enemy waves](features/enemy-waves/design.md) |`
  - `| D-014 | 2026-11-17 | A lost controller pauses the run; after it reconnects the pause menu stays up and the player resumes | resume at once; 3-2-1 countdown | user's choice: the player resumes when ready | [ship controls](features/ship-controls/design.md), [co-op](features/co-op/design.md), [screens](features/screens/design.md) |`
  - `| D-015 | 2026-11-17 | make release builds without the dev aids (Triangle stress, Select autopilot), logs to 127.0.0.1 and includes the song | one build for everything | a player pressing Triangle would spawn 60 aliens; the user may share the song | [music and sound](features/music-and-sound/design.md) |`
- Run `python3 .claude/skills/game-docs/scripts/check_docs.py docs` and fix what it reports.
- Stop: list the changed files and a commit message (`M4: NAME game, unplug pause, difficulty to wave 20, release build`).

## Step 9: PS3 check: unplug and 30-minute soak (needs the user)
1. `make ps3` must build without warnings in the new code. Start `make log > m4.log` in the background in the
   scratchpad directory (never into `build/`), then `make deploy` (if it refuses, ask the user to quit to the XMB;
   if webMAN doesn't answer, ask them to restart the PS3 and re-enable HEN).
2. Ask the user to launch NAME game and check the main menu title and `V1.0`, then in a run:
   - Hold the PS button, choose **Turn Off Controller** (not Quit Game). Expect the pause menu with
     `CONNECT CONTROLLER 1`. Press the PS button to reconnect; the pause menu stays; Start resumes.
   - With two controllers: player 2 joins, turn off player 2's controller → `CONNECT CONTROLLER 2`; reconnect, resume.
   - Press Select (autopilot on), put the controller down and leave it for 30 minutes. Then Select off, Start →
     RETURN TO MENU.
3. From the log: `controller 0 disconnected` and `paused: controller 1 disconnected`, `controller 0 connected`, then
   `scene: playing` only after a Start press; `button 7` and `autopilot on`. If Select logs another number, fix
   `BTN_SELECT`; if it is 7, add 7 to the confirmed list in `CLAUDE.md`.
   If no `disconnected` line appears when the controller is turned off, stop and report the log to the user; don't guess a fix.
4. Soak pass: every stats line during the 30 minutes shows fps ≥ 59.5 and worst work < 16.7 ms; heap at minute 30 is
   at most 64 KB above heap at minute 5; no `watchdog` or `SAVE FAILED` lines. Note the wave reached.
   Heap that keeps rising: find which `std::vector` grows (log their sizes in the stats line) and fix the cause.

## Step 10: balancing playtest and tuning (needs the user and one other person)
1. Deploy, `make log` to a new file. The other person (new to the game) plays two runs alone; the user plays two runs
   alone; then one co-op run. Nobody uses the autopilot or Triangle. After each run ask the player: waves 1–3 calm?
   7–9 busy? 13+ fair? Bosses fair? Did you have a rocket when you needed one?
2. From the log per run: wave reached (`game over` line), waves with deaths, boss times (`boss down … after`), rockets
   at each `wave … start`.
3. Targets: newcomer's runs end at wave 5–9; the user reaches wave 13–22; each boss takes 20–60 s; most wave starts have
   1–5 rockets. Apply **the first matching row only**, one change per round, then replay. At most 3 rounds; whatever is
   still off after that goes into GDD Open questions.

   | Seen in the runs | Change |
   |---|---|
   | Newcomer ends before wave 5 in both runs | `shotChance`: base 0.02 → 0.015 |
   | Newcomer passes wave 9 | `shotChance`: base 0.02 → 0.025 |
   | User ends before wave 13 in both runs | `ENEMY_SHOT_MAX_SPEED` 90 → 84 |
   | User passes wave 22 | `TWO_DIVERS_WAVE` 15 → 13 |
   | A boss falls in under 20 s | `BOSS_HP_PER_APPEARANCE` 50 → 75 |
   | A boss takes over 60 s, or most deaths are at bosses | `BOSS_MIN_INTERVAL` 1.2 → 1.5 |
   | 0 rockets at most wave starts after wave 5 | `PICKUP_CHANCE` 0.02 → 0.03 |
   | 6+ rockets at most wave starts | `PICKUP_CHANCE` 0.02 → 0.015 |
4. Each change: the constant, the matching `design.md` number (drop its `[ASSUMPTION]` once a round confirms it), any
   test that checks the old value, and a History line "tuned in the M4 balancing pass: <old> → <new> (<reason>)".
   `make test` green before redeploying.
5. When the targets hold: tick the enemy-waves balancing criterion and "Waves 1–3 feel calm and 7–9 busy" with
   "PS3 playtest <date>: <runs and waves>", and the rocket criterion if the rocket rows held. Stop and list files.

## Step 11: release and the fresh-player test (needs the user and a player who hasn't installed it)
1. `make release`. Check `dist/` holds `NAME-game-1.0.pkg` (about 5 MB with the song) and the README.
2. Ask the user to give the pkg and README to the player, who installs it by following only the README (USB stick,
   Package Manager) and plays one full run without help. The user reports any step that confused them.
3. If Package Manager rejects `NAME-game-1.0.pkg`, change the `release` target to copy `build/nameGame.gnpdrm.pkg`
   (the finalized package the SDK also builds) to the same `dist/` name, rebuild, and retry.
4. Fix the README where the player got stuck, and rebuild the release so `dist/` holds the fixed copy.
   The release build logs nothing (127.0.0.1), so this step is proven by the user's report only.

## Step 12: final docs sync and work-plan update
- Testing docs: tick the unplug criterion (ship-controls) and add the soak result to GDD › Game-level acceptance as
  `- No frame drops or memory growth over 30 minutes. *(PS3 soak <date>: autopilot to wave N, fps ≥ X, worst work Y ms, heap A → B KB)*`.
- GDD: milestone 4 becomes `**M4, release polish (done <date>):** …`; change log 0.15 `M4 done: soak, balancing and fresh-player install confirmed on the PS3.`
- `docs/work-plan/make_work_plan.py` (read it now): `VERSION = "1.2"`; cover title "NAME game" and drop "working title"
  from the footer; section 1: say M1–M4 are done and set every milestone row to Done (`row_colors` all `DONE`);
  section 3 feature table: every M3 row `implemented`; difficulty table: replace the loop with
  ```python
  rows = [["Wave", "Aliens", "Sway px/s", "Shots per alien/s", "Shot px/s", "Dive every", "Rows red/blue/green", "Clear bonus"]]
  highlight = {}
  for n in range(1, 21):
      if n % 5 == 0:
          rows.append([str(n), "BOSS", "—", "—", "—", "—", "boss %d" % (n // 5), "—"])
          highlight[len(rows) - 1] = NOW
          continue
      alien_rows = min(2 + n // 2, 5)
      red = 2 if n >= 13 else 1 if n >= 7 else 0
      blue = (4 if n >= 13 else 2 if n >= 4 else 0) - red
      dive = ("%.1f s" % max(4 - 0.2 * n, 1.5) + (" ×2" if n >= 15 else "")) if n >= 3 else "—"
      rows.append([str(n), str(8 * alien_rows), str(min(20 + 2 * n, 50)), "%.3f" % min(0.02 + 0.005 * n, 0.08),
                   str(min(70 + 2 * max(n - 10, 0), 90)), dive, "%d/%d/%d" % (red, blue, alien_rows - red - blue),
                   "{:,}".format(1000 * n)])
  story.append(table(rows, [11 * mm, 14 * mm, 17 * mm, 24 * mm, 17 * mm, 22 * mm, FULL - 130 * mm, 25 * mm],
                     row_colors=highlight))
  ```
  (indented to match `build()`; use the step-10 tuned values if any changed) and update the formula note under it; section 6 roadmap: M3 and M4
  "(done)"; section 8 M4 table: a third column with each task's result; section 11: add D-010 to D-015 (from the GDD);
  section 12: the name question answered "NAME game (D-012)". Regenerate with the commands in the script's docstring
  and open a page or two of the PDF to check it.
- Run `check_docs.py` again; then stop and list files with a commit message (`close Milestone 4`).

## Done when
- `make test` passes, `make ps3` and `make release` build, `check_docs.py` is clean.
- On the PS3: an unplugged controller pauses the run until it is back, the 30-minute soak holds 60 fps with flat heap,
  the balancing targets hold, and a fresh player installs `NAME-game-1.0.pkg` from the README and plays a full run
  without help.
