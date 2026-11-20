# Screens

**Status:** implemented · **Updated:** 2026-11-18 · [GDD](../../GDD.md) · [Implementation](implementation.md) · [Testing](testing.md)

The game opens on a main menu with room for more game modes, and a settings screen holds the music switch. It serves pillar 3 (arcade): only the menus the game needs.

## Rules
1. Boot shows the main menu: title `NAME GAME`, `V1.0` in grey at the bottom right, the items CLASSIC, LEVEL PICKER, NAME QUEST and SETTINGS and the saved best as `HI 0012345` under the panel, with the starfield scrolling behind it. No music.
2. CLASSIC starts a run at level 1 that goes through every level in order; the deep sea's clear is the win. LEVEL PICKER opens the level wheel. Both start with 1 player (player 2 still drops in with Start). Play again replays the same mode and level. The `HI` line sits at y 150.
3. Settings has the items MUSIC: MIZRAHIT / DUBSTEP / OFF, SHAKE: 0–100 % in steps of 25, FLASHING: NORMAL / REDUCED and BACK. X or D-pad right changes the item (music steps MIZRAHIT → DUBSTEP → OFF and wraps, shake steps up and wraps; left steps either one back), and each change is saved to settings.txt and survives a reboot.
4. Circle or BACK leaves settings and returns to whichever menu opened it.
5. Start during a run opens the pause menu over the frozen game: RESUME, SETTINGS, RETURN TO MENU. Start or Circle also resume. RETURN TO MENU ends the run without confirmation and stops the music. A controller disconnect also opens it, with the hint CONNECT CONTROLLER n, and RESUME waits for that controller.
6. OFF means no song plays; sound effects are unaffected. Changing the music while paused switches at once: a song starts from the beginning, OFF stops it. Changing it from the main menu plays nothing.
7. At game over and on the level-clear end screen, X or Start plays again and Circle goes to the main menu.
8. Every connected controller drives the menus. Up/down is a D-pad press or a left-stick flick, and the cursor wraps around.
9. X selects. Start selects too, except in the pause menu, where Start resumes.
10. Circle goes back: settings to its opener, pause to resume, level picker to the main menu, main menu to nothing.
11. The level picker is a wheel titled `LEVEL`: SPACE, JUNGLE, GLACIER, VOLCANO, DEEP SEA. The picked level is in the middle at double size in yellow, the two above and below it at normal size (white, then grey). Up/down spins the wheel one row (wrapping), the background shows the picked level, X plays just that level (its clear is the win), Circle returns to the main menu. It opens on the last picked level.

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Menu panel width | 150 | px | |
| Item spacing | 12 | px | cursor easy to follow from the couch |
| Stick flick on / off | 0.6 / 0.3 | stick range | one flick moves one item, no repeats |
| Shake steps | 25 | % per press | five steps, wraps around |
| Picker row spacing | 14 | px | |
| Picker spin time | 0.12 | s per row | spin reads as motion, fast presses don't lag |

## Edge cases
- Circle is button 10 from the SDL port's source only. If it is wrong on the PS3, every screen is still reachable through BACK, RESUME and RETURN TO MENU.
- An old settings.txt keeps working: `music=1` is MIZRAHIT, `music=0` is OFF.
- Settings save fails: the game keeps the in-memory value and logs "SAVE FAILED".
- A missing or unreadable settings.txt gives the defaults (music MIZRAHIT, shake 100 %, flashing normal).

## History
- 2026-11-17: designed (D-010).
- 2026-11-17: numbers confirmed in single-player and co-op playtests.
- 2026-11-17: title renamed NAME GAME with a version label; unplug pause added (D-012, D-014).
- 2026-11-17: music setting chooses MIZRAHIT, DUBSTEP or OFF (D-018).
- 2026-11-18: jungle level (D-020): the JUNGLE menu item.
- 2026-11-18: levels 3-5 (D-024): GLACIER, VOLCANO and DEEP SEA items.
- 2026-11-18: menu split into CLASSIC and a spinning LEVEL PICKER (D-028).
