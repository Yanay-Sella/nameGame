# Host-side commands: build in Docker, deploy to the PS3 over the LAN, read its logs.
PS3_IP   ?= 192.168.1.200
LOG_HOST ?= $(shell ipconfig getifaddr en0)
LOG_PORT ?= 18195
SOURCES  ?= source
IMAGE    := namegame-ps3dev
PKG      := build/nameGame.pkg
VERSION  := 1.0
RELEASE  ?= 0

APP_FOLDER := /dev_hdd0/game/NMGM00001/USRDIR
# A shot is a 54-byte BMP header + 320*180*3 pixel bytes (no trailing comment: make would keep the spaces in the value).
SHOT_SIZE  := 172854

.PHONY: ps3 image deploy release log clean music test launch quit pad shot

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
deploy: ps3
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
	$(MAKE) ps3 RELEASE=1 LOG_HOST=127.0.0.1
	mkdir -p dist
	cp build/nameGame.pkg "dist/NAME-game-$(VERSION).pkg"
	cp README.md "dist/NAME-game-$(VERSION)-README.md"
	ls -l dist

# Build and run the game-rule tests on the Mac (game.cpp and screen.cpp have no SDL or PS3 code).
test:
	mkdir -p build/tests
	c++ -std=c++14 -Wall -Wextra -Isource tests/game_tests.cpp source/game.cpp source/screen.cpp source/settings.cpp -o build/tests/game_tests
	build/tests/game_tests

# Print the game's UDP log output (Ctrl+C to stop).
log:
	python3 -u -c 'import socket; s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM); s.bind(("", $(LOG_PORT))); [print(s.recv(4096).decode(errors="replace"), end="", flush=True) for _ in iter(int, 1)]'

# Start the installed game (webMAN mounts it and starts it); refuses while an app or game already runs.
launch:
	@status=$$(curl -sS --fail -m 10 http://$(PS3_IP)/home.ps3mapi) \
		|| { echo "webMAN didn't answer; try again."; exit 1; }; \
	echo "$$status" | grep -io '[0-9A-F]\{8\}_[^<" ]*\.\(self\|bin\|elf\)' | grep -qv main_vsh \
		&& { echo "An app or game is already running on the PS3; make quit first."; exit 1; } || true
	curl -sS --fail -m 20 -o /dev/null "http://$(PS3_IP)/play.ps3?NMGM00001"
	@echo "Launching NAME game (takes 10-15 s); watch make log for 'nameGame started'."

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

clean:
	rm -rf build

# Convert a song to OGG for the package (the PS3's SDL2_mixer plays OGG and WAV only): make music MUSIC_SRC=path/to/song.flac
music:
	@test -n "$(MUSIC_SRC)" || { echo "Usage: make music MUSIC_SRC=path/to/song"; exit 1; }
	mkdir -p pkgfiles/USRDIR
	ffmpeg -y -loglevel error -i "$(MUSIC_SRC)" -vn -ar 48000 -ac 2 -c:a libvorbis -q:a 5 pkgfiles/USRDIR/music.ogg
