// Throwaway grey-box prototype (see BRIEF.md): ship feel, 320x180 scaling, music and frame rate on the PS3.
// Build: make deploy SOURCES=prototypes/ship-feel
#include <SDL.h>
#include <SDL_mixer.h>
#include <debugnet.h>
#include <net/net.h>
#include <sys/thread.h>
#include <sys/systime.h>
#include <lv2/process.h>
#include <rsx/gcm_sys.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <string>
#include <vector>

// ---- Tuning: change a value, rebuild, play again ----
const float SHIP_SPEED = 150;          // px/s
const float DEADZONE = 0.2f;           // stick length
const float FIRE_INTERVAL = 0.12f;     // s between bullets while R1 is held
const float BULLET_SPEED = 320;        // px/s
const float ROCKET_START_SPEED = 60;   // px/s
const float ROCKET_TOP_SPEED = 260;    // px/s
const float ROCKET_ACCEL = 500;        // px/s^2
const float ROCKET_COOLDOWN = 0.5f;    // s
const float ROCKET_BUFFER = 0.15f;     // s an early R2 press is remembered
const float EXPLOSION_RADIUS = 40;     // px
const float EXPLOSION_TIME = 0.35f;    // s the blast stays on screen
const float SWAY_SPEED = 20;           // px/s
const float ENEMY_SHOT_CHANCE = 0.03f; // per alien per second
const float ENEMY_SHOT_SPEED = 70;     // px/s
const float MOVE_ZONE_TOP = 100;       // px; the ship stays below this line

const int W = 320, H = 180;            // logical screen, scaled up by a whole number

// Button indices from SDL_PSL1GHT's joystick driver (src/joystick/psl1ght/SDL_sysjoystick.c).
enum { BTN_LEFT = 0, BTN_DOWN = 1, BTN_RIGHT = 2, BTN_UP = 3, BTN_START = 4, BTN_R3 = 5, BTN_L3 = 6, BTN_SELECT = 7,
       BTN_SQUARE = 8, BTN_CROSS = 9, BTN_CIRCLE = 10, BTN_TRIANGLE = 11, BTN_R1 = 12, BTN_L1 = 13, BTN_R2 = 14, BTN_L2 = 15 };
static const char* BUTTON_NAMES[16] = {"left", "down", "right", "up", "start", "R3", "L3", "select",
                                       "square", "cross", "circle", "triangle", "R1", "L1", "R2", "L2"};

struct Shot { float x, y, vx, vy; };
struct Enemy { float x, y; bool alive; };
struct Rocket { float x, y, speed; };
struct Explosion { float x, y, age; };

static Uint32 pixels[W * H];   // the 320x180 picture, drawn by the CPU, copied scaled to the screen each frame

static void fillRect(int x, int y, int w, int h, Uint32 color) {
    int x0 = x < 0 ? 0 : x, y0 = y < 0 ? 0 : y;
    int x1 = x + w > W ? W : x + w, y1 = y + h > H ? H : y + h;
    for (int py = y0; py < y1; py++)
        for (int px = x0; px < x1; px++) pixels[py * W + px] = color;
}

// 3x5 font: each glyph is 5 octal digits, one per row, top row first; bit 4 of a row is its left pixel.
static int glyph(char c) {
    switch (c) {
        case '0': case 'O': return 075557; case '1': return 026227; case '2': return 071747; case '3': return 071717;
        case '4': return 055711; case '5': case 'S': return 074717; case '6': return 074757; case '7': return 071122;
        case '8': return 075757; case '9': return 075717; case 'A': return 025755; case 'B': return 065656;
        case 'D': return 065556; case 'E': return 074647; case 'F': return 074644; case 'H': return 055755;
        case 'I': return 072227; case 'J': return 011157; case 'K': return 055655; case 'L': return 044447;
        case 'M': return 057755; case 'N': return 065555; case 'P': return 075744; case 'R': return 065655;
        case 'T': return 072222; case 'U': return 055557; case 'W': return 055775; case 'X': return 055255;
        case '.': return 000002; case ':': return 002020; default: return 0;
    }
}

