"""Art source for levels 3-5 (glacier, volcano, deep sea): palette and sprites (three-levels-plan.md, step 1).

Run: python3 docs/work-plan/levels/levels_art.py
  - checks every sprite's size,
  - writes docs/work-plan/levels/sprites.png (a magnified sheet) and palette.png,
  - prints the C++ palette cases and sprites for source/sprites.h (between the jungle sprites and the font).
Needs Pillow. Symmetric sprites are written as their left half and mirrored by `sym`.
"""
import os
import re

from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", "..", ".."))

NEW_PALETTE = [
    ("a", 0x0A1322, "glacier: deepest"),
    ("A", 0x132238, "glacier: cave wall"),
    ("e", 0x1E3550, "glacier: crystals"),
    ("E", 0x2E5470, "glacier: crystal edges, icicles, snow"),
    ("i", 0x9FE0FF, "pale ice"),
    ("I", 0x4A9AD8, "mid ice"),
    ("q", 0x2A5C9C, "deep ice"),
    ("v", 0x120706, "volcano: deepest"),
    ("V", 0x241009, "volcano: dark rock"),
    ("K", 0x3A1A10, "volcano: rock edge"),
    ("D", 0x5C1C0C, "volcano: glowing crack"),
    ("C", 0x7A3010, "volcano: lava river, embers"),
    ("O", 0xFF6A10, "hot orange"),
    ("N", 0xFFD84A, "white-hot yellow"),
    ("Z", 0xB02810, "deep red-orange"),
    ("1", 0x051220, "sea: deepest"),
    ("2", 0x0A1E30, "sea: water"),
    ("3", 0x0E3030, "sea: kelp"),
    ("4", 0x1A4A3E, "sea: kelp light"),
    ("5", 0x28506A, "sea: light rays, bubbles"),
    ("6", 0x3FB8A0, "teal fish"),
    ("7", 0x1D6A5E, "dark teal"),
    ("8", 0xE0E8C8, "pale belly, teeth"),
    ("9", 0xE6FF6A, "lure glow"),
]


def sym(*halves):
    return [h + h[::-1] for h in halves]


def put(rows, changes):
    rows = list(rows)
    for r, c, ch in changes:
        rows[r] = rows[r][:c] + ch + rows[r][c + len(ch):]
    return rows


def swap(rows, mapping):
    return ["".join(mapping.get(ch, ch) for ch in row) for row in rows]


S = {}  # name -> (comment or None, rows)

# ---------------------------------------------------------------- glacier
S["FLAKE_A"] = ("Glacier formation: a snowflake creature, an ice bat and a frost skull (pale ice on dark blue).", [
    "i....ii....i",
    ".i...ww...i.",
    "..iwwiiwwi..",
    "...wiiiiw...",
    ".wwiqiiqiww.",
    "iiwiiiiiiwii",
    ".wwiiqqiiww.",
    "...wiiiiw...",
    "..iwwiiwwi..",
    ".i...ww...i."])
S["FLAKE_B"] = (None, [
    ".....ii.....",
    "..i..ww..i..",
    "...iwiiwi...",
    "i..wiiiiw..i",
    ".wwiqiiqiww.",
    "..wiiiiiiw..",
    ".wwiiqqiiww.",
    "i..wiiiiw..i",
    "...iwiiwi...",
    "..i..ww..i.."])
ICEBAT_A = [
    "I..........I",
    "II........II",
    "IqI.i..i.IqI",
    "IqqIiiiiIqqI",
    "IqqiciicIqqI",
    ".IqiiwwiiqI.",
    "..IqiiiiqI..",
    "...I.ii.I...",
    ".....ii.....",
    "............"]
ICEBAT_B = [
    "............",
    "............",
    "....i..i....",
    "...IiiiiI...",
    ".IIiciiciII.",
    "IqqIiwwiIqqI",
    "IqqqIiiIqqqI",
    "Iqq.I..I.qqI",
    "Iq........qI",
    "I..........I"]
S["ICEBAT_A"] = (None, ICEBAT_A)
S["ICEBAT_B"] = (None, ICEBAT_B)
FROSTSKULL_A = [
    "i..........i",
    "Ii..wwww..iI",
    ".IiwwwwwwiI.",
    "..wwwwwwww..",
    ".wwqqwwqqww.",
    ".wwqcwwcqww.",
    ".wwwwqqwwww.",
    "..wwwwwwww..",
    "..wqwqqwqw..",
    "...bbbbbb..."]
