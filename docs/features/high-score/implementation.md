# High score save: implementation

**Updated:** 2026-11-17 · [Design](design.md) · [Testing](testing.md)

## Approach
`main.cpp` loads `highscore.txt` at boot (next to the settings load) and keeps the best in a local `highScore`. When a run ends (`game.over && !wasOver`) it takes the best player score; if that beats the stored one it saves the file, sets `game.newHighScore` and logs the result. Each frame it copies the stored value into `game.highScore` so `startGame` resetting `Game` doesn't matter. `drawGame` shows `HI` on the HUD and `NEW HIGH SCORE` on the game-over panel.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/settings.cpp` › `highScoreLoad`, `highScoreSave` | read and write `best=<n>`, clamped to 0..`MAX_SCORE` |
| `source/main.cpp` | boot load, save at game over, copy into `Game` |
| `source/game.cpp` › `drawGame` | `HI` line at y = 9, panel text |
| `source/game.h` | `Game::highScore`, `Game::newHighScore`, `MAX_SCORE` (moved here from `game.cpp`) |

## Data and config
- **Config constants:** `MAX_SCORE` (`game.h`).
- **Persistence:** `highscore.txt` next to EBOOT.BIN, one line `best=<n>`.

## Deviations from design
- The game-over panel grew from 54 to 62 px tall to fit the extra line.

## History
- 2026-11-17: implemented in M3.
