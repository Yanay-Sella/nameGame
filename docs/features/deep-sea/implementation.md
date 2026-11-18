# Deep sea: implementation

**Updated:** 2026-11-18 · [Design](design.md) · [Testing](testing.md)

## Approach
Level 5 reads the shared per-level tables (offset 14, heavies every 4, +15 px/s shots). `updatePlayer` adds the current to the ship's target speed. Bubbles wobble by their `age`; a big bubble pops into 8 by index in `updateGame`. The hook is a small state machine on the boss (`hookState`, `hookX/Y`, `hookPlayer`) in `updateHook`; a hooked ship has `hookedTimer` and rises to `hookLiftTo`. The boot is a `Thrown` kind.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/sprites.h` | palette characters and sprites (art source: `docs/work-plan/levels/levels_art.py`) |
| `source/game.h` | `Player`, `Boss` and `Thrown` fields, boss types, sounds |
| `source/game.cpp` › `updateHook` | cast, catch, hold, reel, boot |
| `source/game.cpp` › `updateBoss`, `updateGame` | angler cycle, fisherman roll, bubble wobble and pop |
| `source/game.cpp` › `buildSea`, `drawBackground` | the background and drifting bubbles |
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
