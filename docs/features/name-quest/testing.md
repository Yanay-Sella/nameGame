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

- [x] The Overseer telegraphs every shot, slam and summon (0.4 s; the slam 0.8 s with a marker); all 4 attacks and phase 2 appear. *(`tests/quest_tests.cpp` › overseerTellsBeforeEveryAttack)*
- [x] The Star Mage's beam column is marked for 1.2 s and burns exactly there; phase 2 marks and burns two columns 64 apart. *(`tests/quest_tests.cpp` › starMageBeamMarksItsColumn)*
- [x] A boss fight closes the edges and portals and opens them when the boss dies; the hero's death ends the fight. *(`tests/quest_tests.cpp` › bossFightLocksAndUnlocksExits)*
- [x] The mini-boss stays down until a rest or death; the big boss stays dead. *(`tests/quest_tests.cpp` › bigBossStaysDeadMiniBossReturns)*
- [x] The Star Mage's death sets its bit, lights world 2's entry, gives 150 x names and shows `GREAT NAME TAKEN`. *(`tests/quest_tests.cpp` › bossDeathLightsTheNextWorld)*
- [x] An Archer's basic arrow hits every world 1 ground enemy (the crawler was once too low: its arrows flew over it). *(`tests/quest_tests.cpp` › arrowsHitEveryGroundEnemy)*
- [x] World 1's layers keep the value rules; its props never block spots; every ground shape has art. *(`tests/quest_tests.cpp` › layersObeyTheValueRules, propsNeverBlockSpots, everyGroundShapeHasArt)*
- [x] The user approves world 1's style frame (`shots/world1-style-frame-1.png`, `-2.png`, sent 2026-11-21). *(user, 2026-11-21)*
- [x] The user approves world 2's style frame (`shots/style-w2-entry.png`, `-right.png`, `-characters.png`, sent 2026-11-21). *(user, 2026-11-21)*
- [x] The user approves world 3's style frame (`shots/style-w3-entry.png`, `-right.png`, `-characters.png`, sent 2026-11-21). *(user, 2026-11-21)*
- [x] Everything in world 1 is reachable and no spot is a soft lock; with the gate closed, the dome and the med bay checkpoint are out of reach and the hangar is not. *(`tests/quest_tests.cpp` › everythingIsReachable, gateGuardsTheSecondHalf)*
- [x] World 1 keeps spec §4's layout rules (main path 13: 0 1 2 3 4 5 9 10 11 12 17 18 19; the hangar 7th, the gate next, the second checkpoint 3 before the dome; 3 dead ends, a loop, a shortcut, 2 portals, 3 tall areas, the surveyors) and §8's counts per band; each world 1 chest appears once, the shard, page and STAR IRON in dead ends. *(`tests/quest_tests.cpp` › worldLayoutRulesHold, chestsMatchTheWorldList)*
- [x] A locked arena's ladder down holds the hero instead of killing them. *(`tests/quest_tests.cpp` › bossFightLocksAndUnlocksExits)*
- [x] `make quest-maps`: all 40 world 1 screens read; nothing unreadable. *(2026-11-21)*
- [x] Boss fights at the reference (level 4, gear 0), with no damage taken: the Overseer falls in 42 s (Knight) and 34 s (Thief); the Archer and Magician win from one catwalk without moving in 273 s and 231 s; the Star Mage in 146 s (Thief), 251 s (Archer) and 205 s (Magician) from one catwalk. A scripted bot that stands under the bosses dies to contact and slams: no verdict on difficulty, for the playtest and phase 9. *(scratch bots on the real game code)*
- [x] On the Mac: the Overseer slams and takes hits; the Star Mage fades between anchors and summons crawlers (TEST GROUNDS areas 4 and 5). *(`make mac-run` 2026-11-21)*
- [ ] On the PS3: load slot 1, TRAVEL to THE DRIFTING STATION ENTRY, walk right. *(PS3 log: `quest: travel to 1.0`, `quest: area 1.0 DOCKING RING`; fps ≈ 60)*
- [ ] On the PS3: the Star Mage in god mode from a dev save (`quest3.txt`, phase 4 step 10): its beam on screen. *(PS3 log: `quest: boss THE STAR MAGE start`, `phase 2`, `down after`; fps ≥ 59 during beams)*
- [x] Quitting the app saves the slot: `quest: save slot 3 ok` before `nameGame exiting`, the playtime kept. *(`make mac-run` with a scripted `quit`, 2026-11-21)*
- [ ] On the PS3: Quit Game from the PS button menu in NAME QUEST, then load the slot: the playtime and names are kept. *(PS3 log: `quest: save slot n ok` before `nameGame exiting`)*
- [ ] On the PS3: no white noise left under Namula and the DRIFTING STATION ENTRY (the hums removed). *(the user listens)*
- [ ] On the PS3: with `quest_namula.ogg`, `quest_world1.ogg` and `quest_boss.ogg` packaged, each plays in its place, the boss song during a fight and the world's song after; a missing song falls back to the ambience. *(the user listens; PS3 log: `audio: quest_world1.ogg loaded`)*
- [ ] The user plays world 1 from a new slot: a first run takes at least 2 hours; each boss takes more than one try; never stuck. Notes: *(user)*
- [x] Coconuts drop 0.3 s after the hero passes under a palm (20 %, 200 px/s) and regrow in 4 s; the tree monkey throws one banana on landing; a peel slides the hero 0.5 s; the stink mist takes 3 % per second; a jump clears the Silverback's wave. *(`tests/quest_tests.cpp` › coconutDropsWhenHeroPassesUnder, treeMonkeyThrowsOnLanding, peelSlidesTheHero, mistHurtsOverTime, silverbackWaveCanBeJumped)*
- [x] An icicle falls 0.25 s after the hero comes within 12 px below (20 %, 280 px/s) and stays gone until a rest; crumbling ice breaks after 0.6 s and returns 4 s later; world 3's wind pushes 40 px/s and turns every 3 s; mashing X frees a frozen hero sooner than waiting 1.2 s; the Titan's phase 2 cracks the middle 10 floor tiles and starts the blizzard. *(`tests/quest_tests.cpp` › icicleFallsOnceUntilRest, crumblingIceBreaksAndReturns, windFlipsEveryThreeSeconds, frozenHeroBreaksFreeSoonerByMashing, titanPhaseTwoCracksTheFloor)*
- [x] The four world 2-3 bosses use every attack of both phases. *(`tests/quest_tests.cpp` › worldTwoBossesUseEveryAttack, worldThreeBossesUseEveryAttack)*
- [x] Garrick asks for a RED POTION and thanks a hero who carries one; Ilsa and Lio speak of the world 2 and 3 Great Names. *(`tests/quest_tests.cpp` › worldTwoAndThreeStoryLines)*
- [x] Worlds 2 and 3: everything is reachable with no soft lock; the gate guards the second half; spec §4's layout rules and §8's counts hold (main paths 13 areas each); each chest appears once, shards, pages and the FROZEN FEATHER in dead ends; arrows hit every ground enemy; the layers keep the value rules. *(`tests/quest_tests.cpp` › everythingIsReachable, gateGuardsTheSecondHalf, worldLayoutRulesHold, chestsMatchTheWorldList, arrowsHitEveryGroundEnemy, layersObeyTheValueRules)*
- [x] `make quest-maps`: all 80 world 2-3 screens read (the icicles were moved under low outcrops so they show below the HUD). *(2026-11-21)*
- [x] On the Mac: the Monkey King leaps to its tree and throws bananas; the Yeti and the Titan fight at 60 fps; Garrick's line fits the box. *(`make mac-run` 2026-11-21)*
- [ ] On the PS3: a dev save at world 2 area 19 (phase 4 step 10 method): the Silverback in god mode, one phase-2 screenshot. *(PS3 log: `quest: boss SILVERBACK start`, `phase 2`, `down after`; fps ≥ 59)*
- [ ] On the PS3: a dev save at world 3 area 19: the Glacier Titan in god mode, one phase-2 screenshot. *(PS3 log: `quest: boss THE GLACIER TITAN start`, `phase 2`, `down after`; fps ≥ 59)*
- [ ] The user plays world 2 from its entry: never stuck; the coconuts, peels and wave feel fair; each boss takes more than one try. Notes: *(user)*
- [ ] The user plays world 3 from its entry: never stuck; the ice, wind and icicles read in time; the frozen block feels fair. Notes: *(user)*

