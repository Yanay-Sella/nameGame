# Weapon upgrades

**Status:** implemented · **Updated:** 2026-11-18 · [GDD](../../GDD.md)

Pickups upgrade the ship's bullets through four levels, so the gun grows from a single stream to a fast spread (pillar 3: arcade power-up). Dying costs one level, so a good run is rewarded and a bad moment hurts without resetting everything.

## Rules
1. Each player has a weapon level 1–4, starting at 1 (also when joining in co-op).
2. Level 1 single: one bullet from the nose. Level 2 double: two straight bullets 3 px left and right of the centre. Level 3 spread: a centre bullet plus two angled ones (sideways ±70 px/s). Level 4 fast spread: level 3 fired at the faster interval. All bullets fly up at 320 px/s and do 1 damage.
3. An alien kill rolls once for a pickup. The first 2% of the roll is a rocket pickup. The next 1% is a weapon upgrade in Classic 1–2 Players and the Level Picker, and the next 2% in Classic 1–4 Players. An upgrade falls like the rocket pickup. See [Bosses](../bosses/design.md) for guaranteed loot.
4. Upgrade pickups are parts: level n takes n parts to reach n + 1 (1 for level 2, 2 more for level 3, 3 more for level 4). At level 4 a pickup gives 1,000 points instead.
5. Dying loses one level, never below 1, and the parts collected toward the next level; if you held a level buff, the buff is lost instead and the weapon level stays.
6. Progress shows twice. A HUD gauge in the player's bottom corner, above the buffs: four rising bars (lit up to the level), then one pip per part the next level takes (filled orange when collected; MAX at level 4). And a pop-up over the ship that rises and blinks out in 0.9 s: "1/2" in orange for a part (the new pip blinks white), "POWER UP!" flashing yellow and white for a level, with a yellow burst of particles, the bars blinking and the extra-life jingle.

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Levels | 1–4 | | each level looks and feels distinct |
| Fire interval, levels 1–3 | 0.12 | s | existing |
| Fire interval, level 4 | 0.08 | s | strong but not buzzing |
| Double side offset | 3 | px | |
| Spread sideways speed | 70 | px/s | covers about a third of the formation |
| Upgrade drop chance | 1 in 1–2 Players and the Level Picker, 2 in 1–4 Players | % per kill | level 4 (6 parts) is a mid-level goal, helped by the bosses' drops |
| Parts per level | level n needs n | pickups | the third level-up feels earned |
| Pop-up time | 0.9 | s | readable without hiding shots |
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
- 2026-11-18: 1 % drops, level n takes n parts, HUD gauge and pop-up (D-033).
