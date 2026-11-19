# Glacier: testing

**Updated:** 2026-11-18 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] Waves play 8 ahead, heavies come every 6 waves (at most 3). *(`tests/game_tests.cpp` › levelLadderGetsHarder)*
- [x] The jungle clear continues into the glacier, and the glacier into the volcano, without leftover hazards. *(eachLevelClearContinuesToTheNext)*
- [x] Boss types and HP follow the table. *(newBossesFollowTheTable)*
- [x] The ship glides on the ice; level 1 stays instant. *(shipGlidesOnTheGlacier)*
- [x] An ice block freezes the ship 1.6 s; mashing R1 breaks out sooner. *(iceBlockFreezesAndMashingBreaksOut)*
- [x] The yeti cycles volley, ice block and sneeze (`AH...`, `ACHOO!`, 7 snowballs). *(yetiCyclesVolleyIceBlockAndSneeze)*
- [x] The golem throws two blocks and icicle rain; icicles hang 0.8 s in their sixths, then fall. *(golemIciclesHangThenFall)*
- [x] Only deadly sprites use magenta; the background is darker than the stars. *(onlyDeadlyShotsUseMagenta, levelBackgroundsStayDarkerThanTheStars)*
- [x] `make ps3` builds without warnings (2026-11-18).
- [ ] PS3 autopilot run from GLACIER at 60 fps, log lines `run starts at level 3`, `boss wave 5: 1 yeti`, `player 1 frozen`.
- [ ] One hand-played glacier run judges the gliding and the freeze (user's verdict).
- [x] A 24 px ice block freezes from its corner and is see-through. *(bigIceBlockFreezesFromItsCorner, seeThroughIceTintsWhatIsUnder)*
- [ ] PS3: the big see-through ice blocks and the ice cube look right.

## History
- 2026-11-18: criteria written.
