# Screens: testing

**Updated:** 2026-11-17 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] Without a settings file, music defaults to on. *(`tests/game_tests.cpp` › settingsDefaultToMusicOn)*
- [x] A saved music setting loads back. *(`tests/game_tests.cpp` › settingsSurviveASave)*
- [x] The menu cursor `>` has a font glyph. *(`tests/game_tests.cpp` › menuCursorHasAGlyph)*
- [x] The title shows `HI` with the saved best; the settings items SHAKE (steps 0–100, wrapping) and FLASHING change and are saved. *(PS3 log 2026-11-17: `settings: … shake 75/50/25/0/…, flashing reduced/normal (saved)`; title and relaunch by player report 2026-11-17; file format: `tests/game_tests.cpp` › shakeAndFlashSettingsSurviveASave)*
- [x] Menu up/down works with the D-pad and the stick, one flick per item. *(player report 2026-11-17; PS3 log: D-pad presses and scene changes)*
- [x] Settings → music off → Back → Classic gives a silent run with sound effects. *(player report 2026-11-17)*
- [x] Start → Settings → music on starts the song; Circle → Resume continues the run; Return to menu stops it. *(player report 2026-11-17; PS3 log: paused → settings → paused → playing, `music on (saved)`)*
- [x] Start → Return to menu shows the main menu. *(PS3 log 2026-11-17: scene paused → main menu; that the music stops is not in the log)*
- [x] The music setting is kept after quitting and relaunching. *(PS3 log 2026-11-17: "music off (saved)", "nameGame exiting", then "settings: music off" at the next start)*
- [x] Losing a run, then Circle at game over, shows the main menu. *(PS3 log 2026-11-17: game over → button 10 → scene: main menu)*
- [x] Circle is button 10 on the PS3. *(PS3 log 2026-11-17: "button 10" on every Circle press)*
- [x] 60 fps in the menus and in play. *(PS3 log 2026-11-17: fps 59.7 in the main menu, 60.0 in play, slowest frame 6 ms)*

## Playtest checks
1. Menu up/down with the D-pad and the left stick.
2. Settings → music off → Back → Classic: silent run, sound effects still play.
3. Start → Settings → music on (song starts) → Circle → Resume.
4. Start → Return to menu.
5. Quit and relaunch: the music, shake and flashing settings are kept, and the title shows `HI`.
7. Settings → SHAKE up and down through 0–100 %; FLASHING normal and reduced; then a rocket blast shows the difference.
6. Lose a run → Circle at game over → main menu.

## History
- 2026-11-17: criteria written.
