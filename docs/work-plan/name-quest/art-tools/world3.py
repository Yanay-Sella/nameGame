"""World 3, The White Silence (spec §13 row 3, §22): snow-capped blue rock, clear and cracked ice, ice-slab platforms,
freezing water, the ice-wall gate, props and landmarks in the world palette ('0'-'3' blue rock, '4'-'7' snow and ice,
'8'-'b' the cave background, 'c'-'f' light: crystal cyan and lantern amber, dark → light), plus the roster's five enemies
and the two bosses in the global palette (spec §8, §12), drawn side-view with a baked outline and doubled. Used by
make_quest_art.py, which writes source/quest_sprites_3.h/.cpp."""
import math
import os
import random

from make_quest_art import Grid, arc, capsule, disc, line, poly, rect
from world1 import OPEN, SHAPES, finish

PALETTE = [0x10182A, 0x1E2A44, 0x34466A, 0x56688E,    # blue rock: dark → light
           0x6C8CB4, 0x9CB8D8, 0xC8DCEE, 0xF4FAFF,    # snow and ice: shadow → white
           0x060A14, 0x0B1424, 0x111E34, 0x182A46,    # the cave background (layers, water depth)
           0x3E9CC8, 0x8EE4F4, 0xD89A3C, 0xF6D488]    # light: crystal cyan, its glow, lantern amber, its flame
ROCK, SNOW, ICE = ("0", "1", "2"), ("4", "5", "6"), ("5", "6", "7")
FROST = ("2", "3", "4")
CAVE, CRYSTAL, AMBER = ("8", "9", "a"), ("c", "d", "d"), ("e", "f", "f")


def rng(*key):
    return random.Random("whitesilence-" + "-".join(str(k) for k in key))


# --- Ground tiles (16x16): a snow cap with drifts over blue rock with frost veins and trapped bubbles ---
def rock(g, r, variant, y0=0):
    for y in range(y0, 16):
        for x in range(16):
            g.set(x, y, "1" if r.random() > 0.12 else "0")
    for _ in range(2 + variant):   # paler lumps of rock
        cx, cy = r.randint(2, 13), r.randint(max(y0 + 3, 4), 14)
        g.paint(disc(cx, cy, r.choice((1.8, 2.4)), 1.4) & rect(0, y0, 16, 16 - y0), ROCK)
    if variant == 1:   # a short frost vein
        g.fill(line(3, max(y0 + 5, 8), 9, max(y0 + 8, 11)), "3")
    if variant == 2:   # bubbles trapped in a pocket of ice
        g.paint(disc(9, max(y0 + 6, 9), 3.2, 2.4) & rect(0, y0, 16, 16 - y0), ("2", "3", "3"))
        for (x, y) in ((8, max(y0 + 5, 8)), (10, max(y0 + 7, 10))):
            g.set(x, y, "6")


def snow_cap(g, r, variant, left, right):
    """Rows 0-6: thick white snow with a blue shadow at its foot, then frost where it meets the rock."""
    for x in range(16):
        g.set(x, 0, "7")
        g.set(x, 1, "7" if r.random() > 0.1 else "6")
        g.set(x, 2, "7" if r.random() > 0.4 else "6")
        g.set(x, 3, "6")
        g.set(x, 4, "6" if r.random() > 0.3 else "5")
        g.set(x, 5, "5")
        g.set(x, 6, "4" if (x + variant) % 4 else "5")
        if (x * 5 + variant) % 7 == 0:
            g.set(x, 7, "4")
    for x in range(variant * 3 + 2, 16, 7):   # glints
        g.set(x, 2, "d")
    for side, on in (("l", left), ("r", right)):
        if on:   # the drift rounding over the edge
            for y in range(0, 9):
                g.set(0 if side == "l" else 15, y, "6" if y < 6 else "4")


def ground_tile(shape, variant):
    r = rng("tile", shape, variant)
    g = Grid(16, 16)
    sides = OPEN[shape]
    rock(g, r, variant if shape == "FILL" else 0, 7 if "t" in sides else 0)
    if "t" in sides:
        snow_cap(g, r, variant, "l" in sides, "r" in sides)
    for side in "lr":
        if side not in sides:
            continue
        x0, x1 = (0, 1) if side == "l" else (15, 14)
        for y in range(7 if "t" in sides else 0, 16):
            g.set(x0, y, "0")
            g.set(x1, y, "3" if (y + variant) % 6 == 0 else g.get(x1, y))
    if "b" in sides:
        for x in range(16):
            g.set(x, 15, "0")
            g.set(x, 14, "0" if x % 3 else "1")
            if x % 5 == 3:   # frost beads under an overhang
                g.set(x, 15, "5")
    for (corner, pts) in (("tl", ((0, 0), (1, 0), (0, 1))), ("tr", ((15, 0), (14, 0), (15, 1))),
                          ("bl", ((0, 15), (0, 14), (1, 15))), ("br", ((15, 15), (15, 14), (14, 15)))):
        if all(c in sides for c in corner):
            for (x, y) in pts:
                g.set(x, y, ".")
    return g


