"""World 1, The Drifting Station (spec §13 row 1, §22): tiles, platforms, props and landmarks in the world palette
('0'-'3' steel, '4'-'7' emergency red and hazard amber, '8'-'b' space, 'c'-'f' console light, dark → light), plus the
roster's five enemies and the two bosses in the global palette (spec §8, §12), drawn like the heroes: art pixels with a
baked outline, doubled. Used by make_quest_art.py, which writes source/quest_sprites_1.h/.cpp."""
import math
import os
import random

from make_quest_art import Grid, arc, capsule, disc, line, poly, rect

PALETTE = [0x161922, 0x272C38, 0x434B5C, 0x6E788A,    # steel: cool blue-grey
           0x40181A, 0x8E2E26, 0x9C7A26, 0xD2B450,    # accent: emergency red, hazard amber
           0x06060D, 0x0D0E1C, 0x17192E, 0x34375A,    # space: black to a far violet grey
           0x1E5664, 0x3E9EAA, 0x8ADCE2, 0xEEF6F2]    # console light: teal to white
STEEL, RED, AMBER, SPACE, GLOW = ("1", "2", "3"), ("4", "5", "5"), ("6", "6", "7"), ("9", "a", "b"), ("c", "d", "e")
DARK_STEEL = ("0", "1", "2")


def rng(*key):
    return random.Random("station-" + "-".join(str(k) for k in key))


# --- Ground tiles (16x16): riveted plating with hazard stripes at ledges, over dark hull panels with vents and bolts ---
SHAPES = ["TOP", "TOP_LEFT", "TOP_RIGHT", "LEFT", "RIGHT", "FILL", "BOTTOM", "BOTTOM_LEFT", "BOTTOM_RIGHT", "COLUMN_TOP",
          "COLUMN", "SINGLE"]
OPEN = {"TOP": "t", "TOP_LEFT": "tl", "TOP_RIGHT": "tr", "LEFT": "l", "RIGHT": "r", "FILL": "", "BOTTOM": "b",
        "BOTTOM_LEFT": "bl", "BOTTOM_RIGHT": "br", "COLUMN_TOP": "tlr", "COLUMN": "lr", "SINGLE": "tlrb"}


def hull(g, r, variant, y0=0):
    """Dark hull panels: a seam grid, bolts at the panel corners, now and then a vent slot or a hatch."""
    for y in range(y0, 16):
        for x in range(16):
            g.set(x, y, "1" if r.random() > 0.06 else "0")
    seam_x = r.choice((7, 8, 9))
    for y in range(y0, 16):
        g.set(seam_x, y, "0")
    for x in range(16):
        g.set(x, 15, "0" if x % 5 else "1")
    for (x, y) in ((seam_x - 2, y0 + 2), (seam_x + 2, y0 + 2), (seam_x - 2, 13), (seam_x + 2, 13)):
        if y0 <= y < 16:
            g.set(x, y, "2")
            g.set(x + 1, y + 1, "0")
    if variant == 1:   # vent slots
        for y in range(max(y0 + 4, 6), 12, 2):
            g.fill(rect(1, y, 5, 1), "0")
            g.fill(rect(1, y + 1, 5, 1), "2")
    elif variant == 2:   # a hatch with a warning light gone dark
        g.fill(rect(10, max(y0 + 4, 6), 5, 6), "0")
        g.fill(rect(11, max(y0 + 5, 7), 3, 4), "2")
        g.set(12, max(y0 + 6, 8), "4")


def plating(g, r, variant, hazard_left, hazard_right):
    """Rows 0-5: a bright top line, riveted plate, a dark seam; hazard stripes on ledge ends."""
    for x in range(16):
        g.set(x, 0, "3")
        g.set(x, 1, "2" if r.random() > 0.1 else "3")
        for y in range(2, 5):
            g.set(x, y, "2")
        g.set(x, 5, "0")
    for x in range(1 + variant, 16, 5):   # rivets
        g.set(x, 3, "3")
        g.set(x + 1, 4, "1")
    if variant == 2:   # a scuffed plate
        for x in range(9, 14):
            g.set(x, 2, "1")
    for side, on in (("l", hazard_left), ("r", hazard_right)):
        if not on:
            continue
        for x in range(7):
            for y in range(1, 5):
                xx = x if side == "l" else 15 - x
                g.set(xx, y, "7" if (x + y) % 4 < 2 else "0")


