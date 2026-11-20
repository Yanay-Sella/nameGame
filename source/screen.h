// CPU pixel buffers that main.cpp copies to the TV scaled (GDD D-008): the shooter's 320x180 and NAME QUEST's 640x360.
#pragma once
#include <stdint.h>
#include "sprites.h"

const int SCREEN_W = 320, SCREEN_H = 180;
const int QUEST_W = 640, QUEST_H = 360;

extern uint32_t screenPixels[SCREEN_W * SCREEN_H];   // ARGB, row by row: the shooter's picture and the default target
extern uint32_t questPixels[QUEST_W * QUEST_H];      // NAME QUEST's picture (spec "Resolution", D-053)
// Every drawing function below clips to and writes into the current target.
extern uint32_t* screenTarget;
extern int screenW, screenH;
void screenSetTarget(uint32_t* pixels, int w, int h);

struct WorldPalette { uint32_t c[16]; };   // a world's 16 colours: four ramps of four (spec §22)

void fillRect(int x, int y, int w, int h, uint32_t color);
void fillCircle(int cx, int cy, int radius, uint32_t color);
void drawSprite(const Sprite& sprite, int x, int y);
// Like drawSprite, mirrored when flip, with every swapFrom pixel drawn as swapTo (swapFrom 0: no swap).
void drawSpriteEx(const Sprite& sprite, int x, int y, bool flip, char swapFrom = 0, uint32_t swapTo = 0);
// Like drawSpriteEx, but the sprite's '0'-'9' 'a'-'f' characters index pal ('.' transparent).
void drawSpritePal(const Sprite& sprite, int x, int y, bool flip, const WorldPalette& pal);
// drawSpriteEx plus a 1 px outline in `outline` around every opaque pixel (characters, spec §22).
void drawSpriteOutlined(const Sprite& sprite, int x, int y, bool flip, uint32_t outline, char swapFrom = 0, uint32_t swapTo = 0);
void drawSpriteSolid(const Sprite& sprite, int x, int y, uint32_t color);   // silhouette, for hit flashes
void drawText(int x, int y, const char* text, uint32_t color, int scale = 1);
int textWidth(const char* text, int scale = 1);
void drawMenu(const char* title, const char* const* items, int count, int selected, const char* hint);   // centred panel
bool screenSaveBmp(const char* path);   // writes the target as a 24-bit BMP; false when the file couldn't be written

// Ordered 4x4 Bayer threshold for (x, y), 0..15: dithered gradients, glows and fog.
inline int bayer4(int x, int y) { static const int M[16] = {0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5}; return M[(y & 3) * 4 + (x & 3)]; }
