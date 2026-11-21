# nameGame — PS3 homebrew game (instructions for every coding agent: Claude Code, Codex)

C++ game for a real PS3, built with the open-source ps3dev toolchain (PSL1GHT) and SDL2.
Sony's official SDK, and the Unity/Unreal PS3 exports, are not available and must not be used.
The game's title is "NAME game" (GDD D-012); the repo, app ID NMGM00001, build/nameGame.pkg and the log lines keep "nameGame".
Full setup history, debugging toolkit and new-project checklist: `docs/ps3-new-project-guide.md`.

## Hardware setup
- PS3 address: `PS3_IP` in the gitignored `local.mk` (the Makefile reads it; or pass `PS3_IP=`). In curl commands use that address (`grep PS3_IP local.mk`). Firmware 4.93, PS3HEN + webMAN MOD.
- HEN turns off on every PS3 reboot: if the PS3 doesn't respond (no FTP/HTTP), ask the user to restart it and re-enable HEN.
- Quit a running game remotely with `make quit` (`/xmb.ps3$exit`); `make launch` starts it again.
- If only the game is stuck, webMAN still answers: restart remotely with `curl http://$PS3_IP/restart.ps3`.
  If webMAN doesn't answer either, ask the user to hold the power button ~10 s (never unplug).
- Never install while an app/game is running (it froze the PS3 once); `make deploy` checks for this.
- Never print or commit the console's PSID/IDPS, which webMAN's status pages show.
  Every webMAN reply (even `/play.ps3` and `/pad.ps3`) can contain them: send curl output to `/dev/null` or filter it.

## Commands (run on the Mac)
- `make ps3` — full rebuild in Docker → `build/nameGame.pkg`. Uses image `namegame-ps3dev` (`docker/Dockerfile`:
  `zeldin/ps3dev-docker` + SDL2 rebuilt with `docker/sdl2-psl1ght-render.patch`), built automatically by `make image`.
- **One PS3, many sessions:** each session works in its own git worktree (own `build/`, `pkgfiles/`, `shots/`), builds with `make ps3` outside any lock,
  and runs every step that touches the PS3 (install, launch, pad, shot, quit) in ONE locked command, never as separate calls:
  `scripts/ps3-lock.sh bash -c 'make install launch && make pad BUTTONS=... && make shot'`.
  It waits while another session holds the lock (`~/.nameGame-ps3.lock`, shared by all worktrees), drops a lock whose owner died,
  and quits the game when done. Run `make log` inside the locked command too (UDP 18195 is shared). Don't call `make deploy/launch/pad/shot` unlocked.
- `make deploy` — build, upload via FTP to `/dev_hdd0/packages/`, install with webMAN (`/install.ps3/...`).
  Then `make launch` starts NAME game on the PS3 (no controller needed).
- `make mac` / `make mac-run [BUTTONS=...]` — native Mac build (Homebrew SDL2, no Docker, no PS3) for fast iteration: the real game code,
  but silent, with keyboard pads (keys in `host/input_keyboard.cpp`) and logs on stdout. `BUTTONS='cross|3|right:1|r1:3|l1|1|quit'` scripts it
  like `make pad` (`|n|` waits, `name:s` holds, `l2+l3` chords, `quit` exits); an `l1` leaves `shots/shot.png`. It opens a 1280x720 window.
  It does NOT use the PS3's SDL port, audio or controller, so confirm anything final on the PS3. PS3-only calls in `main.cpp` are stubbed by `host/ps3_stubs.h` (`NAMEGAME_MAC`).
  `NAMEGAME_QUEST='slot=3 class=0 world=8 area=0' make mac-run BUTTONS=...` starts NAME QUEST in play (spec §20).
- `make log` — print the game's UDP log lines (port 18195). Run it before launching the game.
  `make ps3` deletes `build/`, so don't redirect the log into `build/`.
  A stale `make log` (python on UDP 18195) makes the next one fail with "Address already in use": `lsof -nP -iUDP:18195`, then kill that PID.
- `make launch` / `make quit` — start the installed game (XMB sequence: Game column, webMAN Games, down, cross; `/play.ps3?<id>` just replays the last app, e.g. HEN; takes 10-15 s, the log shows "nameGame started") and
  quit it to the XMB (`/xmb.ps3$exit`; then waits ~10 s for the XMB icons, since launching into a reloading XMB focuses the wrong item). `make launch` refuses while an app runs. `/play.ps3/dev_hdd0/game/NMGM00001` did NOT start it.
- `make pad BUTTONS='cross|1|circle'` — press virtual-pad buttons through webMAN (`/pad.ps3?...`, `|n|` waits n s). The game sees them
  as a normal controller (Start, Cross, Circle and L1 confirmed in the log), so Claude can play and navigate the menus.
- L2+L3 together (dev builds only) toggle a test mode: no ship can die; the log shows `test mode on/off`.
- `make shot` — dev builds only: L1 on the virtual pad makes the game write `shot.bmp` at the current scene's resolution. It is fetched over FTP to
  `shots/shot.png` (gitignored). Read the PNG to see the screen. webMAN's own `/xmb.ps3$ingame_screenshot` saved nothing.