## Playtest checks
1. `NAMEGAME_QUEST='slot=3 class=3 world=0 area=0' make mac-run BUTTONS='2|right:1.5|cross|1|l1|1|quit'`: the Thief in Namula, HUD at the top left.
2. `NAMEGAME_QUEST='slot=3 class=1 world=0 area=2' make mac-run BUTTONS='1|left:1.5|4|quit'`: the hero walks into the sea, `YOU FADED`, back at the hearth.
3. `NAMEGAME_QUEST='slot=3 class=0 world=8 area=1' make mac-run BUTTONS='1|right:0.95|up:1|l1|1|quit'`: the Knight on the shaft's first ladder.
4. `NAMEGAME_QUEST='slot=3 class=0 world=8 area=3 level=30 job=3' make mac-run BUTTONS='1|right:2.5|r2|0.15|l1|1|quit'`: the Lion's Roar arc in front of the Knight (class=1..3 for the other tier-3 skills).
5. `NAMEGAME_QUEST='slot=3 class=1 world=0 area=2 names=500' make mac-run BUTTONS='1|left:1.5|4|select|1|select|0.5|l1|1|quit'`: dies in the sea (`dropped 500 names`), warps back to the shore and picks the echo up (`names recovered 500`, `+500 NAMES` on screen).
6. `NAMEGAME_QUEST='slot=3 class=1 world=0 area=0 names=2000' make mac-run BUTTONS='right:0.3|cross+right:0.5|right:0.4|cross+right:0.5|right:3.5|cross+right:0.5|right:0.5|0.3|up|0.3|cross|cross|0.3|down|down|cross|0.2|cross|0.3|l1|1|quit'`: Marra's shop after buying two MANA POTION.
7. `NAMEGAME_QUEST='slot=3 class=0 world=8 area=1' make mac-run BUTTONS='l2+l3|right:1.5|0.3|up|0.3|cross|0.2|cross|0.3|l1|1|quit'`: the TEST GROUNDS map.
8. `NAMEGAME_QUEST='slot=3 class=0 world=1 area=0 level=4' make mac-run BUTTONS='right:0.7|l1|1|quit'`: world 1's style frame.
9. `NAMEGAME_QUEST='slot=3 class=2 world=8 area=4 level=4' make mac-run BUTTONS='l2+l3|right:6|r1:3|l1|1|quit'`: the Overseer (area=5: the Star Mage).
10. On the PS3, walk Namula's three areas with every class: steps, ladders and the cliffs feel calm; the jump feels right.
11. `NAMEGAME_QUEST='slot=3 class=0 world=2 area=0 level=11 gear=1' make mac-run BUTTONS='right:0.7|l1|1|quit'`: world 2's style frame (world=3 level=17 gear=2: world 3's).
12. `NAMEGAME_QUEST='slot=3 class=1 world=2 area=6 level=11 gear=1' make mac-run BUTTONS='l2+l3|9|l1|1|quit'`: the Monkey King on its tree (area=19: the Silverback; world=3 area=6 or 19 level=17 gear=2: the Yeti, the Titan).
13. `NAMEGAME_QUEST='slot=3 class=0 world=2 area=12 level=11 gear=1' make mac-run BUTTONS='2|right:0.55|1|up|1.5|l1|1|quit'`: Garrick's line.

## History
- 2026-11-20: criteria written; Mac checks done.
- 2026-11-20: phase 2 combat criteria; tests and Mac checks done.
- 2026-11-21: phase 3 criteria; 17 checks and Mac checks done.
- 2026-11-21: phase 4 (in progress): 5 boss checks, world 1 art checks, style frame sent for the user's OK.
- 2026-11-21: phase 4: world 1's areas; layout, gate and chest checks; maps read; boss fight lengths measured. PS3 checks open.
- 2026-11-21: phase 2 review: two checks added; Mac screenshots of the four tier-3 skills and the death and echo loop; `make ps3` builds.
- 2026-11-21: phase 5: worlds 2 and 3: 12 checks for their hazards, bosses and story; both worlds in the layout, reach and chest checks; maps read; Mac checks done. PS3 checks and the user's playtests open.
