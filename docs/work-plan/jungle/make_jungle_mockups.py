"""Draws the jungle plan's mockups into docs/work-plan/jungle/: palette, sprite sheet and three game screens.

The new palette, sprites and the background generator below are the same ones the plan tells the implementer to add
to source/ (docs/work-plan/jungle-level-plan.md), so the pictures show the intended result. Existing sprites, palette
and font are read from source/sprites.h. Needs Pillow. Run: python3 docs/work-plan/jungle/make_jungle_mockups.py
"""
import math
import os
import re

from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
W, H = 320, 180

with open(os.path.join(ROOT, "source", "sprites.h")) as f:
    SOURCE = f.read()
PALETTE = {c: int(v, 16) & 0xFFFFFF for c, v in re.findall(r"case '(.)': return 0x([0-9A-Fa-f]{8});", SOURCE)}
SPRITES = {}
for name, w, h, body in re.findall(r'const Sprite (\w+) = \{(\d+), (\d+),((?:\s*"[^"]*")+)\};', SOURCE):
    SPRITES[name] = (int(w), int(h), "".join(re.findall(r'"([^"]*)"', body)))
GLYPHS = {c: int(v, 8) for c, v in re.findall(r"case '(.)': return 0?([0-7]+);", SOURCE)}

# --- New palette characters (plan step 1) ---------------------------------------------------------------------------
NEW_PALETTE = {
    "J": 0x07120D,  # jungle: deepest shade
    "j": 0x10251A,  # jungle: far trunks
    "f": 0x173A26,  # jungle: dark leaves
    "F": 0x25502F,  # jungle: light leaves
    "t": 0x2A1F14,  # jungle: near trunk bark
    "T": 0x3E2E1C,  # jungle: trunk edge, ropes
    "B": 0x54402A,  # jungle: bridge planks
    "L": 0x687834,  # jungle: firefly
    "l": 0x9BD432,  # lime: leafhopper
    "h": 0x9A5E2C,  # brown: beetle, monkey fur
    "H": 0xD8A868,  # tan: highlights, faces, horn
    "x": 0x1E1A1C,  # insect black
    "X": 0x5C5866,  # slate: shine on black, gorilla fur
    "Y": 0xF0C828,  # banana yellow (not the bullets' pale yellow)
    "M": 0x7DA83A,  # stink mist
}
PALETTE.update(NEW_PALETTE)

