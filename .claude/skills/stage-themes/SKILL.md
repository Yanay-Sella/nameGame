---
name: stage-themes
description: >
  Designs a themed NAME game level (10 waves plus a boss): world look, palette, background,
  enemy roster, ship skin and boss, kept readable at 320x180. Use when planning, drafting or
  reviewing a level or world (space, jungle, frozen, lego...), or when a background, palette or
  enemy might hide bullets. Not for wave pacing maths alone (`level-design`), writing the
  feature doc (`game-docs`) or hit feedback (`game-feel`).
---

# Stage themes (NAME game)

A level is a world: 10 waves, then its own boss, then the next world. Every level brings a theme, its
own enemies, a ship skin and a boss, each harder than the last. The look must never cost readability
(GDD pillar 1) or frame time (pillar 4). Pair this skill with `level-design` for the wave rhythm.

## When to use

- Drafting or reviewing a level brief: theme, palette, background, enemies, ship skin, boss.
- Checking a new palette, background layer or sprite for bullet visibility.
- Deciding what changes from one level to the next and what must stay the same.

## The level bundle (one brief per level)

Write it as `docs/features/levels/<slug>.md` (follow `game-docs`). Every brief answers:

1. **Theme and one-line mood**, such as "frozen: cold, quiet, brittle". The mood drives every choice below.
2. **Palette:** the 4 to 6 theme colours (see Palette rules).
3. **Background:** layers, scroll speeds, and any slow ambient motion (see Background rules).
4. **Enemy roster:** 3 types, each new in look and behaviour; which wave introduces each.
5. **Ship skin:** the player ship recoloured or re-drawn for the world (see Ship skin rules).
6. **Boss:** born from the theme, with attacks that echo the level's enemies.
7. **Wave table:** waves 1 to 10 with intensity, plus the boss, as a `level-design` beats list.

## Palette rules

- **Reserved colours never change between levels:** magenta (`m`) is only for enemy shots, pale
  yellow (`y`) only for player bullets, white (`w`) only for hit flashes. No theme may use them for scenery or enemies.
- **Keep scenery below the shots.** Background colours are darker and less saturated than every
  bullet colour; the brightest background pixel is no brighter than the grey stars (`s`).
- **Theme colours are a ramp, not a rainbow:** one hue family with 3 to 4 steps (dark, mid, light)
  plus one accent. Hue-shift the ramp a little (shadows cooler, lights warmer) so it doesn't look flat.
- **Each world gets its own enemy hue** that no scenery colour shares, so enemies pop off the
  background (jungle: green scenery means orange or red enemies).
- **Test at the real size:** drop the palette into a build, run `make shot`, and check that a
  magenta shot is findable in under a second on the busiest background.
- New colours are new characters in `paletteColor` (`source/sprites.h`); a theme overrides only
  its own background and enemy characters, never the reserved ones.

## Background rules

- Layers sit **behind** the action only; nothing scrolls in front of ships, enemies or shots.
- 2 to 4 layers, far to near, with the far layer slowest (the starfield already does 6, 14 and 30 px/s).
  Keep the pace calm; wave 1 of a new level should feel quieter than the boss before it.
- Draw into the CPU pixel buffer with cheap fills: no per-frame full-screen clears or alpha blending
  (the port has none); prefer a pre-drawn tile that scrolls by moving its start row.
- Add life with one slow ambient detail per world (drifting ice flakes, fireflies, a far planet),
  never with fast motion near the player.
- A background feature must not move at shot speed or look like a 2×4 bar.

## Enemy rules

- 3 enemy types per level; each differs in **silhouette** (12×10 grid), colour and behaviour,
  not only in colour.
- Give each type a theme-fitting behaviour, e.g. frozen enemies that shatter into extra slow shots,
  jungle swarms that fly in loose curves. Keep one behaviour per type so players can read it.
- Introduce one new type at a time, in a calm wave, before a tougher wave uses it (teach, then test).
- Difficulty keeps ramping across levels: level 2's wave 1 may be gentler than level 1's wave 10,
  but its wave 10 and boss must beat level 1's.

## Ship skin rules

- Same 16×14 footprint, same hitbox, same thruster position, so dodging never changes.
- Re-colour or re-draw within the ship's silhouette; keep a bright, distinct cockpit or core so the
  hitbox centre is easy to find.
- The ship must stay visible against the level's background: test it on the darkest and brightest scenery.
- The skin is cosmetic only; no stat changes.

## Boss rules

- The boss is made from the world (an ice colossus, a jungle beast, a brick fortress) and is
  larger than 32×20 only if the screen stays dodgeable.
- Reuse the level's enemy visuals in its parts, so the boss feels like the climax of what the
  player just fought.
- Attack patterns are the two existing families (aimed burst, fan) plus one new theme pattern
  introduced at that boss; all shots stay magenta.
- Each level's boss is stronger than the one before (HP, sway speed, attack interval), following the
  curve in `docs/features/bosses/design.md`.
- The boss health bar and its reward keep the same look in every world.

## Check it

1. **Reserved colours:** `grep` the new sprites and palette for `m`, `y` and `w` outside shots, bullets and flashes.
2. **Readability:** `make shot` on a wave 10 screen and on the boss; find the ship and a shot at a glance.
3. **Frame time:** the PS3 stress run (60 aliens, 100 bullets, 2 explosions) holds 60 fps with the background on.
4. **Pacing:** the wave table passes the `level-design` rise-and-rest check, and rests before the boss.
5. **Distinctness:** the theme is recognisable from a single screenshot with the HUD hidden.

## Gotchas

- A gorgeous background that hides magenta shots is a bug; cut it.
- Reusing the same silhouette in a new colour reads as one enemy, not a new type.
- A theme palette that shares a hue with its enemies makes enemies vanish into the scenery.
- Heavy parallax layers or large alpha fades will drop frames on the real console.
- Lego or toy worlds tempt bright primary colours; keep bright colours for enemies and pickups, not scenery.
- Changing the ship's hitbox with a skin silently changes difficulty.
- Do not design all five worlds at once; finish one level on the PS3, then reuse what worked.

## Related skills

- `level-design`: wave rhythm, teach-then-test, pacing check. Its Godot code is only a model; translate it to C++.
- `game-docs`: where the finished level brief goes and how its status is tracked.
- `game-feel`: hit and boss feedback tiers that every world shares.
- `name-plan`: turns an approved level brief into a work plan for implementation.
