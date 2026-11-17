# Screens: implementation

**Updated:** 2026-11-17 · [Design](design.md) · [Testing](testing.md)

## Approach
`main.cpp` holds a `Scene` (main menu, settings, playing, paused) and one switch per frame that handles the menus' input and the play loop. Menus read the OR of every controller's actions. `drawMenu` draws the shared centred panel; the starfield is split out of the game into `updateBackground` and `drawBackground` so the menus reuse it. The `Settings` struct (music, shake, flashing) is saved by stdio calls, so the Mac tests can build it. The settings scene has four items; shake steps by 25 with wrap-around.

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
- **Persistence:** `settings.txt` next to EBOOT.BIN: `music=`, `shake=` and `flash=` lines.

## Deviations from design
- None.

## History
- 2026-11-17: implemented.
- 2026-11-17: settings gained SHAKE and FLASHING; the title shows the high score.
