"""Writes NAME QUEST's sprite text grids (spec §21, §22), authored at 640x360 detail:
- source/quest_sprites.h: the four heroes (16 frames each, small old-school 32x36 real px) and the objects (global palette);
- source/quest_sprites_0.h/.cpp: Namula's tiles, platforms, props and landmarks (world palette, '0'-'f');
- source/quest_sprites_1.h/.cpp: world 1's art, its five enemies and two bosses;
- source/quest_sprites_2.h/.cpp: world 2's art, its five enemies and two bosses;
- source/quest_sprites_3.h/.cpp: world 3's art, its five enemies and two bosses.
The generated text is committed and is the source of truth for the build. Standard library only.
Run: python3 -I docs/work-plan/name-quest/art-tools/make_quest_art.py [--preview <folder>]
"""
import math
import os
import random
import re
import struct
import sys
import zlib

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", ".."))
SOURCE = os.path.join(ROOT, "source")


class Grid:
    """A sprite being drawn: w x h characters, '.' transparent."""

    def __init__(self, w, h):
        self.w, self.h = w, h
        self.c = [["."] * w for _ in range(h)]

    def set(self, x, y, ch):
        if 0 <= x < self.w and 0 <= y < self.h:
            self.c[y][x] = ch

    def get(self, x, y):
        return self.c[y][x] if 0 <= x < self.w and 0 <= y < self.h else "."

    def paint(self, pixels, ramp):
        """Fills a shape shaded by its own edges: dark at the bottom/right, light at the top/left, mid inside."""
        dark, mid, light = ramp
        for (x, y) in pixels:
            if (x, y + 1) not in pixels or (x + 1, y) not in pixels:
                self.set(x, y, dark)
            elif (x, y - 1) not in pixels or (x - 1, y) not in pixels:
                self.set(x, y, light)
            else:
                self.set(x, y, mid)

    def fill(self, pixels, ch):
        for (x, y) in pixels:
            self.set(x, y, ch)

    def flat(self):
        return "".join("".join(row) for row in self.c)


# --- Shapes: sets of pixels whose centres fall inside (coordinates are in pixels, centres at +0.5) ---
def disc(cx, cy, rx, ry=None):
    ry = rx if ry is None else ry
    out = set()
    for y in range(int(math.floor(cy - ry)) - 1, int(math.ceil(cy + ry)) + 1):
        for x in range(int(math.floor(cx - rx)) - 1, int(math.ceil(cx + rx)) + 1):
            if ((x + 0.5 - cx) / rx) ** 2 + ((y + 0.5 - cy) / ry) ** 2 <= 1:
                out.add((x, y))
    return out


def capsule(x0, y0, x1, y1, r):
    out = set()
    dx, dy = x1 - x0, y1 - y0
    length2 = dx * dx + dy * dy or 1e-9
    for y in range(int(math.floor(min(y0, y1) - r)) - 1, int(math.ceil(max(y0, y1) + r)) + 1):
        for x in range(int(math.floor(min(x0, x1) - r)) - 1, int(math.ceil(max(x0, x1) + r)) + 1):
            px, py = x + 0.5, y + 0.5
            t = max(0, min(1, ((px - x0) * dx + (py - y0) * dy) / length2))
            if (px - x0 - t * dx) ** 2 + (py - y0 - t * dy) ** 2 <= r * r:
                out.add((x, y))
    return out


def poly(points):
    out = set()
    xs, ys = [p[0] for p in points], [p[1] for p in points]
    for y in range(int(min(ys)) - 1, int(max(ys)) + 2):
        for x in range(int(min(xs)) - 1, int(max(xs)) + 2):
            px, py, inside = x + 0.5, y + 0.5, False
            for i in range(len(points)):
                ax, ay = points[i]
                bx, by = points[i - 1]
                if (ay > py) != (by > py) and px < (bx - ax) * (py - ay) / (by - ay) + ax:
                    inside = not inside
            if inside:
                out.add((x, y))
    return out


def rect(x, y, w, h):
    return {(xx, yy) for yy in range(y, y + h) for xx in range(x, x + w)}


def line(x0, y0, x1, y1):
    out = set()
    steps = int(max(abs(x1 - x0), abs(y1 - y0))) + 1
    for i in range(steps + 1):
        t = i / steps
        out.add((int(math.floor(x0 + (x1 - x0) * t)), int(math.floor(y0 + (y1 - y0) * t))))
    return out


def arc(cx, cy, r, a0, a1, thickness=1.0):
    """Pixels of a circle's ring between angles a0..a1 (degrees, 0 = right, 90 = down)."""
    out = set()
    for (x, y) in disc(cx, cy, r + thickness / 2):
        d = math.hypot(x + 0.5 - cx, y + 0.5 - cy)
        a = math.degrees(math.atan2(y + 0.5 - cy, x + 0.5 - cx)) % 360
        if d >= r - thickness / 2 and (a0 <= a <= a1 or a0 <= a + 360 <= a1):
            out.add((x, y))
    return out


def shift(pixels, dx, dy):
    return {(x + dx, y + dy) for (x, y) in pixels}


