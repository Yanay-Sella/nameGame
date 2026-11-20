# Phase 6: world 4 (The Ember Maw) and world 5 (The Sunken Halls)

Read first: [`../name-quest-plan.md`](../name-quest-plan.md), the **world authoring recipe** in
[`phase-4-world-1.md`](phase-4-world-1.md), then spec §5 (underwater row), §8 (worlds 4–5 rosters and traits), §9
(worlds 4–5), §10 (their chests), §12 (worlds 4–5 rows), §13 (their rows, "Areas that are special"), §14 (Calder's spot).
Phases 1–5 done. World 4 fully first, `make test`, then world 5.

**Art first (spec §22, recipe step 0 in phase 4):** for each world in this phase, build its `WorldArt` from its §13 art
brief and get the user's OK on its style frame before authoring its areas. Redraw its roster and bosses per §22
(side-view, outlined, 4-frame movement + tell + attack; bosses 40–64 px); sprite sizes named below are minimums.

## What the player gets
The Ember Maw: lava, fire jets, falling rocks and two climbs ahead of rising lava; the Imp and the Fire Lord.
The Sunken Halls: the whole world underwater (floaty jumps, slower falls), currents, a black trench, pitch-dark areas
lit only around the hero; the Angler in the dark and the Fisherman. Ser Calder, the drowned knight, waits in a dead end.

## Step 1: world 4 mechanics (`source/quest.cpp`, `source/quest_combat.cpp`, `source/quest_sprites.h`)
- Fire jets `f` (spec §9): cycle from `q.time` with a per-tile phase of `(tx % 3) * 0.8 s` so rows of jets ripple;
  warning sparks 0.5 s, then a 3-tile flame column (8 px wide) 0.8 s; SOUND_SIZZLE on start when on screen; 20 % trap.
- Rocks `R` (world 4 meaning of `R`): every 2.2 s, dust under the spawner for 0.5 s, then an 8 x 8 rock (author, `m`
  outline) falls at 220 px/s and breaks on the first solid tile (6 particles); 20 % trap.
- Rising lava (AREA_RISING_LAVA, tall areas): the lava surface starts at the area's lowest `~` row and rises 6 px/s from
  the hero's entry; tiles below the surface count as `~` for death and drawing; the hero leaving (any exit) or dying
  resets it. The top exit must be ≥ 25 s of climbing at 6 px/s ahead of the lava (author check: area height in px / 6 ≥
  the climb time you measure on the Mac + 8 s).
- Scorch (the Imp's lava drop): 3 s, walk x0.6, the hero tinted (`O` trim swap), SOUND_SIZZLE.
- PHOENIX revive (phase 2 trait) — check it now in a real world. Sprites: MAGMA CRAB 14x10, the rock, world 4 tiles,
  lava with a heat shimmer (alternate 2 rows), background.

## Step 2: world 4 bosses and areas
Imp and Fire Lord per spec §12. The Fire Lord's phase-2 lava rise: the arena's bottom floor row becomes deadly for 6 s
every 15 s after 2 s of bubbling (draw bubbles on it); its 3 platforms are one-way `-` rows authored safe above.
Author `source/quest_world4.cpp` (personality: inside a volcano's throat, an old forge city half-melted; the HEART OF
MAGMA behind a PHOENIX, GARRICK'S PICKAXE in a mine dead end). `make test`, `make quest-maps`.

## Step 3: world 5 mechanics
- Underwater physics for the whole world (spec §5 numbers): pass a `physicsWorld` (or the world index) into `moveHero` and
  pick the constants block; walk x0.85. The reachability test simulates with the same constants, so world 5 layouts get
  checked with the floaty jump (layout rule 8 is still the authoring target: design for 4 tiles).
- Bubbles trail the moving hero (like the shooter's deep sea: a 2 px `5` bubble every 0.3 s while moving, rising 20 px/s, 1.5 s).
- Currents `<` `>` (phase 1 push). The trench `~` is black-blue with drifting specks.
- Darkness (AREA_DARK): after drawing everything except the HUD, every pixel farther than 56 px from the hero's centre is
  set to `k`, except within 6 px of lit objects (checkpoints, the echo, enemy shots, the Angler's lure radius 40). Do it
  with one pass over the screen comparing squared distances to a short list of light centres (≤ 16); measure the frame
  time on the PS3 (≤ 3 ms for the pass).
- PUFFER inflate trait (phase 2) checked here. Sprites: IRON CRAB 16x14, world 5 tiles (sand, coral, sunken stone),
  background (light rays, rising bubbles), the Fisherman's hook line and net, the boot (reuse `BOOT`, magenta outline).

## Step 4: world 5 bosses and areas
Angler (the arena is AREA_DARK; its lure is a light source; phase 2 lure-out windows) and Fisherman (hook, live fish
using roster slot A stats, boot, net; phase 2 two hooks) per spec §12. Author `source/quest_world5.cpp` (personality:
the drowned halls of a sunken kingdom; Calder's dead end is a flooded chapel). Chests: spec §10 world 5.

## Step 5: tests
Worlds 4 and 5 in the world tests. Plus `fireJetsRippleAndBurn`, `rocksFallOnTheirCycle`, `risingLavaResetsOnLeave`,
`risingLavaAreasCanBeOutclimbed` (a scripted climb of each AREA_RISING_LAVA area along its main route reaches the top exit
before the lava; write the route as a button script per area in the test), `underwaterJumpIsHigherAndFallIsSlower`,
`darknessHidesFarPixels`, `hookLiftsAndDrops`, `scorchSlows`.

## Step 6: story touches
Ilsa's and Lio's lines after worlds 4 and 5; Calder's lines (stage 1 needs THE STRANGER'S LETTER; author his reading
page; `// phase 7` for the effect); Garrick's pickaxe line is phase 7.

## Step 7: docs
design.md: worlds 4–5; testing.md: checks below; GDD change log: the next version.

## Step 8: Mac self-check and PS3 check
- `make quest-maps` for both worlds; play both rising-lava areas and both bosses on the Mac at the reference level.
- PS3: dev saves for world 4 area 19 and world 5's darkest band-2 area and area 19. Log the fps and worst work while the
  darkness pass runs (must stay ≥ 59.5 fps, worst work < 12 ms). Ask the user to playtest a rising-lava climb and the
  Angler and to say whether the dark felt fair.

## Done when
- `make test` passes, `make ps3` builds, `check_docs.py` is clean.
- On the PS3: worlds 4 and 5 can be played through at 60 fps, including the dark areas.
