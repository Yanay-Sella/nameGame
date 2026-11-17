# Rocket: testing

**Updated:** 2026-11-17 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] An R2 press 0.1 s before the cooldown ends still fires; one 0.3 s early is dropped. *(`tests/game_tests.cpp` › rocketBufferFiresAnEarlyPress)*
- [x] With 0 rockets, R2 plays the empty click and launches nothing. *(`tests/game_tests.cpp` › emptyRocketClicks)*
- [x] The blast kills aliens 0 and 30 px away and spares one 60 px away; kills score for the rocket's owner. *(`tests/game_tests.cpp` › blastKillsOnlyInsideItsRadius)*
- [x] A pickup adds a rocket, never above 9. *(`tests/game_tests.cpp` › pickupAddsARocketUpToNine)*
- [x] R2 is button 14 and fires on the PS3. *(PS3 log 2026-11-17: 6 presses of button 14 in a full run)*
- [ ] Two rockets in flight each damage an alien at most once. *(not proven: no test)*
- [ ] Rockets feel scarce but decisive; pickups arrive about once every 2 waves. *(not proven: needs a playtest focused on rockets)*

## History
- 2026-11-17: criteria written.
