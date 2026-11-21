# NAME QUEST: testing

**Updated:** 2026-11-21 · [Design](design.md) · [Implementation](implementation.md)

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

- [x] Level costs, the reference hit (15/20/25 % of the reference HP within 1 % or 1 HP), and the class differences at level 10. *(`tests/quest_tests.cpp` › levelCostMatchesSpec, referenceHitIsTwentyPercent, classesDifferAsDesigned)*
- [x] A world-1 reference Magician kills fodder in 3 bolts and gets its names. *(`tests/quest_tests.cpp` › basicAttackKillsFodderInThreeHits)*
- [x] The Knight's swing hits only in its window; arrows stop at walls; the charger tells then charges; the armored front guard; the dropper falls and throws; walkers never walk into pits. *(`tests/quest_tests.cpp` › knightSwingHitsOnlyInItsWindow, arrowsStopAtWalls, chargerTellsThenCharges, armoredGuardsItsFront, dropperFallsWhenHeroBelow, walkersNeverWalkIntoPits)*
- [x] R2 pressed during a Knight's swing still lands the Power Slash; spikes hurt 20 % and bounce the hero up. *(`tests/quest_tests.cpp` › skillCastMidSwingStillHits, spikesHurtAndBounce)*
- [x] I-frames block damage; the roll and the blink give i-frames and the blink stops before walls; the strong attack needs MP. *(`tests/quest_tests.cpp` › iframesBlockDamage, rollGivesIframes, blinkStopsBeforeWalls, strongAttackNeedsMp)*
- [x] Death drops the names at the last safe ground, a second death loses the old echo, touching the echo returns them, death respawns enemies. *(`tests/quest_tests.cpp` › deathDropsNamesAtLastSafeGround, secondDeathLosesTheOldEcho, echoReturnsNames, deathRespawnsEnemies)*
- [x] On the Mac: each class's tier-3 skill shows at 640x360 (Lion's Roar arc, Storm Volley fan, Meteor marker and fall, Shadow Partner), numbers float, the MP bar drops; a level-1 Knight dies to the TEST GROUNDS enemies (`dropped 500 names`) and gets them back after warping in (`names recovered 500`). *(`make mac-run` and a scripted run of the real update, 2026-11-20)*
- [ ] On the PS3: an Archer fights the TEST GROUNDS enemies at 60 fps, worst work under 10 ms. *(PS3 log: `quest: area 8.3`, `fps ... quest 8.3`)*
- [ ] On the PS3: death drops names and touching the echo returns them. *(PS3 log: `quest: hero died in ...`, `quest: names recovered n`)*
- [x] Levels spend their cost and raise the stats, the menu stays open, level 40 is the last. *(`tests/quest_tests.cpp` › levelUpSpendsAndRaisesStats, cannotLevelPastForty)*
- [x] Prices scale with the highest world; carry limits stop buying (shards and the locket raise them); gear tiers wait for their world. *(`tests/quest_tests.cpp` › shopPricesScaleWithHighestWorld, carryLimitsStopBuying, gearTierNeedsTheWorld)*
- [x] Touching a brazier lights it; REST heals and respawns enemies; TRAVEL lists NAMULA and the lit checkpoints and goes there. *(`tests/quest_tests.cpp` › restHealsAndRespawnsEnemies, travelListsLitCheckpoints)*
- [x] A chest opens once and is saved; a full bag keeps it shut; the gate wall opens; a lever opens its doors for good. *(`tests/quest_tests.cpp` › chestOpensOnceAndSaves, fullBagKeepsChestShut, gateWallOpens, leverOpensDoorsForGood)*
- [x] Jobs need the level and world 1's gate, then the level and the FROZEN FEATHER (taken). *(`tests/quest_tests.cpp` › jobNeedsLevelAndGate, secondJobNeedsTheFeather)*
- [x] Buffs end on death; a potion heals after its 0.5 s drink. *(`tests/quest_tests.cpp` › buffsEndOnDeath, drinkHealsAfterHalfASecond)*
- [x] NPC presence follows the quest stages; every line fits the dialogue box (4 lines of 72). *(`tests/quest_tests.cpp` › npcPresenceFollowsQuestStages, dialogueFitsTheBox)*
- [x] On the Mac: Namula's NPCs stand at the stalls, the hearth and the stage facing the hero, Ilsa's lantern glows; four talks with Ilsa and two levels (`quest: level 3 (spent 120, names left 1780)`); two MANA POTION from Marra; the TEST GROUNDS brazier lights and rests (`checkpoint 8.0 lit`, `rest at 8.0`); the surveyor's line and the TEST GROUNDS map (current area blinking, the gate's red bar). *(`make mac-run` 2026-11-21)*
- [ ] On the PS3: slot 1 → Select-hold into TEST GROUNDS → rest at its checkpoint → the map at the surveyor → travel to Namula → Ilsa. *(PS3 log: `quest: checkpoint 8.0 lit`, `quest: rest at 8.0`, `quest: travel to 0.0`; fps ≈ 60)*
- [ ] On the PS3: the hearth's crackle plays in Namula and nothing in TEST GROUNDS. *(the user listens)*
- [ ] On the PS3: level up, buy, open a chest, read the map. *(PS3 log: `quest: level`, `quest: bought`, `quest: chest`)*

