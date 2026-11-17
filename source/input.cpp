// Reads the PS3 controllers through SDL's joystick API; player n uses controller n.
#include "input.h"
#include <SDL.h>
#include <debugnet.h>

// Button indices from SDL_PSL1GHT's joystick driver (src/joystick/psl1ght/SDL_sysjoystick.c), see CLAUDE.md.
enum { BTN_LEFT = 0, BTN_DOWN = 1, BTN_RIGHT = 2, BTN_UP = 3, BTN_START = 4, BTN_SELECT = 7, BTN_CROSS = 9, BTN_CIRCLE = 10, BTN_TRIANGLE = 11, BTN_R1 = 12, BTN_R2 = 14,
       BUTTON_COUNT = 16 };

static SDL_Joystick* joysticks[MAX_PLAYERS];
static bool pressed[MAX_PLAYERS][BUTTON_COUNT];   // set by button-down events, cleared by inputRead
static int stickFlick[MAX_PLAYERS][2];   // last flick direction of the left stick per axis: -1, 0 or 1

void inputHandleEvent(const SDL_Event& event) {
    if (event.type == SDL_JOYBUTTONDOWN && event.jbutton.button < BUTTON_COUNT) {
        for (int p = 0; p < MAX_PLAYERS; p++)
            if (joysticks[p] && SDL_JoystickInstanceID(joysticks[p]) == event.jbutton.which) pressed[p][event.jbutton.button] = true;
        debugNetPrintf(DEBUG, "button %d\n", event.jbutton.button);
    }
}

Actions inputRead(int player) {
    // This SDL port sends no controller added/removed events, so controllers are opened and closed by count.
    bool connected = player < SDL_NumJoysticks();
    if (connected && !joysticks[player]) {
        joysticks[player] = SDL_JoystickOpen(player);
        debugNetPrintf(INFO, "controller %d %s\n", player, joysticks[player] ? "connected" : "failed to open");
    } else if (!connected && joysticks[player]) {
        SDL_JoystickClose(joysticks[player]);
        joysticks[player] = NULL;
        debugNetPrintf(INFO, "controller %d disconnected\n", player);
    }

    Actions a = {};
    SDL_Joystick* joystick = joysticks[player];
    if (!joystick) return a;
    a.connected = true;

    // The D-pad wins over the stick and always gives full speed.
    float dx = SDL_JoystickGetButton(joystick, BTN_RIGHT) - SDL_JoystickGetButton(joystick, BTN_LEFT);
    float dy = SDL_JoystickGetButton(joystick, BTN_DOWN) - SDL_JoystickGetButton(joystick, BTN_UP);
    if (dx || dy) {
        float length = sqrtf(dx * dx + dy * dy);
        a.moveX = dx / length;
        a.moveY = dy / length;
    } else {
        a.moveX = SDL_JoystickGetAxis(joystick, 0) / 32768.0f;
        a.moveY = SDL_JoystickGetAxis(joystick, 1) / 32768.0f;
        applyDeadzone(a.moveX, a.moveY, STICK_DEADZONE);
    }
    a.fire = SDL_JoystickGetButton(joystick, BTN_R1);
    a.rocket = pressed[player][BTN_R2];
    a.pause = pressed[player][BTN_START];
    a.confirm = pressed[player][BTN_CROSS] || pressed[player][BTN_START];
    a.stress = pressed[player][BTN_TRIANGLE];
    a.autopilot = pressed[player][BTN_SELECT];
    a.up = pressed[player][BTN_UP];
    a.down = pressed[player][BTN_DOWN];
    a.left = pressed[player][BTN_LEFT];
    a.right = pressed[player][BTN_RIGHT];
    a.back = pressed[player][BTN_CIRCLE];
    // A stick flick counts as one menu press; the hysteresis stops a held stick from repeating.
    for (int axis = 0; axis < 2; axis++) {
        float v = SDL_JoystickGetAxis(joystick, axis) / 32768.0f;
        int dir = v < -0.6f ? -1 : v > 0.6f ? 1 : fabsf(v) < 0.3f ? 0 : stickFlick[player][axis];
        if (dir != 0 && dir != stickFlick[player][axis]) {
            if (axis == 0) (dir < 0 ? a.left : a.right) = true;
            else (dir < 0 ? a.up : a.down) = true;
        }
        stickFlick[player][axis] = dir;
    }
    for (bool& b : pressed[player]) b = false;
    return a;
}
