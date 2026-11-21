#pragma once
#include <vector>

struct Slot { float x, y; };

// Draws a traced shape with `count` slots, ordered by rows; false if it can't be drawn clearly.
bool traceFormation(int shape, const char* word, int count, std::vector<Slot>& slots,
                    int columns = 8, float extraWidth = 0, float extraHeight = 0);
