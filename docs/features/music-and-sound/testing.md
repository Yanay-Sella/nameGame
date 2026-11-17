# Music and sound: testing

**Updated:** 2026-11-17 · [Design](design.md) · [Implementation](implementation.md)

## Acceptance criteria
- [x] The song loads from memory on the PS3. *(PS3 log 2026-11-17: "music loaded" after fixing an argument-order bug in `SDL_RWread` that broke the first memory-loading build)*
- [x] The mixer opens at 48 kHz stereo and the song loads on the PS3 (file loading, before the memory change). *(PS3 log 2026-11-17: "audio: 48000 Hz, format 0x9010, 2 channels, music loaded")*
- [x] Song and effects play together through a full run. *(PS3 playtest 2026-11-17: user said "feels good")*
- [x] Starting a new run restarts the song without a visible freeze. *(PS3 log 2026-11-17: "music restart took 1 ms" after a game over, down from ~350 ms when the song streamed from the file)*
- [ ] Every one of the 10 effects is heard and none drowns the song. *(not proven: no effect-by-effect listening check)*

## History
- 2026-11-17: criteria written.
