// Controller → named actions per player. Gameplay never reads raw button numbers.
#pragma once
#include "game.h"
#include <math.h>

const float STICK_DEADZONE = 0.2f;   // ship-controls/design.md
const int CONTROLLER_PORTS = 7;

// Radial deadzone, rescaled so movement ramps up from 0 at its edge; the result's length is at most 1.
inline void applyDeadzone(float& x, float& y, float dead) {
    float length = sqrtf(x * x + y * y);
    if (length <= dead) {
        x = y = 0;
        return;
    }
    float scaled = fminf((length - dead) / (1 - dead), 1.0f);
    x = x / length * scaled;
    y = y / length * scaled;
}

#ifdef NAMEGAME_MAC
union SDL_Event;
void inputHandleEvent(const SDL_Event& event);   // keyboard events on the Mac
#else
bool inputOpen();
void inputClose();
#endif
void inputRead(Actions* actions);                // one snapshot of all controller ports this frame

void inputRumblePulse(int port, float strength, float seconds);
void inputRumbleUpdate(unsigned activePorts, float shake, float dt);
