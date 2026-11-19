# Plan: beam build-up, shards and strong-attack shake

For the implementer: follow the steps in order. Every design choice below is final, so don't redesign anything.
Read only the files each step names, and don't re-read a file after editing it. Code style: follow `CLAUDE.md` and
the existing code (plain C++14, no new libraries, short one-line comments, no helpers beyond the three named here).
Don't commit: when done, list the changed files and a suggested commit message (include this plan file).

## What the player gets
- **Magic beam (space buff) and mage beam:** the charging ball works as today. When it fires, the ball stays at the
  beam's base (pulsing between radius 5 and 6) and the beam shoots out of it, growing to full length in 0.1 s with
  a 2 px white cap on its moving tip. The player's beam grows up to the top of the screen; the mage's grows down to the bottom.
- While a beam is on, the odd spark (about 25 a second) flies sideways off its edges: 1 px, white or the beam's colour, short-lived.
- When a beam ends on its own, it breaks into small 1×2 "beam shards" along its whole length that drift outward,
  fall slowly and vanish within 0.6 s. (A beam cut short by the frost wave, the mage's death or the player's death just vanishes.)
- **Fire lord's eruption:** the same 0.1 s build-up, rising from the bottom of the screen to the top, with the same shake.
  No ball at its base, no sparks, no shards.
- **Shake:** every beam and eruption kicks the screen when it fires (trauma 0.7), then rumbles lightly (at least trauma 0.45,
  about 1 px) for as long as it's drawn. The frost wave and the bubble shield popping each shake like a rocket blast
  (trauma 0.6; no hit-stop). The rocket already shakes on its blast: unchanged. Today's 0.3 trauma rounds to a 0 px offset, which is why nothing visibly shook.
- **Hits follow the drawing:** the player's beam now damages its column once, at the moment it reaches the top
  (0.1 s after firing) instead of the moment it fires. The mage beam and the eruption kill only where they've grown to.
  The mage beam no longer counts above its ball (no ship can be there anyway).

## Step 1: particle height (`source/game.h`, `source/game.cpp`)
- `game.h` › `struct Particle`: add a last field: `struct Particle { float x, y, vx, vy, life; uint32_t color; int h; };   // h: px tall, 1 or 2 (beam shard)`
- `game.cpp` › `spawnParticles`: the initializer becomes `{x, y, cosf(angle) * s, sinf(angle) * s, 0.3f + randomUnit(game) * 0.2f, color, 1}`.
- `game.cpp` › `updatePlayer`, the deep-sea trail `game.particles.push_back({...})`: add `, 1` as the last value.
  (`-Wextra` warns on a missing field, so every initializer must give `h`.)
- `game.cpp` › `drawGame`, the particle loop: `fillRect((int)p.x, (int)p.y, 1, p.h, p.color)`.

## Step 2: constants and helpers (`source/game.cpp`)
In the Bosses constant block, replace `const float TRAUMA_BEAM = 0.3f;` with:
```cpp
const float BEAM_GROW_TIME = 0.1f;               // s for a beam or eruption to reach its full length
const float TRAUMA_BEAM_FIRE = 0.7f, BEAM_RUMBLE = 0.45f;   // kick when it fires; trauma floor while it's drawn
const float BEAM_SPARK_RATE = 25;                // sparks per s off a beam's edges
const float BEAM_SHARD_GAP = 6;                  // px of beam per shard when it ends
```
- Volcano block: change `const float ERUPTION_WARN = 1.0f, ERUPTION_TIME = 0.6f, TRAUMA_ERUPTION = 0.3f;` to
  `const float ERUPTION_WARN = 1.0f, ERUPTION_TIME = 0.6f;` (keep its comment).
- Game feel block: add `const float TRAUMA_STRONG = 0.6f;   // frost wave and shield pop, like a rocket blast`.

