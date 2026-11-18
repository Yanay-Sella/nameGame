# Plan: levels 3-5, the glacier, the volcano and the deep sea

For the implementer: follow the steps in order. Every design choice below is final, so don't redesign anything.
Read only the files each step names, and don't re-read a file after editing it. Code style: follow `CLAUDE.md` and
the existing code (plain C++14, no new libraries, short one-line comments, no helpers for one-off logic).
Don't commit: the user commits. When done, list the changed files and suggest a one-line commit message.

Source: the user's "3 more levels" PDF (one page, 2026-11-18): an ice level, a lava level and a water level built on
the jungle's architecture (`jungle-level-plan.md`) with rising difficulty, plus anything that improves the experience
and something funny where it fits. The user was asleep, so every open choice below was taken by the planner; each is
marked *(planner)* and listed in the hand-over. Status: **implemented in the same session** (the code steps below
describe what was built). Pictures of the real game drawn on the Mac: `docs/work-plan/levels/` (step 9 draws them).

## What the player gets

### Flow
```
MAIN MENU ─ CLASSIC ─→ level 1 ─→ JUNGLE ─→ GLACIER ─→ VOLCANO ─→ DEEP SEA ─ DEEP SEA CLEAR (3 s) ─→ end screen (win)
          ├ JUNGLE ───────────→ JUNGLE ─→ GLACIER ─→ ...
          ├ GLACIER ─────────────────────→ GLACIER ─→ ...
          ├ VOLCANO ──────────────────────────────────→ VOLCANO ─→ DEEP SEA ─→ ...
          ├ DEEP SEA ────────────────────────────────────────────────→ DEEP SEA ─→ ...
          └ SETTINGS
```
- Main menu items: `CLASSIC`, `JUNGLE`, `GLACIER`, `VOLCANO`, `DEEP SEA`, `SETTINGS`. Each level item starts a fresh run
  there (3 lives, 3 rockets, weapon 1). All are open for now (locking is a later feature, as for the jungle).
- Beating a level's wave-20 bosses shows `<NAME> CLEAR` (level 1: `LEVEL 1 CLEAR`) for 3 s, then the next level starts
  with the same players (score, lives, rockets and weapon carry over). After the deep sea the end screen is titled
  `DEEP SEA CLEAR`. Every level restarts the wave count; its first banner is the level's name for 2 s, then `WAVE 1`;
  the HUD reads `GLACIER n`, `VOLCANO n`, `DEEP SEA n`. Play again restarts at the menu's level.
- The main menu panel is 36 px taller (6 items); the `HI` line moves to y 150.

### Difficulty ladder (the "change the difficulty" part)
| Level | Formation plays as | Heavies per formation wave (max 3) | Enemy shot speed bonus | Boss HP small / big | Boss reward each |
|---|---|---|---|---|---|
| 1 level 1 | wave n | none | 0 | 100 / 200 | 5,000 × n |
| 2 jungle | wave n + 5 | wave / 8 | 0 | 150 / 300 | 5,000 × (n + 4) |
| 3 glacier | wave n + 8 | wave / 6 | +5 px/s | 180 / 340 | 5,000 × (n + 8) |
| 4 volcano | wave n + 11 | wave / 5 | +10 px/s | 200 / 380 | 5,000 × (n + 12) |
| 5 deep sea | wave n + 14 | wave / 4 | +15 px/s | 220 / 420 | 5,000 × (n + 16) |

(n = wave / 5 for bosses.) Heavies roam exactly like the jungle's rhino beetle (12 HP, 1,000 points, 40/25 px/s, box
y 18…84, twice the shot chance); the third heavy starts at x 152. Level 1's numbers are unchanged.

