# Music and sound

**Status:** implemented · **Updated:** 2026-11-17 · [GDD](../../GDD.md) · [Implementation](implementation.md) · [Testing](testing.md)

One of two songs, chosen in Settings, drives the action, and short 8-bit sound effects confirm every hit, kill and pickup without drowning it. Effects are synthesized in code at startup, so the only audio files are the two songs (D-005, D-006).

## Rules
1. The chosen song (MIZRAHIT: `music.ogg`, DUBSTEP: `dubstep.ogg`, both next to EBOOT.BIN) starts when a run starts, loops, and stops at game over or the level-clear end screen, or when returning to the main menu; it keeps playing while paused. With OFF no song plays. The main menu is silent. Changing the choice while paused starts the new song from the beginning, or stops the music for OFF.
2. The mixer opens at 48 kHz stereo, the PS3's native rate.
3. Each game event plays at most one sound per frame (ten aliens dying in one rocket blast play one death sound and one explosion).
4. The shoot sound has its own mixer channel so rapid fire never cuts off other sounds; the other sounds share the remaining channels.
5. If a song or the audio device fails, the game runs silently and logs the error.

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Music volume | 96 / 128 | mixer volume | song clearly on top [ASSUMPTION] |
| Mixer channels | 8 | channels | |
| Shoot | square 1400 → 900 Hz, 0.05 s, volume 0.12 | | quiet: it plays 8 times a second |
| Alien hit (not killed) | square 500 → 380 Hz, 0.04 s, volume 0.2 | | |
| Alien death | noise burst, 0.15 s, volume 0.3 | | |
| Rocket launch | square sweep 250 → 900 Hz, 0.3 s, volume 0.3 | | |
| Explosion | low noise, 0.6 s decay, volume 0.5 | | the loudest effect |
| Pickup | two notes 660 then 990 Hz, 0.06 s each, volume 0.3 | | |
| Player death | noise 0.8 s plus a falling square 600 → 80 Hz, volume 0.5 | | |
| Rocket empty | square 150 Hz, 0.03 s, volume 0.2 | | |
| Wave start | three rising notes 523, 659, 784 Hz, 0.08 s each, volume 0.25 | | |
| Extra life | four rising notes 523, 659, 784, 1047 Hz, 0.07 s each, volume 0.3 | | |
| Banana throw (jungle) | noise sweep 2000 → 600 Hz, 0.12 s, volume 0.2 | | gorilla throw, bunch, poop lob |
| Screech (jungle) | three squares 1400 → 1900, 1900 → 1300, 1500 → 2000 Hz, 0.08 s each, volume 0.2 | | monkey banana throw |
| Slip (jungle) | square 1400 → 300 Hz, 0.35 s, volume 0.3 | | slide whistle |
| Stun (jungle) | squares 200 → 450 then 450 → 180 Hz, volume 0.3 | | boing |
| Splat (jungle) | noise 700 → 90 Hz, 0.25 s, volume 0.4 | | poop lands |
| Laugh (jungle) | three squares 330, 300, 270 Hz falling, 0.09 s each, volume 0.35 | | gorilla healed |
| Pound (jungle) | three noise thumps, 0.1, 0.1, 0.15 s, volume 0.45-0.5 | | chest pound starts |
| Sneeze (glacier) | noise 4000 → 900 Hz, 0.25 s, volume 0.4, over a square 900 → 400 Hz | | yeti ACHOO! |
| Freeze (glacier) | noise 3500 → 5000 Hz and a square 1800 → 2600 Hz, 0.3 s | | ship frozen |
| Crack (glacier) | two noise ticks 5000 → 3000 Hz, 0.06 s | | ice block thrown, icicles form |
| Sizzle (volcano) | noise 6000 → 3000 Hz, 0.4 s, volume 0.25 | | ship scorched |
| Bubble (deep sea) | squares 300 → 900 then 500 → 1100 Hz | | bubble attacks, big bubble pops |
| Cast (deep sea) | noise 2500 → 500 Hz and a square 1200 → 400 Hz, 0.2 s | | hook cast, boot thrown |
| Hooked (deep sea) | squares 800, 1000, then 600 → 1200 Hz | | ship hooked |

## Edge cases
- Game over while a sound plays: it finishes; the song stops at once.
- A new run starts: the song restarts from the beginning.
- Quitting through the PS button menu: audio closes before SDL quits.
- A song file is missing: choosing it plays nothing; the other song still works.

## History
- 2026-11-17: designed.
- 2026-11-17: implemented in M2.
- 2026-11-17: second song (DUBSTEP) and a three-way music setting (D-018).
- 2026-11-18: jungle level (D-020): seven new sounds.
- 2026-11-18: levels 3-5 (D-024): seven more sounds.