def ground_tile(shape, variant):
    r = rng("tile", shape, variant)
    g = Grid(16, 16)
    sides = OPEN[shape]
    hull(g, r, variant if shape == "FILL" else 0, 6 if "t" in sides else 0)
    if "t" in sides:
        plating(g, r, variant, "l" in sides, "r" in sides)
    for side in "lr":
        if side not in sides:
            continue
        x0, x1 = (0, 1) if side == "l" else (15, 14)
        for y in range(16):
            g.set(x0, y, "0")
            if g.get(x1, y) not in "7":
                g.set(x1, y, "2" if y % 6 else "3")
    if "b" in sides:
        for x in range(16):
            g.set(x, 15, "0")
            g.set(x, 14, "2" if x % 4 else "1")
    for (corner, pts) in (("tl", ((0, 0), (1, 0), (0, 1))), ("tr", ((15, 0), (14, 0), (15, 1))),
                          ("bl", ((0, 15), (0, 14), (1, 15))), ("br", ((15, 15), (15, 14), (14, 15)))):
        if all(c in sides for c in corner):
            for (x, y) in pts:
                g.set(x, y, ".")
    return g


def fringe(variant):
    """The plate's lip over the top line: bolt heads, a cable clip, a stub pipe (rows 0-5 above, 6-7 on the cap)."""
    r = rng("fringe", variant)
    g = Grid(16, 8)
    for x in range(16):
        g.set(x, 7, "3")
    for x in range(r.randint(1, 3), 16, r.choice((5, 6))):
        g.set(x, 6, "3")
        g.set(x + 1, 6, "2")
    if variant == 1:   # a cable clip
        g.fill(rect(5, 5, 4, 2), "1")
        g.set(6, 5, "2")
    if variant == 2:   # a stub pipe with a cap
        g.paint(rect(10, 3, 3, 4), STEEL)
        g.fill(rect(9, 2, 5, 1), "2")
    return g


def drape(right):
    """Cables hanging down the side of a ledge: 8x16, attached on the tile's left (flipped for the right)."""
    r = rng("drape", right)
    g = Grid(8, 16)
    for (x, length, col) in ((5, r.randint(6, 12), "1"), (6, r.randint(3, 8), "4"), (7, r.randint(9, 15), "2")):
        for y in range(length):
            g.set(x - (1 if y > length - 3 and x == 5 else 0), y + 1, col)
        g.set(x, length + 1, "0")
    for x in range(4, 8):
        g.set(x, 0, "2")
    if right:
        g.c = [row[::-1] for row in g.c]
    return g


# --- Platforms (16x12): a grated catwalk with a rail truss; supports; a metal ladder; liquid; the space pit ---
def catwalk(end):
    g = Grid(16, 12)
    body = rect(0, 0, 16, 4)
    g.paint(body, STEEL)
    for x in range(0, 16, 2):   # the grating
        g.set(x, 1, "0")
        g.set(x + 1, 2, "0")
    g.fill(rect(0, 0, 16, 1), "3")
    g.fill(rect(0, 4, 16, 1), "0")
    for x in range(16):   # the truss under it
        g.set(x, 9, "2")
        g.set(x, 10, "0")
        d = x % 8
        g.set(x, 5 + (d if d < 4 else 7 - d), "1")
    if end in ("left", "right"):
        x = 1 if end == "left" else 13
        g.paint(rect(x, 0, 2, 10), STEEL)
        g.set(x, 5, "6")
        g.set(x, 6, "0")
    return g


def bracket():
    """A steel brace under a catwalk end, its left side against a wall (flipped for walls on the right)."""
    g = Grid(16, 16)
    g.paint(rect(0, 0, 4, 16), STEEL)
    g.paint(capsule(2, 14, 14, 1, 1.4), STEEL)
    g.fill(rect(0, 0, 16, 2), "1")
    for y in (3, 12):
        g.set(1, y, "3")
    return g


def chain():
    """A hanging cable with clamps, from a catwalk up to a ceiling."""
    g = Grid(4, 16)
    for y in range(16):
        g.set(1, y, "1")
        g.set(2, y, "2" if y % 4 else "3")
    g.fill(rect(0, 7, 4, 2), "0")
    return g


def strut():
    g = Grid(6, 16)
    g.paint(rect(0, 0, 6, 16), STEEL)
    for y in range(0, 16, 4):
        g.fill(line(1, y, 4, y + 3), "0")
    return g


def ladder(top):
    g = Grid(16, 16)
    for x0 in (2, 12):
        g.paint(rect(x0, 0, 2, 16), STEEL)
    for y0 in (3, 9, 15):
        g.fill(rect(4, y0, 8, 1), "2")
        g.fill(rect(4, y0 - 1, 8, 1), "3")
    if top:
        for y in range(0, 2):
            for x in range(16):
                g.set(x, y, ".")
        g.fill(arc(3, 3, 2, 180, 360, 1.2), "3")
        g.fill(arc(13, 3, 2, 180, 360, 1.2), "3")
    return g


