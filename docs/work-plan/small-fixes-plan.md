# Plan: small fixes after play (ships, mage, gorilla, ice, lava, deep sea)

For the implementer: follow the steps in order. Every design choice below is final, so don't redesign anything.
Read only the files each step names, and don't re-read a file after editing it. Code style: follow `CLAUDE.md` and
the existing code (plain C++14, no new libraries, short one-line comments, no helpers for one-off logic).
Commit straight to `main` when done, with this plan file and `docs/work-plan/small-fixes/` in the same commit.

The pixel art is already drawn: `docs/work-plan/small-fixes/new-sprites.h` (17 sprites) and
`docs/work-plan/small-fixes/preview.png`. Step 1 copies it with a script; don't open, redraw or retype the art.

## What the player gets
1. **Co-op ships:** player 2's ship has a gold hull (`Y`), orange cockpit and red-orange stripes; player 1's stays grey.
2. **Mage (level 1, waves 15/20):** a tall starry wizard's hat (rows 0-8 of the sprite) and a wand with a star tip in its
   right hand. The power ball now forms at the wand's star (sprite px 30, 8) instead of the hat tip.
3. **Gorilla (jungle, waves 15/20):** bulky dark-brown fur, light-brown shoulders and arms, a dark leathery face; no more grey "shirt".
4. **Glacier ice blocks (yeti and golem):** 24×24 px instead of 8×8, see-through: the inside shows the picture under it
   tinted pale blue. Same speed (80 px/s aimed) and freeze. The ice cube around a frozen ship is see-through the same way.
