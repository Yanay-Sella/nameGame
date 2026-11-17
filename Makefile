# Host-side commands: build in Docker, deploy to the PS3 over the LAN, read its logs.
PS3_IP   ?= 192.168.1.200
LOG_HOST ?= $(shell ipconfig getifaddr en0)
LOG_PORT ?= 18195
SOURCES  ?= source
IMAGE    := namegame-ps3dev
PKG      := build/nameGame.pkg

.PHONY: ps3 image deploy log clean music test

# Compile and package build/nameGame.pkg inside the container (always a full rebuild so LOG_HOST/LOG_PORT changes apply).
# SOURCES=prototypes/<idea> builds a prototype instead of the game.
ps3: clean image
	docker run --rm -v "$(CURDIR)":/src -w /src $(IMAGE) \
		make -f ps3.mk pkg LOG_HOST=$(LOG_HOST) LOG_PORT=$(LOG_PORT) SOURCES=$(SOURCES)

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
	curl -sS --fail -m 60 -T $(PKG) ftp://$(PS3_IP)/dev_hdd0/packages/nameGame.pkg
	curl -sS --fail -m 60 -o /dev/null "http://$(PS3_IP)/install.ps3/dev_hdd0/packages/nameGame.pkg"
	@echo "Installed. Launch nameGame from the PS3's Game menu."

# Build and run the game-rule tests on the Mac (game.cpp and screen.cpp have no SDL or PS3 code).
test:
	mkdir -p build/tests
	c++ -std=c++14 -Wall -Wextra -Isource tests/game_tests.cpp source/game.cpp source/screen.cpp -o build/tests/game_tests
	build/tests/game_tests

# Print the game's UDP log output (Ctrl+C to stop).
log:
	python3 -u -c 'import socket; s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM); s.bind(("", $(LOG_PORT))); [print(s.recv(4096).decode(errors="replace"), end="", flush=True) for _ in iter(int, 1)]'

clean:
	rm -rf build

# Convert a song to OGG for the package (the PS3's SDL2_mixer plays OGG and WAV only): make music MUSIC_SRC=path/to/song.flac
music:
	@test -n "$(MUSIC_SRC)" || { echo "Usage: make music MUSIC_SRC=path/to/song"; exit 1; }
	mkdir -p pkgfiles/USRDIR
	ffmpeg -y -loglevel error -i "$(MUSIC_SRC)" -vn -ar 48000 -ac 2 -c:a libvorbis -q:a 5 pkgfiles/USRDIR/music.ogg
