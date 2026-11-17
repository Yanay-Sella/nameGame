# Co-op: implementation

**Updated:** 2026-11-17 · [Design](design.md) · [Testing](testing.md)

## Approach
`main.cpp` reads both controllers every frame. A controller without a ship that presses X or Start (the `confirm` action) calls `joinGame`, which appends a `Player` built by `newPlayer` and gives it the respawn invincibility. Everything else in `game.cpp` already loops over `game.players` (D-007): movement, firing, hits, dive targets, game over. `drawGame` picks the ship sprite and the HUD side by player index.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/game.h` | `MAX_PLAYERS`, `joinGame` |
| `source/game.cpp` › `newPlayer`, `joinGame`, `placeShipAtStart` | a new player's run state and spawn point |
| `source/game.cpp` › `drawGame` | per-player ship sprite, HUD on each player's side, "2P PRESS START", co-op game over line |
| `source/sprites.h` › `SHIP2_A`, `SHIP2_B` | player 2's ship colours |
| `source/main.cpp` | reads `MAX_PLAYERS` controllers, the join press, restarts with the same player count |

## Data and config
- **Config constants:** `MAX_PLAYERS` = maximum players (`game.h`), `CO_OP_SPAWN_OFFSET` = player 2 spawn offset (`game.cpp`). Join invincibility reuses `INVINCIBLE_TIME`.
- **Persistence:** none.

## Deviations from design
- None.

## History
- 2026-11-17: implemented.
