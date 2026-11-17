# Plan: remote test workflow (launch, quit, pad, screenshot from the Mac)

For the implementer: follow the steps in order. Every design choice below is final, so don't redesign anything.
Read only the files each step names, and don't re-read a file after editing it. Code style: follow `CLAUDE.md` and
the existing code (plain C++14, no new libraries, short one-line comments, no helpers for one-off logic).
Don't commit: the user commits their own work. At the end, list the changed files (this plan included) and a one-line
commit message in the style of `git log`. Never `git add -A` (the working tree has unrelated uncommitted changes).
Pipe long output: `make test 2>&1 | tail -15`, `make ps3 2>&1 | tail -8`.

## What this gives
Claude (or the user) drives the PS3 from the Mac with no controller in hand. Everything below was probed on the real
PS3 (webMAN MOD 1.47.50d, firmware 4.93, HEN 3.6.0) except the L1 screenshot, which is new:
- `make launch`: starts the installed game with `curl http://<PS3>/play.ps3?NMGM00001` (10-15 s until "nameGame started").
  **Corrected while building:** the first probe wrongly credited `/play.ps3/dev_hdd0/game/NMGM00001`; tested alone for 60 s it
  did not start the game, while `?NMGM00001` started it twice in a row. See "Built" at the end.
- `make quit`: `curl 'http://<PS3>/xmb.ps3$exit'` quits the game to the XMB (log showed "nameGame exiting").
- `make pad BUTTONS='cross|1|circle'`: `curl 'http://<PS3>/pad.ps3?cross|1|circle'` presses virtual-pad buttons, `|n|`
  waits n seconds. SDL sees them as a normal controller: Start = button 4, Cross = 9, Circle = 10 appeared in the log.
- `make shot`: the game dumps its 320x180 picture to `shot.bmp` next to `EBOOT.BIN` when L1 is pressed on the virtual
  pad (`/pad.ps3?l1`); the Makefile fetches it over FTP and converts it to `shots/shot.png` (960x540) for viewing.
  webMAN's own `/xmb.ps3$ingame_screenshot` (R2+O) saved nothing in `/dev_hdd0/photo`, so it is not used.
- Dev builds only: the L1 screenshot is under `DEV_AIDS` (off in `make release`); a release build ignores L1.

## Step 1: BMP writer (`source/screen.h`, `source/screen.cpp`)
Stdio only (the Mac tests build `screen.cpp`).
```cpp
bool screenSaveBmp(const char* path);   // writes screenPixels as a 24-bit BMP; false when the file couldn't be written
```
- Add the declaration to `screen.h` after `drawMenu`. In `screen.cpp` add `#include <stdio.h>` and implement:
  - File = 54-byte header + `SCREEN_W * SCREEN_H * 3` pixel bytes = 172854 bytes (rows are 960 bytes, no padding needed).
  - Header, little-endian, built in a `uint8_t header[54] = {}` with a small lambda `put32(offset, value)` that stores 4 bytes:
    `header[0]='B'; header[1]='M'`; file size at 2; pixel data offset 54 at 10; DIB size 40 at 14; width at 18; height
    `SCREEN_H` at 22 (positive = rows stored bottom-up); planes 1 at 26 (2 bytes: `header[26]=1`); bits per pixel 24 at 28
    (`header[28]=24`); image size `SCREEN_W*SCREEN_H*3` at 34; everything else 0.
  - Pixel rows from `y = SCREEN_H - 1` down to 0; per pixel write bytes `b, g, r` = `p & 0xFF, (p >> 8) & 0xFF, (p >> 16) & 0xFF`
    from the ARGB value into a `uint8_t row[SCREEN_W * 3]`, then `fwrite(row, 1, sizeof row, file)`.
  - `fopen(path, "wb")`; return false if it fails; return `fclose(file) == 0 && all fwrite calls wrote fully`.
    Track success in a `bool ok` that every `fwrite` result is `&=`-ed into.

