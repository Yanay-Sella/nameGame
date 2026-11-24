// nameGame: startup, the scenes (menus, play, pause), the main loop (fixed 60 Hz game steps), and copying the picture to the TV.
#include <SDL.h>
#include <algorithm>
#ifdef NAMEGAME_MAC   // make mac: a native build for fast testing, see host/
#include "../host/ps3_stubs.h"
#else
#include <debugnet.h>
#include <net/net.h>
#include <sys/thread.h>
#include <sys/systime.h>
#include <lv2/process.h>
#include <rsx/gcm_sys.h>
#include <malloc.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "audio.h"
#include "game.h"
#include "input.h"
#include "quest.h"
#include "screen.h"
#include "settings.h"

const float STEP = 1.0f / 60;   // s per game update
const int MAX_STEPS_PER_FRAME = 4;
const bool DEV_AIDS = !RELEASE;

enum Scene { SCENE_MAIN_MENU, SCENE_CLASSIC_MENU, SCENE_CONTROLLERS, SCENE_LEVEL_PICKER, SCENE_SETTINGS,
             SCENE_PLAYING, SCENE_PAUSED, SCENE_QUEST, SCENE_ROLES };
const char* const SCENE_NAMES[] = {"main menu", "classic menu", "controllers", "level picker", "settings", "playing", "paused", "quest", "roles"};
const char* const MAIN_ITEMS[] = {"CLASSIC", "NAME QUEST", "SETTINGS"};
const int QUEST_ITEM = 1, SETTINGS_ITEM = 2;
const char* const CLASSIC_ITEMS[] = {"1-2 PLAYERS", "1-4 PLAYERS", "LEVEL PICKER"};
const int PICKER_ITEM = 2;
const int EXPANDED_MIN_CONTROLLERS = 3;
const char* const PICKER_NAMES[LEVEL_COUNT] = {"SPACE", "JUNGLE", "GLACIER", "VOLCANO", "DEEP SEA"};
const float PICKER_SPIN_TIME = 0.12f;   // s for the picker wheel to turn one row
const char* const BOSS_NAMES[] = {"classic", "mage", "monkey", "gorilla", "yeti", "golem", "imp", "fire lord", "angler",
                                  "fisherman"};   // log names, by BossType
const char* const PAUSE_ITEMS[] = {"RESUME", "SETTINGS", "RETURN TO MENU"};
const char* const MUSIC_NAMES[] = {"off", "mizrahit", "dubstep"};                          // log names, indexed by Music
const char* const MUSIC_LABELS[] = {"MUSIC: OFF", "MUSIC: MIZRAHIT", "MUSIC: DUBSTEP"};   // settings item, indexed by Music

// Progress markers the main loop updates, read by the watchdog thread.
static volatile unsigned frame = 0;
static volatile unsigned visibleFrames = 0;   // frames presented with at least one non-black pixel
static const char* volatile stage = "startup";
const int STALL_EXIT_SECONDS = 5;
const int BLACK_SCREEN_EXIT_SECONDS = 20;

// Quits back to the XMB when the main loop stops for 5 s or nothing but black is presented for 20 s.
static void watchdog(void*) {
    unsigned lastFrame = frame, lastVisible = visibleFrames;
    int stalledSeconds = 0, blackSeconds = 0;
    while (true) {
        sysUsleep(1000000);
        stalledSeconds = (frame == lastFrame) ? stalledSeconds + 1 : 0;
        blackSeconds = (visibleFrames == lastVisible) ? blackSeconds + 1 : 0;
        lastFrame = frame;
        lastVisible = visibleFrames;
        if (stalledSeconds == STALL_EXIT_SECONDS || blackSeconds == BLACK_SCREEN_EXIT_SECONDS) {
            gcmControlRegister* rsx = gcmGetControlRegister();
            if (stalledSeconds == STALL_EXIT_SECONDS)
                debugNetPrintf(ERROR, "watchdog: stuck in '%s' at frame %u; RSX put=0x%x get=0x%x flipStatus=%u\n",
                               stage, frame, rsx->put, rsx->get, gcmGetFlipStatus());
            else
                debugNetPrintf(ERROR, "watchdog: screen black for %d s in '%s' at frame %u; RSX put=0x%x get=0x%x flipStatus=%u\n",
                               BLACK_SCREEN_EXIT_SECONDS, stage, frame, rsx->put, rsx->get, gcmGetFlipStatus());
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
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        debugNetPrintf(ERROR, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
#ifndef NAMEGAME_MAC
    if (!inputOpen()) {
        SDL_Quit();
        return 1;
    }
#endif
    // The window matches the PS3's video output; the picture is copied into it scaled (CLAUDE.md, GDD D-008).
    SDL_DisplayMode desktop;
    SDL_GetDesktopDisplayMode(0, &desktop);
#ifdef NAMEGAME_MAC
    desktop.w = 1280;   // a 4x window instead of the whole desktop
    desktop.h = 720;
#endif
    SDL_Window* window = SDL_CreateWindow("NAME game", 0, 0, desktop.w, desktop.h, 0);
    SDL_Renderer* renderer = window ? SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC) : NULL;
    SDL_Texture* picture = renderer ? SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                                        SCREEN_W, SCREEN_H) : NULL;
    SDL_Texture* expandedPicture = renderer ? SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                                                EXPANDED_W, EXPANDED_H) : NULL;
    SDL_Texture* questPicture = renderer ? SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                                             QUEST_W, QUEST_H) : NULL;   // NAME QUEST's 640x360 (D-053)
    if (!picture || !expandedPicture || !questPicture) {
        debugNetPrintf(ERROR, "window/renderer/texture failed: %s\n", SDL_GetError());
        return 1;
    }
    int outputW, outputH;
    SDL_GetRendererOutputSize(renderer, &outputW, &outputH);
    int scale = SDL_min(outputW / SCREEN_W, outputH / SCREEN_H);
    SDL_Rect pictureRect = {(outputW - SCREEN_W * scale) / 2, (outputH - SCREEN_H * scale) / 2, SCREEN_W * scale, SCREEN_H * scale};
    int expandedScale = SDL_min(outputW / 16, outputH / 9);
    int expandedOutputW = 16 * expandedScale, expandedOutputH = 9 * expandedScale;
    SDL_Rect expandedRect = {(outputW - expandedOutputW) / 2, (outputH - expandedOutputH) / 2, expandedOutputW, expandedOutputH};
    int questScale = SDL_min(outputW / QUEST_W, outputH / QUEST_H);
    SDL_Rect questRect = {(outputW - QUEST_W * questScale) / 2, (outputH - QUEST_H * questScale) / 2, QUEST_W * questScale, QUEST_H * questScale};
    debugNetPrintf(INFO, "output %dx%d, picture x%d at %d,%d, quest x%d\n", outputW, outputH, scale, pictureRect.x, pictureRect.y, questScale);
