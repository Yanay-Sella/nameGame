# Pixel art: implementation

**Updated:** 2026-11-17 · [Design](design.md) · [Testing](testing.md)

## Approach
Sprites, palette and font live in `sprites.h` as text grids. `screen.cpp` draws them into the 320×180 `screenPixels` buffer, skipping transparent pixels; `main.cpp` uploads the buffer each frame and copies it to the TV scaled by a whole number (GDD D-008).

## Code map
| File / module | Role in this feature |
|---|---|
| `source/sprites.h` | `paletteColor`, every `Sprite`, `fontGlyph` |
| `source/screen.cpp` | `fillRect`, `fillCircle`, `drawSprite`, `drawSpriteSolid`, `drawText` |
| `source/game.cpp` › `drawGame` | draw order, animation frames, hit flash, starfield |
| `source/main.cpp` | upload and scaled copy to the TV |

## Data and config
- **Config constants:** `ANIMATION_FRAME`, `HIT_FLASH`, `STAR_COUNTS`, `STAR_SPEEDS`, `STAR_COLORS` (`game.cpp`); `SCREEN_W`, `SCREEN_H` (`screen.h`).
- **Persistence:** none.

## Deviations from design
None.

## History
- 2026-11-17: implemented in M2.
