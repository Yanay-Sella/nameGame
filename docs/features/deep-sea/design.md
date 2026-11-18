# Deep sea

**Status:** implemented · **Updated:** 2026-11-18 · [GDD](../../GDD.md) · [Volcano](../volcano/design.md) · [Bosses](../bosses/design.md) · [Implementation](implementation.md) · [Testing](testing.md)

Level 5, the last: dark water full of scary fish, a current that pushes the ship sideways, a big anglerfish that spits bubbles, and a fisherman fish in a big yellow fisherman's hat that tries to hook you. A cast that catches nothing sometimes brings up an old boot, which it throws at you. The hardest level.

Pictures from the game: [formation wave](../../work-plan/levels/sea-wave.png) · [twin anglers](../../work-plan/levels/sea-twins.png) · [fisherman and hook](../../work-plan/levels/sea-boss.png) · [sprites](../../work-plan/levels/sprites.png)

## Rules
1. In Classic, beating the volcano continues the run here; DEEP SEA in the level picker plays just this level. The banner reads `DEEP SEA`; the HUD reads `DEEP SEA n`. Killing the wave-20 bosses shows `DEEP SEA CLEAR` for 3 s, then the end screen titled `DEEP SEA CLEAR` in yellow. Play again replays the same mode and level.
2. Formation wave n uses level 1's formulas at wave n + 14; the formation's shots fly 15 px/s faster.
3. Enemies: piranha (1 HP), pufferfish (2 HP), lionfish (4 HP), and a roaming shark (12 HP). Formation waves get `wave / 4` heavies, at most 3.
4. **Current:** the water pushes every ship that can move sideways by `18 · sin(0.45 · time)` px/s, one slow back-and-forth every 14 s. The background bubbles drift with it.
5. Bosses: 5 one anglerfish, 10 two anglerfish, 15 one fisherman fish, 20 two fishermen.
6. The anglerfish cycles a bubble stream (5 aimed bubbles 0.12 s apart, 70 px/s, wobbling), a big bubble (aimed at 45 px/s; after 1.4 s or at y 150 it pops into 8 bubbles flying out at 60 px/s) and a bubble fan (7, sideways −70…70, down 65).
7. The fisherman rolls 35 % stream, 15 % big bubble, 20 % fan, 30 % hook cast (a stream while its hook is out).
8. **Hook:** cast at the nearest ship's x, it drops from the rod tip at 110 px/s to 16 px above the bottom, waits 0.3 s and reels back at 160 px/s. A ship that isn't blinking and touches it on the way down or while it waits is hooked: `GOT ONE!`, the ship is lifted 24 px (never above y 100) and dangles 1.6 s, unable to move, shoot or fire rockets, shaking with dizzy stars; then it is let go. A hooked ship can still be shot.
9. **The old boot:** a cast that caught nothing brings up an old boot 35 % of the time; `OLD BOOT` shows and it is thrown at the nearest ship (85 px/s, kills).

## The level bundle
| Part | Deep sea |
|---|---|
| Theme and mood | the deep: dark, scary, with one silly fisherman |
| Palette | 9 new characters (deep water, kelp, light rays, teal fish, pale belly, lure glow) |
| Background | far layer 6 px/s (dithered light rays, distant rocks), near layer 15 px/s (rock walls, swaying kelp), 16 rising bubbles that drift with the current |
| Enemy roster | piranha, pufferfish, lionfish, shark |
| Ship skin | unchanged |
| Bosses | anglerfish (220 HP) at 5, twins at 10, fisherman fish (420 HP) at 15, twins at 20 |

## Numbers
| Boss wave | Bosses | HP each | Sway px/s | Attack every | Reward each |
|---|---|---|---|---|---|
| 5 | 1 anglerfish | 220 | 55 | 1.9 s | 85,000 |
| 10 | 2 anglerfish | 220 | 65 | 1.7 s | 90,000 |
| 15 | 1 fisherman | 420 | 50 | 1.5 s × 0.6-1.4 | 95,000 |
| 20 | 2 fishermen | 420 | 60 | 1.3 s × 0.6-1.4 | 100,000 |

| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Current | 18 sin(0.45 t) | px/s | noticeable, never a fight with the stick |
| Hook | 110 down, 0.3 s wait, 160 up; lift 24 px, stun 1.6 s | | the stun feels fair |
| Boot | 35 % of empty casts, 85 px/s | | funny, not a cheap kill |

## Edge cases
- A hooked ship that dies drops off the hook; the hook reels in.
- A fisherman that dies lets go of its hook; the ship dangles until its own timer ends.
- Two fishermen never hook the same ship twice at once.

## Open questions
- None.

## History
- 2026-11-18: designed and implemented (D-024 to D-027).
