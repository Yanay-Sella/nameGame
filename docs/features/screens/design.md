# Screens

**Status:** in progress · **Updated:** 2026-11-17 · [GDD](../../GDD.md) · [Implementation](implementation.md) · [Testing](testing.md)

The game opens on a main menu with room for more game modes, and a settings screen holds the music switch. It serves pillar 3 (arcade): only the menus the game needs.

## Rules
1. Boot shows the main menu: title "NAMEGAME" and the items CLASSIC and SETTINGS, with the starfield scrolling behind it. No music.
2. CLASSIC is the only game mode for now: today's game, started with 1 player (player 2 still drops in with Start).
3. Settings has the items MUSIC: ON / MUSIC: OFF and BACK. X, or D-pad left/right, on the music item toggles it. The value is saved to settings.txt and survives a reboot.
4. Circle or BACK leaves settings and returns to whichever menu opened it.
5. Start during a run opens the pause menu over the frozen game: RESUME, SETTINGS, RETURN TO MENU. Start or Circle also resume. RETURN TO MENU ends the run without confirmation and stops the music.
6. Music off means the song never plays; sound effects are unaffected. Turning it on while paused starts the song from the beginning; turning it off stops it at once.
7. At game over, X or Start plays again and Circle goes to the main menu.
8. Every connected controller drives the menus. Up/down is a D-pad press or a left-stick flick, and the cursor wraps around.
9. X selects. Start selects too, except in the pause menu, where Start resumes.
10. Circle goes back: settings to its opener, pause to resume, main menu to nothing.

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Menu panel width | 150 | px | |
| Item spacing | 12 | px | cursor easy to follow from the couch [ASSUMPTION] |
| Stick flick on / off | 0.6 / 0.3 | stick range | one flick moves one item, no repeats [ASSUMPTION] |

## Edge cases
- Circle is button 10 from the SDL port's source only. If it is wrong on the PS3, every screen is still reachable through BACK, RESUME and RETURN TO MENU.
- Music toggled while paused: off stops the song at once, on starts it from the beginning.
- Settings save fails: the game keeps the in-memory value and logs "SAVE FAILED".
- A missing or unreadable settings.txt gives the defaults (music on).

## History
- 2026-11-17: designed (D-010).
