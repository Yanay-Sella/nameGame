// Hello world: a bouncing square; any controller button changes its color. Logs go over UDP to the Mac.
#include <SDL.h>
#include <debugnet.h>
#include <net/net.h>
#include <sys/thread.h>
#include <sys/systime.h>
#include <lv2/process.h>
#include <rsx/gcm_sys.h>

static bool running = true;

// Progress markers the main loop updates, read by the watchdog thread.
static volatile unsigned frame = 0;
static const char* volatile stage = "startup";

// Dev aid: if the main loop stops advancing for 5 s, log where it is stuck and quit back to the XMB.
static void watchdog(void* arg) {
    unsigned lastFrame = frame;
    int stalledSeconds = 0;
    while (true) {
        sysUsleep(1000000);
        stalledSeconds = (frame == lastFrame) ? stalledSeconds + 1 : 0;
        lastFrame = frame;
        if (stalledSeconds == 5) {
            gcmControlRegister* rsx = gcmGetControlRegister();
            u8 displayed = 0xff;
            gcmGetCurrentDisplayBufferId(&displayed);
            debugNetPrintf(ERROR, "watchdog: stuck in '%s' at frame %u; RSX put=0x%x get=0x%x ref=0x%x flipStatus=%u lastFlip=%lld displayedBuffer=%u\n",
                           stage, frame, rsx->put, rsx->get, rsx->ref, gcmGetFlipStatus(), gcmGetLastFlipTime(), displayed);
            debugNetPrintf(ERROR, "watchdog: exiting\n");
            sysProcessExit(1);
        }
    }
}

int main(int argc, char** argv) {
    netInitialize();
    debugNetInit(LOG_HOST, LOG_PORT, DEBUG);
    debugNetPrintf(INFO, "nameGame started\n");

    sys_ppu_thread_t watchdogThread;
    sysThreadCreate(&watchdogThread, watchdog, NULL, 1000, 0x4000, 0, (char*)"watchdog");

    stage = "SDL_Init";
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        debugNetPrintf(ERROR, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    debugNetPrintf(INFO, "SDL initialized\n");
    stage = "SDL_CreateWindow";
    // Match the window to the PS3's video output so the PS3 SDL port's screen buffers fit it.
    SDL_DisplayMode desktop;
    SDL_GetDesktopDisplayMode(0, &desktop);
    debugNetPrintf(INFO, "video output %dx%d\n", desktop.w, desktop.h);
    SDL_Window* window = SDL_CreateWindow("nameGame", 0, 0, desktop.w, desktop.h, 0);
    debugNetPrintf(INFO, "window %s\n", window ? "created" : "failed");
    stage = "SDL_CreateRenderer";
    SDL_Renderer* renderer = window ? SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC) : NULL;
    if (!renderer) {
        debugNetPrintf(ERROR, "window/renderer failed: %s\n", SDL_GetError());
        return 1;
    }

    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer, &screenW, &screenH);
    debugNetPrintf(INFO, "screen %dx%d, %d controller(s)\n", screenW, screenH, SDL_NumJoysticks());
    if (SDL_NumJoysticks() > 0) SDL_JoystickOpen(0);

    const SDL_Color colors[] = {{230, 60, 60, 255}, {60, 200, 90, 255}, {70, 120, 240, 255}, {240, 200, 50, 255}};
    int colorIndex = 0;
    SDL_Rect square = {100, 100, 120, 120};
    int dx = 6, dy = 4;

    while (running) {
        // SDL also turns the PS button menu's "Quit Game" into SDL_QUIT.
        stage = "SDL_PollEvent";
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_JOYBUTTONDOWN) {
                colorIndex = (colorIndex + 1) % 4;
                debugNetPrintf(INFO, "button %d pressed\n", event.jbutton.button);
            }
        }

        square.x += dx;
        square.y += dy;
        if (square.x < 0 || square.x + square.w > screenW) dx = -dx;
        if (square.y < 0 || square.y + square.h > screenH) dy = -dy;

        stage = "SDL_RenderClear";
        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
        SDL_RenderClear(renderer);
        const SDL_Color& c = colors[colorIndex];
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
        stage = "SDL_RenderFillRect";
        SDL_RenderFillRect(renderer, &square);
        stage = "SDL_RenderPresent";
        SDL_RenderPresent(renderer);

        if (++frame == 1 || frame % 3600 == 0) debugNetPrintf(INFO, "frame %u\n", frame);
    }

    debugNetPrintf(INFO, "nameGame exiting\n");
    SDL_Quit();
    debugNetFinish();
    return 0;
}