def coolant(frame):
    """A coolant spill's surface (world 1 has no deadly liquid; the slot is filled for the art system)."""
    g = Grid(16, 8)
    for x in range(16):
        crest = 1 if (x + frame * 5) % 16 in (3, 4, 11) else 0
        g.set(x, 2 - crest, "e" if (x + frame * 3) % 9 == 0 else "d")
        for y in range(3 - crest, 8):
            g.set(x, y, "d" if y < 4 else "c")
    return g


def space_pit(variant):
    """A pit tile open to space: black with stars, so a pit never reads as floor (step 3)."""
    r = rng("pit", variant)
    g = Grid(16, 16)
    for y in range(16):
        for x in range(16):
            g.set(x, y, "8")
    for _ in range(3):
        g.set(r.randint(0, 15), r.randint(0, 15), r.choice("abb"))
    return g


# --- Props (spec §22: scattered, 4-16 dp; none looks like a panel, vent or spike) ---
def cable_coil():
    g = Grid(14, 6)
    for (cx, col) in ((5, "1"), (9, "4")):
        g.fill(arc(cx, 5, 4, 180, 360, 1.4), col)
    g.fill(rect(0, 5, 14, 1), "0")
    return g


def bolts():
    g = Grid(10, 4)
    for (x, y) in ((1, 3), (4, 2), (7, 3), (8, 2)):
        g.paint(rect(x, y, 2, 4 - y), STEEL)
    return g


def broken_screen(frame):
    g = Grid(16, 12)
    g.paint(rect(0, 0, 16, 12), DARK_STEEL)
    g.fill(rect(2, 2, 12, 7), "9")
    g.fill(line(4, 2, 9, 8) | line(9, 3, 13, 6), "b")
    if frame:
        g.fill(rect(3, 3 + frame, 4, 1), "c")
    g.fill(rect(6, 10, 4, 2), "0")
    return g


def crate():
    g = Grid(18, 14)
    g.paint(rect(0, 0, 18, 14), STEEL)
    g.paint(rect(2, 2, 14, 10), DARK_STEEL)
    g.fill(rect(0, 6, 18, 2), "6")
    g.fill(rect(4, 6, 2, 2) | rect(12, 6, 2, 2), "0")
    for (x, y) in ((1, 1), (16, 1), (1, 12), (16, 12)):
        g.set(x, y, "3")
    return g


def warning_sign():
    g = Grid(12, 16)
    g.paint(rect(5, 9, 2, 7), DARK_STEEL)
    g.paint(poly([(0, 10), (12, 10), (6, 0)]), AMBER)
    g.fill(rect(5, 3, 2, 4) | rect(5, 8, 2, 1), "0")
    return g


def oxygen_tank():
    g = Grid(8, 16)
    g.paint(disc(4, 9, 4, 7) & rect(0, 3, 8, 13), ("4", "5", "5"))
    g.fill(rect(1, 7, 1, 6), "6")
    g.paint(rect(3, 0, 2, 3), STEEL)
    g.fill(rect(2, 1, 4, 1), "2")
    return g


def helmet():
    g = Grid(14, 10)
    g.paint(disc(7, 6, 6, 5) & rect(0, 0, 14, 10), STEEL)
    g.paint(disc(8, 6, 3.5, 2.6), ("c", "c", "d"))
    g.set(9, 5, "e")
    g.fill(rect(1, 9, 12, 1), "0")
    return g


def antenna(frame):
    g = Grid(8, 16)
    g.fill(line(4, 15, 4, 4), "2")
    g.paint(rect(2, 13, 5, 3), DARK_STEEL)
    g.set(4, 3, "5" if frame else "4")
    g.set(4, 2, "5" if frame else "4")
    return g


def floor_console(frame):
    """A small console with a blinking key row (light)."""
    g = Grid(16, 14)
    g.paint(poly([(1, 14), (15, 14), (15, 5), (12, 2), (1, 2)]), DARK_STEEL)
    g.fill(rect(3, 4, 9, 4), "9")
    g.fill(rect(4, 5, 7, 2), "c")
    for i, x in enumerate(range(3, 13, 3)):
        g.set(x, 10, "e" if (i + frame) % 2 else "c")
    return g


