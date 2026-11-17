// Controller → named actions per player. Gameplay never reads raw button numbers.
#pragma once
#include "game.h"
#include <math.h>

const float STICK_DEADZONE = 0.2f;   // ship-controls/design.md

union SDL_Event;

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

void inputHandleEvent(const SDL_Event& event);   // call for every SDL event
Actions inputRead(int player);                   // held state now, plus presses since the last read
