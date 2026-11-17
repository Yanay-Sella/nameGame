# Bosses: testing

**Updated:** 2026-11-17 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] Waves 5, 10, 15 are a boss with no aliens and a `BOSS` banner; wave 6 is a normal formation. *(`tests/game_tests.cpp` › everyFifthWaveIsABoss)*
- [x] HP is 100, 150, 200 for waves 5, 10, 15. *(`tests/game_tests.cpp` › bossHealthGrowsEachAppearance; PS3 log 2026-11-17: boss wave 5, hp 100 / 10, hp 150 / 15, hp 200)*
- [x] A bullet does 1 damage, a rocket 20; a blast bursting 20 px above it still hits; one 80 px away doesn't. *(`tests/game_tests.cpp` › bulletsAndBlastsDamageTheBoss)*
- [x] Killing it scores 5,000·n, leaves a large explosion and drops an upgrade and a rocket pickup, then the wave-clear pause starts. *(`tests/game_tests.cpp` › killingTheBossScoresAndDropsBothPickups)*
- [x] Touching its body kills the ship. *(`tests/game_tests.cpp` › bossBodyKillsTheShip)*
- [x] It fires a burst of 3 aimed shots at 90 px/s, then a fan of 5. *(`tests/game_tests.cpp` › bossFiresBurstThenFan)*
- [x] It holds fire while no ship is alive. *(`tests/game_tests.cpp` › bossWaitsForALivingShip)*
- [x] The boss, health bar, flash and blast read clearly on screen. *(Mac render 2026-11-17)*
- [x] On the PS3 the bosses of waves 5, 10 and 15 are beaten (co-op run) and 60 fps holds. *(PS3 log 2026-11-17: `boss down at wave 5/10/15`, fps 60.0, worst work 6 ms; player report: burst and fan read well)*
- [ ] Boss waves feel beatable but threatening in single player. *(not proven: the PS3 run was co-op with waves 5–15 reached from a temporary wave 4 start)*
- [ ] Sway speed and attack rate at appearance 4 and later (wave 20+). *(not proven: not played)*

## History
- 2026-11-17: criteria written.
