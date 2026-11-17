# Weapon upgrades

**Status:** implemented · **Updated:** 2026-11-17 · [GDD](../../GDD.md)

Pickups upgrade the ship's bullets through four levels, so the gun grows from a single stream to a fast spread (pillar 3: arcade power-up). Dying costs one level, so a good run is rewarded and a bad moment hurts without resetting everything.

## Rules
1. Each player has a weapon level 1–4, starting at 1 (also when joining in co-op).
2. Level 1 single: one bullet from the nose. Level 2 double: two straight bullets 3 px left and right of the centre. Level 3 spread: a centre bullet plus two angled ones (sideways ±70 px/s). Level 4 fast spread: level 3 fired at the faster interval. All bullets fly up at 320 px/s and do 1 damage.
3. A killed alien drops an upgrade pickup with 3 % chance. One roll per kill: below 2 % a rocket pickup, below 5 % an upgrade pickup, so they never drop together. It falls like the rocket pickup.
4. Touching an upgrade pickup raises the level by 1; at level 4 it gives 1,000 points instead.
5. Dying loses one level, never below 1.
6. The level is shown by the bullets themselves; there is no HUD element.

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Levels | 1–4 | | each level looks and feels distinct |
| Fire interval, levels 1–3 | 0.12 | s | existing |
| Fire interval, level 4 | 0.08 | s | strong but not buzzing |
| Double side offset | 3 | px | |
| Spread sideways speed | 70 | px/s | covers about a third of the formation |
| Upgrade drop chance | 3 | % per kill | level 4 reached in about 4–5 waves |
| Points at max level | 1,000 | points | |
| Pickup fall speed | 30 | px/s | same as the rocket pickup |

## Edge cases
- Two players touch the same pickup on one step: the first player in the array gets it, like rocket pickups.
- A pickup can't be collected by a dead player: the ship isn't there.
- Level 1 death: stays level 1.

## History
- 2026-11-17: designed.
- 2026-11-17: implemented in M3.
- 2026-11-17: numbers confirmed in single-player and co-op playtests.
