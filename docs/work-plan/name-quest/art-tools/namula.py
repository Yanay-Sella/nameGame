"""Namula's world art (spec §13 row 0, §22), drawn with the world palette: '0'-'3' stone, '4'-'7' moss, '8'-'b' night,
'c'-'f' firelight (dark → light in each ramp). Used by make_quest_art.py, which writes source/quest_sprites_0.h/.cpp."""
import math
import os
import random

from make_quest_art import Grid, arc, capsule, disc, line, poly, rect

PALETTE = [0x1F1A1D, 0x3B3133, 0x63544E, 0x948270,    # stone: cool grey-brown
           0x14241C, 0x22402A, 0x3F6634, 0x7A9A4C,    # foliage: moss green
           0x0B0B19, 0x16152B, 0x262241, 0x3F3863,    # night: deep blue-violet
           0x7A4020, 0xD9842E, 0xF5BC52, 0xFFEBC0]    # firelight: amber to warm white (lights and windows)
STONE, MOSS, NIGHT, FIRE = ("1", "2", "3"), ("5", "6", "7"), ("9", "a", "b"), ("c", "d", "e")
WOOD = ("1", "2", "3")


def rng(*key):
    return random.Random("namula-" + "-".join(str(k) for k in key))


# --- Ground tiles (16x16): a cobbled cap with moss over packed earth with buried stones ---
SHAPES = ["TOP", "TOP_LEFT", "TOP_RIGHT", "LEFT", "RIGHT", "FILL", "BOTTOM", "BOTTOM_LEFT", "BOTTOM_RIGHT", "COLUMN_TOP",
          "COLUMN", "SINGLE"]
OPEN = {   # which sides are open (air): top, left, right, bottom
    "TOP": "t", "TOP_LEFT": "tl", "TOP_RIGHT": "tr", "LEFT": "l", "RIGHT": "r", "FILL": "", "BOTTOM": "b",
    "BOTTOM_LEFT": "bl", "BOTTOM_RIGHT": "br", "COLUMN_TOP": "tlr", "COLUMN": "lr", "SINGLE": "tlrb"}


def earth(g, r, y0=0):
    """Packed earth: dark speckle, a few buried stones."""
    for y in range(y0, 16):
        for x in range(16):
            v = r.random()
            g.set(x, y, "0" if v < 0.07 else "2" if v > 0.985 else "1")
    for _ in range(r.choice((1, 2, 2))):
        w, h = r.randint(3, 5), r.randint(2, 3)
        x, y = r.randint(1, 15 - w), r.randint(max(y0 + 1, 1), 15 - h)
        stone = disc(x + w / 2, y + h / 2, w / 2 + 0.2, h / 2 + 0.2)
        g.paint(stone, ("0", "2", "3") if r.random() < 0.6 else ("0", "1", "2"))


def cobbles(g, r):
    """Rows 0-8: a moss lip over two rows of rounded cobbles, moss in some cracks."""
    for x in range(16):
        g.set(x, 0, "7" if r.random() < 0.25 else "6")
        g.set(x, 1, "6" if r.random() < 0.5 else "5")
    for row, (y, h) in enumerate(((2, 3), (5, 3))):
        x = -r.randint(0, 3) - (2 if row else 0)
        while x < 16:
            w = r.randint(4, 6)
            stone = rect(x, y, w - 1, h) - {(x, y), (x + w - 2, y), (x, y + h - 1), (x + w - 2, y + h - 1)}
            g.paint({p for p in stone if 0 <= p[0] < 16}, ("1", "2", "3"))
            for yy in range(y, y + h):   # the crack after the stone
                if 0 <= x + w - 1 < 16:
                    g.set(x + w - 1, yy, "5" if r.random() < 0.45 else "0")
            for xx in (x, x + w - 2):   # rounded corners show the crack colour
                for yy in (y, y + h - 1):
                    if 0 <= xx < 16 and g.get(xx, yy) == ".":
                        g.set(xx, yy, "0")
            x += w
    for x in range(16):
        g.set(x, 8, "0" if r.random() < 0.7 else "5")
    for _ in range(2):   # moss creeping over a cobble
        x = r.randint(0, 14)
        g.set(x, 2, "6")
        g.set(x + 1, 2, "5")