FROSTSKULL_B = [
    ".i........i.",
    "iI..wwww..Ii",
    ".IiwwwwwwiI.",
    "..wwwwwwww..",
    ".wwqqwwqqww.",
    ".wwcqwwqcww.",
    ".wwwwqqwwww.",
    "..wwwwwwww..",
    "...q.qq.q...",
    "..wqwqqwqw.."]
S["FROSTSKULL_A"] = (None, FROSTSKULL_A)
S["FROSTSKULL_B"] = (None, FROSTSKULL_B)
S["YETICLOUD_A"] = ("Glacier heavy: a little yeti riding a cloud.", [
    "......wwww......",
    ".....wwwwww.....",
    "....wwIIIIww....",
    "....wIxIIxIw....",
    "....wIIIIIIw....",
    "..w.wwIqqIww.w..",
    "..wwwwwwwwwwww..",
    "...bwwwwwwwwb...",
    ".bbsbbwwwwbbsbb.",
    "bbbbbbbbbbbbbbbb",
    ".bsbbsbbbbsbbsb.",
    "..ss..ssss..ss.."])
S["YETICLOUD_B"] = (None, [
    "......wwww......",
    ".....wwwwww.....",
    "....wwIIIIww....",
    "....wIxIIxIw....",
    "w...wIIIIIIw...w",
    ".w.wwwIqqIwww.w.",
    "..wwwwwwwwwwww..",
    "...bwwwwwwwwb...",
    ".bbsbbwwwwbbsbb.",
    "bbbbbbbbbbbbbbbb",
    ".bsbbsbbbbsbbsb.",
    ".ss..ssss..ss..."])
YETI_A = sym(
    "............bwww",
    "..........bwwwww",
    ".........bwwwwww",
    ".........wwIIIII",
    ".........wIqqIII",
    ".........wIIxIII",
    ".........wIIIIII",
    ".........wwIqwqw",
    "..........wIqqqq",
    "......bwwwwwIIII",
    ".....bwwwwwwwwww",
    "....bwwbwwwwwwww",
    "...bwwb.bwwwwwww",
    "..IIIb..bwwwwwbw",
    "..III...bwwwwbww",
    ".........wwwwwww",
    ".........bwwwwwb",
    "..........wwwwb.",
    ".........IIIII..",
    "................")
S["YETI_A"] = ("Yeti: white fur, blue face, hands and feet.", YETI_A)
YETI_B = list(YETI_A)
for r, half in [(6, "...II....wIIIIII"), (7, "..III....wwIqwqw"), (8, "...bw.....wIqqqq"), (9, "....bwwwwwwwIIII"),
                (10, ".....bwwwwwwwwww"), (11, "........bwwwwwww"), (12, "........bwwwwwww"),
                (13, "........bwwwwwbw"), (14, "........bwwwwbww")]:
    YETI_B[r] = half + half[::-1]
S["YETI_B"] = (None, YETI_B)
YETI_SNEEZE = list(YETI_A)
for r, half in [(4, ".........wIIIIII"), (5, ".........wIqqIII"), (6, ".........wIIIIII"), (7, ".........wIIqqqq"),
                (8, "..........wIqrrr")]:
    YETI_SNEEZE[r] = half + half[::-1]
S["YETI_SNEEZE"] = ("Sneeze: eyes squeezed shut, mouth wide open.", YETI_SNEEZE)
GOLEM_A = sym(
    "..........qqqqqq",
    ".........qIIIIII",
    ".........qIiiiii",
    ".........qIiccii",
    ".........qIiiiii",
    ".........qIIqqqq",
    "....qqqq.qqIIIII",
    "...qIIIIqqIiiiiI",
    "...qIiiIqIiiwiiI",
    "...qIiiIqIiiiiiI",
    "...qIIIIqIiiiiiI",
    "....qIIqqIIIIIII",
    "....qiiq.qIiiiiI",
    "....qiiq.qIiiiiI",
    "....qIIq.qIIIIII",
    "........qIiiiq..",
    "........qIiiiq..",
    "........qIIIIq..",
    ".......qqqqqqq..",
    "................")
