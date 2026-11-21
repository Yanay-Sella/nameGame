# Screens: implementation

**Updated:** 2026-11-21 · [Design](design.md) · [Testing](testing.md)

## Approach
`main.cpp` holds the `Scene` switch and combines menu actions from connected pads. The Classic submenu selects the variant, and `SCENE_CONTROLLERS` polls ready status before starting 1–4 Players. `drawMenu` draws the shared centred panel. `updateBackground` and `drawBackground` also serve the menus. The `Settings` struct (music, shake, flashing) is saved by stdio calls, so the Mac tests can build it.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/main.cpp` | scenes, menu input, music start and stop, settings load and save, the title high score |
| `source/settings.cpp` | `settingsLoad`, `settingsSave` for settings.txt |
| `source/input.cpp` | menu actions: D-pad, stick flick with hysteresis, Circle |
| `source/screen.cpp` › `drawMenu` | menu panel with title, items, cursor and hint |
| `source/game.cpp` › `updateBackground`, `drawBackground` | the starfield behind the menus |
| `source/sprites.h` › `fontGlyph` | the `>` cursor glyph |

## Data and config
- **Config constants:** stick flick thresholds 0.6 and 0.3 (`input.cpp`).
- **Persistence:** `settings.txt` next to EBOOT.BIN: `music=` (0 off, 1 Mizrahit, 2 Dubstep), `shake=` and `flash=` lines.

## Deviations from design
- None.

## History
- 2026-11-17: implemented.
- 2026-11-17: settings gained SHAKE and FLASHING; the title shows the high score.
- 2026-11-21: added the Classic submenu and controller lobby (D-055).
