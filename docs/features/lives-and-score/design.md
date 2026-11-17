# Lives and score

**Status:** implemented · **Updated:** 2026-11-17 · [GDD](../../GDD.md) · [Implementation](implementation.md) · [Testing](testing.md)

One hit kills, so every death comes from a visible shot or alien; three lives make a run last, and the score is what you take into the next one. It serves pillar 3 (arcade, not a campaign).

## Rules
1. A run starts with 3 lives, 0 points and 3 rockets.
2. An enemy shot or an alien's body touching the ship's hitbox destroys the ship (large explosion) and costs a life.
3. After a death the ship returns at the bottom centre after the respawn delay, then blinks and can't be hit for the invincibility time; it can move and shoot while blinking.
4. Dying with no lives left ends the run: "GAME OVER" with the final score and wave. After the input lock, X or Start starts a new run (the M3 screens replace this).
5. Points come from killing aliens (see [enemy waves](../enemy-waves/design.md)) and wave clear bonuses.
6. Every 50,000 points gives an extra life, up to the maximum; crossing two thresholds at once gives both.
7. The HUD shows the wave at the top centre and, on each player's side (player 1 left, player 2 right), the score at the top, one ship icon per spare life below it and the rockets at the bottom ([co-op](../co-op/design.md)).

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Starting lives | 3 | lives | [ASSUMPTION] |
| Respawn delay | 1.5 | s | long enough to register the death [ASSUMPTION] |
| Invincibility after respawn | 2 | s | enough to get clear of shots [ASSUMPTION] |
| Blink period | 0.1 | s | |
| Extra life every | 50,000 | points | about one per 6–8 waves [ASSUMPTION] |
| Maximum lives | 9 | lives | |
| Game over input lock | 1.5 | s | no accidental restart while still holding X |
| Maximum score shown | 9,999,999 | points | |
| Ship explosion radius | 24 | px | [ASSUMPTION] |

## Edge cases
- Death while a rocket is in flight: the rocket keeps flying and its kills still score.
- Death during the pause between waves: the pause only counts down while a ship is alive.
- Shot and alien body hit in the same frame: one death only.
- Pause during the respawn delay or invincibility: both timers freeze.

## History
- 2026-11-17: designed.
- 2026-11-17: HUD moved to per-player sides for co-op (D-009); spare lives now sit under the score instead of the top right.
- 2026-11-17: implemented in M2.
