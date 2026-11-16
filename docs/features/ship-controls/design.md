# Ship controls

**Status:** designed · **Updated:** 2026-11-17 · [GDD](../../GDD.md)

The player flies a small ship around the bottom of the screen and holds R1 to spray bullets upward. It should feel quick and exact: the ship stops where you let go, and bullets come out at a steady rhythm. It serves pillar 1 (readable) and the core loop's seconds.

## Rules
1. The left stick and the D-pad both drive the `move` action. The stick gets a radial deadzone, rescaled past it; the D-pad gives full speed. Diagonals are no faster than straight moves.
2. The ship moves only inside the movement zone (bottom of the screen) and never leaves the screen.
3. Holding R1 (`fire`, read as held) fires one volley every fire interval, starting on the press frame.
4. Bullets fly straight up (or along their spread angle) and disappear on hitting an enemy or leaving the screen.
5. The gun has upgrade levels (see Weapon upgrades in the GDD index); level 1 fires one bullet.
6. Each ship belongs to a player with its own controller index; game code loops over players, which is one in the MVP (D-007).

## Numbers
| Name | Value | Unit | What to watch in playtests |
|---|---|---|---|
| Logical screen | 320 × 180 | px | readability on a TV from the couch |
| Ship size | 16 × 16 | px | hitbox feels fair |
| Ship hitbox | 6 × 6, centred | px | deaths feel deserved [ASSUMPTION] |
| Ship speed | 150 | px/s | can dodge a shot after seeing it [ASSUMPTION] |
| Movement zone | y 100–180 | px | room to dodge vs crowding the aliens [ASSUMPTION] |
| Stick deadzone | 0.2 | stick length | no drift at rest [ASSUMPTION] |
| Fire interval | 0.12 | s | steady but not a laser [ASSUMPTION] |
| Bullet speed | 320 | px/s | [ASSUMPTION] |
| Bullet damage | 1 | HP | |

## Edge cases
- Pause: holding R1 through pause doesn't fire on resume until the next interval.
- Death: input is ignored until respawn; R1 held during respawn starts firing at once.
- Unplugged controller: the game pauses.

## History
- 2026-11-17: designed.