def ice_tile():
    """Clear ice (`i`): pale blue with a diagonal highlight and a white top edge; slippery, so it reads apart from snow."""
    g = Grid(16, 16)
    for y in range(16):
        for x in range(16):
            g.set(x, y, "5" if (x + y) % 9 else "4")
    g.fill(line(2, 12, 12, 2) | line(3, 12, 13, 2), "7")   # the highlight
    g.fill(line(9, 14, 14, 9), "6")
    g.fill(rect(0, 0, 16, 1), "7")
    g.fill(rect(0, 15, 16, 1), "4")
    return g


def cracked_ice(stage):
    """Crumbling ice (`c`): ice with a crack that spreads over its 0.6 s (stages 0-2), then breaks."""
    g = ice_tile()
    for y in range(16):
        for x in range(16):
            if g.get(x, y) == "5":
                g.set(x, y, "4" if (x * 3 + y) % 7 == 0 else "5")
    cracks = [line(7, 0, 9, 7) | line(9, 7, 6, 11)]
    if stage >= 1:
        cracks.append(line(9, 7, 14, 9) | line(6, 11, 2, 15) | line(9, 7, 4, 5))
    if stage >= 2:
        cracks.append(line(6, 11, 11, 15) | line(14, 9, 15, 14) | line(4, 5, 0, 7) | line(2, 2, 7, 0))
    for c in cracks:
        g.fill(c, "1")
    return g


def fringe(variant):
    """A soft lip of snow heaped over the top line (rows 3-5 above, 6-7 on the cap); a drift or frozen grass now and then."""
    r = rng("fringe", variant)
    g = Grid(16, 8)
    for x in range(16):
        h = 2 + (1 if r.random() < 0.4 else 0)
        for y in range(8 - h, 8):
            g.set(x, y, "7" if y < 8 - h + 1 else "6" if y < 7 else "7")
    if variant == 1:   # a small drift
        g.paint(disc(9, 8, 5.5, 3.6) & rect(0, 0, 16, 8), ("6", "7", "7"))
    if variant == 2:   # frozen grass stalks poking through
        for x in (3, 5, 6, 12):
            h = r.randint(3, 5)
            g.fill(rect(x, 7 - h, 1, h), "4")
            g.set(x, 7 - h, "7")
    return g


def drape(right):
    """Snow and icicle beads down the side of a ledge: 8x16, on the tile's left (flipped for the right)."""
    r = rng("drape", right)
    g = Grid(8, 16)
    for (x, length, col) in ((7, r.randint(6, 11), "6"), (6, r.randint(3, 7), "5"), (5, r.randint(1, 4), "6")):
        for y in range(length):
            g.set(x, y, col if y < length - 1 else "7")
    if right:
        g.c = [row[::-1] for row in g.c]
    return g


# --- Platforms (16x12): an ice slab with a frost rim; supports; a frozen rope ladder; freezing water; the ice wall ---
def slab(end):
    g = Grid(16, 12)
    g.paint(rect(0, 1, 16, 5), ICE)
    g.fill(rect(0, 5, 16, 1), "4")
    g.fill(rect(0, 0, 16, 1), "7")   # the frost rim
    for x in range(1, 16, 5):
        g.set(x, 3, "7")
        g.set(x + 1, 2, "7")
    for x in range(3, 16, 6):   # short frost beads under it (round, never the icicle hazard's point)
        g.fill(rect(x, 6, 1, 2), "5")
    if end == "left":
        for (x, y) in ((0, 1), (0, 5), (1, 5)):
            g.set(x, y, ".")
    if end == "right":
        for (x, y) in ((15, 1), (15, 5), (14, 5)):
            g.set(x, y, ".")
    return g


def bracket():
    """An ice shelf growing out of the wall under a platform's end (flipped for walls on the right)."""
    g = Grid(16, 16)
    g.paint(poly([(0, 0), (14, 0), (0, 14)]), ICE)
    g.fill(line(0, 0, 13, 0), "7")
    return g