def ground_tile(shape, variant):
    r = rng("tile", shape, variant)
    g = Grid(16, 16)
    sides = OPEN[shape]
    earth(g, r)
    if "t" in sides:
        cobbles(g, r)
    if "l" in sides or "r" in sides:
        for side in "lr":
            if side not in sides:
                continue
            for y in range(16):
                inset = 1 if r.random() < 0.3 else 0
                xs = [0, 1] if side == "l" else [15, 14]
                g.set(xs[0], y, "." if inset and "t" not in sides else "0")
                if g.get(xs[1], y) not in "567":
                    g.set(xs[1], y, "1" if r.random() < 0.6 else "0")
                if r.random() < 0.18:
                    g.set(xs[1], y, "5")   # moss on the side face
    if "b" in sides:
        for x in range(16):
            depth = r.choice((0, 1, 1, 2))
            for d in range(depth):
                g.set(x, 15 - d, ".")
            g.set(x, 15 - depth, "0")
    # Rounded open corners.
    if "t" in sides and "l" in sides:
        for (x, y) in ((0, 0), (1, 0), (0, 1)):
            g.set(x, y, ".")
        g.set(1, 1, "6")
        g.set(0, 2, "5")
    if "t" in sides and "r" in sides:
        for (x, y) in ((15, 0), (14, 0), (15, 1)):
            g.set(x, y, ".")
        g.set(14, 1, "6")
        g.set(15, 2, "5")
    if "b" in sides and "l" in sides:
        g.set(0, 15, ".")
        g.set(0, 14, ".")
        g.set(1, 15, ".")
    if "b" in sides and "r" in sides:
        g.set(15, 15, ".")
        g.set(15, 14, ".")
        g.set(14, 15, ".")
    return g


