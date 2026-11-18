# Jungle

**Status:** implemented · **Updated:** 2026-11-18 · [GDD](../../GDD.md) · [Level 1](../level-1/design.md) · [Enemy waves](../enemy-waves/design.md) · [Bosses](../bosses/design.md) · [Implementation](implementation.md) · [Testing](testing.md)

Level 2: a dark, buzzing rainforest of insects, with a heavy rhinoceros beetle that roams, monkeys that throw bananas and peels, and gorillas that throw poop, pound their chests and heal from their own stink. It is waves 1 to 20 again, gentler than level 1's finale at the start and harder by the end. It keeps pillar 1: nothing in the background hides a shot, and magenta marks everything deadly.

Mockups: [formation wave](../../work-plan/jungle/mock-wave.png) · [twin monkeys](../../work-plan/jungle/mock-monkeys.png) · [gorilla](../../work-plan/jungle/mock-gorilla.png) · [sprites](../../work-plan/jungle/sprites.png) · [palette](../../work-plan/jungle/palette.png)

## Rules
1. JUNGLE in the level picker starts a fresh run of just the jungle (its clear is the win; see [screens](../screens/design.md)) (3 lives, 3 rockets, weapon 1). It is always available for now.
2. Beating level 1 continues the same run into the jungle: score, lives, rockets and weapon level carry over. Play again restarts at the level the menu started (CLASSIC → level 1, JUNGLE → jungle).
3. The jungle restarts the wave count. The banner reads `JUNGLE` for 2 s, then `WAVE 1`; the HUD top centre reads `JUNGLE n`.
4. Killing the jungle's wave-20 bosses shows `JUNGLE CLEAR` for 3 s, then the [glacier](../glacier/design.md) starts with the same players (D-024). (Before D-024 this was the end screen.)
5. Jungle wave n uses level 1's formulas at wave n + 5: rows, armour rows, sway, shot chance, shot speed, dive interval, divers from difficulty wave 3, two divers from difficulty wave 15.
6. Enemies are insects with level 1's roles: lime leafhopper (1 HP, 100), brown beetle (2 HP, 200), black stag beetle (4 HP, 400), and the new heavy black rhinoceros beetle (12 HP, 1,000).
7. Jungle formation waves get `wave / 8` heavy beetles (waves 8-14: 1, waves 16-19: 2; none on boss waves). A heavy flies in like an alien, then roams the box x 0…304, y 18…84 at 40 px/s sideways and 25 px/s vertically, never dives, and shoots at twice the formation's shot chance. It counts for the wave clear.
8. Boss waves (n = wave / 5): 5 one monkey, 10 two monkeys, 15 one gorilla, 20 two gorillas. Entry, twins' halves, health bar, flash, rocket damage, pickups and clear bonus are as level 1's bosses.
9. The monkey alternates a banana throw (3 bananas 0.15 s apart straight down, plus a peel lob that lands 1.25 s later on a random point of the ship zone) and a banana bunch (5 bananas in a fan).
10. The gorilla rolls each attack: 30 % banana throw with a peel, 30 % poop throw, 20 % banana bunch, 20 % chest pound. A poop is lobbed at the nearest ship and splats into a stink mist; with 3 mists or poops already out it does a banana bunch instead. A chest pound (0.6 s) ends with 5 coconuts falling, one per fifth of the screen; pounding again while pounding gives a banana bunch.
11. A banana peel lies 5 s (blinking in the last second). A ship whose hitbox touches it slides 0.5 s at 140 px/s the way the stick was pushed (random left or right when centred), still able to shoot, then is stunned 1.0 s: no moving, shooting or rockets, with dizzy stars. The peel is squashed. Peels don't kill and work on a respawning ship.
12. A stink mist grows to radius 20 px in 0.3 s, thins out in its last second and is gone after 6 s. While a ship's centre is inside any mist, that player's bullets heal gorillas (+1 HP each, up to max) instead of damaging them, and are drawn lime. The check happens when the bullet hits. A heal makes the gorilla laugh 0.6 s (`HA HA`, lime health bar). Rockets are unaffected.
13. Magenta marks every deadly enemy projectile: shots, bananas and coconuts. Peels, poop and mist never use it.

## The level bundle
| Part | Jungle |
|---|---|
| Theme and mood | dark rainforest: humid, buzzing, cheeky |
| Palette | 15 new characters in `source/sprites.h` (greens, bark browns, insect colours, banana yellow, mist green); every background colour is darker than the grey stars |
| Background | far layer 8 px/s (thin trunks, leaf clusters), near layer 18 px/s (two edge trunks, rope bridges, vines, leaf clumps), 14 dim fireflies |
| Enemy roster | leafhopper, brown beetle, stag beetle, roaming rhinoceros beetle |
| Ship skin | unchanged |
| Bosses | monkey (150 HP) at 5, twin monkeys at 10, gorilla (300 HP) at 15, twin gorillas at 20 |

## Numbers
| Boss wave | Bosses | HP each | Sway px/s | Attack every | Reward each |
|---|---|---|---|---|---|
| 5 | 1 monkey | 150 | 50 | 2.2 s | 25,000 |
| 10 | 2 monkeys | 150 | 60 | 2.0 s | 30,000 |
| 15 | 1 gorilla | 300 | 45 | 1.8 s × 0.6-1.4 | 35,000 |
| 20 | 2 gorillas | 300 | 55 | 1.6 s × 0.6-1.4 | 40,000 |

| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Banana speed | 85 (bunch: 75 down, ±50 sideways) | px/s | dodgeable at the fan's edges |
| Coconut speed | 110 | px/s | five lanes leave a gap |
| Slide / stun | 0.5 s at 140 px/s / 1.0 s | | stun feels fair, not cruel |
| Peel life | 5 | s | |
| Mist | radius 20, 6 s, at most 3 | | players can avoid healing the gorilla |
| Heavy beetle | 12 HP, 40 / 25 px/s, 2× shot chance | | |

## Sounds
Banana throw (noise sweep), screech, slip (slide whistle), stun (boing), splat, laugh (ha-ha-ha), pound (three thumps), all synthesized like the others. The song stays the player's choice.

## Edge cases
- A peel under a respawning (invincible) ship still makes it slip.
- A stink mist under a dead ship does nothing.
- A gorilla that dies mid-pound drops no coconuts.
- Peels and mists outlive their boss.
- A rocket fired from inside the mist still damages the gorilla.
- The last life lost during `JUNGLE CLEAR` is a game over, a life lost with lives left still wins.

## Open questions
- Lock JUNGLE until level 1 is beaten (user, later).
- A jungle ship skin, a jungle song and new XMB art are out of scope for now.

## History
- 2026-11-18: designed and implemented (D-020 to D-023).
- 2026-11-18: the jungle clear continues into the glacier (D-024); heavy counts now come from the shared level table (D-025).
