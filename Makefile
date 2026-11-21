# Host-side commands: build in Docker, deploy to the PS3 over the LAN, read its logs.
# Your PS3 address goes in local.mk (gitignored): PS3_IP = <address>; or pass PS3_IP= on the command line.
-include local.mk
PS3_IP   ?= 192.168.1.100
LOG_HOST ?= $(shell ipconfig getifaddr en0)
LOG_PORT ?= 18195
SOURCES  ?= source
IMAGE    := namegame-ps3dev
PKG      := build/nameGame.pkg
VERSION  := 1.0
RELEASE  ?= 0

APP_FOLDER := /dev_hdd0/game/NMGM00001/USRDIR
# A shot is a 54-byte BMP header plus 3 bytes per pixel at 320x180, 384x216 or 640x360.
SHOT_SIZE  := 172854
EXPANDED_SHOT_SIZE := 248886
QUEST_SHOT_SIZE := 691254

.PHONY: ps3 image deploy install release log clean music test launch quit pad shot mac mac-run quest-maps ps3-state

# Compile and package build/nameGame.pkg inside the container (always a full rebuild so LOG_HOST/LOG_PORT changes apply).
# SOURCES=prototypes/<idea> builds a prototype instead of the game.
ps3: clean image
	docker run --rm -v "$(CURDIR)":/src -w /src $(IMAGE) \
		make -f ps3.mk pkg LOG_HOST=$(LOG_HOST) LOG_PORT=$(LOG_PORT) SOURCES=$(SOURCES) RELEASE=$(RELEASE) VERSION=$(VERSION)

# Build the toolchain image (ps3dev + patched SDL2); cached after the first run.
image:
	docker build -q -t $(IMAGE) docker

# Upload the pkg over FTP and install it with webMAN; refuses while an app/game runs (installing then can freeze the PS3),
# and when webMAN doesn't answer, since then it can't check.
deploy: ps3 install

