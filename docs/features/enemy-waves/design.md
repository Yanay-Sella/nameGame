# Enemy waves

**Status:** implemented · **Updated:** 2026-11-17 · [GDD](../../GDD.md) · [Implementation](implementation.md) · [Testing](testing.md)

Aliens fly in, line up in a formation at the top of the screen, sway side to side and drop shots; from wave 3 some break off and dive at the ship. Clearing a wave brings a harder one, and every 5th wave is a boss (see Bosses in the GDD index). This is the core loop's minutes.

## Rules
1. A wave starts with its aliens flying in from above the screen into formation slots; they can't shoot until they reach their slot.
2. The formation sways left and right as one and reverses at the screen edges.
3. Every alien has a chance per second to drop a shot straight down; shots are magenta and are the only magenta objects.
4. From wave 3, one alien at a time leaves formation and dives in a curve toward the ship, then exits the bottom and returns to its slot from the top.
5. A wave is cleared when every alien is dead; the next starts after a short pause with a "WAVE n" banner.
6. Alien types: green (1 HP), blue (2 HP, from wave 4), red armoured (4 HP, from wave 7).
7. Every 5th wave is a boss instead of a formation (the Bosses feature, M3).

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Formation | 8 columns × min(2 + wave/2, 5) rows | aliens | screen not too crowded at 5 rows [ASSUMPTION] |
| Alien size | 12 × 10 | px | see [pixel art](../pixel-art/design.md) |
| Sway speed | 20 + 2 × wave, max 50 | px/s | [ASSUMPTION] |
| Shot chance per alien | 0.02 + 0.005 × wave, max 0.08 | per s | wave 1 is calm, wave 10 is busy [ASSUMPTION] |
| Enemy shot speed | 70 | px/s | dodgeable once seen [ASSUMPTION] |
| Dive interval | 4 − 0.2 × wave, min 1.5 | s | [ASSUMPTION] |
| Score per alien | 100 / 200 / 400 | points (green / blue / red) | [ASSUMPTION] |
| Wave clear bonus | 1000 × wave | points | [ASSUMPTION] |
| Pause between waves | 2 | s | |
| Fly-in | from 150 px above the slot at 150 px/s; 0.06 s later per column, 0.12 s per row | | wave starts feel lively, not slow [ASSUMPTION] |
| Dive | 90 px/s down, steering 120 px/s² toward the ship, at most 70 px/s sideways | | dodgeable but threatening [ASSUMPTION] |
| Return to formation | 100 | px/s | [ASSUMPTION] |

## Edge cases
- The player dies mid-wave: the wave continues; diving aliens return to formation during respawn.
- A rocket kills the last alien: the wave clears normally.
- Wave count past 5 rows: only the speeds and shot chances keep rising.

## History
- 2026-11-17: designed; alien size set to 12 × 10 with the pixel art.
- 2026-11-17: implemented in M2.
