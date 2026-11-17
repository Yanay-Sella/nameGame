# Lives and score: implementation

**Updated:** 2026-11-17 · [Design](design.md) · [Testing](testing.md)

## Approach
`updateGame` checks enemy shots and alien bodies against each living ship's hitbox; `killPlayer` takes a life and starts the respawn timer. `updatePlayer` brings the ship back blinking when the timer runs out and a life is left. All points go through `addScore`, which caps the score and grants extra lives. When no ship is alive, no lives are left and the respawn time is over, the run ends; `main.cpp` stops the music and restarts on X or Start after the input lock.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/game.cpp` › `killPlayer`, `updatePlayer` | death, respawn, invincibility |
| `source/game.cpp` › `addScore` | score cap and extra lives |
| `source/game.cpp` › `updateGame` | hit tests, game-over check, next wave waits for a ship |
| `source/game.cpp` › `drawGame` | HUD and the game-over panel |
| `source/main.cpp` | music stop at game over, new run on X/Start |

## Data and config
- **Config constants:** `START_LIVES`, `MAX_LIVES`, `RESPAWN_DELAY`, `INVINCIBLE_TIME`, `BLINK_PERIOD`, `EXTRA_LIFE_EVERY`, `MAX_SCORE`, `SHIP_EXPLOSION_RADIUS` (`game.cpp`); `GAME_OVER_INPUT_LOCK` (`game.h`).
- **Persistence:** none (the saved high score is M3).

## Deviations from design
- The wave pause only counts down while a ship is alive, so after a death between waves the next wave comes the full remaining pause after the respawn.

## History
- 2026-11-17: implemented in M2.
