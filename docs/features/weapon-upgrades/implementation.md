# Weapon upgrades: implementation

**Updated:** 2026-11-18 · [Design](design.md) · [Testing](testing.md)

## Approach
`Player::weaponLevel` (1–4) picks the volley in `updatePlayer`: 1, 2 or 3 bullets, level 4 on the faster interval. `Shot` gained `vx` so spread bullets (and later boss shots) move sideways. A kill makes one roll: the lowest 2 % drops a rocket pickup, the next 1 % an upgrade pickup (`Pickup::type`). Collecting an upgrade adds to `Player::upgradeParts`; reaching `weaponLevel` parts raises the level. `upgradeTimer` and `upgradeLevelUp` drive the pop-up over the ship and the HUD gauge in `drawGame`. Death lowers the level and clears the parts in `killPlayer`.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/game.cpp` › `updatePlayer` | volley per level, fire interval |
| `source/game.cpp` › `damageAlien` | the single drop roll |
| `source/game.cpp` › `updateGame` | `vx` movement, pickup collection by type |
| `source/game.cpp` › `killPlayer` | level and parts loss |
| `source/game.cpp` › `drawGame` | pop-up text and HUD gauge |
| `source/sprites.h` › `UPGRADE_PICKUP` | pickup icon |
| `source/main.cpp` | `weapon n` in the 5 s log line |

## Data and config
- **Config constants:** `MAX_WEAPON_LEVEL`, `FAST_FIRE_INTERVAL`, `DOUBLE_OFFSET`, `SPREAD_SPEED`, `UPGRADE_CHANCE`, `UPGRADE_MAX_POINTS`, `UPGRADE_TEXT_TIME`, `LEVEL_UP_PARTICLES` (`game.cpp`).
- **Persistence:** none.

## Deviations from design
- None.

## History
- 2026-11-17: implemented in M3.
- 2026-11-18: parts and gauge (D-033).
