// Palette, sprites and font as text grids: one character per pixel, '.' is transparent (docs/features/pixel-art).
#pragma once
#include <stdint.h>

// A w x h grid of palette characters, rows concatenated top to bottom.
struct Sprite {
    int w, h;
    const char* pixels;
};

// Palette: one character per colour (ARGB). Magenta 'm' is only for enemy shots.
inline uint32_t paletteColor(char c) {
    switch (c) {
        case 'k': return 0xFF0C0F1E;   // background navy
        case 'n': return 0xFF262B4A;   // far stars
        case 'd': return 0xFF3E4566;   // middle stars
        case 's': return 0xFF6A7090;   // near stars, grey
        case 'b': return 0xFFC0C4D8;   // light grey (ship body)
        case 'w': return 0xFFFFFFFF;
        case 'c': return 0xFF57D4F0;   // cyan
        case 'r': return 0xFFE04848;   // red
        case 'R': return 0xFF8E2430;   // dark red
        case 'o': return 0xFFFFA020;   // orange
        case 'y': return 0xFFFFF080;   // pale yellow: player bullets
        case 'g': return 0xFF3FAE5A;   // green alien
        case 'G': return 0xFF206A34;   // dark green
        case 'u': return 0xFF4878E8;   // blue alien
        case 'U': return 0xFF24409A;   // dark blue
        case 'm': return 0xFFFF40C0;   // magenta: enemy shots only
        case 'p': return 0xFFA070FF;   // purple: pickups
        default: return 0;             // '.' and anything else: transparent
    }
}

const Sprite SHIP_A = {16, 14,
    ".......cc......."
    "......cwwc......"
    "......cwwc......"
    ".....cbwwbc....."
    ".....bbbbbb....."
    "..r..bbbbbb..r.."
    "..r.bbbbbbbb.r.."
    ".rb.bbbccbbb.br."
    ".bbbbbbccbbbbbb."
    "bbbbbbbbbbbbbbbb"
    "bbbrbbbbbbbbrbbb"
    ".bb.rr....rr.bb."
    "....oo....oo...."
    ".....o....o....."};
const Sprite SHIP_B = {16, 14,
    ".......cc......."
    "......cwwc......"
    "......cwwc......"
    ".....cbwwbc....."
    ".....bbbbbb....."
    "..r..bbbbbb..r.."
    "..r.bbbbbbbb.r.."
    ".rb.bbbccbbb.br."
    ".bbbbbbccbbbbbb."
    "bbbbbbbbbbbbbbbb"
    "bbbrbbbbbbbbrbbb"
    ".bb.rr....rr.bb."
    "....yo....oy...."
    "................"};

// Player 2's ship: the same shape with an orange cockpit and blue stripes.
const Sprite SHIP2_A = {16, 14,
    ".......oo......."
    "......owwo......"
    "......owwo......"
    ".....obwwbo....."
    ".....bbbbbb....."
    "..u..bbbbbb..u.."
    "..u.bbbbbbbb.u.."
    ".ub.bbboobbb.bu."
    ".bbbbbboobbbbbb."
    "bbbbbbbbbbbbbbbb"
    "bbbubbbbbbbbubbb"
    ".bb.uu....uu.bb."
    "....oo....oo...."
    ".....o....o....."};
const Sprite SHIP2_B = {16, 14,
    ".......oo......."
    "......owwo......"
    "......owwo......"
    ".....obwwbo....."
    ".....bbbbbb....."
    "..u..bbbbbb..u.."
    "..u.bbbbbbbb.u.."
    ".ub.bbboobbb.bu."
    ".bbbbbboobbbbbb."
    "bbbbbbbbbbbbbbbb"
    "bbbubbbbbbbbubbb"
    ".bb.uu....uu.bb."
    "....yo....oy...."
    "................"};

const Sprite GREEN_ALIEN_A = {12, 10,
    "..g......g.."
    "...g....g..."
    "..gggggggg.."
    ".ggwkggwkgg."
    "gggggggggggg"
    "gGggggggggGg"
    "g.gggggggg.g"
    "g.g......g.g"
    "...gg..gg..."
    "............"};