| Boss wave | Sway px/s (L1, L2, L3, L4, L5) | Attack every s (L1 … L5) |
|---|---|---|
| 5 | 40, 50, 50, 55, 55 | 2.5, 2.2, 2.1, 2.0, 1.9 |
| 10 (twins) | 50, 60, 60, 65, 65 | 2.2, 2.0, 1.9, 1.8, 1.7 |
| 15 | 60, 45, 45, 50, 50 | 1.9, 1.8, 1.7, 1.6, 1.5 (× random 0.6–1.4 from level 2 on) |
| 20 (twins) | 70, 55, 55, 60, 60 | 1.6, 1.6, 1.5, 1.4, 1.3 (× random 0.6–1.4 from level 2 on) |

### Pillar 1 stays: magenta = deadly
Snowballs, icicles, fireballs, eruptions, bubbles and the boot use magenta. The ice block (freezes), the lava drop
(scorches, and helps) and the hook (lifts) never do. Every background pixel is darker than the grey stars `s`.

### Level 3: the glacier *(planner: name, roster, bosses and the slippery ship)*
- **Look:** a frozen cave scrolling down: far layer (7 px/s) of dark ice-crystal pillars with facets; near layer
  (16 px/s) of jagged ice walls on both edges with ice shelves and hanging icicles; 30 dim snowflakes drifting down
  (14 px/s) and swaying.
- **Enemies:** snowflake sprite (1 HP), ice bat (2 HP), frost skull (4 HP), heavy: a little yeti riding a cloud (12 HP).
- **Slippery ice (whole level):** the ship has momentum: its speed moves toward the stick's target speed at 5 per s
  (`v += (target - v) × min(1, 5·dt)`), so it glides into and out of moves. Other levels stay instant.
- **Yeti** (waves 5, 10) cycles three attacks: *snowball volley* (3 aimed snowballs 0.15 s apart, 90 px/s), *ice block*
  (one block thrown at the nearest ship at 80 px/s), *sneeze* (funny one: it rears back for 0.5 s showing `AH...` and its
  sneeze sprite, then `ACHOO!` for 0.6 s and 7 snowballs in a fan, sideways −60…60, down 80 px/s, a small shake).
- **Ice golem** (waves 15, 20) rolls: 30 % volley, 25 % two ice blocks (one at the nearest ship, one at a random point
  of the ship zone), 20 % snowball fan (5, sideways −60…60, down 75), 25 % *icicle rain*: 6 icicles, one at a random x
  in each sixth of the screen, hang shaking at the top for 0.8 s, then fall at 140 px/s. Icicle rain while icicles
  still hang → fan.
- **Ice block** (from the PDF): an 8×8 ice cube. Touching it (hitbox) freezes the ship for 1.6 s: it can't move, shoot
  or fire rockets and is drawn inside an ice cube. **Each fresh R1 press while frozen chips 0.15 s off** (mash to break
  out). The block shatters. Freezing doesn't kill and works on a respawning ship; a block hitting a frozen ship
  refreshes the freeze to 1.6 s.

### Level 4: the volcano *(planner: name, roster, bosses and the eruption)*
- **Look:** a dark volcanic chasm: far layer (8 px/s) of rock blobs, three meandering lava rivers and glowing cracks;
  near layer (18 px/s) of rock walls on both edges with glowing seams; 20 dim embers rising (20 px/s) and flickering.
- **Enemies:** ember sprite (1 HP), fire bat (2 HP), magma skull (4 HP), heavy: a phoenix (12 HP).
- **Fire imp** (waves 5, 10) cycles: *fireball volley* (3 aimed fireballs 0.15 s apart, 95 px/s), *lava drip* (3 lava
  drops, sideways −30, 0, 30, down 60 px/s), *fireball fan* (5, sideways −60…60, down 80).
- **Fire lord** (waves 15, 20) rolls: 30 % volley, 20 % fan, 25 % *lava rain* (5 drops, one at a random x in each fifth,
  from the top at 70 px/s), 25 % *eruption*: a bubbling lava pool and a dotted magenta guide line at the nearest
  ship's x for 1.0 s, then a 16 px fire column from the bottom to the top for 0.6 s (deadly, small shake). Eruption
  while its own is active → fan.
