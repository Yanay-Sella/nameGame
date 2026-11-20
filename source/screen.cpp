// Software drawing into the current target pixel buffer; everything is clipped to it.
#include "screen.h"
#include <stdio.h>
#include <string.h>

uint32_t screenPixels[SCREEN_W * SCREEN_H];
uint32_t questPixels[QUEST_W * QUEST_H];
uint32_t* screenTarget = screenPixels;
int screenW = SCREEN_W, screenH = SCREEN_H;

void screenSetTarget(uint32_t* pixels, int w, int h) {
    screenTarget = pixels;
    screenW = w;
    screenH = h;
}

void fillRect(int x, int y, int w, int h, uint32_t color) {
    int x0 = x < 0 ? 0 : x, y0 = y < 0 ? 0 : y;
    int x1 = x + w > screenW ? screenW : x + w, y1 = y + h > screenH ? screenH : y + h;
    for (int py = y0; py < y1; py++)
        for (int px = x0; px < x1; px++) screenTarget[py * screenW + px] = color;
}

void fillCircle(int cx, int cy, int radius, uint32_t color) {
    for (int dy = -radius; dy <= radius; dy++)
        for (int dx = -radius; dx <= radius; dx++)
            if (dx * dx + dy * dy <= radius * radius) fillRect(cx + dx, cy + dy, 1, 1, color);
}

void drawSprite(const Sprite& sprite, int x, int y) {
    for (int row = 0; row < sprite.h; row++) {
        int py = y + row;
        if (py < 0 || py >= screenH) continue;
        for (int col = 0; col < sprite.w; col++) {
            int px = x + col;
            char c = sprite.pixels[row * sprite.w + col];
            uint32_t color = paletteColor(c);
            if (!color || px < 0 || px >= screenW) continue;
            uint32_t& pixel = screenTarget[py * screenW + px];
            pixel = c == '~' ? 0xFF000000 | (((pixel >> 1) & 0x7F7F7F) + ((pixel >> 2) & 0x3F3F3F) + ((color >> 2) & 0x3F3F3F)) : color;
        }
    }
}

void drawSpriteEx(const Sprite& sprite, int x, int y, bool flip, char swapFrom, uint32_t swapTo) {
    for (int row = 0; row < sprite.h; row++) {
        int py = y + row;
        if (py < 0 || py >= screenH) continue;
        for (int col = 0; col < sprite.w; col++) {
            int px = x + col;
            char c = sprite.pixels[row * sprite.w + (flip ? sprite.w - 1 - col : col)];
            uint32_t color = swapFrom && c == swapFrom ? swapTo : paletteColor(c);
            if (!color || px < 0 || px >= screenW) continue;
            uint32_t& pixel = screenTarget[py * screenW + px];
            pixel = c == '~' ? 0xFF000000 | (((pixel >> 1) & 0x7F7F7F) + ((pixel >> 2) & 0x3F3F3F) + ((color >> 2) & 0x3F3F3F)) : color;
        }
    }
}

void drawSpritePal(const Sprite& sprite, int x, int y, bool flip, const WorldPalette& pal) {
    static int8_t index[256];   // character → palette index, -1 transparent
    static bool ready = false;
    if (!ready) {
        for (int c = 0; c < 256; c++) index[c] = c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1;
        ready = true;
    }
    int col0 = x < 0 ? -x : 0, col1 = x + sprite.w > screenW ? screenW - x : sprite.w;
    int row0 = y < 0 ? -y : 0, row1 = y + sprite.h > screenH ? screenH - y : sprite.h;
    for (int row = row0; row < row1; row++) {
        const char* src = sprite.pixels + row * sprite.w;
        uint32_t* dst = screenTarget + (y + row) * screenW + x;
        for (int col = col0; col < col1; col++) {
            int i = index[(uint8_t)src[flip ? sprite.w - 1 - col : col]];
            if (i >= 0) dst[col] = pal.c[i];
        }
    }
}

void drawSpriteOutlined(const Sprite& sprite, int x, int y, bool flip, uint32_t outline, char swapFrom, uint32_t swapTo) {
    auto opaque = [&](int col, int row) {
        if (col < 0 || row < 0 || col >= sprite.w || row >= sprite.h) return false;
        char c = sprite.pixels[row * sprite.w + (flip ? sprite.w - 1 - col : col)];
        return (swapFrom && c == swapFrom) || paletteColor(c) != 0;
    };
    for (int row = -1; row <= sprite.h; row++) {
        int py = y + row;
        if (py < 0 || py >= screenH) continue;
        for (int col = -1; col <= sprite.w; col++) {
            int px = x + col;
            if (px < 0 || px >= screenW || opaque(col, row)) continue;
            if (opaque(col - 1, row) || opaque(col + 1, row) || opaque(col, row - 1) || opaque(col, row + 1))
                screenTarget[py * screenW + px] = outline;
        }
    }
    drawSpriteEx(sprite, x, y, flip, swapFrom, swapTo);
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

// Sizes are in shooter pixels, scaled by u (1 over the shooter, 2 over NAME QUEST) so the menus look the same on both.
void drawMenu(const char* title, const char* const* items, int count, int selected, const char* hint) {
    int u = screenW / SCREEN_W;
    int w = 150 * u, h = (44 + 12 * (count - 1)) * u;
    int x = (screenW - w) / 2, y = (screenH - h) / 2;
    fillRect(x, y, w, h, paletteColor('n'));
    fillRect(x + u, y + u, w - 2 * u, h - 2 * u, paletteColor('k'));
    drawText((screenW - textWidth(title, 2 * u)) / 2, y + 8 * u, title, 0xFFFFFFFF, 2 * u);
    for (int i = 0; i < count; i++) {
        int textX = (screenW - textWidth(items[i], u)) / 2, textY = y + (28 + 12 * i) * u;
        drawText(textX, textY, items[i], i == selected ? paletteColor('y') : 0xFFFFFFFF, u);
        if (i == selected) drawText(textX - 8 * u, textY, ">", paletteColor('y'), u);
    }
    drawText((screenW - textWidth(hint, u)) / 2, y + (28 + 12 * (count - 1) + 8) * u, hint, paletteColor('s'), u);
}

bool screenSaveBmp(const char* path) {
    const int imageSize = screenW * screenH * 3;   // rows are 960 or 1920 bytes, so no row padding
    uint8_t header[54] = {};
    auto put32 = [&](int offset, uint32_t value) {
        for (int i = 0; i < 4; i++) header[offset + i] = (uint8_t)(value >> (8 * i));
    };
    header[0] = 'B';
    header[1] = 'M';
    put32(2, 54 + imageSize);
    put32(10, 54);
    put32(14, 40);
    put32(18, screenW);
    put32(22, screenH);   // positive height: rows are stored bottom-up
    header[26] = 1;        // colour planes
    header[28] = 24;       // bits per pixel
    put32(34, imageSize);

    FILE* file = fopen(path, "wb");
    if (!file) return false;
    bool ok = fwrite(header, 1, sizeof header, file) == sizeof header;
    static uint8_t row[QUEST_W * 3];
    for (int y = screenH - 1; y >= 0; y--) {
        for (int x = 0; x < screenW; x++) {
            uint32_t p = screenTarget[y * screenW + x];
            row[x * 3] = p & 0xFF;
            row[x * 3 + 1] = (p >> 8) & 0xFF;
            row[x * 3 + 2] = (p >> 16) & 0xFF;
        }
        ok &= fwrite(row, 1, screenW * 3, file) == (size_t)screenW * 3;
    }
    return fclose(file) == 0 && ok;
}
