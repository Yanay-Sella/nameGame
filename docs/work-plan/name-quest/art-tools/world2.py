"""World 2, The Greendeep (spec §13 row 2, §22): tiles, branch platforms, vines, water, thorns, the thorny-root gate,
props and landmarks in the world palette ('0'-'3' soil and bark, '4'-'7' moss and leaves, '8'-'b' the deep green
background, 'c'-'f' light: mushroom teal and sun gold, dark → light), plus the roster's five enemies and the two bosses
in the global palette (spec §8, §12), drawn side-view with a baked outline and doubled. Used by make_quest_art.py, which
writes source/quest_sprites_2.h/.cpp."""
import math
import os
import random

from make_quest_art import Grid, arc, capsule, disc, line, poly, rect
from world1 import OPEN, SHAPES, finish

PALETTE = [0x1A120C, 0x2E2116, 0x4A3522, 0x6E5034,    # soil and bark: dark → light brown
           0x0F2A18, 0x1F4C26, 0x3A7A30, 0x76A844,    # moss and leaves
           0x050C08, 0x0A1810, 0x112619, 0x1B3824,    # the deep green background (layers, water depth)
           0x1E5A46, 0x52BC98, 0xB0923A, 0xD8CCA4]    # light: mushroom teal, its glow, sun gold, pale bone
SOIL, BARK, MOSS, LEAF = ("0", "1", "2"), ("1", "2", "3"), ("4", "5", "6"), ("5", "6", "7")
STONE = ("1", "2", "3")   # temple stone: weathered brown, moss on top
DEEP, GLOW, GOLD = ("8", "9", "a"), ("c", "d", "d"), ("e", "e", "f")


def rng(*key):
    return random.Random("greendeep-" + "-".join(str(k) for k in key))


# --- Ground tiles (16x16): moss and grass over dark soil with stones and buried bones ---
def soil(g, r, variant, y0=0):
    for y in range(y0, 16):
        for x in range(16):
            g.set(x, y, "1" if r.random() > 0.18 else "0")
    for _ in range(2 + variant):   # stones: small rounded lumps
        cx, cy = r.randint(2, 13), r.randint(max(y0 + 3, 4), 14)
        g.paint(disc(cx, cy, r.choice((1.6, 2.2)), 1.4) & rect(0, y0, 16, 16 - y0), BARK)
    if variant == 2:   # a pocket of darker earth
        g.paint(disc(10, max(y0 + 7, 10), 3, 2) & rect(0, y0, 16, 16 - y0), ("0", "0", "1"))
    if variant == 1:   # a root threading through
        g.fill(line(0, max(y0 + 4, 7), 15, max(y0 + 8, 11)), "2")


def moss_cap(g, r, variant, left, right):
    """Rows 0-4: a bright grass top line, moss, then a dark lip where the moss meets the soil."""
    for x in range(16):
        g.set(x, 0, "7" if r.random() > 0.25 else "6")
        g.set(x, 1, "6" if r.random() > 0.2 else "7")
        g.set(x, 2, "5")
        g.set(x, 3, "5" if r.random() > 0.3 else "4")
        g.set(x, 4, "4" if (x + variant) % 5 else "5")   # moss hanging a little into the soil
        if (x * 7 + variant) % 6 == 0:
            g.set(x, 5, "4")
    for x in range(variant + 1, 16, 6):   # small flowers in the grass
        g.set(x, 1, "f" if variant == 1 else "e")
    for side, on in (("l", left), ("r", right)):
        if on:   # the edge: moss rounded over the side
            for y in range(0, 7):
                g.set(0 if side == "l" else 15, y, "5" if y < 5 else "4")


def ground_tile(shape, variant):
    r = rng("tile", shape, variant)
    g = Grid(16, 16)
    sides = OPEN[shape]
    soil(g, r, variant if shape == "FILL" else 0, 5 if "t" in sides else 0)
    if "t" in sides:
        moss_cap(g, r, variant, "l" in sides, "r" in sides)
    for side in "lr":
        if side not in sides:
            continue
        x0, x1 = (0, 1) if side == "l" else (15, 14)
        for y in range(5 if "t" in sides else 0, 16):
            g.set(x0, y, "0")
            g.set(x1, y, "2" if (y + variant) % 5 == 0 else g.get(x1, y))
    if "b" in sides:
        for x in range(16):
            g.set(x, 15, "0")
            g.set(x, 14, "0" if x % 3 else "1")
            if x % 5 == 2:   # root tips under an overhang
                g.set(x, 15, "2")
    for (corner, pts) in (("tl", ((0, 0), (1, 0), (0, 1))), ("tr", ((15, 0), (14, 0), (15, 1))),
                          ("bl", ((0, 15), (0, 14), (1, 15))), ("br", ((15, 15), (15, 14), (14, 15)))):
        if all(c in sides for c in corner):
            for (x, y) in pts:
                g.set(x, y, ".")
    return g


def fringe(variant):
    """Grass blades over the top line (rows 0-5 above, 6-7 on the cap); a fern tip or a clover now and then."""
    r = rng("fringe", variant)
    g = Grid(16, 8)
    for x in range(16):
        g.set(x, 7, "6")
        if r.random() < 0.55:
            h = r.randint(1, 3 if variant else 4)
            for y in range(h):
                g.set(x + (1 if y == h - 1 and r.random() < 0.4 else 0), 6 - y, "6" if y < h - 1 else "7")
    if variant == 1:   # a fern tip
        g.fill(line(4, 6, 7, 1), "5")
        for (x, y) in ((5, 4), (6, 3), (7, 2)):
            g.set(x - 1, y - 1, "6")
            g.set(x + 1, y, "6")
    if variant == 2:   # a clover patch
        for (x, y) in ((10, 5), (12, 5), (11, 4)):
            g.paint(disc(x + 0.5, y + 0.5, 1.2), LEAF)
    return g


