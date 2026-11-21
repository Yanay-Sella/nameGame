# Enemy waves

**Status:** implemented · **Updated:** 2026-11-18 · [GDD](../../GDD.md) · [Implementation](implementation.md) · [Testing](testing.md)

Aliens fly in, line up in a formation at the top of the screen, sway side to side and drop shots; from wave 3 some break off and dive at the ship. Clearing a wave brings a harder one, and every 5th wave is a boss (see Bosses in the GDD index). This is the core loop's minutes.

The base formation and dive rules are below. See [Co-op](../co-op/design.md) for the larger Classic 1–4 Players waves.

## Rules
1. A wave starts with its aliens flying in from above the screen into formation slots; they can't shoot until they reach their slot.
2. The formation sways left and right as one and reverses at the screen edges.
3. Every alien has a chance per second to drop a shot straight down; shots are magenta and are the only magenta objects.
4. From wave 3, one alien at a time (from wave 15 up to two at once) leaves formation and dives in a curve toward the ship, then exits the bottom and returns to its slot from the top.
5. A wave is cleared when every alien is dead; the next starts after a short pause with a "WAVE n" banner.
6. Alien types: green (1 HP), blue (2 HP, from wave 4), red armoured (4 HP, from wave 7); from wave 13 the top two rows are red and the next two blue.
7. Every 5th wave is a boss instead of a formation (the Bosses feature, M3).
8. Each formation wave picks a random shape and a random entry (D-033). Shapes: block, staggered (rows shifted ±4.5 px), V (wings lower), arrow (centre lower), wave (a sine across the columns) and split (two halves 36 px apart). Entries: from the top (as before), a stream from the left or the right (one alien every 0.05 s from y 30, far slots first), a pincer (each half from its own side) and a scatter (from the top, each alien at a random time within 1.2 s). Rows keep their alien types; every shape stays above the ship zone. From D-034 there are also 29 traced shapes drawn with the same aliens (always the wave's 16-40): diagonal lines (/ and \\), zigzag, double zigzag, X, three X, heart, three hearts, diamond, nested diamonds, <>, triangle up and down, box, three circles, two circles, oval, nested ovals, two sine waves, spiral, three plus signs, arrow, bow tie, crown, rainbow arch, bowl, stepped pyramid, three random letters (A-Z) and a word (GO, ZAP, PEW, WIN, UFO, NAME, GAME, BOOM, FIRE). A traced shape spaces the aliens evenly along its lines, so it is drawn only when the alien count fits it (13.5-24 px between neighbours, none overlapping); otherwise another shape is drawn. Top rows keep the tougher aliens.
9. Difficulty counts in difficulty waves (D-033): level 1's waves 1–20 are 1–20; each later level drops 15 below the last level's end and climbs 20 more (jungle 6–25, glacier 11–30, volcano 16–35, deep sea 21–40). The picker plays a level at the same numbers. Past wave 15 aliens gain HP and move more: +1 HP at wave 15, 25 and 35 (all types), the sway speeds up to 80 px/s at wave 30, a third diver joins at wave 30, and from wave 16 the formation bobs up and down (1 px, growing to 4 px at wave 36) in a ripple across the columns.

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Formation | 8 columns × min(2 + wave/2, 5) rows | aliens | screen not too crowded at 5 rows [ASSUMPTION] |
| Alien size | 12 × 10 | px | see [pixel art](../pixel-art/design.md) |
| Sway speed | 20 + 2 × wave, max 80 | px/s | wave 30+ sway is still trackable |
| Shot chance per alien | 0.02 + 0.005 × wave, max 0.08 | per s | wave 1 is calm, wave 10 is busy [ASSUMPTION] |
| Enemy shot speed | 70, +2 per wave after wave 10, max 90 | px/s | late waves stay tense [ASSUMPTION] |
| Armoured rows | wave 4: 2 blue; wave 7: 1 red + 1 blue; wave 13: 2 red + 2 blue | rows | waves 13+ feel harder than 7–12 [ASSUMPTION] |
| Simultaneous divers | 1, from wave 15: 2, from wave 30: 3 | aliens | three divers are dodgeable |
| HP bonus | +1 at wave 15, +2 at 25, +3 at 35 | HP per alien | late formations don't drag at weapon level 1–2 |
| Formation bob | from wave 16: 1 + 0.15 × (wave − 16), max 4; 3 rad/s, phase 0.05 rad per px of slot x | px | |
| Dive interval | 4 − 0.2 × wave, min 1.0 | s | |
| Score per alien | 100 / 200 / 400 | points (green / blue / red) | [ASSUMPTION] |
| Wave clear bonus | 1000 × wave | points | [ASSUMPTION] |
| Pause between waves | 2 | s | |
| Fly-in | straight at the moving slot at 150 px/s; from the top: 150 px above it, 0.06 s later per column, 0.12 s per row | | wave starts feel lively, not slow; a 40-alien side stream settles in about 4 s |
| Dive | 90 px/s down, steering 120 px/s² toward the ship, at most 70 px/s sideways | | dodgeable but threatening [ASSUMPTION] |
| Return to formation | 100 | px/s | [ASSUMPTION] |

## Edge cases
- The player dies mid-wave: the wave continues; diving aliens return to formation during respawn.
- A rocket kills the last alien: the wave clears normally.
- Past wave 40 everything is capped (only a picker or classic deep sea reaches it).

Jungle waves add roaming heavy beetles: see [Jungle](../jungle/design.md). The glacier, volcano and deep sea add heavies every 6, 5 and 4 waves (at most 3) and speed the formation's shots up by 5, 10 and 15 px/s (D-025); rule 9 sets how far ahead each level plays.

## History
- 2026-11-17: designed; alien size set to 12 × 10 with the pixel art.
- 2026-11-17: implemented in M2.
- 2026-11-17: difficulty extended to wave 20 (D-013).
- 2026-11-18: jungle level (D-021): jungle waves use the difficulty wave (n + 5) and add roaming heavy beetles.
- 2026-11-18: levels 3-5 (D-025): the difficulty ladder.
- 2026-11-18: random shapes and entries; difficulty 1–20 per level stepping back 15 at each new level, with HP and movement growing to wave 40 (D-033).
- 2026-11-19: 29 traced shapes: lines, X, heart, <>, circles, zigzag, letters and words (D-034).