S["GOLEM_A"] = ("Ice golem: blocks of ice with glowing cyan eyes.", GOLEM_A)
GOLEM_B = list(GOLEM_A)
for r, half in [(4, "....qqqq.qIiiiii"), (5, "...qIIIIqqIIqqqq"), (6, "...qIiiIqqqIIIII"), (7, "...qIiiIqqIiiiiI"),
                (8, "...qIIIIqIiiwiiI"), (9, "....qIIqqIiiiiiI"), (10, "....qiiq.qIiiiiI"),
                (11, "....qiiq.qIIIIII"), (12, "....qIIq.qIiiiiI"), (13, ".........qIiiiiI"),
                (14, ".........qIIIIII")]:
    GOLEM_B[r] = half + half[::-1]
S["GOLEM_B"] = (None, GOLEM_B)

# ---------------------------------------------------------------- volcano
S["EMBER_A"] = ("Volcano formation: a living ember, a fire bat and a magma skull.", [
    ".....O......",
    "....OO...O..",
    "...OOO..OO..",
    "..OONOOOOO..",
    ".OONNNNNOOO.",
    ".ONxNNNxNOO.",
    "ZONNNNNNNNOZ",
    "ZOONNxxNNOOZ",
    ".ZOONNNNOOZ.",
    "..ZZOOOOZZ.."])
S["EMBER_B"] = (None, [
    "......O.....",
    "..O..OO.....",
    "..OO.OOO....",
    "..OOOOONOO..",
    ".OONNNNNOOO.",
    ".ONxNNNxNOO.",
    "ZONNNNNNNNOZ",
    "ZOONNxxNNOOZ",
    ".ZOONNNNOOZ.",
    "..ZZOOOOZZ.."])
FIRE = {"I": "Z", "q": "O", "i": "x", "c": "N"}
S["FIREBAT_A"] = (None, swap(ICEBAT_A, FIRE))
S["FIREBAT_B"] = (None, swap(ICEBAT_B, FIRE))
MAGMA = {"w": "Z", "q": "x", "c": "N", "i": "O", "I": "O", "b": "R"}
S["MAGMASKULL_A"] = (None, swap(FROSTSKULL_A, MAGMA))
S["MAGMASKULL_B"] = (None, swap(FROSTSKULL_B, MAGMA))
S["PHOENIX_A"] = ("Volcano heavy: a phoenix.", sym(
    "O.......",
    "OO......",
    "ZOO.....",
    ".ZOO..NN",
    "..ZOONxN",
    "...ZONNN",
    "....ZNNN",
    ".....ZNY",
    "....ZOON",
    "...ZO.ON",
    "..ZO..ZO",
    ".O.....Z"))
S["PHOENIX_B"] = (None, sym(
    "........",
    "........",
    "........",
    "......NN",
    ".....NxN",
    "OOZZZONN",
    "ZOOOZNNN",
    ".ZOOZZNY",
    "..ZZ.OON",
    "....ZO.N",
    "...ZO.ZO",
    "..O....Z"))
IMP_A = sym(
    "..............O.",
    "...........O.OO.",
    "..........OONOOO",
    "..........ONNNON",
    "....N.....ZZZZZZ",
    ".....N...ZZZZZZZ",
    ".....NZ..ZNxZZZZ",
    "......ZZ.ZNNZZZZ",
    "..RR.....ZZxwxwx",
    ".RRRR.....ZZxxxx",
    "RROORR...ZZZZZZZ",
    "ROOOORRZZZZZOOOO",
    ".RROORR.ZZZOONNN",
    "..RRRR..ZZZOONNN",
    "...RR...ZZZZOOOO",
    ".........ZZZZZZZ",
    ".........ZZZ....",
    "........ZZZ.....",
    ".......RRR......",
    "................")
S["IMP_A"] = ("Fire imp: flaming hair, little horns, bat wings and a grin.", IMP_A)
IMP_B = list(IMP_A)
for r, half in [(0, "............O..."), (1, "..........O.OO.O"), (5, "RR...N...ZZZZZZZ"), (6, "ROR..NZ..ZNxZZZZ"),
                (7, "ROOR..ZZ.ZNNZZZZ"), (8, ".ROOR....ZZxwxwx"), (9, "..RRR.....ZZxxxx"), (10, "....R....ZZZZZZZ"),
                (11, ".....RRZZZZZOOOO"), (12, "........ZZZOONNN"), (13, "........ZZZOONNN"),
                (14, "........ZZZZOOOO"), (16, "..........ZZZ..."), (17, "..........ZZZ..."),
                (18, ".........RRR....")]:
    IMP_B[r] = half + half[::-1]
