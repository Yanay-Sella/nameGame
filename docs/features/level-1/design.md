# Level 1

**Status:** implemented · **Updated:** 2026-11-17 · [GDD](../../GDD.md) · [Enemy waves](../enemy-waves/design.md) · [Bosses](../bosses/design.md) · [Pixel art](../pixel-art/design.md)

The first world of NAME game: the game as it plays today, packaged as a level. It is waves 1 to 20 of the current game, with the difficulty rising all the way up, and it ends when the wave-20 boss dies. Nothing about the look changes: same navy starfield, same ship, same three aliens, the bosses differ per boss wave ([Bosses](../bosses/design.md)). It is also the template the later worlds (jungle, frozen, lego...) copy: a run of waves, a final boss, then the next level.

## Rules
1. Level 1 is waves 1 to 20. Formation waves are exactly as in [Enemy waves](../enemy-waves/design.md); waves 5, 10, 15 and 20 are bosses exactly as in [Bosses](../bosses/design.md).
2. The bosses at waves 5, 10 and 15 are mid-level bosses; the wave-20 boss is the level boss. Killing it ends the level.
3. The wave banner reads `LEVEL 1` for the first 2 s of wave 1, then `WAVE 1` as today.
4. When the level boss dies: its normal reward, large explosion, guaranteed pickups and wave-clear bonus apply, then a `LEVEL 1 CLEAR` banner shows for 3 s. Enemy shots already in the air keep flying during the banner.
5. The banner is followed by level 2, the [jungle](../jungle/design.md), with the same players (D-020), and the run goes on through the glacier, the volcano and the deep sea (D-024). The end screen comes after the deep sea, titled `DEEP SEA CLEAR` in yellow.
6. The look, the sprites, the palette and the rules above stay as in the current game; the only visible additions are the two banners.

## The level bundle
| Part | Level 1 |
|---|---|
| Theme and mood | Deep space, calm and dark: the classic arcade starfield |
| Palette | Existing palette in `source/sprites.h`; navy background, grey stars, no new colours |
| Background | Existing 3-layer starfield at 6 / 14 / 30 px/s |
| Enemy roster | Green (1 HP), blue (2 HP, from wave 4), red armoured (4 HP, from wave 7); the existing divers from wave 3 |
| Ship skin | The current ship (`SHIP_A` / `SHIP_B`); later levels have their own skins (D-031) |
| Bosses | One classic at wave 5 (100 HP), two at 10 (100 HP each), a mage at 15 (200 HP), two mages at 20 (200 HP each) |

## Wave table
Numbers are what the current code does (`source/game.cpp`); intensity is a feel guess for the `level-design` pacing check. All formations are 8 columns wide.

| Wave | Content | Rows | Intensity | Beat |
|---|---|---|---|---|
| 1 | Green, sway 22 px/s, shot chance 0.025/s | 2 | 0.1 | teach |
| 2 | Green | 3 | 0.25 | combat |
| 3 | Green, first diver (every 3.4 s) | 3 | 0.35 | combat |
| 4 | 2 blue rows + green | 4 | 0.45 | combat |
| 5 | Mid boss: 1 classic, 100 HP, sway 40 px/s, attack every 2.5 s | boss | 0.75 | climax |
| 6 | 2 blue rows + green | 5 | 0.5 | combat |
| 7 | 1 red, 1 blue + green | 5 | 0.6 | combat |
| 8 | Same, sway 36 px/s, shot chance 0.06/s | 5 | 0.7 | combat |
| 9 | Same, sway 38 px/s, shot chance 0.065/s, diver every 2.2 s | 5 | 0.75 | combat |
| 10 | Mid boss: 2 classics, 100 HP each, each in its half, sway 50 px/s, each attacks every 2.2 s | boss | 0.85 | climax |
| 11 | 1 red, 1 blue + green, shots 72 px/s | 5 | 0.6 | combat |
| 12 | Same, shot chance at its 0.08/s cap, shots 74 px/s | 5 | 0.65 | combat |
| 13 | 2 red, 2 blue + 1 green row, diver every 1.5 s (cap) | 5 | 0.75 | combat |
| 14 | Same, shots 78 px/s | 5 | 0.8 | combat |
| 15 | Mid boss: 1 mage, 200 HP, sway 60 px/s, random attack every 1.1–2.7 s, power-ball beam | boss | 0.9 | climax |
| 16 | Armour as wave 13; two aliens dive at once from here | 5 | 0.7 | combat |
| 17 | Same, shots 84 px/s | 5 | 0.75 | combat |
| 18 | Same, shots 86 px/s | 5 | 0.85 | combat |
| 19 | Same, shots 88 px/s | 5 | 0.9 | combat |
| 20 | Level boss: 2 mages, 200 HP each, sway 70 px/s, each attacks every 1.0–2.2 s, reward 20,000 each | boss | 1.0 | climax |

Shot speed tops out at 90 px/s at wave 20. Two divers at once start at wave 15 (the first formation wave after it is 16), aliens gain 1 HP from wave 15 and the formation bobs from wave 16; the sway keeps speeding up into later levels (enemy-waves rule 9, D-033).

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Level length | 20 waves (16 formations, 4 bosses) | waves | a full run is about 15 to 20 min [ASSUMPTION] |
| Level banner | 2 | s | readable, never covers shots |
| Level-clear banner | 3 | s | long enough to enjoy the win |
| Level boss | 2 mages, 200 HP and 20,000 points each | | clearly the finale |

## Edge cases
- The player dies during the level boss fight: respawn as today; the level ends only when the boss dies.
- Both players alive in co-op: the banner and the win apply to the run, not per player.
- The last life is lost on the level boss, or during the `LEVEL 1 CLEAR` banner: game over as today, no level clear. A life lost during the banner with lives left still wins.
- Player 2 can't join once the level is clear.
- A rocket kills the level boss: the same level clear.
- Wave 21 and later are never reached: the level ends at wave 20.

## Open questions
- **Rest before the climax (decided 2026-11-17 for wave 9):** wave 9, and the same way wave 19 before the level boss, stay as they are (5 rows, red and blue), even though the `level-design` check flags "no rest before the climax"; the game plays like this today.
- **Later levels:** are they also 20 waves, or 10? The first note said 10; level 1 is now 20. Default: decide when level 2 is designed.
- **Wave numbering in later levels:** does level 2 restart the HUD at `WAVE 1`, or keep counting `WAVE 21`? Default: restart per level, with the harder numbers carried by a level multiplier.

## History
- 2026-11-17: designed as waves 1 to 10.
- 2026-11-17: extended to waves 1 to 20, ending on the wave-20 boss (changed by the user).
- 2026-11-17: implemented (D-017).
- 2026-11-17: boss line-up per wave: classic, twin classics, mage, twin mages (D-019).
- 2026-11-18: jungle level (D-020): level 1 now continues into the jungle.
- 2026-11-18: levels 3-5 (D-024): the run continues to the deep sea.
