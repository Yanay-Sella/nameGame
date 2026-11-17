# Plan: Milestone 3 (weapon upgrades, bosses, high score save, game feel)

For the implementer (Claude Sonnet). Every design choice below is decided, so don't redesign or re-research it:
write it into the docs, code it, test it. Numbers are starting values and go into `design.md` marked `[ASSUMPTION]`.
Follow `CLAUDE.md` and the existing code style: plain C++14, no new libraries, one-line comments, no helpers for
one-off logic, constants at the top of `game.cpp` grouped under the feature's design note.

## Token budget rules
- Don't read `docs/work-plan/make_work_plan.py` or the PDF: section 8 is summarised here. Don't reread a file you just
  edited. Read `source/game.cpp`, `source/game.h` and `tests/game_tests.cpp` once at the start; read other files only
  where a step names them.
- Copy the format of `docs/features/rocket/{design,implementation,testing}.md` for every new feature doc (same
  headers, tables and History lines). Invoke the `game-docs` skill once, before the first design note, and the
  `game-feel` skill once, before feature 4. Don't open their reference files unless the checker reports something unclear.
- Pipe long output: `make test 2>&1 | tail -15`, `make deploy 2>&1 | tail -8`.
- Use `Edit` for code changes; never rewrite whole files.

## Step 0: before starting
1. `git status --short` and `git log --oneline -3`. Expect `b663393` (or later screens commits) and a clean tree.
2. Another session is building the screens in this same folder, on `main`. So:
   - **Off limits:** the menu code in `source/main.cpp` (the `SCENE_MAIN_MENU`, `SCENE_SETTINGS`, `SCENE_PAUSED` cases and
     the menu drawing), `drawMenu` in `screen.cpp`, `input.cpp`, and `docs/features/screens/`.
   - Before every edit to `main.cpp` or `settings.*`, run `git status --short`. If either file has changes you didn't
     make, stop and ask the user.
   - Commit only your own files, by path (`git add <paths>`), never `git add -A` or `git commit -a`.
   - `make deploy` installs whatever is in the folder, including the other session's unfinished edits. If the build
     breaks in code you didn't touch, stop and tell the user.

## The loop for every feature (the user's process)
1. **Design note:** write `docs/features/<slug>/design.md` (status `designed`), add or update the GDD index row with
   a `[design](features/<slug>/design.md)` link, add a change-log line, bump the GDD version (0.8 → 0.9 → …).
2. **Code** the feature as specified below.
3. **Tests:** add the listed tests to `tests/game_tests.cpp` and call them from `main()`. `make test` must print
   `all tests passed` with no warnings.
4. **Prove each new test can fail:** break the rules in one go (e.g. change the constants or comment out the line each
   test guards), run `make test`, check that every new test's lines print `FAIL`, then undo the breakage exactly
   and rerun to green. Note in the reply which mutation broke which test.
5. **Mac render:** draw the scenario frames listed for the feature with the render tool below and look at them
   (`Read` the PNG). Fix anything unreadable before deploying.
6. **PS3:** start `make log` in the background writing to the scratchpad (never into `build/`), then `make deploy`.
   If webMAN doesn't answer, ask the user to restart the PS3 and re-enable HEN. Ask the user to launch nameGame and run
   the feature's playtest checks; wait for their report, then read the log.
7. **Docs sync:** write `implementation.md` and `testing.md`; set the status to `implemented` in `design.md` and the
   GDD index. Tick testing criteria only with proof (`tests/game_tests.cpp` › testName, or "PS3 log <date>: …").
   Unproven items stay `[ ]` with a reason. Then run
   `python3 .claude/skills/game-docs/scripts/check_docs.py docs` and fix every error. `feature-doc-missing` for
   other features is fine.
8. **Stop:** list the changed files and a one-line commit message in the style of `git log`, and ask the user whether
   to commit. Commit only on a yes, with the `Co-Authored-By` line, then start the next feature.