# --- Heroes (spec §21, §22; the user asked for small old-school heroes): 14x16 art pixels from templates, a 1 art-px
# outline baked in, then doubled: 32x36 real px. The 8x14 dp hitbox covers art columns 3-10, rows 3-16 (with the outline). ---
HERO_FRAMES = ["IDLE_1", "IDLE_2", "WALK_1", "WALK_2", "WALK_3", "WALK_4", "JUMP", "FALL", "ATTACK_1", "ATTACK_2",
               "ATTACK_3", "CLIMB_1", "CLIMB_2", "ROLL_1", "ROLL_2", "HURT"]

# Template keys: A main, B dark, C cape/back, T trim (job colour), G hand, L front leg, K back leg, F feet, S skin,
# E eye, H hair, X the head seen from behind.
HEADS = {   # rows 0-6, facing right
    "KNIGHT": ["....TT......", "...TAAA.....", "..AAAAAA....", "..AABBBB....", "..AAAEEA....", "..AAAAAA....", "...BAAB....."],
    "ARCHER": ["............", "....AAA.....", "...AAAAA....", "..AAAHSS....", "..AASSES....", "..BAASSS....", "..B.AAA....."],
    "MAGICIAN": ["..AA........", "...AAA......", "...AAAA.....", ".BBTTTTBB...", "..HSSES.....", "..HSSSS.....", "...SSS......"],
    "THIEF": ["............", "..H.H.H.....", "..HHHHHH....", ".HHHHSSS....", "..HHSSES....", "..HBBBBB....", "...CCCC....."],
}
TORSOS = {   # rows 7-11
    "down": ["...AAAA.....", "..AAAAAB....", "..AAAAAB....", "..AAAAAG....", "...TTTTT...."],
    "fwd": ["...AAAA.....", "..AAAAABB...", "..AAAA..BG..", "..BAAA......", "...TTTT....."],
    "back": ["...AAAA.....", ".BBAAAAA....", "GB.AAAAA....", "...AAAAA....", "...TTTT....."],
    "up": [".G.AAAA.G...", ".BAAAAAAB...", "...AAAAA....", "...AAAAA....", "...TTTT....."],
    "out": ["...AAAA.....", "GBBAAAABBG..", "...AAAAA....", "...AAAAA....", "...TTTT....."],
    "windup": [".GBAAAA.....", "..BAAAAA....", "...AAAAA....", "...AAAAB....", "...TTTT....."],
    "strike": ["...AAAA.....", "...AAAABBBG.", "...AAAAA....", "...AAAAB....", "...TTTT....."],
    "follow": ["...AAAA.....", "...AAAAB....", "...AAAA.BB..", "...AAAA...G.", "...TTTT....."],
    "hurt": ["G..AAAA..G..", ".BBAAAABB...", "...AAAAA....", "...AAAAA....", "...TTTT....."],
}
LEGS = {   # rows 12-15
    "stand": ["...KK.LL....", "...KK.LL....", "...KK.LL....", "..FFF.FFF..."],
    "strideA": ["...KKLLL....", "..KK...LL...", ".KK.....LL..", ".FF.....FFF."],
    "strideB": ["...LLKKK....", "..LL...KK...", ".LL.....KK..", ".FF.....FFF."],
    "passA": ["....LLL.....", "....LLK.....", "....LKK.....", "...FFFF....."],
    "passB": ["....KKL.....", "....KLL.....", "....KLL.....", "...FFFF....."],
    "tuck": ["...LLLLL....", "..KK..LLL...", "..FF...FF...", "............"],
    "dangle": ["...KK.LL....", "...KK..LL...", "...KK..LL...", "..FFF...FF.."],
}
# Back views for climbing (CLIMB_2 is CLIMB_1 mirrored), and the tucked roll (ROLL_2 is ROLL_1 turned half a circle).
CLIMB = ["..G.........", "..B..XXX....", "..BXXXXXX...", "..BXXXXXXBG.", "...XXXXXXB..", "....XXXX....", "...AAAAAA...",
         "...ACCCCA...", "...CCCCCC...", "...CCCCCC...", "...CCCCCC...", "...TTTTTT...", "...LL..KK...", "...LL..KK...",
         "...FF..KK...", ".......FF..."]
ROLL = ["............", "............", "............", "............", "............", "............", "............",
        "....AAAA....", "...AAXXAA...", "..AAXXXXCA..", "..AXXSSXCA..", "..AAXXXXAA..", "..AALLLLAA..", "..CALLLLAC..",
        "...AAAAAA...", "....AAAA...."]
