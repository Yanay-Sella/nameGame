# Game feel: implementation

**Updated:** 2026-11-17 · [Design](design.md) · [Testing](testing.md)

## Approach
`feedback(game, trauma, hitStop, flash)` in `game.cpp` is called at each event in the design table: it adds trauma (capped at 1), keeps the longest hit-stop and starts a screen flash unless one ran in the last 0.33 s. `updateGame` begins by advancing `feelTime`, decaying trauma and the flash, and, while `hitStop > 0`, counting it down, recording R2 presses and returning, so game time stands still. `drawGame` blends the play field to white for a flash and shifts it by the shake offset before the HUD is drawn; `main.cpp` copies the `shake=` and `flash=` settings into `Game` each frame.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/game.cpp` › `feedback` | the one reuse point for trauma, hit-stop and flash |
| `source/game.cpp` › `explode`, `damageBoss`, `killPlayer` | the events that shake and freeze |
| `source/game.cpp` › `updateGame` | decay, freeze, R2 recording during a stop, pickup particles |
| `source/game.cpp` › `drawGame` | flash blend, shake shift, then the HUD |
| `source/settings.cpp` | `shake=` (clamped 0..100) and `flash=` lines |
| `source/main.cpp` | the `SHAKE` and `FLASHING` settings items; copies settings into `Game` each frame |

## Data and config
- **Config constants:** `TRAUMA_DECAY`, `SHAKE_X`, `SHAKE_Y`, `FLASH_MIN_GAP` and the per-event trauma, stop and flash values (`game.cpp`).
- **Persistence:** `shake=<0..100>` and `flash=<0|1>` in settings.txt.

## Deviations from design
- After the first playtest, ordinary alien kills and the extra life no longer add trauma (the user didn't want shake while shooting); only explosions shake. The design table was updated to match.
- Existing tests that wait for a respawn now add the 0.12 s hit-stop of the death to their waits.

## History
- 2026-11-17: implemented in M3.