### Mac render tool (scratchpad, not committed)
Write once to `<scratchpad>/render.cpp` and change only the scenario per feature:
```cpp
#include <stdio.h>
#include "game.h"
#include "screen.h"
static void save(const char* path) {   // binary PPM
    FILE* f = fopen(path, "wb");
    fprintf(f, "P6 %d %d 255\n", SCREEN_W, SCREEN_H);
    for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
        unsigned char rgb[3] = {(unsigned char)(screenPixels[i] >> 16), (unsigned char)(screenPixels[i] >> 8), (unsigned char)screenPixels[i]};
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
}
int main() {
    Game game; Actions fire = {}; fire.fire = true;
    startGame(game, 1, 7);
    game.players[0].invincibleTimer = 1e9f;
    /* scenario: set state, call updateGame(game, &fire, 1.0f / 60) a few times */
    drawGame(game); save("frame1.ppm");
}
```
Build and view (run from the scratchpad):
```bash
c++ -std=c++14 -I/Users/yanaysella/git/nameGame/source render.cpp /Users/yanaysella/git/nameGame/source/game.cpp /Users/yanaysella/git/nameGame/source/screen.cpp -o render && ./render
python3 -c "import sys;from PIL import Image;[Image.open(p).resize((960,540),Image.NEAREST).save(p[:-4]+'.png') for p in sys.argv[1:]]" frame*.ppm
```

---

## Feature 1: Weapon upgrades (slug `weapon-upgrades`)
Work plan: "4 levels, upgrade pickups, lose a level on death; done when each level is visible and distinct."

**Rules**
1. Each player has a weapon level 1–4, starting at 1 (also when joining in co-op).
2. Level 1 single: one bullet from the nose. Level 2 double: two straight bullets 3 px left and right of the centre.
   Level 3 spread: a centre bullet plus two angled ones (vx −70 and +70 px/s). Level 4 fast spread: level 3 fired at
   the faster interval. All bullets: vy −320 px/s, damage 1.
3. A killed alien drops an upgrade pickup with 3 % chance (one roll per kill: below 2 % → rocket pickup, below 5 %
   → upgrade pickup, so they never drop together). It falls like the rocket pickup (30 px/s).
4. Touching an upgrade pickup raises the level by 1; at level 4 it gives 1,000 points instead.
5. Dying loses one level, never below 1.
6. The level is shown by the bullets themselves; no HUD element.

**Numbers:** fire interval levels 1–3: 0.12 s (existing), level 4: 0.08 s; side offset 3 px; spread vx 70 px/s;
upgrade drop 3 % per kill; points at max level 1,000. Edge cases: pickup collected while dead (can't, the ship isn't
there); two players touch the same pickup on one step (the first in the player array gets it, like rocket pickups).

**Code** (`game.h`, `game.cpp`, `sprites.h`, `main.cpp`)
- `struct Shot`: add `float vx;` **after** `owner` so the existing 4-value initialisers keep vx = 0. Move bullets
  and enemy shots by `vx` too (enemy shots need it for the bosses).
- `struct Player`: `int weaponLevel;` set to 1 in `newPlayer`. `killPlayer`: `weaponLevel = max(1, weaponLevel - 1)`.
- `struct Pickup`: add `PickupType type` (`enum PickupType { PICKUP_ROCKET, PICKUP_UPGRADE };`) as the third member;
  existing `{x, y}` initialisers become `{x, y, PICKUP_ROCKET}`.
- `updatePlayer` fire block: interval `weaponLevel == 4 ? FAST_FIRE_INTERVAL : FIRE_INTERVAL`; push 1, 2 or 3 bullets
  by level (one sound request per volley).