def ceiling_cable(frame):
    g = Grid(12, 14)
    for (x, length, col) in ((3, 10, "1"), (6, 6, "4"), (8, 12, "2")):
        sway = (1 if frame else 0) if x != 6 else 0
        for y in range(length):
            g.set(x + (sway if y > length // 2 else 0), y, col)
    g.fill(rect(1, 0, 10, 2), "1")
    return g


def ceiling_lamp(frame):
    """A caged emergency lamp under the ceiling (light)."""
    g = Grid(10, 10)
    g.fill(rect(4, 0, 2, 3), "1")
    g.paint(disc(5, 6, 4, 3.5) & rect(0, 3, 10, 7), ("4", "5", "5") if frame else ("4", "4", "5"))
    g.fill(line(1, 6, 9, 6) | rect(5, 3, 1, 7), "0")
    return g


def wall_pipe():
    g = Grid(8, 16)
    g.paint(rect(1, 0, 4, 16), STEEL)
    for y in (2, 11):
        g.paint(rect(0, y, 6, 3), DARK_STEEL)
    g.set(6, 12, "6")
    return g


def wall_screen(frame):
    g = Grid(12, 10)
    g.paint(rect(0, 0, 12, 10), DARK_STEEL)
    g.fill(rect(2, 2, 8, 6), "9")
    for i in range(4):   # a slow sweep line on a radar-like screen
        g.set(3 + (i + frame * 2) % 7, 4 + i % 3, "d")
    return g


# --- Landmarks (spec §22: big set pieces, 24-80 dp; ids 1-6 in this order) ---
def airlock_door():
    """A round-cornered airlock with yellow-black frame and a red lamp above (64x72)."""
    g = Grid(64, 72)
    frame = rect(4, 8, 56, 64) - rect(12, 16, 40, 56)
    g.paint(frame, STEEL)
    for (x, y) in frame:
        if x < 10 or x > 53:
            if (x + y) % 8 < 4:
                g.set(x, y, "6")
    g.paint(rect(12, 16, 40, 56), DARK_STEEL)
    g.fill(rect(31, 16, 2, 56), "0")
    for y in range(22, 70, 8):   # door ribs
        g.fill(rect(14, y, 16, 1) | rect(34, y, 16, 1), "2")
    g.paint(disc(22, 44, 4), STEEL)
    g.paint(disc(42, 44, 4), STEEL)
    g.paint(rect(26, 0, 12, 7), DARK_STEEL)
    g.paint(disc(32, 4, 3), RED)
    g.set(31, 3, "7")
    return g


def reactor(frame):
    """The reactor core: a glass column with a pulsing teal core between clamps (48x80), light."""
    g = Grid(48, 80)
    g.paint(rect(4, 70, 40, 10), STEEL)
    g.paint(rect(8, 0, 32, 8), STEEL)
    g.paint(rect(12, 8, 24, 62), ("9", "a", "a"))
    core = 5 + frame
    g.paint(capsule(24, 16, 24, 62, core), GLOW)
    g.fill(capsule(24, 18, 24, 60, core / 2.5), "f")
    for y in (20, 36, 52):
        g.paint(rect(10, y, 28, 4), DARK_STEEL)
        g.set(12, y + 1, "5")
    for x in (12, 35):
        g.fill(rect(x, 8, 1, 62), "b")
    return g


def observation_window():
    """A cracked observation window looking out at space (80x48)."""
    g = Grid(80, 48)
    g.paint(rect(0, 0, 80, 48), STEEL)
    g.fill(rect(6, 6, 68, 36), "8")
    r = rng("window")
    for _ in range(14):
        g.set(r.randint(7, 72), r.randint(7, 40), r.choice("abbe"))
    g.paint(disc(58, 22, 9), ("9", "a", "b"))   # the ringed planet, far away
    g.fill(arc(58, 22, 14, 160, 380, 1.0), "b")
    g.fill(line(20, 6, 30, 22) | line(30, 22, 26, 41) | line(30, 22, 44, 30) | line(30, 22, 12, 34), "e")
    for x in range(6, 74, 17):
        g.fill(rect(x, 6, 2, 36), "1")
    g.fill(rect(0, 44, 80, 4), "6")
    return g


def shuttle():
    """A wrecked shuttle lying on its side (80x40)."""
    g = Grid(80, 40)
    body = poly([(2, 38), (8, 14), (52, 10), (72, 18), (78, 32), (74, 38)])
    g.paint(body, STEEL)
    g.paint(poly([(52, 12), (70, 19), (74, 28), (56, 26)]), ("c", "d", "d"))   # the cockpit glass
    g.fill(line(56, 14, 70, 26), "1")
    for x in range(12, 50, 9):
        g.fill(rect(x, 16, 1, 20), "1")
    g.fill(rect(14, 24, 12, 8), "0")   # a torn hull hole
    g.paint(poly([(10, 16), (22, 2), (30, 4), (22, 14)]), DARK_STEEL)   # the broken wing
    g.fill(rect(32, 30, 12, 2), "5")
    return g


def server_rack(frame):
    """Two server racks with blinking lights (40x56), light."""
    g = Grid(40, 56)
    for x0 in (0, 21):
        g.paint(rect(x0, 0, 19, 56), DARK_STEEL)
        for y in range(4, 52, 6):
            g.fill(rect(x0 + 2, y, 15, 4), "0")
            g.fill(rect(x0 + 3, y + 1, 8, 1), "2")
            k = (y // 6 + x0 + frame) % 3
            g.set(x0 + 14, y + 2, "e" if k == 0 else "c" if k == 1 else "5")
    return g


def crew_mural():
    """A faded crew mural: five figures under a star (64x40)."""
    g = Grid(64, 40)
    g.paint(rect(0, 0, 64, 40), ("0", "1", "1"))
    g.fill(rect(3, 3, 58, 34), "a")
    g.paint(disc(32, 10, 4), AMBER)
    for i, x in enumerate(range(10, 60, 11)):
        col = ("5", "c", "6", "d", "2")[i]
        g.fill(disc(x, 20, 3), col)
        g.fill(rect(x - 3, 24, 7, 10), col)
    for (x, y) in ((8, 30), (40, 12), (52, 33)):   # flaked paint
        g.fill(rect(x, y, 4, 2), "1")
    return g


# --- Enemies and bosses (global palette; art px with a baked 1 px outline, then doubled; facing right) ---
def finish(g, outline="x"):
    """The art grid inside a 1 px border with the outline around every shape, doubled to real px."""
    w, h = g.w + 2, g.h + 2
    o = Grid(w, h)
    for y in range(g.h):
        for x in range(g.w):
            if g.c[y][x] != ".":
                o.set(x + 1, y + 1, g.c[y][x])
    for (x, y) in [(x, y) for y in range(h) for x in range(w)]:
        if o.get(x, y) == "." and any(o.get(x + dx, y + dy) not in (".", "#") for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
            o.set(x, y, "#")
    big = Grid(w * 2, h * 2)
    for y in range(h):
        for x in range(w):
            ch = o.get(x, y)
            for dy in range(2):
                for dx in range(2):
                    big.set(x * 2 + dx, y * 2 + dy, outline if ch == "#" else ch)
    return big


ENEMY_FRAMES = ["WALK_1", "WALK_2", "WALK_3", "WALK_4", "TELL", "ATTACK"]


def crawler(frame):
    """CRAWLER (from GREEN_ALIEN): a six-legged alien up on its legs, antennae forward (14x12 art). At least 12 dp tall
    with its outline (spec §22), so its hitbox reaches the line an Archer's arrow flies along."""
    g = Grid(14, 12)
    lift = 2 if frame == "TELL" else 0
    reach = 2 if frame == "ATTACK" else 0
    body = disc(6.5 + reach, 5.5 - lift / 2, 5.5 + reach / 2, 3.2) | disc(10 + reach, 4.5 - lift, 2.8, 2.6)
    g.paint(body, ("G", "g", "l"))
    for (x, y) in ((3 + reach, 4), (6 + reach, 4)):
        g.set(x, y - lift // 2, "G")   # back spots
    g.set(11 + reach, 4 - lift, "w")
    g.set(12 + reach, 4 - lift, "k")
    g.fill(line(10 + reach, 3 - lift, 12 + reach, 0), "g")
    g.set(13 + reach, 0, "l")
    if frame in ("TELL", "ATTACK"):
        g.set(12 + reach, 6 - lift, "r")
        g.set(13 + reach, 6 - lift, "r")
    step = ENEMY_FRAMES.index(frame) if frame.startswith("WALK") else -1
    for i, x in enumerate((2, 5, 8)):   # three legs from the belly to the floor, each foot stepping in turn
        dx = (-1, 0, 1, 0)[(step + i) % 4] if step >= 0 else 0
        g.fill(line(x, 8, x + dx, 11), "G")
    return g


def floater(frame):
    """FLOATER (from BLUE_ALIEN): a domed jelly alien with swaying tentacles (12x13 art)."""
    g = Grid(12, 13)
    dive = frame == "ATTACK"
    g.paint(disc(6, 4.5, 5, 4.2) & rect(0, 0, 12, 7), ("U", "u", "c"))
    g.fill(rect(1, 6, 10, 1), "U")
    eye = "w" if frame == "TELL" else "k"
    g.set(4, 4, "w")
    g.set(8, 4, "w")
    g.set(5, 4, eye)
    g.set(9, 4, eye)
    sway = {"WALK_1": (0, 1, 0, -1), "WALK_2": (1, 0, -1, 0), "WALK_3": (0, -1, 0, 1), "WALK_4": (-1, 0, 1, 0),
            "TELL": (0, 0, 0, 0), "ATTACK": (-1, -1, -1, -1)}[frame]
    for i, x in enumerate((2, 5, 7, 10)):
        length = 3 if frame == "TELL" else 6
        for y in range(length):
            g.set(x + (sway[i] if y > 2 else 0) - (y // 3 if dive else 0), 7 + y, "u" if y < 4 else "U")
    return g


def gunner(frame):
    """GUNNER (from RED_ALIEN): a squat red alien with an arm cannon (13x12 art)."""
    g = Grid(13, 12)
    bob = 1 if frame in ("WALK_2", "WALK_4") else 0
    g.paint(disc(5.5, 5.5 + bob, 4.5, 3.8), ("R", "r", "o"))
    g.fill(line(3, 2 + bob, 2, 0) | line(7, 2 + bob, 8, 0), "r")
    g.set(6, 5 + bob, "w")
    g.set(7, 5 + bob, "k")
    recoil = -1 if frame == "ATTACK" else 0
    g.paint(rect(7 + recoil, 6 + bob, 6, 3), ("d", "s", "b"))
    g.set(12 + recoil, 7 + bob, "o" if frame == "TELL" else "k")
    if frame == "ATTACK":
        g.set(12, 6 + bob, "w")
        g.set(12, 8 + bob, "w")
    feet = {"WALK_1": (2, 7), "WALK_2": (3, 6), "WALK_3": (2, 7), "WALK_4": (3, 6), "TELL": (2, 7), "ATTACK": (1, 6)}[frame]
    for x in feet:
        g.fill(rect(x, 9 + bob, 2, 3 - bob), "R")
    return g


def hopper(frame):
    """HOPPER DRONE (new, 12x10): a round drone on a spring leg, one lens eye."""
    g = Grid(12, 11)
    squash = {"WALK_1": 0, "WALK_2": 1, "WALK_3": 0, "WALK_4": -1, "TELL": 2, "ATTACK": -2}[frame]
    top = 1 + squash
    g.paint(disc(6, top + 3.5, 5, 3.5 - squash * 0.3), ("d", "s", "b"))
    g.fill(rect(1, top + 4, 10, 1), "d")
    g.paint(disc(8, top + 3, 1.6), ("U", "c", "w") if frame != "TELL" else ("R", "r", "o"))
    g.fill(line(4, top, 3, top - 1), "s")
    g.set(3, max(top - 1, 0), "r")
    spring_top = top + 7
    for y in range(spring_top, 11):
        g.set(5 + (y % 2), y, "b" if y % 2 else "s")
    g.fill(rect(3, 10, 6, 1), "d")
    return g


def sentinel(frame):
    """SENTINEL (new, 16x16): an armoured guard robot, a shield in front and a blade arm; tell = blade raised back."""
    g = Grid(16, 17)
    step = {"WALK_1": 0, "WALK_2": 1, "WALK_3": 0, "WALK_4": -1, "TELL": 0, "ATTACK": 1}[frame]
    g.paint(rect(4, 1, 7, 5), ("d", "s", "b"))   # helmet
    g.fill(rect(6, 3, 5, 1), "r" if frame != "TELL" else "o")   # visor
    g.paint(rect(3, 6, 9, 6), ("d", "s", "b"))   # body
    g.fill(rect(5, 8, 4, 1), "R")
    g.paint(rect(11, 5, 3, 8), ("U", "u", "c"))   # the shield in front
    g.set(12, 8, "w")
    for (x, dx) in ((4, step), (8, -step)):   # legs
        g.paint(rect(x + dx, 12, 3, 4), ("x", "d", "s"))
        g.fill(rect(x + dx - 1, 16, 4, 1), "d")
    if frame == "TELL":   # the blade arm raised behind
        g.fill(line(3, 7, 0, 1) | line(4, 7, 1, 1), "b")
        g.set(0, 0, "w")
    elif frame == "ATTACK":   # swung low in front
        g.fill(line(10, 11, 15, 14) | line(10, 12, 15, 15), "b")
        g.set(15, 13, "w")
    else:
        g.fill(line(3, 8, 2, 14), "b")
        g.set(2, 15, "w")
    return g


ENEMIES = [("CRAWLER", crawler), ("FLOATER", floater), ("GUNNER", gunner), ("HOPPER", hopper), ("SENTINEL", sentinel)]
BOSS_FRAMES = ["IDLE_1", "IDLE_2", "TELL", "ATTACK", "HURT", "PHASE2"]


def overseer(frame):
    """THE OVERSEER (from BOSS_A, 32x20 → 48x30 art): a hovering saucer face, blue dome, two eyes, a mouth, tentacles."""
    g = Grid(48, 30)
    p2 = frame == "PHASE2"
    g.paint(disc(24, 12, 23, 9.5), ("G", "g", "l"))
    g.paint(disc(24, 9, 9, 6.5) & rect(0, 0, 48, 14), ("U", "u", "c"))
    g.fill(arc(24, 9, 6, 200, 250, 1.0), "w")
    for x in (11, 37):
        g.paint(disc(x, 12, 4), ("s", "b", "w"))
        pupil = "o" if frame in ("TELL", "ATTACK") else "r" if p2 else "R"
        if frame == "HURT":
            g.fill(rect(x - 3, 12, 7, 1), "k")
        else:
            g.fill(disc(x + 0.5, 12.5, 1.8), pupil)
    mouth_h = 4 if frame in ("TELL", "ATTACK") else 2
    g.fill(rect(16, 17, 16, mouth_h), "R")
    g.fill(rect(17, 17, 14, 1), "r")
    if frame == "ATTACK":
        g.fill(rect(19, 18, 10, 2), "o")
    for x in (7, 40):   # horns
        g.fill(line(x, 4, x - (2 if x < 24 else -2), 0), "r")
    sway = {"IDLE_1": 0, "IDLE_2": 1, "TELL": 0, "ATTACK": -1, "HURT": 1, "PHASE2": 0}[frame]
    for i, x in enumerate((15, 21, 27, 33)):
        for y in range(21, 30):
            dx = sway * (1 if i % 2 else -1) if y > 25 else 0
            g.set(x + dx, y, "U" if y < 25 else "r")
    if p2:   # cracks across the shell
        g.fill(line(4, 10, 9, 6) | line(40, 16, 45, 12) | line(20, 4, 23, 2), "k")
        g.fill(line(5, 11, 10, 7), "o")
    return g


def star_mage(frame):
    """THE STAR MAGE (from MAGE_A, 32x20 → 30x44 art): the green alien under a tall starry hat, a wand with a star."""
    g = Grid(30, 44)
    p2 = frame == "PHASE2"
    star = "r" if p2 else "Y"
    g.paint(poly([(4, 19), (22, 19), (14, 0)]), ("U", "u", "u"))   # hat
    for (x, y) in ((12, 7), (16, 12), (10, 15), (14, 3)):
        g.set(x, y, star)
    g.fill(rect(1, 18, 24, 2), "c")   # brim
    g.paint(disc(13, 26, 10, 6), ("G", "g", "l"))   # face
    for x in (8, 17):
        g.paint(disc(x, 25, 2.2), ("s", "b", "w"))
        g.set(x + 1, 25, "k" if frame != "HURT" else "w")
    if frame == "HURT":
        g.fill(rect(6, 25, 5, 1) | rect(15, 25, 5, 1), "k")
    g.fill(rect(9, 30, 9, 1), "R")
    g.paint(poly([(5, 31), (21, 31), (24, 42), (2, 42)]), ("U", "u", "c"))   # robe
    g.fill(rect(10, 33, 1, 9) | rect(16, 33, 1, 9), "U")
    for x in (7, 17):
        g.fill(rect(x, 42, 4, 2), "r")
    # The wand: from the hand (21, 34) up to the star; raised while casting.
    tip = {"TELL": (27, 22), "ATTACK": (28, 26)}.get(frame, (25, 25))
    g.fill(line(21, 35, tip[0], tip[1] + 2), "h")
    g.set(21, 35, "g")
    for (dx, dy) in ((0, 0), (1, 0), (-1, 0), (0, 1), (0, -1)):
        g.set(tip[0] + dx, tip[1] + dy, "w" if (dx, dy) == (0, 0) else star)
    if frame == "TELL":
        g.fill(arc(tip[0], tip[1], 3, 0, 360, 1.0), "w")
    return g, tip


def write_frames():
    out = []
    for name, fn in ENEMIES:
        for f in ENEMY_FRAMES:
            out.append(("ENEMY_%s_%s" % (name, f), finish(fn(f))))
    star_tips = []
    for f in BOSS_FRAMES:
        out.append(("BOSS_OVERSEER_" + f, finish(overseer(f))))
    for f in BOSS_FRAMES:
        g, tip = star_mage(f)
        out.append(("BOSS_STAR_MAGE_" + f, finish(g)))
        star_tips.append(((tip[0] + 1) * 2 + 1, (tip[1] + 1) * 2 + 1))   # real px in the finished sprite
    return out, star_tips


def build():
    """Every world 1 palette sprite in source order: (name, Grid)."""
    out = []
    for shape in SHAPES:
        variants = 3 if shape in ("TOP", "FILL") else 1
        for v in range(variants):
            out.append(("STATION_%s_%d" % (shape, v + 1) if variants > 1 else "STATION_" + shape, ground_tile(shape, v)))
    for v in range(3):
        out.append(("STATION_FRINGE_%d" % (v + 1), fringe(v)))
    out.append(("STATION_DRAPE_LEFT", drape(False)))
    out.append(("STATION_DRAPE_RIGHT", drape(True)))
    for end in ("left", "mid", "right"):
        out.append(("STATION_CATWALK_" + end.upper(), catwalk(end)))
    out.append(("STATION_BRACKET", bracket()))
    out.append(("STATION_CABLE", chain()))
    out.append(("STATION_STRUT", strut()))
    out.append(("STATION_LADDER", ladder(False)))
    out.append(("STATION_LADDER_TOP", ladder(True)))
    for f in range(3):
        out.append(("STATION_COOLANT_%d" % (f + 1), coolant(f)))
    out.append(("STATION_PIT", space_pit(0)))
    out.append(("STATION_CABLE_COIL", cable_coil()))
    out.append(("STATION_BOLTS", bolts()))
    for f in range(2):
        out.append(("STATION_SCREEN_%d" % (f + 1), broken_screen(f)))
    out.append(("STATION_CRATE", crate()))
    out.append(("STATION_SIGN", warning_sign()))
    out.append(("STATION_TANK", oxygen_tank()))
    out.append(("STATION_HELMET", helmet()))
    for f in range(2):
        out.append(("STATION_ANTENNA_%d" % (f + 1), antenna(f)))
    for f in range(2):
        out.append(("STATION_CONSOLE_%d" % (f + 1), floor_console(f)))
    for f in range(2):
        out.append(("STATION_HANGING_%d" % (f + 1), ceiling_cable(f)))
    for f in range(2):
        out.append(("STATION_LAMP_%d" % (f + 1), ceiling_lamp(f)))
    out.append(("STATION_WALL_PIPE", wall_pipe()))
    for f in range(2):
        out.append(("STATION_WALL_SCREEN_%d" % (f + 1), wall_screen(f)))
    out.append(("STATION_AIRLOCK", airlock_door()))
    for f in range(2):
        out.append(("STATION_REACTOR_%d" % (f + 1), reactor(f)))
    out.append(("STATION_WINDOW", observation_window()))
    out.append(("STATION_SHUTTLE", shuttle()))
    for f in range(2):
        out.append(("STATION_RACKS_%d" % (f + 1), server_rack(f)))
    out.append(("STATION_MURAL", crew_mural()))
    return out


def write_all(write, sprite_text, root):
    sprites = build()
    characters, star_tips = write_frames()
    h = ["// World 1's art (spec §13 row 1, §22) and its enemies and bosses (§8, §12), generated by",
         "// docs/work-plan/name-quest/art-tools/make_quest_art.py (world1.py): edit the script, not this file.",
         "// STATION_* are palette sprites ('0'-'f' index STATION_PALETTE); ENEMY_* and BOSS_* use the global palette.",
         "#pragma once", "#include \"screen.h\"", "",
         "// Steel (cool blue-grey), emergency red and hazard amber, space, console light; dark → light per ramp.",
         "const WorldPalette STATION_PALETTE = {{" + ", ".join("0xFF%06X" % c for c in PALETTE) + "}};",
         "// The wand star's centre in each Star Mage frame (real px from the sprite's top-left; the beam starts there).",
         "const int STAR_MAGE_STAR[6][2] = {%s};" % ", ".join("{%d, %d}" % t for t in star_tips), ""]
    h += ["extern const Sprite %s;" % name for name, g in sprites + characters]
    write(os.path.join(root, "source", "quest_sprites_1.h"), "\n".join(h) + "\n")
    cpp = ["// World 1's art, generated by docs/work-plan/name-quest/art-tools/make_quest_art.py (world1.py).",
           "#include \"quest_sprites_1.h\"", ""]
    cpp += [sprite_text(name, g, "extern const Sprite") for name, g in sprites + characters]
    write(os.path.join(root, "source", "quest_sprites_1.cpp"), "\n".join(cpp))


def preview(folder, save_png, global_palette):
    rgb = {("%x" % i): ((c >> 16) & 255, (c >> 8) & 255, c & 255) for i, c in enumerate(PALETTE)}
    gpal = global_palette()
    for (name, sprites, colours, bg) in (("world1.png", build(), rgb, (90, 20, 90)),
                                         ("world1_characters.png", write_frames()[0], gpal, (40, 44, 70))):
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