- `make release` — release build (no dev aids, logs to 127.0.0.1, song required) → `dist/NAME-game-<VERSION>.pkg` and the README.
- `make deploy SOURCES=prototypes/<idea>` — build and install a throwaway prototype instead of `source/` (same app ID).
- `make music MUSIC_SRC=<song>` — convert a song to `pkgfiles/USRDIR/music.ogg` (MIZRAHIT); add `MUSIC_OUT=dubstep.ogg` for the DUBSTEP song. Both gitignored, packaged next to EBOOT.BIN.
- `python3 docs/xmb-art/make_xmb_art.py` — redraw the XMB Game menu art (`pkgfiles/ICON0.PNG` 320x176, `PIC1.PNG` 1920x1080)
  from the sprites in `source/sprites.h`; rerun it after changing sprites or the palette.

## Code
- `source/*.cpp` — game code. Design docs: `docs/GDD.md` and `docs/features/`. `ps3.mk` — PSL1GHT build rules (runs inside Docker). `Makefile` — host commands.
- Log with `debugNetPrintf(INFO, ...)`; the Mac's IP is baked in at build time (`LOG_HOST`).
- Poll SDL events every frame: SDL turns the PS button menu's "Quit Game" into `SDL_QUIT`.
- A watchdog thread logs where the main loop is stuck (and RSX state) after 5 s without a frame, or after 20 s of only all-black frames, then exits to the XMB.
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
- `source/input.cpp` polls all seven PS3 ports with PSL1GHT and maps `PAD_CTRL_*` buttons to actions. Each joined player keeps their assigned port across disconnects.
- The pinned SDL joystick driver enumerates by connected count. The game's PSL1GHT polling handles gaps in port numbers and supplies live lobby status.

## Working agreement (applies to every agent and to the user)
- **Own worktree, own branch.** Claude works in `../nameGame-claude` (`claude/work`), Codex in `../nameGame-codex` (`codex/work`); never edit the main folder or `main` (the user merges into it). First command of every session: `pwd; git branch --show-current`. If you are in the main folder or on `main`, stop and tell the user to start you from your worktree (`scripts/agent-sync.sh <claude|codex>` prints the folder). Then run `scripts/agent-sync.sh <your name>` to merge `main` in. Details and merge steps: `docs/COLLAB.md` › Worktrees.
- **Never commit or push.** The user commits. End a task by listing the changed files and a suggested one-line commit message.
- **Read `docs/HANDOFF.md` first** and update it before you stop: it holds the current state, the next step and the session log. It is how Claude and Codex pass work to each other.
- **Read `docs/COLLAB.md` too**: the shared channel between Claude, Codex and the user (agreed conventions, small-change lane, message board). Add questions and notes there; never rewrite another author's text.
- **Plans are specs.** `docs/work-plan/` holds the plans; `docs/work-plan/name-quest-plan.md` explains how to work through a phase.
  Decisions in a plan or in `docs/GDD.md` are final: don't redesign them; if a number is wrong, report it instead of changing it.
- **Classic variants:** when the user requests a change to Classic 1–2 players, ask whether to apply it to Classic 1–4 players too. Check the shared shooter code for effects on both variants before editing.
- **Skills:** the task skills live in `.claude/skills/<name>/SKILL.md` (Codex finds them through `.agents/skills`, symlinks to the same folders).
  Read the matching `SKILL.md` before the task: `name-plan` (write a phase/feature plan), `stage-themes` (a themed level), `game-docs` (GDD and feature docs),
  `game-feel` (juice), `input-systems`, `level-design`, `prototype-fast`. `.claude/skills/_third-party` is not a skill.
- **Verify cheapest first:** `make test`, then `make mac-run` with a `BUTTONS=` script and `l1` screenshots (open `shots/shot.png` and look at it), the PS3 last.
  Known: `tests/game_tests.cpp` has 53 failing checks that fail the same way at commit `ab92b67` (shooter tests, not NAME QUEST); `tests/quest_tests.cpp` must stay green.
- **Graphics:** NAME QUEST draws at 640x360 with small old-school sprites; match the existing sprites' palette, outline and proportions (`source/sprites.h`, `source/quest_sprites*.cpp`, spec §22).
  After every visual change take a screenshot and look at it before saying it is done; the user judges the art.
- **Don't touch the PS3 unless asked.** PS3 steps go in one `scripts/ps3-lock.sh` command; if the PS3 doesn't answer, ask the user.

## Code style (the user's preferences)
- Simple, explicit, readable code: main control flow visible, one function readable top to bottom. Plain C++14, no new libraries, no virtual classes, no `new`/`delete`.
- No helper/wrapper for logic used once, no pass-through functions, no abstraction for hypothetical reuse, no mechanical DRY. Search the codebase for an existing function before adding one.
  Extract only for real reuse, genuinely complex logic, a business rule, an external interaction, or testability.
- Bug fixes and small changes: the smallest reasonable change, no unrelated refactors. Fix the cause, not the symptom (no swallowed errors, retries, special cases that hide the bug); if a root-cause fix is out of scope, say so.
- Comments: at most one line on a function, at most three on a struct or other large definition.
- Before choosing a significant new technology, framework, library or architecture, ask the user, with 2-3 options and a recommendation; don't add its dependencies first.