- **Lava drop** (from the PDF): touching it (hitbox) *scorches* the ship for 3 s: it moves at 0.6× speed, but its
  bullets do **2 damage** (drawn as orange fire bolts) and it trails flames; `HOT!` shows above it for 0.6 s. A new drop
  refreshes the 3 s. Lava drops don't kill, work on a respawning ship, and the check is at hit time like the mist.
  Rockets are unchanged.

### Level 5: the deep sea *(planner: name, roster, angler boss, current and the boot)*
- **Look:** dark deep water: far layer (6 px/s) with dithered light rays and distant rocks; near layer (15 px/s) of rock
  and swaying kelp on both edges; 16 dim bubbles rising (25 px/s) and drifting with the current.
- **Enemies:** piranha (1 HP), pufferfish (2 HP), lionfish (4 HP), heavy: a shark (12 HP).
- **Current (whole level):** the water pushes every ship sideways by `18 · sin(0.45 · time)` px/s (one slow back-and-forth
  every 14 s) whenever it can move; the background bubbles drift with it.
- **Anglerfish** (waves 5, 10; "big scary fish") cycles: *bubble stream* (5 aimed bubbles 0.12 s apart, 70 px/s,
  wobbling sideways), *big bubble* (one 9×9 bubble aimed at the nearest ship at 45 px/s; after 1.4 s, or at y 150, it
  pops into 8 bubbles flying out at 60 px/s), *bubble fan* (7, sideways −70…70, down 65).