def chain():
    """A frozen rope from a platform up to the ceiling, frost beads along it."""
    g = Grid(4, 16)
    for y in range(16):
        g.set(1 + (y // 4) % 2, y, "3")
    for y in (2, 10):
        g.fill(rect(0, y, 4, 2), "6")
    return g


def pillar():
    """A thin ice pillar propping a slab from the ground."""
    g = Grid(6, 16)
    g.paint(rect(1, 0, 4, 16), ICE)
    for y in range(3, 16, 6):
        g.set(2, y, "7")
    return g


def ladder(top):
    """A climbable frozen rope ladder (`|`): two ropes and wooden rungs rimed with frost; its top hooks over."""
    g = Grid(16, 16)
    for x in (3, 12):
        g.fill(rect(x, 0, 1, 16), "2")
        g.fill(rect(x + 1, 0, 1, 16), "3")
    for y in (2, 7, 12):
        g.fill(rect(3, y, 10, 2), "e")
        g.fill(rect(3, y, 10, 1), "6")
    if top:
        g.fill(rect(0, 0, 16, 2), ".")
        g.fill(arc(3, 2, 2, 180, 360, 1) | arc(12, 2, 2, 180, 360, 1), "3")
    return g


def water(frame):
    """Freezing water's surface: pale ice shimmer on deep blue, a skin of slush drifting."""
    g = Grid(16, 8)
    for x in range(16):
        g.set(x, 0, "6" if (x + frame * 3) % 5 else "7")
        for y in range(1, 8):
            g.set(x, y, "c" if y < 2 else "b" if y < 5 else "a")
    for x in range(16):
        if (x + frame * 5) % 11 < 3:
            g.set(x, 1, "d")
    return g


def ice_spikes():
    """Ice spikes (`^`): pale blue shards on a frozen base (16x16, bottom half)."""
    g = Grid(16, 16)
    g.paint(rect(0, 13, 16, 3), FROST)
    for (x, h) in ((2, 9), (6, 12), (10, 10), (14, 12)):
        g.paint(poly([(x - 2, 14), (x + 2, 14), (x, 14 - h)]), ("4", "6", "7"))
    return g


def ice_wall():
    """The gate (`g`): a wall of thick blue ice with trapped bubbles and a crack, one tile that stacks."""
    g = Grid(16, 16)
    for y in range(16):
        for x in range(16):
            g.set(x, y, "3" if (x + y * 2) % 11 else "2")
    g.fill(rect(0, 0, 2, 16), "2")
    g.fill(rect(14, 0, 2, 16), "2")
    g.fill(line(4, 0, 6, 15), "5")
    g.fill(line(10, 2, 12, 13), "4")
    for (x, y) in ((8, 4), (9, 11), (3, 9)):
        g.set(x, y, "7")
    return g


# --- Props (spec §22: 4-16 dp; none looks like the icicle hazard) ---
def snow_mound(kind):
    g = Grid(16 if kind == 0 else 12, 6)
    g.paint(disc(g.w / 2, 6, g.w / 2 - 0.5, 5 if kind == 0 else 4) & rect(0, 0, g.w, 6), SNOW)
    g.fill(rect(2, 1 if kind == 0 else 2, 4, 1), "7")
    return g


def crystals(frame):
    """Small crystal clusters glowing cyan (light), one facet glinting."""
    g = Grid(12, 12)
    for (x, h, lean) in ((3, 7, -1), (6, 11, 0), (9, 6, 1)):
        g.paint(poly([(x - 1.5, 12), (x + 1.5, 12), (x + lean + 0.5, 12 - h)]), CRYSTAL)
    g.set(6, 3 if frame else 6, "7")
    return g


def frozen_grass():
    g = Grid(12, 6)
    for (x, h) in ((1, 4), (3, 6), (5, 3), (8, 5), (10, 4)):
        g.fill(line(x, 6, x + (1 if x % 2 else -1), 6 - h), "4")
        g.set(x + (1 if x % 2 else -1), 6 - h, "7")
    return g


def bones_in_ice():
    g = Grid(16, 8)
    g.paint(disc(8, 6, 7.5, 4) & rect(0, 0, 16, 8), ("4", "5", "6"))
    g.fill(rect(4, 5, 8, 1) | rect(3, 4, 1, 3) | rect(12, 4, 1, 3), "f")   # a bone inside
    g.fill(rect(5, 6, 6, 1), "e")
    g.fill(rect(4, 2, 3, 1), "7")
    return g


def frozen_lantern(frame):
    """An explorer's lantern half sunk in snow, still burning (light)."""
    g = Grid(8, 12)
    g.fill(rect(3, 0, 2, 1) | rect(2, 1, 4, 1), "2")
    g.paint(rect(1, 2, 6, 7), ("1", "2", "2"))
    g.fill(rect(2, 3, 4, 5), "f" if frame else "e")
    g.set(3, 4, "7")
    g.paint(disc(4, 12, 4, 3) & rect(0, 0, 8, 12), SNOW)
    return g


def ice_pebbles():
    g = Grid(12, 4)
    for (x, r) in ((2, 1.6), (6, 2.2), (10, 1.4)):
        g.paint(disc(x, 3, r, r * 0.8) & rect(0, 0, 12, 4), ICE)
    return g


def frozen_shrub():
    g = Grid(14, 12)
    for (x1, y1) in ((2, 2), (7, 0), (12, 3), (4, 5), (10, 6)):
        g.fill(line(7, 12, x1, y1), "2")
        g.paint(disc(x1, y1, 1.6), ("5", "6", "7"))
    return g


def snow_rock():
    g = Grid(14, 9)
    g.paint(disc(7, 7, 6.5, 5) & rect(0, 0, 14, 9), ROCK)
    g.paint(disc(7, 3, 5.5, 2.4) & rect(0, 0, 14, 5), SNOW)
    g.fill(rect(0, 8, 14, 1), "0")
    return g


def ceiling_frost(frame):
    """Rounded frost drips hanging from the ceiling, blunt and blue-white (never the hazard's sharp point)."""
    g = Grid(14, 12)
    for (x, length) in ((2, 5), (5, 10), (8, 4), (11, 8)):
        g.paint(capsule(x, 0, x, length, 1.5 if length > 6 else 1.2), ("4", "5", "6"))
        g.set(x - 1, 1, "7")
    g.set(5 + frame, 9, "7")
    return g


def wall_frost():
    g = Grid(10, 12)
    for (y, length) in ((1, 7), (5, 9), (9, 5)):
        g.fill(line(0, y, length, y + 1), "5")
        for x in range(1, length, 3):
            g.set(x, y - 1, "7")
    return g


# --- Landmarks (spec §22: 24-80 dp; ids 1-6 in this order) ---
def cathedral_arch():
    """The ice cathedral's pointed arch with pillars and a rose window of frost (80x80)."""
    g = Grid(80, 80)
    outer = poly([(2, 80), (2, 34), (14, 12), (40, 0), (66, 12), (78, 34), (78, 80)])
    inner = poly([(16, 80), (16, 38), (24, 24), (40, 14), (56, 24), (64, 38), (64, 80)])
    g.paint(outer, ICE)
    g.fill(inner, ".")
    for x in (2, 64):   # pillars
        g.paint(rect(x, 40, 14, 40), ("4", "5", "6"))
        g.fill(rect(x + 3, 40, 2, 40), "7")
    g.paint(disc(40, 30, 8), ("3", "4", "5"))   # the rose window
    for k in range(6):
        a = k * math.pi / 3
        g.fill(line(40, 30, 40 + math.cos(a) * 7, 30 + math.sin(a) * 7), "7")
    g.fill(rect(0, 78, 80, 2), "4")
    return g


def frozen_mammoth():
    """A mammoth frozen in a block of wall ice: a dark shape behind pale ice, its tusk curling (80x56)."""
    g = Grid(80, 56)
    g.paint(rect(0, 4, 80, 52), ("3", "4", "4"))
    g.paint(disc(42, 28, 26, 17), ("1", "2", "2"))   # the body
    g.paint(disc(64, 22, 11, 10), ("1", "2", "2"))   # the head
    g.fill(capsule(70, 30, 68, 46, 2.4), "2")        # the trunk
    g.fill(arc(64, 38, 10, 20, 150, 2.2), "f")       # the tusk
    for x in (24, 36, 50):
        g.fill(rect(x, 40, 6, 12), "1")
    g.set(66, 19, "0")
    for (x0, y0, x1, y1) in ((4, 8, 20, 52), (60, 6, 76, 30)):   # ice glare over it
        g.fill(line(x0, y0, x1, y1), "6")
    g.fill(rect(0, 4, 80, 1) | rect(0, 55, 80, 1), "5")
    return g


def frozen_waterfall(frame):
    """A waterfall frozen mid-fall: wavy pale columns spilling over a snowy lip, a glint running down, a frozen pool (40x80)."""
    g = Grid(40, 80)
    for (x, w, phase, lean) in ((12, 5, 0.0, -0.12), (17, 7, 1.3, 0.0), (25, 4, 2.1, 0.1)):   # strands flaring out
        cols = set()
        for y in range(4, 72):
            dx = int(round(1.2 * math.sin(y * 0.15 + phase) + lean * (y - 4)))
            cols |= rect(x + dx, y, w + y // 24, 1)
        g.paint(cols, ICE)
    g.fill(line(19, 8, 19, 66), "7")
    gy = 10 + (frame * 23) % 56
    g.fill(rect(18, gy, 3, 3), "d")
    g.paint(poly([(6, 6), (12, 1), (24, 0), (33, 3), (30, 7), (8, 8)]), SNOW)   # the snowy lip it spills from
    g.paint(disc(20, 74, 18, 6) & rect(0, 68, 40, 12), ("4", "5", "6"))
    return g


def crystal_geode(frame):
    """A cracked boulder full of glowing crystals (48x40), light."""
    g = Grid(48, 40)
    g.paint(disc(24, 24, 23, 16) & rect(0, 0, 48, 40), ROCK)
    g.fill(disc(24, 24, 15, 10) & rect(0, 0, 48, 40), "8")
    for (x, h, lean) in ((14, 12, -2), (20, 18, -1), (26, 20, 1), (32, 14, 2), (24, 10, 0)):
        g.paint(poly([(x - 3, 34), (x + 3, 34), (x + lean, 34 - h)]), CRYSTAL)
    g.set(26, 18, "7" if frame else "d")
    g.set(20, 22, "d" if frame else "7")
    return g


def explorer_camp(frame):
    """An abandoned explorer's camp: a tent half buried in snow, a pick, a lantern still lit (64x40), light."""
    g = Grid(64, 40)
    g.paint(poly([(4, 38), (24, 6), (44, 38)]), ("e", "e", "f"))   # the tent
    g.fill(poly([(20, 38), (24, 22), (28, 38)]), "8")
    g.fill(line(24, 6, 24, 2), "2")
    g.paint(disc(22, 40, 22, 6) & rect(0, 0, 64, 40), SNOW)
    g.fill(line(50, 38, 58, 22), "2")   # a pick in the snow
    g.fill(line(54, 22, 62, 24), "3")
    g.paint(rect(46, 28, 6, 7), ("1", "2", "2"))   # the lantern
    g.fill(rect(47, 29, 4, 5), "f" if frame else "e")
    return g


def frozen_tree():
    """A dead tree glazed in ice, its branches beaded with frost (48x64)."""
    g = Grid(48, 64)
    g.paint(capsule(24, 64, 24, 20, 3.2), ("1", "2", "3"))
    for (x1, y1, x2, y2) in ((24, 34, 8, 18), (24, 28, 42, 12), (24, 22, 16, 4), (24, 24, 34, 2), (12, 22, 4, 12)):
        g.fill(capsule(x1, y1, x2, y2, 1.2), "2")
        g.paint(disc(x2, y2, 2.2), ("5", "6", "7"))
    g.fill(line(25, 60, 25, 24), "6")
    g.paint(disc(24, 64, 12, 3) & rect(0, 0, 48, 64), SNOW)
    return g


# --- Enemies and bosses (global palette; side view, facing right; art px with a baked outline, then doubled) ---
ENEMY_FRAMES = ["WALK_1", "WALK_2", "WALK_3", "WALK_4", "TELL", "ATTACK"]


def snowflake(frame):
    """SNOWFLAKE (from FLAKE_A/B): a six-armed snowflake with a small face, turning as it drifts (13x13 art)."""
    g = Grid(13, 13)
    turn = {"WALK_1": 0, "WALK_2": 0.25, "WALK_3": 0.5, "WALK_4": 0.75, "TELL": 0, "ATTACK": 0}[frame]
    reach = 6 if frame != "TELL" else 5
    for k in range(6):
        a = (k + turn) * math.pi / 3
        x1, y1 = 6 + math.cos(a) * reach, 6 + math.sin(a) * reach
        g.fill(line(6, 6, x1, y1), "i")
        g.set(int(round(6 + math.cos(a) * reach * 0.6 + math.cos(a + 1.2) * 1.4)),
              int(round(6 + math.sin(a) * reach * 0.6 + math.sin(a + 1.2) * 1.4)), "w")
    g.paint(disc(6.5, 6.5, 2.8), ("I", "i", "w") if frame != "TELL" else ("c", "w", "w"))
    g.set(7, 6, "k")
    g.set(5, 6, "k")
    if frame == "ATTACK":
        g.fill(rect(5, 8, 3, 1), "k")
    return g


def ice_bat(frame):
    """ICE BAT (from ICEBAT_A/B): a pale blue bat with dark ice wings and cyan eyes (14x11 art); TELL hangs, ATTACK dives."""
    g = Grid(14, 11)
    if frame == "TELL":   # hanging upside down by its feet, wings folded round it, face at the bottom
        g.fill(rect(5, 0, 1, 2) | rect(8, 0, 1, 2), "q")   # feet gripping the ceiling
        g.paint(poly([(3, 2), (11, 2), (10, 8), (7, 10), (4, 8)]), ("U", "q", "q"))   # folded wings
        g.fill(line(7, 2, 7, 7), "U")   # the fold between them
        g.paint(disc(7, 8, 2.2, 1.8), ("I", "i", "i"))   # the face
        g.set(6, 8, "c")
        g.set(8, 8, "c")
        g.fill(rect(5, 10, 1, 1) | rect(9, 10, 1, 1), "i")   # ears pointing down
        return g
    up = {"WALK_1": -3, "WALK_2": 0, "WALK_3": 3, "WALK_4": 0, "ATTACK": -4}[frame]
    for side in (-1, 1):   # wings
        tip = (7 + side * 7, 4 + up)
        g.paint(poly([(7, 4), (7 + side * 2, 7), tip]) | poly([(7 + side * 2, 7), (7 + side * 5, 7), tip]), ("U", "q", "q"))
    g.paint(disc(7, 6, 2.8, 2.6), ("I", "i", "i"))   # body
    g.fill(rect(6, 3, 1, 1) | rect(8, 3, 1, 1), "i")   # ears
    g.set(6, 5, "c")
    g.set(8, 5, "c")
    g.set(7, 8, "w")
    return g


def frost_skull(frame):
    """FROST SKULL (from FROSTSKULL_A/B): a white skull crowned with ice, sitting on a frozen mound; it glows, then spits
    a snowball (13x13 art)."""
    g = Grid(13, 13)
    g.paint(disc(6.5, 12, 6, 3) & rect(0, 0, 13, 13), ("q", "I", "i"))   # the mound
    g.paint(disc(6.5, 6, 4.8, 4.4), ("b", "w", "w"))   # the skull
    g.fill(rect(3, 9, 7, 2), "w")
    for (x, h) in ((3, 3), (6, 4), (9, 3)):   # the ice crown
        g.paint(poly([(x - 1, 2), (x + 1.5, 2), (x + 0.2, 2 - h)]), ("I", "i", "i"))
    eye = "c" if frame == "TELL" else "q"
    g.fill(rect(4, 5, 2, 2) | rect(8, 5, 2, 2), eye)
    if frame == "TELL":
        g.set(4, 5, "w")
        g.set(8, 5, "w")
    g.set(6, 8, "q")
    jaw = 2 if frame == "ATTACK" else 0
    for x in (4, 6, 8):
        g.set(x, 10 + jaw // 2, "q")
    if jaw:
        g.fill(rect(4, 10, 5, 1), "k")
    return g


def yeti_cub(frame):
    """YETI CUB (new, 14x13): a round white cub with a blue face and little horns; TELL lowers its head, ATTACK charges."""
    g = Grid(14, 13)
    bob = 1 if frame in ("WALK_2", "WALK_4") else 0
    lean = {"TELL": 2, "ATTACK": 1}.get(frame, 0)
    y = 6 + bob
    g.paint(disc(6, y + 1, 5.5, 4.6), ("b", "w", "w"))   # body
    g.paint(disc(10 + lean // 2, y - 1 + lean, 3.6, 3.4), ("b", "w", "w"))   # head
    g.paint(disc(11 + lean // 2, y + lean, 2, 1.8), ("q", "I", "I"))   # face
    g.set(12 + lean // 2, y - 1 + lean, "k")
    g.fill(line(9 + lean // 2, y - 4 + lean, 8 + lean // 2, y - 6 + lean), "i")   # horns
    g.fill(line(11 + lean // 2, y - 4 + lean, 12 + lean // 2, y - 6 + lean), "i")
    if frame == "ATTACK":
        g.fill(rect(11, y + 1 + lean, 2, 1), "k")
    step = ENEMY_FRAMES.index(frame) if frame.startswith("WALK") else (1 if frame == "ATTACK" else 0)
    for i, x in enumerate((3, 8)):
        dx = (-1, 0, 1, 0)[(step + i * 2) % 4]
        g.paint(rect(x + dx, y + 4, 2, 13 - (y + 4)), ("b", "w", "w"))
        g.fill(rect(x + dx, 12, 3, 1), "q")
    g.paint(capsule(9, y + 2, 12 + lean, y + 3, 1), ("b", "w", "w"))   # an arm
    return g


def ice_knight(frame):
    """ICE KNIGHT (new, 17x18): a knight in blue ice plate with a pale crest, a round shield in front and a long ice
    sword; TELL raises the sword (wind-up), ATTACK swings it down in front."""
    g = Grid(17, 18)
    bob = 1 if frame in ("WALK_2", "WALK_4") else 0
    y = bob
    g.paint(rect(3, 6 + y, 7, 7), ("U", "u", "I"))   # torso
    g.paint(disc(6.5, 3.5 + y, 3.2, 3.2), ("U", "u", "I"))   # helmet
    g.fill(rect(6, 3 + y, 4, 1), "k")   # visor slit
    g.set(8, 3 + y, "c")
    g.fill(line(4, 0 + y, 2, -2 + y + 2) | rect(5, 0 + y, 3, 1), "i")   # crest
    g.fill(rect(3, 9 + y, 7, 1), "q")   # belt
    step = ENEMY_FRAMES.index(frame) if frame.startswith("WALK") else 0
    for i, x in enumerate((4, 7)):
        dx = (-1, 0, 1, 0)[(step + i * 2) % 4]
        g.paint(rect(x + dx, 13 + y, 2, 5 - y), ("U", "u", "u"))
        g.fill(rect(x + dx, 17, 3, 1), "q")
    # The shield in front.
    g.paint(disc(10.5, 10 + y, 2.6, 3.6), ("q", "I", "i"))
    g.set(10, 9 + y, "w")
    # The sword.
    if frame == "TELL":   # raised over the head
        g.fill(rect(2, 5 + y, 2, 2), "u")
        g.paint(capsule(3, 6 + y, 1, -1 + y + 1, 0.8), ("I", "i", "w"))
        g.fill(line(2, 1, 0, 0), "w")
    elif frame == "ATTACK":   # swung down in front
        g.paint(capsule(11, 7 + y, 16, 16, 0.9), ("I", "i", "w"))
        g.fill(rect(10, 6 + y, 3, 1), "q")
    else:   # held point down at the side
        g.paint(capsule(2, 8 + y, 1, 17, 0.8), ("I", "i", "w"))
        g.fill(rect(0, 8 + y, 4, 1), "q")
    return g


ENEMIES = [("SNOWFLAKE", snowflake), ("ICE_BAT", ice_bat), ("FROST_SKULL", frost_skull), ("YETI_CUB", yeti_cub),
           ("ICE_KNIGHT", ice_knight)]
BOSS_FRAMES = ["IDLE_1", "IDLE_2", "TELL", "ATTACK", "HURT", "PHASE2", "SLIDE"]


def yeti(frame):
    """YETI (from YETI_*, 32x20 → 44x48 art): a huge white yeti with a blue face, curled horns and long arms; TELL rears
    back for the sneeze (AH... AH...), ATTACK sneezes forward, SLIDE lies on its belly, PHASE2 glares with red eyes."""
    if frame == "SLIDE":   # flat on its belly, arms forward
        g = Grid(52, 28)
        g.paint(disc(22, 17, 20, 10), ("b", "w", "w"))
        g.paint(disc(42, 15, 8, 7.5), ("b", "w", "w"))
        g.paint(disc(45, 17, 5, 4), ("q", "I", "I"))
        g.fill(rect(45, 15, 2, 1), "k")
        g.paint(capsule(36, 22, 51, 24, 2.6), ("b", "w", "w"))
        g.fill(arc(40, 8, 4, 180, 330, 1.6), "i")
        for x in range(4, 40, 6):   # snow spray behind
            g.set(x, 27, "i")
        return g
    g = Grid(44, 48)
    p2 = frame == "PHASE2"
    bob = 1 if frame == "IDLE_2" else 0
    rear = -3 if frame == "TELL" else 3 if frame == "ATTACK" else 0
    g.paint(disc(20, 30 + bob, 15, 15), ("b", "w", "w"))   # body
    g.paint(disc(21, 32 + bob, 8, 9), ("s", "b", "b"))     # belly fur shade
    hx, hy = 26 + rear, 12 + bob + (abs(rear) if rear > 0 else 0)
    g.paint(disc(hx, hy, 9.5, 9), ("b", "w", "w"))         # head
    g.paint(disc(hx + 2, hy + 2, 6.5, 6), ("q", "I", "I"))  # face
    eye = "r" if p2 else "k"
    if frame == "HURT":
        g.fill(rect(hx - 1, hy, 3, 1) | rect(hx + 4, hy, 3, 1), "k")
    else:
        g.fill(rect(hx, hy - 1, 2, 2) | rect(hx + 5, hy - 1, 2, 2), eye)
    if frame == "ATTACK":   # the sneeze: mouth wide
        g.fill(rect(hx + 1, hy + 4, 6, 4), "k")
        g.fill(rect(hx + 1, hy + 4, 6, 1), "w")
    elif p2:
        g.fill(rect(hx + 1, hy + 5, 6, 2), "k")
        g.fill(rect(hx + 1, hy + 5, 6, 1), "w")
    else:
        g.fill(rect(hx + 2, hy + 5, 4, 1), "U")
    for side in (-1, 1):   # curled horns
        g.fill(arc(hx + side * 7, hy - 7, 3.5, 90 if side < 0 else -90, 270 if side < 0 else 90, 1.6), "i")
    # Arms.
    if frame == "TELL":
        g.paint(capsule(30, 24, 38, 8, 3), ("b", "w", "w"))
        g.paint(capsule(10, 24, 4, 10, 3), ("b", "w", "w"))
    elif frame == "ATTACK":
        g.paint(capsule(30, 26, 42, 30, 3), ("b", "w", "w"))
        g.paint(capsule(10, 26, 4, 40, 3), ("b", "w", "w"))
    else:
        g.paint(capsule(32, 24, 36, 42, 3.2), ("b", "w", "w"))
        g.paint(capsule(8, 24, 5, 42, 3.2), ("b", "w", "w"))
    g.fill(rect(33, 43, 5, 1) | rect(3, 43, 5, 1), "q")   # claws
    g.paint(capsule(14, 42, 14, 46, 3.4), ("b", "w", "w"))   # legs
    g.paint(capsule(26, 42, 26, 46, 3.4), ("b", "w", "w"))
    g.fill(rect(10, 47, 8, 1) | rect(22, 47, 8, 1), "q")
    return g


def titan(frame):
    """THE GLACIER TITAN (from GOLEM_*, 32x20 → 56x58 art): a hulking golem of blue rock and glacier ice, crystals
    growing from its shoulders, a cyan core in its chest; TELL raises both fists, ATTACK slams them down, PHASE2 cracks
    open with a brighter core."""
    g = Grid(56, 58)
    p2 = frame == "PHASE2"
    bob = 1 if frame == "IDLE_2" else 0
    g.paint(poly([(12, 18 + bob), (44, 18 + bob), (48, 44), (8, 44)]), ("A", "e", "E"))   # torso
    g.paint(disc(28, 12 + bob, 9, 8), ("A", "e", "E"))   # head
    g.fill(rect(23, 11 + bob, 4, 2) | rect(31, 11 + bob, 4, 2), "c" if not p2 else "w")   # eyes
    g.fill(rect(24, 16 + bob, 8, 1), "a")
    core = disc(28, 30 + bob, 5 if not p2 else 6.5)
    g.paint(core, ("I", "c", "w") if not p2 else ("c", "w", "w"))
    if p2 or frame == "HURT":   # cracks in the shell
        g.fill(line(14, 22, 22, 34) | line(40, 20, 34, 38) | line(20, 40, 28, 36), "i")
    for (x, h, lean) in ((10, 12, -3), (15, 9, -1), (42, 12, 3), (46, 8, 4)):   # shoulder crystals
        g.paint(poly([(x - 3, 20 + bob), (x + 3, 20 + bob), (x + lean, 20 + bob - h)]), ("I", "i", "w"))
    # Arms and fists.
    if frame == "TELL":
        g.paint(capsule(10, 22, 4, 4, 4.4), ("A", "e", "E"))
        g.paint(capsule(46, 22, 52, 4, 4.4), ("A", "e", "E"))
        g.paint(disc(4, 4, 5.5), ("q", "I", "i"))
        g.paint(disc(52, 4, 5.5), ("q", "I", "i"))
    elif frame == "ATTACK":
        g.paint(capsule(10, 22, 6, 46, 4.4), ("A", "e", "E"))
        g.paint(capsule(46, 22, 50, 46, 4.4), ("A", "e", "E"))
        g.paint(disc(6, 50, 6, 5), ("q", "I", "i"))
        g.paint(disc(50, 50, 6, 5), ("q", "I", "i"))
    else:
        g.paint(capsule(8, 22, 5, 40, 4.4), ("A", "e", "E"))
        g.paint(capsule(48, 22, 51, 40, 4.4), ("A", "e", "E"))
        g.paint(disc(5, 42, 5.5), ("q", "I", "i"))
        g.paint(disc(51, 42, 5.5), ("q", "I", "i"))
    g.paint(rect(14, 44, 10, 13), ("A", "e", "E"))   # legs
    g.paint(rect(32, 44, 10, 13), ("A", "e", "E"))
    g.fill(rect(12, 57, 14, 1) | rect(30, 57, 14, 1), "a")
    return g


def ice_shard():
    """The Titan's VOLLEY shard (spec §21: a themed deadly projectile with a 1 px magenta outline), pointing right; not
    doubled here (drawn at 2x)."""
    g = Grid(8, 4)
    for y, row in enumerate([".mmmmm..", "mIiiiwwm", "mIiiwwm.", ".mmmmm.."]):
        for x, ch in enumerate(row):
            g.set(x, y, ch)
    return g


def write_frames():
    out = [("ICE_SHARD", ice_shard())]
    for name, fn in ENEMIES:
        for f in ENEMY_FRAMES:
            out.append(("ENEMY_%s_%s" % (name, f), finish(fn(f))))
    for f in BOSS_FRAMES:
        out.append(("BOSS_YETI_" + f, finish(yeti(f))))
    for f in BOSS_FRAMES[:6]:
        out.append(("BOSS_TITAN_" + f, finish(titan(f))))
    return out


def build():
    """Every world 3 palette sprite in source order: (name, Grid)."""
    out = []
    for shape in SHAPES:
        variants = 3 if shape in ("TOP", "FILL") else 1
        for v in range(variants):
            out.append(("FROST_%s_%d" % (shape, v + 1) if variants > 1 else "FROST_" + shape, ground_tile(shape, v)))
    out.append(("FROST_ICE", ice_tile()))
    for s in range(3):
        out.append(("FROST_CRACKED_%d" % (s + 1), cracked_ice(s)))
    for v in range(3):
        out.append(("FROST_FRINGE_%d" % (v + 1), fringe(v)))
    out.append(("FROST_DRAPE_LEFT", drape(False)))
    out.append(("FROST_DRAPE_RIGHT", drape(True)))
    for end in ("left", "mid", "right"):
        out.append(("FROST_SLAB_" + end.upper(), slab(end)))
    out.append(("FROST_BRACKET", bracket()))
    out.append(("FROST_ROPE", chain()))
    out.append(("FROST_PILLAR", pillar()))
    out.append(("FROST_LADDER", ladder(False)))
    out.append(("FROST_LADDER_TOP", ladder(True)))
    for f in range(3):
        out.append(("FROST_WATER_%d" % (f + 1), water(f)))
    out.append(("FROST_SPIKES", ice_spikes()))
    out.append(("FROST_ICE_WALL", ice_wall()))
    out.append(("FROST_SNOW_MOUND", snow_mound(0)))
    out.append(("FROST_SNOW_MOUND_SMALL", snow_mound(1)))
    for f in range(2):
        out.append(("FROST_CRYSTALS_%d" % (f + 1), crystals(f)))
    out.append(("FROST_GRASS", frozen_grass()))
    out.append(("FROST_BONES", bones_in_ice()))
    for f in range(2):
        out.append(("FROST_LANTERN_%d" % (f + 1), frozen_lantern(f)))
    out.append(("FROST_PEBBLES", ice_pebbles()))
    out.append(("FROST_SHRUB", frozen_shrub()))
    out.append(("FROST_ROCK", snow_rock()))
    for f in range(2):
        out.append(("FROST_CEILING_%d" % (f + 1), ceiling_frost(f)))
    out.append(("FROST_WALL_FROST", wall_frost()))
    out.append(("FROST_CATHEDRAL", cathedral_arch()))
    out.append(("FROST_MAMMOTH", frozen_mammoth()))
    for f in range(2):
        out.append(("FROST_FROZEN_FALL_%d" % (f + 1), frozen_waterfall(f)))
    for f in range(2):
        out.append(("FROST_GEODE_%d" % (f + 1), crystal_geode(f)))
    for f in range(2):
        out.append(("FROST_CAMP_%d" % (f + 1), explorer_camp(f)))
    out.append(("FROST_TREE", frozen_tree()))
    return out


def write_all(write, sprite_text, root):
    sprites = build()
    characters = write_frames()
    h = ["// World 3's art (spec §13 row 3, §22) and its enemies and bosses (§8, §12), generated by",
         "// docs/work-plan/name-quest/art-tools/make_quest_art.py (world3.py): edit the script, not this file.",
         "// FROST_* are palette sprites ('0'-'f' index FROST_PALETTE); ENEMY_* and BOSS_* use the global palette.",
         "#pragma once", "#include \"screen.h\"", "",
         "// Blue rock, snow and ice, the cave background, light (crystal cyan, lantern amber); dark → light.",
         "const WorldPalette FROST_PALETTE = {{" + ", ".join("0xFF%06X" % c for c in PALETTE) + "}};", ""]
    h += ["extern const Sprite %s;" % name for name, g in sprites + characters]
    write(os.path.join(root, "source", "quest_sprites_3.h"), "\n".join(h) + "\n")
    cpp = ["// World 3's art, generated by docs/work-plan/name-quest/art-tools/make_quest_art.py (world3.py).",
           "#include \"quest_sprites_3.h\"", ""]
    cpp += [sprite_text(name, g, "extern const Sprite") for name, g in sprites + characters]
    write(os.path.join(root, "source", "quest_sprites_3.cpp"), "\n".join(cpp))


def preview(folder, save_png, global_palette):
    rgb = {("%x" % i): ((c >> 16) & 255, (c >> 8) & 255, c & 255) for i, c in enumerate(PALETTE)}
    gpal = global_palette()
    for (name, sprites, colours, bg) in (("world3.png", build(), rgb, (90, 20, 90)),
                                         ("world3_characters.png", write_frames(), gpal, (40, 44, 70))):
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