# Which torso and legs each frame uses, how far the upper body sinks (bob), the cape flutter, the weapon direction.
FRAMES = {
    "IDLE_1": ("down", "stand", 0, 0, "down"), "IDLE_2": ("down", "stand", 1, 0, "down"),
    "WALK_1": ("fwd", "strideA", 1, 1, "down"), "WALK_2": ("down", "passA", 0, 0, "down"),
    "WALK_3": ("back", "strideB", 1, 1, "down"), "WALK_4": ("down", "passB", 0, 0, "down"),
    "JUMP": ("up", "tuck", 0, 0, "up"), "FALL": ("out", "dangle", 0, 1, "up"),
    "ATTACK_1": ("windup", "stand", 0, 0, "back"), "ATTACK_2": ("strike", "strideA", 1, 1, "fwd"),
    "ATTACK_3": ("follow", "strideA", 1, 0, "low"), "HURT": ("hurt", "dangle", 0, 1, "up"),
}
COLOURS = {   # key → palette character, per class
    "KNIGHT": dict(A="b", B="d", C="r", T="h", G="s", L="s", K="d", F="&", S="@", E="k", H="&", X="b"),
    "ARCHER": dict(A="g", B="G", C="P", T="h", G="@", L="P", K="W", F="&", S="@", E="k", H="&", X="g"),
    "MAGICIAN": dict(A="u", B="U", C="U", T="h", G="@", L="u", K="U", F="&", S="@", E="k", H="&", X="u"),
    "THIEF": dict(A="X", B="x", C="p", T="h", G="@", L="X", K="x", F="x", S="@", E="k", H="&", X="&"),
}
CLASSES = {"KNIGHT": dict(outline="x"), "ARCHER": dict(outline="x"), "MAGICIAN": dict(outline="x"), "THIEF": dict(outline="k")}
BACKS = {   # drawn behind the body (only into empty cells), rows 6-12: rest, flutter
    "KNIGHT": (["............", "............", "..C.........", ".CC.........", ".CC.........", "CCC.........", "CC.........."],
               ["............", "............", "..C.........", ".CC.........", "CCC.........", "CC..........", "............"]),
    "ARCHER": ([".b..........", "..C.........", "..C.........", "..C.........", "..C.........", "............", "............"],) * 2,
    "MAGICIAN": (["............"] * 7,) * 2,
    "THIEF": (["............", "..C.........", ".CC.........", "CC..........", "............", "............", "............"],
              ["............", ".CC.........", "CC..........", "............", "............", "............", "............"]),
}
WEAPON_DIRS = {"down": (1, 1), "up": (0, -1), "back": (-1, -1), "fwd": (1, 0), "low": (1, 1)}