def fringe(variant):
    """Grass blades over a cap: rows 0-5 stand above the tile's top line, rows 6-7 lie over its moss lip."""
    r = rng("fringe", variant)
    g = Grid(16, 8)
    for x in range(16):
        g.set(x, 7, "6")
        if r.random() < 0.7:
            g.set(x, 6, "5" if r.random() < 0.5 else "6")
    x = r.randint(0, 2)
    while x < 16:
        h = r.choice((2, 3, 3, 4, 5, 6))
        lean = r.choice((-1, 0, 0, 1))
        for i in range(h):
            xx = x + (lean if i >= h - 2 else 0)
            g.set(xx, 7 - i, "7" if i == h - 1 else "6" if i > h // 2 else "5")
        x += r.randint(1, 3)
    if variant == 2:   # a tiny white flower
        g.set(11, 2, "f")
        g.set(10, 3, "f")
        g.set(12, 3, "f")
        g.set(11, 3, "e")
        g.set(11, 4, "6")
        g.set(11, 5, "6")
    return g


def drape(right):
    """Moss hanging down the side of an edge cap: 8x16, attached to the tile on its right (left drape)."""
    r = rng("drape", right)
    g = Grid(8, 16)
    for x in range(3, 8):
        length = r.choice((2, 3, 5, 7, 9, 12)) if x > 3 else r.choice((1, 2, 3))
        for y in range(length):
            g.set(x, y + 1, "7" if y == 0 and r.random() < 0.4 else "4" if y >= length - 2 else "5" if (x + y) % 3 else "6")
    for x in range(4, 8):
        g.set(x, 0, "6")
    if right:
        g.c = [row[::-1] for row in g.c]
    return g


# --- Platforms (16x12 pieces), supports, ladders, the sea ---
def plank(end):
    """A wooden plank piece: 'left', 'mid' or 'right'; ends carry rope lashings and a peg."""
    r = rng("plank", end)
    g = Grid(16, 12)
    body = rect(0, 0, 16, 10)
    if end == "left":
        body -= {(0, 0), (0, 9), (1, 0)}
    if end == "right":
        body -= {(15, 0), (15, 9), (14, 0)}
    g.paint(body, WOOD)
    for x in range(16):   # grain
        if (x, 4) in body and r.random() < 0.6:
            g.set(x, 4 if x % 7 < 4 else 5, "1")
        if (x, 7) in body and r.random() < 0.4:
            g.set(x, 7, "1")
    for x in range(16):
        g.set(x, 10, "0")
        if r.random() < 0.6:
            g.set(x, 11, "0")
    if end == "mid":
        g.fill(rect(9, 1, 1, 9), "0")   # the joint between two planks
        g.set(4, 2, "3")
        g.set(12, 2, "3")
    else:
        rx = 3 if end == "left" else 9
        for y in range(-1, 12):   # the rope wraps
            for i in range(4):
                if (y + i) % 3 != 2:
                    g.set(rx + i, y if y >= 0 else 0, "3" if (y + i) % 3 == 0 else "2")
        g.fill(rect(rx - 1, 0, 1, 10), "0")
        g.fill(rect(rx + 4, 0, 1, 10), "0")
        px = 1 if end == "left" else 13
        g.paint(rect(px, 3, 2, 3), ("0", "2", "3"))   # peg
    return g


def bracket():
    """A wooden brace under a platform end, its left side against a wall (flipped for walls on the right)."""
    g = Grid(16, 16)
    g.paint(rect(0, 0, 4, 16), WOOD)
    g.paint(capsule(2, 14, 14, 1, 1.6), WOOD)
    g.fill(rect(0, 0, 16, 2), "1")
    g.set(1, 3, "3")
    g.set(1, 12, "3")
    return g


def rope():
    g = Grid(4, 16)
    for y in range(16):
        for x in range(4):
            g.set(x, y, "3" if (x + y) % 4 == 0 else "2" if (x + y) % 4 == 1 else "1" if x < 3 else "0")
    return g


def post():
    g = Grid(6, 16)
    g.paint(rect(0, 0, 6, 16), WOOD)
    for y in (4, 11):
        g.set(2, y, "1")
        g.set(3, y + 1, "1")
    return g


def ladder(top):
    """A rope ladder: two twisted side ropes and lashed wooden rungs."""
    g = Grid(16, 16)
    for x0 in (1, 12):
        for y in range(16):
            for i in range(3):
                g.set(x0 + i, y, "3" if (y + i) % 3 == 0 else "2" if i < 2 else "1")
    for y0 in (3, 11):
        g.paint(rect(1, y0, 14, 3), WOOD)
        for x in (1, 2, 13, 14):
            g.set(x, y0 + 1, "0")
    if top:
        for y in range(0, 3):
            for x in range(16):
                g.set(x, y, ".")
        g.paint(capsule(1, 1.5, 14, 1.5, 1.4), WOOD)
        for x0 in (2, 13):
            g.fill(arc(x0 + 0.5, 3, 2.2, 180, 360, 1.1), "3")
    return g


def sea_surface(frame):
    """The sea's top line (16x8): wave crests and foam shift each frame."""
    g = Grid(16, 8)
    for x in range(16):
        phase = (x + frame * 5) % 16
        crest = 1 if phase in (2, 3, 4, 10, 11) else 0
        g.set(x, 2 - crest, "3" if phase in (3, 10) else "b")
        for y in range(3 - crest, 8):
            g.set(x, y, "b" if y < 4 else "a")
        if (x * 7 + frame * 3) % 11 == 0:
            g.set(x, 5, "b")
    return g


# --- Props (spec §22: scattered, 4-16 dp) ---
def grass_tuft(kind, frame):
    r = rng("tuft", kind)
    w, h = (12, 10) if kind == "A" else (10, 8)
    g = Grid(w, h)
    for i in range(7 if kind == "A" else 5):
        x = r.randint(1, w - 2)
        bh = r.randint(h // 2, h - 1)
        lean = (1 if frame else -1) * (1 if i % 2 else 0) + r.choice((-1, 0, 1))
        g.paint(capsule(x, h - 0.5, x + lean * bh * 0.3, h - bh, 0.6), ("5", "6", "7"))
        g.set(int(x + lean * bh * 0.3), h - bh, "7")
    return g


def flower(kind, frame=0):
    g = Grid(10, 12)
    petals, centre = ("f", "e") if kind == "A" else ("e", "d")
    for (x, h, lean) in ((3, 9, -1), (7, 7, 1)):
        top_x = x + (lean if frame else 0)
        g.fill(line(x, 11, top_x, 12 - h), "5")
        g.set(x + 1, 9, "6")
        for (dx, dy) in ((0, -1), (-1, 0), (1, 0), (0, 1)):
            g.set(top_x + dx, 12 - h + dy, petals)
        g.set(top_x, 12 - h, centre)
    return g


def pebbles(kind):
    r = rng("pebbles", kind)
    w, h = (10, 4) if kind == "A" else (14, 5)
    g = Grid(w, h)
    x = 0
    while x < w - 2:
        pw = r.randint(2, 4)
        ph = r.randint(2, h)
        g.paint(disc(x + pw / 2, h - ph / 2, pw / 2, ph / 2), ("1", "2", "3"))
        x += pw + r.randint(0, 2)
    return g


def crate():
    g = Grid(20, 18)
    g.paint(rect(0, 0, 20, 18), WOOD)
    g.paint(rect(2, 2, 16, 14), ("1", "2", "2"))
    g.fill(line(2, 2, 17, 15) | line(3, 2, 17, 14), "3")
    g.fill(line(2, 3, 16, 15), "1")
    for (x, y) in ((1, 1), (18, 1), (1, 16), (18, 16)):
        g.set(x, y, "0")
    return g


def barrel():
    g = Grid(16, 20)
    body = disc(8, 10, 8, 10) & rect(1, 0, 14, 20)
    g.paint(body, WOOD)
    for x in (4, 8, 12):   # staves
        g.fill({(x, y) for y in range(1, 19) if (x, y) in body}, "1")
    for y in (3, 16):   # iron hoops
        g.fill({(x, yy) for (x, yy) in body if yy in (y, y + 1)}, "0")
    g.fill(rect(4, 0, 8, 1), "3")
    return g


def bucket():
    g = Grid(12, 13)
    g.paint(poly([(1, 4), (11, 4), (10, 13), (2, 13)]), WOOD)
    g.fill(rect(1, 6, 10, 1) | rect(2, 11, 9, 1), "0")
    g.fill(arc(6, 5, 5, 180, 360, 1.0), "1")   # handle
    g.fill(rect(2, 4, 8, 1), "a")   # water
    g.set(4, 4, "b")
    return g


def stool():
    g = Grid(14, 12)
    g.paint(rect(0, 0, 14, 3), WOOD)
    g.paint(capsule(2.5, 3, 1.5, 11.5, 1.0) | capsule(11.5, 3, 12.5, 11.5, 1.0), WOOD)
    g.fill(line(2, 8, 12, 8), "1")
    return g


def cat(frame):
    """A grey cat asleep, curled up; its tail tip flicks across three frames."""
    g = Grid(24, 11)
    g.paint(disc(11, 7, 8.5, 4.2), ("0", "1", "2"))   # body
    g.paint(disc(17.5, 6.5, 3.8, 3.4), ("0", "1", "2"))   # head
    g.paint(poly([(15.5, 4), (17, 0.5), (18, 4)]) | poly([(18.5, 4), (20.5, 1), (21, 5)]), ("0", "1", "2"))   # ears
    g.set(17, 2, "c")
    g.fill(line(17, 7, 19, 7), "0")   # closed eye
    g.set(21, 8, "3")
    for x in (7, 10, 13):   # stripes
        g.fill(line(x, 3, x - 1, 6), "0")
    tail = [((4, 8), (0.5, 10), (5, 10.5)), ((4, 8), (0.5, 8.5), (2, 5)), ((4, 8), (1, 9.5), (1, 6.5))][frame]
    g.paint(capsule(*tail[0], *tail[1], 1.2) | capsule(*tail[1], *tail[2], 1.0), ("0", "1", "2"))
    g.set(int(tail[2][0]), int(tail[2][1]), "3")
    return g


def basket():
    g = Grid(18, 10)
    g.paint(poly([(1, 3), (14, 3), (13, 10), (2, 10)]), ("1", "2", "3"))
    for x in range(2, 14, 2):
        g.fill(line(x, 4, x, 9), "1")
    g.fill(rect(2, 6, 11, 1), "1")
    for (x, y) in ((4, 2), (8, 2), (15, 8)):   # apples, one rolled out
        g.paint(disc(x + 1, y + 1, 1.6), ("c", "d", "e"))
    return g


def roots(frame):
    """Hanging roots under a ceiling."""
    r = rng("roots")
    g = Grid(10, 16)
    for i, x in enumerate((2, 4, 6, 8)):
        length = r.randint(6, 15)
        sway = (1 if frame else 0) * (1 if i % 2 else -1)
        g.paint(capsule(x, 0, x + sway, length, 0.6), ("4", "5", "6"))
        g.set(x + sway, length, "6")
    return g


def wall_lantern(frame):
    """A lantern on a wall bracket (a light)."""
    g = Grid(10, 16)
    g.fill(rect(0, 2, 6, 1), "0")
    g.fill(rect(5, 2, 1, 3), "0")
    g.paint(rect(2, 5, 8, 9), ("0", "1", "2"))
    g.fill(rect(3, 6, 6, 7), "d" if frame else "e")
    g.fill(rect(5, 8, 2, 4), "f" if frame else "e")
    g.fill(rect(3, 9, 6, 1), "1")
    g.fill(rect(2, 14, 8, 1), "0")
    g.fill(rect(4, 4, 4, 1), "1")
    return g


# --- Landmarks (spec §22: hand-placed, 24-80 dp) ---
def standing_stones():
    """The hearth's stone ring: standing stones in an arc behind the hearth. 32 px of left padding put its centre 16 px
    right of the L tile, so the L sits one tile left of the hearth's C."""
    r = rng("ring")
    g = Grid(176, 64)
    stones = [(46, 30, 9), (64, 44, 11), (86, 52, 12), (110, 52, 12), (132, 44, 11), (150, 30, 9)]
    for (cx, h, w) in stones:
        top = 64 - h
        shape = poly([(cx - w / 2, 64), (cx + w / 2, 64), (cx + w / 2 - 1, top + 6), (cx + r.uniform(-2, 2), top),
                      (cx - w / 2 + 1, top + 5)])
        g.paint(shape, ("0", "1", "2"))
        for _ in range(3):   # cracks and moss
            x, y = r.randint(int(cx - w / 2) + 2, int(cx + w / 2) - 2), r.randint(top + 6, 60)
            g.set(x, y, "0")
            g.set(x, y + 1, "0")
        for x in range(int(cx - w / 2) + 1, int(cx + w / 2)):
            if r.random() < 0.6:
                g.set(x, top + 6 + r.randint(0, 3), "5")
        g.fill(rect(int(cx - 2), top + 14, 4, 1) | rect(int(cx - 1), top + 13, 2, 3), "c")   # a carved name rune, glowing faintly
    for x in range(30, 166):   # a low wall of stones at their feet
        h = 6 + int(2 * math.sin(x * 0.4))
        for y in range(64 - h, 64):
            g.set(x, y, "1" if (x // 5 + y // 3) % 2 else "2")
        g.set(x, 64 - h, "5" if x % 3 else "6")
    return g


def well():
    g = Grid(56, 72)
    # Stone base: fitted blocks in rows.
    for row in range(5):
        y = 72 - (row + 1) * 5
        x = 6 + (3 if row % 2 else 0)
        while x < 50:
            g.paint(rect(x, y, 6, 5) & rect(6, 0, 44, 72), ("0", "1", "2"))
            x += 7
    g.fill(rect(6, 46, 44, 2), "3")
    g.fill(rect(8, 48, 40, 1), "0")
    # Posts, roof and crank.
    g.paint(rect(9, 14, 4, 34) | rect(43, 14, 4, 34), WOOD)
    g.paint(poly([(1, 18), (28, 2), (55, 18), (50, 20), (28, 7), (6, 20)]), ("0", "1", "2"))
    for i in range(6):
        g.fill(line(4 + i * 9, 18 - (i if i < 3 else 5 - i) * 4, 9 + i * 9, 19 - (i if i < 3 else 5 - i) * 4), "0")
    g.paint(capsule(11, 22, 45, 22, 1.4), WOOD)   # axle
    g.fill(line(28, 23, 28, 36), "3")   # rope
    g.paint(poly([(24, 36), (32, 36), (31, 43), (25, 43)]), WOOD)   # bucket
    g.fill(rect(25, 38, 7, 1), "0")
    g.paint(capsule(47, 22, 52, 27, 1.0), WOOD)
    for x in range(8, 48, 3):
        g.set(x, 66 + x % 4, "5")
    return g


def stall(kind):
    """A market stall: posts, a striped awning, a counter with goods, a hanging lantern (its light)."""
    r = rng("stall", kind)
    w, h = (96, 80) if kind == 1 else (88, 76)
    g = Grid(w, h)
    stripes = ("5", "6", "7") if kind == 1 else ("9", "a", "b")
    other = ("1", "2", "3")
    g.paint(rect(4, 18, 4, h - 18) | rect(w - 8, 18, 4, h - 18), WOOD)
    # Awning: slanted cloth in stripes with a scalloped edge.
    for i in range(0, w, 8):
        band = poly([(i, 22), (i + 8, 22), (i + 8, 8), (i, 8)])
        g.paint(band, stripes if (i // 8) % 2 else other)
    for i in range(0, w, 8):
        g.paint(disc(i + 4, 23, 4, 3) & rect(i, 22, 8, 4), stripes if (i // 8) % 2 else other)
    g.fill(rect(0, 6, w, 2), "1")
    # Counter and goods.
    g.paint(rect(2, h - 26, w - 4, 6), WOOD)
    g.paint(rect(4, h - 20, w - 8, 20), ("0", "1", "1"))
    for x in range(8, w - 8, 10):
        g.fill(rect(x, h - 18, 1, 18), "0")
    x = 8
    while x < w - 14:
        kind_of = r.choice(("fruit", "jar", "fish", "bread"))
        if kind_of == "fruit":
            for i in range(3):
                g.paint(disc(x + 2 + i * 3, h - 28, 1.8), ("c", "d", "e") if kind == 1 else ("5", "6", "7"))
        elif kind_of == "jar":
            g.paint(rect(x, h - 34, 6, 8), ("9", "a", "b"))
            g.fill(rect(x + 1, h - 35, 4, 1), "1")
        elif kind_of == "bread":
            g.paint(disc(x + 4, h - 28, 4, 2.2), ("1", "2", "3"))
        else:
            g.paint(poly([(x, h - 29), (x + 8, h - 31), (x + 10, h - 28), (x + 8, h - 26)]), ("9", "a", "b"))
        x += r.randint(10, 14)
    # A string of hanging goods and the lantern.
    if kind == 2:
        for i in range(4):
            g.paint(poly([(14 + i * 9, 26), (17 + i * 9, 26), (16 + i * 9, 36)]), ("9", "a", "b"))
    lx = w - 20
    g.fill(line(lx + 3, 22, lx + 3, 27), "0")
    g.paint(rect(lx, 28, 7, 9), ("0", "1", "1"))
    g.fill(rect(lx + 1, 29, 5, 7), "e")
    g.fill(rect(lx + 2, 31, 3, 3), "f")
    return g


def signpost():
    g = Grid(40, 60)
    g.paint(rect(18, 4, 4, 56), WOOD)
    for (y, right, length) in ((10, True, 16), (22, False, 14), (34, True, 12)):
        x0 = 20 if right else 20 - length
        board = poly([(x0, y), (x0 + length, y), (x0 + length + (3 if right else 0), y + 3),
                      (x0 + length, y + 6), (x0, y + 6), (x0 - (0 if right else 3), y + 3)])
        g.paint(board, WOOD)
        for i in range(2, length - 2, 3):   # carved letters
            g.set(x0 + i, y + 3, "0")
    g.fill(rect(16, 58, 8, 2), "1")
    g.set(17, 57, "5")
    g.set(22, 57, "6")
    return g


def statue():
    """A statue of a nameless hero on a plinth: sword planted, the face worn away."""
    g = Grid(56, 104)
    g.paint(rect(6, 84, 44, 20), ("0", "1", "2"))
    g.paint(rect(3, 80, 50, 5), ("1", "2", "3"))
    g.fill(rect(14, 90, 28, 1), "0")   # an empty name plate
    g.fill(rect(14, 97, 28, 1), "0")
    g.fill(rect(14, 90, 1, 8) | rect(41, 90, 1, 8), "0")
    stone = ("1", "2", "3")
    g.paint(capsule(24, 62, 23, 79, 3.2) | capsule(32, 62, 33, 79, 3.2), stone)   # legs
    g.paint(poly([(18, 34), (38, 34), (36, 64), (20, 64)]), stone)   # body
    g.paint(poly([(18, 36), (12, 72), (22, 66)]), ("0", "1", "2"))   # cloak
    g.paint(disc(28, 26, 6.5, 7.5), stone)   # head
    g.fill(rect(26, 24, 6, 3), "1")   # the worn-away face
    g.paint(capsule(36, 38, 40, 52, 2.6) | capsule(20, 38, 22, 54, 2.6), stone)   # arms
    g.paint(capsule(41, 30, 41, 79, 1.6), ("1", "2", "3"))   # sword, point down
    g.fill(rect(37, 50, 9, 2), "2")
    for (x, y) in ((22, 34), (35, 64), (10, 80), (44, 80), (27, 19), (18, 70)):   # moss
        g.set(x, y, "5")
        g.set(x + 1, y, "6")
    return g


def boats():
    g = Grid(136, 44)
    for (x0, w, y0, flip) in ((4, 64, 18, False), (68, 60, 10, True)):
        hull = poly([(x0, y0), (x0 + w, y0), (x0 + w - 8, 44), (x0 + 10, 44)]) if not flip else \
            poly([(x0 + 4, y0 + 22), (x0 + w - 4, y0 + 22), (x0 + w, y0 + 8), (x0 + w / 2, y0), (x0, y0 + 8)])
        g.paint(hull, WOOD)
        for i in range(1, 4):   # planks
            y = y0 + i * (44 - y0) // 4 if not flip else y0 + i * 5
            g.fill({(x, yy) for (x, yy) in hull if yy == y}, "1")
        g.fill({(x, y) for (x, y) in hull if (x - x0) % 16 == 0}, "0")
    g.paint(capsule(10, 14, 50, 4, 1.2), WOOD)   # an oar leaning on the first boat
    g.paint(disc(52, 3.5, 3, 1.6), WOOD)
    g.fill(rect(70, 33, 56, 1), "0")
    for x in range(0, 136, 3):   # sand and weed at their keels
        g.set(x, 43, "2" if x % 2 else "5")
    return g


def hut():
    """Tobin and Garrick's hut on the shore: plank walls, a thatched roof, a lit window (its light), a chimney."""
    r = rng("hut")
    g = Grid(136, 104)
    g.paint(rect(12, 44, 112, 60), WOOD)
    for y in range(48, 104, 6):
        g.fill(rect(12, y, 112, 1), "1")
    for x in range(12, 124, 28):
        g.fill(rect(x, 44, 2, 60), "0")
    g.paint(rect(84, 1, 12, 30), ("0", "1", "2"))   # chimney
    g.fill(rect(82, 0, 16, 3), "1")
    roof = poly([(0, 50), (68, 6), (136, 50), (128, 54), (68, 16), (8, 54)])
    roof |= poly([(8, 50), (68, 12), (128, 50)])
    g.paint(roof, ("4", "5", "6"))
    for (x, y) in roof:   # thatch strands
        if r.random() < 0.18:
            g.set(x, y, "7" if r.random() < 0.3 else "4")
    g.paint(rect(28, 70, 22, 34), ("0", "1", "1"))   # door
    g.fill(rect(30, 72, 18, 32), "1")
    g.fill(rect(39, 72, 1, 32), "0")
    g.set(45, 88, "3")
    g.paint(rect(76, 62, 26, 20), ("0", "1", "1"))   # window
    g.fill(rect(78, 64, 22, 16), "d")
    g.fill(rect(81, 66, 16, 12), "e")
    g.fill(rect(88, 64, 2, 16) | rect(78, 71, 22, 2), "1")
    g.fill(rect(74, 82, 30, 3), "2")   # sill
    for x in (110, 116):   # a net drying on the wall
        g.fill(line(x, 56, x + 6, 90), "3")
    g.fill(line(108, 70, 122, 70) | line(108, 80, 122, 80), "3")
    return g


# --- Background sprites for the layers (spec §22 value rules: L1 ≤ 30 %, L2 ≤ 40 % luminance) ---
def house(n):
    """A village house for L2, in night colours, with windows (some dimly lit), a chimney and roof tiles."""
    r = rng("house", n)
    w = r.choice((72, 84, 96, 108, 120))
    wall_h = r.randint(60, 96)
    roof_h = r.randint(30, 48)
    chimney = r.random() < 0.85
    h = wall_h + roof_h + (18 if chimney else 0)
    g = Grid(w, h)
    top = h - wall_h
    wall = ("9", "a", "a") if n % 2 else ("9", "9", "a")
    g.paint(rect(4, top, w - 8, wall_h), wall)
    for y in range(top + 6, h, 7):   # timber lines
        if r.random() < 0.5:
            g.fill(rect(4, y, w - 8, 1), "9")
    for x in (4, w - 6):
        g.fill(rect(x, top, 2, wall_h), "8")
    peak = w // 2 + r.randint(-8, 8)
    roof = poly([(0, top + 2), (peak, top - roof_h), (w, top + 2)])
    g.paint(roof, ("8", "9", "a"))
    for (x, y) in roof:   # roof tile rows
        if (y - top) % 5 == 0 or ((x + (y // 5) * 3) % 8 == 0):
            g.set(x, y, "8")
    if chimney:
        cx = r.choice((peak - 22, peak + 14))
        cy = top - roof_h + 10 + abs(cx - peak) * roof_h // (w // 2)
        g.paint(rect(cx, cy - 26, 9, 26), ("8", "9", "a"))
        g.fill(rect(cx - 1, cy - 28, 11, 3), "9")
    for i in range(r.randint(2, 4)):   # windows; about half are lit
        ww, wh = r.choice(((10, 12), (12, 14), (8, 12)))
        x = 10 + i * ((w - 20) // 4) + r.randint(0, 6)
        y = top + r.randint(10, max(11, wall_h - 40))
        if x + ww > w - 8:
            continue
        lit = r.random() < 0.55
        g.fill(rect(x - 1, y - 1, ww + 2, wh + 2), "8")
        g.fill(rect(x, y, ww, wh), "c" if lit else "9")
        g.fill(rect(x + ww // 2, y, 1, wh) | rect(x, y + wh // 2, ww, 1), "8")
    door_x = r.randint(10, w - 26)
    g.fill(rect(door_x, h - 24, 12, 24), "8")
    g.fill(rect(door_x + 1, h - 23, 10, 23), "9")
    return g, (cx + 4 if chimney else -1, (cy - 28) if chimney else -1)


def lighthouse():
    """The lighthouse on the far cliffs (L1): a small tower silhouette with a lit window and the lamp room."""
    g = Grid(20, 70)
    tower = poly([(3, 70), (17, 70), (14, 16), (6, 16)])
    g.paint(tower, ("8", "9", "9"))
    for y in (26, 42, 58):   # bands
        g.fill({(x, yy) for (x, yy) in tower if y <= yy < y + 4}, "a")
    g.fill(rect(5, 13, 10, 3), "9")
    g.fill(rect(7, 6, 6, 7), "8")
    g.fill(rect(8, 7, 4, 5), "c")
    g.paint(poly([(5, 6), (15, 6), (10, 0)]), ("8", "9", "9"))
    g.fill(rect(9, 34, 2, 4), "c")
    return g


# --- Writing the C++ files ---
def build():
    """Every Namula sprite in source order: (name, Grid)."""
    out = []
    for shape in SHAPES:
        variants = 3 if shape in ("TOP", "FILL") else 1
        for v in range(variants):
            out.append(("NAMULA_%s_%d" % (shape, v + 1) if variants > 1 else "NAMULA_" + shape, ground_tile(shape, v)))
    for v in range(3):
        out.append(("NAMULA_FRINGE_%d" % (v + 1), fringe(v)))
    out.append(("NAMULA_DRAPE_LEFT", drape(False)))
    out.append(("NAMULA_DRAPE_RIGHT", drape(True)))
    out.append(("NAMULA_PLANK_LEFT", plank("left")))
    out.append(("NAMULA_PLANK_MID", plank("mid")))
    out.append(("NAMULA_PLANK_RIGHT", plank("right")))
    out.append(("NAMULA_BRACKET", bracket()))
    out.append(("NAMULA_ROPE", rope()))
    out.append(("NAMULA_POST", post()))
    out.append(("NAMULA_LADDER", ladder(False)))
    out.append(("NAMULA_LADDER_TOP", ladder(True)))
    for f in range(3):
        out.append(("NAMULA_SEA_%d" % (f + 1), sea_surface(f)))
    for kind in "AB":
        for f in range(2):
            out.append(("NAMULA_TUFT_%s_%d" % (kind, f + 1), grass_tuft(kind, f)))
    for kind in "AB":
        for f in range(2):
            out.append(("NAMULA_FLOWER_%s_%d" % (kind, f + 1), flower(kind, f)))
    out.append(("NAMULA_PEBBLES_A", pebbles("A")))
    out.append(("NAMULA_PEBBLES_B", pebbles("B")))
    out.append(("NAMULA_CRATE", crate()))
    out.append(("NAMULA_BARREL", barrel()))
    out.append(("NAMULA_BUCKET", bucket()))
    out.append(("NAMULA_STOOL", stool()))
    for f in range(3):
        out.append(("NAMULA_CAT_%d" % (f + 1), cat(f)))
    out.append(("NAMULA_BASKET", basket()))
    for f in range(2):
        out.append(("NAMULA_ROOTS_%d" % (f + 1), roots(f)))
    for f in range(2):
        out.append(("NAMULA_WALL_LANTERN_%d" % (f + 1), wall_lantern(f)))
    out.append(("NAMULA_STONE_RING", standing_stones()))
    out.append(("NAMULA_WELL", well()))
    out.append(("NAMULA_STALL_1", stall(1)))
    out.append(("NAMULA_STALL_2", stall(2)))
    out.append(("NAMULA_SIGNPOST", signpost()))
    out.append(("NAMULA_STATUE", statue()))
    out.append(("NAMULA_BOATS", boats()))
    out.append(("NAMULA_HUT", hut()))
    out.append(("NAMULA_LIGHTHOUSE", lighthouse()))
    chimneys = []
    for n in range(10):
        g, chimney = house(n)
        out.append(("NAMULA_HOUSE_%d" % (n + 1), g))
        chimneys.append(chimney)
    return out, chimneys


def write_all(write, sprite_text, root):
    sprites, chimneys = build()
    h = ["// Namula's world art (spec §13 row 0, §22), generated by docs/work-plan/name-quest/art-tools/make_quest_art.py",
         "// (namula.py): edit the script, not this file. Palette sprites: '0'-'f' index NAMULA_PALETTE, '.' transparent.",
         "#pragma once", "#include \"screen.h\"", "",
         "// Stone (cool grey-brown), moss, night (blue-violet), firelight (lights and windows only); dark → light per ramp.",
         "const WorldPalette NAMULA_PALETTE = {{" + ", ".join("0xFF%06X" % c for c in PALETTE) + "}};",
         "const int NAMULA_HOUSE_COUNT = 10;",
         "// Each house's chimney top (x, y in the house sprite; -1: none), where the smoke rises from.",
         "const int NAMULA_CHIMNEYS[NAMULA_HOUSE_COUNT][2] = {%s};" % ", ".join("{%d, %d}" % c for c in chimneys), ""]
    h += ["extern const Sprite %s;" % name for name, g in sprites]
    write(os.path.join(root, "source", "quest_sprites_0.h"), "\n".join(h) + "\n")
    cpp = ["// Namula's world art, generated by docs/work-plan/name-quest/art-tools/make_quest_art.py (namula.py).",
           "#include \"quest_sprites_0.h\"", ""]
    cpp += [sprite_text(name, g, "extern const Sprite") for name, g in sprites]
    write(os.path.join(root, "source", "quest_sprites_0.cpp"), "\n".join(cpp))


def preview(folder, save_png):
    sprites, _ = build()
    rgb = {("%x" % i): ((c >> 16) & 255, (c >> 8) & 255, c & 255) for i, c in enumerate(PALETTE)}
    bg = (60, 20, 60)
    rows, row, x, row_h, width = [], [], 0, 0, 1100
    for name, g in sprites:
        if x + g.w + 4 > width:
            rows.append((row, row_h))
            row, x, row_h = [], 0, 0
        row.append((x, g))
        x += g.w + 4
        row_h = max(row_h, g.h + 4)
    rows.append((row, row_h))
    total_h = sum(rh for _, rh in rows)
    img = [[bg] * width for _ in range(total_h)]
    y0 = 0
    for row, rh in rows:
        for (x0, g) in row:
            for y in range(g.h):
                for x in range(g.w):
                    if g.c[y][x] != ".":
                        img[y0 + y + 2][x0 + x + 2] = rgb[g.c[y][x]]
        y0 += rh
    save_png(os.path.join(folder, "namula.png"), width, total_h, img, 2)