# Upload and install the already built pkg (no rebuild): run it under scripts/ps3-lock.sh so only one session uses the PS3.
install:
	@test -f $(PKG) || { echo "$(PKG) is missing; run make ps3 first."; exit 1; }
	@status=$$(curl -sS --fail -m 10 http://$(PS3_IP)/home.ps3mapi) \
		|| { echo "webMAN didn't answer, so it can't check for a running game; try again."; exit 1; }; \
	echo "$$status" | grep -io '[0-9A-F]\{8\}_[^<" ]*\.\(self\|bin\|elf\)' | grep -qv main_vsh \
		&& { echo "An app or game is running on the PS3; quit it to the XMB first."; exit 1; } || true
	curl -sS --fail -m 600 -w "uploaded %{size_upload} bytes in %{time_total}s\n" -T $(PKG) ftp://$(PS3_IP)/dev_hdd0/packages/nameGame.pkg
	curl -sS --fail -m 60 -o /dev/null "http://$(PS3_IP)/install.ps3/dev_hdd0/packages/nameGame.pkg"
	@echo "Installed. Start it with make launch."

# Release package for other players: no dev aids, logs go nowhere, copied to dist/ with the install guide.
release:
	@test -f pkgfiles/USRDIR/music.ogg || { echo "pkgfiles/USRDIR/music.ogg is missing; run make music first."; exit 1; }
	@test -f pkgfiles/USRDIR/dubstep.ogg || { echo "pkgfiles/USRDIR/dubstep.ogg is missing; run make music MUSIC_OUT=dubstep.ogg first."; exit 1; }
	$(MAKE) ps3 RELEASE=1 LOG_HOST=127.0.0.1
	mkdir -p dist
	cp build/nameGame.pkg "dist/NAME-game-$(VERSION).pkg"
	cp README.md "dist/NAME-game-$(VERSION)-README.md"
	ls -l dist

# Build and run the game-rule and NAME QUEST tests on the Mac (game.cpp, quest*.cpp and screen.cpp have no SDL or PS3 code).
test:
	mkdir -p build/tests
	c++ -std=c++14 -Wall -Wextra -Isource tests/game_tests.cpp source/game.cpp source/formations.cpp source/screen.cpp source/settings.cpp -o build/tests/game_tests
	c++ -std=c++14 -O2 -Wall -Wextra -Isource tests/quest_tests.cpp $(wildcard source/quest*.cpp) source/screen.cpp -o build/tests/quest_tests
	# Both suites always run (a shooter failure must not hide the quest results); either failing fails the target.
	build/tests/game_tests; shooter=$$?; build/tests/quest_tests && exit $$shooter

# Draw every screen of every NAME QUEST area (2 per area, enemies at their spots) into shots/maps/ for checking levels by eye.
quest-maps:
	mkdir -p build/tests build/maps shots/maps
	c++ -std=c++14 -O2 -Wall -Wextra -Isource tests/quest_tests.cpp $(wildcard source/quest*.cpp) source/screen.cpp -o build/tests/quest_tests
	build/tests/quest_tests --maps
	for f in build/maps/*.bmp; do sips -s format png "$$f" --out "shots/maps/$$(basename "$$f" .bmp).png" >/dev/null; done
	@ls shots/maps | wc -l | xargs echo "PNG files in shots/maps:"

# Native Mac build for fast testing (SDL2 from Homebrew, silent, keyboard pads): game logic and drawing are the real code,
# but not the PS3's SDL port, audio or controller, so confirm anything final on the PS3.
MAC_BIN := build/mac/nameGame
mac:
	mkdir -p build/mac
	c++ -std=c++14 -O2 -Wall -Wextra -Wno-unused-parameter -DNAMEGAME_MAC -DRELEASE=0 -DGAME_VERSION=\"$(VERSION)\" \
		-DLOG_HOST=\"127.0.0.1\" -DLOG_PORT=$(LOG_PORT) $$(sdl2-config --cflags) -Isource \
		source/main.cpp source/game.cpp source/formations.cpp source/screen.cpp source/settings.cpp $(wildcard source/quest*.cpp) \
		host/input_keyboard.cpp host/audio_silent.cpp \
		$$(sdl2-config --libs) -o $(MAC_BIN)

# Run the Mac build; BUTTONS='cross|1|l1|1|quit' plays a script like make pad (plus hold 'right:2', chord 'l2+l3', 'quit'),
# and an l1 in it leaves shots/shot.png. Without BUTTONS you play with the keyboard (keys: host/input_keyboard.cpp).
mac-run: mac
	rm -f build/mac/shot.bmp
	NAMEGAME_PAD='$(BUTTONS)' $(MAC_BIN)
	@if [ -f build/mac/shot.bmp ]; then mkdir -p shots && sips -s format png --resampleWidth 1280 build/mac/shot.bmp --out shots/shot.png >/dev/null && echo "shots/shot.png"; fi

# Print the game's UDP log output (Ctrl+C to stop).
log:
	python3 -u -c 'import socket; s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM); s.bind(("", $(LOG_PORT))); [print(s.recv(4096).decode(errors="replace"), end="", flush=True) for _ in iter(int, 1)]'

# Print `running` if an app or game runs on the PS3, else `idle`; fails when webMAN doesn't answer (scripts/ps3-lock.sh).
ps3-state:
	@status=$$(curl -sS --fail -m 10 http://$(PS3_IP)/home.ps3mapi 2>/dev/null) || exit 1; \
	echo "$$status" | grep -io '[0-9A-F]\{8\}_[^<" ]*\.\(self\|bin\|elf\)' | grep -qv main_vsh && echo running || echo idle

# Start the installed game (webMAN mounts it and starts it); refuses while an app or game already runs.
launch:
	@status=$$(curl -sS --fail -m 10 http://$(PS3_IP)/home.ps3mapi) \
		|| { echo "webMAN didn't answer; try again."; exit 1; }; \
	echo "$$status" | grep -io '[0-9A-F]\{8\}_[^<" ]*\.\(self\|bin\|elf\)' | grep -qv main_vsh \
		&& { echo "An app or game is already running on the PS3; make quit first."; exit 1; } || true
	@# /play.ps3?<id> replays the last app (HEN), so open the newest Game-column item through the XMB: webMAN Games, down, cross.
	curl -sS --fail -m 20 -o /dev/null 'http://$(PS3_IP)/xmb.ps3$$focus_category%20game'; sleep 2
	curl -sS --fail -m 20 -o /dev/null 'http://$(PS3_IP)/xmb.ps3$$focus_index%2010'; sleep 2
	curl -sS --fail -m 30 -o /dev/null 'http://$(PS3_IP)/pad.ps3?down|1|cross'
	@echo "Launching NAME game (takes 10-15 s); watch make log for 'nameGame started'."

# Quit the running game to the XMB, then wait until the game is gone and the XMB has redrawn its icons (~10 s),
# because make launch's XMB navigation lands on the wrong item while the XMB is still reloading.
quit:
	curl -sS --fail -m 10 -o /dev/null 'http://$(PS3_IP)/xmb.ps3$$exit'
	@for i in $$(seq 30); do \
		curl -sS -m 10 http://$(PS3_IP)/home.ps3mapi 2>/dev/null | grep -io '[0-9A-F]\{8\}_[^<" ]*\.\(self\|bin\|elf\)' | grep -qv main_vsh || break; \
		sleep 1; \
	done; sleep 10

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
		if curl -s -m 10 -o shots/shot.bmp ftp://$(PS3_IP)$(APP_FOLDER)/shot.bmp && \
			{ [ "$$(stat -f %z shots/shot.bmp)" = "$(SHOT_SIZE)" ] || [ "$$(stat -f %z shots/shot.bmp)" = "$(EXPANDED_SHOT_SIZE)" ] || [ "$$(stat -f %z shots/shot.bmp)" = "$(QUEST_SHOT_SIZE)" ]; }; then \
			sips -s format png --resampleWidth 1280 shots/shot.bmp --out shots/shot.png >/dev/null && echo "shots/shot.png" && exit 0; \
		fi; \
	done; echo "No screenshot arrived: is a dev build of the game running? Check make log."; exit 1

clean:
	rm -rf build

MUSIC_OUT ?= music.ogg
# Convert a song to OGG for the package (the PS3's SDL2_mixer plays OGG and WAV only): make music MUSIC_SRC=path/to/song [MUSIC_OUT=dubstep.ogg]
music:
	@test -n "$(MUSIC_SRC)" || { echo "Usage: make music MUSIC_SRC=path/to/song [MUSIC_OUT=dubstep.ogg]"; exit 1; }
	mkdir -p pkgfiles/USRDIR
	ffmpeg -y -loglevel error -i "$(MUSIC_SRC)" -vn -ar 48000 -ac 2 -c:a libvorbis -q:a 5 pkgfiles/USRDIR/$(MUSIC_OUT)
