# Starting a new PS3 homebrew project: setup guide

Everything needed to go from an empty repo to C++/SDL2 code running on a real PS3, built on a Mac.
It's distilled from setting up nameGame, including what went wrong and how it was diagnosed.
The nameGame repo is the working template: copy its files instead of writing them from scratch.

---

## 1. What you need

### On the PS3
| Item | Notes |
|---|---|
| **PS3HEN** (or CFW) | HEN works on every model and on the latest firmware (we used 4.93 + HEN 3.6.0). **It turns off on every reboot**: re-enable it after each restart. |
| **webMAN MOD** | Provides FTP (upload builds), HTTP commands (install packages, restart) and the PS3MAPI page (list running processes). |
| **Network** | Same LAN as the Mac. Find the IP under Settings → Network Settings → Settings and Connection Status List (or scan the LAN for webMAN's web page). |

### On the Mac
- **Docker Desktop**. The official image `zeldin/ps3dev-docker` has a **native arm64** build, so there's no emulation on Apple Silicon.
- git, curl, python3 (all built into macOS / Xcode tools).

### Not usable
- Sony's official SDK and the Unity/Unreal PS3 exports were licensed-only and are discontinued; leaked copies are piracy.
  The legal route is **ps3dev**: ps3toolchain (compilers) + **PSL1GHT** (SDK) + ps3libraries (SDL2, zlib, libpng, freetype…).

---

## 2. Project layout (copy from nameGame)

```
Makefile                         host commands: build in Docker, deploy, read logs
ps3.mk                           PSL1GHT build rules (runs inside Docker)
docker/Dockerfile                ps3dev image + SDL2 rebuilt with our fixes
docker/sdl2-psl1ght-render.patch the SDL2 fixes (see section 5)
source/main.cpp                  game code
CLAUDE.md                        project facts for Claude sessions
.gitignore                       build/
```

For a new game, change these in `ps3.mk`:
- `TARGET` (output name)
- `TITLE` (name shown on the XMB)
- `APPID`: 9 chars, 4 letters + 5 digits, unique per game, e.g. `NMGM00001`. It becomes `/dev_hdd0/game/<APPID>`.

Update the same names in the `Makefile` (`PKG` path and the FTP/install file names).

Key parts of `ps3.mk`:
- `include $(PSL1GHT)/ppu_rules` provides the `.elf → .self → .pkg` rules.
- `LIBS := -lSDL2 -ldebugnet -lnet -lsysmodule -lgcm_sys -lrsx -lsysutil -lio -laudio -lrt -llv2 -lm` (order matters).
- `-DLOG_HOST=\"$(LOG_HOST)\" -DLOG_PORT=$(LOG_PORT)` bakes the Mac's IP into the build for logging.

---

## 3. The daily loop

```bash
make log      # in its own terminal: prints the game's log lines (UDP port 18195)
make deploy   # build in Docker → upload over FTP → install with webMAN
```

Then start it remotely with `make launch` (`curl http://<PS3>/play.ps3?<APPID>`; takes 10-15 s; webMAN MOD 1.47.50d, firmware 4.93, HEN 3.6.0).
`/play.ps3/dev_hdd0/game/<APPID>` did not start it (tested for 60 s); `?<name>` and `col=game&seg=...` were not reliable either.

What `make deploy` does:
1. `make clean` + full rebuild. A full rebuild is needed because the log IP/port are compile-time flags, and make doesn't notice flag changes.
2. Checks `http://<PS3>/home.ps3mapi` for any running process besides `main_vsh.self`, and **refuses to install if a game or app is running**. Installing while an app ran froze our PS3.
3. `curl -T build/x.pkg ftp://<PS3>/dev_hdd0/packages/x.pkg`
4. `curl http://<PS3>/install.ps3/dev_hdd0/packages/x.pkg`

---

## 4. Logging and debugging toolkit

- **Network logs:** use `debugnet` (`netInitialize(); debugNetInit(LOG_HOST, LOG_PORT, DEBUG); debugNetPrintf(INFO, ...)`).
  On the Mac, listen with a small Python UDP server (`make log`); macOS `nc -ul` proved unreliable.
  Don't write the log file into `build/`, because `make clean` deletes it.
- **Watchdog thread** (keep it in every project): a thread that checks once a second whether the main loop's frame counter has advanced.
  After 5 s without progress it logs:
  - the current `stage` string (set before each major call),
  - the graphics chip (RSX) state: `gcmGetControlRegister()` put/get, `gcmGetFlipStatus()`, `gcmGetLastFlipTime()`,

  then calls `sysProcessExit(1)` to **return to the XMB without a PS3 restart**.
- **Reading the RSX state:** if `put == get` and `lastFlip == 0`, the chip is idle and has never shown a frame. The main thread died or is stuck **before** the flip.
  A "hang" can really be a **crashed main thread**: on the PS3, other threads keep running after one crashes.
- **Bisect with minimal programs.** We proved the hardware and SDK were fine with a raw PSL1GHT flip test (no SDL), then read the SDL2 port source.
  The SDL2 port the toolchain uses is `shagkur/SDL_PSL1GHT` (branch `sdl2_master`); its renderer is `src/render/psl1ght/SDL_PSL1GHTrender.c`.
- **Is the PS3 the problem?** Launch known-good homebrew (Apollo Save Tool, PKGi, IRISMAN). Check health on webMAN's `/cpursx.ps3` (temperatures, free space, firmware).

---

## 5. SDL2 on PS3: required fixes and rules

The prebuilt SDL2 in `zeldin/ps3dev-docker` has two renderer bugs. **Always build with our patched image** (`docker/`):
1. `PSL1GHT_QueueFillRects` converts rects to ints, then `SDL_memcpy`s the raw float bits over them.
   `SDL_RenderFillRect` then writes far outside the framebuffer: crash, black screen.
2. Clear, points, lines and fill-rects use the renderer's *latest* draw color instead of each queued command's color and blend mode.
   Symptom: the whole screen is one color that changes with every `SDL_SetRenderDrawColor`.

Rules for the game code:
- Create the window at the PS3's output size, without `SDL_WINDOW_FULLSCREEN`:
  `SDL_GetDesktopDisplayMode(0, &m); SDL_CreateWindow(title, 0, 0, m.w, m.h, 0);` (ours is 1920x1080).
- Call `SDL_PollEvent` every frame. SDL turns the PS button menu's **Quit Game** into `SDL_QUIT`, so no extra sysutil callback is needed.
- `SDL_RenderCopyEx` (rotation/flip) does nothing in this port. Logical size/scaling is untested.
- Controller buttons arrive as `SDL_JOYBUTTONDOWN` with indices 0–15 (physical mapping still to be documented).

---

## 6. Safety rules for the PS3

- **Game stuck, webMAN still answering:** restart remotely with `curl http://<PS3>/restart.ps3`.
- **Everything frozen** (no HTTP/FTP): hold the PS3 power button ~10 s until it beeps. **Don't unplug**; repeated power cuts risk corruption.
- After any restart: enable HEN, then wait ~30 s before launching (network startup).
- Never install a package while a game or app is running.
- webMAN status pages show the console's PSID/IDPS: never print, share or commit them.

---

## 6b. Driving the PS3 from the Mac

Verified on a real PS3; the Makefile wraps these as `make launch|quit|pad|shot`.

| Goal | Command |
|---|---|
| Launch | `/play.ps3?<APPID>` (game starts after 10-15 s) |
| Quit to the XMB | `/xmb.ps3$exit` (the game logs its "exiting" line) |
| Press buttons | `/pad.ps3?cross\|1\|circle` (`\|n\|` waits n seconds); SDL sees them as a normal controller (Start 4, Cross 9, Circle 10, L1 13 in the log) |
| Restart the PS3 | `/restart.ps3` |
| Screenshot | the game writes `shot.bmp` on L1 (dev builds); `make shot` fetches it over FTP and converts it to `shots/shot.png` |

- webMAN's built-in `/xmb.ps3$ingame_screenshot` (R2+O) saved nothing in `/dev_hdd0/photo`, so the game dumps its own pixel buffer (`screenSaveBmp` in `source/screen.cpp`).
- Every webMAN reply, even `/play.ps3` and `/pad.ps3`, can contain the PSID/IDPS: send curl output to `/dev/null`.
- A stale `make log` listener keeps UDP 18195 and the next one fails with "Address already in use": `lsof -nP -iUDP:18195`, then kill that PID.
- In a Makefile never put a trailing comment on a variable line (`X := 1   # why`): the spaces stay in the value.

---

## 7. Working with Claude on the project

- Keep the project in a git repo (a private GitHub repo for backup) and open the Claude session in that folder.
- `CLAUDE.md` holds the PS3 IP, commands, rules and SDL quirks, so each new session knows them.
- Claude can't change its own permission rules. To cut prompts, add `.claude/settings.json` yourself:
  ```json
  { "permissions": { "allow": [
      "Bash(make ps3)", "Bash(make deploy)", "Bash(make -s log)", "Bash(make clean)",
      "Bash(curl * ftp://192.168.1.200/*)", "Bash(curl * http://192.168.1.200/*)" ] } }
  ```
- Typical test round: Claude runs `make deploy` and starts `make log`, you launch the game and describe the screen, Claude reads the logs.

---

## 8. New-project checklist

1. [ ] PS3: HEN enabled, webMAN MOD installed, IP known.
2. [ ] Mac: Docker Desktop running.
3. [ ] New GitHub repo, cloned locally; open a Claude session in it.
4. [ ] Copy `Makefile`, `ps3.mk`, `docker/`, `.gitignore`, `CLAUDE.md`, and `source/main.cpp` (hello world + watchdog) from nameGame.
5. [ ] Set `TARGET`, `TITLE`, `APPID` in `ps3.mk`; update file names in `Makefile`; set `PS3_IP` if it changed; update `CLAUDE.md`.
6. [ ] `make image` (first time ~30 s), then `make deploy` with `make log` running.
7. [ ] Launch on the PS3: dark screen, bouncing square, buttons change its color, Quit Game logs "exiting".
8. [ ] Commit.
