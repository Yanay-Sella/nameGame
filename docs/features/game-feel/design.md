# Game feel

**Status:** implemented · **Updated:** 2026-11-17 · [GDD](../../GDD.md)

Shake, hit-stop, flashes and particles make impacts read by size (the [game-feel](../../../.claude/skills/game-feel/SKILL.md) tiers). Held R1 gets no feedback, so spraying never buzzes; the rocket, the boss and the player's death hit hardest (pillar 2).

## Rules
| Event | Tier | Trauma | Hit-stop | Particles | Screen flash |
|---|---|---|---|---|---|
| bullet hits alien or boss, no kill | small | 0 | 0 | the existing sprite flash | no |
| alien dies | medium | 0 | 0 | 8 (existing) | no |
| pickup collected | small | 0 | 0 | 4 | no |
| extra life | medium | 0 | 0 | 0 | no |
| rocket explosion | large | 0.6 | 0.06 s | 20 | 0.05 s |
| boss dies | large | 1.0 | 0.15 s | 40 | 0.08 s |
| player dies | large | 0.8 | 0.12 s | existing | 0.08 s |

1. Only explosions shake the screen (rocket blasts, the boss's death, the player's death); shooting and ordinary kills never do (playtest 2026-11-17: shake from R1 kills was unwanted). Shooting gets no feedback. Trauma adds and caps at 1, and decays 1.5 per second of real time (also during hit-stop).
2. Shake offset = trauma² · strength · (4 px x, 3 px y) · smooth sines at two unrelated rates, never random per frame.
3. Hit-stop freezes the game step; overlapping stops take the longest remaining, not the sum. R2 presses during a stop are still recorded. Music and menus never stop.
4. Shake moves only the picture of the play field: the HUD stays still and collisions never move.
5. A screen flash draws the play field halfway to white for its duration. At most one every 0.33 s; later ones are dropped. Reduce flashing turns screen flashes off; the small sprite hit flashes stay.
6. Options: shake 0, 25, 50, 75 or 100 % (default 100) and flashing NORMAL or REDUCED (default NORMAL), saved in settings.txt as `shake=` and `flash=` lines. They are items in the settings screen.

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Trauma decay | 1.5 | per s | shake gone in under 0.7 s |
| Max shake offset | 4 × 3 | px | strong but readable at 320×180 |
| Flash strength | 50 % to white | | |
| Flash rate limit | 1 per 0.33 s | | keeps large flashes at 3 per second at most |
| Event values | see the table | | rocket blasts, deaths and the boss death feel punchy, not nauseating |

## Edge cases
- No hit-stop on ordinary kills, a deliberate deviation from the medium tier (D-011): spread fire kills several aliens a second and would stutter.
- Events can't happen during a freeze, so overlapping stops only come from events in the same step.
- Pausing doesn't run the game step, so shake and stops simply wait.

## History
- 2026-11-17: designed.
- 2026-11-17: implemented in M3.
- 2026-11-17: numbers confirmed in single-player and co-op playtests.
