#pragma once
#include <vector>

struct Slot { float x, y; };

// Draws a traced shape with `count` slots (rows of 8, top row first, left to right); false if it can't be drawn clearly.
bool traceFormation(int shape, const char* word, int count, std::vector<Slot>& slots);
