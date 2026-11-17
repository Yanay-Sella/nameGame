# Enemy waves: implementation

**Updated:** 2026-11-17 · [Design](design.md) · [Testing](testing.md)

## Approach
`startWave` builds the formation (rows from `formationRows`, types by row and wave) with every alien `ALIEN_ENTERING` above the screen. `updateAlien` is a small state machine: entering → in formation → diving → returning → in formation. Only aliens in formation or diving may shoot. From wave 3 a dive starts every `diveInterval(wave)` seconds if no alien is already diving.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/game.cpp` › `startWave`, `formationRows` | formation layout and alien types |
| `source/game.cpp` › `updateAlien`, `startDive` | fly-in, sway, dives, return, enemy shots |
| `source/game.cpp` › `updateGame` | wave clear bonus, pause between waves, next wave |
| `source/game.cpp` › `shotChance`, `enemyShotSpeed`, `diveInterval` | the difficulty formulas |

## Data and config
- **Config constants:** `FORMATION_COLUMNS`, `COLUMN_SPACING`, `ROW_SPACING`, `FORMATION_TOP`, `SWAY_RANGE`, `WAVE_PAUSE`, `BANNER_TIME`, `ALIEN_HP`, `ALIEN_SCORE`, `WAVE_CLEAR_BONUS`, `ENEMY_SHOT_SPEED`, `SHOT_SPEEDUP_AFTER_WAVE`, `ENEMY_SHOT_SPEED_STEP`, `ENEMY_SHOT_MAX_SPEED`, `MORE_ARMOUR_WAVE`, `TWO_DIVERS_WAVE`, `ENTRY_DISTANCE`, `ENTRY_SPEED`, `ENTRY_COLUMN_DELAY`, `ENTRY_ROW_DELAY`, `DIVE_SPEED`, `DIVE_STEER`, `DIVE_MAX_SIDEWAYS`, `RETURN_SPEED` (all `game.cpp`).
- **Persistence:** none.

## Deviations from design
- Rule 7 (every 5th wave is a boss) waits for the Bosses feature in M3; until then waves 5, 10, … are normal formations.
- The formation sways 40 px either side of the centre (`SWAY_RANGE`), a number the design didn't give.

## History
- 2026-11-17: implemented in M2.
