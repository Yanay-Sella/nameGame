# nameGame — PS3 homebrew game

C++ game for a real PS3, built with the open-source ps3dev toolchain (PSL1GHT) and SDL2.
Sony's official SDK, and the Unity/Unreal PS3 exports, are not available and must not be used.
Full setup history, debugging toolkit and new-project checklist: `docs/ps3-new-project-guide.md`.

## Hardware setup
- PS3 at `192.168.1.200` (override with `PS3_IP=`), firmware 4.93, PS3HEN + webMAN MOD.
- HEN turns off on every PS3 reboot: if the PS3 doesn't respond (no FTP/HTTP), ask the user to restart it and re-enable HEN.
- If only the game is stuck, webMAN still answers: restart remotely with `curl http://192.168.1.200/restart.ps3`.
  If webMAN doesn't answer either, ask the user to hold the power button ~10 s (never unplug).
- Never install while an app/game is running (it froze the PS3 once); `make deploy` checks for this.
- Never print or commit the console's PSID/IDPS, which webMAN's status pages show.

## Commands (run on the Mac)
- `make ps3` — full rebuild in Docker → `build/nameGame.pkg`. Uses image `namegame-ps3dev` (`docker/Dockerfile`:
  `zeldin/ps3dev-docker` + SDL2 rebuilt with `docker/sdl2-psl1ght-render.patch`), built automatically by `make image`.
- `make deploy` — build, upload via FTP to `/dev_hdd0/packages/`, install with webMAN (`/install.ps3/...`).
  The user then launches nameGame from the PS3's Game menu; webMAN can't start installed hdd0 games remotely.
- `make log` — print the game's UDP log lines (port 18195). Run it before the user launches the game.
  `make ps3` deletes `build/`, so don't redirect the log into `build/`.
- `make deploy SOURCES=prototypes/<idea>` — build and install a throwaway prototype instead of `source/` (same app ID).
- `make music MUSIC_SRC=<song>` — convert a song to `pkgfiles/USRDIR/music.ogg` (gitignored), packaged next to EBOOT.BIN.
- `python3 docs/xmb-art/make_xmb_art.py` — redraw the XMB Game menu art (`pkgfiles/ICON0.PNG` 320x176, `PIC1.PNG` 1920x1080)
  from the sprites in `source/sprites.h`; rerun it after changing sprites or the palette.

## Code
- `source/*.cpp` — game code. Design docs: `docs/GDD.md` and `docs/features/`. `ps3.mk` — PSL1GHT build rules (runs inside Docker). `Makefile` — host commands.
- Log with `debugNetPrintf(INFO, ...)`; the Mac's IP is baked in at build time (`LOG_HOST`).
- Poll SDL events every frame: SDL turns the PS button menu's "Quit Game" into `SDL_QUIT`.
- A watchdog thread logs where the main loop is stuck (and RSX state) after 5 s without a frame, then exits to the XMB.
- Debug freezes from the logs (watchdog line), not by guessing; a "hang" may be a crashed main thread.
- Available libraries: see `$PS3DEV/portlibs/ppu/lib` in the image (SDL2, SDL2_mixer, freetype, libpng, libjpeg, zlib, curl...).
  No SDL2_image or SDL2_ttf (only SDL 1.2 versions).
- SDL2_mixer plays OGG Vorbis, WAV and MikMod modules only (no FLAC/MP3); it links with `-lmikmod -lvorbisfile -lvorbis -logg`.

## PS3 SDL2 port quirks (shagkur/SDL_PSL1GHT)
- Upstream render bugs fixed by our patch (keep using our image): `SDL_RenderFillRect` wrote outside the framebuffer,
  and every clear/draw used the last-set draw color instead of its own.
- Create the window at `SDL_GetDesktopDisplayMode` size (the PS3's output, e.g. 1920x1080), without `SDL_WINDOW_FULLSCREEN`.
- No-ops in this port: `SDL_RenderCopyEx` (rotation/flip), `SDL_SetRenderTarget`, texture scale modes.
- `SDL_RenderCopy` is a hardware scaled blit (nearest-neighbour) with no alpha blending: transparent pixels come out opaque.
  So the game draws into a CPU pixel buffer and copies it to the screen scaled once per frame (`docs/GDD.md` D-008).
- `SDL_RenderClear`/`FillRect` are CPU writes to the framebuffer: avoid full-screen clears every frame.
- Audio device is 48 kHz stereo float; open the mixer at 48000 Hz to avoid resampling.
- Controller (from the port's `SDL_sysjoystick.c`): buttons 0 left, 1 down, 2 right, 3 up, 4 start, 5 R3, 6 L3, 7 select,
  8 square, 9 cross, 10 circle, 11 triangle, 12 R1, 13 L1, 14 R2, 15 L2; axes 0/1 left stick x/y, 2/3 right stick x/y.
  Confirmed on the PS3: 0, 1, 2, 3, 4, 9, 11, 12, 14; the rest is from the source.
- No `SDL_JOYDEVICEADDED`/`REMOVED` events: open controllers yourself from `SDL_NumJoysticks()` (see `source/input.cpp`).
