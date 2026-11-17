"""Draws the XMB Game menu art into pkgfiles/: ICON0.PNG (320x176 icon) and PIC1.PNG (1920x1080 backdrop).

The palette and sprites are read from source/sprites.h, so the art always matches the game.
Needs Pillow (python3 -m pip install pillow). Run from anywhere: python3 docs/xmb-art/make_xmb_art.py
"""
import math
import os
import random
import re

from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = os.path.join(ROOT, "pkgfiles")

with open(os.path.join(ROOT, "source", "sprites.h")) as f:
    SOURCE = f.read()
PALETTE = {c: ((int(v, 16) >> 16) & 255, (int(v, 16) >> 8) & 255, int(v, 16) & 255)
           for c, v in re.findall(r"case '(.)': return 0x([0-9A-Fa-f]{8});", SOURCE)}
SPRITES = {}
for name, w, h, body in re.findall(r'const Sprite (\w+) = \{(\d+), (\d+),((?:\s*"[^"]*")+)\};', SOURCE):
    pixels = "".join(re.findall(r'"([^"]*)"', body))
    SPRITES[name] = [pixels[row * int(w):(row + 1) * int(w)] for row in range(int(h))]

# Colours only the XMB art uses: nebula, planet shading and the logo's outline.
EXTRA = {"v": (40, 26, 72), "V": (70, 40, 110), "q": (18, 22, 44), "P": (16, 26, 70), "O": (60, 14, 24)}
PALETTE.update(EXTRA)

# Chunky logo font: 7 rows, '#' is ink. Widths vary per letter.
LOGO_FONT = {
    "N": ["##...##", "###..##", "####.##", "##.####", "##..###", "##...##", "##...##"],
    "A": [".####.", "##..##", "##..##", "######", "##..##", "##..##", "##..##"],
    "M": ["##...##", "###.###", "#######", "##.#.##", "##...##", "##...##", "##...##"],
    "E": ["#####", "##...", "##...", "####.", "##...", "##...", "#####"],
    "G": [".####.", "##..##", "##....", "##.###", "##..##", "##..##", ".####."],
    " ": ["...", "...", "...", "...", "...", "...", "..."],
}
LOGO_FILL = "wyyyooorrR"   # top-to-bottom gradient, one colour per band


class Canvas:
    def __init__(self, w, h):
        self.w, self.h = w, h
        self.px = [[PALETTE["k"]] * w for _ in range(h)]

    def dot(self, x, y, color, size=1):
        rgb = PALETTE[color] if isinstance(color, str) else color
        for yy in range(max(0, y), min(self.h, y + size)):
            for xx in range(max(0, x), min(self.w, x + size)):
                self.px[yy][xx] = rgb

    def sprite(self, name, x, y, scale=1, solid=None):
        for row, line in enumerate(SPRITES[name]):
            for col, c in enumerate(line):
                if c in PALETTE and c != ".":
                    self.dot(x + col * scale, y + row * scale, solid or c, scale)

    def save(self, path, scale):
        image = Image.new("RGB", (self.w, self.h))
        image.putdata([p for row in self.px for p in row])
        image.resize((self.w * scale, self.h * scale), Image.NEAREST).save(path, optimize=True)
        print("wrote", os.path.relpath(path, ROOT), image.width * scale, "x", image.height * scale)


BAYER = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]


def dithered(x, y, amount):
    """True for `amount` (0..1) of the pixels in an ordered-dither pattern."""
    return BAYER[y % 4][x % 4] < amount * 16


def nebula(canvas, cx, cy, rx, ry, seed):
    rnd = random.Random(seed)
    lumps = [(cx + rnd.uniform(-rx, rx) * 0.6, cy + rnd.uniform(-ry, ry) * 0.6, rnd.uniform(0.5, 1)) for _ in range(5)]
    for y in range(canvas.h):
        for x in range(canvas.w):
            d = min(math.hypot((x - lx) / (rx * s), (y - ly) / (ry * s)) for lx, ly, s in lumps)
            glow = max(0.0, 1 - d)
            if dithered(x, y, glow * 1.6 - 0.9):
                canvas.dot(x, y, "V")
            elif dithered(x, y, glow * 1.6):
                canvas.dot(x, y, "v")
            elif dithered(x, y, glow * 2.2):
                canvas.dot(x, y, "q")


