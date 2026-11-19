# Deep sea: testing

**Updated:** 2026-11-18 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] Waves play 14 ahead with shots 15 px/s faster; up to 3 heavies fly in at x 60, 244 and 152. *(`tests/game_tests.cpp` › levelLadderGetsHarder, heaviesFlyInAtThreePlaces)*
- [x] The current drifts an idle ship by the formula; level 1 doesn't drift. *(currentPushesShipsInTheDeepSea)*
- [x] The anglerfish cycles stream, big bubble (pops into 8) and fan. *(anglerCyclesStreamBigBubbleAndFan)*
- [x] The hook catches, lifts 24 px, stuns 1.6 s, then lets go. *(hookLiftsAndStunsTheShip)*
- [x] A blinking ship isn't hooked. *(invincibleShipsSlipOffTheHook)*
- [x] About a third of empty casts bring an old boot, thrown at 85 px/s. *(emptyCastsSometimesBringABoot)*
- [x] Killing the deep sea's wave-20 bosses ends the run as a win with its reward. *(killingTheLevelBossClearsTheLevel, deathDuringTheClearOnlyLosesOnTheLastLife)*
- [x] `make ps3` builds without warnings (2026-11-18).
- [ ] PS3 autopilot run from DEEP SEA at 60 fps, log lines `boss wave 15: 1 fisherman`, `player 1 hooked` or `boot thrown`, `level 5 clear at`.
- [ ] One hand-played deep-sea run judges the current and the hook stun (user's verdict).
- [x] A moving ship trails bubbles only in the deep sea. *(bubbleTrailOnlyWhileMovingInTheDeepSea)*
- [x] The fisherman keeps at most 3 live fish out; they land, chase, kill and die as specified. *(fishermanKeepsAtMostThreeLiveFishOut, liveFishLandsAndChasesTheShip, liveFishKillsAndDies, shotsAndTimeEndLiveFish)*
- [ ] PS3: the fisherman, live fish and bubble trail look right.

## History
- 2026-11-18: criteria written.
