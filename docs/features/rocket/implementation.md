# Rocket: implementation

**Updated:** 2026-11-17 · [Design](design.md) · [Testing](testing.md)

## Approach
R2 sets `Player::rocketPressedAt` (game time); `updatePlayer` launches when the cooldown is over and the press is younger than the buffer, or plays the empty click. Each step a rocket speeds up and climbs; touching a visible alien or the top of the screen calls `explode`, which damages every alien in range once. Kills may drop a `Pickup` that falls and is collected by touching the ship.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/game.cpp` › `updatePlayer` | buffer, cooldown, ammo, launch, empty click |
| `source/game.cpp` › `updateGame` | rocket flight, hit test, pickups falling and collected |
| `source/game.cpp` › `explode`, `damageAlien` | blast damage, kills, pickup drops |
| `source/game.cpp` › `drawExplosion`, `drawGame` | blast picture, rockets, pickups, HUD rocket icons |
| `source/main.cpp` | pausing clears the buffered press |

## Data and config
- **Config constants:** `START_ROCKETS`, `MAX_ROCKETS`, `ROCKET_COOLDOWN`, `ROCKET_BUFFER`, `ROCKET_START_SPEED`, `ROCKET_TOP_SPEED`, `ROCKET_ACCEL`, `EXPLOSION_RADIUS`, `EXPLOSION_DAMAGE`, `PICKUP_CHANCE`, `PICKUP_FALL_SPEED`, `EXPLOSION_GROW`, `EXPLOSION_FADE` (all `game.cpp`, same names as the design numbers).
- **Persistence:** none.

## Deviations from design
- An alien is in the blast when its centre is within the radius plus half an alien's width (6 px), so the edge of the circle you see counts.

## History
- 2026-11-17: implemented in M2.