S["IMP_B"] = (None, IMP_B)
FIRELORD_A = sym(
    "...H......O.....",
    "...HH....OO..O..",
    "....HH..OONO.OO.",
    ".....HHOONNOOONO",
    "......HZZZZZZZZZ",
    ".....OZZRRZZZZZZ",
    "....OOZRNNNZZZZZ",
    "....OZZZRRZZZZZZ",
    "...OOZZZZZxxxxxx",
    "...OZZZZZZxNxNxN",
    "..OOZZZZZZZxxxxx",
    ".ZZZZRRZZZZZZZZZ",
    "ZZRRZZRRZZZOZZZZ",
    "ZRR..ZRRZZZOOZZZ",
    "OO...ZRRZZZONOZZ",
    "ONO...ZRZZZONNOZ",
    "OO.....ZRZZZOOZZ",
    ".......ZZRZZZZZZ",
    "........ZZRRZZZZ",
    ".........ZZZZZZZ")
S["FIRELORD_A"] = ("Fire lord: horned head in a mane of flame, molten core, magma fists.", FIRELORD_A)
FIRELORD_B = list(FIRELORD_A)
for r, half in [(0, "...H.....O......"), (1, "...HH...OO...O.."), (2, "....HH.OONO..OO."), (3, ".....HHONNNOONOO"),
                (14, "NO...ZRRZZZONOZZ"), (15, "OOO...ZRZZZONNOZ"), (16, ".O.....ZRZZZOOZZ")]:
    FIRELORD_B[r] = half + half[::-1]
S["FIRELORD_B"] = (None, FIRELORD_B)

# ---------------------------------------------------------------- deep sea
S["PIRANHA_A"] = ("Deep-sea formation: a piranha, a pufferfish and a lionfish.", [
    ".....XXX....",
    "...XXXXXX..X",
    "..XbbbbbbXXX",
    ".XbxYbbbbbXX",
    "X8bbbbbbbbX.",
    ".8.8bbbbbbX.",
    "X8rrrrrbbbXX",
    ".XrrrrrbbXXX",
    "...XXXXXX..X",
    ".....XX....."])
S["PIRANHA_B"] = (None, [
    ".....XXX....",
    "...XXXXXX...",
    "..XbbbbbbX.X",
    ".XbxYbbbbbXX",
    "X8bbbbbbbbXX",
    "X8888bbbbbX.",
    "XrrrrrrbbbX.",
    ".XrrrrrbbXXX",
    "...XXXXXX.XX",
    ".....XX....X"])
S["PUFFER_A"] = (None, sym(
    "...h..",
    ".h.YYY",
    "..YYYY",
    "hYxYYY",
    ".YYYYY",
    "hYYYYY",
    ".YoYoY",
    "hYYYYr",
    "..YYYY",
    "...h.."))
S["PUFFER_B"] = (None, sym(
    "......",
    "h..YYY",
    ".YYYYY",
    ".YxYYY",
    "hYYYYY",
    ".YYYYY",
    "hYoYoY",
    ".YYYYr",
    "h.YYYY",
    "......"))
S["LIONFISH_A"] = (None, [
    ".R..R..R..R.",
    "..R.R.R..R..",
    "...rwrwrwr..",
    ".rwYxwrwrwrR",
    "rwrwrwrwrwRR",
    ".rwrwrwrwr.R",
    "..R.R..R....",
    ".R..R...R...",
    "R...R....R..",
    "............"])
S["LIONFISH_B"] = (None, [
    "R...R..R...R",
    ".R..R.R..R..",
    "...rwrwrwr..",
    ".rwYxwrwrwr.",
    "rwrwrwrwrwRR",
    ".rwrwrwrwrRR",
    "..R..R..R...",
    "..R..R...R..",
    ".R...R...R..",
    "............"])
S["SHARK_A"] = ("Deep-sea heavy: a shark.", [
    "........s.......",
    ".......ss.......",
    "......sss.......",
    "...sssssssss...s",
    ".ssbbbbbbbbbs.ss",
    "sbbxbbbbbbbbbsss",
    "sbbbbbbbbbbbbbs.",
    "s8888wwwwwwbbsss",
    ".swwwwwwwwwbbs.s",
    "..sswwwwwbss...s",
    "....sss.ss......",
    "........s......."])