5. **Volcano imp:** its lava drip is one big drop (10×12 px) falling straight down from the imp at 60 px/s. It scorches
   like the small drops (doesn't kill). The fire lord's lava rain keeps its 5 small drops.
6. **Deep sea bubble trail:** while the stick moves the ship (stick length > 0.2), a bubble leaves one of the two engine
   nozzles every 0.05 s and rises at 15 px/s for 0.6 s, drifting a little sideways. Only on level 5.
7. **Fisherman (deep sea, waves 15/20):** drawn as a fish seen from the side in a yellow fisherman's hat, holding the rod
   (tip at sprite px 31, 0) and a fish bucket (centre px 16, 17). New attack **live fish**:
   - It lobs 2 fish from the bucket to random points of the ship zone (the peel arc, 0.8 s; the second 0.2 s later),
     with an orange landing mark. At most 3 live fish are out (landed + in flight): with 2 out it throws 1; with 3 it streams.
   - A landed fish (9×6, orange with magenta fins = deadly) swims at the nearest living ship at 35 px/s, facing the way it swims.
     Touching a non-blinking ship kills the ship and the fish. One bullet, a rocket touch or a rocket blast kills it (200 points).
     It blinks in its last second and is gone 6 s after landing. Landing plays the splat sound.
   - Rolls: 30 % stream, 15 % big bubble, 15 % fan, 20 % live fish, 20 % hook cast.

## Step 1: palette, art and see-through drawing (`source/sprites.h`, `source/screen.cpp`)
- Run `python3 docs/work-plan/small-fixes/apply_sprites.py` from the repo root. It replaces SHIP2_A/B, MAGE_A/B,
  GORILLA_A/B/LAUGH/POUND, FISHERMAN_A/B, ICE_BLOCK, ICE_CUBE in place and adds BIG_LAVA_DROP, LIVE_FISH_L_A/B,
  LIVE_FISH_R_A/B just above the `// 3x5 font` comment. It prints one `replaced`/`added` line per sprite (12 + 5).
- `sprites.h` › `paletteColor`: add these cases after `case 'X'`:
  ```cpp
          case 'W': return 0xFF3A2212;   // gorilla: dark brown (shadows, hands, feet)
          case 'P': return 0xFF62391E;   // gorilla: brown fur
          case 'S': return 0xFF8A5631;   // gorilla: light brown (shoulders, arms)
          case '~': return 0xFF9FE0FF;   // see-through ice: drawSprite mixes a quarter of it into the pixel under it
  ```
- `sprites.h` comments (edit the one-line comment above each sprite; add one where there is none):
  - above SHIP2_A: `// Player 2's ship: the same shape with a gold hull, an orange cockpit and red-orange stripes.`
  - above MAGE_A (none today): `// Mage: the green alien under a tall starry hat, holding a wand whose star (30, 8) casts the power ball.`
  - above GORILLA_A: `// Gorilla: bulky brown fur, light-brown shoulders, a dark leathery face and angry red eyes.`
  - above FISHERMAN_A: `// Fisherman fish: a fish in a yellow fisherman's hat with a rod (tip at 31, 0) and a fish bucket (centre 16, 17).`
  - above ICE_BLOCK keep `// Harmful but not deadly: no magenta.`; above BIG_LAVA_DROP: `// The imp's big lava drop: scorches, doesn't kill.`
  - above LIVE_FISH_L_A: `// Live fish from the fisherman's bucket: magenta fins mean it kills on touch; L swims left, R right.`
- `screen.cpp` › `drawSprite`: keep the pixel char, and for `'~'` blend instead of overwrite (3/4 under + 1/4 ice; no channel can overflow):
  ```cpp
  char c = sprite.pixels[row * sprite.w + col];
  uint32_t color = paletteColor(c);
  if (!color || px < 0 || px >= SCREEN_W) continue;
  uint32_t& pixel = screenPixels[py * SCREEN_W + px];
  pixel = c == '~' ? 0xFF000000 | (((pixel >> 1) & 0x7F7F7F) + ((pixel >> 2) & 0x3F3F3F) + ((color >> 2) & 0x3F3F3F)) : color;
  ```
  `drawSpriteSolid` stays as is. Don't rerun `make_xmb_art.py` (it only uses SHIP_A) or `levels_art.py`.

## Step 2: types (`source/game.h`)
- `enum ThrownKind`: append `THROWN_BIG_LAVA` after `THROWN_BOOT`. `Thrown` comment: "...except the ice block (freezes) and the lava drops (scorch)".
- `struct Player`: add `float trailTimer;       // s until the next bubble of the deep-sea trail`.
- `struct Lob`: add a last field `bool fish;` and change its comment to
  `// a harmless peel or poop, or a live fish, in flight; waits while age < 0`. Existing `{..., false}` / `{..., true}` inits stay valid.
- After `struct Mist`, add:
  ```cpp
  struct Fish { float x, y, life; bool left; };   // a live fish swimming at the nearest ship; top-left of its 9x6 sprite
  ```
- `struct Game`: after `mists`, add `std::vector<Fish> fish;      // live fish in the ship zone (deep sea)`.

## Step 3: rules (`source/game.cpp`)
Constants (edit in place):
- Bosses: replace the `HAT_TIP_X/Y` line with
  `const int WAND_TIP_X = 30, WAND_TIP_Y = 8;    // px in the mage sprite: the wand's star, where the power ball appears`;
  in `updateBoss` attack 2 use `WAND_TIP_X/Y`.
- Volcano: `const int LAVA_RAIN_DROPS = 5;` and `const float LAVA_DROP_SPEED = 60, LAVA_RAIN_SPEED = 70;   // px/s: the imp's big drop falls straight down`
  (`LAVA_DRIP_DROPS` and `LAVA_DRIP_SIDEWAYS` go away).
- Deep sea: replace the `FISHER_*_CHANCE` and `ROD_TIP` lines, and add after them:
  ```cpp
  const float FISHER_STREAM_CHANCE = 0.3f, FISHER_BIG_CHANCE = 0.15f, FISHER_FAN_CHANCE = 0.15f, FISHER_FISH_CHANCE = 0.2f;   // the rest casts the hook
  const int ROD_TIP_X = 31, ROD_TIP_Y = 0;   // px in the fisherman sprite
  const int BUCKET_X = 16, BUCKET_Y = 17;    // px in the fisherman sprite: live fish are thrown from here
  const int FISH_PER_THROW = 2, MAX_FISH = 3;   // live fish out: landed plus in flight
  const float FISH_SPEED = 35, FISH_LIFE = 6, FISH_BLINK = 1, FISH_THROW_GAP = 0.2f;   // px/s, s, s, s
  const int FISH_W = 9, FISH_H = 6, FISH_SCORE = 200;
  const float TRAIL_GAP = 0.05f, TRAIL_RISE = 15, TRAIL_LIFE = 0.6f;   // s between bubbles, px/s, s
  ```
- `THROWN_W = {6, 6, 6, 24, 4, 6, 5, 5, 9, 7, 10}`, `THROWN_H = {6, 6, 6, 24, 9, 6, 6, 5, 9, 6, 12}`;
  `THROWN_SPRITES`: append `{&BIG_LAVA_DROP, &BIG_LAVA_DROP}`.

`explode`, after the boss loop:
```cpp
for (Fish& f : game.fish) {   // live fish inside the blast die
    float dx = f.x + FISH_W / 2.0f - x, dy = f.y + FISH_H / 2.0f - y;
    if (f.life <= 0 || dx * dx + dy * dy > EXPLOSION_RADIUS * EXPLOSION_RADIUS) continue;
    f.life = 0;   // removed in updateGame
    if (owner >= 0) addScore(game, game.players[owner], FISH_SCORE);
    spawnParticles(game, f.x + FISH_W / 2.0f, f.y + FISH_H / 2.0f, 8, paletteColor('O'), 50);
}
```

`updatePlayer`, right after the two `player.x/y = clamp` lines (before the peel check):
```cpp
if (game.level == 5 && a.moveX * a.moveX + a.moveY * a.moveY > 0.04f) {   // the deep sea: a moving ship leaves bubbles
    player.trailTimer -= dt;
    if (player.trailTimer <= 0) {
        player.trailTimer = TRAIL_GAP;
        float nozzleX = player.x + (randomUnit(game) < 0.5f ? 4 : 11);
        game.particles.push_back({nozzleX, player.y + SHIP_H, (randomUnit(game) - 0.5f) * 16, -TRAIL_RISE, TRAIL_LIFE, paletteColor('i')});
    }
}
```

`updateBoss`:
- Right after `float roll = big ? randomUnit(game) : 0;` add the live-fish count (used by the fisherman):
  `int fishOut = (int)game.fish.size();` then `for (const Lob& lob : game.lobs) fishOut += lob.fish;`
- Attack-id comment: end it with `12 big bubble, 13 hook cast, 14 live fish.`
- Fisherman branch:
  ```cpp
  attack = roll < FISHER_STREAM_CHANCE ? 0 : roll < FISHER_STREAM_CHANCE + FISHER_BIG_CHANCE ? 12
         : roll < FISHER_STREAM_CHANCE + FISHER_BIG_CHANCE + FISHER_FAN_CHANCE ? 1
         : roll < FISHER_STREAM_CHANCE + FISHER_BIG_CHANCE + FISHER_FAN_CHANCE + FISHER_FISH_CHANCE ? 14 : 13;
  if ((attack == 13 && boss.hookState != HOOK_NONE) || (attack == 14 && fishOut >= MAX_FISH)) attack = 0;
  ```
- Attack 9 becomes: `game.thrown.push_back({centreX - THROWN_W[THROWN_BIG_LAVA] / 2.0f, bottomY, 0, LAVA_DROP_SPEED, THROWN_BIG_LAVA, 0, 0});`
  plus `game.soundRequested[SOUND_BANANA_THROW] = true;`, comment `// one big lava drop, straight down`.
- New branch before the final `else {   // 13`:
  ```cpp
  } else if (attack == 14) {   // live fish lobbed from the bucket to random points of the ship zone
      for (int i = 0; i < std::min(FISH_PER_THROW, MAX_FISH - fishOut); i++) {
          float toX = 8 + randomUnit(game) * (SCREEN_W - 16);
          float toY = MOVE_ZONE_TOP + 7 + randomUnit(game) * (SCREEN_H - MOVE_ZONE_TOP - 14);
          game.lobs.push_back({boss.x + BUCKET_X, boss.y + BUCKET_Y, toX, toY, -FISH_THROW_GAP * i, false, true});
      }
      game.soundRequested[SOUND_BANANA_THROW] = true;
  ```

`updateGame`:
- Bullet loop, after the boss loop:
  ```cpp
  for (Fish& f : game.fish) {
      if (bullet.y <= -50 || f.life <= 0 || !overlap(bullet.x, bullet.y, 2, 4, f.x, f.y, FISH_W, FISH_H)) continue;
      f.life = 0;   // removed below
      addScore(game, game.players[bullet.owner], FISH_SCORE);
      spawnParticles(game, f.x + FISH_W / 2.0f, f.y + FISH_H / 2.0f, 8, paletteColor('O'), 50);
      game.soundRequested[SOUND_ALIEN_DEATH] = true;
      bullet.y = -100;   // spent; removed below
  }
  ```
- Rocket loop: `for (const Fish& f : game.fish) if (f.life > 0 && overlap(rocket.x, rocket.y, 4, 8, f.x, f.y, FISH_W, FISH_H)) hit = true;`
- Lob landing: order the branches `if (lob.poop) {...} else if (lob.fish) {...} else {peel}`. The fish branch:
  `game.fish.push_back({lob.toX - FISH_W / 2.0f, lob.toY - FISH_H / 2.0f, FISH_LIFE, randomUnit(game) < 0.5f});`,
  `spawnParticles(game, lob.toX, lob.toY, 4, paletteColor('i'), 25);`, `game.soundRequested[SOUND_SPLAT] = true;`.
- After `for (Mist& mist ...) mist.age += dt;`:
  ```cpp
  for (Fish& f : game.fish) {   // a live fish swims at the nearest living ship
      f.life -= dt;
      float bestDx = 0, bestDy = 0, best = -1;
      for (const Player& p : game.players) {
          float dx = p.x + SHIP_W / 2.0f - (f.x + FISH_W / 2.0f), dy = p.y + SHIP_H / 2.0f - (f.y + FISH_H / 2.0f);
          if (p.alive && (best < 0 || dx * dx + dy * dy < best)) { best = dx * dx + dy * dy; bestDx = dx; bestDy = dy; }
      }
      if (best < 0) continue;
      float length = sqrtf(best) + 0.001f;
      f.x += bestDx / length * FISH_SPEED * dt;
      f.y += bestDy / length * FISH_SPEED * dt;
      if (fabsf(bestDx) > 1) f.left = bestDx < 0;
  }
  ```
- Ice/lava collision loop: include `THROWN_BIG_LAVA` with the lava drop (`if (t.kind != THROWN_ICE_BLOCK && t.kind != THROWN_LAVA_DROP && t.kind != THROWN_BIG_LAVA) continue;`),
  and spawn both particle bursts at the hazard's centre (`t.x + THROWN_W[t.kind] / 2.0f, t.y + THROWN_H[t.kind] / 2.0f`).
  The deadly-thrown loop skips `THROWN_BIG_LAVA` too.
- Deadly checks (inside `if (player.invincibleTimer == 0)`), after the thrown loop:
  ```cpp
  for (Fish& f : game.fish) {   // the deadly box is 1 px inside the sprite
      if (!hit && f.life > 0 && overlap(f.x + 1, f.y + 1, FISH_W - 2, FISH_H - 2, hx, hy, HITBOX, HITBOX)) {
          hit = true;
          f.life = 0;   // removed below
      }
  }
  ```
- Removal block: erase fish with `life <= 0`. Level-clear block: add `game.fish.clear();` next to `game.lobs.clear();`.

`drawGame`:
- After the peels loop:
  ```cpp
  int swim = (int)(game.time * 8) % 2;
  for (const Fish& f : game.fish) {   // blinks in its last second
      if (f.life <= FISH_BLINK && fmodf(f.life, 0.2f) < 0.1f) continue;
      drawSprite(f.left ? (swim ? LIVE_FISH_L_B : LIVE_FISH_L_A) : (swim ? LIVE_FISH_R_B : LIVE_FISH_R_A), (int)f.x, (int)f.y);
  }
  ```
- Lob drawing: `const Sprite& sprite = lob.fish ? LIVE_FISH_R_A : lob.poop ? POOP : PEEL;` and the mark colour
  `paletteColor(lob.fish ? 'O' : lob.poop ? 'M' : 'H')`.
- Nothing else changes: the frozen-ship `ICE_CUBE` is drawn as today and is see-through through step 1.

## Step 4: log (`source/main.cpp`)
- Next to `size_t mistsBefore = game.mists.size();` add `size_t fishBefore = game.fish.size();`.
- Next to the `stink mist at` log: `if (game.fish.size() > fishBefore) debugNetPrintf(INFO, "live fish landed, %d out\n", (int)game.fish.size());`

## Step 5: tests (`tests/game_tests.cpp`)
- `ALL_SPRITES`: append `&BIG_LAVA_DROP, &LIVE_FISH_L_A, &LIVE_FISH_L_B, &LIVE_FISH_R_A, &LIVE_FISH_R_B`.
- `onlyDeadlyShotsUseMagenta`: add the four `LIVE_FISH_*` sprites to the deadly list.
- `impCyclesFireballsLavaAndFan`, middle check becomes
  `CHECK(countThrown(game, THROWN_BIG_LAVA) == 1 && countThrown(game, THROWN_LAVA_DROP) == 0 && game.thrown[0].vx == 0 && game.thrown[0].vy == 60);`
- New functions, called from `main()` after `emptyCastsSometimesBringABoot();`:
  - `playerTwoShipIsGold`: `SHIP2_A` has `'Y'` and no `'b'`; `SHIP_A` has `'b'` and no `'Y'` (`strchr`).
  - `mageBallFormsAtTheWand`: `startBossGame(game, 15)`; up to 60 times: `ballTimer = beamTimer = 0`, `attackTimer = 0.001f`,
    one `updateGame`; when `ballTimer > 0`, CHECK `ballX == mage.x + 30 && ballY == mage.y + 8`, remember it happened, break. CHECK it happened.
  - `bigIceBlockFreezesFromItsCorner`: `startSafeGame(game, 3)`; push `{p.x - 17, p.y - 6, 0, 0, THROWN_ICE_BLOCK, 0, 0}`
    (only a 24 px block reaches the hitbox); one step; CHECK `p.frozenTimer > 1.5f && game.thrown.empty()`.
  - `seeThroughIceTintsWhatIsUnder`: `fillRect(0, 0, 30, 30, 0xFF0C0F1E)`, `drawSprite(ICE_BLOCK, 0, 0)`; CHECK pixel (10, 12)
    `== (0xFF000000u | (((0xFF0C0F1Eu >> 1) & 0x7F7F7F) + ((0xFF0C0F1Eu >> 2) & 0x3F3F3F) + ((0xFF9FE0FFu >> 2) & 0x3F3F3F)))`
    and `!= 0xFF0C0F1E`.
  - `bubbleTrailOnlyWhileMovingInTheDeepSea`: for levels 1 and 5: `startSafeGame`, clear particles, `run(NO_INPUT, 0.5f)` → empty;
    `run(right, 0.5f)` (`moveX = 1`) → level 1 empty; level 5 `size() >= 8` and every particle has `vy < 0`.
  - `fishermanKeepsAtMostThreeLiveFishOut`: `startBossGame(game, 15, 5)`; up to 200 steps: `bosses[0].attackTimer = 0.001f`,
    `hookState = HOOK_NONE`, one step; count `fish.size()` + fish lobs; CHECK every count `<= 3`; CHECK a count of 3 was reached.
    Also CHECK every fish lob seen has `toX` in [8, 312] and `toY` in [107, 173].
  - `liveFishLandsAndChasesTheShip`: `startBossGame(game, 15, 5)`; `attackTimer = 100`; push lob
    `{100, 40, 200, 120, 0, false, true}`; run 0.85 s → `fish.size() == 1 && lobs.empty() && soundRequested[SOUND_SPLAT]`.
    Then set `fish[0] = {p.x + 60, p.y + 4, 6, false}` (level with the ship's centre), run 0.5 s →
    `fish[0].left && fabsf(startX - fish[0].x - 17.5f) < 1` (35 px/s, all sideways).
  - `liveFishKillsAndDies`: `startSafeGame(game, 5)`, `p.invincibleTimer = 0`, `fish.push_back({p.x + 4, p.y + 4, 6, true})`,
    one step → `!p.alive && game.fish.empty()`.
  - `shotsAndTimeEndLiveFish`: `startSafeGame(game, 5)`; fish at `{100, 120, 6, true}` and bullet `{104, 126, -320, 0, 0}`; one step →
    `fish.empty() && players[0].score == 200`. Then push `{20, 170, 6, true}` (the ship is invincible), run 5.9 s → 1 fish, run 0.2 s → 0.
- Run `make test`: all must pass. If an older deep-sea test breaks because the trail uses `randomUnit`, adjust that test's input, not the rule.

## Step 6: docs
- `docs/features/co-op/design.md` rule 8: "Player 2's ship has the same shape with a gold hull, an orange cockpit and red-orange stripes, so the two ships read apart at a glance."
- `docs/features/bosses/design.md`: rule 5 "A mage is the classic alien under a tall starry wizard's hat, holding a wand with a star tip."; rule 6 "appears at the star of the mage's wand"; numbers row `Power ball | at the wand's star (30, 8), ...`. `bosses/testing.md` line 24: "charges at the wand's star".
- `docs/features/jungle/design.md`: append to rule 10: "The gorilla is bulky with brown fur, light-brown shoulders and a dark face."
- `docs/features/glacier/design.md` rule 8: start "**Ice block:** a see-through 24×24 block (both bosses throw it): touching it freezes...", and its Numbers-table row `Ice block | 80 aimed, 24×24 | px/s, px | ...` (keep the last column).
- `docs/features/volcano/design.md` rule 5: "a big lava drop (one 10×12 drop straight down at 60 px/s)"; rule 7: "**Lava drops** (big or small)".
- `docs/features/deep-sea/design.md`: summary line 5 → "...a fisherman fish in a yellow fisherman's hat that tries to hook you and throws live fish from its bucket...";
  rule 7 → the 30/15/15/20/20 rolls ("a stream while its hook is out or 3 live fish are out"); new rule 10 **Live fish** and rule 11 **Bubble trail**
  (texts from "What the player gets" 6 and 7); bundle row `Ship skin | unchanged; a moving ship trails bubbles`; numbers rows for live fish (2 per throw, 3 out, 35 px/s, 6 s, 200 points) and the trail (every 0.05 s, rising 15 px/s, 0.6 s).
- Testing notes: add ticked criteria with the new test names to co-op, bosses, glacier, volcano and deep-sea `testing.md`; add unticked
  "PS3: ... looks right" criteria for each look change (step 7).
- `docs/GDD.md`: version `0.23`; decision row
  `| D-029 | 2026-11-18 | Playtest fixes: gold player-2 ship; bigger mage hat and a wand whose star casts the power ball; brown, bulky gorilla; 24 px see-through ice blocks for both glacier bosses; the imp's lava drip is one big drop straight down; moving ships trail bubbles in the deep sea; the fisherman throws live fish (2 a throw, at most 3 out) that chase the nearest ship | ice blocks bigger for the yeti only; an aimed big drop; fish that flop and spit or hop | user's playtest notes and choices | [co-op](features/co-op/design.md), [bosses](features/bosses/design.md), [jungle](features/jungle/design.md), [glacier](features/glacier/design.md), [volcano](features/volcano/design.md), [deep sea](features/deep-sea/design.md) |`;
  change log `**0.23 (2026-11-18):** playtest fixes: ships, mage, gorilla, ice, lava, deep sea (D-029).`
- Run `python3 .claude/skills/game-docs/scripts/check_docs.py docs` and fix what it reports.

## Step 7: PS3 check (needs the user for the look)
1. `make ps3` must build without warnings in the changed code.
2. Tell the user to get ready to play, then run in the background (one locked command; the user plays for up to 15 minutes):
   `mkdir -p shots && scripts/ps3-lock.sh bash -c 'make log > shots/ps3.log 2>&1 & LOG=$!; make install launch && sleep 900; kill $LOG'`.
   The user picks levels in the LEVEL PICKER (the dev autopilot, Select, helps reach boss waves) and checks: glacier wave 5 (yeti ice block),
   volcano wave 5 (imp big drop), deep sea any wave (bubble trail) and wave 15 (fisherman, live fish), jungle wave 15 (gorilla),
   level 1 wave 15 (mage), and player 2 joining once. If they finish early, run `make shot` first (step 4) and let the command run out, or `make quit`.
3. Log lines that prove it: `player 1 frozen`, `player 1 scorched`, `live fish landed, 2 out` (never more than 3), `boss wave 15: 1 ...`, `mage beam at x ...`, fps ≈ 60
   while ice blocks and fish are on screen.
4. While the game runs, when the user says a 24 px ice block or the fisherman and a live fish are on screen, run `make shot`
   (our own background command holds the PS3 lock, so this doesn't clash) and read `shots/shot.png` to confirm the see-through ice and the new art.
5. Tick the testing criteria the log and shots prove; leave the look verdicts to the user.

## Done when
- `make test` passes, `make ps3` builds, `check_docs.py` is clean.
- On the PS3: the new art shows, ice blocks are big and see-through, the imp throws one big drop, ships trail bubbles in the deep sea, and the fisherman's live fish land and chase the ship at 60 fps.
