// Software drawing into the 320x180 pixel buffer; everything is clipped to the screen.
#include "screen.h"
#include <string.h>

uint32_t screenPixels[SCREEN_W * SCREEN_H];

void fillRect(int x, int y, int w, int h, uint32_t color) {
    int x0 = x < 0 ? 0 : x, y0 = y < 0 ? 0 : y;
    int x1 = x + w > SCREEN_W ? SCREEN_W : x + w, y1 = y + h > SCREEN_H ? SCREEN_H : y + h;
    for (int py = y0; py < y1; py++)
        for (int px = x0; px < x1; px++) screenPixels[py * SCREEN_W + px] = color;
}

void fillCircle(int cx, int cy, int radius, uint32_t color) {
    for (int dy = -radius; dy <= radius; dy++)
        for (int dx = -radius; dx <= radius; dx++)
            if (dx * dx + dy * dy <= radius * radius) fillRect(cx + dx, cy + dy, 1, 1, color);
}

void drawSprite(const Sprite& sprite, int x, int y) {
    for (int row = 0; row < sprite.h; row++) {
        int py = y + row;
        if (py < 0 || py >= SCREEN_H) continue;
        for (int col = 0; col < sprite.w; col++) {
            int px = x + col;
            uint32_t color = paletteColor(sprite.pixels[row * sprite.w + col]);
            if (color && px >= 0 && px < SCREEN_W) screenPixels[py * SCREEN_W + px] = color;
        }
    }
}

void drawSpriteSolid(const Sprite& sprite, int x, int y, uint32_t color) {
    for (int row = 0; row < sprite.h; row++)
        for (int col = 0; col < sprite.w; col++)
            if (paletteColor(sprite.pixels[row * sprite.w + col])) fillRect(x + col, y + row, 1, 1, color);
}

void drawText(int x, int y, const char* text, uint32_t color, int scale) {
    for (; *text; text++, x += 4 * scale) {
        int glyph = fontGlyph(*text);
        for (int row = 0; row < 5; row++)
            for (int col = 0; col < 3; col++)
                if ((glyph >> (3 * (4 - row) + (2 - col))) & 1) fillRect(x + col * scale, y + row * scale, scale, scale, color);
    }
}

int textWidth(const char* text, int scale) {
    int length = (int)strlen(text);
    return length == 0 ? 0 : (length * 4 - 1) * scale;
}