def hero(cls, frame):
    col = COLOURS[cls]
    rows = [list("." * 14) for _ in range(16)]
    def stamp(lines, top, only_empty=False):
        for r, text in enumerate(lines):
            for c, k in enumerate(text):
                if k != "." and 0 <= top + r < 16 and (not only_empty or rows[top + r][c] == "."):
                    rows[top + r][c] = k
    if frame.startswith("CLIMB"):
        stamp([t if frame == "CLIMB_1" else t[::-1] for t in CLIMB], 0)
    elif frame.startswith("ROLL"):
        stamp(ROLL if frame == "ROLL_1" else [t[::-1] for t in ROLL[7:][::-1]], 0 if frame == "ROLL_1" else 7)
    else:
        torso, legs, bob, flutter, weapon_dir = FRAMES[frame]
        stamp(LEGS[legs], 12)
        stamp(TORSOS[torso], 7 + bob)
        stamp(HEADS[cls], bob)
        if frame == "HURT":
            for r in range(7):
                rows[r] = [{"E": "B"}.get(k, k) for k in rows[r]]   # eyes squeezed
        stamp(BACKS[cls][flutter], 6 + bob, only_empty=True)
        hand = next(((c, r) for r in range(16) for c in range(14) if rows[r][c] == "G"), None)
        if hand:
            weapon(rows, cls, hand, WEAPON_DIRS[weapon_dir], frame)
    g = Grid(16, 18)   # the template inside a 1 px border, the outline in that border and around every shape
    for r in range(16):
        for c in range(14):
            k = rows[r][c]
            if k != ".":
                g.set(c + 1, r + 1, col.get(k, k))
    outline = CLASSES[cls]["outline"]
    for (x, y) in [(x, y) for y in range(18) for x in range(16)]:
        if g.get(x, y) == "." and any(g.get(x + dx, y + dy) not in (".", "#") for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
            g.set(x, y, "#")
    big = Grid(32, 36)
    for y in range(18):
        for x in range(16):
            ch = g.get(x, y)
            for dy in range(2):
                for dx in range(2):
                    big.set(x * 2 + dx, y * 2 + dy, outline if ch == "#" else ch)
    return big


def weapon(rows, cls, hand, direction, frame):
    """The class weapon from the hand: sword, bow, staff or dagger ('w'/'b' blades, 'h' trim bow, '&' wood)."""
    hx, hy = hand
    dx, dy = direction
    def put(x, y, k):
        if 0 <= x < 14 and 0 <= y < 16 and rows[y][x] == ".":
            rows[y][x] = k
    if cls == "KNIGHT":
        for i in range(1, 5):
            put(hx + dx * i, hy + dy * i, "w" if i == 4 else "b")
        put(hx - dy, hy + dx, "h")   # crossguard
    elif cls == "ARCHER":
        bx = hx + 1
        for y in range(hy - 2, hy + 3):
            put(bx + (1 if abs(y - hy) <= 1 else 0), y, "h")
        if dx == 1 and dy == 0:   # drawn: an arrow on the string
            for i in range(1, 4):
                put(hx + 2 + i, hy, "w" if i == 3 else "b")
    elif cls == "MAGICIAN":
        for y in range(hy - 4, hy + 4):
            put(hx + (1 if dx > 0 else 0), y, "&")
        put(hx + (1 if dx > 0 else 0), hy - 5, "c")
        put(hx + (1 if dx > 0 else 0), hy - 6, "w")
    else:   # THIEF: a short dagger along the same direction
        for i in range(1, 3):
            put(hx + dx * i, hy + dy * i, "w" if i == 2 else "b")


# --- Objects (global palette, outlined when drawn) ---
def flames(g, cx, base, w, h, seed):
    """A fire: layered tongues, deep red outside to white-hot inside; seed varies the frame."""
    rnd = random.Random(seed)
    layers = [("Z", 1.0), ("O", 0.74), ("N", 0.48), ("w", 0.22)]
    tongues = [(cx + rnd.uniform(-w * 0.35, w * 0.35), h * rnd.uniform(0.55, 1.0)) for _ in range(4)]
    tongues.append((cx, h))
    for ch, scale in layers:
        shape = set()
        for (tx, th) in tongues:
            tw = w * 0.32 * scale + 0.6
            tip_x = tx + rnd.uniform(-1.5, 1.5)
            shape |= poly([(tx - tw, base), (tx + tw, base), (tip_x, base - th * scale)])
            shape |= disc(tx, base - tw * 0.6, tw, tw * 0.8)
        g.fill(shape, ch)


def stone_ring(g, cx, base, w, h, ramp=("*", "%", "%")):
    """A ring of fitted stones, drawn as rows of rounded blocks."""
    rows = int(h // 4)
    for r in range(rows):
        y = base - (r + 1) * 4
        width = w - r * 2
        x = cx - width / 2
        offset = 0 if r % 2 == 0 else 3
        xs = x - offset
        while xs < cx + width / 2:
            sw = 6 if (int(xs) + r) % 3 else 5
            block = {(px, py) for (px, py) in rect(int(xs), int(y), sw, 4) if cx - width / 2 <= px < cx + width / 2}
            block -= {(int(xs), int(y)), (int(xs) + sw - 1, int(y) + 3)}
            g.paint(block, ramp)
            xs += sw


def hearth(n):
    g = Grid(48, 56)
    stone_ring(g, 24, 56, 40, 16)
    g.paint(rect(8, 36, 32, 4), ("=", "*", "%"))
    g.fill(rect(10, 37, 28, 2), "=")
    g.paint(capsule(14, 38, 34, 36, 1.6) | capsule(14, 36, 34, 38, 1.6), ("W", "P", "S"))   # logs
    flames(g, 24, 38, 22, 34, n * 7 + 3)
    for (x, y) in [(11, 37), (37, 38), (16, 39)]:
        g.set(x, y, "O")
    return g


def brazier(lit, n=0):
    g = Grid(32, 40)
    g.paint(poly([(12, 40), (20, 40), (18, 34), (14, 34)]), ("x", "d", "s"))   # foot
    g.paint(rect(15, 24, 2, 11), ("x", "d", "s"))
    g.paint(poly([(6, 18), (26, 18), (22, 25), (10, 25)]), ("x", "d", "s"))   # bowl
    g.fill(line(7, 18, 25, 18), "s")
    for x in (9, 13, 17, 21):
        g.set(x, 21, "x")
    if lit:
        flames(g, 16, 18, 16, 17, n * 5 + 1)
    else:
        for x in range(9, 24):
            g.set(x, 17, "K" if x % 3 else "D")
        g.set(14, 16, "D")
        g.set(18, 16, "K")
    return g


def arch_stone(g):
    """The carved arch shared by portals and closed arches: 32x56."""
    outer = poly([(2, 56), (2, 20), (6, 9), (16, 3), (26, 9), (30, 20), (30, 56)])
    inner = poly([(8, 56), (8, 22), (11, 14), (16, 10), (21, 14), (24, 22), (24, 56)])
    g.paint(outer - inner, ("!", "X", "s"))
    for y in range(24, 56, 7):   # block seams
        g.fill(rect(2, y, 6, 1), "!")
        g.fill(rect(24, y, 6, 1), "!")
    g.set(16, 5, "z")
    g.fill(rect(15, 6, 3, 1), "!")
    return inner


def portal(n):
    g = Grid(32, 56)
    inner = arch_stone(g)
    for (x, y) in inner:
        d = math.hypot(x + 0.5 - 16, (y + 0.5 - 34) * 0.55)
        swirl = (d * 1.7 + math.atan2(y - 34, x - 16) * 2.2 + n * 1.6) % 4
        g.set(x, y, "c" if swirl < 0.8 else "p" if swirl < 2.2 else "U")
    for (x, y) in disc(16, 34, 2.6):
        g.set(x, y, "w")
    return g


def arch_closed():
    g = Grid(32, 56)
    inner = arch_stone(g)
    g.paint(inner, ("=", "!", "!"))
    for y in range(20, 56, 6):
        g.fill({(x, yy) for (x, yy) in rect(8, y, 16, 1) if (x, yy) in inner}, "=")
    g.fill(line(14, 30, 18, 36) | line(18, 30, 14, 36), "X")   # a carved rune
    return g


def chest(opened):
    g = Grid(24, 20)
    g.paint(rect(2, 9, 20, 11), ("W", "P", "S"))
    g.fill(rect(2, 13, 20, 1), "W")
    if opened:
        g.paint(poly([(2, 9), (22, 9), (22, 2), (2, 5)]), ("W", "P", "S"))
        g.fill(rect(4, 7, 16, 2), "x")
        for x in range(6, 18, 3):
            g.set(x, 8, "z")
    else:
        g.paint(poly([(2, 9), (22, 9), (21, 3), (3, 3)]), ("W", "P", "S"))
    for x in (5, 18):   # iron bands
        g.paint(rect(x, 3 if not opened else 9, 2, 17 if not opened else 11), ("d", "s", "b"))
    g.paint(rect(10, 8, 4, 5), ("Y", "z", "N"))   # lock
    g.set(11, 10, "x")
    return g


# --- NPCs (spec §14, §22): like the heroes, small old-school templates with a baked outline, doubled, facing right.
# 14x18 art px (16x20 dp with the outline); Tobin 10x10 (12x12 dp), the Gatekeeper 14x22 (16x24 dp). Frame B breathes
# (head and shoulders 1 px lower) and blinks. Keys map to the global palette per NPC. ---
NPC_TEMPLATES = {
    "ILSA": ([".....AAA......", "....AAAAA.....", "...AAAAAAA....", "...AABSSSA....", "...AASSESA....", "...AASSSSA....",
              "...AAA$$AA....", "..AAAAAAAAA...", "..AAAAAAAAAG..", "..BAAAAAAAA.L.", "..BAAAAAAAA.L.", "..BAAAAAAAALOL",
              "..BAAAAAAAALNL", "..BBAAAAAAA.L.", "..BBAAAAAAAA..", "..BBBAAAAAAA..", "...BBBAAAAA...", "...FF...FF...."],
             dict(A="7", B="4", S="@", E="k", F="&", G="@", L="K", O="o", N="N")),
    "MARRA": (["....HHHH......", "...HHHHHH.....", "...HHSSSS.....", "...HSSESS.....", "...HSSSSS.....", "....$SSS......",
               "...CCCCCC.....", "..CAAAAAAC....", "..AAAAAAAAG...", "..AAWWWWAA.ll.", "..AAWWWWAA.l..", "..AAWWWWAA....",
               "..AAWWWWAA....", "..AAAAAAAA....", "..BAAAAAAB....", "..BBAAAABB....", "...BB..BB.....", "...FF..FF....."],
              dict(H="h", S="@", E="k", A="g", B="G", C="P", W="b", G="@", l="l", F="&")),
    "BRANN": (["....HHHH......", "...HHHHHH.....", "...HSSSSS..XXX", "...HSSESS..XXX", "...SSSSSS...T.", "...DDDDDD...T.",
               "..SDDDDDDS..T.", ".SSAWWWWASS.T.", ".SSAWWWWAASSG.", "..SAWWWWAA....", "..AAWWWWAA....", "..AAWWWWAA....",
               "..AAWWWWAA....", "..BBWWWWBB....", "..BBBBBBBB....", "..BB....BB....", "..BB....BB....", ".FFF...FFF...."],
              dict(H="&", S="@", E="k", D="h", A="X", W="W", B="d", X="s", T="&", G="@", F="x")),
    "LIO": (["........r.....", "...CCCCCr.....", "..CCCCCCC.....", "...HSSSSS.....", "...HSSESS.....", "...HSSSSS.....",
             "....$SS$......", "...AAAAAA.....", "..AAAAAAAA....", "..AALLLLLLLLG.", "..AOOOAAAA....", "..OOOOOAAA....",
             "..OOkOOAAA....", "..AOOOAAAA....", "..BBBBBBBB....", "...KK..JJ.....", "...KK..JJ.....", "..FFF..FFF...."],
            dict(r="Y", C="r", H="H", S="@", E="k", A="u", B="U", L="&", O="S", k="x", K="U", J="u", G="@", F="&")),
    "PELL": (["....HHHH......", "...HHHHHH.....", "...HSSSSS.....", "...HSwEwS.....", "...HSSSSS.....", "....$SS$......",
              "...AAAAAA.....", "..AAAAAAAAOO..", ".OOOOOOOOOOOC.", "COOOOOOOOOOOC.", ".OOOOOOOOOOOC.", "..AAAAAAAAG...",
              "..AAAAAAAA....", "..AAAAAAAA....", "..BBBBBBBB....", "...KK..JJ.....", "...KK..JJ.....", "..FFF..FFF...."],
             dict(H="s", S="@", E="k", w="b", A="h", B="W", O="H", C="S", G="@", K="W", J="h", F="&")),
    "SELA": (["..............", "....HHHH......", "...HHHHHH.....", "...HHSSSS.....", "...HSSESS.....", "...HSSSS$.....",
              "..CCC$$CC.....", ".CCCCCCCCC....", ".CCCCCCCCCG...", "..CCCCCCCC.T..", "..AAAAAAAA.T..", "..AAAAAAAA.T..",
              "..AAAAAAAA.T..", "..AAAAAAAA.T..", "..BAAAAAAB.T..", "..BBAAAABB.T..", "..BBBBBBBB.T..", "...FF..FF..T.."],
             dict(H="b", S="@", E="k", C="p", A="d", B="n", G="@", T="&", F="&")),
    "ODO": (["....AAAA......", "...AAAAAA.....", "...ABBBBA.....", "...ASSESA.....", "...ASSSSA.....", "...HHHHHH.....",
             "...HHHHH......", "..AAAAAAAA....", ".AACCCCCCAA...", ".AACCCCCCAG...", ".A.CCCCCC.T...", "...CCCCCC.T...",
             "...CCCCCC.T...", "...AAAAAA.T...", "...AA..AA.T...", "...AA..AA.T...", "...AA..AA.T...", "..FFF.FFF.L..."],
            dict(A="b", B="s", S="@", E="k", H="w", C="r", G="s", T="b", L="s", F="d")),
    "WREN": ([".....AAA......", "....AAAAA.....", "...AAAAAAA....", "...AASSSSA....", "...AASSESS....", "...AASSSSS....",
              "..LAAA$$A.....", ".L.AAAAAAA....", ".L.ABBBBBA....", ".L.ABBBBBAG...", ".L.ABBBBBA....", ".L.ABBBBBA....",
              "..LACCCCCA....", "...ABBBBBA....", "...KK..JJ.....", "...KK..JJ.....", "...KK..JJ.....", "..FFF..FFF...."],
             dict(A="G", B="g", S="@", E="k", L="h", C="&", G="@", K="W", J="&", F="&")),
    "YSOLDE": (["......A....N..", ".....AA...NcN.", "....AAA....T..", "...AAAAA...T..", "..CCCCCCC..T..", "...HSSSS...T..",
                "...HSSES...T..", "...HHSS$...T..", "..AAAAAAA..T..", "..AAAAAAAAGT..", "..AAAAAAA..T..", "..AACCAAA..T..",
                "..AAAAAAA..T..", "..AAAAAAAA.T..", "..BAAAAAAA.T..", ".BBAAAAAAAAT..", ".BBBBBBBBBBT..", "...FF..FF..T.."],
               dict(A="p", B="U", C="U", H="b", S="@", E="k", N="w", c="c", T="&", G="@", F="&")),
    "KIT": (["..............", "...HHHHH......", "..HHHHHHH.....", "..HHSSSSS.....", "..HCCwCCw.....", "..HSSSSSS.....",
             "...DDDDDDD....", "..DDAAAADDD...", "..AAAAAAA.DD..", "..AAAAAAAG....", "..AAAAAAA.w...", "..ACCCCCA.....",
             "..AAAAAAA.....", "...KK..JJ.....", "...KK...JJ....", "...KK...JJ....", "..KK.....JJ...", "..FF.....FF..."],
            dict(H="&", S="@", C="x", w="w", D="R", A="d", G="@", K="n", J="d", F="x")),
    "TOBIN": (["...HHHH...", "..HHHHHH..", "..HSSSSS..", "..HSSESS..", "...SSSS...", "..AAAAAA..", ".SAAAAAAS.", "..AAAAAA..",
               "..KK..JJ..", "..FF..FF.."],
              dict(H="H", S="@", E="k", A="o", K="h", J="h", F="&")),
    # Tobin's father, an old miner sitting hurt in the Greendeep: grey beard, a yellow cap with a lamp, a bandaged leg.
    "GARRICK": (["....CCCCL.....", "...CCCCCC.....", "...HSSSSS.....", "...HSSESS.....", "...HSSSSS.....", "...WWWWWW.....",
                 "..AAWWWWAA....", ".SAAAAAAAS....", ".SAAAAAAAS....", "..AAAAAAAA....", "..KKKKKBBBBFF.", "..KKKKKKKKKFF."],
                dict(C="Y", L="w", H="s", S="@", E="k", W="b", A="R", K="U", B="w", F="&")),
    "GATEKEEPER": ([".....AAAA.....", "....AAAAAA....", "....ABBBBA....", "....AwBBwA....", "....ABBBBA....", ".....AAAA.....",
                    "...AAAAAAAA...", "..AAAAAAAAAA..", "..AA.AAAA.AA..", "..AA.AAAA.AA..", "..AAGAAAAGAA..", "..A..ATTA..A..",
                    ".....ATTA.....", "....AAAAAA....", "....AATTAA....", "....AATTAA....", "...AAATTAAA...", "..CCCCCCCCCC..",
                    "..CoCoCoCoCo..", ".CCCCCCCCCCCC.", ".DDDDDDDDDDDD.", "DDDDDDDDDDDDDD"],
                   dict(A="s", B="d", w="c", G="s", T="b", C="X", o="!", D="d")),
    "SURVEYOR": (["....CCCC......", "..CCCCCCCC....", "...HSSSSS.....", "...HSSESS.....", "...HSSSSS..LL.", "....$SS$..LL..",
                  "...AAAAAA.LL..", "..AAAAAAAGL...", "..AOAAAAAA....", "..AAOAAAAA....", ".DDDOAAAAA....", ".DDDDAAAAA....",
                  ".DDDDAAAAA....", "..AAAAAAAA....", "...KK..JJ.....", "...KK..JJ.....", "...KK..JJ.....", "..FFF..FFF...."],
                 dict(C="W", H="&", S="@", E="k", A="P", O="&", D="S", L="z", G="@", K="W", J="P", F="&")),
}


def npc(name, frame):
    rows, colours = NPC_TEMPLATES[name]
    rows = [list(r) for r in rows]
    w, h = len(rows[0]), len(rows)
    assert all(len(r) == w for r in rows), name + ": rows of different widths"
    if frame == 1 and name != "GATEKEEPER":   # breathe: the head and shoulders 1 px lower; blink
        top = 8 if h >= 18 else 5
        rows = [["."] * w] + rows[:top - 1] + rows[top:]
        rows = [["S" if k == "E" else k for k in r] for r in rows]
    g = Grid(w + 2, h + 2)
    for r in range(h):
        for c in range(w):
            k = rows[r][c]
            if k != ".":
                col = colours.get(k, {"S": "@", "$": "$", "E": "k"}.get(k, k))
                if name == "GATEKEEPER" and frame == 1 and k == "w":   # the statue doesn't breathe: its eyes glow
                    col = "w"
                g.set(c + 1, r + 1, col)
    for (x, y) in [(x, y) for y in range(h + 2) for x in range(w + 2)]:
        if g.get(x, y) == "." and any(g.get(x + dx, y + dy) not in (".", "#") for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
            g.set(x, y, "#")
    big = Grid((w + 2) * 2, (h + 2) * 2)
    for y in range(h + 2):
        for x in range(w + 2):
            ch = g.get(x, y)
            for dy in range(2):
                for dx in range(2):
                    big.set(x * 2 + dx, y * 2 + dy, "x" if ch == "#" else ch)
    return big


def lever(on):
    """A shortcut lever (8x14 dp): an iron box with a handle, tilted left (closed) or right (pulled)."""
    g = Grid(16, 28)
    g.paint(rect(3, 20, 10, 8), ("x", "d", "s"))
    g.fill(rect(4, 21, 8, 1), "b")
    tip = (13, 7) if on else (3, 7)
    g.fill(line(8, 21, tip[0], tip[1]) | line(9, 21, tip[0] + 1, tip[1]), "h")
    g.paint(disc(tip[0] + 0.5, tip[1], 2.2), ("R", "r", "r"))
    return g


def chest_ajar():
    g = Grid(24, 20)
    g.paint(rect(2, 9, 20, 11), ("W", "P", "S"))
    g.fill(rect(2, 13, 20, 1), "W")
    g.paint(poly([(2, 9), (22, 9), (22, 4), (3, 3)]), ("W", "P", "S"))
    g.fill(rect(4, 8, 16, 1), "z")
    for x in (5, 18):
        g.paint(rect(x, 9, 2, 11), ("d", "s", "b"))
    g.paint(rect(10, 8, 4, 5), ("Y", "z", "N"))
    return g


def item_icon(kind):
    """The HUD's 8x8 dp item icons (spec §15), authored at 16x16."""
    g = Grid(16, 16)
    if kind in ("RED", "BIG", "MANA"):
        body = {"RED": ("R", "r", "o"), "BIG": ("R", "r", "o"), "MANA": ("U", "u", "c")}[kind]
        r = 5.5 if kind == "BIG" else 4.5
        g.paint(disc(8, 15.5 - r, r), body)
        g.paint(rect(6, 2 if kind == "BIG" else 4, 4, 4), ("d", "b", "w"))
        g.fill(rect(6, 1 if kind == "BIG" else 3, 4, 2), "h")
        g.set(6, 15 - int(r * 1.4), "w")
    elif kind == "WHETSTONE":
        g.paint(poly([(1, 13), (11, 4), (15, 8), (5, 15)]), ("d", "s", "b"))
        g.fill(line(4, 12, 11, 6), "b")
    elif kind == "TONIC":
        g.paint(rect(5, 5, 6, 10), ("x", "X", "s"))
        g.paint(rect(6, 2, 4, 3), ("d", "s", "b"))
        g.fill(rect(6, 9, 4, 2), "R")
    else:   # SWIFT FEATHER
        g.paint(poly([(2, 15), (5, 7), (11, 1), (13, 3), (9, 10)]), ("c", "b", "w"))
        g.fill(line(2, 15, 11, 3), "s")
    return g


# --- Output ---
def sprite_text(name, g, prefix="const Sprite"):
    rows = ['    "%s"' % "".join(g.c[y]) for y in range(g.h)]
    assert not any("??" in r for r in rows), name + ": '??' would start a C++ trigraph"
    return "%s %s = {%d, %d,\n%s};\n" % (prefix, name, g.w, g.h, "\n".join(rows))


def write(path, text):
    with open(path, "w") as f:
        f.write(text)
    print("wrote", os.path.relpath(path, ROOT))


def build_objects():
    out = []
    for cls in CLASSES:
        for frame in HERO_FRAMES:
            out.append(("HERO_%s_%s" % (cls, frame), hero(cls, frame)))
    out.append(("CHECKPOINT_UNLIT", brazier(False)))
    for i, n in enumerate("ABC"):
        out.append(("CHECKPOINT_LIT_" + n, brazier(True, i)))
    for i, n in enumerate("ABC"):
        out.append(("HEARTH_" + n, hearth(i)))
    out.append(("PORTAL_A", portal(0)))
    out.append(("PORTAL_B", portal(1)))
    out.append(("ARCH_CLOSED", arch_closed()))
    out.append(("CHEST_CLOSED", chest(False)))
    out.append(("CHEST_OPEN", chest(True)))
    out.append(("CHEST_AJAR", chest_ajar()))
    out.append(("LEVER_OFF", lever(False)))
    out.append(("LEVER_ON", lever(True)))
    for name in NPC_TEMPLATES:
        for i, n in enumerate("AB"):
            out.append(("NPC_%s_%s" % (name, n), npc(name, i)))
    for kind in ("RED", "BIG", "MANA", "WHETSTONE", "TONIC", "FEATHER"):
        out.append(("ICON_" + kind, item_icon(kind)))
    return out


def main():
    import namula   # Namula's world art (same folder)
    import world1   # The Drifting Station's art, enemies and bosses
    import world2   # The Greendeep's art, enemies and bosses
    import world3   # The White Silence's art, enemies and bosses
    objects = build_objects()
    header = ["// NAME QUEST sprites (spec §21, §22), generated by docs/work-plan/name-quest/art-tools/make_quest_art.py: edit that",
              "// script, not this file. Heroes are 14x16 art px doubled (32x36 real px with their baked outline), facing right; their trim",
              "// is 'h' only (recoloured by job tier).",
              "#pragma once", "#include \"sprites.h\"", ""]
    for name, g in objects:
        header.append(sprite_text(name, g))
    header.append("// Namula's world art (palette sprites, '0'-'f'): quest_sprites_0.cpp.")
    header.append("#include \"quest_sprites_0.h\"")
    write(os.path.join(SOURCE, "quest_sprites.h"), "\n".join(header) + "\n")
    namula.write_all(write, sprite_text, ROOT)
    world1.write_all(write, sprite_text, ROOT)
    world2.write_all(write, sprite_text, ROOT)
    world3.write_all(write, sprite_text, ROOT)
    if "--preview" in sys.argv:
        folder = sys.argv[sys.argv.index("--preview") + 1]
        preview_objects(objects, folder)
        namula.preview(folder, save_png)
        world1.preview(folder, save_png, global_palette)
        world2.preview(folder, save_png, global_palette)
        world3.preview(folder, save_png, global_palette)


# --- Previews (PNG, for checking the art by eye) ---
def global_palette():
    with open(os.path.join(SOURCE, "sprites.h")) as f:
        text = f.read()
    return {c: ((int(v, 16) >> 16) & 255, (int(v, 16) >> 8) & 255, int(v, 16) & 255)
            for c, v in re.findall(r"case '(.)': return 0x([0-9A-Fa-f]{8});", text)}


def save_png(path, w, h, rgb_rows, scale=1):
    raw = b""
    for row in rgb_rows:
        line_bytes = b"".join(bytes(px) * scale for px in row)
        raw += (b"\x00" + line_bytes) * scale
    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)
    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w * scale, h * scale, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(png)


def preview_objects(objects, folder):
    pal = global_palette()
    bg = (40, 36, 60)
    heroes = [g for name, g in objects if name.startswith("HERO_")]
    cols = 16
    w, h = cols * 36, 4 * 44
    img = [[bg] * w for _ in range(h)]
    for i, g in enumerate(heroes):
        ox, oy = (i % cols) * 36 + 2, (i // cols) * 44 + 2
        for y in range(g.h):
            for x in range(g.w):
                if g.c[y][x] != ".":
                    img[oy + y][ox + x] = pal[g.c[y][x]]
    save_png(os.path.join(folder, "heroes.png"), w, h, img, 3)
    rest = [g for name, g in objects if not name.startswith("HERO_")]
    w = sum(g.w + 4 for g in rest)
    img = [[bg] * w for _ in range(60)]
    x0 = 0
    for g in rest:
        for y in range(g.h):
            for x in range(g.w):
                if g.c[y][x] != ".":
                    img[y + 2][x0 + x + 2] = pal[g.c[y][x]]
        x0 += g.w + 4
    save_png(os.path.join(folder, "objects.png"), w, 60, img, 3)


if __name__ == "__main__":
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    main()
