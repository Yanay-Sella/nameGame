# Bosses: implementation

**Updated:** 2026-11-17 · [Design](design.md) · [Testing](testing.md)

## Approach
`startWave` fills `Game::bosses` (1 or 2, classic or mage) instead of a formation when `isBossWave(wave)`. `updateBoss` flies each to y = 20, sways it in its range, ticks the mage's ball and beam, and, with a living ship, casts attacks on its own timer. Bullets and rocket contact hit them in the existing loops, `explode` hits them by circle-versus-rectangle, and `damageBoss` holds the single death path (score, explosion, both pickups). The wave ends when no alien and no boss is alive, so the normal clear bonus and pause apply once.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/game.h` › `Game::bosses`, `BossType`, `Boss`, `isBossWave`, `bossHealth`, `bossCount`, `bossType` | state and the tested formulas |
| `source/game.cpp` › `startWave` | boss set-up, no formation |
| `source/game.cpp` › `updateBoss` | entry, sway, burst, fan, and the mage's ball and beam |
| `source/game.cpp` › `damageBoss`, `explode`, `updateGame` | damage, death, body contact, wave end |
| `source/game.cpp` › `drawGame` | sprites, flash, ball and beam, summed health bar, `BOSS` banner |
| `source/sprites.h` › `BOSS_A`, `BOSS_B`, `MAGE_A`, `MAGE_B` | 32×20 sprites, two frames each |
| `source/main.cpp` | logs `boss wave n: k type, hp h`, `boss down at wave n ..., k left` and `mage beam at x N` |

## Data and config
- **Config constants:** the `BOSS_*` block in `game.cpp`, named as in the design numbers.
- **Persistence:** none.

## Deviations from design
- Entry speed is the aliens' `ENTRY_SPEED` (150 px/s), so the boss is in place in about a quarter of a second.
- Health bar top edge is y = 16.

## History
- 2026-11-17: implemented in M3.
- 2026-11-17: boss line-up per wave: classic, twin classics, mage, twin mages (D-019).