def drape(right):
    """Hanging moss and root tips down the side of a ledge: 8x16, on the tile's left (flipped for the right)."""
    r = rng("drape", right)
    g = Grid(8, 16)
    for (x, length, col) in ((7, r.randint(8, 14), "5"), (6, r.randint(4, 9), "4"), (5, r.randint(2, 6), "5")):
        for y in range(length):
            g.set(x, y, col if y < length - 2 else "4")
    g.fill(line(6, 0, 3, r.randint(9, 13)), "2")   # a root curling away from the wall
    g.set(3, 12, "1")
    if right:
        g.c = [row[::-1] for row in g.c]
    return g


# --- Platforms (16x12): a mossy branch with leaves and knots; supports; a vine; water; thorns; the thorn gate ---
def branch(end):
    g = Grid(16, 12)
    g.paint(rect(0, 1, 16, 5), BARK)
    g.fill(rect(0, 5, 16, 1), "0")
    for x in range(0, 16, 3):   # bark grain
        g.set(x, 3, "1")
        g.set(x + 1, 4, "1")
    if end == "mid":
        g.paint(disc(9, 3.5, 1.6, 1.3), ("0", "1", "1"))   # a knot
    for x in range(16):   # moss along the top
        g.set(x, 0, "6" if x % 4 else "7")
        g.set(x, 1, "5" if x % 3 else g.get(x, 1))
    for x in range(2, 16, 7):   # leaves hanging under it
        g.fill(rect(x, 6, 2, 2), "5")
        g.set(x, 8, "4")
    if end == "left":
        for (x, y) in ((0, 1), (0, 5), (1, 5)):
            g.set(x, y, ".")
        g.paint(disc(2, 0, 2.6, 1.8), LEAF)   # a leaf tuft on the end
    if end == "right":
        for (x, y) in ((15, 1), (15, 5), (14, 5)):
            g.set(x, y, ".")
        g.paint(disc(13, 0, 2.6, 1.8), LEAF)
    return g


def bracket():
    """A side branch growing out of the wall under a platform's end (flipped for walls on the right)."""
    g = Grid(16, 16)
    g.paint(capsule(0, 14, 14, 1, 1.8), BARK)
    g.fill(line(0, 13, 13, 1), "3")
    g.paint(disc(9, 8, 2.2, 1.6), LEAF)
    return g


