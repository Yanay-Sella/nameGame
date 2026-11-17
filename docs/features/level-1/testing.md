# Level 1: testing

**Updated:** 2026-11-17 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] The run starts with a 3.5 s banner (`LEVEL 1` for 2 s, then `WAVE 1`), and wave 2 has the usual 1.5 s banner. *(`tests/game_tests.cpp` › levelBannerComesBeforeWaveOne)*
- [x] Killing the wave-20 boss gives the reward and clear bonus, starts a 3 s `LEVEL 1 CLEAR` banner, keeps enemy shots in the air, then ends the run as a win; wave 21 never starts. *(`tests/game_tests.cpp` › killingTheLevelBossClearsTheLevel)*
- [x] A life lost during the banner wins if lives are left and is a game over on the last life. *(`tests/game_tests.cpp` › deathDuringTheClearOnlyLosesOnTheLastLife)*
- [ ] The `LEVEL 1` banner shows on the PS3 screen.
- [ ] An autopilot run ends at wave 20 with `level 1 clear` and `level clear` 3.0 s later.
- [ ] The end-screen panel reads `LEVEL 1 CLEAR` in yellow with score, wave, `NEW HIGH SCORE` and `X PLAY AGAIN   O MENU`.
- [ ] The high score is saved on the win.
- [ ] X starts a new run from the end screen.

## History
- 2026-11-17: criteria written.
