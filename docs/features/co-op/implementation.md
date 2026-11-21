# Co-op: implementation

**Updated:** 2026-11-21 · [Design](design.md) · [Testing](testing.md)

## Approach
`input.cpp` polls PS3 port status through PSL1GHT and produces `Actions` per port. `main.cpp` binds each player to a port for the run. A confirm from a spare port calls `joinGame`. `Game::variant` selects the party limit, dimensions, wave setup and pickup chance. The simulation and co-op UI use `game.players` for either variant.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/input.cpp`, `source/input.h` | live port status, held controls, presses and rumble |
| `source/main.cpp` | Classic submenu, controller lobby, port assignments, drop-in, disconnect pause and replay |
| `source/game.h`, `source/game.cpp` | party limits, start and join, variant wave balance, arena bounds, scores, role combat and HUD |
| `source/sprites.h` | Tank, Assassin and Mage hulls, translucent Assassin, heavy rocket and piercing orb |
| `source/formations.cpp` | traced slot layout with the variant's row width and arena margins |
| `source/screen.cpp`, `source/screen.h` | 384 × 216 frame buffer for 1–4 Players |
| `host/input_keyboard.cpp` | Mac scripts can send `pN.button`, `pN.connect` and `pN.disconnect` |

## Data and config
- `CONTROLLER_PORTS` covers the seven PS3 ports. The lobby can assign up to `MAX_PLAYERS`. `STANDARD_PLAYERS` caps Classic 1–2 Players and the Level Picker.
- `EXPANDED_COLUMNS`, `SECOND_EXTRA_HEAVY_WAVE`, `EXPANDED_HP_SCALE` and `EXPANDED_UPGRADE_CHANCE` hold the expanded wave tuning. `startWave` rounds HP up based on the party size at spawn.
- Each variant has its own high score. See [High Score](../high-score/design.md).
- `SCENE_ROLES` stages picks by port until the whole party is ready. `startGame` applies the picks at a three- or four-player start. `joinGame` applies the existing party's picks when the third player joins and applies only the newcomer's pick when the fourth joins.
- `ROLE_SPEED`, `ROLE_FIRE_INTERVAL` and `ROLE_BULLET_DAMAGE` tune R1 and movement. Enemy HP supports fractions from role projectiles. Each Mage orb stores its own hit history and damage falloff. Fighter keeps the existing Jungle shot damage calculation.

## Deviations from design
- None.

## History
- 2026-11-17: implemented.
- 2026-11-21: added the 1–4 player variant and port-based input (D-055).
- 2026-11-21: added the role picker, ship art and role combat (D-057).