S["SHARK_B"] = (None, [
    "........s.......",
    ".......ss.......",
    "......sss.......",
    "...sssssssss....",
    ".ssbbbbbbbbbs..s",
    "sbbxbbbbbbbbbsss",
    "sbbbbbbbbbbbbbss",
    "s8.8.wwwwwwbbss.",
    "s8888wwwwwwbbs..",
    "..sswwwwwbss..s.",
    "....sss.ss.....s",
    "........s......."])
ANGLER_A = sym(
    "................",
    "...........77777",
    "........77766666",
    "......7766666666",
    ".....76666666666",
    "....766wr6666666",
    "...7666ww6666666",
    "..76666666666666",
    ".766666666666666",
    "7666xxxxxxxxxxxx",
    "766x8x8x8x8x8x8x",
    "76xxx8xxx8xxx8xx",
    "76xxxxxxxxxrrrrr",
    "76xx8xxx8xxx8xxx",
    "766x8x8x8x8x8x8x",
    ".766666666666666",
    "..77666666666666",
    "....777666666666",
    ".......777777777",
    "................")
ANGLER_A = swap(ANGLER_A, {"6": "7", "7": "6"})   # dark body, lighter rim: scarier
S["ANGLER_A"] = ("Anglerfish: a gaping mouth of fangs, red eyes and a glowing lure that sways.",
                 put(ANGLER_A, [(0, 21, "699"), (1, 22, "99")]))
S["ANGLER_B"] = (None, put(put(ANGLER_A, [(11, 0, "67xxxxxxxxxxxxxxxxxxxxxxxxxxxx76"), (13, 0, "67x8x8x8x8x8x8x88x8x8x8x8x8x8x76")]),
                           [(0, 8, "996"), (1, 8, "99")]))
FISHERMAN_A = [
    "........YYYYYYYY................",
    "......YYYYYYYYYYYY............h.",
    ".....YYYYYYYYYYYYYY..........h..",
    "....ooooooooooooooooo.......h...",
    "...ooooooooooooooooooo.....h....",
    ".....766666666666667......h.....",
    "....7666wwx666wwx66667...h......",
    "....7666wxx666wxx66667..h.......",
    "....76666666666666666788........",
    "....7666666rrrr6666667h.........",
    "....766666r8888r66666h7.........",
    "..7.766666rrrrrr666h6667........",
    ".767.7666666666666h666667.......",
    ".7667.766666666668h7.777........",
    "..77...77666666h888.............",
    ".........7777h777...............",
    "..........77....77..............",
    ".........767....767.............",
    "........7667....7667............",
    "........777......777............"]
S["FISHERMAN_A"] = ("Fisherman fish: a big yellow fisherman's hat and a fishing rod (tip at 30, 1).", FISHERMAN_A)
S["FISHERMAN_B"] = (None, put(FISHERMAN_A, [(9, 11, "rrrr"), (10, 10, "r88rr"), (11, 10, "rrrrrr"),
                                              (17, 9, "76"), (17, 17, "67"), (18, 8, ".766"), (18, 16, "667.")]))

# ---------------------------------------------------------------- hazards
S["SNOWBALL"] = ("Glacier, volcano and deep-sea hazards: magenta marks the deadly ones.", [
    ".mmmm.",
    "mwwwwm",
    "mwwwwm",
    "mwwwbm",
    "mwbbbm",
    ".mmmm."])
S["ICICLE"] = (None, [
    "IiiI",
    "wiiI",
    "wiiI",
    ".iI.",
    ".iI.",
    ".iI.",
    ".mm.",
    ".mm.",
    "..m."])
S["FIREBALL_A"] = (None, [
    "..m...",
    ".mOm..",
    ".mOOm.",
    "mONNOm",
    "mONNOm",
    ".mmmm."])
S["FIREBALL_B"] = (None, [
    "...m..",
    "..mOm.",
    ".mOOm.",
    "mONNOm",
    "mONNOm",
    ".mmmm."])
S["BUBBLE"] = (None, [
    ".mmm.",
    "mw..m",
    "m...m",
    "m...m",
    ".mmm."])
