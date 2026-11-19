#include "formations.h"
#include "game.h"
#include <algorithm>
#include <cmath>
#include <cstring>

// The box the slots may fill (px): 40 px of sway each side, and the aliens stay above y 100 with a 4 px bob.
static const float LEFT = 40, TOP = 18, W = 228, H = 68;
static const float ALIEN_W = 12, ALIEN_H = 10;   // px: the green alien, as in game.cpp
static const float MIN_SPACING = 13.5f, MAX_SPACING = 24;   // px between neighbours along the shape: closer they crowd, sparser they stop reading as a shape
static const float MAX_PUSH = 6;                 // px a slot may be pushed off the traced line
static const float LETTER_MAX_W = 62, LETTER_GAP = 12;   // px
static const float TAU = 6.2831853f;

struct Pt { float x, y; };
typedef std::vector<Pt> Stroke;

// A stroke font on a 3 x 5 lattice (x 0-2, y 0-4, y down): "xy" points joined into strokes, '/' starts the next stroke.
static const char* const LETTER_STROKES[26] = {
    "04 01 10 21 24/02 22",          // A
    "04 00 10 21 12 02/12 23 14 04", // B
    "20 10 01 03 14 24",             // C
    "00 10 21 23 14 04 00",          // D
    "20 00 04 24/02 12",             // E
    "04 00 20/02 12",                // F
    "20 10 01 03 14 24 22 12",       // G
    "00 04/20 24/02 22",             // H
    "00 20/10 14/04 24",             // I
    "20 23 14 03",                   // J
    "00 04/20 02 24",                // K
    "00 04 24",                      // L
    "04 00 12 20 24",                // M
    "04 00 24 20",                   // N
    "10 01 03 14 23 21 10",          // O
    "04 00 10 21 12 02",             // P
    "10 01 03 14 23 21 10/12 24",    // Q
    "04 00 10 21 12 02/12 24",       // R
    "21 10 01 12 23 14 03",          // S
    "00 20/10 14",                   // T
    "00 03 14 23 20",                // U
    "00 14 20",                      // V
    "00 04 12 24 20",                // W
    "00 24/20 04",                   // X
    "00 11 20/11 14",                // Y
    "00 20 04 24",                   // Z
};

