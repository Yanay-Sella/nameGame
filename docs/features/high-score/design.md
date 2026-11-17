# High score save

**Status:** implemented · **Updated:** 2026-11-17 · [GDD](../../GDD.md)

The best score is written to a small file on the PS3's hard drive, so it survives quitting and rebooting. It gives a run something to beat (pillar 3: a score and a high score).

## Rules
1. The high score is the best single-player score of any finished run; in co-op, the better of the two players' scores.
2. It is loaded at boot and written at game over, only when beaten. A run left through RETURN TO MENU doesn't count (that path is screen code).
3. The HUD shows `HI 0012345` centred at y = 9, under `WAVE n`. The game-over panel adds `NEW HIGH SCORE` in yellow when the run beat it.
4. Missing or unreadable file: high score 0. A save failure keeps the value in memory and logs `SAVE FAILED`.
5. The title shows the high score once the screens feature adds it; this feature doesn't touch the menu.

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| File | `highscore.txt`, one line `best=<n>`, next to EBOOT.BIN | | survives reinstalling the pkg [ASSUMPTION] |
| Range | 0 to 9,999,999 | points | loaded values are clamped |
| HUD position | centred, y = 9 | px | doesn't touch the score, lives or boss bar [ASSUMPTION] |

## Edge cases
- Garbage, negative or huge value in the file: treated as 0 or the maximum.
- A run that ties the stored score isn't a new high score.
- RETURN TO MENU mid-run: the score is not saved, even if it beat the stored one.
- The file is its own file, not settings.txt, so the settings format stays the screens feature's.

## History
- 2026-11-17: designed.
- 2026-11-17: implemented in M3.
