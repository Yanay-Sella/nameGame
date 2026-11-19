# Glacier

**Status:** implemented · **Updated:** 2026-11-18 · [GDD](../../GDD.md) · [Jungle](../jungle/design.md) · [Volcano](../volcano/design.md) · [Bosses](../bosses/design.md) · [Implementation](implementation.md) · [Testing](testing.md)

Level 3: a frozen cave of ice monsters where the ship glides on the ice, a yeti that sneezes snowballs, and an ice golem whose ice blocks freeze you in place. It is waves 1 to 20 again, a step harder than the jungle. Pillar 1 holds: the background is darker than the stars, and magenta marks everything deadly.

Pictures from the game: [formation wave](../../work-plan/levels/glacier-wave.png) · [twin yetis](../../work-plan/levels/glacier-twins.png) · [sneeze](../../work-plan/levels/glacier-sneeze.png) · [golem](../../work-plan/levels/glacier-boss.png) · [sprites](../../work-plan/levels/sprites.png)

## Rules
1. In Classic, beating the jungle continues the run here; GLACIER in the level picker plays just this level (its clear is the win). The banner reads `GLACIER` for 2 s, then `WAVE 1`; the HUD reads `GLACIER n`. The wave-20 clear shows `GLACIER CLEAR` for 3 s, then in Classic the [volcano](../volcano/design.md) starts.
2. Formation wave n uses the enemy-wave formulas at difficulty wave n + 10 (D-033); the formation's shots fly 5 px/s faster.
3. Enemies: snowflake creature (1 HP), ice bat (2 HP), frost skull (4 HP), and a roaming heavy, a little yeti riding a cloud (12 HP, as the jungle's heavy beetle). Formation waves get `wave / 6` heavies, at most 3.
4. **Slippery ice:** the ship's speed eases toward the stick's at 5 per second, so it glides into and out of every move.
5. Bosses (n = wave / 5): 5 one yeti, 10 two yetis, 15 one ice golem, 20 two ice golems (HP, sway and timing in Numbers). Entry, twins, health bar, pickups and rewards work as for every boss.
6. The yeti cycles a snowball volley (3 aimed snowballs), an ice block thrown at the nearest ship, and a sneeze: `AH...` for 0.5 s, then `ACHOO!` and a fan of 7 snowballs.
7. The ice golem rolls 30 % volley, 25 % two ice blocks (one at the nearest ship, one at a random point of the ship zone), 20 % snowball fan, 25 % icicle rain: 6 icicles hang shaking at the top for 0.8 s, one per sixth of the screen, then fall. Icicle rain while icicles still hang becomes a fan.
8. **Ice block:** a see-through 24×24 block (both bosses throw it): touching it freezes the ship for 1.6 s inside an ice cube: no moving, shooting or rockets. Each fresh R1 press takes 0.15 s off. It doesn't kill, works on a respawning ship, and a second block refreshes the freeze.

## The level bundle
| Part | Glacier |
|---|---|
| Theme and mood | a frozen cave: cold, quiet, a little comic |
| Palette | 7 new characters in `source/sprites.h` (cave blues, pale, mid and deep ice) |
| Background | far layer 7 px/s (faceted ice pillars, frost), near layer 16 px/s (jagged ice walls, shelves with icicles), 30 dim snowflakes |
| Enemy roster | snowflake, ice bat, frost skull, yeti on a cloud |
| Ship skin | pale ice-crystal ship (D-031) |
| Bosses | yeti (180 HP) at 5, twin yetis at 10, ice golem (340 HP) at 15, twin golems at 20 |

## Numbers
| Boss wave | Bosses | HP each | Sway px/s | Attack every | Reward each |
|---|---|---|---|---|---|
| 5 | 1 yeti | 180 | 50 | 2.1 s | 45,000 |
| 10 | 2 yetis | 180 | 60 | 1.9 s | 50,000 |
| 15 | 1 ice golem | 340 | 45 | 1.7 s × 0.6-1.4 | 55,000 |
| 20 | 2 ice golems | 340 | 55 | 1.5 s × 0.6-1.4 | 60,000 |

| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Ice grip | 5 | per s | gliding is fun, not mushy |
| Snowball | 90 aimed; fans 60 sideways, 75-80 down | px/s | |
| Ice block | 80 aimed, 24×24 | px/s, px | readable as "not deadly but avoid" |
| Freeze / mash | 1.6 / 0.15 per press | s | mashing feels worth it |
| Icicles | 6, hang 0.8, fall 140 | s, px/s | the lanes leave a gap |

## Edge cases
- A frozen ship can still be shot; the freeze is cleared when it dies.
- A respawning (blinking) ship is still frozen by a block.
- Ice blocks, icicles and snowballs left when the golem dies keep flying; they are cleared when the next level starts.

## Open questions
- Unlock levels in order (user, later). A glacier ship skin, song and XMB art are out of scope.

## History
- 2026-11-18: designed and implemented (D-024 to D-027).
