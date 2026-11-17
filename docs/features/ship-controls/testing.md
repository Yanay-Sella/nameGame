# Ship controls: testing

**Updated:** 2026-11-17 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] Holding R1 fires one bullet every 0.12 s: 9 shots in the first second. *(`tests/game_tests.cpp` › holdingFireShootsEvery012Seconds)*
- [x] The ship can't leave the movement zone (x 0–304, y 100–166). *(`tests/game_tests.cpp` › shipStaysInsideMovementZone)*
- [x] The stick deadzone is radial and rescaled: nothing inside 0.2, a slow start at its edge, diagonals capped at length 1. *(`tests/game_tests.cpp` › deadzoneIsRadialAndRescaled)*
- [x] R1 is button 12 and the stick moves the ship on the PS3. *(PS3 log 2026-11-17: button 12 presses, score rising)*
- [x] Movement and shooting feel tight. *(PS3 playtest 2026-11-17: user said "feels good")*
- [x] D-pad presses (buttons 0–3) and Triangle (11) reach the game. *(PS3 log 2026-11-17: presses of 0, 1, 2, 3 and 11)*
- [ ] The D-pad moves the ship at full speed in 8 directions. *(not proven: the presses were logged, the movement wasn't reported)*
- [ ] An unplugged controller pauses the game. *(not built: M4)*

## Playtest checks
1. On the PS3, move with the D-pad in all 8 directions; the ship moves at full speed and diagonals are no faster.

## History
- 2026-11-17: criteria written.
