# Level 1: testing

**Updated:** 2026-11-17 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] The run starts with a 3.5 s banner (`LEVEL 1` for 2 s, then `WAVE 1`), and wave 2 has the usual 1.5 s banner. *(`tests/game_tests.cpp` › levelBannerComesBeforeWaveOne)*
- [x] Killing the wave-20 boss gives the reward and clear bonus, starts a 3 s `LEVEL 1 CLEAR` banner, keeps enemy shots in the air, then ends the run as a win; wave 21 never starts. *(`tests/game_tests.cpp` › killingTheLevelBossClearsTheLevel)*
- [x] A life lost during the banner wins if lives are left and is a game over on the last life. *(`tests/game_tests.cpp` › deathDuringTheClearOnlyLosesOnTheLastLife)*
- [x] The `LEVEL 1` banner shows on the PS3 screen. *(PS3 screenshot 2026-11-17)*
- [x] Before D-020, an autopilot run ended at wave 20 with `level 1 clear` and `level clear` 3.0 s later; now the jungle follows the banner. *(PS3 log 2026-11-17: `level 1 clear at 381.5 s`, `level clear at 384.5 s`, no wave 21)*
- [x] Before D-020, the end-screen panel read `LEVEL 1 CLEAR` in yellow (now `JUNGLE CLEAR` after the jungle) with score, wave, `NEW HIGH SCORE` and `X PLAY AGAIN   O MENU`. *(PS3 screenshot 2026-11-17)*
- [x] The high score is saved on the win. *(PS3 log 2026-11-17: `high score: 381200 (saved)`)*
- [x] X starts a new run from the end screen. *(PS3 log 2026-11-17: `new run`)*

## History
- 2026-11-17: criteria written.
- 2026-11-18: jungle level (D-020): the end screen follows the jungle.
