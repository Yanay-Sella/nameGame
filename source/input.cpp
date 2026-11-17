// Reads the PS3 controllers through SDL's joystick API; player n uses controller n.
#include "input.h"
#include <SDL.h>
#include <debugnet.h>

// Button indices from SDL_PSL1GHT's joystick driver (src/joystick/psl1ght/SDL_sysjoystick.c), see CLAUDE.md.
enum { BTN_LEFT = 0, BTN_DOWN = 1, BTN_RIGHT = 2, BTN_UP = 3, BTN_START = 4, BTN_CROSS = 9, BTN_TRIANGLE = 11, BTN_R1 = 12, BTN_R2 = 14,
       BUTTON_COUNT = 16 };

static SDL_Joystick* joysticks[MAX_PLAYERS];
static bool pressed[MAX_PLAYERS][BUTTON_COUNT];   // set by button-down events, cleared by inputRead

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
    for (bool& b : pressed[player]) b = false;
    return a;
}