# --- New sprites (plan step 1) --------------------------------------------------------------------------------------
NEW_SPRITES = {
    "LEAFHOPPER_A": (12, 10,
        ".l........l."
        "..l......l.."
        "...GllllG..."
        "..lrllllrl.."
        ".GllllllllG."
        "GlGllllllGlG"
        "G.lGllllGl.G"
        "..lGllllGl.."
        ".G..llll..G."
        "G...G..G...G"),
    "LEAFHOPPER_B": (12, 10,
        ".l........l."
        "..l......l.."
        "...GllllG..."
        "..lrllllrl.."
        ".GllllllllG."
        "lGGllllllGGl"
        "..lGllllGl.."
        ".GlGllllGlG."
        "G...llll...G"
        "....G..G...."),
    "BEETLE_A": (12, 10,
        "...x....x..."
        "....xxxx...."
        "...xHxxHx..."
        "x.hhhxxhhh.x"
        ".hHHhxxhhhh."
        "xhHhhxxhhhhx"
        ".hhhhxxhhhh."
        "xhhhhxxhhHhx"
        "..hhhxxhhh.."
        ".x..hhhh..x."),
    "BEETLE_B": (12, 10,
        "...x....x..."
        "....xxxx...."
        "...xHxxHx..."
        "..hhhxxhhh.."
        "xhHHhxxhhhhx"
        ".hHhhxxhhhh."
        "xhhhhxxhhhhx"
        ".hhhhxxhhHh."
        "x.hhhxxhhh.x"
        "....hhhh...."),
    "STAG_A": (12, 10,
        ".h........h."
        "..h......h.."
        "..hh....hh.."
        "...xXxxXx..."
        "..xrxxxxrx.."
        "x.xXXxxXXx.x"
        ".xXXxxxxXXx."
        "xxXxxxxxxXxx"
        ".xxXxxxxXxx."
        "x..xxxxxx..x"),
    "STAG_B": (12, 10,
        "..h......h.."
        "..h......h.."
        "...hh..hh..."
        "...xXxxXx..."
        "x.xrxxxxrx.x"
        ".xxXXxxXXxx."
        "..XXxxxxXX.."
        "xxXxxxxxxXxx"
        ".xxXxxxxXxx."
        "...xxxxxx..."),
    "RHINO_A": (16, 12,
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
        ".x...xx..xx...x."),
    "RHINO_B": (16, 12,
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
        "x....xx..xx....x"),
    # Monkey: brown fur, tan face and belly, a banana in the raised hand on frame A.
    "MONKEY_A": (32, 20,
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
        "........................hh......"),
    "MONKEY_B": (32, 20,
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
        "........................hh......"),
    # Gorilla: slate fur with a lighter silver back, tan face, angry red eyes.
    "GORILLA_A": (32, 20,
        "...........XXXXXXXXXX..........."
        "..........XXXXXXXXXXXX.........."
        ".........XXHHHHHHHHHHXX........."
        ".........XHHxrHHHHrxHHX........."
        ".........XHHHHHHHHHHHHX........."
        "..........XHHHxxxxHHHX.........."
        "......XXXXXXHHHHHHHHXXXXXX......"
        "....XXXbbXXXXXXXXXXXXXbbXXXX...."
        "...XXXbbXXXXxxxxxxXXXXXbbXXXX..."
        "..XXXbbXXXXxxxxxxxxXXXXXbbXXXX.."
        "..XXbbXX.XXxxxxxxxxXX.XXbbXXX..."
        "..XXXXX..XXXxxxxxxXXX..XXXXX...."
        "..XXXX...XXXXXXXXXXXX...XXXX...."
        "..HHHH...XXXXXXXXXXXX...HHHH...."
        "..HHHH...XXXX....XXXX...HHHH...."
        ".........XXXX....XXXX..........."
        "........XXXXX....XXXXX.........."
        "........HHHH......HHHH.........."
        "................................"
        "................................"),
    "GORILLA_B": (32, 20,
        "...........XXXXXXXXXX..........."
        "..........XXXXXXXXXXXX.........."
        ".........XXHHHHHHHHHHXX........."
        ".........XHHrxHHHHxrHHX........."
        ".........XHHHHHHHHHHHHX........."
        "..........XHHHxxxxHHHX.........."
        "......XXXXXXHHHHHHHHXXXXXX......"
        "....XXXbbXXXXXXXXXXXXXbbXXXX...."
        "...XXXbbXXXXxxxxxxXXXXXbbXXXX..."
        "..XXXbbXXXXxxxxxxxxXXXXXbbXXXX.."
        "..XXbbXX.XXxxxxxxxxXX.XXbbXXX..."
        "...XXXXX.XXXxxxxxxXXX.XXXXX....."
        "...XXXX..XXXXXXXXXXXX..XXXX....."
        "...HHHH..XXXXXXXXXXXX..HHHH....."
        "...HHHH..XXXX....XXXX..HHHH....."
        ".........XXXX....XXXX..........."
        "........XXXXX....XXXXX.........."
        "........HHHH......HHHH.........."
        "................................"
        "................................"),
    # Laugh: eyes squeezed shut, mouth wide open, head tipped back.
    "GORILLA_LAUGH": (32, 20,
        "...........XXXXXXXXXX..........."
        "..........XXXXXXXXXXXX.........."
        ".........XXHHHHHHHHHHXX........."
        ".........XHHxxHHHHxxHHX........."
        ".........XHHHHHHHHHHHHX........."
        "..........XHHrrrrrrHHX.........."
        "......XXXXXXHrrrrrrHXXXXXX......"
        "....XXXbbXXXXHHHHHHXXXXbbXXX...."
        "...XXXbbXXXXxxxxxxXXXXXbbXXXX..."
        "..XXXbbXXXXxxxxxxxxXXXXXbbXXXX.."
        "..XXbbXX.XXxxxxxxxxXX.XXbbXXX..."
        "..XXXXX..XXXxxxxxxXXX..XXXXX...."
        "..XXXX...XXXXXXXXXXXX...XXXX...."
        "..HHHH...XXXXXXXXXXXX...HHHH...."
        "..HHHH...XXXX....XXXX...HHHH...."
        ".........XXXX....XXXX..........."
        "........XXXXX....XXXXX.........."
        "........HHHH......HHHH.........."
        "................................"
        "................................"),
    # Chest pound: both fists on the chest.
    "GORILLA_POUND": (32, 20,
        "...........XXXXXXXXXX..........."
        "..........XXXXXXXXXXXX.........."
        ".........XXHHHHHHHHHHXX........."
        ".........XHHxrHHHHrxHHX........."
        ".........XHHHHHHHHHHHHX........."
        "..........XHHHrrrrHHHX.........."
        "......XXXXXXHHHHHHHHXXXXXX......"
        "....XXXbbXXXXXXXXXXXXXbbXXXX...."
        "...XXXbbXXXXxxxxxxXXXXXbbXXXX..."
        "...XXbbXXXHHHHxxHHHHXXXbbXXX...."
        "....XXXXXXHHHHxxHHHHXXXXXXX....."
        ".....XXXXXHHHHxxHHHHXXXXX......."
        "......XXXXXXxxxxxxXXXXXX........"
        ".........XXXXXXXXXXXX..........."
        ".........XXXXXXXXXXXX..........."
        ".........XXXX....XXXX..........."
        "........XXXXX....XXXXX.........."
        "........HHHH......HHHH.........."
        "................................"
        "................................"),
    # Thrown hazards: magenta marks every thing that kills on touch.
    "BANANA_A": (6, 6,
        "....m."
        "....Ym"
        "...YY."
        "..YYY."
        "mYYY.."
        ".mm..."),
    "BANANA_B": (6, 6,
        ".m...."
        "mY...."
        ".YY..."
        ".YYY.."
        "..YYYm"
        "...mm."),
    "COCONUT": (6, 6,
        ".mmmm."
        "mhhhhm"
        "mhxhxm"
        "mhhhhm"
        "mhhhhm"
        ".mmmm."),
    # Harmless while flying or lying: no magenta.
    "PEEL": (8, 6,
        "Y......Y"
        ".Y....Y."
        "..YHHY.."
        "..YHHY.."
        ".YYHHYY."
        "Y..YY..Y"),
    "POOP": (6, 5,
        "...h.."
        "..hhh."
        ".hhxhh"
        "hhhhhh"
        ".hhhh."),
}
SPRITES.update(NEW_SPRITES)


