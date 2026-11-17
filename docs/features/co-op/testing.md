# Co-op: testing

**Updated:** 2026-11-17 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] A run starts with one player; a joining player 2 has 3 lives, 3 rockets, 0 points, blinks for 2 s, starts 40 px right of the centre, and their bullets are theirs. *(`tests/game_tests.cpp` › secondPlayerJoinsBlinkingWithAFreshRun)*
- [x] The run goes on while one player is still in and ends when both are out. *(`tests/game_tests.cpp` › coOpRunEndsWhenBothPlayersAreOut)*
- [x] A player who is out gets no wave clear bonus. *(`tests/game_tests.cpp` › waveBonusSkipsAPlayerWhoIsOut)*
- [x] Start on a second controller joins player 2 on the PS3. *(PS3 log 2026-11-17: "controller 1 connected", then "player 2 joined at wave 1")*
- [ ] Both HUDs and both ships are easy to tell apart on the TV. *(not proven: no co-op playtest)*
- [x] 60 fps after player 2 joined, including a Triangle stress press. *(PS3 log 2026-11-17: fps 59.9–60.0, slowest frame 5 ms)*

## Playtest checks
1. Start a run with one controller, then press Start on the second: player 2 appears blinking at the right, the log shows "player 2 joined".
2. Let player 1 lose all lives while player 2 plays on; the run continues. Lose player 2's last life: game over shows both scores.
3. Press X at game over: the new run has both ships.

## History
- 2026-11-17: criteria written.
