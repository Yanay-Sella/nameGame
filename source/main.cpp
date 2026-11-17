// nameGame: startup, the scenes (menus, play, pause), the main loop (fixed 60 Hz game steps), and copying the picture to the TV.
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
#include "settings.h"

const float STEP = 1.0f / 60;   // s per game update
const int MAX_STEPS_PER_FRAME = 4;

enum Scene { SCENE_MAIN_MENU, SCENE_SETTINGS, SCENE_PLAYING, SCENE_PAUSED };
const char* const SCENE_NAMES[] = {"main menu", "settings", "playing", "paused"};
const char* const MAIN_ITEMS[] = {"CLASSIC", "SETTINGS"};          // game modes are added above SETTINGS
const char* const PAUSE_ITEMS[] = {"RESUME", "SETTINGS", "RETURN TO MENU"};

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

// Dev aid (Triangle): tops the screen up to the 60 fps stress case of 60 aliens, 100 bullets and 2 explosions.
static void addStressLoad(Game& game) {
    for (int i = 0; i < 100; i++) {
        Shot bullet = {(float)(i * 3 % SCREEN_W), (float)(SCREEN_H - 20 - i % 60), -40, 0};
        game.bullets.push_back(bullet);
    }
    for (int i = (int)game.aliens.size(); i < 60; i++) {
        Alien alien = {};
        alien.type = (AlienType)(i % 3);
        alien.state = ALIEN_IN_FORMATION;
        alien.slotX = alien.x = 10.0f + (i % 20) * 15;
        alien.slotY = alien.y = 88.0f - (i / 20) * 12;
        alien.hp = 4;
        alien.alive = true;
        game.aliens.push_back(alien);
    }
    Explosion blasts[2] = {{100, 60, 0, 40}, {220, 60, 0, 40}};
    game.explosions.insert(game.explosions.end(), blasts, blasts + 2);
    debugNetPrintf(INFO, "stress: %d aliens, %d bullets, %d explosions\n", (int)game.aliens.size(),
                   (int)game.bullets.size(), (int)game.explosions.size());
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
    std::string exeFolder = exePath.substr(0, exePath.find_last_of('/'));
    audioOpen(exeFolder.c_str());
    std::string settingsPath = exeFolder + "/settings.txt";
    Settings settings = settingsLoad(settingsPath.c_str());
    debugNetPrintf(INFO, "settings: music %s\n", settings.music ? "on" : "off");

    Game game;
    startGame(game, 1, (uint32_t)SDL_GetPerformanceCounter());   // gives the menu its stars; a second player joins with Start (co-op)
    Scene scene = SCENE_MAIN_MENU, settingsBack = SCENE_MAIN_MENU;
    int selected = 0;
    bool running = true;
    auto setScene = [&](Scene next) {
        scene = next;
        debugNetPrintf(INFO, "scene: %s\n", SCENE_NAMES[next]);
    };

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
        Actions actions[MAX_PLAYERS];
        for (int p = 0; p < MAX_PLAYERS; p++) actions[p] = inputRead(p);
        // Every controller drives the menus.
        Actions menu = {};
        for (const Actions& a : actions) {
            menu.up |= a.up; menu.down |= a.down; menu.left |= a.left; menu.right |= a.right;
            menu.confirm |= a.confirm; menu.back |= a.back; menu.pause |= a.pause;
        }
        int itemCount = scene == SCENE_PAUSED ? 3 : 2;
        if (scene != SCENE_PLAYING) {
            if (menu.up) selected = (selected + itemCount - 1) % itemCount;
            if (menu.down) selected = (selected + 1) % itemCount;
        }

        switch (scene) {
        case SCENE_MAIN_MENU:
            if (menu.confirm && selected == 0) {
                startGame(game, 1, (uint32_t)SDL_GetPerformanceCounter());
                if (settings.music) audioStartMusic();
                setScene(SCENE_PLAYING);
            } else if (menu.confirm && selected == 1) {
                settingsBack = SCENE_MAIN_MENU;
                selected = 0;
                setScene(SCENE_SETTINGS);
            }
            break;
        case SCENE_SETTINGS:
            if (menu.back || (menu.confirm && selected == 1)) {
                selected = 1;   // the SETTINGS item in both menus
                setScene(settingsBack);
            } else if (selected == 0 && (menu.confirm || menu.left || menu.right)) {
                settings.music = !settings.music;
                if (!settings.music) audioStopMusic();
                else if (settingsBack == SCENE_PAUSED) audioStartMusic();
                bool saved = settingsSave(settingsPath.c_str(), settings);
                debugNetPrintf(INFO, "settings: music %s (%s)\n", settings.music ? "on" : "off", saved ? "saved" : "SAVE FAILED");
            }
            break;
        case SCENE_PAUSED:
            if (menu.pause || menu.back || (menu.confirm && selected == 0)) {
                setScene(SCENE_PLAYING);
            } else if (menu.confirm && selected == 1) {
                settingsBack = SCENE_PAUSED;
                selected = 0;
                setScene(SCENE_SETTINGS);
            } else if (menu.confirm && selected == 2) {
                audioStopMusic();
                selected = 0;
                setScene(SCENE_MAIN_MENU);
            }
            break;
        case SCENE_PLAYING:
            for (int p = 0; p < MAX_PLAYERS; p++) {
                // A controller without a ship joins the running game with Start or X (co-op/design.md).
                if (p >= (int)game.players.size()) {
                    if (actions[p].confirm && p == (int)game.players.size() && !game.over) {
                        joinGame(game);
                        debugNetPrintf(INFO, "player %d joined at wave %d\n", p + 1, game.wave);
                    }
                    continue;
                }
                if (actions[p].pause && !game.over) {
                    for (Player& player : game.players) player.rocketPressedAt = -1;   // a buffered R2 doesn't survive a pause
                    selected = 0;
                    setScene(SCENE_PAUSED);
                    break;   // a second Start in the same frame can't act
                }
                if (actions[p].stress) addStressLoad(game);
                // Game over: after the input lock, X or Start begins a new run and Circle goes to the main menu.
                if (game.over && game.time - game.overTime > GAME_OVER_INPUT_LOCK) {
                    if (actions[p].confirm) {
                        startGame(game, (int)game.players.size(), (uint32_t)SDL_GetPerformanceCounter());   // co-op runs stay co-op
                        Uint64 musicStart = SDL_GetPerformanceCounter();
                        if (settings.music) audioStartMusic();
                        debugNetPrintf(INFO, "new run (music restart took %.0f ms)\n", (SDL_GetPerformanceCounter() - musicStart) * 1000.0 / counterHz);
                    } else if (actions[p].back) {
                        selected = 0;
                        setScene(SCENE_MAIN_MENU);
                        break;
                    }
                }
            }
            break;
        }
        bool wasOver = game.over;

        stage = "update";
        int steps = 0;
        while (accumulator >= STEP && steps < MAX_STEPS_PER_FRAME) {
            if (scene == SCENE_PLAYING) updateGame(game, actions, STEP);
            else if (scene == SCENE_MAIN_MENU || (scene == SCENE_SETTINGS && settingsBack == SCENE_MAIN_MENU)) updateBackground(game, STEP);
            for (Actions& a : actions) a.rocket = false;   // a press counts once, in the first step
            accumulator -= STEP;
            steps++;
        }
        if (steps == MAX_STEPS_PER_FRAME) accumulator = 0;   // fell far behind: drop the backlog rather than spiral
        if (game.over && !wasOver) {
            audioStopMusic();
            for (size_t p = 0; p < game.players.size(); p++)
                debugNetPrintf(INFO, "game over: player %d score %d, wave %d\n", (int)p + 1, game.players[p].score, game.wave);
        }
        if (scene == SCENE_PLAYING) {
            for (int s = 0; s < SOUND_COUNT; s++) {
                if (game.soundRequested[s]) audioPlay((Sound)s);
                game.soundRequested[s] = false;
            }
        }

        stage = "draw";
        if (scene == SCENE_MAIN_MENU) {
            drawBackground(game);
            drawMenu("NAMEGAME", MAIN_ITEMS, 2, selected, "X SELECT");
        } else if (scene == SCENE_SETTINGS) {
            if (settingsBack == SCENE_PAUSED) drawGame(game);
            else drawBackground(game);
            const char* settingsItems[] = {settings.music ? "MUSIC: ON" : "MUSIC: OFF", "BACK"};
            drawMenu("SETTINGS", settingsItems, 2, selected, "X CHANGE  O BACK");
        } else {
            drawGame(game);
            if (scene == SCENE_PAUSED) drawMenu("PAUSED", PAUSE_ITEMS, 3, selected, "X SELECT  O RESUME");
        }

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
