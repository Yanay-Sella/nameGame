# Jungle: testing

**Updated:** 2026-11-18 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] Jungle wave 1 plays at difficulty wave 6: 40 aliens, two beetle rows, a 3.5 s banner. *(`tests/game_tests.cpp` › jungleDifficultyIsFiveWavesAhead)*
- [x] Heavies: `wave / 8` on jungle formation waves, one with 12 HP on wave 8. *(heavyBeetlesComeEveryEightWaves)*
- [x] A heavy roams y 18…84 and sweeps more than 100 px sideways. *(heavyBeetleRoamsTheUpperScreen)*
- [x] Level 1 clear continues into the jungle with the score unchanged. *(levelOneClearContinuesIntoTheJungle)*
- [x] Boss HP and types follow the jungle table. *(jungleBossesFollowTheTable)*
- [x] Monkey bananas fall straight down, a peel is lobbed, then lands. *(monkeyThrowsBananasStraightDownThenAPeel)*
- [x] The monkey alternates with a 5-banana bunch. *(monkeyAlternatesWithABananaBunch)*
- [x] A peel slides the ship, then stuns it (no move, no shooting). *(peelSlidesTheShipThenStunsIt)*
- [x] A peel lies 5 s. *(peelLiesFiveSeconds)*
- [x] A banana kills the ship. *(bananaKillsTheShip)*
- [x] Bullets from inside a mist heal the gorilla and make it laugh; outside they hurt. *(mistShotsHealTheGorilla)*
- [x] A poop splats into a mist that fades after 6 s. *(poopSplatsIntoAMistThatFades)*
- [x] A pound drops five coconuts, one per fifth. *(gorillaPoundDropsFiveCoconuts)*
- [x] No poop past three mists; pounds and peels still happen. *(gorillaNeverPoopsPastThreeMists)*
- [x] A dead gorilla drops no coconuts. *(deadGorillaDropsNoCoconuts)*
- [x] Only deadly projectiles use magenta. *(onlyDeadlyShotsUseMagenta)*
- [x] The jungle background is darker than the stars and the edge trunks are drawn. *(levelBackgroundsStayDarkerThanTheStars)*
- [x] The jungle boss clear continues into the glacier (D-024; before that it ended the run). *(eachLevelClearContinuesToTheNext)*
- [ ] `make ps3` builds without warnings in the new code.
- [ ] PS3 autopilot run from JUNGLE: 60 fps, the log lines of the plan's step 8, ends with `JUNGLE CLEAR`.
- [ ] PS3 screenshots match the mockups: a magenta shot is findable at a glance, edge trunks don't hide the ship.
- [ ] One hand-played jungle run judges the peel stun and the mist (user's verdict).

## History
- 2026-11-18: criteria written.
