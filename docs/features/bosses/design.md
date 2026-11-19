# Bosses

**Status:** implemented · **Updated:** 2026-11-18 · [GDD](../../GDD.md)

Every 5th wave a boss replaces the formation (GDD D-002), and each boss wave is different (D-019). It is the rocket's moment (pillar 2): a wall of HP that bullets chip at and a rocket blast really hurts, while the attacks keep the player dodging.

## Rules
The jungle has its own line-up (monkeys and gorillas): see [Jungle](../jungle/design.md). So do the [glacier](../glacier/design.md) (yeti, ice golem), the [volcano](../volcano/design.md) (fire imp, fire lord) and the [deep sea](../deep-sea/design.md) (anglerfish, fisherman fish). Every level's boss HP, sway, attack interval and reward now come from one per-level table in `game.cpp` (D-025); level 1's values are unchanged.

1. Waves 5, 10, 15… are bosses alone, with no formation. The banner reads `BOSS` instead of `WAVE n`.
2. The line-up: wave 5 one classic (100 HP), wave 10 two classics (100 HP each), wave 15 one mage (200 HP), wave 20 two mages (200 HP each, the level boss). Bosses fly in from above to y = 20, then sway at 40 + 10·(n−1) px/s (n = wave / 5, capped at 80), turning at the ends of their range.
3. One boss sways the full width. Twins each sway in their own half (left x 0…128, right x 160…288), start in the middle of it and move apart; both attack at the full rate, the second twin's first attack at 1.5 × the interval.
4. A classic alternates every max(2.5 − 0.3·(n−1), 1.2) s: an aimed burst of 3 shots 0.15 s apart at the nearest living ship (90 px/s), then a fan of 5 shots straight down, spread −60…+60 px/s sideways (70 px/s down). Nothing is cast while entering or when no ship is alive.
5. A mage is the classic alien under a tall starry wizard's hat, holding a wand with a star tip. It attacks on the same interval, but each time rolls 35 % aimed burst, 35 % fan, 30 % power ball (the burst and fan are the classic's), and the next gap is the interval × a random 0.6…1.4. If it rolls the power ball while its own ball or beam is active, it fans instead.
6. Power ball: appears at the star of the mage's wand where it is at that moment and stays there while the mage sways on. It charges 1.5 s (a magenta circle growing from radius 1 to 5, with a dotted guide line down to the bottom), then fires: the ball stays at the beam's base and a 24 px wide magenta column (white core 8 px) shoots down to the bottom in 0.1 s, lasts 0.8 s with the odd spark, then breaks into shards. It kicks the screen when it fires and rumbles while it's on; the explosion sound plays. A ship touching the grown part of the beam dies (respawn invincibility protects it).
7. Bullets do 1 damage, a rocket blast 20 (the rocket explodes on touching a boss; the blast hits it when the circle overlaps its rectangle). Each hit flashes it white for 0.06 s.
8. Touching a boss's body kills the ship.
9. One health bar 120×3 px sits under the top line at y = 16: red fill on dark red, showing the summed HP of the wave's bosses.
10. Each boss kill gives 5,000·n points to the killer, a large explosion, a guaranteed upgrade pickup and a rocket pickup; the normal wave-clear bonus and pause come once, when the last boss dies (the last wave-20 mage ends level 1 instead: [Level 1](../level-1/design.md)).

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Wave 5 | 1 classic, 100 HP, sway 40, attack every 2.5 s | px/s | as before |
| Wave 10 | 2 classics, 100 HP each, own halves, sway 50, each every 2.2 s | | two streams of fire stay dodgeable |
| Wave 15 | 1 mage, 200 HP, sway 60, attack every 1.1–2.7 s | | the ball is readable |
| Wave 20 | 2 mages, 200 HP each, own halves, sway 70, each every 1.0–2.2 s | | beatable but threatening |
| Boss size | 32×20 | px | big, but the fan has gaps to fly through |
| Entry speed, stop height | 150, y = 20 | px/s | same as aliens' entry |
| Burst | 3 shots, 0.15 s apart, 90 px/s | | dodgeable by moving |
| Fan | 5 shots, sideways −60…+60, down 70 | px/s | |
| Mage rolls | 35 % burst, 35 % fan, 30 % power ball | | |
| Mage gaps | interval × 0.6–1.4 | s | |
| Power ball | at the wand's star (30, 8), charges 1.5 s, radius 1→5 | | |
| Beam | 24 px (core 8) × full height, grows in 0.1 s, 0.8 s, trauma 0.7 then 0.45 | | clear to see, easy to leave |
| Bullet / blast damage | 1 / 20 | HP | |
| Reward | 5,000·n per boss | points | |

## Edge cases
- Both ships dead or waiting to respawn: bosses cast nothing new and their attack timers stand still.
- A charging ball still fires its beam if every ship has died meanwhile.
- A dead mage's ball and beam vanish with it.
- A second ball is never cast while one is active: the mage fans instead.
- A boss dies while a burst is half fired: remaining burst shots are dropped; shots already in the air stay.
- Two blasts or bullets on one step: each applies; each death happens once.
- Each twin drops both pickups when it dies, so twin waves drop two of each.
- Sprite note: bosses are their own `Boss` struct, since the 12×10 alien size is used in every collision.

## History
- 2026-11-17: designed.
- 2026-11-17: implemented in M3.
- 2026-11-17: numbers confirmed in single-player and co-op playtests.
- 2026-11-17: boss line-up per wave: classic, twin classics, mage, twin mages (D-019).
- 2026-11-18: jungle level (D-020): monkey and gorilla line-up.
- 2026-11-18: levels 3-5 (D-024, D-025): yeti, golem, imp, fire lord, anglerfish, fisherman; per-level boss table.
- 2026-11-18: beam build-up, shards and shake (D-032).
