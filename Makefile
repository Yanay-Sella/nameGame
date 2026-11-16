# Host-side commands: build in Docker, deploy to the PS3 over the LAN, read its logs.
PS3_IP   ?= 192.168.1.200
LOG_HOST ?= $(shell ipconfig getifaddr en0)
LOG_PORT ?= 18195
IMAGE    := namegame-ps3dev
PKG      := build/nameGame.pkg

.PHONY: ps3 image deploy log clean

# Compile and package build/nameGame.pkg inside the container (always a full rebuild so LOG_HOST/LOG_PORT changes apply).
ps3: clean image
	docker run --rm -v "$(CURDIR)":/src -w /src $(IMAGE) \
		make -f ps3.mk pkg LOG_HOST=$(LOG_HOST) LOG_PORT=$(LOG_PORT)

# Build the toolchain image (ps3dev + patched SDL2); cached after the first run.
image:
	docker build -q -t $(IMAGE) docker

# Upload the pkg over FTP and install it with webMAN; refuses while an app/game runs (installing then can freeze the PS3).
deploy: ps3
	@curl -sS --fail -m 10 http://$(PS3_IP)/home.ps3mapi | grep -io '[0-9A-F]\{8\}_[^<" ]*\.\(self\|bin\|elf\)' | grep -v main_vsh \
		&& { echo "An app or game is running on the PS3; quit it to the XMB first."; exit 1; } || true
	curl -sS --fail -m 60 -T $(PKG) ftp://$(PS3_IP)/dev_hdd0/packages/nameGame.pkg
	curl -sS --fail -m 60 -o /dev/null "http://$(PS3_IP)/install.ps3/dev_hdd0/packages/nameGame.pkg"
	@echo "Installed. Launch nameGame from the PS3's Game menu."

# Print the game's UDP log output (Ctrl+C to stop).
log:
	python3 -u -c 'import socket; s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM); s.bind(("", $(LOG_PORT))); [print(s.recv(4096).decode(errors="replace"), end="", flush=True) for _ in iter(int, 1)]'

clean:
	rm -rf build