## Step 2: L1 action (`source/game.h`, `source/input.cpp`)
- `game.h` › `struct Actions`: add `bool screenshot;   // L1 pressed: dev screenshot (main.cpp)` after `autopilot`.
- `input.cpp`: add `BTN_L1 = 13` to the button enum (value from the SDL port's source; unconfirmed on the PS3 until step 6),
  and after `a.autopilot = ...` add `a.screenshot = pressed[player][BTN_L1];`.

## Step 3: trigger in `source/main.cpp`
- In the "Every controller drives the menus" loop, add `menu.screenshot |= a.screenshot;` to the OR list.
- After the whole draw block (`if (scene == SCENE_MAIN_MENU) {...} else {...}`) and before `stage = "present";`, add:
```cpp
        if (DEV_AIDS && menu.screenshot) {
            stage = "screenshot";
            std::string shotPath = exeFolder + "/shot.bmp";
            debugNetPrintf(INFO, "shot: %s %s\n", shotPath.c_str(), screenSaveBmp(shotPath.c_str()) ? "saved" : "SAVE FAILED");
        }
```
- Update the `DEV_AIDS` comment to `// Triangle stress test, Select autopilot and L1 screenshot; off in make release`.
- Don't add anything else: no queue, no thread, no UDP. The write is about 170 KB on the main thread; one slow frame is fine.

## Step 4: Makefile targets (`Makefile`, `.gitignore`)
- `.gitignore`: add the line `shots/`.
- `Makefile`: add `launch quit pad shot` to `.PHONY`. Add near the `log` target (spaces below are tabs in the real file):
```make
APP_FOLDER := /dev_hdd0/game/NMGM00001/USRDIR
# A shot is a 54-byte BMP header + 320*180*3 pixel bytes (no trailing comment: make would keep the spaces in the value).
SHOT_SIZE  := 172854

# Start the installed game (webMAN mounts it and starts it); refuses while an app or game already runs.
launch:
	@status=$$(curl -sS --fail -m 10 http://$(PS3_IP)/home.ps3mapi) \
		|| { echo "webMAN didn't answer; try again."; exit 1; }; \
	echo "$$status" | grep -io '[0-9A-F]\{8\}_[^<" ]*\.\(self\|bin\|elf\)' | grep -qv main_vsh \
		&& { echo "An app or game is already running on the PS3; make quit first."; exit 1; } || true
	curl -sS --fail -m 20 -o /dev/null "http://$(PS3_IP)/play.ps3?NMGM00001"
	@echo "Launching NAME game; watch make log."

# Quit the running game to the XMB.
quit:
	curl -sS --fail -m 10 -o /dev/null 'http://$(PS3_IP)/xmb.ps3$$exit'

# Press virtual-pad buttons: make pad BUTTONS='cross|1|circle' (names: up down left right cross circle square triangle
# l1 l2 r1 r2 select start; |n| waits n seconds).
pad:
	@test -n "$(BUTTONS)" || { echo "Usage: make pad BUTTONS='cross|1|circle'"; exit 1; }
	curl -sS --fail -m 30 -o /dev/null 'http://$(PS3_IP)/pad.ps3?$(BUTTONS)'

# Dev build only: press L1 on the virtual pad so the game writes shot.bmp, fetch it over FTP, convert it to shots/shot.png.
shot:
	mkdir -p shots
	-curl -s -m 10 -o /dev/null -Q "DELE $(APP_FOLDER)/shot.bmp" ftp://$(PS3_IP)/
	@if curl -s -m 10 -o /dev/null ftp://$(PS3_IP)$(APP_FOLDER)/shot.bmp; then echo "Couldn't delete the old shot.bmp on the PS3."; exit 1; fi
	curl -sS --fail -m 10 -o /dev/null 'http://$(PS3_IP)/pad.ps3?l1'
	@for i in 1 2 3 4 5 6 7 8 9 10; do \
		sleep 1; \
		if curl -s -m 10 -o shots/shot.bmp ftp://$(PS3_IP)$(APP_FOLDER)/shot.bmp && [ "$$(stat -f %z shots/shot.bmp)" = "$(SHOT_SIZE)" ]; then \
			sips -s format png --resampleWidth 960 shots/shot.bmp --out shots/shot.png >/dev/null && echo "shots/shot.png" && exit 0; \
		fi; \
	done; echo "No screenshot arrived: is a dev build of the game running? Check make log."; exit 1
```
- Every curl sends its output to `/dev/null`: webMAN's replies to `/play.ps3` contain the console's PSID/IDPS, which must
  never be printed. Don't remove `-o /dev/null`, and don't add `-v`.
- `make test` doesn't need a Makefile change (`screen.cpp` is already in its `c++` line).

## Step 5: test (`tests/game_tests.cpp`)
New test function, called from `main()` after `waveBonusSkipsAPlayerWhoIsOut();`:
- `screenshotIsAValidBmp`: draw something known (`fillRect(0, 0, SCREEN_W, SCREEN_H, 0xFF000000); fillRect(0, 0, 1, 1, 0xFF112233);`),
  `CHECK(screenSaveBmp("build/tests/shot.bmp"))`; read it back with `fopen`/`fread` into `uint8_t data[54 + SCREEN_W * SCREEN_H * 3]`
  and check: `fread` returned 172854 bytes; `data[0] == 'B' && data[1] == 'M'`; the width at byte 18 is 320 and the height at 22 is 180
  (`data[18] + data[19] * 256`, `data[22]`); the first 3 bytes of the file's last row (`data[size - 960]` on, the top-left pixel, since rows are bottom-up) are
  `0x33, 0x22, 0x11` (blue, green, red); the first 3 pixel bytes (offset 54, bottom-left) are 0.
- `CHECK(!screenSaveBmp("build/tests/no-such-folder/shot.bmp"))`.
- `make test` must pass. Then, once on the Mac, run `sips -g pixelWidth -g pixelHeight build/tests/shot.bmp`; it must report 320x180.

## Step 6: PS3 check (needs the user to do nothing but keep the PS3 on with HEN)
No controller presses are needed; Claude does it all remotely.
1. `make ps3 2>&1 | tail -8`: builds without warnings in the new code.
2. If the game is running: `make quit`. Start `make log` in the background first (`lsof -nP -iUDP:18195` shows a stale listener;
   kill it if the port is taken). `make deploy` (refuses while a game runs), then `make launch`.
3. `make pad BUTTONS='start'` (main menu → CLASSIC is selected, Start confirms), wait 3 s, `make shot`: it must print
   `shots/shot.png`; open it (Read the PNG): it must show the run, with the ship at the bottom. The log must show
   `button 13` and `shot: /dev_hdd0/game/NMGM00001/USRDIR/shot.bmp saved`.
4. If `button 13` doesn't appear (virtual pad names or the SDL index differ), run `make pad BUTTONS='l1'` and find which
   button number the log prints, then fix `BTN_L1`, and if it is a different pad name fix it in `Makefile` and this plan's text.
5. `make quit`; the log must show `nameGame exiting`. Then `make launch` once more to prove the round trip.
6. Don't leave a game running. Report to the user which log lines proved each step.

## Step 7: docs (`CLAUDE.md`, `docs/ps3-new-project-guide.md`, `docs/GDD.md`)
Run `python3 .claude/skills/game-docs/scripts/check_docs.py docs` at the end and fix what it reports.
- `CLAUDE.md`:
  - Hardware setup: after the line about `restart.ps3`, add `- Quit a running game remotely with `make quit` (`/xmb.ps3$exit`); `make launch` starts it again.`
  - Commands: replace the two lines of the `make deploy` item that say the user launches the game and webMAN can't start it with
    `  Then `make launch` starts NAME game on the PS3 (no controller needed).` Add after the `make log` item:
    ```
    - `make launch` / `make quit` — start the installed game (`/play.ps3/dev_hdd0/game/NMGM00001`, mount + start) and quit it to the XMB
      (`/xmb.ps3$exit`). `make launch` refuses while an app runs. Other `/play.ps3?...` forms only mounted or did nothing.
    - `make pad BUTTONS='cross|1|circle'` — press virtual-pad buttons through webMAN (`/pad.ps3?...`, `|n|` waits n s). The game sees them
      as a normal controller (Start, Cross and Circle confirmed), so Claude can play and navigate the menus.
    - `make shot` — dev builds only: L1 on the virtual pad makes the game write `shot.bmp` (its 320x180 picture); fetched over FTP to
      `shots/shot.png` (gitignored). Read the PNG to see the screen. webMAN's own `/xmb.ps3$ingame_screenshot` saved nothing.
    ```
  - Under "Never print or commit the console's PSID/IDPS", append: `Every webMAN reply (even /play.ps3 and /pad.ps3) can contain them: send curl output to /dev/null or filter it.`
  - Under the `make log` item add: `A stale `make log` (python on UDP 18195) makes the next one fail with "Address already in use": `lsof -nP -iUDP:18195`, then kill it.`
- `docs/ps3-new-project-guide.md`:
  - Section 3: replace the paragraph "Then launch the game from the PS3's Game menu. **webMAN can't start installed games remotely**: `/play.ps3/...` only mounts them." with
    "Then start it remotely: `curl http://<PS3>/play.ps3/dev_hdd0/game/<APPID>` (webMAN MOD 1.47.50d, firmware 4.93, HEN 3.6.0; mounts and starts it). Short forms (`/play.ps3?<APPID>`, by name, `col=game&seg=...`) did nothing."
  - New section after section 6, titled `## 6b. Driving the PS3 from the Mac`: a table of verified commands (launch, quit `/xmb.ps3$exit`, pad `/pad.ps3?cross|1|circle`
    with the note that SDL sees them as a controller, restart `/restart.ps3`), then: the built-in `/xmb.ps3$ingame_screenshot` did not save anything in
    `/dev_hdd0/photo`, so the game dumps its own pixel buffer as BMP on L1 and the Makefile fetches it over FTP (copy `screenSaveBmp` and `make shot`);
    every webMAN reply can contain the PSID/IDPS, so send curl output to `/dev/null`; a stale `make log` listener holds UDP 18195.
  - Section 7: replace the "Typical test round" line with "Typical test round: Claude runs `make deploy`, starts `make log`, then `make launch`, drives the game with `make pad` and `make shot`, reads the log, and `make quit`s. The user only re-enables HEN after a PS3 reboot."
  - Section 8 checklist item 7: change "Launch on the PS3:" to "`make launch`, then check on the PS3:".
  - Section 7's permissions example: add `"Bash(make launch)", "Bash(make quit)", "Bash(make pad *)", "Bash(make shot)"` to the allow list.
- `docs/GDD.md`:
  - Decisions: add after D-015 the row `| D-016 | 2026-11-17 | Claude drives PS3 tests remotely: launch and quit through webMAN web commands, input through webMAN's virtual pad, screenshots from a dev-only L1 dump of the game's pixel buffer | ask the user to launch and press buttons; an in-game UDP input listener; webMAN's built-in screenshot | the probe showed webMAN launch, quit and pad work inside the game; its screenshot saved nothing | [CLAUDE.md](../CLAUDE.md) |`
    (match the column order and link style of the D-015 row; if `check_docs.py` rejects the link, point it at `features/music-and-sound/design.md` as D-015 does.)
  - Change log: add `**0.16 (2026-11-17):** remote test workflow: make launch, quit, pad and shot (D-016).`
  - Decision D-015's text lists the dev aids: append `, L1 screenshot` after `Select autopilot` there.

## Done when
- `make test` passes, `make ps3` builds, `check_docs.py` is clean.
- `make launch`, `make pad BUTTONS='start'`, `make shot` (a PNG of the running game appears in `shots/`) and `make quit` all work on the PS3 with no controller touched, proven by the log lines in step 6.

## Built (2026-11-17)
Done and checked on the PS3 with no controller touched: `make launch` (`/play.ps3?NMGM00001`, twice), `make pad` (button 4, 9),
`make shot` (the log showed `button 13` and `shot: ... saved`; `shots/shot.png` showed the run), `make quit` ("nameGame exiting").
Changes made while building:
- Launch uses `/play.ps3?NMGM00001`, not the full path (see above). Docs, Makefile and the `ps3-homebrew-setup` skill say so.
- The first `SHOT_SIZE := 172854   # comment` kept the spaces before `#`, so the size check never matched: no trailing comments on make variables.
- The test for the top-left pixel reads the first 3 bytes of the file's last row (`data[size - 960]`), not the last 3 bytes.
- `make launch` doesn't wait for the game: watch `make log` for `nameGame started`.
