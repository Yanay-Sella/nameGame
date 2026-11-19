# Level buffs: implementation

**Updated:** 2026-11-18 · [Design](design.md) · [Testing](testing.md)

## Approach
A buff is a bit of `Player::buffs`; `PICKUP_BUFF` carries its level. `Player::strongAttack` picks what R2 fires in the existing rocket block. The frost wave sets `Game::freezeTimer`, which makes `updateGame` skip the sway, the aliens, the bosses and dives. The shield is a `Player` flag popped in the ship-hit block, and its bubbles are a `Game` list updated after the rockets. Skins come from a `SHIP_SKINS` table indexed by level.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/sprites.h` | ship skins, wand, bananas, fire bullet, shield bubbles, buff icons |
| `source/game.h` | `BUFF_*` bits, `Player` buff fields, `PICKUP_BUFF`, `ShieldBubble`, freeze fields, `Actions::cycle` |
| `source/input.cpp` | Square (button 8) sets `cycle` |
| `source/game.cpp` › `damageBoss` | drops the buff |
| `source/game.cpp` › `killPlayer` | loses every buff |
| `source/game.cpp` › `frostWave`, `spawnShieldBubbles` | the glacier and deep-sea attacks |
| `source/game.cpp` › `updatePlayer` | magic beam, volcano fire rate, the R2 block |
| `source/game.cpp` › `updateGame` | Square cycling, freeze, bubbles, shield pop, pickup collection |
| `source/game.cpp` › `drawGame` | skins, wand, beam, ice, bananas, ring, bubbles, HUD icons |
| `source/main.cpp` | test mode grants all buffs; log lines |

## Data and config
- **Config constants:** the "Level buffs" block in `game.cpp`.
- **Persistence:** none.

## Deviations from design
- The `player %d buffs` log compares against the last logged value kept across frames (not a before/after snapshot), so the test-mode grant is logged too.

## History
- 2026-11-18: implemented (D-030, D-031).