def stars(canvas, count, seed, bright=4):
    rnd = random.Random(seed)
    for i in range(count):
        x, y = rnd.randrange(canvas.w), rnd.randrange(canvas.h)
        canvas.dot(x, y, "nds"[i % 3])
    for _ in range(bright):   # a few twinkling crosses
        x, y = rnd.randrange(canvas.w), rnd.randrange(canvas.h)
        for dx, dy in ((-1, 0), (1, 0), (0, -1), (0, 1)):
            canvas.dot(x + dx, y + dy, "d")
        canvas.dot(x, y, "w")


def planet(canvas, cx, cy, r, light=(0.55, -0.6, 0.58)):
    for y in range(cy - r, cy + r + 1):
        for x in range(cx - r, cx + r + 1):
            dx, dy = (x - cx) / r, (y - cy) / r
            if dx * dx + dy * dy > 1:
                continue
            dz = math.sqrt(max(0.0, 1 - dx * dx - dy * dy))
            lit = dx * light[0] + dy * light[1] + dz * light[2]
            lit += math.sin((dy * 6 + 0.4 * math.sin(dx * 5)) * 2.5) * 0.08   # cloud bands
            if lit < 0.12:
                color = "q" if lit < -0.1 else "P"
            elif lit < 0.5:
                color = "U" if not (lit < 0.2 and dithered(x, y, 1 - (lit - 0.12) * 12)) else "P"
            else:
                color = "u" if not (lit < 0.6 and dithered(x, y, 1 - (lit - 0.5) * 10)) else "U"
            if dx * dx + dy * dy > 0.9 and lit > 0.35:   # thin atmosphere rim on the lit side
                color = "c"
            canvas.dot(x, y, color)


def explosion(canvas, cx, cy, r, fade=0.0, seed=1):
    rnd = random.Random(seed)
    for dy in range(-r, r + 1):
        for dx in range(-r, r + 1):
            d = math.hypot(dx, dy) / r
            if d <= 1 and rnd.random() >= fade * d * d:
                canvas.dot(cx + dx, cy + dy, "w" if d < 0.35 else "y" if d < 0.65 else "o")


def particles(canvas, cx, cy, color, count, radius, seed):
    rnd = random.Random(seed)
    for i in range(count):
        a = i * 2 * math.pi / count + rnd.uniform(0, 0.5)
        d = radius * rnd.uniform(0.6, 1.2)
        canvas.dot(int(cx + math.cos(a) * d), int(cy + math.sin(a) * d), color)