#ifdef NAMEGAME_MAC
    questClock = [] { return SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency(); };
#else
    questClock = [] { return sysGetSystemTime() / 1e6; };   // µs; SDL's counter on the PS3 ticks only every 1 ms
#endif

    stage = "audio";
    std::string exePath = argc > 0 ? argv[0] : "/dev_hdd0/game/NMGM00001/USRDIR/EBOOT.BIN";
    std::string exeFolder = exePath.substr(0, exePath.find_last_of('/'));
    audioOpen(exeFolder.c_str());
    std::string settingsPath = exeFolder + "/settings.txt";
    Settings settings = settingsLoad(settingsPath.c_str());
    debugNetPrintf(INFO, "settings: music %s, shake %d, flashing %s\n", MUSIC_NAMES[settings.music], settings.shake,
                   settings.reduceFlashing ? "reduced" : "normal");
    const std::string highScorePaths[] = {exeFolder + "/highscore.txt", exeFolder + "/highscore-expanded.txt"};
    int highScores[] = {highScoreLoad(highScorePaths[CLASSIC_STANDARD].c_str()), highScoreLoad(highScorePaths[CLASSIC_EXPANDED].c_str())};
    debugNetPrintf(INFO, "high score: %d\n", highScores[CLASSIC_STANDARD]);
    debugNetPrintf(INFO, "classic 1-4 record: %d\n", highScores[CLASSIC_EXPANDED]);

    Game game;
    startGame(game, 1, (uint32_t)SDL_GetPerformanceCounter());   // gives the menu its stars; a second player joins with Start (co-op)
    Quest quest;
    bool questPaused = false;   // the pause menu (and settings from it) sits over NAME QUEST, not the shooter
    Actions questCarry = {};    // NAME QUEST presses read in a frame that ran no game step, kept for the next frame
    Scene scene = SCENE_MAIN_MENU, settingsBack = SCENE_MAIN_MENU;
    int selected = 0;
    int lobbyPlayerChoice = 0;   // 0 starts with as many players as connected pads, up to four
    int playerPorts[MAX_PLAYERS] = {};
    int rolePorts[MAX_PLAYERS] = {};
    ShipRole roles[MAX_PLAYERS] = {};
    bool roleReady[MAX_PLAYERS] = {};
    int roleParty = 0, roleJoinFrom = 0;
    Scene roleBack = SCENE_CONTROLLERS;
    Actions roleCarry[MAX_PLAYERS] = {};
    int questPort = 0;
    int runLevel = 1;   // the level the menu started; play again restarts there
    bool runSingleLevel = false;   // a level picker run: play again replays just that level
    float pickerSpin = 0;   // rows the picker wheel still has to turn: + after Down, - after Up
    bool autopilot = false;
    bool godMode = false;   // dev test mode (L2+L3): ships can't die
    int loggedBuffs[MAX_PLAYERS] = {}, loggedStrong[MAX_PLAYERS] = {};   // what the log last said about each player's buffs
    const char* const STRONG_NAMES[6] = {"rocket", "magic beam", "", "frost wave", "", "bubble shield"};   // by Player::strongAttack
    float bossStart = 0;   // game time the current boss appeared
    bool running = true;
    auto setScene = [&](Scene next) {
        scene = next;
        debugNetPrintf(INFO, "scene: %s\n", SCENE_NAMES[next]);
    };
    auto chooseRoles = [&](int count, int joined, const int* ports, Scene back) {
        roleParty = count;
        roleJoinFrom = joined;
        roleBack = back;
        std::copy_n(ports, count, rolePorts);
        for (int p = 0; p < count; p++) {
            roles[p] = back == SCENE_PLAYING && p < (int)game.players.size() && game.players.size() > STANDARD_PLAYERS
                       ? game.players[p].role : (ShipRole)p;
            roleReady[p] = joined > STANDARD_PLAYERS && p < joined;
        }
        setScene(SCENE_ROLES);
    };
    auto openQuest = [&]() {
        audioStopMusic();   // the shooter's song stops; NAME QUEST picks its own sound (D-057)
        questOpen(quest, exeFolder.c_str());
        quest.devAids = DEV_AIDS;
        setScene(SCENE_QUEST);
    };
    if (DEV_AIDS && getenv("NAMEGAME_QUEST")) {   // Mac test scripts start straight into NAME QUEST play (spec §20)
        openQuest();
        questStartDev(quest, getenv("NAMEGAME_QUEST"));
    }

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
#ifdef NAMEGAME_MAC
            inputHandleEvent(event);