def hanging_vine():
    """A thin hanging vine from a platform up to the ceiling, a leaf every few pixels."""
    g = Grid(4, 16)
    for y in range(16):
        g.set(1 + (y // 5) % 2, y, "5")
    for y in (3, 11):
        g.set(0, y, "6")
        g.set(3, y + 2, "6")
    return g


def post():
    """A young trunk propping a branch from the ground."""
    g = Grid(6, 16)
    g.paint(rect(1, 0, 4, 16), BARK)
    for y in range(2, 16, 5):
        g.set(2, y, "1")
    g.set(1, 9, "5")
    g.set(0, 10, "5")
    return g


def vine(top):
    """A climbable vine (`|`): two twisted green ropes with leaves; its top loops over a branch stub."""
    g = Grid(16, 16)
    for y in range(16):
        a = int(round(7 + 2.2 * math.sin((y + 0) * 0.78)))
        b = int(round(8 - 2.2 * math.sin((y + 0) * 0.78)))
        g.set(a, y, "6")
        g.set(a + 1, y, "5")
        g.set(b, y, "5" if g.get(b, y) == "." else "4")
    for (x, y, d) in ((4, 2, -1), (11, 6, 1), (4, 10, -1), (11, 14, 1)):   # leaves
        g.paint(disc(x, y, 1.8, 1.1), LEAF)
        g.set(x + d * 2, y, "5")
    if top:
        for y in range(0, 3):
            for x in range(16):
                g.set(x, y, ".")
        g.paint(rect(3, 0, 10, 3), BARK)
        g.fill(arc(8, 3, 3, 180, 360, 1.2), "6")
    return g


def water(frame):
    """Dark jungle water's surface: a lighter crest moving along, a lily pad drifting, bubbles."""
    g = Grid(16, 8)
    for x in range(16):
        crest = 1 if (x + frame * 5) % 16 in (2, 3, 10) else 0
        g.set(x, 1 - crest, "c" if (x + frame * 3) % 7 else "d")
        for y in range(2 - crest, 8):
            g.set(x, y, "c" if y < 3 else "b" if y < 5 else "a")
    pad = (frame * 4 + 3) % 12
    g.paint(disc(pad + 2, 2, 2.2, 0.9), ("4", "5", "5"))
    g.set(pad + 2, 2, "4")
    g.set((frame * 6 + 9) % 16, 5, "c")
    return g


def thorns():
    """Thorn spikes (`^`): brown, hooked, on a tangle of roots; read as danger against the green (16x16, bottom half)."""
    g = Grid(16, 16)
    g.paint(capsule(0, 14, 15, 13, 1.6), BARK)
    for (x, h, lean) in ((1, 9, 1), (5, 12, -1), (9, 10, 1), (13, 12, -1)):
        tip = (x + lean * 2, 15 - h)
        g.paint(poly([(x - 1.5, 15), (x + 1.5, 15), tip]), ("1", "3", "3"))
        g.set(tip[0], tip[1], "f")
        g.set(tip[0] + lean, tip[1] + 1, "3")
    return g


def thorn_gate():
    """The gate (`g`): a wall of thick thorny roots, one 16x16 tile that stacks seamlessly."""
    g = Grid(16, 16)
    for (x0, x1, w) in ((3, 6, 2.4), (11, 9, 2.6), (7, 8, 1.8)):
        g.paint(capsule(x0, -1, x1, 17, w), BARK)
    g.fill(line(3, 0, 6, 15) | line(11, 0, 9, 15), "1")
    for (x, y, d) in ((1, 3, -1), (14, 6, 1), (2, 11, -1), (15, 13, 1), (8, 8, 1)):   # thorns jutting out
        g.fill(line(x - d * 2, y, x + d, y - 2), "3")
        g.set(x + d, y - 2, "f")
    return g


def palm_crown(has_coconuts):
    """The coconut palm's crown (`R`, spec §9): drooping fronds around the top of its trunk (40x22)."""
    g = Grid(40, 22)
    for (dx, droop, length) in ((-1, 0.9, 18), (1, 0.9, 18), (-1, 0.3, 14), (1, 0.3, 14), (-1, -0.4, 9), (1, -0.4, 9)):
        pts = []
        for i in range(length):
            t = i / length
            pts.append((20 + dx * i, 9 - 6 * t + droop * 14 * t * t))
        for i, (x, y) in enumerate(pts):
            g.paint(disc(x, y, 1.6 - i * 0.05), LEAF)
            if i % 2 == 0 and i > 2:
                g.set(int(x), int(y) + 2, "5")
                g.set(int(x), int(y) + 3, "4")
    g.paint(rect(18, 9, 4, 13), BARK)
    for y in range(11, 22, 3):
        g.fill(rect(18, y, 4, 1), "1")
    if not has_coconuts:
        g.paint(disc(20, 10, 2.2), ("1", "2", "2"))   # an empty husk scar
    return g


def palm_trunk():
    g = Grid(8, 16)
    g.paint(rect(2, 0, 4, 16), BARK)
    for y in range(1, 16, 4):
        g.fill(rect(2, y, 4, 1), "1")
        g.set(5, y + 1, "3")
    return g


# --- Props (spec §22: 4-16 dp; none looks like a thorn or a coconut) ---
def fern(frame):
    g = Grid(16, 12)
    for (dx, h) in ((-1, 9), (0, 11), (1, 8)):
        sway = frame if dx >= 0 else 0
        top = (8 + dx * 5 + sway, 11 - h)
        g.fill(line(8, 11, top[0], top[1]), "5")
        for i in range(1, h, 2):
            y = 11 - i
            x = 8 + (top[0] - 8) * i // h
            g.set(x - 1, y, "6")
            g.set(x + 1, y - 1, "6")
            g.set(x - 2, y + 1, "4" if i > 4 else "5")
    return g


def mushroom(kind, frame=0):
    g = Grid(12, 12)
    if kind == 0:   # a cluster of small brown caps
        for (x, h) in ((3, 5), (7, 8), (10, 4)):
            g.fill(rect(x, 12 - h, 1, h), "f")
            g.paint(disc(x + 0.5, 12 - h, 2.4, 1.4) & rect(0, 0, 12, 12 - h + 1), BARK)
    elif kind == 1:   # a tall pale stalk with a wide cap
        g.paint(rect(5, 5, 2, 7), ("e", "f", "f"))
        g.paint(disc(6, 5, 5, 2.6) & rect(0, 0, 12, 6), ("1", "2", "3"))
        g.set(4, 4, "f")
        g.set(8, 3, "f")
    else:   # glowing teal mushrooms (light), the gills pulsing
        for (x, h, r) in ((3, 6, 2.6), (8, 9, 3.2)):
            g.fill(rect(x, 12 - h, 1, h), "f")
            g.paint(disc(x + 0.5, 12 - h, r, 1.8) & rect(0, 0, 12, 12 - h + 1), ("c", "d", "d"))
            g.set(x, 12 - h + 1, "d" if frame else "c")
    return g


def flowers():
    g = Grid(12, 10)
    for (x, h, col) in ((2, 6, "e"), (6, 9, "f"), (9, 5, "d")):
        g.fill(rect(x, 10 - h, 1, h), "5")
        g.set(x - 1, 10 - h, col)
        g.set(x + 1, 10 - h, col)
        g.set(x, 9 - h, col)
        g.set(x, 10 - h, "e" if col != "e" else "3")
        g.set(x + 1, 10 - h // 2, "6")
    return g


def fallen_leaves():
    g = Grid(14, 4)
    for (x, col) in ((1, "3"), (5, "e"), (9, "2"), (11, "6")):
        g.paint(disc(x + 1.5, 2.5, 2, 1) & rect(0, 0, 14, 4), (col, col, col))
        g.set(x + 1, 2, "1")
    return g


def root_loop():
    g = Grid(16, 6)
    g.fill(arc(8, 6, 6, 180, 360, 1.6), "2")
    g.fill(arc(8, 6, 6, 200, 300, 1.0), "3")
    g.fill(line(0, 5, 3, 5) | line(13, 5, 15, 5), "1")
    return g


def snail():
    g = Grid(10, 7)
    g.paint(disc(5, 3.5, 3.2), BARK)
    g.fill(arc(5, 3.5, 1.6, 0, 300, 1.0), "e")
    g.fill(rect(1, 6, 8, 1), "d")
    g.set(1, 5, "d")
    g.set(0, 4, "d")
    return g


def mossy_stone():
    g = Grid(14, 8)
    g.paint(disc(7, 6, 6.5, 4.5) & rect(0, 0, 14, 8), ("9", "a", "b"))
    g.paint(disc(7, 3, 5, 2) & rect(0, 0, 14, 5), MOSS)
    g.fill(rect(0, 7, 14, 1), "8")
    return g


def ceiling_vines(frame):
    g = Grid(14, 16)
    for (x, length) in ((2, 14), (6, 9), (10, 15), (12, 6)):
        sway = frame if x > 5 else 0
        for y in range(length):
            g.set(x + (sway if y > length // 2 else 0), y, "5" if y < length - 3 else "6")
        g.set(x - 1 + (sway if length > 8 else 0), length - 3, "6")
    g.fill(rect(0, 0, 14, 2), "4")
    return g


def drip(frame):
    """Water dripping from moss under the ceiling: a drop forming, then falling (2 frames)."""
    g = Grid(8, 14)
    g.paint(disc(4, 1, 4, 2) & rect(0, 0, 8, 3), MOSS)
    if frame == 0:
        g.fill(rect(4, 3, 1, 2), "c")
    else:
        g.fill(rect(4, 3, 1, 1), "c")
        g.fill(rect(4, 10, 1, 2), "d")
    return g


def wall_fern():
    g = Grid(10, 12)
    for (y, length) in ((2, 8), (6, 9), (10, 6)):
        g.fill(line(0, y, length, y - 2), "5")
        for x in range(2, length, 2):
            g.set(x, y - 2 - (x // 3) % 2, "6")
    return g


def bone():
    g = Grid(12, 4)
    g.fill(rect(2, 2, 8, 1), "f")
    for (x, y) in ((1, 1), (1, 3), (10, 1), (10, 3)):
        g.set(x, y, "f")
    g.fill(rect(3, 3, 6, 1), "e")
    return g


# --- Landmarks (spec §22: 24-80 dp; ids 1-6 in this order) ---
def statue_head():
    """A fallen statue's head lying on its side, moss in its eye and on its crown (64x48): Garrick's dead end."""
    g = Grid(64, 48)
    head = poly([(4, 46), (6, 18), (16, 6), (36, 2), (54, 10), (62, 30), (60, 46)])
    g.paint(head, STONE)
    g.fill(poly([(36, 46), (44, 26), (60, 30), (60, 46)]), "1")   # the cheek in shadow
    g.fill(rect(16, 20, 12, 4), "0")   # the closed eye
    g.fill(rect(18, 19, 8, 1), "3")
    g.paint(poly([(30, 22), (40, 22), (36, 34), (30, 34)]), ("0", "1", "1"))   # the nose
    g.fill(line(14, 38, 30, 40), "0")   # the mouth
    g.fill(line(14, 39, 30, 41), "1")
    g.paint(poly([(10, 12), (22, 2), (40, 0), (56, 8), (40, 10), (20, 14)]), MOSS)   # moss on the crown
    g.fill(line(42, 4, 58, 20) | line(43, 4, 58, 22), "0")   # a crack
    for (x, y) in ((8, 30), (52, 40), (46, 16)):
        g.paint(disc(x, y, 2.6, 1.6), LEAF)
    g.fill(rect(0, 46, 64, 2), "0")
    return g


def temple_stairs():
    """Overgrown temple stairs climbing to a broken doorway (72x56)."""
    g = Grid(72, 56)
    for i in range(6):
        y = 56 - (i + 1) * 7
        g.paint(rect(i * 5, y, 72 - i * 10, 7), STONE)
        g.fill(rect(i * 5, y, 72 - i * 10, 1), "3")
        for x in range(i * 5 + 3, 72 - i * 5 - 3, 9):
            g.set(x, y + 3, "0")
    g.paint(rect(26, 0, 6, 14), STONE)   # the doorway's jambs
    g.paint(rect(40, 2, 6, 12), STONE)
    g.fill(rect(32, 2, 8, 12), "0")
    for x in (4, 20, 50, 64):   # vines over the steps
        g.fill(line(x, 14, x + 3, 56), "5")
        g.set(x + 1, 30, "6")
    return g


def waterfall(frame):
    """A thin waterfall spilling from a crack into a pool, its streaks moving down (40x72), light."""
    g = Grid(40, 72)
    g.paint(rect(0, 0, 40, 8), ("8", "9", "a"))
    g.fill(rect(14, 6, 12, 2), "8")
    for x in range(15, 25):
        for y in range(8, 64):
            g.set(x, y, "b" if (y + x * 3 + frame * 4) % 9 > 2 else "c")
    for x in (15, 24):
        g.fill(rect(x, 8, 1, 56), "a")
    g.paint(disc(20, 66, 16, 5) & rect(0, 62, 40, 10), ("a", "b", "b"))   # the pool
    for (x, y) in ((10 + frame * 2, 64), (28 - frame * 2, 66), (20, 63)):
        g.set(x, y, "d")
    return g


def giant_flower(frame):
    """A giant red-gold flower on a thick stem with two big leaves (48x64)."""
    g = Grid(48, 64)
    g.paint(capsule(24, 20, 24, 64, 2.6), LEAF)
    g.paint(poly([(24, 50), (4, 38), (10, 34), (24, 44)]), LEAF)
    g.paint(poly([(24, 42), (44, 30), (40, 26), (24, 36)]), LEAF)
    for k in range(6):
        a = k * math.pi / 3 + frame * 0.12
        g.paint(disc(24 + math.cos(a) * 11, 16 + math.sin(a) * 8, 7.5, 5.5), ("3", "e", "f"))
    g.paint(disc(24, 16, 6, 4.6), ("1", "2", "3"))
    for (x, y) in ((22, 14), (26, 17), (24, 15)):
        g.set(x, y, "1")
    return g


def rope_bridge():
    """A broken rope bridge sagging between two posts (80x28)."""
    g = Grid(80, 28)
    for x0 in (2, 74):
        g.paint(rect(x0, 2, 4, 26), BARK)
        g.paint(rect(x0 - 1, 1, 6, 3), ("1", "3", "3"))
    planks = []
    for x in range(6, 74):
        t = (x - 6) / 68
        y = int(6 + 14 * math.sin(t * math.pi))
        if 34 < x < 44:
            continue   # the break
        g.set(x, y - 6, "2")   # the hand rope
        if x % 4 == 0:
            g.fill(rect(x, y - 6, 1, 6), "2")
            planks.append((x, y))
    for (x, y) in planks:
        g.paint(rect(x - 1, y, 4, 2), BARK)
    g.fill(line(34, 12, 36, 24) | line(44, 12, 42, 22), "2")   # the torn ends hanging
    return g


def idol(frame):
    """A squat stone idol with glowing eyes on a pedestal (40x56), light."""
    g = Grid(40, 56)
    g.paint(rect(6, 44, 28, 12), STONE)
    g.fill(rect(6, 44, 28, 1), "3")
    g.paint(poly([(10, 44), (8, 14), (14, 4), (26, 4), (32, 14), (30, 44)]), STONE)
    g.fill(rect(10, 24, 20, 2), "0")
    g.paint(poly([(12, 30), (28, 30), (24, 40), (16, 40)]), ("0", "1", "1"))   # the open mouth
    g.fill(rect(16, 32, 8, 6), "0")
    for x in (14, 23):
        g.paint(rect(x, 15, 4, 3), ("c", "d", "d") if frame else ("c", "c", "d"))
    g.paint(disc(20, 4, 6, 3) & rect(0, 0, 40, 5), MOSS)
    g.fill(line(30, 8, 34, 44), "5")   # a vine down its side
    return g


# --- Enemies and bosses (global palette; side view, facing right; art px with a baked outline, then doubled) ---
ENEMY_FRAMES = ["WALK_1", "WALK_2", "WALK_3", "WALK_4", "TELL", "ATTACK"]


def leafhopper(frame):
    """LEAFHOPPER (from LEAFHOPPER_A/B): a lime grasshopper with red eyes and long jumping legs (15x11 art)."""
    g = Grid(15, 11)
    crouch = {"WALK_1": 0, "WALK_2": 1, "WALK_3": 0, "WALK_4": 1, "TELL": 2, "ATTACK": -2}[frame]
    y = 4 + max(crouch, 0)
    g.paint(capsule(3, y, 10, y - 1, 2.4), ("G", "l", "l"))   # body
    g.paint(disc(11.5, y - 1.5, 2.6, 2.3), ("G", "l", "l"))   # head
    g.set(12, y - 2, "r")
    g.set(13, y - 2, "r")
    g.fill(line(12, y - 4, 14, 0), "l")   # antennae
    g.fill(line(11, y - 4, 10, 0), "G")
    g.fill(line(3, y - 2, 8, y - 3), "G")   # the wing fold
    knee = (2, y - 4) if crouch >= 0 else (1, y - 1)
    foot = (5, 10) if crouch >= 0 else (0, 10)
    g.fill(line(6, y, knee[0], knee[1]) | line(knee[0], knee[1], foot[0], foot[1]), "G")   # the big hind leg
    step = ENEMY_FRAMES.index(frame) if frame.startswith("WALK") else 0
    for i, x in enumerate((8, 10)):
        dx = (-1, 0, 1, 0)[(step + i * 2) % 4]
        g.fill(line(x, y + 1, x + dx, 10), "G")
    return g


def beetle(frame):
    """BEETLE (from BEETLE_A/B): a brown shelled beetle with tan highlights and black legs (14x10 art)."""
    g = Grid(14, 10)
    bob = 1 if frame in ("WALK_2", "WALK_4") else 0
    g.paint(disc(6, 5 + bob, 6, 4) & rect(0, 0, 14, 7 + bob), ("h", "h", "H"))   # the shell
    g.fill(line(6, 1 + bob, 6, 6 + bob), "x")   # the split between the wing cases
    g.set(3, 3 + bob, "H")
    g.set(9, 3 + bob, "H")
    g.paint(disc(12, 5 + bob, 2, 1.8), ("x", "x", "X"))   # head
    g.set(13, 4 + bob, "w")
    g.fill(line(12, 3 + bob, 13, 1 + bob), "x")
    open_ = frame == "ATTACK"
    if open_:
        g.fill(line(13, 6 + bob, 14, 7 + bob), "x")
    step = ENEMY_FRAMES.index(frame) if frame.startswith("WALK") else 0
    for i, x in enumerate((3, 6, 9)):
        dx = (-1, 0, 1, 0)[(step + i) % 4]
        g.fill(line(x, 7 + bob, x + dx, 9), "x")
    return g


def stag(frame):
    """STAG (from STAG_A/B): a black stag beetle with big brown mandibles and red eyes (18x12 art); it lowers them to charge."""
    g = Grid(18, 12)
    g.paint(disc(6, 7, 6, 3.8) & rect(0, 0, 18, 10), ("x", "x", "X"))   # the body
    g.fill(line(6, 4, 6, 9), "x")
    g.set(3, 5, "X")
    g.set(8, 5, "X")
    g.paint(disc(11.5, 6.5, 2.6, 2.3), ("x", "x", "X"))   # head
    g.set(12, 5, "r")
    g.set(13, 5, "r")
    lift = {"TELL": -2, "ATTACK": 2}.get(frame, 0)
    for (base_y, tip_y, curl) in ((5, 1, 1), (8, 10, -1)):   # the two mandibles, curving toward each other
        tip_y = tip_y + lift
        g.fill(capsule(13, base_y, 16, (base_y + tip_y) / 2, 0.8) | capsule(16, (base_y + tip_y) / 2, 17, tip_y, 0.8), "h")
        g.set(16, tip_y + curl, "H")
        g.set(15, (base_y + tip_y) // 2 + curl, "H")
    step = ENEMY_FRAMES.index(frame) if frame.startswith("WALK") else 0
    for i, x in enumerate((2, 5, 8)):
        dx = (-1, 0, 1, 0)[(step + i) % 4]
        g.fill(line(x, 9, x + dx, 11), "x")
    return g


def tree_monkey(frame):
    """TREE MONKEY (new, 12x12): a small brown monkey with a tan face and a long tail; TELL hangs by the tail, ATTACK drops."""
    g = Grid(12, 12)
    if frame == "TELL":   # hanging upside down from its tail, arms reaching down
        g.fill(line(6, 0, 6, 3), "h")
        g.paint(disc(6, 6, 3, 2.6), ("h", "h", "h"))
        g.paint(disc(6, 9.5, 2.6, 2.2), ("h", "H", "H"))
        g.set(5, 10, "x")
        g.set(7, 10, "x")
        g.fill(line(3, 7, 2, 11) | line(9, 7, 10, 11), "h")
        return g
    drop = frame == "ATTACK"
    bob = 1 if frame in ("WALK_2", "WALK_4") else 0
    y = 4 + bob
    g.paint(disc(5, y + 2.5, 3.6, 2.6), ("h", "h", "h"))   # body
    g.paint(disc(5, y + 2.8, 2, 1.6), ("H", "H", "H"))     # belly
    g.paint(disc(8.5, y - 0.5, 2.8, 2.6), ("h", "h", "h"))  # head
    g.paint(disc(9.5, y, 1.6, 1.4), ("H", "H", "H"))       # face
    g.set(10, y - 1, "x")
    g.set(7, y - 3, "H")   # ear
    for i in range(6):   # the tail curling up behind
        t = i / 5
        g.set(int(1 - math.sin(t * 3) * 1.5), int(y + 3 - t * 6), "h")
    if drop:   # arms and legs flung up
        g.fill(line(4, y + 1, 2, 0) | line(7, y + 1, 9, 0), "h")
        g.fill(line(4, y + 4, 3, 11) | line(7, y + 4, 8, 11), "h")
    else:
        step = ENEMY_FRAMES.index(frame)
        for i, x in enumerate((3, 7)):
            dx = (-1, 1, 1, -1)[(step + i * 2) % 4]
            g.fill(line(x, y + 4, x + dx, 11), "h")
            g.set(x + dx, 11, "W")
    return g


def rhino(frame):
    """RHINO (from RHINO_A/B): a black armoured rhino beetle-beast, tan horn, red eye, lime plates (19x15 art).
    TELL: head down, horn cocked back; ATTACK: the horn thrust up and forward."""
    g = Grid(19, 15)
    step = {"WALK_1": 0, "WALK_2": 1, "WALK_3": 0, "WALK_4": -1, "TELL": 0, "ATTACK": 1}[frame]
    g.paint(disc(8, 7, 7.5, 5) & rect(0, 0, 19, 11), ("x", "x", "X"))   # the armoured back
    for x in (3, 7, 11):   # lime plates
        g.fill(rect(x, 4, 2, 1), "l")
        g.set(x + 1, 5, "G")
    g.fill(line(2, 9, 14, 9), "X")
    head_y = 8 if frame == "TELL" else 6
    g.paint(disc(15, head_y, 3, 2.8), ("x", "x", "X"))
    g.set(16, head_y - 1, "r")
    tip = {"ATTACK": (19, head_y - 8), "TELL": (15, head_y - 8)}.get(frame, (18, head_y - 8))
    g.paint(poly([(15, head_y - 1), (18, head_y), tip]) | capsule(16.5, head_y - 1, tip[0], tip[1], 1.0), ("H", "H", "w"))
    g.set(17, head_y - 2, "h")
    for (x, dx) in ((3, step), (7, -step), (11, step), (14, -step)):   # stout legs
        g.paint(rect(x + dx, 10, 2, 4), ("x", "x", "X"))
        g.fill(rect(x + dx - 1, 14, 3, 1), "X")
    return g


ENEMIES = [("LEAFHOPPER", leafhopper), ("BEETLE", beetle), ("STAG", stag), ("TREE_MONKEY", tree_monkey), ("RHINO", rhino)]
BOSS_FRAMES = ["IDLE_1", "IDLE_2", "TELL", "ATTACK", "HURT", "PHASE2"]


def monkey_king(frame):
    """MONKEY KING (from MONKEY_A/B, 32x20 → 38x44 art): a big brown monkey on its haunches with a gold crown, a tan
    face and belly, a long curled tail; TELL raises a banana, ATTACK throws, PHASE2 bares its teeth, red-faced."""
    g = Grid(38, 44)
    p2 = frame == "PHASE2"
    bob = 1 if frame == "IDLE_2" else 0
    for i in range(14):   # the tail curling behind
        t = i / 13
        g.paint(disc(4 + math.sin(t * 4) * 3, 40 - t * 22, 1.4), ("W", "h", "h"))
    g.paint(disc(17, 30 + bob, 11, 10), ("W", "h", "h"))   # body
    g.paint(disc(18, 31 + bob, 6.5, 7), ("h", "H", "H"))   # belly
    g.paint(disc(19, 14 + bob, 8.5, 8), ("W", "h", "h"))   # head
    face = ("r", "r", "o") if p2 else ("h", "H", "H")
    g.paint(disc(21, 16 + bob, 6, 5.4), face)
    for x in (18, 24):
        g.paint(disc(x, 14 + bob, 1.6), ("w", "w", "w"))
        g.set(x + (0 if frame == "HURT" else 1), 14 + bob, "x" if frame != "HURT" else "w")
    if frame == "HURT":
        g.fill(rect(16, 14 + bob, 4, 1) | rect(22, 14 + bob, 4, 1), "x")
    mouth = rect(18, 19 + bob, 7, 3 if p2 or frame in ("TELL", "ATTACK") else 1)
    g.fill(mouth, "R")
    if p2:
        g.fill(rect(18, 19 + bob, 7, 1), "w")
    g.paint(disc(11, 13 + bob, 2.4), ("h", "H", "H"))   # ear
    # The crown: gold with red gems.
    g.fill(poly([(12, 7 + bob), (26, 7 + bob), (27, 1 + bob), (23, 4 + bob), (19, 0 + bob), (15, 4 + bob), (11, 1 + bob)]), "Y")
    g.fill(rect(12, 6 + bob, 15, 1), "o")
    for x in (15, 19, 23):
        g.set(x, 5 + bob, "r")
    # Legs: big feet in front.
    g.paint(capsule(10, 38, 12, 42, 3), ("W", "h", "h"))
    g.paint(capsule(24, 38, 27, 42, 3), ("W", "h", "h"))
    g.fill(rect(8, 43, 7, 1) | rect(23, 43, 7, 1), "W")
    # Arms: the throwing arm is the front one.
    if frame == "TELL":   # raised high with a banana
        g.paint(capsule(26, 26, 32, 8, 2.4), ("W", "h", "h"))
        g.paint(poly([(30, 5), (32, 2), (36, 4), (35, 8), (33, 7)]), ("Y", "Y", "Y"))
        g.set(32, 2, "W")
    elif frame == "ATTACK":   # flung forward
        g.paint(capsule(26, 26, 36, 22, 2.4), ("W", "h", "h"))
    else:
        g.paint(capsule(26, 26, 29, 36, 2.4), ("W", "h", "h"))
    g.paint(capsule(9, 26, 7, 36, 2.4), ("W", "h", "h"))
    return g


def silverback(frame):
    """SILVERBACK (from GORILLA_*, 32x20 → 52x40 art): a huge gorilla on its knuckles, dark leathery face, red eyes;
    TELL pounds its chest (GORILLA_POUND), ATTACK slams both fists down; PHASE2 roars with a silver back."""
    g = Grid(52, 40)
    p2 = frame == "PHASE2"
    pound = frame == "TELL"
    slam = frame == "ATTACK"
    bob = 1 if frame == "IDLE_2" else 0
    g.paint(disc(22, 20 + bob, 18, 14), ("W", "P", "P"))   # the hunched back and body
    g.paint(disc(20, 12 + bob, 14, 7) & rect(0, 0, 52, 16 + bob), ("P", "S", "S") if not p2 else ("s", "b", "b"))   # shoulders
    if p2:
        g.fill(line(10, 10 + bob, 30, 9 + bob), "w")
    g.paint(disc(39, 13 + bob, 9.5, 9), ("W", "P", "P"))   # head
    g.paint(disc(42, 16 + bob, 7, 6), ("x", "X", "X"))   # the face
    g.fill(rect(36, 12 + bob, 12, 2), "x")   # the brow
    eye = "r" if frame != "HURT" else "w"
    g.fill(rect(39, 14 + bob, 2, 1) | rect(44, 14 + bob, 2, 1), eye)
    g.set(42, 17 + bob, "W")   # nostrils
    g.set(44, 17 + bob, "W")
    if p2 or pound or slam:   # a roaring mouth
        g.fill(rect(39, 19 + bob, 8, 3), "R")
        g.fill(rect(39, 19 + bob, 8, 1), "w")
    else:
        g.fill(rect(40, 20 + bob, 5, 1), "W")
    # Legs behind.
    g.paint(capsule(10, 30, 8, 37, 3.6), ("W", "P", "P"))
    g.paint(capsule(20, 31, 20, 37, 3.6), ("W", "P", "P"))
    g.fill(rect(4, 38, 9, 2) | rect(16, 38, 9, 2), "W")
    # Arms: knuckle-walking, pounding the chest, or slammed down in front.
    if pound:
        g.paint(capsule(30, 18, 34, 24, 3.4), ("W", "P", "S"))
        g.paint(capsule(36, 22, 38, 28, 3.4), ("W", "P", "S"))
        g.paint(disc(34, 26, 3.4), ("W", "W", "P"))
        g.paint(disc(38, 28, 3.4), ("W", "W", "P"))
    elif slam:
        g.paint(capsule(32, 18, 44, 34, 4), ("W", "P", "S"))
        g.paint(disc(46, 36, 4.4, 3.6), ("W", "W", "P"))
        g.fill(rect(40, 39, 12, 1), "W")
    else:
        g.paint(capsule(30, 18, 32, 34, 4), ("W", "P", "S"))
        g.paint(capsule(38, 20, 42, 34, 3.6), ("W", "P", "S"))
        g.paint(disc(32, 36, 3.6, 3), ("W", "W", "P"))
        g.paint(disc(42, 36, 3.4, 3), ("W", "W", "P"))
    return g


def write_frames():
    out = []
    for name, fn in ENEMIES:
        for f in ENEMY_FRAMES:
            out.append(("ENEMY_%s_%s" % (name, f), finish(fn(f))))
    for f in BOSS_FRAMES:
        out.append(("BOSS_MONKEY_KING_" + f, finish(monkey_king(f))))
    for f in BOSS_FRAMES:
        out.append(("BOSS_SILVERBACK_" + f, finish(silverback(f))))
    return out


def build():
    """Every world 2 palette sprite in source order: (name, Grid)."""
    out = []
    for shape in SHAPES:
        variants = 3 if shape in ("TOP", "FILL") else 1
        for v in range(variants):
            out.append(("GREEN_%s_%d" % (shape, v + 1) if variants > 1 else "GREEN_" + shape, ground_tile(shape, v)))
    for v in range(3):
        out.append(("GREEN_FRINGE_%d" % (v + 1), fringe(v)))
    out.append(("GREEN_DRAPE_LEFT", drape(False)))
    out.append(("GREEN_DRAPE_RIGHT", drape(True)))
    for end in ("left", "mid", "right"):
        out.append(("GREEN_BRANCH_" + end.upper(), branch(end)))
    out.append(("GREEN_BRACKET", bracket()))
    out.append(("GREEN_HANGING_VINE", hanging_vine()))
    out.append(("GREEN_POST", post()))
    out.append(("GREEN_VINE", vine(False)))
    out.append(("GREEN_VINE_TOP", vine(True)))
    for f in range(3):
        out.append(("GREEN_WATER_%d" % (f + 1), water(f)))
    out.append(("GREEN_THORNS", thorns()))
    out.append(("GREEN_THORN_GATE", thorn_gate()))
    out.append(("GREEN_PALM", palm_crown(True)))
    out.append(("GREEN_PALM_BARE", palm_crown(False)))
    out.append(("GREEN_PALM_TRUNK", palm_trunk()))
    for f in range(2):
        out.append(("GREEN_FERN_%d" % (f + 1), fern(f)))
    out.append(("GREEN_MUSHROOMS_BROWN", mushroom(0)))
    out.append(("GREEN_MUSHROOM_TALL", mushroom(1)))
    for f in range(2):
        out.append(("GREEN_MUSHROOMS_GLOW_%d" % (f + 1), mushroom(2, f)))
    out.append(("GREEN_FLOWERS", flowers()))
    out.append(("GREEN_LEAVES", fallen_leaves()))
    out.append(("GREEN_ROOT", root_loop()))
    out.append(("GREEN_SNAIL", snail()))
    out.append(("GREEN_STONE", mossy_stone()))
    for f in range(2):
        out.append(("GREEN_CEILING_VINES_%d" % (f + 1), ceiling_vines(f)))
    for f in range(2):
        out.append(("GREEN_DRIP_%d" % (f + 1), drip(f)))
    out.append(("GREEN_WALL_FERN", wall_fern()))
    out.append(("GREEN_BONE", bone()))
    out.append(("GREEN_STATUE_HEAD", statue_head()))
    out.append(("GREEN_TEMPLE_STAIRS", temple_stairs()))
    for f in range(2):
        out.append(("GREEN_WATERFALL_%d" % (f + 1), waterfall(f)))
    for f in range(2):
        out.append(("GREEN_FLOWER_%d" % (f + 1), giant_flower(f)))
    out.append(("GREEN_ROPE_BRIDGE", rope_bridge()))
    for f in range(2):
        out.append(("GREEN_IDOL_%d" % (f + 1), idol(f)))
    return out


def write_all(write, sprite_text, root):
    sprites = build()
    characters = write_frames()
    h = ["// World 2's art (spec §13 row 2, §22) and its enemies and bosses (§8, §12), generated by",
         "// docs/work-plan/name-quest/art-tools/make_quest_art.py (world2.py): edit the script, not this file.",
         "// GREEN_* are palette sprites ('0'-'f' index GREEN_PALETTE); ENEMY_* and BOSS_* use the global palette.",
         "#pragma once", "#include \"screen.h\"", "",
         "// Soil and bark, moss and leaves, the deep green background, light (mushroom teal, sun gold, bone); dark → light.",
         "const WorldPalette GREEN_PALETTE = {{" + ", ".join("0xFF%06X" % c for c in PALETTE) + "}};", ""]
    h += ["extern const Sprite %s;" % name for name, g in sprites + characters]
    write(os.path.join(root, "source", "quest_sprites_2.h"), "\n".join(h) + "\n")
    cpp = ["// World 2's art, generated by docs/work-plan/name-quest/art-tools/make_quest_art.py (world2.py).",
           "#include \"quest_sprites_2.h\"", ""]
    cpp += [sprite_text(name, g, "extern const Sprite") for name, g in sprites + characters]
    write(os.path.join(root, "source", "quest_sprites_2.cpp"), "\n".join(cpp))


def preview(folder, save_png, global_palette):
    rgb = {("%x" % i): ((c >> 16) & 255, (c >> 8) & 255, c & 255) for i, c in enumerate(PALETTE)}
    gpal = global_palette()
    for (name, sprites, colours, bg) in (("world2.png", build(), rgb, (90, 20, 90)),
                                         ("world2_characters.png", write_frames(), gpal, (40, 44, 70))):
        rows, row, x, row_h, width = [], [], 0, 0, 1100
        for _, g in sprites:
            if x + g.w + 4 > width:
                rows.append((row, row_h))
                row, x, row_h = [], 0, 0
            row.append((x, g))
            x += g.w + 4
            row_h = max(row_h, g.h + 4)
        rows.append((row, row_h))
        img = [[bg] * width for _ in range(sum(rh for _, rh in rows))]
        y0 = 0
        for row, rh in rows:
            for (x0, g) in row:
                for y in range(g.h):
                    for xx in range(g.w):
                        if g.c[y][xx] != ".":
                            img[y0 + y + 2][x0 + xx + 2] = colours[g.c[y][xx]]
            y0 += rh
        save_png(os.path.join(folder, name), width, len(img), img, 2)