- **Fisherman fish** (waves 15, 20; big yellow fisherman's hat and a fishing rod) rolls: 35 % stream, 15 % big bubble,
  20 % fan, 30 % *hook cast* (with its hook already out → stream).
- **Hook** (from the PDF): cast at the nearest ship's x, the hook drops from the rod tip at 110 px/s on a line to 10 px
  above the bottom, waits 0.3 s, and reels back up at 160 px/s. A ship (not invincible) whose body (10×10 inside the
  sprite) it touches on the way down or while waiting is **hooked**: the fisherman shouts `GOT ONE!` (1 s), the ship is
  lifted 24 px (60 px/s, never above y 100) and dangles stunned for 1.6 s: no moving, shooting or rockets, shaking,
  dizzy stars; then the hook lets go and reels up. A hooked ship can still be shot.
- **The old boot** *(planner: the funny one)*: a cast that catches nothing comes back with an old boot 35 % of the time;
  the fisherman shows `OLD BOOT` (0.8 s) and throws it at the nearest ship (85 px/s, deadly, magenta laces).

### Sounds (synthesized, `{start s, length s, start Hz, end Hz, volume, noise}`)
| Sound | When | Tones |
|---|---|---|
| `SOUND_SNEEZE` | yeti `ACHOO!` | `{0, 0.25, 4000, 900, 0.4, noise}`, `{0, 0.1, 900, 400, 0.25}` |
| `SOUND_FREEZE` | ship frozen | `{0, 0.3, 3500, 5000, 0.25, noise}`, `{0, 0.3, 1800, 2600, 0.15}` |
| `SOUND_CRACK` | icicles form, ice block thrown | `{0, 0.06, 5000, 3000, 0.3, noise}`, `{0.08, 0.06, 4500, 2500, 0.25, noise}` |
| `SOUND_SIZZLE` | ship scorched | `{0, 0.4, 6000, 3000, 0.25, noise}` |
| `SOUND_BUBBLE` | bubble attacks, big bubble pops | `{0, 0.08, 300, 900, 0.25}`, `{0.09, 0.06, 500, 1100, 0.2}` |
| `SOUND_CAST` | hook cast, boot throw | `{0, 0.2, 2500, 500, 0.2, noise}`, `{0, 0.2, 1200, 400, 0.15}` |
| `SOUND_HOOKED` | ship hooked (reel ticks) | `{0, 0.08, 800, 800, 0.3}`, `{0.1, 0.08, 1000, 1000, 0.3}`, `{0.2, 0.15, 600, 1200, 0.3}` |

Eruptions use `SOUND_EXPLOSION` (as the mage beam). Throws of snowballs and fireballs use `SOUND_BANANA_THROW`.

### Required technology
Nothing new: C++14, the CPU pixel buffer (D-008), sprites as text grids (D-006), synthesized SFX. The scrolling
background keeps two 320×360 tiles, rebuilt in code when the level changes (one-off, during the level banner).

Out of scope: unlocking levels in order, level ship skins, level songs, new XMB art (don't rerun `make_xmb_art.py`).

## Step 1: data (`source/sprites.h`, `source/game.h`)
- Palette (after the jungle block), each `// <level>: <use>`:
  glacier `a 0A1322` deepest, `A 132238` cave wall, `e 1E3550` crystals, `E 2E5470` crystal edges/icicles/snow,
  `i 9FE0FF` pale ice, `I 4A9AD8` mid ice, `q 2A5C9C` deep ice;
  volcano `v 120706` deepest, `V 241009` dark rock, `K 3A1A10` rock edge, `D 5C1C0C` glowing crack, `C 7A3010` lava river/embers,
  `O FF6A10` hot orange, `N FFD84A` white-hot, `Z B02810` deep red-orange;
  sea `1 051220` deepest, `2 0A1E30` water, `3 0E3030` kelp, `4 1A4A3E` kelp light, `5 28506A` light rays/bubbles,
  `6 3FB8A0` teal fish, `7 1D6A5E` dark teal, `8 E0E8C8` pale belly/teeth, `9 E6FF6A` lure glow.
- Sprites: formation `FLAKE_A/B`, `ICEBAT_A/B`, `FROSTSKULL_A/B`, `EMBER_A/B`, `FIREBAT_A/B`, `MAGMASKULL_A/B`,
  `PIRANHA_A/B`, `PUFFER_A/B`, `LIONFISH_A/B` (12×10); heavies `YETICLOUD_A/B`, `PHOENIX_A/B`, `SHARK_A/B` (16×12);
  bosses `YETI_A/B`, `YETI_SNEEZE`, `GOLEM_A/B`, `IMP_A/B`, `FIRELORD_A/B`, `ANGLER_A/B`, `FISHERMAN_A/B` (32×20);
  hazards `SNOWBALL` 6×6, `ICE_BLOCK` 8×8, `ICICLE` 4×9, `ICE_CUBE` 18×16 (drawn over a frozen ship at x − 1, y − 1),
  `FIREBALL_A/B` 6×6, `LAVA_DROP` 5×6, `BUBBLE` 5×5, `BIG_BUBBLE` 9×9, `HOOK` 5×6, `BOOT` 7×6.
- `game.h`: `LEVEL_COUNT = 5`; sounds above; `BossType` adds `BOSS_YETI, BOSS_GOLEM, BOSS_IMP, BOSS_FIRELORD, BOSS_ANGLER,
  BOSS_FISHERMAN`; `ThrownKind` adds `THROWN_SNOWBALL, THROWN_ICE_BLOCK, THROWN_ICICLE, THROWN_FIREBALL, THROWN_LAVA_DROP,
  THROWN_BUBBLE, THROWN_BIG_BUBBLE, THROWN_BOOT`; `Thrown` gets `float delay, age;` (hang time; time moving).
  `Player` gets `vx, vy` (current speed, for the ice), `frozenTimer`, `fireWasHeld`, `scorchTimer`, `hookedTimer`,
  `hookLiftTo`. `Boss`: `poundTimer` becomes `windupTimer` (gorilla pound, yeti sneeze), plus `sayTimer` and
  `const char* say` (speech under the boss), and the hook: `hookState` (`HOOK_NONE, HOOK_DOWN, HOOK_WAIT, HOOK_HOLD,
  HOOK_REEL`), `hookX, hookY, hookTimer`, `hookPlayer`, `hookCaught`. The mage's `ballX/ballTimer/beamTimer` also
  carry the fire lord's eruption. `const char* levelName(int level)` and `levelLabel(int level)` are declared.

## Step 2: rules (`source/game.cpp`)
- Per-level tables replace the level-1 formulas and the `JUNGLE_BOSS_*` constants: `LEVEL_WAVE_OFFSET`,
  `HEAVY_EVERY` (0 = none), `SHOT_SPEED_BONUS`, `LEVEL_BOSS_HP[5][4]`, `LEVEL_BOSS_SPEED[5][4]`,
  `LEVEL_BOSS_INTERVAL[5][4]`, `REWARD_OFFSET`, and `BOSS_TYPES[5][2]` (small, big). `heavyCount` =
  `min(wave / HEAVY_EVERY, 3)`. Big bosses (mage, gorilla, golem, fire lord, fisherman) use random gaps.
- `THROWN_W/H[kind]`; the deadly box is 1 px inside the sprite. Ice blocks and lava drops are handled before the
  invincibility check (they don't kill); the rest kill like shots.
- `updatePlayer`: ice momentum, sea current, scorch speed, the frozen/hooked stun with R1 mash.
- `updateBoss`: the cycles and rolls above; windups; icicles as `Thrown` with `delay`; the hook state machine.
- `updateGame`: thrown move (delay, age, bubble wobble, big-bubble pop), bullet damage 2 when scorched, eruption hits,
  the hook catch, level continue `level < LEVEL_COUNT → level + 1`.

## Step 3: drawing (`source/game.cpp`)
Backgrounds `buildGlacier`, `buildVolcano`, `buildSea` next to `buildJungle` (shared tiles rebuilt on a level change),
overlays (snow, embers, bubbles), the new sprites by level, ice cube, scorch bolts and `HOT!`, eruption, hook line,
boss speech, labels from `levelLabel`, banners and the end panel from `levelName`.

## Step 4: sounds (`source/audio.cpp`) — the table above.

## Step 5: menu and logs (`source/main.cpp`)
Six items; `runLevel = selected + 1` for items 0-4, item 5 is settings; settings back selects item 5 (main) or 1
(pause). Logs: `level %d starts at %.1f s`, `wave %d: %d heavies`, `player %d frozen`, `player %d scorched`,
`player %d hooked`, `<boss> beam/eruption at x`, `boot thrown`, boss names for the new types.

## Step 6: tests (`tests/game_tests.cpp`)
Level ladder, heavy counts, continue chain (jungle → glacier), every boss attack, freeze + mash, ice momentum, scorch
damage, eruption kill, current drift, hook catch/lift/stun/release, boot, backgrounds darker than the stars, magenta
only on deadly sprites, `killingTheLevelBossClearsTheLevel` on the deep sea. `make test` passes.

## Step 7: docs
`docs/features/glacier|volcano|deep-sea/{design,implementation,testing}.md`, GDD (summary, index, flow, decisions
D-024 to D-027, change log 0.22), level-1/jungle/bosses/enemy-waves/music-and-sound/screens notes; `check_docs.py`.

## Step 8: PS3 check
`make ps3`, then one locked autopilot run per new level (start from its menu item), screenshots every 15 s, and the log
lines above. Hand-play judgement of the freeze, scorch and hook goes to the user.

## Step 9: pictures
`docs/work-plan/levels/render_levels.cpp` draws formation and boss scenes of each level with the game's own code to
PNGs (`c++ … && ./render && sips -s format png`), so the art can be checked without the PS3.

## Done when
- `make test` passes, `make ps3` builds, `check_docs.py` reports no new errors.
- On the PS3: each new level plays from its menu item at 60 fps with its enemies, bosses and hazards.
