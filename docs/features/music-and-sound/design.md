# Music and sound

**Status:** designed · **Updated:** 2026-11-17 · [GDD](../../GDD.md)

The user's song drives the action, and short 8-bit sound effects confirm every hit, kill and pickup without drowning it. Effects are synthesized in code at startup, so the only audio file is the song (D-005, D-006).

## Rules
1. The song (`music.ogg` next to EBOOT.BIN) starts when a run starts, loops, and stops at game over; it keeps playing while paused. The title screen (M3) is silent.
2. The mixer opens at 48 kHz stereo, the PS3's native rate.
3. Each game event plays at most one sound per frame (ten aliens dying in one rocket blast play one death sound and one explosion).
4. The shoot sound has its own mixer channel so rapid fire never cuts off other sounds; the other sounds share the remaining channels.
5. If the song or audio device fails, the game runs silently and logs the error.

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

## Edge cases
- Game over while a sound plays: it finishes; the song stops at once.
- A new run starts: the song restarts from the beginning.
- Quitting through the PS button menu: audio closes before SDL quits.

## History
- 2026-11-17: designed.
