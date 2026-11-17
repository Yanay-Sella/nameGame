# Pixel art: testing

**Updated:** 2026-11-17 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] Every sprite's text is exactly width × height characters. *(`tests/game_tests.cpp` › spritesHaveTheirSize)*
- [x] Only the enemy shot uses magenta. *(`tests/game_tests.cpp` › onlyEnemyShotsUseMagenta)*
- [x] Wave 1, wave 4 and wave 7 frames, the blast and the game-over panel render as intended. *(Mac render check 2026-11-17: frames drawn by `drawGame` and saved as images)*
- [x] The picture fills a 1080p TV at ×6 and looks good. *(PS3 log 2026-11-17: "output 1920x1080, picture x6"; user said "looks good")*
- [ ] The picture is right at 720p (×4). *(not proven: the PS3 outputs 1080p)*

## History
- 2026-11-17: criteria written.
