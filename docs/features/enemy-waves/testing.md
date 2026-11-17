# Enemy waves: testing

**Updated:** 2026-11-17 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] Formations have 2, 3, 4, then 5 rows of 8 as the waves rise. *(`tests/game_tests.cpp` › formationGrowsToFiveRows)*
- [x] Clearing a wave gives 1,000 × wave and starts the next one after 2 s. *(`tests/game_tests.cpp` › clearingAWaveStartsTheNextAfterThePause)*
- [x] Shot chance and dive interval follow the formulas. *(`tests/game_tests.cpp` › enemyFormulasFollowTheDesign)*
- [x] Aliens fly in, reach their slots within 2.5 s and never shoot while entering. *(`tests/game_tests.cpp` › aliensFlyInBeforeTheyCanShoot)*
- [x] No dives before wave 3; at wave 3 one alien dives at a time. *(`tests/game_tests.cpp` › divesStartAtWaveThree)*
- [x] Waves 1–4 play through on the PS3 at 60 fps. *(PS3 log 2026-11-17: wave 1 → 4, game over at 13,900)*
- [ ] Waves 1–3 feel calm and 7–9 busy. *(not proven: the playtest ended at wave 4)*

## History
- 2026-11-17: criteria written.
