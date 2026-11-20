# NAME QUEST: testing

**Updated:** 2026-11-20 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] Every area has a valid size, only legend characters, at most 4 portals with links, and chest counts matching their lists. *(`tests/quest_tests.cpp` › areaSizesAndObjectsAreValid)*
- [x] Every open link arrives on a tile the hero can stand on, or on a ladder. *(`tests/quest_tests.cpp` › linksArriveOnStandableTiles)*
- [x] Areas joined by an edge touch on the world map. *(`tests/quest_tests.cpp` › edgeLinkedAreasAreMapNeighbours)*
- [x] Every arrival point, checkpoint and object in Namula and the TEST GROUNDS is reachable by the Knight, and from every arrival point a checkpoint can be reached. *(`tests/quest_tests.cpp` › everythingIsReachable)*
- [x] A held jump peaks 36 ± 1 px; a tap peaks at most 12 px. *(`tests/quest_tests.cpp` › jumpPeakIsFourAndAHalfTiles)*
- [x] Coyote time, the jump buffer, dropping through one-way platforms, climbing and jumping off ladders, ice braking (at least 0.4 s) and the Thief's one double jump work. *(`tests/quest_tests.cpp` › coyoteAndBufferWork, oneWayDropThrough, ladderClimbAndJumpOff, iceSlides, thiefDoubleJumpsOthersDont)*
- [x] A save survives a write and a read with every field; a missing slot is empty; deleting removes it. *(`tests/quest_tests.cpp` › saveRoundTrip, missingSaveIsUnused, deleteRemovesTheSlot)*
- [x] Every quest sprite has its size. *(`tests/quest_tests.cpp` › questSpritesHaveTheirSize)*
- [x] On the Mac: Namula shows the hero and the HUD; walking into the sea logs `quest: hero died in 0.2 (liquid)` and the hero comes back at the hearth; the Knight climbs the TEST GROUNDS shaft's ladder. *(`make mac-run` 2026-11-20)*
- [ ] On the PS3: main menu → NAME QUEST → empty slot 1 → ARCHER → the intro → walking and jumping in Namula. *(PS3 log: `scene: quest`, `quest: slot 1 new (class ARCHER, level 1)`, `quest: area 0.0 NAMULA`)*
- [ ] On the PS3: RETURN TO MENU saves the slot. *(PS3 log: `quest: save slot 1 ok`)*
- [ ] On the PS3: 60 fps in Namula, worst work under 10 ms. *(PS3 log: `fps ... quest 0.0`)*
- [ ] On the PS3: after a relaunch, X on slot 1 loads it. *(PS3 log: `quest: slot 1 loaded (class ARCHER, level 1)`)*

## Playtest checks
1. `NAMEGAME_QUEST='slot=3 class=3 world=0 area=0' make mac-run BUTTONS='2|right:1.5|cross|1|l1|1|quit'`: the Thief in Namula, HUD at the top left.
2. `NAMEGAME_QUEST='slot=3 class=1 world=0 area=2' make mac-run BUTTONS='1|left:1.5|4|quit'`: the hero walks into the sea, `YOU FADED`, back at the hearth.
3. `NAMEGAME_QUEST='slot=3 class=0 world=8 area=1' make mac-run BUTTONS='1|right:0.95|up:1|l1|1|quit'`: the Knight on the shaft's first ladder.
4. On the PS3, walk Namula's three areas with every class: steps, ladders and the cliffs feel calm; the jump feels right.

## History
- 2026-11-20: criteria written; Mac checks done.
