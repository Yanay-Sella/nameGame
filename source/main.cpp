// nameGame: startup, the main loop (fixed 60 Hz game steps), pause, and copying the picture to the TV.
#include <SDL.h>
#include <debugnet.h>
#include <net/net.h>
#include <sys/thread.h>
#include <sys/systime.h>
#include <lv2/process.h>
#include <rsx/gcm_sys.h>
#include <string>
#include "audio.h"
#include "game.h"
#include "input.h"
#include "screen.h"

const float STEP = 1.0f / 60;   // s per game update
const int MAX_STEPS_PER_FRAME = 4;
const int PLAYER_COUNT = 1;

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
            debugNetPrintf(ERROR, "watchdog: stuck in '%s' at frame %u; RSX put=0x%x get=0x%x flipStatus=%u\n",
                           stage, frame, rsx->put, rsx->get, gcmGetFlipStatus());
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
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) != 0) {
        debugNetPrintf(ERROR, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    // The window matches the PS3's video output; the picture is copied into it scaled (CLAUDE.md, GDD D-008).
    SDL_DisplayMode desktop;
    SDL_GetDesktopDisplayMode(0, &desktop);
    SDL_Window* window = SDL_CreateWindow("nameGame", 0, 0, desktop.w, desktop.h, 0);
    SDL_Renderer* renderer = window ? SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC) : NULL;
    SDL_Texture* picture = renderer ? SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                                        SCREEN_W, SCREEN_H) : NULL;
    if (!picture) {
        debugNetPrintf(ERROR, "window/renderer/texture failed: %s\n", SDL_GetError());
        return 1;
    }
    int outputW, outputH;
    SDL_GetRendererOutputSize(renderer, &outputW, &outputH);
    int scale = SDL_min(outputW / SCREEN_W, outputH / SCREEN_H);
    SDL_Rect pictureRect = {(outputW - SCREEN_W * scale) / 2, (outputH - SCREEN_H * scale) / 2, SCREEN_W * scale, SCREEN_H * scale};
    debugNetPrintf(INFO, "output %dx%d, picture x%d at %d,%d\n", outputW, outputH, scale, pictureRect.x, pictureRect.y);

    stage = "audio";
    std::string exePath = argc > 0 ? argv[0] : "/dev_hdd0/game/NMGM00001/USRDIR/EBOOT.BIN";
    audioOpen(exePath.substr(0, exePath.find_last_of('/')).c_str());

    Game game;
    startGame(game, PLAYER_COUNT, (uint32_t)SDL_GetPerformanceCounter());
    audioStartMusic();
    bool paused = false, running = true;

    const double counterHz = (double)SDL_GetPerformanceFrequency();
    Uint64 lastCounter = SDL_GetPerformanceCounter();
    float accumulator = 0, statsTimer = 0, worstWorkMs = 0;
    int framesSinceStats = 0;

    while (running) {
        Uint64 frameStart = SDL_GetPerformanceCounter();
        accumulator += (float)((frameStart - lastCounter) / counterHz);
        lastCounter = frameStart;

        // SDL also turns the PS button menu's "Quit Game" into SDL_QUIT.
        stage = "events";
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            inputHandleEvent(event);
        }
        Actions actions[PLAYER_COUNT];
        for (int p = 0; p < PLAYER_COUNT; p++) actions[p] = inputRead(p);
        for (int p = 0; p < PLAYER_COUNT; p++) {
            if (actions[p].pause && !game.over) paused = !paused;
            // Game over: X or Start begins a new run once the input lock has passed (M3's screens replace this).
            if (actions[p].confirm && game.over && game.time - game.overTime > GAME_OVER_INPUT_LOCK) {
                startGame(game, PLAYER_COUNT, (uint32_t)SDL_GetPerformanceCounter());
                Uint64 musicStart = SDL_GetPerformanceCounter();
                audioStartMusic();
                debugNetPrintf(INFO, "new run (music restart took %.0f ms)\n", (SDL_GetPerformanceCounter() - musicStart) * 1000.0 / counterHz);
            }
        }
        bool wasOver = game.over;

        stage = "update";
        int steps = 0;
        while (accumulator >= STEP && steps < MAX_STEPS_PER_FRAME) {
            if (!paused) updateGame(game, actions, STEP);
            for (Actions& a : actions) a.rocket = false;   // a press counts once, in the first step
            accumulator -= STEP;
            steps++;
        }
        if (steps == MAX_STEPS_PER_FRAME) accumulator = 0;   // fell far behind: drop the backlog rather than spiral
        if (game.over && !wasOver) {
            audioStopMusic();
            debugNetPrintf(INFO, "game over: score %d, wave %d\n", game.players[0].score, game.wave);
        }
        for (int s = 0; s < SOUND_COUNT; s++) {
            if (game.soundRequested[s]) audioPlay((Sound)s);
            game.soundRequested[s] = false;
        }

        stage = "draw";
        drawGame(game);
        if (paused) drawText((SCREEN_W - textWidth("PAUSED", 2)) / 2, 84, "PAUSED", 0xFFFFFFFF, 2);

        stage = "present";
        SDL_UpdateTexture(picture, NULL, screenPixels, SCREEN_W * sizeof(uint32_t));
        if (frame < 2) {   // borders are never drawn over, so clearing both screen buffers once is enough
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
        }
        SDL_RenderCopy(renderer, picture, NULL, &pictureRect);
        float workMs = (float)((SDL_GetPerformanceCounter() - frameStart) * 1000.0 / counterHz);
        SDL_RenderPresent(renderer);

        // Every 5 s: frames per second and the worst work time (the 60 fps budget is 16.7 ms).
        framesSinceStats++;
        worstWorkMs = SDL_max(worstWorkMs, workMs);
        statsTimer += (float)((SDL_GetPerformanceCounter() - frameStart) / counterHz);
        if (statsTimer >= 5) {
            debugNetPrintf(INFO, "fps %.1f, worst work %.1f ms, wave %d, score %d\n", framesSinceStats / statsTimer,
                           worstWorkMs, game.wave, game.players[0].score);
            framesSinceStats = 0;
            statsTimer = 0;
            worstWorkMs = 0;
        }
        frame++;
    }

    debugNetPrintf(INFO, "nameGame exiting\n");
    audioClose();
    SDL_Quit();
    debugNetFinish();
    return 0;
}
