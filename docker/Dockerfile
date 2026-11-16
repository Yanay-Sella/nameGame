# ps3dev toolchain plus SDL2 rebuilt with our render fixes (sdl2-psl1ght-render.patch) to upstream SDL_PSL1GHT:
# 1. QueueFillRects overwrote its converted rects with raw float bits -> fills far outside the framebuffer (crash).
# 2. Clear/points/lines/fill-rects used the renderer's latest color, not each queued command's color and blend.
FROM zeldin/ps3dev-docker:latest

ARG SDL2_COMMIT=b81fbad4489918f5ed9c9c052238d7a25da9bab9
COPY sdl2-psl1ght-render.patch /tmp/

RUN cd /tmp \
 && wget -q -O sdl2.tar.gz https://github.com/shagkur/SDL_PSL1GHT/archive/${SDL2_COMMIT}.tar.gz \
 && tar xzf sdl2.tar.gz \
 && cd SDL_PSL1GHT-${SDL2_COMMIT} \
 && patch -p1 < /tmp/sdl2-psl1ght-render.patch \
 && ./script.sh \
 && make -j"$(nproc)" \
 && make install \
 && cd /tmp && rm -rf SDL_PSL1GHT-${SDL2_COMMIT} sdl2.tar.gz sdl2-psl1ght-render.patch