## Playtest checks
1. `NAMEGAME_QUEST='slot=3 class=3 world=0 area=0' make mac-run BUTTONS='2|right:1.5|cross|1|l1|1|quit'`: the Thief in Namula, HUD at the top left.
2. `NAMEGAME_QUEST='slot=3 class=1 world=0 area=2' make mac-run BUTTONS='1|left:1.5|4|quit'`: the hero walks into the sea, `YOU FADED`, back at the hearth.
3. `NAMEGAME_QUEST='slot=3 class=0 world=8 area=1' make mac-run BUTTONS='1|right:0.95|up:1|l1|1|quit'`: the Knight on the shaft's first ladder.
4. `NAMEGAME_QUEST='slot=3 class=0 world=8 area=3 level=30 job=3' make mac-run BUTTONS='1|right:2.5|r2|0.15|l1|1|quit'`: the Lion's Roar arc in front of the Knight (class=1..3 for the other tier-3 skills).
5. `NAMEGAME_QUEST='slot=3 class=1 world=0 area=2 names=500' make mac-run BUTTONS='1|left:1.5|4|select|1|select|0.5|l1|1|quit'`: dies in the sea (`dropped 500 names`), warps back to the shore and picks the echo up (`names recovered 500`, `+500 NAMES` on screen).
6. `NAMEGAME_QUEST='slot=3 class=1 world=0 area=0 names=2000' make mac-run BUTTONS='right:0.3|cross+right:0.5|right:0.4|cross+right:0.5|right:3.5|cross+right:0.5|right:0.5|0.3|up|0.3|cross|cross|0.3|down|down|cross|0.2|cross|0.3|l1|1|quit'`: Marra's shop after buying two MANA POTION.
7. `NAMEGAME_QUEST='slot=3 class=0 world=8 area=1' make mac-run BUTTONS='l2+l3|right:1.5|0.3|up|0.3|cross|0.2|cross|0.3|l1|1|quit'`: the TEST GROUNDS map.
8. On the PS3, walk Namula's three areas with every class: steps, ladders and the cliffs feel calm; the jump feels right.

## History
- 2026-11-20: criteria written; Mac checks done.
- 2026-11-20: phase 2 combat criteria; tests and Mac checks done.
- 2026-11-21: phase 3 criteria; 17 checks and Mac checks done.
- 2026-11-21: phase 2 review: two checks added; Mac screenshots of the four tier-3 skills and the death and echo loop; `make ps3` builds.
