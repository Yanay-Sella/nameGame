// Palette, sprites and font as text grids: one character per pixel, '.' is transparent (docs/features/pixel-art).
#pragma once
#include <stdint.h>

// A w x h grid of palette characters, rows concatenated top to bottom.
struct Sprite {
    int w, h;
    const char* pixels;
};

// Palette: one character per colour (ARGB). Magenta 'm' is only for deadly enemy projectiles.
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
        case 'J': return 0xFF07120D;   // jungle: deepest shade
        case 'j': return 0xFF10251A;   // jungle: far trunks
        case 'f': return 0xFF173A26;   // jungle: dark leaves
        case 'F': return 0xFF25502F;   // jungle: light leaves
        case 't': return 0xFF2A1F14;   // jungle: near trunk bark
        case 'T': return 0xFF3E2E1C;   // jungle: trunk edge, ropes
        case 'B': return 0xFF54402A;   // jungle: bridge planks
        case 'L': return 0xFF687834;   // jungle: firefly
        case 'l': return 0xFF9BD432;   // lime: leafhopper
        case 'h': return 0xFF9A5E2C;   // brown: beetle, monkey fur
        case 'H': return 0xFFD8A868;   // tan: highlights, faces, horn
        case 'x': return 0xFF1E1A1C;   // insect black
        case 'X': return 0xFF5C5866;   // slate: shine on black, gorilla fur
        case 'W': return 0xFF3A2212;   // gorilla: dark brown (shadows, hands, feet)
        case 'P': return 0xFF62391E;   // gorilla: brown fur
        case 'S': return 0xFF8A5631;   // gorilla: light brown (shoulders, arms)
        case '~': return 0xFF9FE0FF;   // see-through ice: drawSprite mixes a quarter of it into the pixel under it
        case 'Y': return 0xFFF0C828;   // banana yellow (not the bullets' pale yellow)
        case 'M': return 0xFF7DA83A;   // stink mist
        case 'a': return 0xFF0A1322;   // glacier: deepest
        case 'A': return 0xFF132238;   // glacier: cave wall
        case 'e': return 0xFF1E3550;   // glacier: crystals
        case 'E': return 0xFF2E5470;   // glacier: crystal edges, icicles, snow
        case 'i': return 0xFF9FE0FF;   // pale ice
        case 'I': return 0xFF4A9AD8;   // mid ice
        case 'q': return 0xFF2A5C9C;   // deep ice
        case 'v': return 0xFF120706;   // volcano: deepest
        case 'V': return 0xFF241009;   // volcano: dark rock
        case 'K': return 0xFF3A1A10;   // volcano: rock edge
        case 'D': return 0xFF5C1C0C;   // volcano: glowing crack
        case 'C': return 0xFF7A3010;   // volcano: lava river, embers
        case 'O': return 0xFFFF6A10;   // hot orange
        case 'N': return 0xFFFFD84A;   // white-hot yellow
        case 'Z': return 0xFFB02810;   // deep red-orange
        case '1': return 0xFF051220;   // sea: deepest
        case '2': return 0xFF0A1E30;   // sea: water
        case '3': return 0xFF0E3030;   // sea: kelp
        case '4': return 0xFF1A4A3E;   // sea: kelp light
        case '5': return 0xFF28506A;   // sea: light rays, bubbles
        case '6': return 0xFF3FB8A0;   // teal fish
        case '7': return 0xFF1D6A5E;   // dark teal
        case '8': return 0xFFE0E8C8;   // pale belly, teeth
        case '9': return 0xFFE6FF6A;   // lure glow
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

// Player 2's ship: the same shape with a gold hull, an orange cockpit and red-orange stripes.
const Sprite SHIP2_A = {16, 14,
    ".......oo......."
    "......owwo......"
    "......owwo......"
    ".....oYwwYo....."
    ".....YYYYYY....."
    "..Z..YYYYYY..Z.."
    "..Z.YYYYYYYY.Z.."
    ".ZY.YYYooYYY.YZ."
    ".YYYYYYooYYYYYY."
    "YYYYYYYYYYYYYYYY"
    "YYYZYYYYYYYYZYYY"
    ".YY.ZZ....ZZ.YY."
    "....oo....oo...."
    ".....o....o....."};