- `damageAlien`: the single roll above. Pickup collection in `updateGame` branches on `type`.
- `sprites.h`: `UPGRADE_PICKUP` 8×8, pale yellow `y` and orange `o` (the bullet colours), an up-arrow or chevron so it
  reads differently from the purple rocket pickup. No `m`. Add it to both sprite arrays in the tests. Rerun
  `python3 docs/xmb-art/make_xmb_art.py` only if an existing sprite changed (it shouldn't).
- `main.cpp`: append `, weapon %d` with `game.players[0].weaponLevel` to the 5 s fps log line (outside the menu code).

**Tests:** `weaponLevelsFireTheirPattern` (levels 1–3: 1, 2, 3 bullets on the first press step, level 3 vx −70/0/70;
level 4: 3 bullets every 0.08 s over 1 s), `upgradePickupRaisesLevelUpToFour` (and +1,000 at level 4),
`deathLosesOneWeaponLevel` (3 → 2, 1 → 1), `rocketPickupStillAddsARocket` only if the existing pickup test breaks
on the new field.

**Render:** levels 1–4 each after 0.5 s of held fire (4 frames), plus one frame with both pickup types on screen.

**PS3 checks:** pick up upgrades to level 4 and see each pattern; die and see the level drop; log shows `weapon n`.

---

## Feature 2: Bosses (slug `bosses`)
Work plan: "one boss design scaled per appearance; health bar; attack patterns; big reward; waves 5, 10, 15 beatable
but threatening." GDD D-002: every 5th wave.

**Rules**
1. Waves 5, 10, 15… are a boss alone (no formation). The banner reads `BOSS` instead of `WAVE n`.
2. Appearance n = wave / 5. HP = 100 + 50·(n−1). The boss flies in from above to y = 20, then sways the full screen
   width at 40 + 10·(n−1) px/s, capped at 80, turning at the edges.
3. Attacks, alternating, every max(2.5 − 0.3·(n−1), 1.2) s: an aimed burst of 3 shots 0.15 s apart at the nearest
   living ship (speed 90 px/s), then a fan of 5 shots straight down spread −60…+60 px/s sideways (vy 70 px/s).
   It doesn't shoot while entering or when no ship is alive. Its shots are ordinary magenta enemy shots.
4. Bullets do 1 damage, a rocket blast 20 (the rocket explodes on touching it; the blast hits it when the circle
   overlaps its rectangle). Each hit flashes it white for 0.06 s (existing `HIT_FLASH`).
5. Touching the boss's body kills the ship.
6. A health bar 120×3 px, centred at y = 16 (under the `HI` line feature 3 adds at y = 9): red `r` fill on dark red `R`.
7. Killing it: 5,000·n points to the killer, a large explosion, a guaranteed upgrade pickup and a rocket pickup;
   then the normal wave-clear bonus and pause before the next wave.

**Code** (`game.h`, `game.cpp`, `sprites.h`, `main.cpp`)
- New `struct Boss { float x, y, vx, attackTimer, burstTimer; int hp, maxHp, burstShotsLeft, nextAttack; float flash; bool alive; };`
  and `Boss boss;` in `Game`. Separate from `Alien` on purpose: the 12×10 alien size is used in every collision.
- `int bossHealth(int wave)` and `bool isBossWave(int wave)` in `game.h` next to `formationRows` (tested, and match the docs).
- `startWave`: if `isBossWave(wave)`, set up the boss above the screen and skip the formation.
- `updateGame`: move and shoot the boss; bullets/rockets/blasts damage it (extend the existing loops, keep
  `damageAlien` for aliens and write the boss's death inline once); body contact in the player-hit block;
  `anyAlienAlive` becomes "any alien or the boss alive". Dives are skipped on boss waves (no aliens anyway).
- `drawGame`: boss sprite (flash with `drawSpriteSolid`), health bar while alive, `BOSS` banner.
- `sprites.h`: `BOSS_A`, `BOSS_B` 32×20, two animation frames, our own alien design in the alien colours (`g G u U r R w`),
  no `m`. Add both to the sprite tests.
- `main.cpp`: log `boss wave %d, hp %d` when a boss wave starts and `boss down at wave %d` when it dies (compare
  `game.boss.alive` before and after the update steps, next to the existing `wasOver` check).

**Tests:** `everyFifthWaveIsABoss` (clear wave 4, run past the pause: boss alive, no aliens, wave 6 has aliens),
`bossHealthGrowsEachAppearance` (100, 150, 200 for waves 5, 10, 15), `bulletsAndBlastsDamageTheBoss` (−1, −20),
`killingTheBossScoresAndDropsBothPickups`, `bossBodyKillsTheShip`, `bossFiresBurstThenFan` (3 shots, then 5),
`bossWaitsForALivingShip` (no shots with every ship dead).

**Render:** boss entering, boss mid-fight at half health with a fan in the air, boss flashing on a hit, boss blast.

**PS3 checks:** reach wave 5 (or temporarily start at wave 4 via a one-line change in `startGame`, reverted
before the commit, and say so to the user), beat the boss, confirm the burst and fan read clearly and 60 fps holds.

---

## Feature 3: High score save (slug `high-score`)
Work plan: "a small file on the PS3 hard drive, written on game over; survives quitting and a PS3 reboot."

**Rules**
1. The high score is the best single-player score of any finished run (in co-op, the better of the two).
2. It is loaded at boot and written at game over, only when beaten. Runs left through RETURN TO MENU don't count
   (that path is screen code; note it as an edge case).
3. The HUD shows `HI 0012345` centred at y = 9, under `WAVE n`. The game-over panel adds `NEW HIGH SCORE` in yellow
   when the run beat it.
4. Main-menu display is the screens feature's job: don't add it. Say in `design.md` that the title shows it once the
   screens session adds it, and tell the user in the handoff.
5. Missing or unreadable file: high score 0. Save failure: keep it in memory and log `SAVE FAILED`.

**Code**
- `settings.h/.cpp`: `int highScoreLoad(const char* path);` and `bool highScoreSave(const char* path, int score);`,
  stdio only, file `highscore.txt` with one line `best=<n>`. Own file, not settings.txt, so the screens session's
  settings format stays theirs. Clamp loaded values to 0…`MAX_SCORE`.
- `Game`: `int highScore; bool newHighScore;` (draw-only, set by `main.cpp`).
- `main.cpp` (not the menu cases): load next to `settingsLoad` and log `high score: %d`. In the existing
  `if (game.over && !wasOver)` block: take the best player score; if it beats the stored one, save it, set
  `game.newHighScore`, and log `high score: %d (saved|SAVE FAILED)`. Right before `stage = "draw"`, copy the
  stored value into `game.highScore` (so `startGame` resetting `Game` doesn't matter).

**Tests:** `highScoreDefaultsToZero` (missing file and a garbage file), `highScoreSurvivesASave` (write to
`build/tests/`), `highScoreIsClamped` (negative and huge values).

**Render:** HUD with `HI`, game-over panel with and without `NEW HIGH SCORE` (single player and co-op).

**PS3 checks:** finish a run with a new best (log `saved`); quit and relaunch (log shows the value at boot); ask the
user to reboot the PS3, re-enable HEN and relaunch (log shows it again). That proves the M3 "survives a reboot"
criterion. After the next `make deploy` (feature 4), confirm the log still shows it: reinstalling must not wipe it.

---

## Feature 4: Game feel (slug `game-feel`)
Invoke the `game-feel` skill first. GDD direction and work plan tiers apply; options: shake 0–100 %, reduce flashing,
large flashes at most 3 per second. Score pop-ups (work plan) are out of scope unless the user asks.

**Rules and numbers** (event → trauma / hit-stop / particles / screen flash)
| Event | Tier | Trauma | Hit-stop | Particles | Screen flash |
|---|---|---|---|---|---|
| bullet hits alien or boss, no kill | small | 0 | 0 | existing sprite flash | no |
| alien dies | medium | 0.12 | 0 | 8 (existing) | no |
| pickup collected | small | 0 | 0 | 4 | no |
| extra life | medium | 0.2 | 0 | 0 | no |
| rocket explosion | large | 0.6 | 0.06 s | 20 | 0.05 s |
| boss dies | large | 1.0 | 0.15 s | 40 | 0.08 s |
| player dies | large | 0.8 | 0.12 s | existing | 0.08 s |
- No hit-stop on ordinary kills, a deliberate deviation from the medium tier: spread fire kills several aliens per
  second and would stutter. Log it as decision D-011 in the GDD.
- Shooting gets no feedback (GDD). Trauma adds and caps at 1, decays 1.5 per second of real time (also during
  hit-stop). Shake offset = trauma² · strength · (4 px x, 3 px y) · smooth sines (two unrelated rates), never random
  per frame.
- Hit-stop freezes the game step; overlapping stops take the longest remaining, not the sum. R2 presses during a
  stop are still recorded (game time doesn't move, so the 0.15 s buffer still holds). Music and menus never stop.
- Shake moves only the picture of the play field: the HUD stays still, collisions never move.
- Screen flash: the play field drawn halfway to white for its duration. At most one every 0.33 s (later ones are
  dropped). Reduce flashing turns screen flashes off; the small sprite hit flashes stay.
- Options: shake 0, 25, 50, 75, 100 % (default 100) and flashing NORMAL / REDUCED (default NORMAL), saved in
  settings.txt as `shake=` and `flash=` lines.

**Code**
- `Game`: `float trauma, hitStop, flashTimer, lastFlashTime, feelTime;` plus draw-only `float shakeStrength; bool reduceFlashing;`
  copied from `Settings` by `main.cpp` each frame next to `highScore`.
- One `static void feedback(Game&, float trauma, float hitStop, float flash)` in `game.cpp`, called at each event
  above (it is a real reuse point, 4+ call sites).
- `updateGame` top: advance `feelTime`, decay `trauma`; if `hitStop > 0`, count it down, record R2 presses, return.
- `drawGame`: after the play field and before the HUD, apply the flash (blend each pixel 50 % to white) and the shake
  (shift `screenPixels` by the integer offset, fill the uncovered edge with `paletteColor('k')`).
- `settings.h/.cpp`: `int shake = 100; bool reduceFlashing = false;` with load/save lines. This touches the screens
  session's file: check `git status` first (step 0).
- **Settings menu items** (`SHAKE: 100%`, `FLASHING: NORMAL`) live in `main.cpp`'s `SCENE_SETTINGS` case, which is
  off limits. Before touching it, ask the user: "Is the screens session finished, so I can add the shake and flash
  items to the settings menu?" If not, ship the feature with the settings saved and read from settings.txt, and
  leave the menu items to the screens session (say so in `implementation.md` under Deviations).
- Performance: the shift and the blend are one pass each over 57,600 pixels; check that `worst work` in the log stays
  under 8 ms during the stress test (Triangle).

**Tests:** `overlappingHitStopsTakeTheLongest` (two explosions 2 steps apart: frozen for the longer one only, then
updates resume), `traumaDecaysToZero` (1.0 → 0 within 1/1.5 s), `inputDuringHitStopStillFires` (R2 pressed during
a stop launches afterwards), `flashesAreRateLimited` (3 large events within 0.2 s give one flash),
`reducedFlashingDrawsNoFlash` (draw a frame with a flash: no pixel changed vs. one without), `zeroShakeDrawsNoOffset`,
`shakeAndFlashSettingsSurviveASave`.

**Render:** the same frame with trauma 1 at shake 100 % and 0 %, a flash frame, a flash frame with reduce flashing.

**PS3 checks:** rocket blasts, deaths and the boss death feel punchy, not nauseating; holding R1 doesn't buzz;
input still works through freezes; stress test fps; options (if the menu items were added) change the feel and
survive a relaunch.

---

## After feature 4
Don't mark M3 done in the GDD: screens are still open in the other session, and the playtest/tuning task (work plan
M3 #7) comes after. Hand off with: what's implemented, PS3 checks still unproven, and every `[ASSUMPTION]` the user
should confirm in play.
