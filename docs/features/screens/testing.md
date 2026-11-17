# Screens: testing

**Updated:** 2026-11-17 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] Without a settings file, music defaults to on. *(`tests/game_tests.cpp` › settingsDefaultToMusicOn)*
- [x] A saved music setting loads back. *(`tests/game_tests.cpp` › settingsSurviveASave)*
- [x] The menu cursor `>` has a font glyph. *(`tests/game_tests.cpp` › menuCursorHasAGlyph)*
- [ ] Menu up/down works with the D-pad and the stick, one flick per item. *(partly: the log shows D-pad presses and scene changes main menu → settings → main menu → playing; the stick isn't logged)*
- [ ] Settings → music off → Back → Classic gives a silent run with sound effects. *(PS3 check pending)*
- [ ] Start → Settings → music on starts the song; Circle → Resume continues the run. *(partly: the log shows paused → settings, "music on (saved)", then paused → playing; the song and Circle are not in the log)*
- [x] Start → Return to menu shows the main menu. *(PS3 log 2026-11-17: scene paused → main menu; that the music stops is not in the log)*
- [ ] The music setting is kept after quitting and relaunching. *(PS3 check pending)*
- [ ] Losing a run, then Circle at game over, shows the main menu. *(PS3 check pending)*
- [ ] Circle is button 10 on the PS3. *(PS3 check pending)*
- [x] 60 fps in the menus and in play. *(PS3 log 2026-11-17: fps 59.7 in the main menu, 60.0 in play, slowest frame 6 ms)*

## Playtest checks
1. Menu up/down with the D-pad and the left stick.
2. Settings → music off → Back → Classic: silent run, sound effects still play.
3. Start → Settings → music on (song starts) → Circle → Resume.
4. Start → Return to menu.
5. Quit and relaunch: the music setting is kept.
6. Lose a run → Circle at game over → main menu.

## History
- 2026-11-17: criteria written.