S["BIG_BUBBLE"] = (None, [
    "..mmmmm..",
    ".m.....m.",
    "mww.....m",
    "mw......m",
    "m.......m",
    "m.......m",
    "m......cm",
    ".m....cm.",
    "..mmmmm.."])
S["BOOT"] = (None, [
    "..hhh..",
    "..hmh..",
    "..hmh..",
    "..hhhh.",
    "hhhhhhh",
    "xxxxxxx"])
S["ICE_BLOCK"] = ("Harmful but not deadly: no magenta.", [
    "qqqqqqqq",
    "qiiwiiIq",
    "qiwiiiIq",
    "qwiiiiIq",
    "qiiiiiIq",
    "qiiiiIIq",
    "qIIIIIIq",
    "qqqqqqqq"])
S["ICE_CUBE"] = (None, [
    "qIIIIIIIIIIIIIIIIq",
    "Iiw.............iI",
    "Iw...............I",
    "I...........w....I",
    "I..........w.....I",
    "I.....w..........I",
    "I....w...........I",
    "I................I",
    "I...........w....I",
    "I..........w.....I",
    "I................I",
    "I...w............I",
    "I..w.............I",
    "I...............wI",
    "Ii.............wiI",
    "qIIIIIIIIIIIIIIIIq"])
S["LAVA_DROP"] = (None, [
    "..O..",
    "..O..",
    ".OOO.",
    "ONNOO",
    "OZOOO",
    ".ZZZ."])
S["HOOK"] = (None, [
    "..bb.",
    "...b.",
    "...b.",
    "b..b.",
    "bb.b.",
    ".bb.."])


def main():
    with open(os.path.join(ROOT, "source", "sprites.h")) as f:
        source = f.read()
    palette = {c: int(v, 16) & 0xFFFFFF for c, v in re.findall(r"case '(.)': return 0x([0-9A-Fa-f]{8});", source)}
    for c, v, _ in NEW_PALETTE:
        assert c not in palette or palette[c] == v, "palette clash: " + c
        palette[c] = v
    rgb = {c: (v >> 16, v >> 8 & 255, v & 255) for c, v in palette.items()}
    ok = True
    for name, (_, rows) in S.items():
        w = len(rows[0])
        for r in rows:
            if len(r) != w:
                print("BAD WIDTH", name, len(r), w, r)
                ok = False
            for ch in r:
                if ch != "." and ch not in palette:
                    print("UNKNOWN COLOUR", name, ch)
                    ok = False
    if not ok:
        raise SystemExit(1)

    zoom, pad, x, y, rowh = 4, 6, 6, 6, 0
    sheet = Image.new("RGB", (900, 560), (12, 15, 30))
    draw = ImageDraw.Draw(sheet)
    for name, (_, rows) in S.items():
        w, h = len(rows[0]), len(rows)
        if x + w * zoom > 900:
            x, y, rowh = 6, y + rowh + pad + 10, 0
        for ry, row in enumerate(rows):
            for rx, ch in enumerate(row):
                if ch != ".":
                    draw.rectangle([x + rx * zoom, y + ry * zoom, x + rx * zoom + zoom - 1, y + ry * zoom + zoom - 1],
                                   fill=rgb[ch])
        draw.text((x, y + h * zoom + 1), name[:12], fill=(110, 110, 140))
        x += w * zoom + pad + 30
        rowh = max(rowh, h * zoom)
    sheet.save(os.path.join(HERE, "sprites.png"))

    pal = Image.new("RGB", (len(NEW_PALETTE) * 24, 24))
    pd = ImageDraw.Draw(pal)
    for i, (c, v, _) in enumerate(NEW_PALETTE):
        pd.rectangle([i * 24, 0, i * 24 + 23, 23], fill=(v >> 16, v >> 8 & 255, v & 255))
    pal.save(os.path.join(HERE, "palette.png"))

    for c, v, comment in NEW_PALETTE:
        print("        case '%s': return 0xFF%06X;   // %s" % (c, v, comment))
    print()
    for name, (comment, rows) in S.items():
        if comment:
            print("// " + comment)
        print("const Sprite %s = {%d, %d," % (name, len(rows[0]), len(rows)))
        print("\n".join('    "%s"' % r for r in rows[:-1]))
        print('    "%s"};' % rows[-1])


if __name__ == "__main__":
    main()