bool traceFormation(int shape, const char* word, int count, std::vector<Slot>& slots) {
    std::vector<Stroke> strokes;
    auto line = [&](float x1, float y1, float x2, float y2) { strokes.push_back({{x1, y1}, {x2, y2}}); };
    auto poly = [&](Stroke points, bool closed) {
        if (closed) points.push_back(points[0]);
        strokes.push_back(points);
    };
    auto arc = [&](float cx, float cy, float rx, float ry, float from, float to) {
        Stroke s;
        for (int i = 0; i <= 36; i++) s.push_back({cx + rx * cosf(from + (to - from) * i / 36), cy + ry * sinf(from + (to - from) * i / 36)});
        strokes.push_back(s);
    };
    auto heart = [&](float cx, float top, float w, float h) {   // 16 sin^3 t by 13 cos t - 5 cos 2t - 2 cos 3t - cos 4t, fitted to w x h
        Stroke s;
        for (int i = 0; i <= 48; i++) {
            float t = TAU * i / 48;
            float yUp = 13 * cosf(t) - 5 * cosf(2 * t) - 2 * cosf(3 * t) - cosf(4 * t);   // -17..12
            s.push_back({cx + 16 * powf(sinf(t), 3) * w / 32, top + (12 - yUp) * h / 29});
        }
        strokes.push_back(s);
    };

    switch (shape) {
        case SHAPE_DIAGONALS_R: case SHAPE_DIAGONALS_L: {
            int lines = count / 5;
            for (int i = 0; i < lines; i++) {
                float x = i * (W - 40) / (lines - 1);
                if (shape == SHAPE_DIAGONALS_R) line(x, H, x + 40, 0); else line(x, 0, x + 40, H);
            }
            break;
        }
        case SHAPE_ZIGZAG: {
            Stroke s;
            for (int i = 0; i <= count / 4; i++) s.push_back({i * W / (count / 4), i % 2 ? H : 0});
            poly(s, false);
            break;
        }
        case SHAPE_ZIGZAGS:   // two zigzags, one above the other
            for (int row = 0; row < 2; row++) {
                Stroke s;
                for (int i = 0; i <= 8; i++) s.push_back({i * W / 8, row * 38 + (i % 2 ? 30.0f : 0.0f)});
                poly(s, false);
            }
            break;
        case SHAPE_X: line(0, 0, W, H); line(W, 0, 0, H); break;
        case SHAPE_X3:
            for (float x = 0; x < W; x += 80) { line(x, 0, x + 68, H); line(x + 68, 0, x, H); }
            break;
        case SHAPE_HEART: heart(114, 0, 75, H); break;
        case SHAPE_HEARTS: for (float cx : {36.0f, 114.0f, 192.0f}) heart(cx, 6, 62, 56); break;
        case SHAPE_DIAMOND: poly({{0, 34}, {114, 0}, {228, 34}, {114, 68}}, true); break;
        case SHAPE_DIAMONDS:
            poly({{0, 34}, {114, 0}, {228, 34}, {114, 68}}, true);
            poly({{57, 34}, {114, 17}, {171, 34}, {114, 51}}, true);
            break;
        case SHAPE_CHEVRONS:   // < >
            poly({{100, 0}, {10, 34}, {100, 68}}, false);
            poly({{128, 0}, {218, 34}, {128, 68}}, false);
            break;
        case SHAPE_TRIANGLE: poly({{0, 68}, {114, 0}, {228, 68}}, true); break;
        case SHAPE_TRIANGLE_DOWN: poly({{0, 0}, {228, 0}, {114, 68}}, true); break;
        case SHAPE_BOX: poly({{0, 0}, {228, 0}, {228, 68}, {0, 68}}, true); break;
        case SHAPE_CIRCLES: for (float cx : {33.0f, 114.0f, 195.0f}) arc(cx, 34, 33, 33, 0, TAU); break;
        case SHAPE_TWINS: arc(60, 34, 34, 34, 0, TAU); arc(168, 34, 34, 34, 0, TAU); break;
        case SHAPE_OVAL: arc(114, 34, 114, 34, 0, TAU); break;
        case SHAPE_OVALS: arc(114, 34, 114, 34, 0, TAU); arc(114, 34, 66, 20, 0, TAU); break;
        case SHAPE_WAVES:   // two sine waves
            for (float y : {18.0f, 50.0f}) {
                Stroke s;
                for (float x = 0; x <= W; x += 4) s.push_back({x, y + 12 * sinf(x / W * TAU * 2)});
                poly(s, false);
            }
            break;
        case SHAPE_SPIRAL: {
            Stroke s;
            for (int i = 0; i <= 120; i++) {
                float u = i / 120.0f, r = 0.12f + 0.88f * u;
                s.push_back({114 + 114 * r * cosf(u * 3 * TAU), 34 + 34 * r * sinf(u * 3 * TAU)});
            }
            poly(s, false);
            break;
        }
        case SHAPE_PLUSES:
            for (float x = 0; x < W; x += 80) { line(x, 34, x + 68, 34); line(x + 34, 0, x + 34, H); }
            break;
        case SHAPE_POINTER: line(0, 34, W, 34); poly({{160, 0}, {228, 34}, {160, 68}}, false); break;   // an arrow to the right
        case SHAPE_BOWTIE: poly({{0, 0}, {228, 68}, {228, 0}, {0, 68}}, true); break;
        case SHAPE_CROWN: poly({{0, 68}, {0, 0}, {57, 34}, {114, 0}, {171, 34}, {228, 0}, {228, 68}}, true); break;
        case SHAPE_RAINBOW: arc(114, 68, 114, 66, TAU / 2, TAU); arc(114, 68, 76, 44, TAU / 2, TAU); break;
        case SHAPE_BOWL: arc(114, 0, 114, 66, 0, TAU / 2); arc(114, 0, 76, 44, 0, TAU / 2); break;
        case SHAPE_PYRAMID: {   // six steps up, six down, and the base
            Stroke s = {{0, H}};
            for (int i = 0; i < 6; i++) { s.push_back({i * 19.0f, H - (i + 1) * H / 6}); s.push_back({(i + 1) * 19.0f, H - (i + 1) * H / 6}); }
            for (int i = 5; i >= 0; i--) { s.push_back({W - i * 19.0f, H - (i + 1) * H / 6}); s.push_back({W - i * 19.0f, H - i * H / 6}); }
            poly(s, true);
            break;
        }
        case SHAPE_LETTERS: case SHAPE_WORD: {
            int letters = (int)strlen(word);
            float letterW = std::min(LETTER_MAX_W, (W - (letters - 1) * LETTER_GAP) / letters);
            float left = (W - (letters * letterW + (letters - 1) * LETTER_GAP)) / 2;
            for (int i = 0; i < letters; i++) {
                Stroke s;
                for (const char* c = LETTER_STROKES[word[i] - 'A'];; c++) {
                    if (*c >= '0' && *c <= '9') {
                        s.push_back({left + i * (letterW + LETTER_GAP) + (c[0] - '0') * letterW / 2, (c[1] - '0') * H / 4});
                        c++;
                    } else if (*c == '/' || *c == 0) {
                        strokes.push_back(s);
                        s.clear();
                        if (*c == 0) break;
                    }
                }
            }
            break;
        }
        default: return false;
    }

    float length = 0;
    for (const Stroke& s : strokes)
        for (size_t j = 1; j < s.size(); j++) length += hypotf(s[j].x - s[j - 1].x, s[j].y - s[j - 1].y);
    if (length / count > MAX_SPACING || length / count < MIN_SPACING) return false;

    // Share the slots out along the strokes by length; try dots on the stroke ends first (they keep corners), then shifted spacings.
    for (float phase : {-1.0f, 0.5f, 0.0f, 0.25f, 0.75f}) {
        slots.clear();
        float before = 0;   // length of the strokes already handled
        for (const Stroke& s : strokes) {
            float strokeLength = 0;
            for (size_t j = 1; j < s.size(); j++) strokeLength += hypotf(s[j].x - s[j - 1].x, s[j].y - s[j - 1].y);
            int dots = (int)roundf((before + strokeLength) * count / length) - (int)roundf(before * count / length);
            before += strokeLength;
            bool closed = hypotf(s[0].x - s.back().x, s[0].y - s.back().y) < 0.01f;
            for (int dot = 0; dot < dots; dot++) {
                float along;   // distance along the stroke
                if (phase >= 0) along = (dot + phase) * strokeLength / dots;
                else if (closed) along = dot * strokeLength / dots;
                else along = dots > 1 ? dot * strokeLength / (dots - 1) : strokeLength / 2;
                size_t j = 1;
                float segmentStart = 0, segment = hypotf(s[1].x - s[0].x, s[1].y - s[0].y);
                while (j + 1 < s.size() && along > segmentStart + segment) {
                    segmentStart += segment;
                    j++;
                    segment = hypotf(s[j].x - s[j - 1].x, s[j].y - s[j - 1].y);
                }
                float t = segment > 0.001f ? std::min((along - segmentStart) / segment, 1.0f) : 0;
                slots.push_back({LEFT + std::max(0.0f, std::min(s[j - 1].x + (s[j].x - s[j - 1].x) * t, W)),
                                 TOP + std::max(0.0f, std::min(s[j - 1].y + (s[j].y - s[j - 1].y) * t, H))});
            }
        }
        if ((int)slots.size() < count) continue;

        // Push apart slots that would overlap (sharp corners, crossings), keeping them in the box.
        std::vector<Slot> sampled = slots;
        bool settled = false;
        for (int pass = 0; pass < 40 && !settled; pass++) {
            settled = true;
            for (size_t a = 0; a < slots.size(); a++) {
                for (size_t b = a + 1; b < slots.size(); b++) {
                    float dx = slots[b].x - slots[a].x, dy = slots[b].y - slots[a].y;
                    float overlapX = ALIEN_W - fabsf(dx), overlapY = ALIEN_H - fabsf(dy);
                    if (overlapX <= 0 || overlapY <= 0) continue;
                    settled = false;
                    if (overlapX < overlapY) {
                        float push = (overlapX + 0.5f) / 2 * (dx < 0 ? -1 : 1);
                        slots[a].x -= push;
                        slots[b].x += push;
                    } else {
                        float push = (overlapY + 0.5f) / 2 * (dy < 0 ? -1 : 1);
                        slots[a].y -= push;
                        slots[b].y += push;
                    }
                }
            }
            for (Slot& slot : slots) {
                slot.x = std::max(LEFT, std::min(slot.x, LEFT + W));
                slot.y = std::max(TOP, std::min(slot.y, TOP + H));
            }
        }
        if (!settled) continue;
        float moved = 0;
        for (size_t i = 0; i < slots.size(); i++) moved = std::max(moved, hypotf(slots[i].x - sampled[i].x, slots[i].y - sampled[i].y));
        if (moved > MAX_PUSH) continue;   // the shape would come out smudged

        // Rows of 8 from the top (the tougher aliens take the top rows), each row left to right.
        std::sort(slots.begin(), slots.end(), [](const Slot& a, const Slot& b) { return a.y < b.y; });
        for (int row = 0; row < count; row += 8)
            std::sort(slots.begin() + row, slots.begin() + std::min(row + 8, count), [](const Slot& a, const Slot& b) { return a.x < b.x; });
        return true;
    }
    return false;
}
