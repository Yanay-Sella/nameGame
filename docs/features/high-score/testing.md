# High score save: testing

**Updated:** 2026-11-17 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] A missing file and a garbage file both load as 0. *(`tests/game_tests.cpp` › highScoreDefaultsToZero)*
- [x] A saved score loads back; saving to a missing folder reports failure. *(`tests/game_tests.cpp` › highScoreSurvivesASave)*
- [x] Negative values load as 0 and huge ones as 9,999,999. *(`tests/game_tests.cpp` › highScoreIsClamped)*
- [x] The HUD `HI` line and the `NEW HIGH SCORE` panel (single player and co-op) read clearly. *(Mac render 2026-11-17)*
- [x] A finished run with a new best is saved. *(PS3 log 2026-11-17: `high score: 220900 (saved)`, co-op run, player 1's score)*
- [x] The value is read at the next launch. *(PS3 log 2026-11-17: `high score: 220900` at boot)*
- [x] The value survives a PS3 reboot. *(PS3 log 2026-11-17: `high score: 220900` at boot after a reboot and HEN re-enabled)*
- [x] Reinstalling the pkg doesn't wipe the file. *(PS3 log 2026-11-17: `high score: 220900` at launch after the game-feel `make deploy`)*
- [ ] A run left through RETURN TO MENU doesn't save. *(not proven: no test, the path is screen code)*
- [ ] `SAVE FAILED` path on the console. *(not proven: no way to force a failed write on the PS3)*

## History
- 2026-11-17: criteria written.
