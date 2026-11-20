// Software drawing into the 320x180 pixel buffer; everything is clipped to the screen.
#include "screen.h"
#include <stdio.h>
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
            char c = sprite.pixels[row * sprite.w + col];
            uint32_t color = paletteColor(c);
            if (!color || px < 0 || px >= SCREEN_W) continue;
            uint32_t& pixel = screenPixels[py * SCREEN_W + px];
            pixel = c == '~' ? 0xFF000000 | (((pixel >> 1) & 0x7F7F7F) + ((pixel >> 2) & 0x3F3F3F) + ((color >> 2) & 0x3F3F3F)) : color;
        }
    }
}

void drawSpriteEx(const Sprite& sprite, int x, int y, bool flip, char swapFrom, uint32_t swapTo) {
    for (int row = 0; row < sprite.h; row++) {
        int py = y + row;
        if (py < 0 || py >= SCREEN_H) continue;
        for (int col = 0; col < sprite.w; col++) {
            int px = x + col;
            char c = sprite.pixels[row * sprite.w + (flip ? sprite.w - 1 - col : col)];
            uint32_t color = swapFrom && c == swapFrom ? swapTo : paletteColor(c);
            if (!color || px < 0 || px >= SCREEN_W) continue;
            uint32_t& pixel = screenPixels[py * SCREEN_W + px];
            pixel = c == '~' ? 0xFF000000 | (((pixel >> 1) & 0x7F7F7F) + ((pixel >> 2) & 0x3F3F3F) + ((color >> 2) & 0x3F3F3F)) : color;
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

void drawMenu(const char* title, const char* const* items, int count, int selected, const char* hint) {
    int w = 150, h = 44 + 12 * (count - 1);
    int x = (SCREEN_W - w) / 2, y = (SCREEN_H - h) / 2;
    fillRect(x, y, w, h, paletteColor('n'));
    fillRect(x + 1, y + 1, w - 2, h - 2, paletteColor('k'));
    drawText((SCREEN_W - textWidth(title, 2)) / 2, y + 8, title, 0xFFFFFFFF, 2);
    for (int i = 0; i < count; i++) {
        int textX = (SCREEN_W - textWidth(items[i])) / 2, textY = y + 28 + 12 * i;
        drawText(textX, textY, items[i], i == selected ? paletteColor('y') : 0xFFFFFFFF);
        if (i == selected) drawText(textX - 8, textY, ">", paletteColor('y'));
    }
    drawText((SCREEN_W - textWidth(hint)) / 2, y + 28 + 12 * (count - 1) + 8, hint, paletteColor('s'));
}

bool screenSaveBmp(const char* path) {
    const int imageSize = SCREEN_W * SCREEN_H * 3;   // rows are 960 bytes, so no row padding
    uint8_t header[54] = {};
    auto put32 = [&](int offset, uint32_t value) {
        for (int i = 0; i < 4; i++) header[offset + i] = (uint8_t)(value >> (8 * i));
    };
    header[0] = 'B';
    header[1] = 'M';
    put32(2, 54 + imageSize);
    put32(10, 54);
    put32(14, 40);
    put32(18, SCREEN_W);
    put32(22, SCREEN_H);   // positive height: rows are stored bottom-up
    header[26] = 1;        // colour planes
    header[28] = 24;       // bits per pixel
    put32(34, imageSize);

    FILE* file = fopen(path, "wb");
    if (!file) return false;
    bool ok = fwrite(header, 1, sizeof header, file) == sizeof header;
    uint8_t row[SCREEN_W * 3];
    for (int y = SCREEN_H - 1; y >= 0; y--) {
        for (int x = 0; x < SCREEN_W; x++) {
            uint32_t p = screenPixels[y * SCREEN_W + x];
            row[x * 3] = p & 0xFF;
            row[x * 3 + 1] = (p >> 8) & 0xFF;
            row[x * 3 + 2] = (p >> 16) & 0xFF;
        }
        ok &= fwrite(row, 1, sizeof row, file) == sizeof row;
    }
    return fclose(file) == 0 && ok;
}