Add these three static functions right after `spawnParticles` (`randomUnit`, `paletteColor` and `std::min` are already available there):
```cpp
// How far a firing beam has grown, 0..1: fast at first, easing into its full length.
static float beamReach(float elapsed) {
    float t = std::min(elapsed / BEAM_GROW_TIME, 1.0f);
    return 1 - (1 - t) * (1 - t);
}

// Now and then a spark flies sideways off a random point of the beam's edge.
static void beamSpark(Game& game, float x, float top, float bottom, int width, uint32_t color, float dt) {
    if (randomUnit(game) >= BEAM_SPARK_RATE * dt) return;
    float side = randomUnit(game) < 0.5f ? -1.0f : 1.0f;
    game.particles.push_back({x + side * width / 2.0f, top + randomUnit(game) * (bottom - top), side * (30 + randomUnit(game) * 30),
                              (randomUnit(game) - 0.5f) * 40, 0.12f + randomUnit(game) * 0.1f,
                              randomUnit(game) < 0.5f ? paletteColor('w') : color, 1});
}

// A finished beam breaks into small shards along its length that drift off, fall and vanish.
static void spawnBeamShards(Game& game, float x, float top, float bottom, int width, uint32_t color) {
    for (float y = top; y < bottom; y += BEAM_SHARD_GAP) {
        float side = randomUnit(game) - 0.5f;   // -0.5..0.5 across the beam
        game.particles.push_back({x + side * width, y + randomUnit(game) * BEAM_SHARD_GAP, side * 40, 5 + randomUnit(game) * 15,
                                  0.35f + randomUnit(game) * 0.25f, randomUnit(game) < 0.5f ? paletteColor('w') : color, 2});
    }
}
```
(Braced-list elements are evaluated left to right, so the random sequence stays deterministic.)

## Step 3: strong-attack shake (`source/game.cpp`)
- `frostWave`: `feedback(game, TRAUMA_BEAM, 0, FLASH_BLAST);` → `feedback(game, TRAUMA_STRONG, 0, FLASH_BLAST);`
- `updateGame`, the ship-hit block `if (hit && player.shield) { ... }`: add `feedback(game, TRAUMA_STRONG, 0, 0);` after the `spawnParticles` line.
- Don't touch `explode` (the rocket already uses `TRAUMA_BLAST` 0.6).

## Step 4: the player's magic beam (`source/game.cpp` › `updatePlayer`)
Replace the line `player.beamTimer = std::max(0.0f, player.beamTimer - dt);` and the `if (player.beamCharge > 0) { ... }` block after it with this order:
1. `float beamBefore = player.beamTimer;` then the existing decrement line.
2. Damage once when the beam reaches the top:
   ```cpp
   const float reachedAt = MAGIC_BEAM_TIME - BEAM_GROW_TIME;   // beamTimer when the beam reaches the top
   if (beamBefore > reachedAt && player.beamTimer <= reachedAt) {
       float left = player.beamX - MAGIC_BEAM_W / 2.0f;
       // ... the existing three loops (aliens, bosses, fish), moved here unchanged
   }
   ```
3. While on: `if (player.beamTimer > 0) {` set `game.trauma = std::max(game.trauma, BEAM_RUMBLE);` and call
   `beamSpark(game, player.beamX, player.beamBottom * (1 - beamReach(MAGIC_BEAM_TIME - player.beamTimer)), player.beamBottom, MAGIC_BEAM_W, paletteColor('c'), dt);` `}`
