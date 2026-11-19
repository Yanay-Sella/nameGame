# NAME game

An arcade shooter for the PS3: hold R1 to spray bullets at waves of aliens, save the R2 rocket for emergencies, beat a boss every 5th wave and chase the high score. One or two players.

## What you need

- A PS3 running PS3HEN (tested on firmware 4.93). HEN must be enabled again after every PS3 restart.
- A USB stick formatted FAT32.
- One DualShock 3 controller (a second one for two players).

## Install

1. Copy `NAME-game-1.0.pkg` to the root of the USB stick (not into a folder).
2. Plug the stick into the PS3 and enable HEN.
3. Go to Game → Package Manager → Install Package Files → Standard and pick `NAME-game-1.0.pkg`.
4. NAME game appears in the Game column.

Installing a newer version the same way keeps your high score and settings.

## Controls

| Action | Button |
|---|---|
| Move | Left stick or D-pad |
| Fire | Hold R1 |
| Rocket | R2 |
| Pause | Start |
| Menus | X select, Circle back |
| Player 2 | Press Start on a second controller during a run |

## Saving

The high score and settings are saved automatically.

## Troubleshooting

- **The file isn't in Package Manager:** HEN wasn't enabled, or the file isn't at the root of the stick.
- **No music:** Settings → MUSIC: MIZRAHIT or DUBSTEP.
- **"CONNECT CONTROLLER" on screen:** press the controller's PS button, then Start to resume.
- **The game is frozen:** hold the PS3's power button for about 10 seconds until it beeps (never pull the plug).

## Uninstall

Highlight NAME game in the Game column, press Triangle, then choose Delete.

## For developers

See `CLAUDE.md` and `docs/ps3-new-project-guide.md`.

## Music and legal

- The songs are not in this repository and the code can't play any without you adding them: `make music MUSIC_SRC=<your song>` converts a file you have the rights to. Don't publish builds that contain music you don't own.
- This is a hobby project. It is not affiliated with Sony. It uses only the open-source ps3dev/PSL1GHT toolchain and SDL2, with no Sony SDK code. Running it on a PS3 needs custom firmware (PS3HEN) that you set up yourself, at your own risk.
- Code and art are under the MIT License (`LICENSE`).