const Sprite GREEN_ALIEN_B = {12, 10,
    "..g......g.."
    "...g....g..."
    "..gggggggg.."
    ".ggwkggwkgg."
    "gggggggggggg"
    "gGggggggggGg"
    "..gggggggg.."
    ".g.g....g.g."
    "g..........g"
    "............"};
const Sprite BLUE_ALIEN_A = {12, 10,
    ".....uu....."
    "....uuuu...."
    "...uuuuuu..."
    "..uuwuuwuu.."
    "..uukuukuu.."
    "..uuuuuuuu.."
    "..UuUuuUuU.."
    "...u.uu.u..."
    "..u.u..u.u.."
    ".u........u."};
const Sprite BLUE_ALIEN_B = {12, 10,
    ".....uu....."
    "....uuuu...."
    "...uuuuuu..."
    "..uuwuuwuu.."
    "..uukuukuu.."
    "..uuuuuuuu.."
    "..UuUuuUuU.."
    "...u.uu.u..."
    "...u.uu.u..."
    "..u..uu..u.."};
const Sprite RED_ALIEN_A = {12, 10,
    ".r........r."
    "..r......r.."
    "..RRRRRRRR.."
    ".RrrrrrrrrR."
    "RrrwkrrwkrrR"
    "RrrrrrrrrrrR"
    ".RRRRRRRRRR."
    ".r.r....r.r."
    "r...r..r...r"
    "............"};
const Sprite RED_ALIEN_B = {12, 10,
    ".r........r."
    "..r......r.."
    "..RRRRRRRR.."
    ".RrrrrrrrrR."
    "RrrwkrrwkrrR"
    "RrrrrrrrrrrR"
    ".RRRRRRRRRR."
    "..r.r..r.r.."
    "..r..rr..r.."
    "............"};

const Sprite BULLET = {2, 4,
    "ww"
    "yy"
    "yy"
    "yy"};
const Sprite ENEMY_SHOT = {2, 4,
    "mm"
    "mw"
    "mm"
    "mm"};
const Sprite ROCKET = {4, 8,
    ".ww."
    ".bb."
    "bbbb"
    "brrb"
    "bbbb"
    "rbbr"
    "r..r"
    ".oo."};
const Sprite ROCKET_PICKUP = {8, 8,
    ".pppppp."
    "pp.ww.pp"
    "p..bb..p"
    "p.bbbb.p"
    "p.brrb.p"
    "p.r..r.p"
    "pp.oo.pp"
    ".pppppp."};
const Sprite UPGRADE_PICKUP = {8, 8,
    ".oooooo."
    "oo.yy.oo"
    "o.yyyy.o"
    "oyyyyyyo"
    "o..yy..o"
    "o..yy..o"
    "oo.yy.oo"
    ".oooooo."};
const Sprite LIFE_ICON = {7, 5,
    "...c..."
    "..cbc.."
    ".bbbbb."
    "bbbbbbb"
    "b.r.r.b"};
const Sprite ROCKET_ICON = {3, 5,
    ".w."
    "bbb"
    "brb"
    "bbb"
    "r.r"};

// 3x5 font: 5 octal digits, one per row from the top; bit 2 of a row is its left pixel. 0 = blank.
inline int fontGlyph(char c) {
    switch (c) {
        case '0': return 075557; case '1': return 026227; case '2': return 071747; case '3': return 071717;
        case '4': return 055711; case '5': return 074717; case '6': return 074757; case '7': return 071122;
        case '8': return 075757; case '9': return 075717;
        case 'A': return 025755; case 'B': return 065656; case 'C': return 034443; case 'D': return 065556;
        case 'E': return 074647; case 'F': return 074644; case 'G': return 034553; case 'H': return 055755;
        case 'I': return 072227; case 'J': return 011157; case 'K': return 055655; case 'L': return 044447;
        case 'M': return 057755; case 'N': return 065555; case 'O': return 025552; case 'P': return 065644;
        case 'Q': return 025563; case 'R': return 065655; case 'S': return 034716; case 'T': return 072222;
        case 'U': return 055557; case 'V': return 055552; case 'W': return 055775; case 'X': return 055255;
        case 'Y': return 055222; case 'Z': return 071247;
        case '.': return 000002; case ':': return 002020; case '-': return 000700; case '!': return 022202;
        case '/': return 011244; case '+': return 002720; case '%': return 051245; case '>': return 042124;
        default: return 0;
    }
}
