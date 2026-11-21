# Co-op: testing

**Updated:** 2026-11-21 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] A run starts with one player; a joining player 2 has 3 lives, 3 rockets, 0 points, blinks for 2 s, starts 40 px right of the centre, and their bullets are theirs. *(`tests/game_tests.cpp` › secondPlayerJoinsBlinkingWithAFreshRun)*
- [x] The run goes on while one player is still in and ends when both are out. *(`tests/game_tests.cpp` › coOpRunEndsWhenBothPlayersAreOut)*
- [x] A player who is out gets no wave clear bonus. *(`tests/game_tests.cpp` › waveBonusSkipsAPlayerWhoIsOut)*
- [x] In Classic 1–2 Players, an eliminated player returns invincible with three lives after the last boss in a wave falls. The check covers regular-wave clears, the first boss in a twin wave, a full party wipe and preserving score and equipment on return. *(`tests/game_tests.cpp` › bossWaveRevivesEliminatedTeammates)*
- [x] Start on a second controller joins player 2 on the PS3. *(PS3 log 2026-11-17: "controller 1 connected", then "player 2 joined at wave 1")*
- [x] Both HUDs and both ships are easy to tell apart on the TV. *(player report 2026-11-17)*
- [x] 60 fps after player 2 joined, including a Triangle stress press. *(PS3 log 2026-11-17: fps 59.9–60.0, slowest frame 5 ms)*

## Classic variants

- [x] Party limits are two and four respectively. The fourth ship can reach the enlarged arena's edge and own its bullets. *(`tests/game_tests.cpp` › `classicVariantsCapThePartyAndReadPlayerFour`)*
- [x] All five worlds keep 20 waves. The expanded run increases each spawn count and HP, uses distinct boss lanes and wins on the Deep Sea clear. *(`tests/game_tests.cpp` › `expandedClassicScalesEveryWave`)*
- [x] The upgrade-drop sample is within the 1% and 2% ranges respectively. The rocket drop sample stays near 2% in both variants. *(`tests/game_tests.cpp` › `killDropsFollowTheRoll`)*
- [x] With two simulated pads, the lobby blocks play. It admits three and four, offers a starting party size, and shows a nonconsecutive port. *(Mac scripted input and screenshots, 2026-11-21)*
- [x] A spare simulated pad joins in play. Unplugging a joined pad pauses and blocks Resume until its port reconnects. *(Mac scripted input and screenshots, 2026-11-21)*
- [ ] Verify four physical controllers, PS button menu exit, reconnect, TV readability and 60 fps in the expanded arena. *(PS3 playtest pending)*

## Ship roles

- [x] Picks apply only to expanded parties of at least three. Joining third preserves the run's score and joining fourth preserves existing roles. *(`tests/game_tests.cpp` › `rolesRequireThreeExpandedPlayers`)*
- [x] Fighter and Mage share base move speed and fire cadence. Tank is slower with the highest single-shot damage. Assassin is faster with lower damage per shot. *(`tests/game_tests.cpp` › `rolesApplyMovementFireCadenceAndDamage`)*
- [x] Tank loses armor before a life, gets armor back on respawn, and launches a rocket that damages enemies outside the standard radius. *(`tests/game_tests.cpp` › `tankArmorAndHeavyRocket`)*
- [x] Assassin phases through an enemy shot and an ice block, cannot extend the active phase, and can be hit again after five seconds. *(`tests/game_tests.cpp` › `assassinPhasesThroughAttacksForFiveSeconds`)*
- [x] Mage fires seven orbs. A single orb deals 4, 2, 1 and 0.5 HP to four consecutive targets. *(`tests/game_tests.cpp` › `mageOrbsPierceEachEnemyOnceAndHalveDamage`)*
- [x] Three and four simulated controllers choose independently. Canceling a third-player join returns to play, and a reconnecting fourth controller can complete selection. The lobby, four ship sprites and role effects render in the Mac build. *(Mac scripts and reviewed screenshots, 2026-11-21)*
- [ ] Check ship and effect readability on a TV and role balance during a full four-player run. *(PS3 playtest pending)*

## Playtest checks
1. Start a run with one controller, then press Start on the second: player 2 appears blinking at the right, the log shows "player 2 joined".
2. Let player 1 lose all lives while player 2 plays on; the run continues. Lose player 2's last life: game over shows both scores.
3. Press X at game over: the new run has both ships.
4. Player 2's ship is gold (`playerTwoShipIsGold`, ticked) and looks right on the PS3 (unticked: PS3 look check).
5. In Classic 1–2 Players, let one player run out of lives, then clear the boss wave with the other. Check that the returning ship blinks and its HUD shows two spare hearts.


## History
- 2026-11-17: criteria written.
- 2026-11-21: added the Classic variant checks.
