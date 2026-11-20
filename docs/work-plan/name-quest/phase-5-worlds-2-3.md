# Phase 5: world 2 (The Greendeep) and world 3 (The White Silence)

Read first: [`../name-quest-plan.md`](../name-quest-plan.md), the **world authoring recipe** in
[`phase-4-world-1.md`](phase-4-world-1.md), then spec §8 (worlds 2–3 rosters and traits), §9 (worlds 2–3), §10 (their
chests), §12 (worlds 2–3 rows), §13 (their rows), §14 (Garrick's world 2 spot, Ilsa/Lio lines). Phases 1–4 done.
Work world 2 fully (steps 1–3), run `make test`, then world 3 (steps 4–6). If the session gets long, write
`progress.md` after world 2 (master plan rule 9).

**Art first (spec §22, recipe step 0 in phase 4):** for each world in this phase, build its `WorldArt` from its §13 art
brief and get the user's OK on its style frame before authoring its areas. Redraw its roster and bosses per §22
(side-view, outlined, 4-frame movement + tell + attack; bosses 40–64 px); sprite sizes named below are minimums.

## What the player gets
Two more worlds of 20 areas, each with its own hazards, five enemies, a mini-boss and a boss: the Monkey King and the
Silverback in a jungle of vines, thorns and falling coconuts; the Yeti and the Glacier Titan in a frozen cave of ice
floors, crumbling ice, icicles and flipping wind. Garrick waits, hurt, in a Greendeep dead end.

## Step 1: world 2 mechanics (`source/quest.cpp`, `source/quest_combat.cpp`, `source/quest_sprites.h`)
- Coconuts `R` (spec §9 band 2): a palm-top object; when the hero's centre passes within 8 px horizontally below it, a
  coconut (`COCONUT` sprite with the spec §21 magenta outline) drops after 0.3 s at 200 px/s; trap damage 20 %; the palm regrows its coconut after 4 s.
- TREE MONKEY trait TRAIT_BANANA_ON_LAND: on landing it throws one banana (`BANANA_A/B` with the magenta outline) at
  the hero at 90 px/s.
- Peels (Monkey King) and stink mist (Silverback): reuse the shooter's `PEEL` and mist drawing; peel slide 0.5 s no
  control at the hero's speed or 100, whichever is more, in the facing direction; mist 3 % max HP per s while inside.
- Sprites: TREE MONKEY 12x12, world 2 tiles (mossy earth, vines `|` as green ropes, thorns `^` as brown spikes, water
  `~` dark green), world 2 background (jungle layers and fireflies, spec §13).

## Step 2: world 2 bosses (`source/quest_bosses.cpp`)
Monkey King and Silverback, spec §12 world 2 row exactly. The Monkey King's two trees are `X` anchors 0–1 (one-way
platforms 6 tiles up). The Silverback's shockwave is a ground wave the hero must jump (12 px tall).

## Step 3: author world 2 (new `source/quest_world2.cpp`)
Remove the phase-4 placeholder. Personality: a drowned jungle swallowing an old temple; vines replace ladders; light
through leaves. Garrick (`N`, NPC_GARRICK) sits in a dead end deep in band 1, near a landmark (a fallen statue's head).
Chests: spec §10 world 2. The gate is a wall of thorny roots. Then `make test` and `make quest-maps` for world 2.

## Step 4: world 3 mechanics
- Ice floors `i` (phase 1), crumbling `c` (phase 1 timing; draw cracks spreading during the 0.6 s), icicles `Y` (spec §9:
  `ICICLE` sprite; reset on rest/death via `resetHazards`), wind zones `<`/`>` in band 2 that flip every 3 s (zones in an
  area flip together; snow streaks show the direction; 40 px/s here, not the currents' 50).
- Frozen (Yeti's ice block): 1.2 s, no control, X or R1 presses cut 0.15 s each, the `ICE_CUBE` drawn over the hero,
  SOUND_FREEZE, SOUND_CRACK when it breaks.
- Sprites: YETI CUB 12x12, ICE KNIGHT 16x16 (+ wind-up), world 3 tiles (snow-topped rock, clear ice with a highlight,
  cracked ice), freezing water `~` (pale `i` shimmer on `q`), background (spec §13).

## Step 5: world 3 bosses
Yeti and Glacier Titan, spec §12 world 3 row. The Titan's phase 2 turns the middle 10 tiles of the arena floor into
`c` tiles in the live `AreaState` (restored when the fight resets).

## Step 6: author world 3 (new `source/quest_world3.cpp`)
Personality: silence and cold; a cave system under a glacier, a frozen waterfall, an ice cathedral. The FROZEN FEATHER
chest sits behind an ICE KNIGHT in a dead end. Chests: spec §10 world 3. The gate is an ice wall.

## Step 7: tests
World 2 and 3 in `everythingIsReachable`, `gateGuardsTheSecondHalf`, `worldLayoutRulesHold`, `chestsMatchTheWorldList`.
Plus `coconutDropsWhenHeroPassesUnder`, `treeMonkeyThrowsOnLanding`, `peelSlidesTheHero`, `mistHurtsOverTime`,
`icicleFallsOnceUntilRest`, `windFlipsEveryThreeSeconds`, `frozenHeroBreaksFreeSoonerByMashing`,
`titanPhaseTwoCracksTheFloor`, `silverbackWaveCanBeJumped` (a hero jumping at the right moment clears it).

## Step 8: story touches
Ilsa's and Lio's lines after worlds 2 and 3 (author). Garrick's world 2 lines: without a RED POTION
`I... I CAN'T WALK. DO YOU HAVE A RED POTION?`; with one, a thank-you page (author). Phase 7 adds the stage effect;
leave `// phase 7` there.

## Step 9: docs
design.md: worlds 2–3; testing.md: PS3 checks below plus a user playtest checklist per world. GDD change log: the next version.

## Step 10: Mac self-check and PS3 check
- `make quest-maps` and read all 80 images. Play each boss on the Mac at its world's reference level and gear.
- PS3: dev saves (phase 4 step 10 method) at world 2 area 19 and world 3 area 19; fight each boss in god mode, screenshot
  one phase-2 moment each. Expect the boss log lines and fps ≥ 59. Ask the user to playtest world 2 (and world 3 if they
  have time) and record their notes.

## Done when
- `make test` passes, `make ps3` builds, `check_docs.py` is clean.
- On the PS3: worlds 2 and 3 can be played through; each boss lights the next world's entry.