# --- Background generator (plan step 4, same algorithm) --------------------------------------------------------------
class Rng:
    def __init__(self, seed):
        self.s = seed

    def unit(self):
        s = self.s
        s ^= (s << 13) & 0xFFFFFFFF
        s ^= s >> 17
        s ^= (s << 5) & 0xFFFFFFFF
        self.s = s
        return (s & 0xFFFFFF) / float(0x1000000)


TILE_H = 360  # the jungle tile is two screens tall and wraps vertically
FIREFLIES = 14


def leaf_cluster(tile, cx, cy, r, ink, light):
    ri = int(r)
    for dy in range(-ri, ri + 1):
        for dx in range(-ri, ri + 1):
            if dx * dx + dy * dy > r * r:
                continue
            x, y = cx + dx, (cy + dy) % TILE_H
            if x < 0 or x >= W:
                continue
            if (x * 7 + y * 3) % 11 == 0 and dx * dx + dy * dy < r * r * 0.4:
                tile[y][x] = light
            elif (x + y) % 2 == 0 or dx * dx + dy * dy < r * r * 0.3:
                tile[y][x] = ink


def build_jungle():
    rng = Rng(0x4A554E47)  # "JUNG"
    far = [["J"] * W for _ in range(TILE_H)]
    mid = [["."] * W for _ in range(TILE_H)]
    two_pi = 2 * math.pi
    # Far layer: six thin trunks with dark grooves, then leaf clusters.
    for i in range(6):
        cx = 24 + i * 56 + (rng.unit() - 0.5) * 16
        width = 10 + int(rng.unit() * 8)
        phase = rng.unit() * two_pi
        for y in range(TILE_H):
            left = int(cx + 3 * math.sin(two_pi * 2 * y / TILE_H + phase) - width / 2)
            for x in range(max(left, 0), min(left + width, W)):
                groove = (x - left) % 4 == 1 and (y // 3 + i) % 5 != 0
                far[y][x] = "J" if groove else "j"
    for _ in range(70):
        leaf_cluster(far, int(rng.unit() * W), int(rng.unit() * TILE_H), 5 + rng.unit() * 9, "f", "F")
    # Mid layer: the two big edge trunks.
    for side in range(2):
        phase = rng.unit() * two_pi
        for y in range(TILE_H):
            width = int(26 + 6 * math.sin(two_pi * 2 * y / TILE_H + phase) + 3 * math.sin(two_pi * 5 * y / TILE_H + phase * 2))
            for i in range(width):
                x = i if side == 0 else W - 1 - i
                bark = (i + (y // 6) * 2) % 7 == 0
                mid[y][x] = "T" if i >= width - 3 else "J" if bark else "t"
        for _ in range(2):  # knot holes
            hy, hx = int(rng.unit() * TILE_H), 8 + int(rng.unit() * 10)
            if side == 1:
                hx = W - 1 - hx
            for dy in range(-4, 5):
                for dx in range(-2, 3):
                    if dx * dx * 16 + dy * dy * 4 <= 64:
                        mid[(hy + dy) % TILE_H][hx + dx] = "J"
    # Two rope bridges.
    for b in range(2):
        y0 = 70 + b * 180
        for x in range(20, 300):
            deck = y0 + int(12 * math.sin(math.pi * (x - 20) / 280))
            mid[deck % TILE_H][x] = "T"
            mid[(deck - 6) % TILE_H][x] = "T"
            if x % 5 != 0:
                mid[(deck + 1) % TILE_H][x] = "B"
                mid[(deck + 2) % TILE_H][x] = "B"
            if x % 10 == 0:
                for y in range(deck - 5, deck):
                    mid[y % TILE_H][x] = "T"
    # Hanging vines.
    for _ in range(7):
        x0, top, length, phase = 40 + rng.unit() * 240, int(rng.unit() * TILE_H), 30 + int(rng.unit() * 50), rng.unit() * two_pi
        for t in range(length):
            x = int(x0 + 3 * math.sin(t * 0.15 + phase))
            y = (top + t) % TILE_H
            mid[y][x] = "f"
            if t % 6 == 0:
                mid[y][x - 1] = "F"
                mid[y][x + 1] = "F"
    # Leaf clumps on the trunks' inner edges.
    for i in range(22):
        cx = 30 + int(rng.unit() * 8) if i % 2 == 0 else W - 31 - int(rng.unit() * 8)
        leaf_cluster(mid, cx, int(rng.unit() * TILE_H), 6 + rng.unit() * 6, "f", "F")
    return far, mid


FAR, MID = build_jungle()
FAR_SPEED, MID_SPEED, FIREFLY_SPEED = 8, 18, 22


def draw_jungle(px, t):
    far_off, mid_off = int(t * FAR_SPEED) % TILE_H, int(t * MID_SPEED) % TILE_H
    for y in range(H):
        far_row = FAR[(y - far_off) % TILE_H]
        mid_row = MID[(y - mid_off) % TILE_H]
        for x in range(W):
            c = mid_row[x] if mid_row[x] != "." else far_row[x]
            px[y][x] = PALETTE[c]
    for i in range(FIREFLIES):
        if math.sin(t * 2.5 + i * 1.7) <= 0.2:
            continue
        x = int((i * 97 + 13) % W + 6 * math.sin(t * 0.5 + i))
        y = int(((i * 53 + 29) + t * FIREFLY_SPEED + 4 * math.sin(t * 0.7 + i * 1.3)) % H)
        if 0 <= x < W:
            px[y][x] = PALETTE["L"]


# --- Screen helpers -------------------------------------------------------------------------------------------------
def blank():
    return [[0] * W for _ in range(H)]


def sprite(px, name, x, y, solid=None):
    w, h, pixels = SPRITES[name]
    for row in range(h):
        for col in range(w):
            c = pixels[row * w + col]
            if c == "." or c not in PALETTE:
                continue
            X, Y = x + col, y + row
            if 0 <= X < W and 0 <= Y < H:
                px[Y][X] = solid if solid is not None else PALETTE[c]


def rect(px, x, y, w, h, color):
    for Y in range(max(y, 0), min(y + h, H)):
        for X in range(max(x, 0), min(x + w, W)):
            px[Y][X] = color


def text(px, x, y, s, color, scale=1):
    for ch in s:
        g = GLYPHS.get(ch, 0)
        for row in range(5):
            for col in range(3):
                if (g >> (3 * (4 - row) + (2 - col))) & 1:
                    rect(px, x + col * scale, y + row * scale, scale, scale, color)
        x += 4 * scale


def text_w(s, scale=1):
    return (len(s) * 4 - 1) * scale if s else 0


def centred(px, y, s, color, scale=1):
    text(px, (W - text_w(s, scale)) // 2, y, s, color, scale)


def mist(px, cx, cy, r, frame):
    ri = int(r)
    for dy in range(-ri, ri + 1):
        for dx in range(-ri, ri + 1):
            d2 = dx * dx + dy * dy
            if d2 > r * r:
                continue
            x, y = cx + dx, cy + dy
            edge = d2 > r * r * 0.55
            if 0 <= x < W and 0 <= y < H and ((x + y + frame) % 2 == 0 if not edge else (x + 2 * y + frame) % 4 == 0):
                px[y][x] = PALETTE["M"]


def hud(px, label, score, lives, rockets):
    text(px, 2, 2, "SCORE %07d" % score, PALETTE["w"])
    centred(px, 2, label, PALETTE["w"])
    centred(px, 9, "HI %07d" % 381200, PALETTE["s"])
    for i in range(lives - 1):
        sprite(px, "LIFE_ICON", 2 + i * 9, 9)
    for i in range(rockets):
        sprite(px, "ROCKET_ICON", 2 + i * 5, H - 7)


def health_bar(px, fraction, healing=False):
    x = (W - 120) // 2
    rect(px, x, 16, 120, 3, PALETTE["R"])
    rect(px, x, 16, int(120 * fraction), 3, PALETTE["l" if healing else "r"])


def save(px, name, scale=4):
    img = Image.new("RGB", (W, H))
    img.putdata([(c >> 16 & 255, c >> 8 & 255, c & 255) for row in px for c in row])
    img.resize((W * scale, H * scale), Image.NEAREST).save(os.path.join(HERE, name))


# --- Mock 1: a formation wave (jungle wave 9: 5 rows, 1 heavy rhino beetle) -------------------------------------------
def mock_wave():
    px = blank()
    draw_jungle(px, 3.0)
    rows = [("STAG", 2), ("BEETLE", 2), ("LEAFHOPPER", 1)]
    types = ["STAG", "STAG", "BEETLE", "BEETLE", "LEAFHOPPER"]
    left = (W - 7 * 18 - 12) // 2 + 14
    for row, kind in enumerate(types):
        for col in range(8):
            if (row, col) in ((4, 2), (3, 6), (4, 7)):
                continue  # already shot
            sprite(px, kind + ("_A" if (row + col) % 2 else "_B"), left + col * 18, 18 + row * 14)
    sprite(px, "RHINO_A", 214, 66)
    sprite(px, "LEAFHOPPER_A", 120, 104)  # a diver
    for x, y in ((90, 60), (176, 92), (222, 84), (130, 124), (60, 140), (250, 120)):
        sprite(px, "ENEMY_SHOT", x, y)
    for i in range(4):
        sprite(px, "BULLET", 148 + (i % 2) * 6, 140 - i * 14)
    sprite(px, "SHIP_A", 144, 156)
    hud(px, "JUNGLE 9", 214300, 3, 4)
    save(px, "mock-wave.png")


# --- Mock 2: twin monkeys (jungle wave 10): bananas, a peel lob, a peel on the ground, a slipping ship -----------------
def mock_monkeys():
    px = blank()
    draw_jungle(px, 7.0)
    sprite(px, "MONKEY_A", 40, 20)
    sprite(px, "MONKEY_B", 230, 20)
    for i in range(3):
        sprite(px, "BANANA_A" if i % 2 else "BANANA_B", 53, 44 + i * 13)
    for i in range(5):
        sprite(px, "BANANA_A" if i % 2 else "BANANA_B", 243 + (i - 2) * 8, 52 + abs(i - 2) * 2)
    # Peel in flight on its arc, with its landing mark.
    sprite(px, "PEEL", 120, 70)
    for dx, dy in ((-3, 0), (3, 0), (0, -2), (0, 2)):
        rect(px, 168 + dx, 150 + dy, 1, 1, PALETTE["H"])
    sprite(px, "PEEL", 70, 130)
    # Ship stunned after a slip: jitter and dizzy stars.
    sprite(px, "SHIP_A", 201, 150)
    for i in range(3):
        a = 4.0 + i * 2.09
        rect(px, 209 + int(6 * math.cos(a)), 146 + int(2 * math.sin(a)), 1, 1, PALETTE["Y"])
    hud(px, "JUNGLE 10", 268900, 2, 3)
    health_bar(px, 0.62)
    save(px, "mock-monkeys.png")


# --- Mock 3: the gorilla (jungle wave 15): mist, a healing laugh, coconut rain -----------------------------------------
def mock_gorilla():
    px = blank()
    draw_jungle(px, 11.0)
    mist(px, 120, 150, 20, 0)
    mist(px, 230, 128, 14, 1)
    sprite(px, "GORILLA_LAUGH", 140, 20)
    text(px, 146, 43, "HA HA", PALETTE["M"])  # under the gorilla: boss.y + 20 + 3
    for x, y in ((40, 30), (100, 70), (196, 52), (260, 96), (290, 40)):
        sprite(px, "COCONUT", x, y)
    sprite(px, "POOP", 170, 92)
    for dx, dy in ((-3, 0), (3, 0), (0, -2), (0, 2)):
        rect(px, 214 + dx, 146 + dy, 1, 1, PALETTE["M"])
    # The ship stands in the mist: its bullets turn lime and heal the gorilla.
    sprite(px, "SHIP_A", 112, 146)
    for i in range(5):
        x, y = 119, 132 - i * 18
        rect(px, x, y, 2, 4, PALETTE["l"])
    for i in range(3):
        rect(px, 150 + i * 5, 42 + (i % 2) * 3, 1, 1, PALETTE["l"])
    hud(px, "JUNGLE 15", 402100, 3, 2)
    health_bar(px, 0.48, healing=True)
    save(px, "mock-gorilla.png")


# --- Palette swatches and sprite sheet ------------------------------------------------------------------------------
def palette_sheet():
    keys = list(NEW_PALETTE)
    img = Image.new("RGB", (len(keys) * 64, 96), (12, 15, 30))
    d = ImageDraw.Draw(img)
    for i, k in enumerate(keys):
        c = NEW_PALETTE[k]
        d.rectangle([i * 64 + 4, 4, i * 64 + 59, 59], fill=(c >> 16, c >> 8 & 255, c & 255))
        d.text((i * 64 + 26, 66), k, fill=(255, 255, 255))
        d.text((i * 64 + 6, 80), "%06X" % c, fill=(160, 160, 180))
    img.save(os.path.join(HERE, "palette.png"))


def sprite_sheet():
    names = list(NEW_SPRITES)
    scale, cell_w, cell_h, per_row = 6, 34 * 6, 22 * 6 + 16, 5
    rows = (len(names) + per_row - 1) // per_row
    img = Image.new("RGB", (per_row * cell_w, rows * cell_h), (7, 18, 13))
    d = ImageDraw.Draw(img)
    for i, name in enumerate(names):
        w, h, pixels = SPRITES[name]
        ox, oy = (i % per_row) * cell_w + 6, (i // per_row) * cell_h + 4
        for row in range(h):
            for col in range(w):
                c = pixels[row * w + col]
                if c != ".":
                    v = PALETTE[c]
                    d.rectangle([ox + col * scale, oy + row * scale, ox + col * scale + scale - 1, oy + row * scale + scale - 1],
                                fill=(v >> 16, v >> 8 & 255, v & 255))
        d.text((ox, (i // per_row) * cell_h + cell_h - 14), "%s %dx%d" % (name, w, h), fill=(220, 220, 220))
    img.save(os.path.join(HERE, "sprites.png"))


def background_only():
    px = blank()
    draw_jungle(px, 0.0)
    save(px, "background.png")


if __name__ == "__main__":
    for name, (w, h, pixels) in NEW_SPRITES.items():
        assert len(pixels) == w * h, name
    palette_sheet()
    sprite_sheet()
    background_only()
    mock_wave()
    mock_monkeys()
    mock_gorilla()
    print("wrote palette.png, sprites.png, background.png, mock-wave.png, mock-monkeys.png, mock-gorilla.png")
