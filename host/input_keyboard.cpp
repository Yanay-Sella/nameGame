// Mac build: two keyboard pads and a NAMEGAME_PAD script that can drive any port.
// Player 1: arrows, Z cross (jump), X circle (dodge), C square (potion), V triangle (item), Enter start, Backspace select, Space R1 (fire), Right Shift R2 (rocket),
//           P L1 (screenshot), 1 L2, 2 L3 (1 + 2 together: test mode).
// Player 2: WASD, K cross (joins the run), L square, H R1 (fire), J R2 (rocket).
#include "../source/input.h"
#include <SDL.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

enum { BTN_LEFT, BTN_DOWN, BTN_RIGHT, BTN_UP, BTN_START, BTN_L3, BTN_SELECT, BTN_SQUARE, BTN_CROSS, BTN_CIRCLE, BTN_TRIANGLE,
       BTN_R1, BTN_L1, BTN_R2, BTN_L2, BUTTON_COUNT };
const char* const BUTTON_NAMES[BUTTON_COUNT] = {"left", "down", "right", "up", "start", "l3", "select", "square", "cross", "circle",
                                                "triangle", "r1", "l1", "r2", "l2"};

struct KeyBinding { SDL_Scancode key; int player, button; };
const KeyBinding KEYS[] = {
    {SDL_SCANCODE_LEFT, 0, BTN_LEFT}, {SDL_SCANCODE_DOWN, 0, BTN_DOWN}, {SDL_SCANCODE_RIGHT, 0, BTN_RIGHT}, {SDL_SCANCODE_UP, 0, BTN_UP},
    {SDL_SCANCODE_Z, 0, BTN_CROSS}, {SDL_SCANCODE_X, 0, BTN_CIRCLE}, {SDL_SCANCODE_C, 0, BTN_SQUARE}, {SDL_SCANCODE_V, 0, BTN_TRIANGLE},
    {SDL_SCANCODE_RETURN, 0, BTN_START}, {SDL_SCANCODE_BACKSPACE, 0, BTN_SELECT}, {SDL_SCANCODE_SPACE, 0, BTN_R1},
    {SDL_SCANCODE_RSHIFT, 0, BTN_R2}, {SDL_SCANCODE_P, 0, BTN_L1}, {SDL_SCANCODE_1, 0, BTN_L2}, {SDL_SCANCODE_2, 0, BTN_L3},
    {SDL_SCANCODE_A, 1, BTN_LEFT}, {SDL_SCANCODE_S, 1, BTN_DOWN}, {SDL_SCANCODE_D, 1, BTN_RIGHT}, {SDL_SCANCODE_W, 1, BTN_UP},
    {SDL_SCANCODE_K, 1, BTN_CROSS}, {SDL_SCANCODE_L, 1, BTN_SQUARE}, {SDL_SCANCODE_H, 1, BTN_R1}, {SDL_SCANCODE_J, 1, BTN_R2},
};

static bool connected[CONTROLLER_PORTS] = {true, true};
static bool held[CONTROLLER_PORTS][BUTTON_COUNT];
static bool pressed[CONTROLLER_PORTS][BUTTON_COUNT];
static bool scriptHeld[BUTTON_COUNT];
static int scriptPort;

enum ScriptAction { SCRIPT_BUTTONS, SCRIPT_QUIT, SCRIPT_CONNECT, SCRIPT_DISCONNECT };
struct ScriptStep { double seconds; unsigned buttons; int port; ScriptAction action; };
static std::vector<ScriptStep> script;
static size_t scriptStep = 0;
static bool scriptStepStarted = false;
static Uint32 scriptStepEnds = 0;

