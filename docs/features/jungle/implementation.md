# Jungle: implementation

**Updated:** 2026-11-18 · [Design](design.md) · [Testing](testing.md)

## Approach
`Game::level` (1 space, 2 jungle) selects the sprite table, the background, the boss line-up and the HUD label. Formation numbers read `difficultyWave(level, wave)`. Heavy beetles are aliens in a new `ALIEN_ROAMING` state. Bananas and coconuts are `Thrown`, peels and poop in flight are `Lob`, landed peels are `Peel`, stink mists are `Mist`; ships get slide and stun timers and an `inMist` flag that turns their bullets into heals against gorillas. The background is two 320×360 tiles generated once by `buildJungle` (same algorithm as the mockup script) and copied per row each frame.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/sprites.h` | 15 palette characters, 19 sprites |
| `source/game.h` | new structs, `level`, `sceneryTime`, `difficultyWave`, `heavyCount` |
| `source/game.cpp` › `startWave`, `updateAlien`, `updateBoss`, `updatePlayer`, `updateGame` | waves, heavies, bosses, hazards, level continue |
| `source/game.cpp` › `buildJungle`, `drawBackground`, `drawGame` | background and drawing |
| `source/audio.cpp` | 7 new sounds |
| `source/main.cpp` | menu item, `runLevel`, logs |
| `docs/work-plan/jungle/make_jungle_mockups.py` | the art source and mockups |

## Data and config
- **Config constants:** the `Jungle` block in `game.cpp`.
- **Persistence:** none new.

## Deviations from design
- None.

## History
- 2026-11-18: implemented (D-020 to D-023).
