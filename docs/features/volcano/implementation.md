# Volcano: implementation

**Updated:** 2026-11-18 · [Design](design.md) · [Testing](testing.md)

## Approach
Level 4 reads the shared per-level tables (offset 11, heavies every 5, +10 px/s shots). Fireballs and lava drops are `Thrown` kinds. A lava drop sets the ship's `scorchTimer`: `updatePlayer` slows it, `updateGame` doubles its bullets' damage at hit time and `drawGame` draws fire bolts and `HOT!`. The eruption reuses the mage's `ballX/ballTimer/beamTimer` with its own width and drawing.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/sprites.h` | palette characters and sprites (art source: `docs/work-plan/levels/levels_art.py`) |
| `source/game.h` | `Player`, `Boss` and `Thrown` fields, boss types, sounds |
| `source/game.cpp` › `updatePlayer`, `updateGame` | scorch speed and damage |
| `source/game.cpp` › `updateBoss` | imp cycle, fire lord roll, lava rain, eruption |
| `source/game.cpp` › `buildVolcano`, `drawBackground` | the background and embers |
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