// A pN. prefix targets a port; pN.connect and pN.disconnect simulate hot-plug for tests.
static void loadScript() {
    const char* text = getenv("NAMEGAME_PAD");
    if (!text) return;
    std::string rest = text;
    while (!rest.empty()) {
        size_t bar = rest.find('|');
        std::string token = rest.substr(0, bar);
        rest = bar == std::string::npos ? "" : rest.substr(bar + 1);
        if (token.empty()) continue;
        if (token == "quit") { script.push_back({0, 0, 0, SCRIPT_QUIT}); continue; }
        if (isdigit((unsigned char)token[0])) { script.push_back({atof(token.c_str()), 0, 0, SCRIPT_BUTTONS}); continue; }
        int port = 0;
        if (token.size() > 3 && token[0] == 'p' && token[2] == '.' && token[1] >= '1' && token[1] < '1' + CONTROLLER_PORTS) {
            port = token[1] - '1';
            token = token.substr(3);
        }
        if (token == "connect" || token == "disconnect") {
            script.push_back({0, 0, port, token == "connect" ? SCRIPT_CONNECT : SCRIPT_DISCONNECT});
            continue;
        }
        double hold = 0.1;
        size_t colon = token.find(':');
        if (colon != std::string::npos) { hold = atof(token.c_str() + colon + 1); token = token.substr(0, colon); }
        unsigned buttons = 0;
        for (size_t start = 0; start <= token.size();) {
            size_t plus = token.find('+', start);
            std::string name = token.substr(start, plus == std::string::npos ? std::string::npos : plus - start);
            start = plus == std::string::npos ? token.size() + 1 : plus + 1;
            for (int b = 0; b < BUTTON_COUNT; b++) if (name == BUTTON_NAMES[b]) buttons |= 1u << b;
        }
        if (buttons) script.push_back({hold, buttons, port, SCRIPT_BUTTONS});
        else SDL_Log("NAMEGAME_PAD: unknown button '%s'", token.c_str());
    }
}

// Runs the script one frame further: starts the next step when the last one ended.
static void advanceScript() {
    static bool loaded = false;
    if (!loaded) { loaded = true; loadScript(); }
    if (scriptStep >= script.size()) return;
    const ScriptStep& step = script[scriptStep];
    if (!scriptStepStarted) {
        scriptStepStarted = true;
        scriptStepEnds = SDL_GetTicks() + (Uint32)(step.seconds * 1000);
        if (step.action == SCRIPT_QUIT) { SDL_Event quit = {}; quit.type = SDL_QUIT; SDL_PushEvent(&quit); }
        if (step.action == SCRIPT_CONNECT || step.action == SCRIPT_DISCONNECT) connected[step.port] = step.action == SCRIPT_CONNECT;
        scriptPort = step.port;
        for (int b = 0; b < BUTTON_COUNT; b++)
            if (step.buttons & (1u << b)) { scriptHeld[b] = true; pressed[scriptPort][b] = true; }
    }
    if ((int)(SDL_GetTicks() - scriptStepEnds) >= 0) {
        memset(scriptHeld, 0, sizeof scriptHeld);
        scriptStep++;
        scriptStepStarted = false;
    }
}

void inputHandleEvent(const SDL_Event& event) {
    if ((event.type != SDL_KEYDOWN && event.type != SDL_KEYUP) || event.key.repeat) return;
    for (const KeyBinding& k : KEYS) {
        if (k.key != event.key.keysym.scancode) continue;
        held[k.player][k.button] = event.type == SDL_KEYDOWN;
        if (event.type == SDL_KEYDOWN) pressed[k.player][k.button] = true;
    }
}

void inputRead(Actions* actions) {
    advanceScript();
    for (int port = 0; port < CONTROLLER_PORTS; port++) {
        Actions& a = actions[port];
        a = {};
        a.connected = connected[port];
        if (a.connected) {
            auto down = [&](int button) { return held[port][button] || (port == scriptPort && scriptHeld[button]); };
            const bool* press = pressed[port];
            float dx = down(BTN_RIGHT) - down(BTN_LEFT), dy = down(BTN_DOWN) - down(BTN_UP);
            if (dx || dy) {
                float length = sqrtf(dx * dx + dy * dy);
                a.moveX = dx / length;
                a.moveY = dy / length;
            }
            a.fire = down(BTN_R1);
            a.rocket = press[BTN_R2];
            a.pause = press[BTN_START];
            a.confirm = press[BTN_CROSS] || press[BTN_START];
            a.autopilot = press[BTN_SELECT];
            a.screenshot = press[BTN_L1];
            a.godMode = (press[BTN_L2] || press[BTN_L3]) && down(BTN_L2) && down(BTN_L3);
            a.cycle = press[BTN_SQUARE];
            a.jump = press[BTN_CROSS];
            a.jumpHeld = down(BTN_CROSS);
            a.selectHeld = down(BTN_SELECT);
            a.item = press[BTN_TRIANGLE];
            a.up = press[BTN_UP];
            a.down = press[BTN_DOWN];
            a.left = press[BTN_LEFT];
            a.right = press[BTN_RIGHT];
            a.back = press[BTN_CIRCLE];
        }
        for (bool& b : pressed[port]) b = false;
    }
}

void inputRumblePulse(int, float, float) {}
void inputRumbleUpdate(unsigned, float, float) {}
