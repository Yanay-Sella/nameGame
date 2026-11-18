# Volcano

**Status:** implemented · **Updated:** 2026-11-18 · [GDD](../../GDD.md) · [Glacier](../glacier/design.md) · [Deep sea](../deep-sea/design.md) · [Bosses](../bosses/design.md) · [Implementation](implementation.md) · [Testing](testing.md)

Level 4: a chasm of lava and fire monsters. Its bosses throw fireballs and lava drops; a lava drop slows the ship down but makes its bullets hit twice as hard for 3 seconds, so players choose when to grab one. The fire lord also opens eruptions under the ship. Harder again than the glacier.

Pictures from the game: [formation wave](../../work-plan/levels/volcano-wave.png) · [twin imps](../../work-plan/levels/volcano-twins.png) · [fire lord](../../work-plan/levels/volcano-boss.png) · [sprites](../../work-plan/levels/sprites.png)

## Rules
1. In Classic, beating the glacier continues the run here; VOLCANO in the level picker plays just this level (its clear is the win). The banner reads `VOLCANO`; the HUD reads `VOLCANO n`. The wave-20 clear shows `VOLCANO CLEAR` for 3 s, then in Classic the [deep sea](../deep-sea/design.md) starts.
2. Formation wave n uses level 1's formulas at wave n + 11; the formation's shots fly 10 px/s faster.
3. Enemies: living ember (1 HP), fire bat (2 HP), magma skull (4 HP), and a roaming phoenix (12 HP). Formation waves get `wave / 5` heavies, at most 3.
4. Bosses: 5 one fire imp, 10 two imps, 15 one fire lord, 20 two fire lords.
5. The fire imp cycles a fireball volley (3 aimed), a lava drip (3 drops, sideways −30, 0, 30, down 60 px/s) and a fireball fan (5).
6. The fire lord rolls 30 % volley, 20 % fan, 25 % lava rain (5 drops, one per fifth, from the top at 70 px/s), 25 % eruption: a bubbling pool and a dotted magenta guide line at the nearest ship's x for 1.0 s, then a 16 px column of fire from the bottom to the top for 0.6 s that kills. An eruption while its own is active becomes a fan.
7. **Lava drop:** touching it scorches the ship for 3 s: 0.6× speed, but each bullet does 2 damage (orange fire bolts), flames trail the ship and `HOT!` shows for 0.6 s. A new drop refreshes the 3 s. It doesn't kill and works on a respawning ship; the damage is decided when the bullet hits. Rockets are unchanged.

## The level bundle
| Part | Volcano |
|---|---|
| Theme and mood | a lava chasm: hot, angry, loud |
| Palette | 8 new characters (rock browns, glowing cracks, lava river, hot orange, white-hot, deep red) |
| Background | far layer 8 px/s (rock, three meandering lava rivers, glowing cracks), near layer 18 px/s (rock walls with glowing seams), 20 rising embers |
| Enemy roster | ember, fire bat, magma skull, phoenix |
| Ship skin | unchanged |
| Bosses | fire imp (200 HP) at 5, twin imps at 10, fire lord (380 HP) at 15, twin fire lords at 20 |

## Numbers
| Boss wave | Bosses | HP each | Sway px/s | Attack every | Reward each |
|---|---|---|---|---|---|
| 5 | 1 imp | 200 | 55 | 2.0 s | 65,000 |
| 10 | 2 imps | 200 | 65 | 1.8 s | 70,000 |
| 15 | 1 fire lord | 380 | 50 | 1.6 s × 0.6-1.4 | 75,000 |
| 20 | 2 fire lords | 380 | 60 | 1.4 s × 0.6-1.4 | 80,000 |

| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Fireball | 95 aimed; fan 60 sideways, 80 down | px/s | |
| Scorch | 3 s, 0.6× speed, 2 damage | | worth grabbing, risky to hold |
| Eruption | 1.0 s warning, 0.6 s, 16 px | | warning is readable |

## Edge cases
- A scorched ship that dies loses the scorch.
- A lava drop on a respawning ship still scorches it, so the reward is open to a careful player.
- A fire lord that dies cancels its eruption (its warning and column vanish with it).

## Open questions
- Should a scorched ship's rocket also hit harder? (No for now: pillar 2.)

## History
- 2026-11-18: designed and implemented (D-024 to D-027).
