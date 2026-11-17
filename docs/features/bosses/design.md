# Bosses

**Status:** implemented · **Updated:** 2026-11-17 · [GDD](../../GDD.md)

Every 5th wave a single big boss replaces the formation (GDD D-002). It is the rocket's moment (pillar 2): a wall of HP that bullets chip at and a rocket blast really hurts, while its two attack patterns keep the player dodging.

## Rules
1. Waves 5, 10, 15… are a boss alone, with no formation. The banner reads `BOSS` instead of `WAVE n`.
2. Appearance n = wave / 5. HP = 100 + 50·(n−1). The boss flies in from above to y = 20, then sways the full screen width at 40 + 10·(n−1) px/s (capped at 80), turning at the edges.
3. Attacks alternate every max(2.5 − 0.3·(n−1), 1.2) s: an aimed burst of 3 shots 0.15 s apart at the nearest living ship (90 px/s), then a fan of 5 shots straight down, spread −60…+60 px/s sideways (70 px/s down). It doesn't shoot while entering or when no ship is alive. Its shots are ordinary magenta enemy shots.
4. Bullets do 1 damage, a rocket blast 20 (the rocket explodes on touching it; the blast hits it when the circle overlaps its rectangle). Each hit flashes it white for 0.06 s.
5. Touching the boss's body kills the ship.
6. A health bar 120×3 px sits under the top line at y = 16: red fill on dark red.
7. Killing it gives 5,000·n points to the killer, a large explosion, a guaranteed upgrade pickup and a rocket pickup; then the normal wave-clear bonus and pause before the next wave.

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Boss size | 32×20 | px | big, but the fan has gaps to fly through |
| HP | 100 + 50·(n−1) | HP | wave 5 about 25 s of steady fire plus a rocket or two |
| Entry speed, stop height | 150, y = 20 | px/s | same as aliens' entry |
| Sway speed | 40 + 10·(n−1), max 80 | px/s | |
| Attack interval | max(2.5 − 0.3·(n−1), 1.2) | s | threatening, never constant fire |
| Burst | 3 shots, 0.15 s apart, 90 px/s | | dodgeable by moving |
| Fan | 5 shots, sideways −60…+60, down 70 | px/s | |
| Bullet / blast damage | 1 / 20 | HP | |
| Reward | 5,000·n | points | |

## Edge cases
- Both ships dead or waiting to respawn: the boss holds fire and its attack timer stands still.
- The boss dies while a burst is half fired: remaining burst shots are dropped; shots already in the air stay.
- Two blasts or bullets on one step: each applies; the death happens once.
- Sprite note: the boss is its own `Boss` struct, since the 12×10 alien size is used in every collision.

## History
- 2026-11-17: designed.
- 2026-11-17: implemented in M3.
- 2026-11-17: numbers confirmed in single-player and co-op playtests.
