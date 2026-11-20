// The 320x180 picture: drawing into a CPU pixel buffer that main.cpp copies to the TV scaled (GDD D-008).
#pragma once
#include <stdint.h>
#include "sprites.h"

const int SCREEN_W = 320, SCREEN_H = 180;

extern uint32_t screenPixels[SCREEN_W * SCREEN_H];   // ARGB, row by row

void fillRect(int x, int y, int w, int h, uint32_t color);
void fillCircle(int cx, int cy, int radius, uint32_t color);
void drawSprite(const Sprite& sprite, int x, int y);
// Like drawSprite, mirrored when flip, with every swapFrom pixel drawn as swapTo (swapFrom 0: no swap).
void drawSpriteEx(const Sprite& sprite, int x, int y, bool flip, char swapFrom = 0, uint32_t swapTo = 0);
void drawSpriteSolid(const Sprite& sprite, int x, int y, uint32_t color);   // silhouette, for hit flashes
void drawText(int x, int y, const char* text, uint32_t color, int scale = 1);
int textWidth(const char* text, int scale = 1);
void drawMenu(const char* title, const char* const* items, int count, int selected, const char* hint);   // centred panel
bool screenSaveBmp(const char* path);   // writes screenPixels as a 24-bit BMP; false when the file couldn't be written