static void drawText(int x, int y, const char* text, Uint32 color) {
    for (; *text; text++, x += 4) {
        int g = glyph(*text);
        for (int row = 0; row < 5; row++)
            for (int col = 0; col < 3; col++)
                if ((g >> (3 * (4 - row) + (2 - col))) & 1) fillRect(x + col, y + row, 1, 1, color);
    }
}

static bool overlap(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh) {
    return ax < bx + bw && bx < ax + aw && ay < by + bh && by < ay + ah;
}

static float randomUnit() { return rand() / (float)RAND_MAX; }

static std::vector<Enemy> makeFormation() {
    std::vector<Enemy> enemies;
    for (int row = 0; row < 5; row++)
        for (int col = 0; col < 8; col++) enemies.push_back({90.0f + col * 18, 14.0f + row * 14, true});
    return enemies;
}

// A short square-wave beep in the mixer's output format; NULL if the format isn't one we write.
static Mix_Chunk* makeBeep(float startHz, float endHz, float seconds) {
    int freq, channels;
    Uint16 format;
    if (!Mix_QuerySpec(&freq, &format, &channels)) return NULL;
    int frames = (int)(freq * seconds);
    int sampleBytes = (format == AUDIO_F32SYS) ? 4 : (format == AUDIO_S16SYS) ? 2 : 0;
    if (sampleBytes == 0) {
        debugNetPrintf(ERROR, "beep: unsupported mixer format 0x%x\n", format);
        return NULL;
    }
    Uint8* data = (Uint8*)SDL_malloc(frames * channels * sampleBytes);
    float phase = 0;
    for (int i = 0; i < frames; i++) {
        float t = i / (float)frames;
        phase += (startHz + (endHz - startHz) * t) / freq;
        float value = (fmodf(phase, 1.0f) < 0.5f ? 0.25f : -0.25f) * (1 - t);
        for (int ch = 0; ch < channels; ch++) {
            int index = i * channels + ch;
            if (sampleBytes == 4) ((float*)data)[index] = value;
            else ((Sint16*)data)[index] = (Sint16)(value * 32767);
        }
    }
    return Mix_QuickLoad_RAW(data, frames * channels * sampleBytes);
}

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
    debugNetPrintf(INFO, "ship-feel prototype started, argv[0]=%s\n", argc > 0 ? argv[0] : "(none)");

    sys_ppu_thread_t watchdogThread;
    sysThreadCreate(&watchdogThread, watchdog, NULL, 1000, 0x4000, 0, (char*)"watchdog");

    stage = "SDL_Init";
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) != 0) {
        debugNetPrintf(ERROR, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_DisplayMode desktop;
    SDL_GetDesktopDisplayMode(0, &desktop);
    SDL_Window* window = SDL_CreateWindow("nameGame", 0, 0, desktop.w, desktop.h, 0);
    SDL_Renderer* renderer = window ? SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC) : NULL;
    SDL_Texture* screenTexture = renderer ? SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, W, H) : NULL;
    if (!screenTexture) {
        debugNetPrintf(ERROR, "window/renderer/texture failed: %s\n", SDL_GetError());
        return 1;
    }
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer, &screenW, &screenH);
    int scale = SDL_min(screenW / W, screenH / H);
    SDL_Rect screenRect = {(screenW - W * scale) / 2, (screenH - H * scale) / 2, W * scale, H * scale};
    debugNetPrintf(INFO, "screen %dx%d, scale x%d, picture at %d,%d\n", screenW, screenH, scale, screenRect.x, screenRect.y);

    stage = "audio";
    Mix_Music* music = NULL;
    Mix_Chunk* rocketBeep = NULL;
    if (Mix_OpenAudio(48000, AUDIO_S16SYS, 2, 1024) != 0) {
        debugNetPrintf(ERROR, "Mix_OpenAudio failed: %s\n", Mix_GetError());
    } else {
        int freq, channels;
        Uint16 format;
        Mix_QuerySpec(&freq, &format, &channels);
        debugNetPrintf(INFO, "audio %d Hz, format 0x%x, %d channels\n", freq, format, channels);
        // The song is packaged next to EBOOT.BIN (pkgfiles/USRDIR/music.ogg).
        std::string musicPath = argc > 0 ? argv[0] : "/dev_hdd0/game/NMGM00001/USRDIR/EBOOT.BIN";
        musicPath = musicPath.substr(0, musicPath.find_last_of('/') + 1) + "music.ogg";
        music = Mix_LoadMUS(musicPath.c_str());
        if (!music) debugNetPrintf(ERROR, "Mix_LoadMUS(%s) failed: %s\n", musicPath.c_str(), Mix_GetError());
        else if (Mix_PlayMusic(music, -1) != 0) debugNetPrintf(ERROR, "Mix_PlayMusic failed: %s\n", Mix_GetError());
        else debugNetPrintf(INFO, "music playing: %s\n", musicPath.c_str());
        rocketBeep = makeBeep(900, 200, 0.25f);
    }

    SDL_Joystick* joystick = SDL_NumJoysticks() > 0 ? SDL_JoystickOpen(0) : NULL;
    debugNetPrintf(INFO, "%d controller(s)\n", SDL_NumJoysticks());

    float shipX = W / 2 - 8, shipY = H - 24;
    float fireTimer = 0, rocketCooldown = 0, rocketPressedAt = -1, hitFlash = 0, swayOffset = 0, swayDir = 1;
    float gameTime = 0;
    int hits = 0;
    bool paused = false;
    std::vector<Shot> bullets, enemyShots;
    std::vector<Rocket> rockets;
    std::vector<Explosion> explosions;
    std::vector<Enemy> enemies = makeFormation();

    Uint64 lastCounter = SDL_GetPerformanceCounter();
    const double counterHz = (double)SDL_GetPerformanceFrequency();
    int framesThisSecond = 0, fps = 0;
    float secondTimer = 0, workMs = 0, worstWorkMs = 0, shownWorstMs = 0;

    while (running) {
        Uint64 frameStart = SDL_GetPerformanceCounter();
        float dt = (float)((frameStart - lastCounter) / counterHz);
        lastCounter = frameStart;
        if (dt > 0.05f) dt = 0.05f;

        // SDL also turns the PS button menu's "Quit Game" into SDL_QUIT.
        stage = "events";
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_JOYDEVICEADDED && !joystick) joystick = SDL_JoystickOpen(event.jdevice.which);
            if (event.type == SDL_JOYBUTTONDOWN) {
                int b = event.jbutton.button;
                debugNetPrintf(INFO, "button %d (%s)\n", b, b < 16 ? BUTTON_NAMES[b] : "?");
                if (b == BTN_START) {
                    paused = !paused;
                    rocketPressedAt = -1;
                }
                if (b == BTN_R2 && !paused) rocketPressedAt = gameTime;
                if (b == BTN_TRIANGLE && !paused)   // stress test: 100 slow bullets
                    for (int i = 0; i < 100; i++) bullets.push_back({randomUnit() * W, H - randomUnit() * 60, 0, -60});
            }
        }

        stage = "update";
        if (!paused) {
            gameTime += dt;

            // Movement: left stick with a radial deadzone, or the D-pad at full speed.
            float mx = 0, my = 0;
            if (joystick) {
                float sx = SDL_JoystickGetAxis(joystick, 0) / 32768.0f, sy = SDL_JoystickGetAxis(joystick, 1) / 32768.0f;
                float length = sqrtf(sx * sx + sy * sy);
                if (length > DEADZONE) {
                    float scaled = SDL_min((length - DEADZONE) / (1 - DEADZONE), 1.0f);
                    mx = sx / length * scaled;
                    my = sy / length * scaled;
                }
                float dx = SDL_JoystickGetButton(joystick, BTN_RIGHT) - SDL_JoystickGetButton(joystick, BTN_LEFT);
                float dy = SDL_JoystickGetButton(joystick, BTN_DOWN) - SDL_JoystickGetButton(joystick, BTN_UP);
                if (dx || dy) {
                    float length = sqrtf(dx * dx + dy * dy);
                    mx = dx / length;
                    my = dy / length;
                }
            }
            shipX = SDL_max(SDL_min(shipX + mx * SHIP_SPEED * dt, W - 16.0f), 0.0f);
            shipY = SDL_max(SDL_min(shipY + my * SHIP_SPEED * dt, H - 16.0f), MOVE_ZONE_TOP);

            fireTimer = SDL_max(fireTimer - dt, 0.0f);
            if (joystick && SDL_JoystickGetButton(joystick, BTN_R1) && fireTimer == 0) {
                bullets.push_back({shipX + 7, shipY - 4, 0, -BULLET_SPEED});
                fireTimer = FIRE_INTERVAL;
            }

            rocketCooldown = SDL_max(rocketCooldown - dt, 0.0f);
            if (rocketPressedAt >= 0 && gameTime - rocketPressedAt > ROCKET_BUFFER) rocketPressedAt = -1;
            if (rocketPressedAt >= 0 && rocketCooldown == 0) {
                rockets.push_back({shipX + 6, shipY - 8, ROCKET_START_SPEED});
                rocketCooldown = ROCKET_COOLDOWN;
                rocketPressedAt = -1;
                if (rocketBeep) Mix_PlayChannel(-1, rocketBeep, 0);
            }

            swayOffset += swayDir * SWAY_SPEED * dt;
            if (swayOffset > 50 || swayOffset < -50) swayDir = -swayDir;

            for (Shot& b : bullets) b.y += b.vy * dt;
            for (Shot& s : enemyShots) s.y += s.vy * dt;

            for (Enemy& e : enemies) {
                if (!e.alive) continue;
                float ex = e.x + swayOffset;
                if (randomUnit() < ENEMY_SHOT_CHANCE * dt) enemyShots.push_back({ex + 5, e.y + 12, 0, ENEMY_SHOT_SPEED});
                for (Shot& b : bullets) {
                    if (b.y > -10 && overlap(b.x, b.y, 2, 4, ex, e.y, 12, 12)) {
                        e.alive = false;
                        b.y = -100;   // spent; removed below
                        break;
                    }
                }
            }

            for (Rocket& r : rockets) {
                r.speed = SDL_min(r.speed + ROCKET_ACCEL * dt, ROCKET_TOP_SPEED);
                r.y -= r.speed * dt;
                bool hit = r.y < 0;
                for (Enemy& e : enemies)
                    if (e.alive && overlap(r.x, r.y, 4, 8, e.x + swayOffset, e.y, 12, 12)) hit = true;
                if (hit) {
                    explosions.push_back({r.x + 2, r.y + 4, 0});
                    for (Enemy& e : enemies) {
                        float ddx = e.x + swayOffset + 6 - (r.x + 2), ddy = e.y + 6 - (r.y + 4);
                        if (e.alive && ddx * ddx + ddy * ddy <= (EXPLOSION_RADIUS + 6) * (EXPLOSION_RADIUS + 6)) e.alive = false;
                    }
                    r.y = -1000;   // spent; removed below
                }
            }
            for (Explosion& x : explosions) x.age += dt;

            hitFlash = SDL_max(hitFlash - dt, 0.0f);
            for (Shot& s : enemyShots) {
                if (hitFlash == 0 && overlap(s.x, s.y, 2, 4, shipX + 5, shipY + 5, 6, 6)) {
                    hits++;
                    hitFlash = 1;
                    s.y = H + 100;
                    debugNetPrintf(INFO, "ship hit (%d)\n", hits);
                }
            }

            bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Shot& b) { return b.y < -8; }), bullets.end());
            enemyShots.erase(std::remove_if(enemyShots.begin(), enemyShots.end(), [](const Shot& s) { return s.y > H; }), enemyShots.end());
            rockets.erase(std::remove_if(rockets.begin(), rockets.end(), [](const Rocket& r) { return r.y < -100; }), rockets.end());
            explosions.erase(std::remove_if(explosions.begin(), explosions.end(), [](const Explosion& x) { return x.age > EXPLOSION_TIME; }), explosions.end());
            bool anyAlive = false;
            for (Enemy& e : enemies) anyAlive |= e.alive;
            if (!anyAlive) enemies = makeFormation();
        }

        stage = "draw";
        fillRect(0, 0, W, H, 0xFF101018);
        fillRect(0, (int)MOVE_ZONE_TOP - 1, W, 1, 0xFF202030);   // top of the movement zone
        for (Enemy& e : enemies)
            if (e.alive) fillRect((int)(e.x + swayOffset), (int)e.y, 12, 12, 0xFF50C060);
        for (Shot& b : bullets) fillRect((int)b.x, (int)b.y, 2, 4, 0xFFFFF080);
        for (Shot& s : enemyShots) fillRect((int)s.x, (int)s.y, 2, 4, 0xFFFF40C0);
        for (Rocket& r : rockets) fillRect((int)r.x, (int)r.y, 4, 8, 0xFFFFA020);
        for (Explosion& x : explosions) {
            float radius = EXPLOSION_RADIUS * SDL_min(x.age / 0.1f, 1.0f);
            for (int py = (int)(x.y - radius); py <= (int)(x.y + radius); py++)
                for (int px = (int)(x.x - radius); px <= (int)(x.x + radius); px++)
                    if ((px - x.x) * (px - x.x) + (py - x.y) * (py - x.y) <= radius * radius) fillRect(px, py, 1, 1, 0xFFFF8020);
        }
        bool shipVisible = hitFlash == 0 || fmodf(hitFlash, 0.2f) < 0.1f;
        if (shipVisible) fillRect((int)shipX, (int)shipY, 16, 16, hitFlash > 0 ? 0xFFFF4040 : 0xFFC0C0D0);

        int objects = (int)(bullets.size() + enemyShots.size() + rockets.size() + explosions.size() + enemies.size()) + 1;
        char line[64];
        snprintf(line, sizeof line, "FPS %d  WORK %.1fMS  MAX %.1fMS  OBJ %d", fps, workMs, shownWorstMs, objects);
        drawText(2, 2, line, 0xFFFFFFFF);
        snprintf(line, sizeof line, "SPD %d  INT %.2f  HITS %d", (int)SHIP_SPEED, FIRE_INTERVAL, hits);
        drawText(2, 9, line, 0xFFFFFFFF);
        if (paused) drawText(W / 2 - 12, H / 2, "PAUSED", 0xFFFFFFFF);

        stage = "present";
        SDL_UpdateTexture(screenTexture, NULL, pixels, W * sizeof(Uint32));
        if (frame < 2) {   // letterbox borders are never drawn over, so clearing both screen buffers once is enough
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
        }
        if (SDL_RenderCopy(renderer, screenTexture, NULL, &screenRect) != 0 && frame == 0)
            debugNetPrintf(ERROR, "SDL_RenderCopy failed: %s\n", SDL_GetError());
        float thisWorkMs = (float)((SDL_GetPerformanceCounter() - frameStart) * 1000.0 / counterHz);
        SDL_RenderPresent(renderer);

        // Frame stats: "work" is the time before waiting for vsync, so 16.7 ms is the budget at 60 fps.
        framesThisSecond++;
        worstWorkMs = SDL_max(worstWorkMs, thisWorkMs);
        secondTimer += dt;
        if (secondTimer >= 1) {
            fps = framesThisSecond;
            workMs = thisWorkMs;
            shownWorstMs = worstWorkMs;
            debugNetPrintf(INFO, "fps %d work %.1f ms (max %.1f) objects %d\n", fps, workMs, worstWorkMs, objects);
            framesThisSecond = 0;
            secondTimer = 0;
            worstWorkMs = 0;
        }
        frame++;
    }

    debugNetPrintf(INFO, "ship-feel exiting\n");
    Mix_CloseAudio();
    SDL_Quit();
    debugNetFinish();
    return 0;
}
