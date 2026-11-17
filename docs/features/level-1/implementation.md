# Level 1: implementation

**Updated:** 2026-11-17 · [Design](design.md) · [Testing](testing.md)

## Approach
`startWave` lengthens the wave-1 banner so `drawGame` can read `LEVEL 1` first. `updateGame` turns the wave-20 clear into `levelClear` + `clearTimer` instead of the next-wave pause, then sets `won` and `over` when the timer ends and someone has lives left. `drawGame` draws the banners and the panel title. main.cpp's game-over block saves the high score and stops the music for both endings.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/game.h` › `Game::levelClear, clearTimer, won` | level-clear state |
| `source/game.cpp` › `startWave` | the longer wave-1 banner |
| `source/game.cpp` › `updateGame` | wave-20 clear, banner timer, win |
| `source/game.cpp` › `drawGame` | `LEVEL 1` and `LEVEL 1 CLEAR` banners, panel title |
| `source/main.cpp` | join guard and the `level 1 clear at` / `level clear at` logs |

## Data and config
- **Config constants:** `LEVEL_LAST_WAVE`, `LEVEL_BANNER_TIME`, `LEVEL_CLEAR_TIME` in `game.cpp`.
- **Persistence:** none new; the high score is saved by the existing game-over block.

## Deviations from design
- None.

## History
- 2026-11-17: implemented (D-017).