4. Ended on its own: `if (beamBefore > 0 && player.beamTimer == 0) spawnBeamShards(game, player.beamX, 0, player.beamBottom, MAGIC_BEAM_W, paletteColor('c'));`
5. The charge block stays, minus the damage loops. At the fire moment (inside `if (player.beamCharge <= 0)`) it keeps
   `beamCharge = 0`, `beamTimer = MAGIC_BEAM_TIME`, `beamBottom = ...`, and the explosion sound, and does:
   `feedback(game, TRAUMA_BEAM_FIRE, 0, 0);` (was `TRAUMA_BEAM`) and
   `spawnParticles(game, player.beamX, player.beamBottom, 6, paletteColor('c'), 40);` (the ball's burst as it fires).
   It runs after steps 1-4, so the first decrement and damage check happen on later steps.

## Step 5: the mage beam and the eruption (`source/game.cpp` › `updateBoss`, `updateGame`)
`updateBoss`, the `if (boss.ballTimer > 0) { ... } else { ... }` block:
- At the fire moment: `feedback(game, TRAUMA_BEAM_FIRE, 0, 0);` for both types (replaces the `TRAUMA_ERUPTION : TRAUMA_BEAM` choice),
  plus `if (boss.type != BOSS_FIRELORD) spawnParticles(game, boss.ballX, boss.ballY, 6, paletteColor('m'), 40);`
- The `else` branch: `float before = boss.beamTimer;`, the existing decrement, then
  `if (before > 0 && boss.beamTimer == 0 && boss.type != BOSS_FIRELORD) spawnBeamShards(game, boss.ballX, boss.ballY, SCREEN_H, BEAM_W, paletteColor('m'));`
- Right after the whole if/else:
  ```cpp
  if (boss.beamTimer > 0) {   // a firing beam or eruption rumbles; the mage's throws sparks
      game.trauma = std::max(game.trauma, BEAM_RUMBLE);
      if (boss.type != BOSS_FIRELORD)
          beamSpark(game, boss.ballX, boss.ballY, boss.ballY + beamReach(BEAM_TIME - boss.beamTimer) * (SCREEN_H - boss.ballY), BEAM_W, paletteColor('m'), dt);
  }
  ```

`updateGame`, the ship-hit loop over `game.bosses`: replace the two beam lines (`int beamW = ...` and the `boss.beamTimer > 0 && overlap(...)` line) with:
```cpp
if (boss.alive && boss.beamTimer > 0) {   // only the part of the beam or eruption that has grown kills
    bool lord = boss.type == BOSS_FIRELORD;
    float reach = beamReach((lord ? ERUPTION_TIME : BEAM_TIME) - boss.beamTimer);
    float top = lord ? SCREEN_H * (1 - reach) : boss.ballY, length = lord ? SCREEN_H - top : reach * (SCREEN_H - boss.ballY);
    int beamW = lord ? ERUPTION_W : BEAM_W;
    if (overlap(boss.ballX - beamW / 2.0f, top, beamW, length, hx, hy, HITBOX, HITBOX)) hit = true;
}
```

## Step 6: drawing (`source/game.cpp` › `drawGame`)
The pulsing base ball, used twice: radius `int r = BALL_MAX_RADIUS + (int)(game.feelTime * 20) % 2;`, then
`fillCircle(x, y, r, <beam colour>)` and `fillCircle(x, y, r - 2, paletteColor('w'))`.
- **Player beam** (`if (player.beamTimer > 0)`): `float reach = beamReach(MAGIC_BEAM_TIME - player.beamTimer);`
  `int bottom = (int)player.beamBottom, tip = (int)(player.beamBottom * (1 - reach));`
  Draw the `'c'` rect and the `'w'` core from `tip` with height `bottom - tip` (same x and widths as today);
  if `reach < 1`, a white cap `fillRect((int)player.beamX - MAGIC_BEAM_W / 2, tip, MAGIC_BEAM_W, 2, paletteColor('w'))`;
  then the base ball at `((int)player.beamX, bottom)` in `'c'`.
- **Mage beam** (`if (boss.beamTimer > 0)`, after the ball block): `float reach = beamReach(BEAM_TIME - boss.beamTimer);`
  `int length = (int)(reach * (SCREEN_H - by));` Draw the `'m'` rect and `'w'` core from `by` with height `length`;
  if `reach < 1`, a white cap `fillRect(bx - BEAM_W / 2, by + length - 2, BEAM_W, 2, paletteColor('w'))`; then the base ball at `(bx, by)` in `'m'`.
- **Eruption** (fire lord's `if (boss.beamTimer > 0)`): `float reach = beamReach(ERUPTION_TIME - boss.beamTimer);`
  `int tip = (int)(SCREEN_H * (1 - reach));` Its three rects start at `tip` with height `SCREEN_H - tip`;
  if `reach < 1`, a cap `fillRect(bx - ERUPTION_W / 2, tip, ERUPTION_W, 2, paletteColor('N'))`. No ball.
- Leave the charging drawings (guide lines, growing balls, the bubbling pool) unchanged.

## Step 7: tests (`tests/game_tests.cpp`)
Existing tests to adjust:
- `mageBallBeamsItsColumnAfterOneAndAHalfSeconds`: change the middle `run(game, NO_INPUT, 0.2f);` to `run(game, NO_INPUT, 0.3f);` (the beam needs its 0.1 s to reach the ship).
- `beamDrawsAMagentaColumn`: add `game.bosses[0].ballY = 21;` after the `ballX` line.
- `frostWaveFreezesHealsAndShatters`: right after its first `updateGame(game, &r2, DT);` add `CHECK(game.trauma > 0.5f);`.
- `bubbleShieldTakesAHitAndChains`: append `&& game.trauma > 0.5f` to the check that has `game.shieldBubbles.size() == 3`.
- `magicBeamHitsItsColumnOnce`: no change; it must still pass.

New tests (add each to the list in `main` next to the related ones):
```cpp
static void magicBeamHitsWhenItReachesTheTop() {
    // same setup as magicBeamHitsItsColumnOnce (aliens at x 158, y 20/40/60 with hp 40; space buff armed), then:
    // updateGame with R2 once; step NO_INPUT until p.beamTimer > 0 (at most 120 steps)
    // CHECK: every alien hp still 40, game.trauma > 0.6f
    // run 0.15 s → CHECK aliens 0-2 hp == 10, game.trauma >= 0.44f
    // run 0.3 s  → CHECK p.beamTimer == 0 and at least 20 particles with h == 2
}
static void mageBeamKillsOnlyOnceGrown() {
    // startBossGame(game, 15); mage = bosses[0]; p.invincibleTimer = 0
    // mage.ballX = p.x + 8; mage.ballY = 21; mage.ballTimer = DT / 2
    // updateGame once → CHECK mage.beamTimer > 0 && p.alive (the beam has no length yet)
    // run 0.15 s → CHECK !p.alive
}
static void mageBeamRumblesThenShatters() {
    // startBossGame(game, 15); p.invincibleTimer = 1e9f; mage.ballX = 20; mage.ballY = 21; mage.ballTimer = DT / 2
    // run 0.5 s → CHECK game.trauma >= 0.44f
    // run 0.4 s → CHECK mage.beamTimer == 0 and at least 20 particles with h == 2
}
static void eruptionLeavesNoShards() {
    // startBossGame(game, 15, 4); p.invincibleTimer = 1e9f; lord.ballX = 20; lord.ballTimer = DT / 2
    // run 0.7 s → CHECK lord.beamTimer == 0 and no particle with h == 2
}
```
Write them out in full with the existing helpers (`startSafeGame`, `startBossGame`, `parkedAlien`, `run`, `DT`, `NO_INPUT`).
`make test` must pass. If an unrelated test fails because the beams now use extra random numbers, report it rather than loosening it.

## Step 8: docs
- `docs/features/level-buffs/design.md` rule 6 becomes: "**Space (wand):** a wand sits on the ship's right wing. Magic beam: a ball grows at the wand's star for 0.8 s, then fires: the ball stays at the base and a 12 px beam shoots up to the top of the screen in 0.1 s and stays 0.4 s in all, throwing the odd spark, then breaks into small shards. When it reaches the top, every alien, boss and live fish touching the column takes 30 damage once. Cyan edges, white core, never magenta. It kicks the screen when it fires and rumbles while it's on." In the Numbers table, `Magic ball / beam` becomes `0.8 / 0.4 (grows in 0.1), 12 wide`. Rule 10: add "The pop shakes the screen like a rocket blast." Add a rule-8 sentence: "The wave shakes the screen like a rocket blast." Add the History line `- 2026-11-18: beam build-up, shards and shake (D-032).`
- `docs/features/bosses/design.md` rule 6: replace from "then fires a beam" to the end of the sentence with "then fires: the ball stays at the beam's base and a 24 px wide magenta column (white core 8 px) shoots down to the bottom in 0.1 s, lasts 0.8 s with the odd spark, then breaks into shards. It kicks the screen when it fires and rumbles while it's on; the explosion sound plays. A ship touching the grown part of the beam dies (respawn invincibility protects it)." Delete "The ball disappears when the beam starts." Numbers row `Beam`: `24 px (core 8) × full height, grows in 0.1 s, 0.8 s, trauma 0.7 then 0.45`. **Updated:** 2026-11-18; History line as above.
- `docs/features/volcano/design.md` rule 6: "then a 16 px column of fire from the bottom to the top for 0.6 s that kills" → "then a 16 px column of fire that rises from the bottom to the top in 0.1 s and lasts 0.6 s, shaking the screen; only its risen part kills". History line as above.
- `docs/features/game-feel/design.md`: rule 1's first sentence becomes "Only explosions, beams and strong attacks shake the screen (rocket blasts, beams and eruptions, the frost wave, the bubble shield popping, the boss's death, the player's death); shooting and ordinary kills never do." Add table rows: `| beam or eruption fires | large | 0.7, then at least 0.45 while on | 0 | 6 at the ball, sparks, shards at the end | no |`, `| frost wave | large | 0.6 | 0 | existing | 0.05 s |`, `| bubble shield pops | large | 0.6 | 0 | existing | no |`. **Updated:** 2026-11-18; History line `- 2026-11-18: beams, frost wave and shield pop shake (D-032).`
- `docs/GDD.md`: decision row after D-031:
  `| D-032 | 2026-11-18 | Beams grow from their ball in 0.1 s, spark, and end in shards; the eruption grows too; beams, the frost wave and the shield pop shake the screen; the player's beam hits when it reaches the top | beams appearing at once; kick-only shake; hit-stop on the shield pop | user's request: beams felt flat and trauma 0.3 rounded to no visible shake | [level buffs](features/level-buffs/design.md), [bosses](features/bosses/design.md), [volcano](features/volcano/design.md), [game feel](features/game-feel/design.md) |`
  Change log: `**0.26 (2026-11-18):** beam build-up, shards and strong-attack shake (D-032).`
- Run `python3 .claude/skills/game-docs/scripts/check_docs.py docs` and fix what it reports.

## Step 9: PS3 check (needs the user)
1. `make test`, then `make ps3`, then in one locked command: `scripts/ps3-lock.sh bash -c 'make install'` (reaching a wave-15 boss with a buff can't be driven by `make pad`, so the user plays it).
2. Ask the user to start `make log` in a terminal, launch NAME game, open the level picker, and with L3 test mode on play to wave 15 of LEVEL 1, then VOLCANO and GLACIER/DEEP SEA for their buffs. They check:
   - The mage's beam shoots down from its ball, sparks a little, rumbles the screen and breaks into shards (log: `mage beam at x ...`).
   - After catching the wand buff, R2's beam shoots up from the ball at the wand, and enemies take damage as it reaches the top (log: `player 1 magic beam at x ...`).
   - On the volcano, the fire lord's eruption rises from the floor and shakes (log: `eruption at x ...`).
   - The frost wave and a shield pop shake the screen; with SHAKE: 0% in settings nothing shakes.
3. If the shake is too strong or too weak, change only `TRAUMA_BEAM_FIRE`, `BEAM_RUMBLE` or `TRAUMA_STRONG`; if the shards look too dense or sparse, `BEAM_SHARD_GAP`.

## Done when
- `make test` passes, `make ps3` builds, `check_docs.py` is clean.
- On the PS3: both beams grow out of a ball at their base in a blink, spark, shake the screen and break into shards; the eruption rises and shakes; the frost wave and a shield pop shake.
