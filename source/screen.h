// The 320x180 picture: drawing into a CPU pixel buffer that main.cpp copies to the TV scaled (GDD D-008).
#pragma once
#include <stdint.h>
#include "sprites.h"

const int SCREEN_W = 320, SCREEN_H = 180;

extern uint32_t screenPixels[SCREEN_W * SCREEN_H];   // ARGB, row by row

void fillRect(int x, int y, int w, int h, uint32_t color);
void fillCircle(int cx, int cy, int radius, uint32_t color);
void drawSprite(const Sprite& sprite, int x, int y);
void drawSpriteSolid(const Sprite& sprite, int x, int y, uint32_t color);   // silhouette, for hit flashes
void drawText(int x, int y, const char* text, uint32_t color, int scale = 1);
int textWidth(const char* text, int scale = 1);