def logo(canvas, text, y, unit):
    """Gradient letters with a dark outline and a drop shadow, centred."""
    glyphs = [LOGO_FONT[c] for c in text]
    width = sum(len(g[0]) for g in glyphs) + len(glyphs) - 1
    ink = set()
    x = 0
    for g in glyphs:
        for row, line in enumerate(g):
            for col, c in enumerate(line):
                if c == "#":
                    ink.add((x + col, row))
        x += len(g[0]) + 1
    # Work in sub-pixels of unit/ sub so the outline is thinner than a letter stroke.
    sub = 2
    s = unit // sub
    fine = {(fx * sub + i, fy * sub + j) for fx, fy in ink for i in range(sub) for j in range(sub)}
    outline = {(fx + dx, fy + dy) for fx, fy in fine for dx in (-1, 0, 1) for dy in (-1, 0, 1)} - fine
    left = (canvas.w - width * unit) // 2
    rows = 7 * sub
    for fx, fy in fine | outline:   # shadow
        canvas.dot(left + (fx + 1) * s, y + (fy + 2) * s, "q", s)
    for fx, fy in outline:
        canvas.dot(left + fx * s, y + fy * s, "O", s)
    for fx, fy in fine:
        band = LOGO_FILL[min(fy * len(LOGO_FILL) // rows, len(LOGO_FILL) - 1)]
        canvas.dot(left + fx * s, y + fy * s, band, s)
    # Glint: a white tick on the top-left of each letter's first stroke.
    x = 0
    for g in glyphs:
        first = next((col for col, c in enumerate(g[1]) if c == "#"), None)
        if first is not None:
            canvas.dot(left + (x + first) * unit, y + unit, "w", s)
        x += len(g[0]) + 1


def make_icon():
    # 160x88 drawn at x2: the game's sprites appear at x2 and x3, the logo at a 4 px unit.
    c = Canvas(160, 88)
    nebula(c, 120, 40, 60, 30, seed=3)
    stars(c, 90, seed=7, bright=3)
    planet(c, 150, 104, 40)

    # Alien formation behind the logo.
    for col, name in enumerate(["GREEN_ALIEN_A", "BLUE_ALIEN_B", "RED_ALIEN_A", "BLUE_ALIEN_A"]):
        c.sprite(name, 22 + col * 24, 36 + (col % 2) * 4, solid="w" if col == 1 else None)   # col 1: hit flash
    explosion(c, 124, 42, 9, fade=0.5)
    particles(c, 124, 42, "g", 10, 13, seed=2)

    # Diving red alien and its shots.
    c.sprite("RED_ALIEN_B", 126, 58, 2)
    for y in (72, 80):
        c.sprite("ENEMY_SHOT", 115, y)

    # The ship, bullets and a rocket streaking up.
    ship_x, ship_y = 31, 60
    c.sprite("SHIP_A", ship_x, ship_y, 2)
    for y in range(ship_y - 6, 46, -7):   # the stream ends at the flashing alien
        c.sprite("BULLET", ship_x + 15, y)
    c.sprite("ROCKET", 92, 52)
    for i in range(4):
        c.dot(93 + i % 2, 61 + i * 2, "o" if i < 2 else "r")

    logo(c, "NAME GAME", 5, 2)
    c.save(os.path.join(OUT, "ICON0.PNG"), 2)


def make_backdrop():
    # 320x180 drawn at x6, like the game; the left half stays calm for the XMB's own text.
    c = Canvas(320, 180)
    for y in range(c.h):   # darker at the top, a faint lift toward the planet
        for x in range(c.w):
            if dithered(x, y, (y / c.h) * 0.5 * (x / c.w)):
                c.dot(x, y, "q")
    nebula(c, 250, 70, 80, 40, seed=11)
    stars(c, 160, seed=5, bright=7)
    planet(c, 332, 214, 80)

    left = 196
    rows = [("RED_ALIEN_A", 18), ("BLUE_ALIEN_A", 32), ("BLUE_ALIEN_B", 46), ("GREEN_ALIEN_A", 60)]
    for row, (name, y) in enumerate(rows):
        for col in range(6):
            if (row, col) in ((1, 4), (3, 1), (2, 5)):
                continue   # gaps: aliens already shot down or diving
            if (row, col) == (3, 2):
                c.sprite(name, left + col * 18, y, solid="w")
                continue
            c.sprite(name, left + col * 18, y)
    explosion(c, left + 4 * 18 + 6, 37, 10, fade=0.5)
    particles(c, left + 4 * 18 + 6, 37, "u", 14, 15, seed=9)
    particles(c, left + 1 * 18 + 6, 64, "g", 10, 9, seed=4)

    c.sprite("GREEN_ALIEN_B", 168, 104)   # diving
    for y in (118, 130, 141):
        c.sprite("ENEMY_SHOT", 174, y)
    c.sprite("ROCKET_PICKUP", 268, 116)

    ship_x, ship_y = 236, 146
    c.sprite("SHIP_B", ship_x, ship_y)
    for y in range(ship_y - 6, 68, -12):   # the stream ends at the flashing alien
        c.sprite("BULLET", ship_x + 7, y)
    c.sprite("ROCKET", ship_x + 26, 92)
    for i in range(5):
        c.dot(ship_x + 27 + (i % 2), 100 + i * 2, "o" if i < 2 else "r")
    c.save(os.path.join(OUT, "PIC1.PNG"), 6)


if __name__ == "__main__":
    make_icon()
    make_backdrop()
