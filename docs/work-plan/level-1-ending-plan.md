# Plan: level 1 ending (wave-20 boss ends the run)

For the implementer: follow the steps in order. Every design choice below is final, so don't redesign anything.
Read only the files each step names, and don't re-read a file after editing it. Code style: follow `CLAUDE.md` and
the existing code (plain C++14, no new libraries, short one-line comments, no helpers for one-off logic).
Commit straight to `main` when done, with this plan file in the same commit.

## What the player gets
```
CLASSIC → "LEVEL 1" banner 2 s → "WAVE 1" banner 1.5 s → waves 1..19 as today → wave 20 boss
   wave-20 boss dies → reward, explosion, pickups, wave-clear bonus as today → "LEVEL 1 CLEAR" banner 3 s
      → end screen: panel titled "LEVEL 1 CLEAR", score and wave, NEW HIGH SCORE if beaten, X PLAY AGAIN / O MENU
```
- **Start of a run:** the wave banner (yellow, scale 2, y = 84) reads `LEVEL 1` for the first 2 s of wave 1, then
  `WAVE 1` for the usual 1.5 s. Every other wave's banner is unchanged (`WAVE n`, `BOSS`).
- **Level boss:** the wave-20 boss is the normal boss (250 HP, reward 20,000). Killing it (bullet or rocket) gives the
  normal reward, explosion, both pickups and the wave-clear bonus (20 × 1,000 to each player with lives left), then the
  `LEVEL 1 CLEAR` banner (yellow, scale 2, y = 84) shows for 3 s of game time with `SOUND_WAVE_START`. Wave 21 never starts.
- **During the clear banner:** play goes on: ships move and shoot, pickups fall and can be collected, enemy shots already in
  the air keep flying and can still hit. A player who loses a life but has lives left still gets the win. If every player
  has lost their last life, the run ends as a normal `GAME OVER` (no level clear). Player 2 can't join once the level is clear.
