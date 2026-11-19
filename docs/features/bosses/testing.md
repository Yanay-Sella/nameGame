# Bosses: testing

**Updated:** 2026-11-17 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] Waves 5, 10, 15 are a boss with no aliens and a `BOSS` banner; wave 6 is a normal formation. *(`tests/game_tests.cpp` › everyFifthWaveIsABoss)*
- [x] Boss waves follow the table: HP 100/100/200/200, 1/2/1/2 bosses, classic then mage from wave 15. *(`tests/game_tests.cpp` › bossWavesFollowTheTable)*
- [x] A bullet does 1 damage, a rocket 20; a blast bursting 20 px above it still hits; one 80 px away doesn't. *(`tests/game_tests.cpp` › bulletsAndBlastsDamageTheBoss)*
- [x] Killing a boss scores 5,000·n, leaves a large explosion and drops an upgrade and a rocket pickup, then the wave-clear pause starts. *(`tests/game_tests.cpp` › killingTheBossScoresAndDropsBothPickups)*
- [x] Touching its body kills the ship. *(`tests/game_tests.cpp` › bossBodyKillsTheShip)*
- [x] It fires a burst of 3 aimed shots at 90 px/s, then a fan of 5. *(`tests/game_tests.cpp` › bossFiresBurstThenFan)*
- [x] It holds fire while no ship is alive. *(`tests/game_tests.cpp` › bossWaitsForALivingShip)*
- [x] The bosses, health bar, flash and blast read clearly on screen. *(Mac render 2026-11-17)*
- [x] On the PS3 the bosses of waves 5, 10 and 15 are beaten (co-op run) and 60 fps holds. *(PS3 log 2026-11-17: `boss down at wave 5/10/15`, fps 60.0, worst work 6 ms; player report: burst and fan read well)*
- [x] Boss waves feel beatable but threatening in single player. *(player report 2026-11-17: numbers fine as is)*
- [x] Sway speed and attack rate hold up at appearance 4 (wave 20). *(player report 2026-11-17: numbers fine)*
- [x] Twins sway each in their own half. *(`tests/game_tests.cpp` › twinBossesSwayInTheirOwnHalves)*
- [x] Both twins must die before the wave clears; each drops both pickups. *(`tests/game_tests.cpp` › twinBossesBothMustDie)*
- [x] A mage's ball beams its column after 1.5 s and kills a ship in it. *(`tests/game_tests.cpp` › mageBallBeamsItsColumnAfterOneAndAHalfSeconds)*
- [x] A dead mage casts no beam. *(`tests/game_tests.cpp` › deadMageCastsNoBeam)*
- [x] The mage attacks in random order and gaps (0.6–1.4 × interval). *(`tests/game_tests.cpp` › mageAttacksInRandomOrderAndGaps)*
- [x] The beam draws as a magenta column with a white core. *(`tests/game_tests.cpp` › beamDrawsAMagentaColumn)*
- [ ] Waves 10 and 20 show two bosses, each in its half.
- [ ] The mage's ball charges at the wand's star and the beam reads clearly; 60 fps holds.
- [ ] Twin mages at wave 20 are beatable but threatening (player report).
- [x] The mage's ball forms at the wand's star. *(mageBallFormsAtTheWand)*
- [ ] PS3: the mage's hat and wand look right.

## History
- 2026-11-17: criteria written.
- 2026-11-17: line-up criteria added (D-019). Earlier PS3 evidence: hp 150/200/250 at waves 10/15/20 under the old rule.
