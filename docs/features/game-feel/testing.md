# Game feel: testing

**Updated:** 2026-11-17 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] Overlapping events in one step take the longest hit-stop (0.12 s, not the 0.18 s sum), then the game runs again. *(`tests/game_tests.cpp` › overlappingHitStopsTakeTheLongest; events can't happen during a freeze, so overlap means the same step)*
- [x] Trauma 1 reaches 0 inside 1 / 1.5 s. *(`tests/game_tests.cpp` › traumaDecaysToZero)*
- [x] An R2 press during a stop launches after it. *(`tests/game_tests.cpp` › inputDuringHitStopStillFires)*
- [x] Three blasts within 0.2 s give one screen flash; a later one flashes again. *(`tests/game_tests.cpp` › flashesAreRateLimited)*
- [x] Reduce flashing draws no flash. *(`tests/game_tests.cpp` › reducedFlashingDrawsNoFlash)*
- [x] Shake strength 0 draws no offset. *(`tests/game_tests.cpp` › zeroShakeDrawsNoOffset)*
- [x] `shake=` and `flash=` survive a save and load; shake clamps to 100. *(`tests/game_tests.cpp` › shakeAndFlashSettingsSurviveASave)*
- [x] Shooting and ordinary kills add no trauma. *(`tests/game_tests.cpp` › shootingAndKillsNeverShake)*
- [x] Shake moves the play field only, the HUD stays still; flash and reduced flash look as designed. *(Mac render 2026-11-17)*
- [x] Blasts, deaths and the boss death feel punchy, not nauseating; holding R1 doesn't buzz; stress test holds 60 fps. *(player report 2026-11-17; PS3 log 2026-11-17: fps 59.9 or better through 15 Triangle presses)*
- [x] Shooting no longer shakes; blasts still do. *(player report 2026-11-17 on the final build; rule: `tests/game_tests.cpp` › shootingAndKillsNeverShake)*
- [x] The stress test stays well inside the 16.7 ms frame. *(PS3 log 2026-11-17: worst work 11 ms, the same as the M2 stress result; fps 59.9 or better)*
- [x] The shake and flash options are in the settings screen and are saved. *(PS3 log 2026-11-17: shake steps 100→0→100 and flashing toggles, each `(saved)`; player report 2026-11-17: all fine)*
- [x] Input feels right through every freeze. *(player report 2026-11-17: feels good; R2 buffering by `tests/game_tests.cpp` › inputDuringHitStopStillFires)*

## History
- 2026-11-17: criteria written.
