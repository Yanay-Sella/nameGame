# Glacier: implementation

**Updated:** 2026-11-18 · [Design](design.md) · [Testing](testing.md)

## Approach
Level 3 reads the shared per-level tables in `game.cpp` (offset 8, heavies every 6, +5 px/s shots, boss table). The ship's `vx, vy` ease toward the stick's speed (`ICE_GRIP`) only when `game.level == 3`. Snowballs, ice blocks and icicles are `Thrown` kinds; icicles hang with `delay`. The yeti's sneeze is a `windupTimer` (shared with the gorilla's pound) and `say` text. A frozen ship has `frozenTimer` and is drawn with `ICE_CUBE`; `fireWasHeld` finds fresh R1 presses.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/sprites.h` | palette characters and sprites (art source: `docs/work-plan/levels/levels_art.py`) |
| `source/game.h` | `Player`, `Boss` and `Thrown` fields, boss types, sounds |
| `source/game.cpp` › `updatePlayer` | ice grip, freeze and R1 mash |
| `source/game.cpp` › `updateBoss` | yeti cycle, golem roll, sneeze, icicle rain |
| `source/game.cpp` › `buildGlacier`, `drawBackground` | the background and snowfall |
| `source/audio.cpp` | the level's sounds |
| `source/main.cpp` | menu item and logs |
| `docs/work-plan/levels/render_levels.cpp` | draws the level's scenes on the Mac |

## Data and config
- **Config constants:** the per-level tables and the level's block in `game.cpp`.
- **Persistence:** none new.

## Deviations from design
- None.

## History
- 2026-11-18: implemented (D-024 to D-027).
