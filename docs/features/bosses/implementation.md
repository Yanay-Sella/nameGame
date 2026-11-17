# Bosses: implementation

**Updated:** 2026-11-17 · [Design](design.md) · [Testing](testing.md)

## Approach
`startWave` sets up `Game::boss` instead of a formation when `isBossWave(wave)`. `updateBoss` flies it to y = 20, sways it and, with a living ship, alternates an aimed burst and a fan on a timer. Bullets and rocket contact hit it in the existing loops, `explode` hits it by circle-versus-rectangle, and `damageBoss` holds the single death path (score, explosion, both pickups). The wave ends when no alien and no boss is alive, so the normal clear bonus and pause apply.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/game.h` › `Boss`, `isBossWave`, `bossHealth` | state and the tested formulas |
| `source/game.cpp` › `startWave` | boss set-up, no formation |
| `source/game.cpp` › `updateBoss` | entry, sway, burst and fan |
| `source/game.cpp` › `damageBoss`, `explode`, `updateGame` | damage, death, body contact, wave end |
| `source/game.cpp` › `drawGame` | sprite, flash, health bar, `BOSS` banner |
| `source/sprites.h` › `BOSS_A`, `BOSS_B` | 32×20 sprite, two frames |
| `source/main.cpp` | logs `boss wave n, hp h` and `boss down at wave n` |

## Data and config
- **Config constants:** the `BOSS_*` block in `game.cpp`, named as in the design numbers.
- **Persistence:** none.

## Deviations from design
- Entry speed is the aliens' `ENTRY_SPEED` (150 px/s), so the boss is in place in about a quarter of a second.
- Health bar top edge is y = 16.

## History
- 2026-11-17: implemented in M3.
