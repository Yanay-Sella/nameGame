# Music and sound: implementation

**Updated:** 2026-11-17 · [Design](design.md) · [Testing](testing.md)

## Approach
`audio.cpp` opens SDL2_mixer at 48 kHz, reads `music.ogg` and `dubstep.ogg` from the folder holding EBOOT.BIN into memory once (about 190 ms each at boot) and plays the one chosen in settings from there, and synthesizes every effect once at startup from a list of `Tone`s (square or sample-and-hold noise, swept and linearly faded) into a `Mix_Chunk` in the mixer's sample format. The game only sets `soundRequested[sound]`; `main.cpp` plays each requested sound once per frame.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/audio.cpp` › `audioOpen`, `synthesize` | mixer setup, song read into memory, the 10 effects |
| `source/audio.cpp` › `audioStartMusic`, `audioStopMusic`, `audioPlay` | song control, shoot on its own channel |
| `source/game.h` › `Sound` | the list of effects the rules can ask for |
| `source/main.cpp` | plays requested sounds; the chosen song at a new run, off at game over |

## Data and config
- **Config constants:** `MUSIC_VOLUME`, `MIXER_CHANNELS`, `SHOOT_CHANNEL`; each effect's tones in `audioOpen`, in the order of the design table.
- **Persistence:** none. The songs are `pkgfiles/USRDIR/music.ogg` and `dubstep.ogg`, made by `make music` (gitignored).

## Deviations from design
- The song plays from a copy in memory, not from the file: a restart after game over froze ~350 ms, and a startup probe showed an immediate restart from the file takes only 2 ms, so seeking back through the file on the hard drive was the likely cause. Confirmed: from memory the restart after a game over takes 1 ms.

## History
- 2026-11-17: implemented in M2.
