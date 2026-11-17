# Ship controls: implementation

**Updated:** 2026-11-17 · [Design](design.md) · [Testing](testing.md)

## Approach
`input.cpp` turns each controller into named `Actions` (move, fire, rocket, pause, confirm, stress); gameplay never reads button numbers. `main.cpp` runs the game in fixed 1/60 s steps and `updatePlayer` in `game.cpp` applies the actions. Players are a `std::vector<Player>` with one controller each (GDD D-007); player 2 is appended when they join ([co-op](../co-op/implementation.md)).

## Code map
| File / module | Role in this feature |
|---|---|
| `source/input.h` | `Actions` reader API, `applyDeadzone`, `STICK_DEADZONE` |
| `source/input.cpp` | button numbers, opening controllers by count (the port sends no connect events), D-pad over stick |
| `source/game.cpp` › `updatePlayer` | movement inside the zone, the fire rhythm, bullets |
| `source/main.cpp` | fixed 60 Hz steps; a press counts once, in the first step of a frame |

## Data and config
- **Config constants:** `SHIP_SPEED` = ship speed, `FIRE_INTERVAL` = fire interval, `BULLET_SPEED` = bullet speed, `MOVE_ZONE_TOP` = top of the movement zone, `HITBOX`/`HITBOX_X`/`HITBOX_Y` = ship hitbox (all `game.cpp`); `STICK_DEADZONE` = stick deadzone (`input.h`).
- **Persistence:** none.

## Deviations from design
- The fire timer subtracts the interval instead of resetting, so the rhythm stays exactly 0.12 s at 60 Hz steps (resetting gave 0.133 s).
- Unplug: `inputRead` sets `Actions.connected`; `main.cpp` pauses the run while a joined player's controller is gone and blocks Resume.

## History
- 2026-11-17: implemented in M2.
