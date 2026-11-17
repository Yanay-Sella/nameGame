# Pixel art

**Status:** implemented · **Updated:** 2026-11-17 · [GDD](../../GDD.md) · [Implementation](implementation.md) · [Testing](testing.md)

Everything is chunky pixel art at 320×180, scaled by a whole number to the TV (D-004). Sprites, the font and the palette are text grids in `source/sprites.h`, one character per pixel (D-006), drawn into the CPU pixel buffer (D-008). It serves pillar 1 (readable at a glance).

## Rules
1. A sprite is a grid of palette characters; `.` is transparent. Every row of a sprite has the same width.
2. Only palette colours are used. Magenta (`m`) is used only by enemy shots; player bullets are pale yellow (`y`).
3. An alien that is hit but not killed flashes white for the hit-flash time.
4. Animated sprites have 2 frames that alternate at the animation rate.
5. The background is a dark starfield of 3 layers scrolling down at different speeds; stars are never brighter than grey, so they can't be mistaken for shots.
6. Text uses a 3×5 pixel font (letters, digits and a few signs); banners draw it at 2× size.

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Logical screen | 320 × 180 | px | |
| Ship | 16 × 14, 2 frames (thruster) | px | |
| Aliens (green, blue, red) | 12 × 10, 2 frames each | px | readable at a glance from the couch |
| Player bullet / enemy shot | 2 × 4 | px | shots visible against stars |
| Rocket | 4 × 8 | px | |
| Rocket pickup | 8 × 8 | px | |
| HUD icons (life, rocket) | 7 × 5, 3 × 5 | px | |
| Animation rate | 0.25 | s per frame | [ASSUMPTION] |
| Hit flash | 0.06 | s | [ASSUMPTION] |
| Star layers | 30 / 20 / 10 stars at 6 / 14 / 30 px/s | | background calm, not distracting [ASSUMPTION] |

## Edge cases
- Sprites partly off screen are clipped, never wrapped.
- A sprite whose text length doesn't match its size is a bug: `make test` checks every sprite.

## History
- 2026-11-17: designed.
- 2026-11-17: implemented in M2.
