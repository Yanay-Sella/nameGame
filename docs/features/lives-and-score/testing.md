# Lives and score: testing

**Updated:** 2026-11-17 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] An enemy shot on the hitbox kills; three deaths end the run, and the ship returns after each of the first two. *(`tests/game_tests.cpp` › threeDeathsEndTheRun)*
- [x] An alien's body touching the ship kills it. *(`tests/game_tests.cpp` › alienBodyKillsTheShip)*
- [x] Crossing 50,000 and 100,000 in one go gives two extra lives. *(`tests/game_tests.cpp` › extraLivesAtEvery50000)*
- [x] The next wave doesn't start while no ship is alive. *(`tests/game_tests.cpp` › nextWaveWaitsForALivingShip)*
- [x] Game over and a new run work on the PS3. *(PS3 log 2026-11-17: "game over: score 13900, wave 4", then "new run")*
- [x] Deaths feel fair. *(PS3 playtest 2026-11-17: user said "feels good")*
- [ ] Respawn invincibility is long enough to get clear. *(not proven: no focused playtest)*

## History
- 2026-11-17: criteria written.