#endif
        }
        Actions pads[CONTROLLER_PORTS];
        inputRead(pads);
        Actions actions[MAX_PLAYERS] = {};
        if (scene == SCENE_QUEST) {
            Actions& a = actions[0];
            a = pads[questPort];
            a.jump |= questCarry.jump; a.item |= questCarry.item; a.back |= questCarry.back; a.up |= questCarry.up;
            a.down |= questCarry.down; a.left |= questCarry.left; a.right |= questCarry.right;
            a.godMode |= questCarry.godMode; a.autopilot |= questCarry.autopilot; a.rocket |= questCarry.rocket;
            a.cycle |= questCarry.cycle; a.firePressed |= questCarry.firePressed; a.nextSkill |= questCarry.nextSkill;
        }
        questCarry = {};
        // Every controller drives the menus.
        Actions menu = {};
        int connectedCount = 0;
        int connectedPorts[MAX_PLAYERS] = {};
        for (int port = 0; port < CONTROLLER_PORTS; port++) {
            const Actions& a = pads[port];
            if (!a.connected) continue;
            if (connectedCount < MAX_PLAYERS) connectedPorts[connectedCount] = port;
            connectedCount++;
            menu.up |= a.up; menu.down |= a.down; menu.left |= a.left; menu.right |= a.right;
            menu.confirm |= a.confirm; menu.item |= a.item; menu.back |= a.back; menu.pause |= a.pause; menu.screenshot |= a.screenshot;
        }
        int available = SDL_min(connectedCount, MAX_PLAYERS);
        int lobbyPlayers = lobbyPlayerChoice == 0 ? available : SDL_min(lobbyPlayerChoice, available);
        // A run waits for a disconnected player's assigned port (D-014).
        int lostController = -1;
        for (int p = (int)game.players.size() - 1; p >= 0; p--)
            if (!pads[playerPorts[p]].connected) lostController = playerPorts[p];
        if (questPaused) lostController = pads[questPort].connected ? -1 : questPort;
        int itemCount = scene == SCENE_MAIN_MENU ? SDL_arraysize(MAIN_ITEMS)
                      : scene == SCENE_CLASSIC_MENU ? SDL_arraysize(CLASSIC_ITEMS)
                      : scene == SCENE_LEVEL_PICKER ? LEVEL_COUNT : scene == SCENE_SETTINGS ? 4 : scene == SCENE_PAUSED ? 3 : 0;
        if (itemCount > 0) {
            if (menu.up) selected = (selected + itemCount - 1) % itemCount;
            if (menu.down) selected = (selected + 1) % itemCount;
        }

        auto startRun = [&](int playerCount, ClassicVariant variant, const int* ports, const ShipRole* picks = nullptr) {
            std::copy_n(ports, playerCount, playerPorts);
            startGame(game, playerCount, (uint32_t)SDL_GetPerformanceCounter(), runLevel, runSingleLevel, variant, picks);
            audioStartMusic(settings.music);
            setScene(SCENE_PLAYING);
        };
        Scene frameScene = scene;   // a press that changes the scene this frame never also reaches NAME QUEST
        switch (scene) {
        case SCENE_MAIN_MENU:
            if (menu.confirm && selected == 0) {
                game.level = 1;
                setScene(SCENE_CLASSIC_MENU);
            } else if (menu.confirm && selected == QUEST_ITEM) {
                questPort = connectedPorts[0];
                openQuest();
            } else if (DEV_AIDS && menu.item && selected == QUEST_ITEM) {   // dev: Triangle on NAME QUEST warps into the TEST GROUNDS (slot 3)
                questPort = connectedPorts[0];
                openQuest();
                questStartDev(quest, "slot=3 class=0 world=8 area=0");
            } else if (menu.confirm && selected == SETTINGS_ITEM) {
                settingsBack = SCENE_MAIN_MENU;
                selected = 0;
                setScene(SCENE_SETTINGS);
            }
            break;
        case SCENE_CLASSIC_MENU:
            if (menu.back) {
                selected = 0;
                setScene(SCENE_MAIN_MENU);
            } else if (menu.confirm && selected == 0) {
                runLevel = 1;
                runSingleLevel = false;
                debugNetPrintf(INFO, "classic run starts\n");
                startRun(1, CLASSIC_STANDARD, connectedPorts);
            } else if (menu.confirm && selected == PICKER_ITEM) {
                selected = runSingleLevel ? runLevel - 1 : 0;   // back on the last picked level
                pickerSpin = 0;
                setScene(SCENE_LEVEL_PICKER);
            } else if (menu.confirm) {
                lobbyPlayerChoice = 0;
                lobbyPlayers = available;
                setScene(SCENE_CONTROLLERS);
            }
            break;
        case SCENE_CONTROLLERS:
            if (menu.back) {
                selected = 1;
                setScene(SCENE_CLASSIC_MENU);
            } else if (available >= EXPANDED_MIN_CONTROLLERS) {
                if (menu.left || menu.right) {
                    lobbyPlayers = SDL_max(1, SDL_min(lobbyPlayers + (menu.left ? -1 : 1), available));
                    lobbyPlayerChoice = lobbyPlayers;
                }
                if (menu.confirm) {
                    runLevel = 1;
                    runSingleLevel = false;
                    debugNetPrintf(INFO, "classic 1-4 run starts: %d players, %d controllers\n", lobbyPlayers, connectedCount);
                    if (lobbyPlayers > STANDARD_PLAYERS) chooseRoles(lobbyPlayers, 0, connectedPorts, SCENE_CONTROLLERS);
                    else startRun(lobbyPlayers, CLASSIC_EXPANDED, connectedPorts);
                }
            }
            break;
        case SCENE_ROLES: {
            bool allReady = true, cancel = false;
            for (int p = 0; p < roleParty; p++) {
                const Actions& a = pads[rolePorts[p]];
                bool locked = roleJoinFrom > STANDARD_PLAYERS && p < roleJoinFrom;
                if (!a.connected) {
                    if (!locked) roleReady[p] = false;
                    allReady = false;
                    continue;
                }
                if (a.back) {
                    if (locked || !roleReady[p]) cancel = true;
                    else roleReady[p] = false;
                } else if (!roleReady[p]) {
                    if (a.left || a.right) roles[p] = (ShipRole)((roles[p] + (a.left ? ROLE_COUNT - 1 : 1)) % ROLE_COUNT);
                    if (a.jump) roleReady[p] = true;
                }
                allReady &= roleReady[p];
            }
            if (cancel) {
                setScene(roleBack);
            } else if (allReady) {
                for (int p = 0; p < roleParty; p++)
                    debugNetPrintf(INFO, "player %d role: %s\n", p + 1, ROLE_NAMES[roles[p]]);
                if (roleJoinFrom == 0) {
                    startRun(roleParty, CLASSIC_EXPANDED, rolePorts, roles);
                } else {
                    for (int p = roleJoinFrom; p < roleParty; p++) {
                        if (joinGame(game, roles)) {
                            playerPorts[p] = rolePorts[p];
                            debugNetPrintf(INFO, "player %d joined at wave %d\n", p + 1, game.wave);
                        }
                    }
                    setScene(SCENE_PLAYING);
                }
            }
            break;
        }
        case SCENE_LEVEL_PICKER:
            // The wheel shows the picked level in the middle; it starts a run of just that level.
            if (menu.down) pickerSpin = SDL_min(pickerSpin + 1, 2.0f);
            if (menu.up) pickerSpin = SDL_max(pickerSpin - 1, -2.0f);
            game.level = selected + 1;   // the background previews the picked level
            if (menu.confirm) {
                runLevel = selected + 1;
                runSingleLevel = true;
                debugNetPrintf(INFO, "picked level %d starts\n", runLevel);
                startRun(1, CLASSIC_STANDARD, connectedPorts);
            } else if (menu.back) {
                game.level = 1;
                selected = PICKER_ITEM;
                setScene(SCENE_CLASSIC_MENU);
            }
            break;
        case SCENE_SETTINGS:
            if (menu.back || (menu.confirm && selected == 3)) {
                selected = settingsBack == SCENE_MAIN_MENU ? SETTINGS_ITEM : 1;   // the SETTINGS item
                setScene(settingsBack);
            } else if (selected < 3 && (menu.confirm || menu.left || menu.right)) {
                if (selected == 0) {
                    settings.music = (Music)((settings.music + (menu.left ? MUSIC_CHOICES - 1 : 1)) % MUSIC_CHOICES);   // Mizrahit → Dubstep → Off → Mizrahit
                    if (settingsBack == SCENE_PAUSED && !questPaused) audioStartMusic(settings.music);   // the main menu and NAME QUEST stay silent
                } else if (selected == 1) {
                    settings.shake = (settings.shake + (menu.left ? -25 : 25) + 125) % 125;   // 0, 25 ... 100, wrapping
                } else {
                    settings.reduceFlashing = !settings.reduceFlashing;
                }
                bool saved = settingsSave(settingsPath.c_str(), settings);
                debugNetPrintf(INFO, "settings: music %s, shake %d, flashing %s (%s)\n", MUSIC_NAMES[settings.music], settings.shake,
                               settings.reduceFlashing ? "reduced" : "normal", saved ? "saved" : "SAVE FAILED");
            }
            break;
        case SCENE_PAUSED:
            if (menu.pause || menu.back || (menu.confirm && selected == 0)) {
                if (lostController < 0) {
                    setScene(questPaused ? SCENE_QUEST : SCENE_PLAYING);
                    questPaused = false;
                }
            } else if (menu.confirm && selected == 1) {
                settingsBack = SCENE_PAUSED;
                selected = 0;
                setScene(SCENE_SETTINGS);
            } else if (menu.confirm && selected == 2) {
                if (questPaused) questSave(quest);   // RETURN TO MENU autosaves the slot (spec §17)
                questPaused = false;
                audioStopMusic();
                selected = 0;
                setScene(SCENE_MAIN_MENU);
            }
            break;
        case SCENE_QUEST:
            if (quest.wantsMainMenu) {
                quest.wantsMainMenu = false;
                selected = QUEST_ITEM;
                setScene(SCENE_MAIN_MENU);
            } else if ((actions[0].pause || !actions[0].connected) && questCanPause(quest)) {
                quest.hero.queuedAttack = BASIC_NONE;
                questPaused = true;
                selected = 0;
                setScene(SCENE_PAUSED);
            }
            break;
        case SCENE_PLAYING: {
            if (lostController >= 0 && !game.over) {
                for (Player& player : game.players) player.rocketPressedAt = -1;   // as for a Start pause
                selected = 0;
                debugNetPrintf(INFO, "paused: controller %d disconnected\n", lostController + 1);
                setScene(SCENE_PAUSED);
                break;
            }
            int playerCount = (int)game.players.size();
            bool canJoin = !game.over && !game.levelClear;
            for (int p = 0; p < playerCount; p++) {
                const Actions& a = pads[playerPorts[p]];
                if (a.pause && !game.over) {
                    for (Player& player : game.players) player.rocketPressedAt = -1;   // a buffered R2 doesn't survive a pause
                    selected = 0;
                    setScene(SCENE_PAUSED);
                    break;   // a second Start in the same frame can't act
                }
                if (DEV_AIDS && a.godMode) {
                    godMode = !godMode;
                    if (!godMode) for (Player& player : game.players) player.invincibleTimer = 0;
                    else for (Player& player : game.players) player.buffs = BUFF_ALL;   // test mode on: every level buff
                    debugNetPrintf(INFO, "test mode %s\n", godMode ? "on" : "off");
                }
                if (DEV_AIDS && a.autopilot) { autopilot = !autopilot; debugNetPrintf(INFO, "autopilot %s\n", autopilot ? "on" : "off"); }
                // Game over: after the input lock, X or Start begins a new run and Circle goes to the main menu.
                if (game.over && game.time - game.overTime > GAME_OVER_INPUT_LOCK) {
                    if (a.confirm) {
                        if (game.variant == CLASSIC_EXPANDED && (available < EXPANDED_MIN_CONTROLLERS || lostController >= 0)) {
                            lobbyPlayerChoice = playerCount;
                            lobbyPlayers = SDL_min(playerCount, available);
                            game.level = 1;
                            setScene(SCENE_CONTROLLERS);
                        } else if (game.variant == CLASSIC_EXPANDED && playerCount > STANDARD_PLAYERS) {
                            chooseRoles(playerCount, 0, playerPorts, SCENE_PLAYING);
                        } else {
                            startGame(game, playerCount, (uint32_t)SDL_GetPerformanceCounter(), runLevel, runSingleLevel, game.variant);
                            Uint64 musicStart = SDL_GetPerformanceCounter();
                            audioStartMusic(settings.music);
                            debugNetPrintf(INFO, "new run (music restart took %.0f ms)\n", (SDL_GetPerformanceCounter() - musicStart) * 1000.0 / counterHz);
                        }
                        break;
                    } else if (a.back) {
                        selected = 0;
                        setScene(SCENE_MAIN_MENU);
                        break;
                    }
                }
            }
            if (scene == SCENE_PLAYING && canJoin) {
                int partyPorts[MAX_PLAYERS];
                std::copy_n(playerPorts, playerCount, partyPorts);
                int partyCount = playerCount;
                for (int port = 0; port < CONTROLLER_PORTS && partyCount < game.playerLimit(); port++) {
                    if (!pads[port].connected || !pads[port].confirm) continue;
                    if (std::find(playerPorts, playerPorts + playerCount, port) == playerPorts + playerCount)
                        partyPorts[partyCount++] = port;
                }
                if (partyCount > playerCount && partyCount > STANDARD_PLAYERS) {
                    for (Player& player : game.players) player.rocketPressedAt = -1;
                    chooseRoles(partyCount, playerCount, partyPorts, SCENE_PLAYING);
                } else {
                    for (int p = playerCount; p < partyCount; p++) {
                        if (joinGame(game)) {
                            playerPorts[p] = partyPorts[p];
                            debugNetPrintf(INFO, "player %d joined at wave %d\n", p + 1, game.wave);
                        }
                    }
                }
            }
            break;
        }
        }
        if (scene == SCENE_PLAYING)
            for (size_t p = 0; p < game.players.size(); p++) {
                actions[p] = pads[playerPorts[p]];
                actions[p].rocket |= roleCarry[p].rocket;
                actions[p].cycle |= roleCarry[p].cycle;
            }
        for (Actions& carry : roleCarry) carry = {};
        // Dev aid (Select): player 1 sweeps along the bottom firing, a rocket every 5 s, invincible (30-minute soak test).
        if (autopilot && scene == SCENE_PLAYING) {
            Player& pilot = game.players[0];
            float targetX = game.arenaWidth() / 2.0f - 8 + game.arenaWidth() * 0.375f * sinf(game.time * 0.7f);
            actions[0].moveX = SDL_max(-1.0f, SDL_min((targetX - pilot.x) / 20, 1.0f));
            actions[0].moveY = 1;
            actions[0].fire = true;
            actions[0].rocket = frame % 300 == 0;
            if (pilot.alive) pilot.invincibleTimer = 1;   // runs out 1 s after the autopilot is switched off
        }

        if (godMode && scene == SCENE_PLAYING)   // dev test mode: every living ship stays invincible
            for (Player& player : game.players) if (player.alive) player.invincibleTimer = 1;

        bool wasOver = game.over;
        int bossesBefore = 0;
        bool beamBefore[MAX_BOSSES] = {}, laughBefore[MAX_BOSSES] = {};
        for (size_t i = 0; i < game.bosses.size(); i++) {
            bossesBefore += game.bosses[i].alive;
            beamBefore[i] = game.bosses[i].beamTimer > 0;
            laughBefore[i] = game.bosses[i].laughTimer > 0;
        }
        bool clearBefore = game.levelClear;
        int waveBefore = game.wave;
        int levelBefore = game.level;
        size_t mistsBefore = game.mists.size();
        size_t fishBefore = game.fish.size();
        int livesBefore[MAX_PLAYERS] = {};
        bool slidingBefore[MAX_PLAYERS] = {}, frozenBefore[MAX_PLAYERS] = {}, scorchedBefore[MAX_PLAYERS] = {}, hookedBefore[MAX_PLAYERS] = {};
        for (size_t p = 0; p < game.players.size(); p++) {
            livesBefore[p] = game.players[p].lives;
            slidingBefore[p] = game.players[p].slideTimer > 0;
            frozenBefore[p] = game.players[p].frozenTimer > 0;
            scorchedBefore[p] = game.players[p].scorchTimer > 0;
            hookedBefore[p] = game.players[p].hookedTimer > 0;
        }
        bool shieldBefore[MAX_PLAYERS] = {}, beamFiredBefore[MAX_PLAYERS] = {};
        for (size_t p = 0; p < game.players.size(); p++) {
            shieldBefore[p] = game.players[p].shield;
            beamFiredBefore[p] = game.players[p].beamTimer > 0;
        }
        bool frozenWaveBefore = game.freezeTimer > 0;
        int buffPickupsBefore = (int)std::count_if(game.pickups.begin(), game.pickups.end(), [](const Pickup& pk) { return pk.type == PICKUP_BUFF; });
        bool bootBefore = std::any_of(game.thrown.begin(), game.thrown.end(), [](const Thrown& t) { return t.kind == THROWN_BOOT; });

        stage = "update";
        int steps = 0;
        while (accumulator >= STEP && steps < MAX_STEPS_PER_FRAME) {
            if (scene == SCENE_PLAYING) updateGame(game, actions, STEP);
            else if (scene == SCENE_QUEST && frameScene == SCENE_QUEST) questUpdate(quest, actions[0], STEP);
            else if (scene == SCENE_MAIN_MENU || scene == SCENE_CLASSIC_MENU || scene == SCENE_CONTROLLERS ||
                     scene == SCENE_LEVEL_PICKER || (scene == SCENE_SETTINGS && settingsBack == SCENE_MAIN_MENU))
                updateBackground(game, STEP);
            if (pickerSpin > 0) pickerSpin = SDL_max(0.0f, pickerSpin - STEP / PICKER_SPIN_TIME);
            if (pickerSpin < 0) pickerSpin = SDL_min(0.0f, pickerSpin + STEP / PICKER_SPIN_TIME);
            for (Actions& a : actions) a.rocket = a.cycle = false;   // a press counts once, in the first step
            Actions& a0 = actions[0];
            a0.firePressed = a0.nextSkill = false;
            a0.jump = a0.item = a0.back = a0.up = a0.down = a0.left = a0.right = a0.confirm = a0.pause = a0.godMode = a0.autopilot = false;
            accumulator -= STEP;
            steps++;
        }
        if (steps == MAX_STEPS_PER_FRAME) accumulator = 0;   // fell far behind: drop the backlog rather than spiral
        if (steps == 0 && scene == SCENE_QUEST && frameScene == SCENE_QUEST) questCarry = actions[0];
        if (steps == 0 && scene == SCENE_PLAYING && game.players.size() > STANDARD_PLAYERS)
            std::copy_n(actions, game.players.size(), roleCarry);
        if (game.level != levelBefore) debugNetPrintf(INFO, "level %d starts at %.1f s\n", game.level, game.time);
        if (game.wave != waveBefore) {
            for (size_t p = 0; p < game.players.size(); p++)
                debugNetPrintf(INFO, "level %d wave %d start at %.0f s: player %d lives %d, rockets %d, weapon %d, score %d\n", game.level,
                               game.wave, game.time, (int)p + 1, game.players[p].lives, game.players[p].rockets, game.players[p].weaponLevel,
                               game.players[p].score);
            int heavies = heavyCount(game.level, game.wave, game.variant);
            if (heavies > 0) debugNetPrintf(INFO, "wave %d: %d heavies\n", game.wave, heavies);
        }
        for (size_t p = 0; p < game.players.size(); p++)
            if (game.players[p].slideTimer > 0 && !slidingBefore[p]) debugNetPrintf(INFO, "player %d slipped on a peel\n", (int)p + 1);
        for (size_t p = 0; p < game.players.size(); p++) {
            const Player& player = game.players[p];
            if (player.frozenTimer > 0 && !frozenBefore[p]) debugNetPrintf(INFO, "player %d frozen\n", (int)p + 1);
            if (player.scorchTimer > 0 && !scorchedBefore[p]) debugNetPrintf(INFO, "player %d scorched\n", (int)p + 1);
            if (player.hookedTimer > 0 && !hookedBefore[p]) debugNetPrintf(INFO, "player %d hooked\n", (int)p + 1);
        }
        for (size_t p = 0; p < game.players.size(); p++) {
            const Player& player = game.players[p];
            if (player.buffs != loggedBuffs[p]) debugNetPrintf(INFO, "player %d buffs 0x%02x, strong attack %s\n", (int)p + 1, player.buffs, STRONG_NAMES[player.strongAttack]);
            else if (player.strongAttack != loggedStrong[p]) debugNetPrintf(INFO, "player %d strong attack %s\n", (int)p + 1, STRONG_NAMES[player.strongAttack]);
            loggedBuffs[p] = player.buffs;
            loggedStrong[p] = player.strongAttack;
            if (player.beamTimer > 0 && !beamFiredBefore[p]) debugNetPrintf(INFO, "player %d magic beam at x %.0f\n", (int)p + 1, player.beamX);
            if (player.shield && !shieldBefore[p]) debugNetPrintf(INFO, "player %d shield up\n", (int)p + 1);
            if (!player.shield && shieldBefore[p] && player.alive) debugNetPrintf(INFO, "player %d shield popped\n", (int)p + 1);
        }
        if (game.freezeTimer > 0 && !frozenWaveBefore) {
            int aliensFrozen = (int)std::count_if(game.aliens.begin(), game.aliens.end(), [](const Alien& a) { return a.alive; });
            int bossesFrozen = (int)std::count_if(game.bosses.begin(), game.bosses.end(), [](const Boss& b) { return b.alive; });
            debugNetPrintf(INFO, "frost wave: %d aliens, %d bosses frozen\n", aliensFrozen, bossesFrozen);
        }
        if (std::count_if(game.pickups.begin(), game.pickups.end(), [](const Pickup& pk) { return pk.type == PICKUP_BUFF; }) > buffPickupsBefore) {
            for (const Pickup& pk : game.pickups)
                if (pk.type == PICKUP_BUFF) debugNetPrintf(INFO, "buff pickup dropped: level %d at wave %d\n", pk.level, game.wave);
        }
        if (!bootBefore && std::any_of(game.thrown.begin(), game.thrown.end(), [](const Thrown& t) { return t.kind == THROWN_BOOT; }))
            debugNetPrintf(INFO, "boot thrown\n");
        if (game.mists.size() > mistsBefore)
            debugNetPrintf(INFO, "stink mist at %.0f,%.0f\n", game.mists.back().x, game.mists.back().y);
        if (game.fish.size() > fishBefore) debugNetPrintf(INFO, "live fish landed, %d out\n", (int)game.fish.size());
        for (size_t i = 0; i < game.bosses.size(); i++)
            if (game.bosses[i].laughTimer > 0 && !laughBefore[i]) debugNetPrintf(INFO, "gorilla healed to %d hp\n", game.bosses[i].hp);
        for (size_t p = 0; p < game.players.size(); p++)
            if (game.players[p].lives < livesBefore[p])
                debugNetPrintf(INFO, "player %d died at wave %d (%d lives left)\n", (int)p + 1, game.wave, game.players[p].lives);
        int bossesAlive = 0;
        for (const Boss& boss : game.bosses) bossesAlive += boss.alive;
        if (bossesAlive > 0 && bossesBefore == 0) {
            bossStart = game.time;
            debugNetPrintf(INFO, "boss wave %d: %d %s, hp %d\n", game.wave, bossesAlive,
                           BOSS_NAMES[game.bosses[0].type], game.bosses[0].hp);
        }
        if (bossesAlive < bossesBefore)
            debugNetPrintf(INFO, "boss down at wave %d after %.0f s, %d left\n", game.wave, game.time - bossStart, bossesAlive);
        for (size_t i = 0; i < game.bosses.size(); i++)
            if (game.bosses[i].beamTimer > 0 && !beamBefore[i])
                debugNetPrintf(INFO, "%s at x %.0f\n", game.bosses[i].type == BOSS_FIRELORD ? "eruption" : "mage beam", game.bosses[i].ballX);
        if (game.levelClear && !clearBefore) debugNetPrintf(INFO, "level %d clear at %.1f s\n", game.level, game.time);
        int& highScore = highScores[game.variant];
        if (game.over && !wasOver) {
            audioStopMusic();
            int best = 0;
            for (const Player& player : game.players) best = SDL_max(best, player.score);
            if (best > highScore) {
                highScore = best;
                game.newHighScore = true;
                bool saved = highScoreSave(highScorePaths[game.variant].c_str(), highScore);
                debugNetPrintf(INFO, "high score: %d (%s)\n", highScore, saved ? "saved" : "SAVE FAILED");
            }
            for (size_t p = 0; p < game.players.size(); p++)
                debugNetPrintf(INFO, "%s at %.1f s: player %d score %d, wave %d\n", game.won ? "level clear" : "game over", game.overTime,
                               (int)p + 1, game.players[p].score, game.wave);
        }
        if (scene == SCENE_PLAYING) {
            for (int s = 0; s < SOUND_COUNT; s++) {
                if (game.soundRequested[s]) audioPlay((Sound)s);
                game.soundRequested[s] = false;
            }
        }
        if (scene == SCENE_QUEST) {
            for (int s = 0; s < SOUND_COUNT; s++) {
                if (quest.soundRequested[s]) audioPlay((Sound)s);
                quest.soundRequested[s] = false;
            }
        }
        // NAME QUEST's song or ambience plays in the quest and in the pause menu and settings over it (spec §16, D-057).
        bool overQuest = scene == SCENE_QUEST || (questPaused && (scene == SCENE_PAUSED || scene == SCENE_SETTINGS));
        audioSetQuestSound(overQuest ? quest.ambient : 0, quest.arenaLocked, settings.music != MUSIC_OFF);
        for (const std::string& line : quest.logLines) debugNetPrintf(INFO, "%s\n", line.c_str());
        quest.logLines.clear();
        quest.shakeStrength = settings.shake / 100.0f;
        quest.reduceFlashing = settings.reduceFlashing;

        // Rumble: a short buzz for a lost life or a popped shield, a long one for a boss kill, and the screen shake's strength between.
        unsigned activePorts = 0;
        for (size_t p = 0; scene == SCENE_PLAYING && p < game.players.size(); p++) {
            int port = playerPorts[p];
            activePorts |= 1u << port;
            if (game.players[p].lives < livesBefore[p] || (shieldBefore[p] && !game.players[p].shield && game.players[p].alive))
                inputRumblePulse(port, 0.8f, 0.25f);
            if (bossesAlive < bossesBefore) inputRumblePulse(port, 1.0f, 1.5f);
        }
        if (scene == SCENE_QUEST) {
            activePorts = 1u;   // NAME QUEST uses controller port 0
            switch (quest.rumbleRequested) {
            case QUEST_RUMBLE_NONE: break;
            case QUEST_RUMBLE_HURT: inputRumblePulse(0, 0.28f, 0.10f); break;
            case QUEST_RUMBLE_DEATH: inputRumblePulse(0, 0.45f, 0.32f); break;
            case QUEST_RUMBLE_BOSS: inputRumblePulse(0, 0.60f, 0.65f); break;
            }
        }
        quest.rumbleRequested = QUEST_RUMBLE_NONE;
        float rumbleShake = scene == SCENE_PLAYING ? game.trauma * settings.shake / 100.0f : 0;
        inputRumbleUpdate(activePorts, rumbleShake, steps * STEP);

        game.highScore = highScore;
        game.shakeStrength = settings.shake / 100.0f;
        game.reduceFlashing = settings.reduceFlashing;
        stage = "draw";
        bool questFrame = scene == SCENE_QUEST || questPaused;
        bool shooterFrame = scene == SCENE_PLAYING || scene == SCENE_PAUSED || (scene == SCENE_SETTINGS && settingsBack == SCENE_PAUSED);
        bool expandedFrame = shooterFrame && game.variant == CLASSIC_EXPANDED;
        const uint32_t* previousTarget = screenTarget;
        SDL_Texture* shown = picture;
        const SDL_Rect* destination = &pictureRect;
        if (questFrame) {
            screenSetTarget(questPixels, QUEST_W, QUEST_H);
            shown = questPicture;
            destination = &questRect;
        } else if (expandedFrame) {
            screenSetTarget(expandedPixels, EXPANDED_W, EXPANDED_H);
            shown = expandedPicture;
            destination = &expandedRect;
        } else {
            screenSetTarget(screenPixels, SCREEN_W, SCREEN_H);
        }
        if (scene == SCENE_MAIN_MENU) {
            drawBackground(game);
            drawMenu("NAME GAME", MAIN_ITEMS, SDL_arraysize(MAIN_ITEMS), selected, "X SELECT");
            char best[24];
            snprintf(best, sizeof best, "HI %07d", highScores[CLASSIC_STANDARD]);
            drawText((SCREEN_W - textWidth(best)) / 2, 150, best, paletteColor('y'));
            drawText(SCREEN_W - 2 - textWidth("V" GAME_VERSION), SCREEN_H - 7, "V" GAME_VERSION, paletteColor('s'));
        } else if (scene == SCENE_CLASSIC_MENU) {
            drawBackground(game);
            drawMenu("CLASSIC", CLASSIC_ITEMS, SDL_arraysize(CLASSIC_ITEMS), selected, "X SELECT  O BACK");
        } else if (scene == SCENE_CONTROLLERS) {
            drawBackground(game);
            const int panelW = 174, panelH = 124, panelX = (SCREEN_W - panelW) / 2, panelY = (SCREEN_H - panelH) / 2;
            fillRect(panelX, panelY, panelW, panelH, paletteColor('n'));
            fillRect(panelX + 1, panelY + 1, panelW - 2, panelH - 2, paletteColor('k'));
            drawText((SCREEN_W - textWidth("1-4 PLAYERS", 2)) / 2, panelY + 9, "1-4 PLAYERS", paletteColor('w'), 2);
            char status[32];
            snprintf(status, sizeof status, "%d CONNECTED", connectedCount);
            drawText((SCREEN_W - textWidth(status)) / 2, panelY + 25, status, paletteColor('w'));
            for (int p = 0; p < MAX_PLAYERS; p++) {
                int y = panelY + 40 + 11 * p;
                snprintf(status, sizeof status, "PLAYER %d", p + 1);
                drawText(panelX + 24, y, status, paletteColor(p < available ? 'w' : 's'));
                if (p < available) snprintf(status, sizeof status, "CONTROLLER %d", connectedPorts[p] + 1);
                else snprintf(status, sizeof status, "WAITING");
                drawText(panelX + panelW - 24 - textWidth(status), y, status, paletteColor(p < available ? 'c' : 's'));
            }
            bool ready = available >= EXPANDED_MIN_CONTROLLERS;
            if (ready) snprintf(status, sizeof status, "<  START WITH %d  >", lobbyPlayers);
            else snprintf(status, sizeof status, "%d CONTROLLERS REQUIRED", EXPANDED_MIN_CONTROLLERS);
            drawText((SCREEN_W - textWidth(status)) / 2, panelY + 90, status, paletteColor('y'));
            const char* hint = ready ? "X PLAY  O BACK" : "O BACK";
            drawText((SCREEN_W - textWidth(hint)) / 2, panelY + 108, hint, paletteColor('s'));
        } else if (scene == SCENE_ROLES) {
            drawBackground(game);
            const char* title = "CHOOSE YOUR ROLE";
            drawText((SCREEN_W - textWidth(title, 2)) / 2, 19, title, paletteColor('w'), 2);
            const char* const traits[ROLE_COUNT][3] = {
                {"NORMAL SPEED", "NORMAL SHOTS", ""}, {"SLOW SHIP", "SLOW HEAVY FIRE", "2 HITS / LIFE"},
                {"FAST SHIP", "FAST LIGHT FIRE", ""}, {"NORMAL SPEED", "STRONG SHOTS", ""}
            };
            const int wide = 74, gap = 4, top = 45, height = 94;
            const int left = (SCREEN_W - roleParty * (wide + gap) + gap) / 2;
            for (int p = 0; p < roleParty; p++) {
                int x = left + p * (wide + gap), middle = x + wide / 2;
                ShipRole role = roles[p];
                bool connected = pads[rolePorts[p]].connected;
                uint32_t ink = paletteColor(connected ? PLAYER_COLORS[p] : 's');
                fillRect(x, top, wide, height, ink);
                fillRect(x + 1, top + 1, wide - 2, height - 2, paletteColor('k'));
                char label[16];
                snprintf(label, sizeof label, "PLAYER %d", p + 1);
                drawText(middle - textWidth(label) / 2, top + 6, label, ink);
                drawShip(role, p, roleJoinFrom > 0 ? game.level : 1, (frame / 12) % 2, middle - 8, top + 18);
                drawText(middle - textWidth(ROLE_NAMES[role]) / 2, top + 38, ROLE_NAMES[role], paletteColor('w'));
                if (connected && !roleReady[p]) {
                    drawText(x + 4, top + 38, "<", ink);
                    drawText(x + wide - 9, top + 38, ">", ink);
                }
                for (int line = 0; line < 3; line++) {
                    const char* trait = traits[role][line];
                    drawText(middle - textWidth(trait) / 2, top + 47 + line * 7, trait, paletteColor('b'));
                }
                drawText(middle - textWidth(ROLE_SPECIALS[role]) / 2, top + 68, ROLE_SPECIALS[role], paletteColor('y'));
                const char* status = !connected ? "RECONNECT" : roleReady[p] ? "READY" : "X READY";
                drawText(middle - textWidth(status) / 2, top + 82, status, ink);
            }
            const char* hint = "LEFT / RIGHT CHOOSE   O UNREADY / BACK";
            drawText((SCREEN_W - textWidth(hint)) / 2, 152, hint, paletteColor('b'));
            const char* special = "R2 USES YOUR SPECIAL IN PLAY";
            drawText((SCREEN_W - textWidth(special)) / 2, 163, special, paletteColor('s'));
        } else if (scene == SCENE_LEVEL_PICKER) {
            // A wheel: the picked level big in the middle, two levels above and below it smaller, wrapping around.
            drawBackground(game);
            const int panelW = 150, panelH = 120, panelX = (SCREEN_W - panelW) / 2, panelY = (SCREEN_H - panelH) / 2;
            const int rowH = 14, middleY = panelY + 58;   // middleY: centre of the picked row
            fillRect(panelX, panelY, panelW, panelH, paletteColor('n'));
            fillRect(panelX + 1, panelY + 1, panelW - 2, panelH - 2, paletteColor('k'));
            drawText((SCREEN_W - textWidth("LEVEL", 2)) / 2, panelY + 8, "LEVEL", 0xFFFFFFFF, 2);
            for (int offset = -3; offset <= 3; offset++) {
                float row = offset + pickerSpin;   // rows from the middle, while the wheel turns
                if (row <= -2.5f || row > 2.5f) continue;   // 5 rows, so a level never shows twice
                const char* name = PICKER_NAMES[(selected + offset + 2 * LEVEL_COUNT) % LEVEL_COUNT];
                float distance = row < 0 ? -row : row;
                int scale = distance < 0.5f ? 2 : 1;
                uint32_t color = distance < 0.5f ? paletteColor('y') : distance < 1.5f ? 0xFFFFFFFF : paletteColor('s');
                int y = middleY + (int)(row * rowH) - 5 * scale / 2;
                drawText((SCREEN_W - textWidth(name, scale)) / 2, y, name, color, scale);
            }
            drawText((SCREEN_W - textWidth("X PLAY  O BACK")) / 2, panelY + panelH - 9, "X PLAY  O BACK", paletteColor('s'));
        } else if (scene == SCENE_SETTINGS) {
            if (settingsBack == SCENE_PAUSED && questPaused) questDraw(quest);
            else if (settingsBack == SCENE_PAUSED) drawGame(game);
            else drawBackground(game);
            char shake[24];
            snprintf(shake, sizeof shake, "SHAKE: %d%%", settings.shake);
            const char* settingsItems[] = {MUSIC_LABELS[settings.music], shake,
                                           settings.reduceFlashing ? "FLASHING: REDUCED" : "FLASHING: NORMAL", "BACK"};
            drawMenu("SETTINGS", settingsItems, 4, selected, "X CHANGE  O BACK");
        } else if (scene == SCENE_QUEST) {
            questDraw(quest);
        } else {
            if (questPaused) questDraw(quest);
            else drawGame(game);
            if (scene == SCENE_PAUSED) {
                char hint[32] = "X SELECT  O RESUME";
                if (lostController >= 0) snprintf(hint, sizeof hint, "CONNECT CONTROLLER %d", lostController + 1);
                drawMenu("PAUSED", PAUSE_ITEMS, 3, selected, hint);
            }
        }

        if (DEV_AIDS && menu.screenshot) {
            stage = "screenshot";
            std::string shotPath = exeFolder + "/shot.bmp";
            debugNetPrintf(INFO, "shot: %s %s\n", shotPath.c_str(), screenSaveBmp(shotPath.c_str()) ? "saved" : "SAVE FAILED");
        }

        stage = "present";
        bool visible = false;   // an all-black picture counts towards the watchdog's black-screen exit
        for (int i = 0; i < screenW * screenH && !visible; i++) visible = (screenTarget[i] & 0xFFFFFF) != 0;
        SDL_UpdateTexture(shown, NULL, screenTarget, screenW * sizeof(uint32_t));
        static int bordersToClear = 2;   // both screen buffers, at the start and whenever the picture size changes
        if (screenTarget != previousTarget) bordersToClear = 2;
        if (bordersToClear > 0) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            bordersToClear--;
        }
        SDL_RenderCopy(renderer, shown, NULL, destination);
        float workMs = (float)((SDL_GetPerformanceCounter() - frameStart) * 1000.0 / counterHz);
        SDL_RenderPresent(renderer);
        if (visible) visibleFrames++;

        // Every 5 s: frames per second and the worst work time (the 60 fps budget is 16.7 ms).
        framesSinceStats++;
        worstWorkMs = SDL_max(worstWorkMs, workMs);
        statsTimer += (float)((SDL_GetPerformanceCounter() - frameStart) / counterHz);
        if (statsTimer >= 5) {
            if (scene == SCENE_QUEST || questPaused)
                debugNetPrintf(INFO, "fps %.1f, worst work %.1f ms, quest %d.%d, heap %d KB, draw bg %.1f tiles %.1f light %.1f ms\n",
                               framesSinceStats / statsTimer, worstWorkMs, quest.area.world, quest.area.index,
                               (int)(mallinfo().uordblks / 1024), questDrawMs[0], questDrawMs[1], questDrawMs[2]);
            else
                debugNetPrintf(INFO, "fps %.1f, worst work %.1f ms, wave %d, score %d, weapon %d, heap %d KB\n", framesSinceStats / statsTimer,
                               worstWorkMs, game.wave, game.players[0].score, game.players[0].weaponLevel, (int)(mallinfo().uordblks / 1024));
            questDrawMs[0] = questDrawMs[1] = questDrawMs[2] = 0;
            framesSinceStats = 0;
            statsTimer = 0;
            worstWorkMs = 0;
        }
        frame++;
    }

    // Quitting (the PS button menu's Quit Game) autosaves a NAME QUEST slot in play, also from its pause menu (spec §17).
    if ((scene == SCENE_QUEST || questPaused) && questCanPause(quest)) questSave(quest);
    for (const std::string& line : quest.logLines) debugNetPrintf(INFO, "%s\n", line.c_str());
    debugNetPrintf(INFO, "nameGame exiting\n");
#ifndef NAMEGAME_MAC
    inputClose();
#endif
    audioClose();
    SDL_Quit();
    debugNetFinish();
    return 0;
}
