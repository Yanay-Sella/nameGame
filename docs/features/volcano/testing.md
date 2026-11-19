# Volcano: testing

**Updated:** 2026-11-18 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] Waves play 11 ahead, heavies come every 5 waves (at most 3). *(`tests/game_tests.cpp` › levelLadderGetsHarder)*
- [x] The volcano clear continues into the deep sea. *(eachLevelClearContinuesToTheNext)*
- [x] Boss types and HP follow the table. *(newBossesFollowTheTable)*
- [x] A lava drop scorches: 0.6× speed, bullets do 2 damage for 3 s. *(lavaDropScorchesForDoubleDamage)*
- [x] The imp cycles fireball volley, lava drip and fan. *(impCyclesFireballsLavaAndFan)*
- [x] The fire lord rolls volley, lava rain and eruption. *(fireLordRollsAllFourAttacks)*
- [x] An eruption's warning is harmless; its column kills only inside its 16 px. *(eruptionKillsOnlyInItsColumn)*
- [x] `make ps3` builds without warnings (2026-11-18).
- [ ] PS3 autopilot run from VOLCANO at 60 fps, log lines `boss wave 15: 1 fire lord`, `eruption at x`, `player 1 scorched`.
- [ ] One hand-played volcano run judges the scorch trade-off (user's verdict).
- [x] The imp throws one big lava drop straight down at 60 px/s. *(impCyclesFireballsLavaAndFan)*
- [ ] PS3: the imp's big lava drop looks right.

## History
- 2026-11-18: criteria written.