- **End screen:** the game-over panel, with the title `LEVEL 1 CLEAR` in yellow instead of `GAME OVER` in red. Everything else
  in the panel is as today (score and `WAVE 20` line, `NEW HIGH SCORE`, `X PLAY AGAIN   O MENU` after 1.5 s). The music stops,
  and the high score is saved if beaten, exactly as at game over (main.cpp's existing `game.over && !wasOver` block).

## Step 1: game state (`source/game.h`)
Add to `struct Game`, after `float overTime;`:
```cpp
    bool levelClear;       // the level boss is dead: the "LEVEL 1 CLEAR" banner runs, then the run ends
    float clearTimer;      // s left of the "LEVEL 1 CLEAR" banner
    bool won;              // the run ended on the level clear, not on the last life
```
`startGame` does `game = Game()`, which zeroes these; no other reset needed.

## Step 2: rules and drawing (`source/game.cpp`)
- New constant block after the `// Co-op` block:
  ```cpp
  // Level 1 (level-1/design.md)
  const int LEVEL_LAST_WAVE = 20;        // the level boss's wave; killing it ends the run
  const float LEVEL_BANNER_TIME = 2;     // s of "LEVEL 1" before "WAVE 1"
  const float LEVEL_CLEAR_TIME = 3;      // s of "LEVEL 1 CLEAR" before the end screen
  ```
- `startWave`: replace `game.bannerTimer = BANNER_TIME;` with
  `game.bannerTimer = BANNER_TIME + (wave == 1 ? LEVEL_BANNER_TIME : 0);   // "LEVEL 1" first, then "WAVE 1"`.
- `updateGame`, the block after `if (game.over) return;` (comment "The next wave waits for a living ship"): replace the
  `if / else if` on `anyAlienAlive` with this (keep the `anyAlienAlive` line above it, and the `runOver` block below it unchanged):
  ```cpp
  if (!anyAlienAlive && game.nextWaveTimer == 0 && !game.levelClear) {
      for (Player& player : game.players)
          if (player.lives > 0) addScore(game, player, WAVE_CLEAR_BONUS * game.wave);
      if (game.wave == LEVEL_LAST_WAVE) {   // the level boss is dead (level-1 rule 4)
          game.levelClear = true;
          game.clearTimer = LEVEL_CLEAR_TIME;
          game.soundRequested[SOUND_WAVE_START] = true;
      } else {
          game.nextWaveTimer = WAVE_PAUSE;
      }
  } else if (game.levelClear) {
      // The banner's end is a win only while someone has lives left; otherwise the game over below ends the run.
      game.clearTimer = std::max(0.0f, game.clearTimer - dt);
      bool livesLeft = std::any_of(game.players.begin(), game.players.end(), [](const Player& p) { return p.lives > 0; });
      if (game.clearTimer == 0 && livesLeft) {
          game.won = true;
          game.over = true;
          game.overTime = game.time;
          return;
      }
  } else if (!anyAlienAlive && anyPlayerAlive) {
      game.nextWaveTimer -= dt;
      if (game.nextWaveTimer <= 0) startWave(game, game.wave + 1);
  }
  ```
  The bonus loop moves inside the first branch unchanged; don't add any enemy-shot clearing.
- `drawGame`:
  - `2P PRESS START` line: condition becomes `!coOp && !game.over && !game.levelClear`.
  - Game-over panel: the title line becomes
    `drawCentredText(66, game.won ? "LEVEL 1 CLEAR" : "GAME OVER", paletteColor(game.won ? 'y' : 'r'), 2);`
    (102 px wide at scale 2, fits the 142 px panel). Leave the rest of the panel unchanged.
  - Banners: between the `if (game.over) { ... }` panel and the `else if (game.bannerTimer > 0)` branch, insert
    `else if (game.levelClear) drawCentredText(84, "LEVEL 1 CLEAR", paletteColor('y'), 2);`
    (`levelClear && !over` is exactly the 3 s banner, plus up to 1.5 s more on the losing path; that is fine).
  - In the `bannerTimer` branch, the text becomes `LEVEL 1` while `game.wave == 1 && game.bannerTimer > BANNER_TIME`,
    else the existing `BOSS` / `WAVE %d` choice:
    ```cpp
    const char* banner = game.wave == 1 && game.bannerTimer > BANNER_TIME ? "LEVEL 1" : isBossWave(game.wave) ? "BOSS" : "WAVE %d";
    snprintf(text, sizeof text, banner, game.wave);
    ```

## Step 3: wiring and logs (`source/main.cpp`)
- `SCENE_PLAYING` join condition: add `&& !game.levelClear` to
  `if (actions[p].confirm && p == (int)game.players.size() && !game.over)`.
- Before the fixed-step loop, next to `bool bossWasAlive = game.boss.alive;`, add `bool clearBefore = game.levelClear;`.
  After the `boss down at wave` log line, add:
  ```cpp
  if (game.levelClear && !clearBefore) debugNetPrintf(INFO, "level 1 clear at %.1f s\n", game.time);
  ```
- In the `if (game.over && !wasOver)` block, the final per-player log becomes:
  ```cpp
  debugNetPrintf(INFO, "%s at %.1f s: player %d score %d, wave %d\n", game.won ? "level clear" : "game over", game.overTime,
                 (int)p + 1, game.players[p].score, game.wave);
  ```
  Leave the music stop and high-score save in that block as they are: they now also run on a win.
- Nothing else in main.cpp changes (pause, autopilot, game-over input all work on `game.over` as before).

## Step 4: tests (`tests/game_tests.cpp`)
Add three functions after `bossWaitsForALivingShip` and call them from `main()` after `bossWaitsForALivingShip();`.
Use the existing helpers `startSafeGame`, `startBossGame(game, wave)`, `run`, `NO_INPUT`, `DT`.
- `levelBannerComesBeforeWaveOne`:
  `startSafeGame`; `CHECK(fabsf(game.bannerTimer - 3.5f) < 1e-4f)`. `run(game, NO_INPUT, 2.1f)`;
  `CHECK(game.wave == 1 && game.bannerTimer > 0 && game.bannerTimer < 1.5f)`. Kill every alien, `run(game, NO_INPUT, 2.2f)`;
  `CHECK(game.wave == 2 && game.bannerTimer <= 1.5f)`.
- `killingTheLevelBossClearsTheLevel`:
  `startBossGame(game, 20)`; `CHECK(game.boss.hp == 250)`; set `game.boss.hp = 1`; push an enemy shot
  `{10, 40, 0, -1, 0}` (parked away from the ship) and a bullet `{game.boss.x + 10, game.boss.y + 5, 0, 0, 0}`;
  remember `before = game.players[0].score`; one `updateGame(game, &NO_INPUT, DT)`.
  `CHECK(!game.boss.alive && game.levelClear && game.clearTimer == 3 && !game.over)`;
  `CHECK(game.players[0].score == before + 20000 + 20 * 1000)` (comment: 5,000 x 4 and the wave 20 clear bonus);
  `CHECK(game.enemyShots.size() == 1)` (comment: shots in the air stay).
  `run(game, NO_INPUT, 2.9f)` → `CHECK(!game.over && game.wave == 20)` (the boss death's 0.15 s hit-stop comes first).
  `run(game, NO_INPUT, 0.5f)` → `CHECK(game.over && game.won && game.wave == 20)`.
  `run(game, NO_INPUT, 3)` → `CHECK(game.wave == 20 && !game.boss.alive && game.aliens.empty())`.
- `deathDuringTheClearOnlyLosesOnTheLastLife`: loop `for (int lives : {1, 2})`:
  `startBossGame(game, 20)`; `game.players[0].lives = lives`; kill the boss as above (hp 1 + bullet, one step);
  `CHECK(game.levelClear)`; `run(game, NO_INPUT, 0.2f)` (past the boss death's hit-stop, which skips collisions);
  then `Player& p = game.players[0]; p.invincibleTimer = 0;` push the enemy shot
  `{p.x + 7, p.y + 5, 0, -1, 0}` (parked on the hitbox, as in `threeDeathsEndTheRun`); one step;
  `CHECK(!game.players[0].alive && game.players[0].lives == lives - 1)`; `run(game, NO_INPUT, 4)`;
  `CHECK(game.over && game.won == (lives == 2))`.
- `make test`: everything passes, including the existing tests (none of them reaches a wave-20 boss kill).

## Step 5: docs
Copy the shape of `docs/features/bosses/implementation.md` and `testing.md`.
- `docs/features/level-1/design.md`: status `implemented`; remove `[ASSUMPTION]` from rules 3, 4 and 5; add to rule 4:
  "Enemy shots already in the air keep flying during the banner." Replace the edge case "The last life is lost on the level
  boss: game over as today, no level clear." with "The last life is lost on the level boss, or during the `LEVEL 1 CLEAR`
  banner: game over as today, no level clear. A life lost during the banner with lives left still wins." Add edge case
  "Player 2 can't join once the level is clear." Add to the end screen (rule 5): "The end screen is the game-over panel
  titled `LEVEL 1 CLEAR` in yellow." History: `- 2026-11-17: implemented (D-017).`
- New `docs/features/level-1/implementation.md`: approach (one paragraph: `startWave` lengthens the wave-1 banner;
  `updateGame` turns the wave-20 clear into `levelClear` + `clearTimer` instead of the next-wave pause, then sets `won` and
  `over`; `drawGame` draws the banners and the panel title; main.cpp's game-over block saves the high score and stops the
  music for both endings). Code map: `game.h` › `Game::levelClear, clearTimer, won`; `game.cpp` › `startWave`,
  `updateGame`, `drawGame`; `main.cpp` › join guard and the `level 1 clear at` / `level clear at` logs. Config constants:
  `LEVEL_LAST_WAVE`, `LEVEL_BANNER_TIME`, `LEVEL_CLEAR_TIME`. Deviations: none.
- New `docs/features/level-1/testing.md`: criteria, ticked with the test name for the three Mac tests; unchecked for the PS3
  checks in step 6 (LEVEL 1 banner on screen, autopilot run ends at wave 20 with `level 1 clear` then `level clear` 3.0 s
  later, end-screen panel, high score saved, X starts a new run).
- `docs/features/bosses/design.md` rule 7: append "(the wave-20 boss ends level 1 instead: [Level 1](../features/level-1/design.md))".
- `docs/features/co-op/design.md`: rule 1 "While it's running and not paused" → "While it's running, not paused and the level
  isn't clear"; rule 4 "The run ends when every player is out." → "The run ends when every player is out or the level is clear."
- `docs/features/high-score/design.md` rule 2: "written at game over" → "written when a run ends (game over or level clear)".
- `docs/features/music-and-sound/design.md` rule 1: "stops at game over" → "stops at game over or the level-clear end screen".
- `docs/features/screens/design.md` rule 7: "At game over, X or Start" → "At game over and on the level-clear end screen, X or Start".
- `docs/GDD.md`:
  - Version `0.18`.
  - Summary: "The run ends when you lose your last life, and you chase the saved high score." → "The run is level 1, waves 1
    to 20: it ends when you beat the wave-20 boss or lose your last life, and you chase the saved high score."
  - Session row: "one run is about 5–15 min; play until game over" → "one run is about 15–20 min: level 1 clear or game over".
  - Feature index › Level 1: status `implemented`, scope `MVP`.
  - Screens and flow: the last diagram line becomes `└─ game over / LEVEL 1 CLEAR: X/Start = play again, Circle = MAIN MENU`;
    the **Game over** bullet becomes "**Game over / level clear:** final score and wave; the panel reads GAME OVER, or LEVEL 1
    CLEAR after the wave-20 boss; X or Start plays again, Circle goes to the main menu."
  - Decision row: `| D-017 | 2026-11-17 | A run is level 1: killing the wave-20 boss shows LEVEL 1 CLEAR for 3 s, then the end
    screen, and the high score is saved; enemy shots stay, so losing the last life during the banner is still game over;
    replaces D-002's endless run | endless waves; clear enemy shots at the kill; lock the win at the kill | user's choice: a
    level structure for later worlds; the finale stays dangerous to the end | [level 1](../features/level-1/design.md),
    [bosses](../features/bosses/design.md), [screens](../features/screens/design.md), [high score](../features/high-score/design.md) |`
  - Change log: `**0.18 (2026-11-17):** level 1 ending implemented: the wave-20 boss ends the run with LEVEL 1 CLEAR (D-017).`
- Don't edit older decision rows, `docs/work-plan/make_work_plan.py` or the PDF.
- Run `python3 .claude/skills/game-docs/scripts/check_docs.py docs` and fix what it reports.

## Step 6: PS3 check (Claude drives it remotely; the user only re-enables HEN if the PS3 doesn't answer)
The autopilot run will overwrite the saved high score on the PS3; the user agreed to that.
1. `make ps3` builds without warnings in the changed code.
2. If `lsof -nP -iUDP:18195` shows a stale python, kill it. Start `make log` in the background, redirected to a file in your
   scratchpad directory (not `build/`). `make quit` if a game runs, then `make deploy`, then `make launch`; wait for
   `nameGame started` in the log.
3. LEVEL 1 banner: delete the old shot (`curl -s -m 10 -o /dev/null -Q "DELE /dev_hdd0/game/NMGM00001/USRDIR/shot.bmp" ftp://$PS3_IP/`),
   then `make pad BUTTONS='cross|1|l1'` (CLASSIC is the selected item at boot), then fetch it:
   `curl -s -m 10 -o shots/shot.bmp ftp://$PS3_IP/dev_hdd0/game/NMGM00001/USRDIR/shot.bmp && sips -s format png shots/shot.bmp --out shots/shot.png`.
   Read `shots/shot.png`: it shows `LEVEL 1`. The log shows `scene: playing`.
4. `make pad BUTTONS='select'` → log `autopilot on`. Wait (use Monitor on the log file, until `level clear at` or
   `game over at` appears; about 8–10 min): the log shows `boss wave 20, hp 250`, `boss down at wave 20`,
   `level 1 clear at T s`, then `level clear at T' s: player 1 score N, wave 20` with T' − T ≈ 3.0 (± 0.1) and
   `high score: N (saved)`.
   No `wave 21 start` line may appear.
5. `make shot` and read it: the panel title is `LEVEL 1 CLEAR` in yellow, with `SCORE N   WAVE 20`, `NEW HIGH SCORE` and
   `X PLAY AGAIN   O MENU`.
6. `make pad BUTTONS='cross'` → log `new run (music restart ...)` and `wave 1 start`. Then `make quit`.
7. If a line is missing: no `level 1 clear at` after `boss down at wave 20` → check the step 2 branch order in `updateGame`;
   `wave 21 start` → the `LEVEL_LAST_WAVE` check isn't reached; no `high score` line while the score beat the old best →
   `game.over` wasn't set on the win. Fix, `make test`, redeploy, repeat from 2.
8. Tick the testing.md criteria the log and screenshots prove, with "PS3 log 2026-11-17" / "PS3 screenshot 2026-11-17" notes.

## Done when
- `make test` passes, `make ps3` builds, `check_docs.py` is clean.
- On the PS3: a run starts with `LEVEL 1`, and the autopilot run ends at the wave-20 boss with `LEVEL 1 CLEAR`, the end-screen
  panel and the high score saved.