const Sprite SHIP2_B = {16, 14,
    ".......oo......."
    "......owwo......"
    "......owwo......"
    ".....oYwwYo....."
    ".....YYYYYY....."
    "..Z..YYYYYY..Z.."
    "..Z.YYYYYYYY.Z.."
    ".ZY.YYYooYYY.YZ."
    ".YYYYYYooYYYYYY."
    "YYYYYYYYYYYYYYYY"
    "YYYZYYYYYYYYZYYY"
    ".YY.ZZ....ZZ.YY."
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
const Sprite BOSS_A = {32, 20,
    "......r..................r......"
    "......rr...GGGGGGGGGG...rr......"
    ".......rGGGggggggggggGGGr......."
    "....GGGggggggggggggggggggGGG...."
    "...GggggggggUUUUUUUUggggggggG..."
    "..GggggggguuuuuuuuuuuugggggggG.."
    "..GggwwwgguuuuUUUUuuuuggwwwggG.."
    ".GggwwwwggguuuuuuuuuugggwwwwggG."
    ".GggwwRRggguuuuuuuuuugggRRwwggG."
    ".GggwwRRggguuuUUUUuuugggRRwwggG."
    ".GGggwwgggguuuUUUUuuuggggwwggGG."
    "..GGgggggggggUUUUUUgggggggggGG.."
    "...GGGggggrrrrRRRRrrrrggggGGG..."
    "....GGGGGGrRRRRRRRRRRrGGGGGG...."
    "......GGGGGGGGGGGGGGGGGGGG......"
    ".....GG.....UU....UU.....GG....."
    "....GG......UU....UU......GG...."
    "...GG.......rr....rr.......GG..."
    "..GG.........rr..rr.........GG.."
    "..G...........r..r...........G.."};
const Sprite BOSS_B = {32, 20,
    "......r..................r......"
    "......rr...GGGGGGGGGG...rr......"
    ".......rGGGggggggggggGGGr......."
    "....GGGggggggggggggggggggGGG...."
    "...GggggggggUUUUUUUUggggggggG..."
    "..GggggggguuuuuuuuuuuugggggggG.."
    "..GggwwwgguuuuUUUUuuuuggwwwggG.."
    ".GggwwwwggguuuuuuuuuugggwwwwggG."
    ".GggwwwRggguuuuuuuuuugggRwwwggG."
    ".GggwwwRggguuuUUUUuuugggRwwwggG."
    ".GGggwwgggguuuUUUUuuuggggwwggGG."
    "..GGgggggggggUUUUUUgggggggggGG.."
    "...GGGggggrrrrRRRRrrrrggggGGG..."
    "....GGGGGGrRRRRRRRRRRrGGGGGG...."
    "......GGGGGGGGGGGGGGGGGGGG......"
    ".....GG.....UU....UU.....GG....."
    ".....GG.....UU....UU.....GG....."
    "....GG.......rr..rr.......GG...."
    "....GG.......rr..rr.......GG...."
    "...GG..........rr..........GG..."};
// Mage: the green alien under a tall starry hat, holding a wand whose star (30, 8) casts the power ball.
const Sprite MAGE_A = {32, 20,
    ".............Y.................."
    ".............UU................."
    "............UuuU................"
    "...........UuuuuU..............."
    "..........UuuYuuuU.............."
    ".........UuuuuuuuuU............."
    "........UuuuuuuuYuuU............"
    "......UccccccccccccU..........Y."
    ".UUUUUUUUUUUUUUUUUUUUUUUU....YwY"
    "..GggggggggggggggggggggG......Y."
    ".GggwwwgggggggggggggwwwgG....h.."
    "GggwwRRwggggggggggwRRwwggG..h..."
    "GggwwRRwggggggggggwRRwwgggGh...."
    "GGggwwgggggggggggggggwwggGGG...."
    ".GGggggrrrrRRRRRRrrrrggggGGG...."
    "..GGGGGGrRRRRRRRRRRrGGGGGG......"
    "...GG.....UU....UU.....GG......."
    "..GG......UU....UU......GG......"
    ".GG........rr..rr........GG....."
    ".G..........r..r..........G....."};
const Sprite MAGE_B = {32, 20,
    ".............Y.................."
    ".............UU................."
    "............UuuU................"
    "...........UuuuuU..............."
    "..........UuuYuuuU.............."
    ".........UuuuuuuuuU............."
    "........UuuuuuuuYuuU............"
    "......UccccccccccccU..........w."
    ".UUUUUUUUUUUUUUUUUUUUUUUU....wYw"
    "..GggggggggggggggggggggG......w."
    ".GggwwwgggggggggggggwwwgG....h.."
    "GggwwwRRggggggggggRRwwwggG..h..."
    "GggwwwRRggggggggggRRwwwgggGh...."
    "GGggwwgggggggggggggggwwggGGG...."
    ".GGggggrrrrRRRRRRrrrrggggGGG...."
    "..GGGGGGrRRRRRRRRRRrGGGGGG......"
    "...GG.....UU....UU.....GG......."
    "...GG.....UU....UU.....GG......."
    "..GG.......rr..rr.......GG......"
    "..GG.......rr..rr.......GG......"};
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

const Sprite LEAFHOPPER_A = {12, 10,
    ".l........l."
    "..l......l.."
    "...GllllG..."
    "..lrllllrl.."
    ".GllllllllG."
    "GlGllllllGlG"
    "G.lGllllGl.G"
    "..lGllllGl.."
    ".G..llll..G."
    "G...G..G...G"};
const Sprite LEAFHOPPER_B = {12, 10,
    ".l........l."
    "..l......l.."
    "...GllllG..."
    "..lrllllrl.."
    ".GllllllllG."
    "lGGllllllGGl"
    "..lGllllGl.."
    ".GlGllllGlG."
    "G...llll...G"
    "....G..G...."};
const Sprite BEETLE_A = {12, 10,
    "...x....x..."
    "....xxxx...."
    "...xHxxHx..."
    "x.hhhxxhhh.x"
    ".hHHhxxhhhh."
    "xhHhhxxhhhhx"
    ".hhhhxxhhhh."
    "xhhhhxxhhHhx"
    "..hhhxxhhh.."
    ".x..hhhh..x."};
const Sprite BEETLE_B = {12, 10,
    "...x....x..."
    "....xxxx...."
    "...xHxxHx..."
    "..hhhxxhhh.."
    "xhHHhxxhhhhx"
    ".hHhhxxhhhh."
    "xhhhhxxhhhhx"
    ".hhhhxxhhHh."
    "x.hhhxxhhh.x"
    "....hhhh...."};
const Sprite STAG_A = {12, 10,
    ".h........h."
    "..h......h.."
    "..hh....hh.."
    "...xXxxXx..."
    "..xrxxxxrx.."
    "x.xXXxxXXx.x"
    ".xXXxxxxXXx."
    "xxXxxxxxxXxx"
    ".xxXxxxxXxx."
    "x..xxxxxx..x"};
const Sprite STAG_B = {12, 10,
    "..h......h.."
    "..h......h.."
    "...hh..hh..."
    "...xXxxXx..."
    "x.xrxxxxrx.x"
    ".xxXXxxXXxx."
    "..XXxxxxXX.."
    "xxXxxxxxxXxx"
    ".xxXxxxxXxx."
    "...xxxxxx..."};
const Sprite RHINO_A = {16, 12,
    ".......HH......."
    "......HHHH......"
    ".......HH......."
    "....xxxHHxxx...."
    "x..xXxrxxrxXx..x"
    ".xxXXxxxxxxXXxx."
    "...xXlxxxxlXx..."
    "x.xXlXxxxxXlXx.x"
    ".xxXXxxxxxxXXxx."
    "...xXxxxxxxXx..."
    "..x.xxxxxxxx.x.."
    ".x...xx..xx...x."};
const Sprite RHINO_B = {16, 12,
    ".......HH......."
    "......HHHH......"
    ".......HH......."
    "....xxxHHxxx...."
    "...xXxrxxrxXx..."
    "xxxXXxxxxxxXXxxx"
    "...xXlxxxxlXx..."
    ".xxXlXxxxxXlXxx."
    "x..XXxxxxxxXX..x"
    "...xXxxxxxxXx..."
    ".x..xxxxxxxx..x."
    "x....xx..xx....x"};
// Monkey: brown fur, tan face and belly, a banana in the raised hand on frame A.
const Sprite MONKEY_A = {32, 20,
    "......................YY........"
    ".....................YY........."
    "....................HYh........."
    "...........hhhhhh..hHh.........."
    "..........hhhhhhhh.hh..........."
    ".........hhHHHHHHhhh............"
    "........HhHHxHHxHHhH............"
    "........HhHHHHHHHHhH............"
    ".........hHHHrrHHHh............."
    "..........hHHHHHHh.............."
    "......hhhhhhhhhhhhhh............"
    ".....hh..hhHHHHHhh.hh..........."
    "....hh...hHHHHHHHh..hh.........."
    "...HH....hHHHHHHHh...HH........."
    ".........hhHHHHHhh.............."
    "..........hhhhhhh.....hhh......."
    ".........hh.....hh...h...h......"
    "........hh.......hh..h.........."
    ".......HH.........HH..hh........"
    "........................hh......"};
const Sprite MONKEY_B = {32, 20,
    "................................"
    "................................"
    "................................"
    "...........hhhhhh..............."
    "..........hhhhhhhh.............."
    ".........hhHHHHHHhh............."
    "........HhHHxHHxHHhH............"
    "........HhHHHHHHHHhH............"
    ".........hHHHrrHHHh............."
    "..........hHHHHHHh.............."
    "......hhhhhhhhhhhhhhh..........."
    ".....hh..hhHHHHHhh..hh.........."
    "....hh...hHHHHHHHh...hh........."
    "...HH....hHHHHHHHh....HH........"
    ".........hhHHHHHhh......YY......"
    "..........hhhhhhh.....hhhY......"
    ".........hh.....hh...h...h......"
    ".........hh.....hh...h.........."
    "........HH.......HH...hh........"
    "........................hh......"};
// Gorilla: bulky brown fur, light-brown shoulders, a dark leathery face and angry red eyes.
const Sprite GORILLA_A = {32, 20,
    "............PPPPPPPP............"
    "..........PPPPPPPPPPPP.........."
    ".........PPXXXXXXXXXXPP........."
    ".....PPPPPXxxrXXXXrxxXPPPPP....."
    "...PPPPPPPXXXXXXXXXXXXPPPPPPP..."
    "..PPPSSPPPPXXXxxxxXXXPPPPSSPPP.."
    ".PPPSSSSPPPPXXXXXXXXPPPPSSSSPPP."
    ".PPSSSSPPPPPPPPPPPPPPPPPPSSSSPP."
    "PPPSSSPPPWPPPPPPPPPPPPWPPPSSSPPP"
    "PPPSSPPPPWWPPPPWWPPPPWWPPPPSSPPP"
    "PPPPPPPP.PWWWWWPPWWWWWP.PPPPPPPP"
    "PPPPPPP..PPPPPPPPPPPPPP..PPPPPPP"
    ".PPPPPP..PPPPPPPPPPPPPP..PPPPPP."
    ".PPPPP...PPPPPPPPPPPPPP...PPPPP."
    ".PPPPP...WPPPPPPPPPPPPW...PPPPP."
    ".PPPPP....PPPPP..PPPPP....PPPPP."
    ".WWWWW....PPPPP..PPPPP....WWWWW."
    "WWWWWW....PPPP....PPPP....WWWWWW"
    ".........WWWWW....WWWWW........."
    "................................"};
const Sprite GORILLA_B = {32, 20,
    "............PPPPPPPP............"
    "..........PPPPPPPPPPPP.........."
    ".........PPXXXXXXXXXXPP........."
    ".....PPPPPXxrxXXXXxrxXPPPPP....."
    "...PPPPPPPXXXXXXXXXXXXPPPPPPP..."
    "..PPPSSPPPPXXXxxxxXXXPPPPSSPPP.."
    ".PPPSSSSPPPPXXXXXXXXPPPPSSSSPPP."
    ".PPSSSSPPPPPPPPPPPPPPPPPPSSSSPP."
    "PPPSSSPPPWPPPPPPPPPPPPWPPPSSSPPP"
    "PPPSSPPPPWWPPPPWWPPPPWWPPPPSSPPP"
    "PPPPPPPP.PWWWWWPPWWWWWP.PPPPPPPP"
    "PPPPPPP..PPPPPPPPPPPPPP..PPPPPPP"
    "..PPPPPP.PPPPPPPPPPPPPP.PPPPPP.."
    "..PPPPP..PPPPPPPPPPPPPP..PPPPP.."
    "..PPPPP..WPPPPPPPPPPPPW..PPPPP.."
    "..PPPPP...PPPPP..PPPPP...PPPPP.."
    "..WWWWW...PPPPP..PPPPP...WWWWW.."
    ".WWWWWW...PPPP....PPPP...WWWWWW."
    ".........WWWWW....WWWWW........."
    "................................"};
// Laugh: eyes squeezed shut, mouth wide open, head tipped back.
const Sprite GORILLA_LAUGH = {32, 20,
    "............PPPPPPPP............"
    "..........PPPPPPPPPPPP.........."
    ".........PPXXXXXXXXXXPP........."
    ".....PPPPPXxxxXXXXxxxXPPPPP....."
    "...PPPPPPPXXXXXXXXXXXXPPPPPPP..."
    "..PPPSSPPPPXXrrrrrrXXPPPPSSPPP.."
    ".PPPSSSSPPPPXrrrrrrXPPPPSSSSPPP."
    ".PPSSSSPPPPPPXXXXXXPPPPPPSSSSPP."
    "PPPSSSPPPWPPPPPPPPPPPPWPPPSSSPPP"
    "PPPSSPPPPWWPPPPWWPPPPWWPPPPSSPPP"
    "PPPPPPPP.PWWWWWPPWWWWWP.PPPPPPPP"
    "PPPPPPP..PPPPPPPPPPPPPP..PPPPPPP"
    ".PPPPPP..PPPPPPPPPPPPPP..PPPPPP."
    ".PPPPP...PPPPPPPPPPPPPP...PPPPP."
    ".PPPPP...WPPPPPPPPPPPPW...PPPPP."
    ".PPPPP....PPPPP..PPPPP....PPPPP."
    ".WWWWW....PPPPP..PPPPP....WWWWW."
    "WWWWWW....PPPP....PPPP....WWWWWW"
    ".........WWWWW....WWWWW........."
    "................................"};
// Chest pound: both fists on the chest.
const Sprite GORILLA_POUND = {32, 20,
    "............PPPPPPPP............"
    "..........PPPPPPPPPPPP.........."
    ".........PPXXXXXXXXXXPP........."
    ".....PPPPPXxxrXXXXrxxXPPPPP....."
    "...PPPPPPPXXXXXXXXXXXXPPPPPPP..."
    "..PPPSSPPPPXXXrrrrXXXPPPPSSPPP.."
    ".PPPSSSSPPPPXXXXXXXXPPPPSSSSPPP."
    ".PPSSSSPPPPPPPPPPPPPPPPPPSSSSPP."
    "..PPSSSPPPPPPPPPPPPPPPPPSSSPP..."
    "...PPSSPPWWWWPPPPWWWWPPPSSPP...."
    "....PPPPPWWWWWPPWWWWWPPPPP......"
    ".....PPPPWWWWWPPWWWWWPPPP......."
    "......PPPPWWWPPPPWWWPPPP........"
    ".........PPPPPPPPPPPPPP........."
    ".........WPPPPPPPPPPPPW........."
    "..........PPPPP..PPPPP.........."
    "..........PPPPP..PPPPP.........."
    "..........PPPP....PPPP.........."
    ".........WWWWW....WWWWW........."
    "................................"};
// Thrown hazards: magenta marks every thing that kills on touch.
const Sprite BANANA_A = {6, 6,
    "....m."
    "....Ym"
    "...YY."
    "..YYY."
    "mYYY.."
    ".mm..."};
const Sprite BANANA_B = {6, 6,
    ".m...."
    "mY...."
    ".YY..."
    ".YYY.."
    "..YYYm"
    "...mm."};
const Sprite COCONUT = {6, 6,
    ".mmmm."
    "mhhhhm"
    "mhxhxm"
    "mhhhhm"
    "mhhhhm"
    ".mmmm."};
// Harmless while flying or lying: no magenta.
const Sprite PEEL = {8, 6,
    "Y......Y"
    ".Y....Y."
    "..YHHY.."
    "..YHHY.."
    ".YYHHYY."
    "Y..YY..Y"};
const Sprite POOP = {6, 5,
    "...h.."
    "..hhh."
    ".hhxhh"
    "hhhhhh"
    ".hhhh."};

// Glacier formation: a snowflake creature, an ice bat and a frost skull (pale ice on dark blue).
const Sprite FLAKE_A = {12, 10,
    "i....ii....i"
    ".i...ww...i."
    "..iwwiiwwi.."
    "...wiiiiw..."
    ".wwiqiiqiww."
    "iiwiiiiiiwii"
    ".wwiiqqiiww."
    "...wiiiiw..."
    "..iwwiiwwi.."
    ".i...ww...i."};
const Sprite FLAKE_B = {12, 10,
    ".....ii....."
    "..i..ww..i.."
    "...iwiiwi..."
    "i..wiiiiw..i"
    ".wwiqiiqiww."
    "..wiiiiiiw.."
    ".wwiiqqiiww."
    "i..wiiiiw..i"
    "...iwiiwi..."
    "..i..ww..i.."};
const Sprite ICEBAT_A = {12, 10,
    "I..........I"
    "II........II"
    "IqI.i..i.IqI"
    "IqqIiiiiIqqI"
    "IqqiciicIqqI"
    ".IqiiwwiiqI."
    "..IqiiiiqI.."
    "...I.ii.I..."
    ".....ii....."
    "............"};
const Sprite ICEBAT_B = {12, 10,
    "............"
    "............"
    "....i..i...."
    "...IiiiiI..."
    ".IIiciiciII."
    "IqqIiwwiIqqI"
    "IqqqIiiIqqqI"
    "Iqq.I..I.qqI"
    "Iq........qI"
    "I..........I"};
const Sprite FROSTSKULL_A = {12, 10,
    "i..........i"
    "Ii..wwww..iI"
    ".IiwwwwwwiI."
    "..wwwwwwww.."
    ".wwqqwwqqww."
    ".wwqcwwcqww."
    ".wwwwqqwwww."
    "..wwwwwwww.."
    "..wqwqqwqw.."
    "...bbbbbb..."};
const Sprite FROSTSKULL_B = {12, 10,
    ".i........i."
    "iI..wwww..Ii"
    ".IiwwwwwwiI."
    "..wwwwwwww.."
    ".wwqqwwqqww."
    ".wwcqwwqcww."
    ".wwwwqqwwww."
    "..wwwwwwww.."
    "...q.qq.q..."
    "..wqwqqwqw.."};
// Glacier heavy: a little yeti riding a cloud.
const Sprite YETICLOUD_A = {16, 12,
    "......wwww......"
    ".....wwwwww....."
    "....wwIIIIww...."
    "....wIxIIxIw...."
    "....wIIIIIIw...."
    "..w.wwIqqIww.w.."
    "..wwwwwwwwwwww.."
    "...bwwwwwwwwb..."
    ".bbsbbwwwwbbsbb."
    "bbbbbbbbbbbbbbbb"
    ".bsbbsbbbbsbbsb."
    "..ss..ssss..ss.."};
const Sprite YETICLOUD_B = {16, 12,
    "......wwww......"
    ".....wwwwww....."
    "....wwIIIIww...."
    "....wIxIIxIw...."
    "w...wIIIIIIw...w"
    ".w.wwwIqqIwww.w."
    "..wwwwwwwwwwww.."
    "...bwwwwwwwwb..."
    ".bbsbbwwwwbbsbb."
    "bbbbbbbbbbbbbbbb"
    ".bsbbsbbbbsbbsb."
    ".ss..ssss..ss..."};
// Yeti: white fur, blue face, hands and feet.
const Sprite YETI_A = {32, 20,
    "............bwwwwwwb............"
    "..........bwwwwwwwwwwb.........."
    ".........bwwwwwwwwwwwwb........."
    ".........wwIIIIIIIIIIww........."
    ".........wIqqIIIIIIqqIw........."
    ".........wIIxIIIIIIxIIw........."
    ".........wIIIIIIIIIIIIw........."
    ".........wwIqwqwwqwqIww........."
    "..........wIqqqqqqqqIw.........."
    "......bwwwwwIIIIIIIIwwwwwb......"
    ".....bwwwwwwwwwwwwwwwwwwwwb....."
    "....bwwbwwwwwwwwwwwwwwwwbwwb...."
    "...bwwb.bwwwwwwwwwwwwwwb.bwwb..."
    "..IIIb..bwwwwwbwwbwwwwwb..bIII.."
    "..III...bwwwwbwwwwbwwwwb...III.."
    ".........wwwwwwwwwwwwww........."
    ".........bwwwwwbbwwwwwb........."
    "..........wwwwb..bwwww.........."
    ".........IIIII....IIIII........."
    "................................"};
const Sprite YETI_B = {32, 20,
    "............bwwwwwwb............"
    "..........bwwwwwwwwwwb.........."
    ".........bwwwwwwwwwwwwb........."
    ".........wwIIIIIIIIIIww........."
    ".........wIqqIIIIIIqqIw........."
    ".........wIIxIIIIIIxIIw........."
    "...II....wIIIIIIIIIIIIw....II..."
    "..III....wwIqwqwwqwqIww....III.."
    "...bw.....wIqqqqqqqqIw.....wb..."
    "....bwwwwwwwIIIIIIIIwwwwwwwb...."
    ".....bwwwwwwwwwwwwwwwwwwwwb....."
    "........bwwwwwwwwwwwwwwb........"
    "........bwwwwwwwwwwwwwwb........"
    "........bwwwwwbwwbwwwwwb........"
    "........bwwwwbwwwwbwwwwb........"
    ".........wwwwwwwwwwwwww........."
    ".........bwwwwwbbwwwwwb........."
    "..........wwwwb..bwwww.........."
    ".........IIIII....IIIII........."
    "................................"};
// Sneeze: eyes squeezed shut, mouth wide open.
const Sprite YETI_SNEEZE = {32, 20,
    "............bwwwwwwb............"
    "..........bwwwwwwwwwwb.........."
    ".........bwwwwwwwwwwwwb........."
    ".........wwIIIIIIIIIIww........."
    ".........wIIIIIIIIIIIIw........."
    ".........wIqqIIIIIIqqIw........."
    ".........wIIIIIIIIIIIIw........."
    ".........wIIqqqqqqqqIIw........."
    "..........wIqrrrrrrqIw.........."
    "......bwwwwwIIIIIIIIwwwwwb......"
    ".....bwwwwwwwwwwwwwwwwwwwwb....."
    "....bwwbwwwwwwwwwwwwwwwwbwwb...."
    "...bwwb.bwwwwwwwwwwwwwwb.bwwb..."
    "..IIIb..bwwwwwbwwbwwwwwb..bIII.."
    "..III...bwwwwbwwwwbwwwwb...III.."
    ".........wwwwwwwwwwwwww........."
    ".........bwwwwwbbwwwwwb........."
    "..........wwwwb..bwwww.........."
    ".........IIIII....IIIII........."
    "................................"};
// Ice golem: blocks of ice with glowing cyan eyes.
const Sprite GOLEM_A = {32, 20,
    "..........qqqqqqqqqqqq.........."
    ".........qIIIIIIIIIIIIq........."
    ".........qIiiiiiiiiiiIq........."
    ".........qIicciiiicciIq........."
    ".........qIiiiiiiiiiiIq........."
    ".........qIIqqqqqqqqIIq........."
    "....qqqq.qqIIIIIIIIIIqq.qqqq...."
    "...qIIIIqqIiiiiIIiiiiIqqIIIIq..."
    "...qIiiIqIiiwiiIIiiwiiIqIiiIq..."
    "...qIiiIqIiiiiiIIiiiiiIqIiiIq..."
    "...qIIIIqIiiiiiIIiiiiiIqIIIIq..."
    "....qIIqqIIIIIIIIIIIIIIqqIIq...."
    "....qiiq.qIiiiiIIiiiiIq.qiiq...."
    "....qiiq.qIiiiiIIiiiiIq.qiiq...."
    "....qIIq.qIIIIIIIIIIIIq.qIIq...."
    "........qIiiiq....qiiiIq........"
    "........qIiiiq....qiiiIq........"
    "........qIIIIq....qIIIIq........"
    ".......qqqqqqq....qqqqqqq......."
    "................................"};
const Sprite GOLEM_B = {32, 20,
    "..........qqqqqqqqqqqq.........."
    ".........qIIIIIIIIIIIIq........."
    ".........qIiiiiiiiiiiIq........."
    ".........qIicciiiicciIq........."
    "....qqqq.qIiiiiiiiiiiIq.qqqq...."
    "...qIIIIqqIIqqqqqqqqIIqqIIIIq..."
    "...qIiiIqqqIIIIIIIIIIqqqIiiIq..."
    "...qIiiIqqIiiiiIIiiiiIqqIiiIq..."
    "...qIIIIqIiiwiiIIiiwiiIqIIIIq..."
    "....qIIqqIiiiiiIIiiiiiIqqIIq...."
    "....qiiq.qIiiiiIIiiiiIq.qiiq...."
    "....qiiq.qIIIIIIIIIIIIq.qiiq...."
    "....qIIq.qIiiiiIIiiiiIq.qIIq...."
    ".........qIiiiiIIiiiiIq........."
    ".........qIIIIIIIIIIIIq........."
    "........qIiiiq....qiiiIq........"
    "........qIiiiq....qiiiIq........"
    "........qIIIIq....qIIIIq........"
    ".......qqqqqqq....qqqqqqq......."
    "................................"};
// Volcano formation: a living ember, a fire bat and a magma skull.
const Sprite EMBER_A = {12, 10,
    ".....O......"
    "....OO...O.."
    "...OOO..OO.."
    "..OONOOOOO.."
    ".OONNNNNOOO."
    ".ONxNNNxNOO."
    "ZONNNNNNNNOZ"
    "ZOONNxxNNOOZ"
    ".ZOONNNNOOZ."
    "..ZZOOOOZZ.."};
const Sprite EMBER_B = {12, 10,
    "......O....."
    "..O..OO....."
    "..OO.OOO...."
    "..OOOOONOO.."
    ".OONNNNNOOO."
    ".ONxNNNxNOO."
    "ZONNNNNNNNOZ"
    "ZOONNxxNNOOZ"
    ".ZOONNNNOOZ."
    "..ZZOOOOZZ.."};
const Sprite FIREBAT_A = {12, 10,
    "Z..........Z"
    "ZZ........ZZ"
    "ZOZ.x..x.ZOZ"
    "ZOOZxxxxZOOZ"
    "ZOOxNxxNZOOZ"
    ".ZOxxwwxxOZ."
    "..ZOxxxxOZ.."
    "...Z.xx.Z..."
    ".....xx....."
    "............"};
const Sprite FIREBAT_B = {12, 10,
    "............"
    "............"
    "....x..x...."
    "...ZxxxxZ..."
    ".ZZxNxxNxZZ."
    "ZOOZxwwxZOOZ"
    "ZOOOZxxZOOOZ"
    "ZOO.Z..Z.OOZ"
    "ZO........OZ"
    "Z..........Z"};
const Sprite MAGMASKULL_A = {12, 10,
    "O..........O"
    "OO..ZZZZ..OO"
    ".OOZZZZZZOO."
    "..ZZZZZZZZ.."
    ".ZZxxZZxxZZ."
    ".ZZxNZZNxZZ."
    ".ZZZZxxZZZZ."
    "..ZZZZZZZZ.."
    "..ZxZxxZxZ.."
    "...RRRRRR..."};
const Sprite MAGMASKULL_B = {12, 10,
    ".O........O."
    "OO..ZZZZ..OO"
    ".OOZZZZZZOO."
    "..ZZZZZZZZ.."
    ".ZZxxZZxxZZ."
    ".ZZNxZZxNZZ."
    ".ZZZZxxZZZZ."
    "..ZZZZZZZZ.."
    "...x.xx.x..."
    "..ZxZxxZxZ.."};
// Volcano heavy: a phoenix.
const Sprite PHOENIX_A = {16, 12,
    "O..............O"
    "OO............OO"
    "ZOO..........OOZ"
    ".ZOO..NNNN..OOZ."
    "..ZOONxNNxNOOZ.."
    "...ZONNNNNNOZ..."
    "....ZNNNNNNZ...."
    ".....ZNYYNZ....."
    "....ZOONNOOZ...."
    "...ZO.ONNO.OZ..."
    "..ZO..ZOOZ..OZ.."
    ".O.....ZZ.....O."};
const Sprite PHOENIX_B = {16, 12,
    "................"
    "................"
    "................"
    "......NNNN......"
    ".....NxNNxN....."
    "OOZZZONNNNOZZZOO"
    "ZOOOZNNNNNNZOOOZ"
    ".ZOOZZNYYNZZOOZ."
    "..ZZ.OONNOO.ZZ.."
    "....ZO.NN.OZ...."
    "...ZO.ZOOZ.OZ..."
    "..O....ZZ....O.."};
// Fire imp: flaming hair, little horns, bat wings and a grin.
const Sprite IMP_A = {32, 20,
    "..............O..O.............."
    "...........O.OO..OO.O..........."
    "..........OONOOOOOONOO.........."
    "..........ONNNONNONNNO.........."
    "....N.....ZZZZZZZZZZZZ.....N...."
    ".....N...ZZZZZZZZZZZZZZ...N....."
    ".....NZ..ZNxZZZZZZZZxNZ..ZN....."
    "......ZZ.ZNNZZZZZZZZNNZ.ZZ......"
    "..RR.....ZZxwxwxxwxwxZZ.....RR.."
    ".RRRR.....ZZxxxxxxxxZZ.....RRRR."
    "RROORR...ZZZZZZZZZZZZZZ...RROORR"
    "ROOOORRZZZZZOOOOOOOOZZZZZRROOOOR"
    ".RROORR.ZZZOONNNNNNOOZZZ.RROORR."
    "..RRRR..ZZZOONNNNNNOOZZZ..RRRR.."
    "...RR...ZZZZOOOOOOOOZZZZ...RR..."
    ".........ZZZZZZZZZZZZZZ........."
    ".........ZZZ........ZZZ........."
    "........ZZZ..........ZZZ........"
    ".......RRR............RRR......."
    "................................"};
const Sprite IMP_B = {32, 20,
    "............O......O............"
    "..........O.OO.OO.OO.O.........."
    "..........OONOOOOOONOO.........."
    "..........ONNNONNONNNO.........."
    "....N.....ZZZZZZZZZZZZ.....N...."
    "RR...N...ZZZZZZZZZZZZZZ...N...RR"
    "ROR..NZ..ZNxZZZZZZZZxNZ..ZN..ROR"
    "ROOR..ZZ.ZNNZZZZZZZZNNZ.ZZ..ROOR"
    ".ROOR....ZZxwxwxxwxwxZZ....ROOR."
    "..RRR.....ZZxxxxxxxxZZ.....RRR.."
    "....R....ZZZZZZZZZZZZZZ....R...."
    ".....RRZZZZZOOOOOOOOZZZZZRR....."
    "........ZZZOONNNNNNOOZZZ........"
    "........ZZZOONNNNNNOOZZZ........"
    "........ZZZZOOOOOOOOZZZZ........"
    ".........ZZZZZZZZZZZZZZ........."
    "..........ZZZ......ZZZ.........."
    "..........ZZZ......ZZZ.........."
    ".........RRR........RRR........."
    "................................"};
// Fire lord: horned head in a mane of flame, molten core, magma fists.
const Sprite FIRELORD_A = {32, 20,
    "...H......O..........O......H..."
    "...HH....OO..O....O..OO....HH..."
    "....HH..OONO.OO..OO.ONOO..HH...."
    ".....HHOONNOOONOONOOONNOOHH....."
    "......HZZZZZZZZZZZZZZZZZZH......"
    ".....OZZRRZZZZZZZZZZZZRRZZO....."
    "....OOZRNNNZZZZZZZZZZNNNRZOO...."
    "....OZZZRRZZZZZZZZZZZZRRZZZO...."
    "...OOZZZZZxxxxxxxxxxxxZZZZZOO..."
    "...OZZZZZZxNxNxNNxNxNxZZZZZZO..."
    "..OOZZZZZZZxxxxxxxxxxZZZZZZZOO.."
    ".ZZZZRRZZZZZZZZZZZZZZZZZZRRZZZZ."
    "ZZRRZZRRZZZOZZZZZZZZOZZZRRZZRRZZ"
    "ZRR..ZRRZZZOOZZZZZZOOZZZRRZ..RRZ"
    "OO...ZRRZZZONOZZZZONOZZZRRZ...OO"
    "ONO...ZRZZZONNOZZONNOZZZRZ...ONO"
    "OO.....ZRZZZOOZZZZOOZZZRZ.....OO"
    ".......ZZRZZZZZZZZZZZZRZZ......."
    "........ZZRRZZZZZZZZRRZZ........"
    ".........ZZZZZZZZZZZZZZ........."};
const Sprite FIRELORD_B = {32, 20,
    "...H.....O............O.....H..."
    "...HH...OO...O....O...OO...HH..."
    "....HH.OONO..OO..OO..ONOO.HH...."
    ".....HHONNNOONOOOONOONNNOHH....."
    "......HZZZZZZZZZZZZZZZZZZH......"
    ".....OZZRRZZZZZZZZZZZZRRZZO....."
    "....OOZRNNNZZZZZZZZZZNNNRZOO...."
    "....OZZZRRZZZZZZZZZZZZRRZZZO...."
    "...OOZZZZZxxxxxxxxxxxxZZZZZOO..."
    "...OZZZZZZxNxNxNNxNxNxZZZZZZO..."
    "..OOZZZZZZZxxxxxxxxxxZZZZZZZOO.."
    ".ZZZZRRZZZZZZZZZZZZZZZZZZRRZZZZ."
    "ZZRRZZRRZZZOZZZZZZZZOZZZRRZZRRZZ"
    "ZRR..ZRRZZZOOZZZZZZOOZZZRRZ..RRZ"
    "NO...ZRRZZZONOZZZZONOZZZRRZ...ON"
    "OOO...ZRZZZONNOZZONNOZZZRZ...OOO"
    ".O.....ZRZZZOOZZZZOOZZZRZ.....O."
    ".......ZZRZZZZZZZZZZZZRZZ......."
    "........ZZRRZZZZZZZZRRZZ........"
    ".........ZZZZZZZZZZZZZZ........."};
// Deep-sea formation: a piranha, a pufferfish and a lionfish.
const Sprite PIRANHA_A = {12, 10,
    ".....XXX...."
    "...XXXXXX..X"
    "..XbbbbbbXXX"
    ".XbxYbbbbbXX"
    "X8bbbbbbbbX."
    ".8.8bbbbbbX."
    "X8rrrrrbbbXX"
    ".XrrrrrbbXXX"
    "...XXXXXX..X"
    ".....XX....."};
const Sprite PIRANHA_B = {12, 10,
    ".....XXX...."
    "...XXXXXX..."
    "..XbbbbbbX.X"
    ".XbxYbbbbbXX"
    "X8bbbbbbbbXX"
    "X8888bbbbbX."
    "XrrrrrrbbbX."
    ".XrrrrrbbXXX"
    "...XXXXXX.XX"
    ".....XX....X"};
const Sprite PUFFER_A = {12, 10,
    "...h....h..."
    ".h.YYYYYY.h."
    "..YYYYYYYY.."
    "hYxYYYYYYxYh"
    ".YYYYYYYYYY."
    "hYYYYYYYYYYh"
    ".YoYoYYoYoY."
    "hYYYYrrYYYYh"
    "..YYYYYYYY.."
    "...h....h..."};
const Sprite PUFFER_B = {12, 10,
    "............"
    "h..YYYYYY..h"
    ".YYYYYYYYYY."
    ".YxYYYYYYxY."
    "hYYYYYYYYYYh"
    ".YYYYYYYYYY."
    "hYoYoYYoYoYh"
    ".YYYYrrYYYY."
    "h.YYYYYYYY.h"
    "............"};
const Sprite LIONFISH_A = {12, 10,
    ".R..R..R..R."
    "..R.R.R..R.."
    "...rwrwrwr.."
    ".rwYxwrwrwrR"
    "rwrwrwrwrwRR"
    ".rwrwrwrwr.R"
    "..R.R..R...."
    ".R..R...R..."
    "R...R....R.."
    "............"};
const Sprite LIONFISH_B = {12, 10,
    "R...R..R...R"
    ".R..R.R..R.."
    "...rwrwrwr.."
    ".rwYxwrwrwr."
    "rwrwrwrwrwRR"
    ".rwrwrwrwrRR"
    "..R..R..R..."
    "..R..R...R.."
    ".R...R...R.."
    "............"};
// Deep-sea heavy: a shark.
const Sprite SHARK_A = {16, 12,
    "........s......."
    ".......ss......."
    "......sss......."
    "...sssssssss...s"
    ".ssbbbbbbbbbs.ss"
    "sbbxbbbbbbbbbsss"
    "sbbbbbbbbbbbbbs."
    "s8888wwwwwwbbsss"
    ".swwwwwwwwwbbs.s"
    "..sswwwwwbss...s"
    "....sss.ss......"
    "........s......."};
const Sprite SHARK_B = {16, 12,
    "........s......."
    ".......ss......."
    "......sss......."
    "...sssssssss...."
    ".ssbbbbbbbbbs..s"
    "sbbxbbbbbbbbbsss"
    "sbbbbbbbbbbbbbss"
    "s8.8.wwwwwwbbss."
    "s8888wwwwwwbbs.."
    "..sswwwwwbss..s."
    "....sss.ss.....s"
    "........s......."};
// Anglerfish: a gaping mouth of fangs, red eyes and a glowing lure that sways.
const Sprite ANGLER_A = {32, 20,
    ".....................699........"
    "...........6666666666.99........"
    "........6667777777777666........"
    "......66777777777777777766......"
    ".....6777777777777777777776....."
    "....677wr77777777777777rw776...."
    "...6777ww77777777777777ww7776..."
    "..6777777777777777777777777776.."
    ".677777777777777777777777777776."
    "6777xxxxxxxxxxxxxxxxxxxxxxxx7776"
    "677x8x8x8x8x8x8xx8x8x8x8x8x8x776"
    "67xxx8xxx8xxx8xxxx8xxx8xxx8xxx76"
    "67xxxxxxxxxrrrrrrrrrrxxxxxxxxx76"
    "67xx8xxx8xxx8xxxxxx8xxx8xxx8xx76"
    "677x8x8x8x8x8x8xx8x8x8x8x8x8x776"
    ".677777777777777777777777777776."
    "..6677777777777777777777777766.."
    "....666777777777777777777666...."
    ".......666666666666666666......."
    "................................"};
const Sprite ANGLER_B = {32, 20,
    "........996....................."
    "........99.6666666666..........."
    "........6667777777777666........"
    "......66777777777777777766......"
    ".....6777777777777777777776....."
    "....677wr77777777777777rw776...."
    "...6777ww77777777777777ww7776..."
    "..6777777777777777777777777776.."
    ".677777777777777777777777777776."
    "6777xxxxxxxxxxxxxxxxxxxxxxxx7776"
    "677x8x8x8x8x8x8xx8x8x8x8x8x8x776"
    "67xxxxxxxxxxxxxxxxxxxxxxxxxxxx76"
    "67xxxxxxxxxrrrrrrrrrrxxxxxxxxx76"
    "67x8x8x8x8x8x8x88x8x8x8x8x8x8x76"
    "677x8x8x8x8x8x8xx8x8x8x8x8x8x776"
    ".677777777777777777777777777776."
    "..6677777777777777777777777766.."
    "....666777777777777777777666...."
    ".......666666666666666666......."
    "................................"};
// Fisherman fish: a fish in a yellow fisherman's hat with a rod (tip at 31, 0) and a fish bucket (centre 16, 17).
const Sprite FISHERMAN_A = {32, 20,
    "...............................h"
    "...................YYYYYY......h"
    "..................YYYYYYYY....h."
    "77.........77.....YYYYYYYY....h."
    "766.......7777.oooooooooooooo.h."
    ".7666....777777777777777777...h."
    "..766667777777777777777777777.h."
    "...766777777777777777777ww7777h."
    "....76766666666666666766wx6667h."
    ".....7766666666666666766666667h."
    ".....77666666666666667666666rr7."
    ".....776666666666666676666666r7."
    "....76788888888888888888888877.."
    "...76667788888888888888888777..."
    "..7666...77788888888888777......"
    ".7666.......77777O77777........."
    "766.........7bdOOddb............"
    "77...........bsssssb............"
    ".............bsssssb............"
    "..............bsssb............."};
const Sprite FISHERMAN_B = {32, 20,
    "...............................h"
    "...................YYYYYY......h"
    "..................YYYYYYYY....h."
    "...........77.....YYYYYYYY....h."
    "77........7777.oooooooooooooo.h."
    "766......777777777777777777...h."
    ".7666..7777777777777777777777.h."
    "..7766777777777777777777ww7777h."
    "....76766666666666666766wx6667h."
    ".....7766666666666666766666667h."
    ".....77666666666666667666666rr7."
    ".....776666666666666676666666r7."
    "....76788888888888888888888877.."
    "..766667788888888888888888777..."
    ".7666....77788888888888777......"
    "766.........777O7777777........."
    "77..........7bddOOdb............"
    ".............bsssssb............"
    ".............bsssssb............"
    "..............bsssb............."};
// Glacier, volcano and deep-sea hazards: magenta marks the deadly ones.
const Sprite SNOWBALL = {6, 6,
    ".mmmm."
    "mwwwwm"
    "mwwwwm"
    "mwwwbm"
    "mwbbbm"
    ".mmmm."};
const Sprite ICICLE = {4, 9,
    "IiiI"
    "wiiI"
    "wiiI"
    ".iI."
    ".iI."
    ".iI."
    ".mm."
    ".mm."
    "..m."};
const Sprite FIREBALL_A = {6, 6,
    "..m..."
    ".mOm.."
    ".mOOm."
    "mONNOm"
    "mONNOm"
    ".mmmm."};
const Sprite FIREBALL_B = {6, 6,
    "...m.."
    "..mOm."
    ".mOOm."
    "mONNOm"
    "mONNOm"
    ".mmmm."};
const Sprite BUBBLE = {5, 5,
    ".mmm."
    "mw..m"
    "m...m"
    "m...m"
    ".mmm."};
const Sprite BIG_BUBBLE = {9, 9,
    "..mmmmm.."
    ".m.....m."
    "mww.....m"
    "mw......m"
    "m.......m"
    "m.......m"
    "m......cm"
    ".m....cm."
    "..mmmmm.."};
const Sprite BOOT = {7, 6,
    "..hhh.."
    "..hmh.."
    "..hmh.."
    "..hhhh."
    "hhhhhhh"
    "xxxxxxx"};
// Harmful but not deadly: no magenta.
const Sprite ICE_BLOCK = {24, 24,
    "qIIIIIIIIIIIIIIIIIIIIIIq"
    "IiiiiiiiiiiiiiiiiiiiiiiI"
    "Iiw~~~~~~~~~~~~~~~~~~~iI"
    "Iw~~w~~~~~~~~~~~~~~~~~qI"
    "Ii~w~~~~~~~~~~~~~~~~~~qI"
    "I~~~~~~~~~~~~~~~~~~~~~qI"
    "I~~~~~~~~~~~~~~~~~~~~~qI"
    "I~~~~~~~~~~~~~~~~~~~~~qI"
    "I~~~~~~~~~~~~~~~~w~~~~qI"
    "I~~~~~~~~~~~~~~~w~~~~~qI"
    "I~~~~~~~~~~~~~~w~~~~~~qI"
    "I~~~~~~~~~~~~~~~~~~~~~qI"
    "I~~~~~~~~~~~~~~~~~~~~~qI"
    "I~~~~~~~~~~~~~~~~~~~~~qI"
    "I~~~~~w~~~~~~~~~~~~~~~qI"
    "I~~~~w~~~~~~~~~~~~~~~~qI"
    "I~~~~~~~~~~~~~~~~~~~~~qI"
    "I~~~~~~~~~~~~~~~~~~~~~qI"
    "I~~~~~~~~~~~~~~~~~~~~~qI"
    "I~~~~~~~~~~~~~~~~~~~~~qI"
    "I~~~~~~~~~~~~~~~~~~~~~qI"
    "I~~~~~~~~~~~~~~~~~~~~wqI"
    "IqqqqqqqqqqqqqqqqqqqqqqI"
    "qIIIIIIIIIIIIIIIIIIIIIIq"};
const Sprite ICE_CUBE = {18, 16,
    "qIIIIIIIIIIIIIIIIq"
    "Iiw~~~~~~~~~~~~~iI"
    "Iw~~~~~~~~~~~~~~~I"
    "I~~~~~~~~~~~w~~~~I"
    "I~~~~~~~~~~w~~~~~I"
    "I~~~~~w~~~~~~~~~~I"
    "I~~~~w~~~~~~~~~~~I"
    "I~~~~~~~~~~~~~~~~I"
    "I~~~~~~~~~~~w~~~~I"
    "I~~~~~~~~~~w~~~~~I"
    "I~~~~~~~~~~~~~~~~I"
    "I~~~w~~~~~~~~~~~~I"
    "I~~w~~~~~~~~~~~~~I"
    "I~~~~~~~~~~~~~~~wI"
    "Ii~~~~~~~~~~~~~wiI"
    "qIIIIIIIIIIIIIIIIq"};
const Sprite LAVA_DROP = {5, 6,
    "..O.."
    "..O.."
    ".OOO."
    "ONNOO"
    "OZOOO"
    ".ZZZ."};
const Sprite HOOK = {5, 6,
    "..bb."
    "...b."
    "...b."
    "b..b."
    "bb.b."
    ".bb.."};

// The imp's big lava drop: scorches, doesn't kill.
const Sprite BIG_LAVA_DROP = {10, 12,
    "....OO...."
    "....OO...."
    "...OOOO..."
    "...ONOO..."
    "..OONOOO.."
    ".OONNOOOO."
    ".ONNNOOOO."
    "OONNOOOOOO"
    "OONOOOOOZO"
    "OOOOOOOZZO"
    ".ZOOOZZZZ."
    "..ZZZZZZ.."};

// Live fish from the fisherman's bucket: magenta fins mean it kills on touch; L swims left, R right.
const Sprite LIVE_FISH_L_A = {9, 6,
    ".OOOO.mm."
    "OwOOOOmmm"
    "OxOONOOm."
    "ZOONNOOm."
    ".OOOOOmmm"
    "..ZZZ.mm."};

const Sprite LIVE_FISH_L_B = {9, 6,
    ".OOOO.m.."
    "OwOOOOmm."
    "OxOONOOmm"
    "ZOONNOOmm"
    ".OOOOOmm."
    "..ZZZ.m.."};

const Sprite LIVE_FISH_R_A = {9, 6,
    ".mm.OOOO."
    "mmmOOOOwO"
    ".mOONOOxO"
    ".mOONNOOZ"
    "mmmOOOOO."
    ".mm.ZZZ.."};

const Sprite LIVE_FISH_R_B = {9, 6,
    "..m.OOOO."
    ".mmOOOOwO"
    "mmOONOOxO"
    "mmOONNOOZ"
    ".mmOOOOO."
    "..m.ZZZ.."};

// Ship skins per level (same 16x14 body and hitbox): jungle wood and leaves, glacier ice, volcano obsidian, deep-sea submarine.
const Sprite SHIP_JUNGLE_A = {16, 14,
    ".......cc......."
    "......cwwc......"
    "......hwwh......"
    ".....hHHHHh....."
    ".....hBhhBh....."
    "..l..hBhhBh..l.."
    ".llg.hBhhBh.gll."
    "lggghhBccBhhgggl"
    ".lgghHHccHHhggl."
    "..lghhhhhhhhgl.."
    "...hhhhhhhhhh..."
    "....hh....hh...."
    "....oo....oo...."
    ".....o....o....."};
const Sprite SHIP_JUNGLE_B = {16, 14,
    ".......cc......."
    "......cwwc......"
    "......hwwh......"
    ".....hHHHHh....."
    ".....hBhhBh....."
    "..l..hBhhBh..l.."
    ".llg.hBhhBh.gll."
    "lggghhBccBhhgggl"
    ".lgghHHccHHhggl."
    "..lghhhhhhhhgl.."
    "...hhhhhhhhhh..."
    "....hh....hh...."
    "....yo....oy...."
    "................"};
const Sprite SHIP_GLACIER_A = {16, 14,
    ".......cc......."
    "......cwwc......"
    "......cwwc......"
    ".....iIwwIi....."
    "....iIIIIIIi...."
    ".i..IIqIIqII..i."
    ".iI.IqIIIIqI.Ii."
    "iIIiIIIccIIIiIIi"
    ".IIIIIIccIIIIII."
    "..qIIIIIIIIIIq.."
    "...qIiIIIIiIq..."
    "....qq....qq...."
    "....oo....oo...."
    ".....o....o....."};
const Sprite SHIP_GLACIER_B = {16, 14,
    ".......cc......."
    "......cwwc......"
    "......cwwc......"
    ".....iIwwIi....."
    "....iIIIIIIi...."
    ".i..IIqIIqII..i."
    ".iI.IqIIIIqI.Ii."
    "iIIiIIIccIIIiIIi"
    ".IIIIIIccIIIIII."
    "..qIIIIIIIIIIq.."
    "...qIiIIIIiIq..."
    "....qq....qq...."
    "....yo....oy...."
    "................"};
const Sprite SHIP_VOLCANO_A = {16, 14,
    ".......cc......."
    "......cwwc......"
    "......cwwc......"
    ".....XXwwXX....."
    ".....XOXXOX....."
    "..N..XXOOXX..N.."
    "..O.XXXXXXXX.O.."
    ".OX.XXXccXXX.XO."
    ".XXXOXXccXXOXXX."
    "XXXXXOXXXXOXXXXX"
    "XXXOXXXXXXXXOXXX"
    ".XX.OO....OO.XX."
    "....NO....ON...."
    ".....O....O....."};
const Sprite SHIP_VOLCANO_B = {16, 14,
    ".......cc......."
    "......cwwc......"
    "......cwwc......"
    ".....XXwwXX....."
    ".....XOXXOX....."
    "..N..XXOOXX..N.."
    "..O.XXXXXXXX.O.."
    ".OX.XXXccXXX.XO."
    ".XXXOXXccXXOXXX."
    "XXXXXOXXXXOXXXXX"
    "XXXOXXXXXXXXOXXX"
    ".XX.OO....OO.XX."
    "....ON....NO...."
    "................"};
const Sprite SHIP_SEA_A = {16, 14,
    ".......cc......."
    "......cwwc......"
    "......bccb......"
    "....bbbbbbbb...."
    "...bbNbbbbNbb..."
    "..bbNwNbbNwNbb.."
    "..bbbNbbbbNbbb.."
    ".IbbbbbccbbbbbI."
    "IIbbbbbccbbbbbII"
    ".IbbbbbbbbbbbbI."
    "..bbbbbbbbbbbb.."
    "....bb....bb...."
    "....ii....ii...."
    ".....i....i....."};
const Sprite SHIP_SEA_B = {16, 14,
    ".......cc......."
    "......cwwc......"
    "......bccb......"
    "....bbbbbbbb...."
    "...bbNbbbbNbb..."
    "..bbNwNbbNwNbb.."
    "..bbbNbbbbNbbb.."
    ".IbbbbbccbbbbbI."
    "IIbbbbbccbbbbbII"
    ".IbbbbbbbbbbbbI."
    "..bbbbbbbbbbbb.."
    "....bb....bb...."
    ".....i....i....."
    "....i......i...."};
// Player 2's skins: the hull gold and the cockpit orange, as SHIP2_A.
const Sprite SHIP2_JUNGLE_A = {16, 14,
    ".......oo......."
    "......owwo......"
    "......YwwY......"
    ".....YHHHHY....."
    ".....YBYYBY....."
    "..l..YBYYBY..l.."
    ".llg.YBYYBY.gll."
    "lgggYYBooBYYgggl"
    ".lggYHHooHHYggl."
    "..lgYYYYYYYYgl.."
    "...YYYYYYYYYY..."
    "....YY....YY...."
    "....oo....oo...."
    ".....o....o....."};
const Sprite SHIP2_JUNGLE_B = {16, 14,
    ".......oo......."
    "......owwo......"
    "......YwwY......"
    ".....YHHHHY....."
    ".....YBYYBY....."
    "..l..YBYYBY..l.."
    ".llg.YBYYBY.gll."
    "lgggYYBooBYYgggl"
    ".lggYHHooHHYggl."
    "..lgYYYYYYYYgl.."
    "...YYYYYYYYYY..."
    "....YY....YY...."
    "....yo....oy...."
    "................"};
const Sprite SHIP2_GLACIER_A = {16, 14,
    ".......oo......."
    "......owwo......"
    "......owwo......"
    ".....iYwwYi....."
    "....iYYYYYYi...."
    ".i..YYqYYqYY..i."
    ".iY.YqYYYYqY.Yi."
    "iYYiYYYooYYYiYYi"
    ".YYYYYYooYYYYYY."
    "..qYYYYYYYYYYq.."
    "...qYiYYYYiYq..."
    "....qq....qq...."
    "....oo....oo...."
    ".....o....o....."};
const Sprite SHIP2_GLACIER_B = {16, 14,
    ".......oo......."
    "......owwo......"
    "......owwo......"
    ".....iYwwYi....."
    "....iYYYYYYi...."
    ".i..YYqYYqYY..i."
    ".iY.YqYYYYqY.Yi."
    "iYYiYYYooYYYiYYi"
    ".YYYYYYooYYYYYY."
    "..qYYYYYYYYYYq.."
    "...qYiYYYYiYq..."
    "....qq....qq...."
    "....yo....oy...."
    "................"};
const Sprite SHIP2_VOLCANO_A = {16, 14,
    ".......oo......."
    "......owwo......"
    "......owwo......"
    ".....YYwwYY....."
    ".....YOYYOY....."
    "..N..YYOOYY..N.."
    "..O.YYYYYYYY.O.."
    ".OY.YYYooYYY.YO."
    ".YYYOYYooYYOYYY."
    "YYYYYOYYYYOYYYYY"
    "YYYOYYYYYYYYOYYY"
    ".YY.OO....OO.YY."
    "....NO....ON...."
    ".....O....O....."};
const Sprite SHIP2_VOLCANO_B = {16, 14,
    ".......oo......."
    "......owwo......"
    "......owwo......"
    ".....YYwwYY....."
    ".....YOYYOY....."
    "..N..YYOOYY..N.."
    "..O.YYYYYYYY.O.."
    ".OY.YYYooYYY.YO."
    ".YYYOYYooYYOYYY."
    "YYYYYOYYYYOYYYYY"
    "YYYOYYYYYYYYOYYY"
    ".YY.OO....OO.YY."
    "....ON....NO...."
    "................"};
const Sprite SHIP2_SEA_A = {16, 14,
    ".......oo......."
    "......owwo......"
    "......YooY......"
    "....YYYYYYYY...."
    "...YYNYYYYNYY..."
    "..YYNwNYYNwNYY.."
    "..YYYNYYYYNYYY.."
    ".IYYYYYooYYYYYI."
    "IIYYYYYooYYYYYII"
    ".IYYYYYYYYYYYYI."
    "..YYYYYYYYYYYY.."
    "....YY....YY...."
    "....ii....ii...."
    ".....i....i....."};
const Sprite SHIP2_SEA_B = {16, 14,
    ".......oo......."
    "......owwo......"
    "......YooY......"
    "....YYYYYYYY...."
    "...YYNYYYYNYY..."
    "..YYNwNYYNwNYY.."
    "..YYYNYYYYNYYY.."
    ".IYYYYYooYYYYYI."
    "IIYYYYYooYYYYYII"
    ".IYYYYYYYYYYYYI."
    "..YYYYYYYYYYYY.."
    "....YY....YY...."
    ".....i....i....."
    "....i......i...."};

// Level buffs: the wand on a ship with the space buff (star centre at 1, 1), banana and fire bullets, shield bubbles.
const Sprite WAND = {3, 7,
    ".N."
    "NwN"
    ".N."
    ".h."
    ".h."
    ".h."
    ".h."};
// Wands held by the ship for the armed strong attack: ice wand (glacier) and bubble stick (deep sea), same 3x7 box as WAND.
const Sprite WAND_ICE = {3, 7,
    ".i."
    "iwi"
    ".i."
    ".h."
    ".h."
    ".h."
    ".h."};
const Sprite WAND_BUBBLE = {3, 7,
    "iii"
    "i.i"
    "iii"
    ".h."
    ".h."
    ".h."
    ".h."};
const Sprite PLAYER_BANANA = {3, 5,
    ".h."
    "..Y"
    "..Y"
    ".YY"
    "YY."};
const Sprite FIRE_BANANA = {3, 5,
    ".N."
    "N.Y"
    "O.Y"
    ".YY"
    "YYO"};
const Sprite FIRE_BULLET = {2, 4,
    "NN"
    "NO"
    "OO"
    "OZ"};
const Sprite SHIELD_BUBBLE_3 = {8, 8,
    "..iiii.."
    ".i....i."
    "iww....i"
    "iw.....i"
    "i......i"
    "i......i"
    ".i....i."
    "..iiii.."};
const Sprite SHIELD_BUBBLE_2 = {4, 4,
    ".ii."
    "iw.i"
    "i..i"
    ".ii."};
const Sprite SHIELD_BUBBLE_1 = {2, 2,
    "wi"
    "ii"};
// Level buff icons (HUD and pickup), by level: wand star, banana, snowflake, flame, bubble.
const Sprite BUFF_ICON_SPACE = {5, 5,
    "..N.."
    "NNwNN"
    ".NwN."
    ".N.N."
    "N...N"};
const Sprite BUFF_ICON_JUNGLE = {5, 5,
    "....h"
    "...Y."
    "..YY."
    "YYY.."
    ".Y..."};
const Sprite BUFF_ICON_GLACIER = {5, 5,
    "i.i.i"
    ".iii."
    "iiwii"
    ".iii."
    "i.i.i"};
const Sprite BUFF_ICON_VOLCANO = {5, 5,
    "..O.."
    ".OO.."
    ".ONO."
    "ONNNO"
    ".OOO."};
const Sprite BUFF_ICON_SEA = {5, 5,
    ".iii."
    "iw..i"
    "i...i"
    "i...i"
    ".iii."};

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
