"""The four NAME QUEST heroes (spec §21, §22, phase 5b step 2), on the build of the user's reference archer (approved
2026-11-22): a 32x28 art-px canvas doubled to 64x56 real px, facing right, no outline. Head rows 0-14 (face rows 10-13),
body rows 15-21, legs rows 22-27 (feet on row 27); the 8x14 dp hitbox covers art columns 12-19, rows 14-27, and the
canvas is symmetric around it. Per class: 15 body frames, nine poses per R1 strike over six leg poses. Trim 'h' and
tier details '1'-'3' are recoloured by job tier when drawn. Writes source/quest_sprites_hero.h/.cpp; called by
make_quest_art.py. Standard library only."""
import math
import os

W, H = 32, 28
SHIFT = 4   # templates are written in the reference's columns; the canvas puts the body's centre on column 15.5
CLASS_NAMES = ["KNIGHT", "ARCHER", "MAGICIAN", "THIEF"]
BODY_FRAMES = ["IDLE_1", "IDLE_2", "RUN_1", "RUN_2", "RUN_3", "RUN_4", "JUMP", "PEAK", "FALL", "LAND", "CLIMB_1", "CLIMB_2",
               "ROLL_1", "ROLL_2", "HURT"]
SKILL_POSES = ["RECOVER", "SKILL_1", "SKILL_2", "SKILL_3", "SKILL_4", "SKILL_5"]
STRIKES = {f"STRIKE_{strike+1}_{frame+1}": (strike, frame)
           for strike in range(3) for frame in range(9)}
DRAWS = {f"DRAW_{strike+1}_{frame+1}": (strike, frame)
         for strike in range(3) for frame in range(3)}
ARM_POSES = SKILL_POSES + list(STRIKES) + list(DRAWS)
LEG_POSES = ["STAND", "RUN_1", "RUN_2", "RUN_3", "RUN_4", "AIR"]
TIER_CHARS = "123"


def T(*rows, top=0):
    """A template: rows written in the reference's columns, starting at canvas row `top` ('.' empty)."""
    return (top, list(rows))


# --- Legs (rows 22-27) for the Knight, Archer and Thief. Keys: T back thigh, U front thigh, B back boot, F front boot,
# G and E the back and front boot's sole rows. RUN_1/RUN_3 are contacts (legs apart), RUN_2/RUN_4 passes (one foot up). ---
LEGS = {
    "STAND": T("..........TT.UU", ".........BB..FF", "........BB...FF", "........BB...FF", "........GG..EEE",
               "........GG..EEE", top=22),
    "RUN_1": T("..........TTUU", ".........BB..FF", "........BB....FF", ".......BB.....FF", "......GG.....EEE",
               ".....GG......EEE", top=22),
    "RUN_2": T("..........TTUU", ".........BB.FF", "........BBB.FF", ".......GG...FF", "............EE",
               "...........EEE", top=22),
    "RUN_3": T("..........UUTT", ".........FF..BB", "........FF....BB", ".......FF.....BB", "......EE.....GGG",
               ".....EE......GGG", top=22),
    "RUN_4": T("..........UUTT", ".........FF.BB", "........FFF.BB", ".......EE...BB", "............GG",
               "...........GGG", top=22),
}
LEGS["AIR"] = T("..........TT.UU", ".........BB...FF", ".........BB...FF", "........BB....FF", "........GG...EEE", top=22)
LEGS["TUCK"] = T(".........TTUUUU", "........BB...FF", "........BB...FF", ".......GG...EEE", top=22)
LEGS["DANGLE"] = T("..........TT.UU", "..........BB.FF", "..........BB..FF", "..........BB..FF", ".........GG...EE",
                   ".........G.....E", top=22)
LEGS["SQUAT"] = T(".........TT..UU", "........BB....FF", ".......BB.....FF", ".......GG....EEE", ".......GG....EEE", top=23)
# The Magician's robe from the waist down (rows 18-27): the hem sways back while running.
ROBE_TOP = [".........^^^^^^```", "........^^^^^^````", "........^^^^^^````"]
ROBE = {
    "STAND": T(*ROBE_TOP, ".......^^^^^^^`````", ".......^^^^^^^`````", "......^^^^^^^^``````", "......^^^^^^^```````",
               "......hhhhhhhhhhhhhh", ".........&&...&&", ".........&&...&&", top=18),
    "RUN_1": T(*ROBE_TOP, "......^^^^^^^^````", "......^^^^^^^^`````", ".....^^^^^^^^^`````", ".....^^^^^^^^``````",
               ".....hhhhhhhhhhhhhh", "........&&.....&&", ".......&&......&&", top=18),
    "RUN_2": T(*ROBE_TOP, ".......^^^^^^^````", ".......^^^^^^^`````", "......^^^^^^^^`````", "......^^^^^^^``````",
               "......hhhhhhhhhhhhh", "..........&&&&", "...........&&", top=18),
}
ROBE["AIR"] = T(*ROBE_TOP, ".......^^^^^^^`````", "......^^^^^^^^`````", "......^^^^^^^^``````", ".....^^^^^^^^^```````",
               ".....hhhhhhhhhhhhhhh", ".........&&....&&", top=18)
ROBE["TUCK"] = T(*ROBE_TOP, "......^^^^^^^^````", ".....^^^^^^^^^`````", ".....^^^^^^^^``````", ".....hhhhhhhhhhhhhh",
                 ".........&&.&&", top=18)
ROBE["DANGLE"] = T(*ROBE_TOP, ".......^^^^^^^`````", ".......^^^^^^^`````", "......^^^^^^^^``````", "......^^^^^^^```````",
                   "......hhhhhhhhhhhhhh", "..........&&..&&", "..........&...&", top=18)
ROBE["SQUAT"] = T(*ROBE_TOP, "......^^^^^^^^`````", "......^^^^^^^^``````", ".....^^^^^^^^^```````", ".....^^^^^^^^````````",
                  ".....hhhhhhhhhhhhhhhh", ".........&&...&&", "........&&&..&&&", top=18)
ROBE["RUN_3"], ROBE["RUN_4"] = ROBE["RUN_1"], ROBE["RUN_2"]

# --- Per class: the head (hair or headgear, face, neck), the idle upper body (torso, both arms and the weapon), the back
# layer drawn only into empty cells ("rest", and "flutter" lagging behind while running), the leg colours, tier details. ---
CLASSES = {}

CLASSES["KNIGHT"] = dict(
    head=T("..........111", "...........1", "..........hhh", ".........bbbbb", "........bbbbbbs", ".......bbbbbbbss",
           "......bbbbbbbbsss", "......bbbbbbbbsss", "......bsssssssssd", "......bsdsssssssd", "......bsds@@@@@sd",
           "......bsds]]@@]sd", "......bsdsw]@@]sd", "......bsds+@@@+sd", "........d.sssss.d"),
    torso=T(".......bbbbsbbsbbb", "....hhhsbbbbbbbsss", "...hsssh&bbbbbs]]", "...hsdsh&bbbbbs]]@", "...hsdsh&hhhhhh]@h",
            "....hhh.@ssssss.ddd", "........sssssdd..b", ".................bw", ".................bw", ".................bw",
            ".................bw", ".................b", top=15),
    back={"rest": T("......r", ".....rr", "....rrR", "....rRR", "....rRR", "....rRR", "....rRRR", ".....RRR", ".....RR",
                    top=14),
          "flutter": T("......r", ".....rr", "....rrR", "...rrRR", "...rRR", "..rRR", "..RR", top=14),
          "up": T("....rr", "...rrR", "...rRR", "....rRR", ".....rr", "......r", top=10)},
    legs=dict(T="s", U="d", B="s", F="d", G="&", E="&"),
    tier=T(".................22", "", "", "..3", "..3", "..3", top=14),   # 1 plume (in the head), 2 pauldron rim, 3 shield crest
)

CLASSES["ARCHER"] = dict(
    head=T("", "........1", ".......1", "........///", ".......&///&", ".......&/;;;;", ".......&,//////",
           ".......&&///////", ".......&&&//////&", ".......&&&&&&&&&&", ".......&.&@@@@@&&", ".......&.&]]@@]&&",
           ".......&.&w]@@]@&", ".......&..+@@@+&", "...........&;;"),
    torso=T(".........@/&;;&", "........//&&&&&", "...../::/&&&&&]]:::::/", "....../:/&&&&&&]].....:/",
            ".......//&;;;;;]]...//", ".......@@;;;;;;]///", ".........;;;,,,", top=15),
    back={"rest": T("......]&", "......]&", "......]&", "......]&", "......]&", "......]&", "......]&", "......]&",
                    "......]&", "......]&", top=6),
          "flutter": T("......]&", "......]&", "......]&", "......]&", "......]&", "......]&", ".....]&", ".....]&",
                       "....]&", "....]&", top=6),
          "up": T(".....]&", ".....]&", "......]&", "......]&", "......]&", "......]&", "......]&", "......]&", top=1)},
    legs=dict(T="@", U="$", B="/", F="&", G="/", E="&"),
    tier=T("..............2", ".............2", "", "", "", "", "", "", "", "................3", top=4),   # 1 feather, 2 band tails, 3 earring
)

CLASSES["MAGICIAN"] = dict(
    head=T("....|^", ".....|^^", "......|^^^", ".......`^^^", ".......``^^^^", "......``hhhh^^", "...^^^^^^^^^^^^^^^^",
           "....`````````````", "......>>>>>>>>>>:", "......>>>>>>>>>>>:", "......>>:>@@@@@>>:", "......>>:>,,@@,>:",
           "......>>:>w,@@,@>:", "......>>:.+@@@+>:", "......>>:..^^^.>:"),
    torso=T("", "", "", "", "", "", "", "", "", ".....................w", ".....................c2", "....................&",
            "....................&", "....................&", "....................&", "......>>:@^^^^^^:...&",
            "......>:.@^^^^^^``..&", ".......:.@@^^^^^``@@&", "....................&", "....................&",
            "....................&", "....................&", "....................&", "....................&",
            "....................&", "....................&", "....................&", "....................&"),
    back={"rest": T("", top=0), "flutter": T("......>", "......>", "......:", top=15), "up": T("", top=0)},
    tier=T("...1", "..1.1", "...1", "", "", "", "", "", "", "", ".........33...3", top=4),   # 1 hat star, 2 gem (torso), 3 collar
)

CLASSES["THIEF"] = dict(
    head=T("", "", "", "........&.&.&", ".......&&&&&&&&", "......&&/&&&&&&&", "......&//&&&&&&&&", ".....&&/&&&&&&&&&",
           ".....&&&&&&&&&&&&&", "......&&&&&&&&&&&", "......&&.&@@@@@&&", "......&&.&]]@@]&&", "......&&.&w]@@]@&",
           "......&&..<<<<<<&", ".......pp.XXXXX"),
    torso=T(".......pppppppp", ".......-ppppp-", "........<<<<<<X]]2", ".......@<<<<<X]]@b", ".......@XXXXXX]@.bw",
            ".......h<hhhhhh", "........XXXXXXX", top=15),
    back={"rest": T("..pppp-", ".p---", top=15), "flutter": T(".ppppp-", "p---", "", top=14), "flat": T("ppppppp-", top=15),
          "up": T("..pp", ".pp-", "..pp-", "....pp", top=11)},
    legs=dict(T="X", U="X", B="<", F="X", G="x", E="x"),
    tier=T("........33", "", "", "", "", "...1", "..1", top=13),   # 1 third scarf tail, 2 pommel (torso), 3 mask knot
)

# Upper-body templates shared by the R2 skill and recovery poses.
ARMS = {
    "KNIGHT": {
        "RECOVER": T(".......bbbbsbbsbbb", "....hhhsbbbbbbbsss", "...hsssh&bbbbbs]]", "...hsdsh&bbbbbs.]]",
                     "...hsdsh&hhhhhhs.@h", "....hhh.@ssssss...db", "........sssssdd.....bw", ".....................bw",
                     "......................bw", ".......................b", top=15),
    },
    "ARCHER": {   # the bow held up in front: drawn to the cheek, released (string straight, hand flung back), lowered
        "WINDUP": T("..................../", "...................:./", "..................:../", ".................:..../",
                    ".........@/&;;&.@//////bw", "........//&&&&&]@@@@@@&", "........//&&&&&..:..../",
                    ".........&&&&&&...:../", ".........&;;;;;....:./", ".........;;;;;;...../", ".........;;;,,,", top=11),
        "ACTIVE": T("..................../", "....................:/", "....................:/", "....................:/",
                    ".......@@@/&;;&.....:&", "........//&&&&&]@@@@@&", "........//&&&&&.....:/",
                    ".........&&&&&&.....:/", ".........&;;;;;.....:/", ".........;;;;;;...../", ".........;;;,,,", top=11),
        "RECOVER": T("..................../", "...................:/", ".........@/&;;&....:/", "........//&&&&&@...:/",
                     "........//&&&&&]@@@@&", ".........&&&&&&....:&", ".........&;;;;;....:/", ".........;;;;;;....:/",
                     ".........;;;,,,...../", top=13),
    },
    "MAGICIAN": {
        "RECOVER": T("..................w", "..................c2", "..................&", "..................&",
                     "..................&", "..................&", "...................&", "...................&",
                     "......>>:@^^^^^^:..&", "......>:.@^^^^^^``.&", ".......:.@@^^^^^``@&", "....................&",
                     "....................&", "....................&", "....................&", "....................&",
                     ".....................&", ".....................&", ".....................&", ".....................&",
                     ".....................&", top=7),
    },
    "THIEF": {
        "ACTIVE": T(".......pppppppp", ".......-ppppp-", "........<<<<<<X", ".......@<<<<<X]]]]]@bbw", ".......@XXXXXXX",
                    ".......h<hhhhhh", "........XXXXXXX", top=15),
        "RECOVER": T(".......pppppppp", ".......-ppppp-", "........<<<<<<X]]", ".......@<<<<<X]]]@bw", ".......@XXXXXXX",
                     ".......h<hhhhhh", "........XXXXXXX", top=15),
    },
}

CLIMB = T("..................@", "..................S", ".....@............S", ".....S............S", ".....S............S",
          ".....S............S", ".....S............S", ".....SSSAAAAAAAASSS", ".........AAAAAA", ".........AAAAAA",
          ".........AAAAAA", ".........AAAAAA", ".........hhhhhh", ".........AAAAAA", top=8)
FACE_CHARS = "@]w+,<$"   # the face seen from behind becomes hair or helm
ROLL = T(".........HHHH", ".......HHHHHHHH", "......HHHHHHHHAA", "......HHHHHHHAAAA", ".....AHHHHHAAAAAA",
         ".....AAAAAAAAAASA", ".....AAAAAAAAABBA", "......AABBBBBBBB", "......FFBBBBBBFF", ".......FFFBBBFFF",
         ".........FFFFF", top=16)
ROLL_BOX = (5, 16, 12, 11)   # template columns 5-16, rows 16-26
BACK_VIEW = {"KNIGHT": dict(A="r", S="s", face="s", H="b", B="d", F="&"),
             "ARCHER": dict(A="&", S="@", face="&", H="&", B="/", F="/"),
             "MAGICIAN": dict(A="^", S="^", face=">", H="^", B="`", F="&"),
             "THIEF": dict(A="<", S="<", face="&", H="&", B="X", F="x")}

# --- Skill poses (phase 5b step 6 table). Each: the upper body, its move (dx, dy), a head change ("roar" mouth open,
# "back" turned away, "down" head bowed) and a back-layer override ("flat" for the Shadow Dash's scarf). ---
ARCHER_CORE = T(".........@/&;;&", "........//&&&&&", "........//&&&&&", ".........&&&&&&", ".........&;;;;;",
                ".........;;;;;;", ".........;;;,,,", top=15)


def rotated_bow(pose, degrees):
    """The archer's bow and arms of a basic pose turned about the front shoulder (negative: aiming up), over the core."""
    top, rows = ARMS["ARCHER"][pose]
    ctop, crows = ARCHER_CORE
    core = {(x, ctop + r): ch for r, line in enumerate(crows) for x, ch in enumerate(line) if ch != "."}
    bow = {(x, top + r): ch for r, line in enumerate(rows) for x, ch in enumerate(line) if ch != "." and core.get((x, top + r)) != ch}
    px, py, a = 15, 16, math.radians(degrees)
    out = dict(core)
    for (bx, by), ch in bow.items():   # each bow pixel's four quarter points, turned, mark the cells they land in
        for qx in (0.25, 0.75):
            for qy in (0.25, 0.75):
                x, y = bx + qx - px, by + qy - py
                tx, ty = px + x * math.cos(a) - y * math.sin(a), py + x * math.sin(a) + y * math.cos(a)
                if 0 <= tx < W - SHIFT and 0 <= ty < H:
                    out[(int(tx), int(ty))] = ch
    return T(*["".join(out.get((x, y), ".") for x in range(W - SHIFT)).rstrip(".") for y in range(H)])


SKILLS = {
    "KNIGHT": {
        "SKILL_1": (T("...........bw", "............bw", ".............bw", "..............bw", "...............bw",
                      "................b.d", ".................h", "................d@@", ".................]]",
                      ".................]", ".................]", ".................]", ".................]",
                      ".................]", ".......bbbbsbbsbbb", "....hhhsbbbbbbbsss", "...hsssh&bbbbbss", "...hsdsh&bbbbbs",
                      "...hsdsh&hhhhhhs", "....hhh.@ssssss", "........sssssdd"), 0, 0, None, None),   # RAISE
        "SKILL_2": (T(".......bbbbsbbsbbb", "....hhhsbbbbbbbsss..dbbbbb", "...hsssh&bbbbbs]]]@@dwwwwww",
                      "...hsdsh&bbbbbs.....d", "...hsdsh&hhhhhhs", "....hhh.@ssssss", "........sssssdd", top=15),
                    1, 1, None, None),   # CLEAVE: lunging
        "SKILL_3": (T(".................b", ".................bw", ".................bw", ".................bw",
                      ".................bw", ".................bw", ".................bw", "................ddd",
                      ".................h", ".................@", ".................@", ".................]",
                      ".................]", ".................]", ".................]", ".......bbbbsbbsbbb",
                      "....hhhsbbbbbbbsss", "...hsssh&bbbbbss", "...hsdsh&bbbbbs", "...hsdsh&hhhhhhs", "....hhh.@ssssss",
                      "........sssssdd"), 0, -1, None, None),   # UPPERCUT: stretched up
        "SKILL_4": (T(".......bbbbsbbsbbb", "....hhhsbbbbbbbsss", "...hsssh&bbbbbs]]", "...hsdsh&bbbbbs]]",
                      "...hsdsh&hhhhhh@@", "....hhh.@ssssssddd", "........sssssdd.bw", "................bw",
                      "................bw", "................b", top=15), 0, 2, None, None),   # SLAM: low, point down
        "SKILL_5": (T(".......bbbbsbbsbbb", "....hhhsbbbbbbbsss", "...hsssh&bbbbbs]]", "...hsdsh&bbbbbs.]",
                      "...hsdsh&hhhhhhs.@", "....hhh.@ssssss", ".......dsssssdd", ".....wb", "....wb", "...wb", top=15),
                    0, 0, "roar", None),   # ROAR: sword back low, mouth open
    },
    "ARCHER": {
        "SKILL_1": (ARMS["ARCHER"]["WINDUP"], -1, 0, None, None),   # FULL DRAW: leaning back
        "SKILL_2": (T("..................../", "....................:/", "....................:/", "....................:/",
                      ".........@/&;;&@....:&", "........//&&&&&]@@@@@&", "........//&&&&&.....:/",
                      ".........&&&&&&.....:/", ".........&;;;;;.....:/", ".........;;;;;;...../", ".........;;;,,,", top=11),
                    1, 0, None, None),   # BIG RELEASE: thrust forward, both hands out
        "SKILL_3": (rotated_bow("WINDUP", -45), 0, 0, None, None),   # AIM UP
        "SKILL_4": (rotated_bow("ACTIVE", -45), 0, 0, None, None),   # UP RELEASE
        "SKILL_5": (rotated_bow("WINDUP", -30), 0, 0, None, None),   # STORM: tilted, drawn
    },
    "MAGICIAN": {
        "SKILL_1": (T("...................wc", "...................cc", "...................&", "...................&",
                      "...................&", "...................&", "...................&", "...................&",
                      "...................&", "...................&", "...................&", "...................&",
                      "...................&", "..................@&", "...................&", "......>>:@^^^^^^:.@&",
                      "......>:.@^^^^^^``.&", ".......:.@@^^^^^``.&", "...................&", "...................&",
                      "...................&", "...................&", "...................&"), 0, 0, None, None),   # GATHER
        "SKILL_2": (T("......>>:@^^^^^^:..........w", "......>:.@^^^^^^``@&@&&&&&&c", ".......:.@@^^^^^``", top=15),
                    0, 0, None, None),   # THRUST
        "SKILL_3": (T("....................w", "....................c", "....................&", "....................&",
                      "....................&", "....................&", "....................&", "...................@&",
                      "...................^&", "...................^&", "...................@&", "...................^&",
                      "...................^&", "...................^", "..................^", "......>>:@^^^^^^:^",
                      "......>:.@^^^^^^``", ".......:.@@^^^^^``"), 0, 0, None, None),   # OVERHEAD
        "SKILL_4": (T("....w", "....c", "....&", "....&", "....&", "....&", "....&", "....&", "....&", "....&",
                      "....&.>>:@^^^^^^:....@", "....&.>:.@^^^^^^``^^@@", "....&..:.@@^^^^^``...@", "....&", "....&",
                      "....&", "....&", "....&", "....&", "....&", "....&", "....&", "....&", top=5),
                    0, 0, None, None),   # POINT: staff behind, fingers spread forward
        "SKILL_5": (T("......>>:@^^^^^^:^^@", "......>:.@^^^^^^``^@@", "....c&&&&&&&&&&&&&&", top=15),
                    0, 0, None, None),   # PUSH: palms forward, staff under the arm
    },
    "THIEF": {
        "SKILL_1": (T(".......pppppppp.....@", ".......-ppppp-.....@", "........<<<<<<X]]]]]@", ".......@<<<<<X......@",
                      ".......@XXXXXXX", ".......h<hhhhhh", "........XXXXXXX", top=14), 0, 0, None, None),   # THROW
        "SKILL_2": (ARMS["THIEF"]["ACTIVE"], 2, 2, None, "flat"),   # DASH: low lunge, scarf streaming flat
        "SKILL_3": (T(".......pppppppp", ".......-ppppp-", "wb@]]]]]<<<<<<X]]]]@bw", ".......@<<<<<X", ".......@XXXXXXX",
                      ".......h<hhhhhh", "........XXXXXXX", top=15), 0, 0, "back", None),   # SPIN_A: turned away
        "SKILL_4": (T(".......pppppppp", ".......-pppw-.w", "........<<<<b<b", ".......@<<<<<@", ".......@XXXX@X@",
                      ".......h<hhhhhh", "........XXXXXXX", top=15), 0, 0, None, None),   # SPIN_B: daggers crossed
        "SKILL_5": (T(".......pppppppp", ".......-ppppp-", "........@]]]]]X", ".......@<]]]]]@", ".......@XXXXXXX",
                      ".......h<hhhhhh", "........XXXXXXX", top=15), 0, 0, "down", None),   # CROSS: head bowed
    },
}

# Each body frame: legs, upper-body drop (dy) and lean (dx, back is -1), back-layer variant, eyes shut (climb and roll
# are drawn by body_frame).
BODY = {
    "IDLE_1": ("STAND", 0, 0, "rest", False), "IDLE_2": ("STAND", 1, 0, "rest", False),
    "RUN_1": ("RUN_1", 1, 0, "flutter", False), "RUN_2": ("RUN_2", 0, 0, "rest", False),
    "RUN_3": ("RUN_3", 1, 0, "flutter", False), "RUN_4": ("RUN_4", 0, 0, "rest", False),
    "JUMP": ("TUCK", -1, 0, "rest", False), "PEAK": ("AIR", 0, 0, "flutter", False), "FALL": ("DANGLE", 0, 0, "up", False),
    "LAND": ("SQUAT", 1, 0, "rest", False), "HURT": ("DANGLE", 0, -1, "up", True),
}


def stamp(grid, template, dy=0, keys=None, only_empty=False, dx=0):
    top, rows = template
    for r, text in enumerate(rows):
        for c, k in enumerate(text):
            x, y = c + SHIFT + dx, top + r + dy
            if k == "." or not (0 <= y < H and 0 <= x < W):
                continue
            if only_empty and grid[y][x] != ".":
                continue
            grid[y][x] = keys.get(k, k) if keys else k


def doubled(g):
    rows = []
    for row in g:
        line = "".join(ch * 2 for ch in row)
        rows += [line, line]
    return rows


def compose(cls, legs, dy, back, dx=0, shut=False, torso=None, head=None, head_dy=0):
    """Legs, then the upper body moved by dx, dy, then the back layer and tier details into the empty cells; doubled."""
    c = CLASSES[cls]
    g = [["."] * W for _ in range(H)]
    stamp(g, (ROBE if cls == "MAGICIAN" else LEGS)[legs], 0, keys=c.get("legs"))
    stamp(g, torso or c["torso"], dy, dx=dx)
    stamp(g, head or c["head"], dy + head_dy, dx=dx)
    stamp(g, c["back"].get(back, c["back"]["rest"]), dy, only_empty=True, dx=dx)
    if "tier" in c:
        stamp(g, c["tier"], dy, only_empty=True, dx=dx)
    if shut:   # eyes shut: each eye becomes a 1 px line on its lower row (face rows 11-12, columns 9-15 of the template)
        eye = "," if cls == "MAGICIAN" else "]"
        for y in range(11 + dy, 13 + dy):
            for x in range(9 + SHIFT + dx, 16 + SHIFT + dx):
                if y == 11 + dy and g[y][x] == eye:
                    g[y][x] = "@"
                elif y == 12 + dy and g[y][x] == "w":
                    g[y][x] = eye
    return doubled(g)


def body_frame(cls, name):
    c, keys = CLASSES[cls], BACK_VIEW[cls]
    if name.startswith("CLIMB"):
        g = [["."] * W for _ in range(H)]
        stamp(g, (ROBE if cls == "MAGICIAN" else LEGS)["STAND"], 0, keys=c.get("legs"))
        stamp(g, CLIMB, 0, keys=keys)
        top, rows = c["head"]
        stamp(g, (top, ["".join(keys["face"] if ch in FACE_CHARS else ch for ch in r) for r in rows]))
        if name == "CLIMB_2":
            g = [row[::-1] for row in g]
        return doubled(g)
    if name.startswith("ROLL"):
        g = [["."] * W for _ in range(H)]
        stamp(g, ROLL, 0, keys=keys)
        if name == "ROLL_2":
            x0, y0, bw, bh = ROLL_BOX
            x0 += SHIFT
            box = [g[y0 + y][x0:x0 + bw] for y in range(bh)]
            for y in range(bh):
                g[y0 + y][x0:x0 + bw] = box[bh - 1 - y][::-1]
        return doubled(g)
    legs, dy, dx, back, shut = BODY[name]
    return compose(cls, legs, dy, back, dx, shut)


def arm_frame(cls, pose, legs):
    bob = 1 if legs in ("RUN_1", "RUN_3") else 0
    back = "flutter" if legs in ("RUN_1", "RUN_3") else "rest"
    if pose in ARMS[cls]:
        return compose(cls, legs, bob, back, torso=ARMS[cls][pose])
    torso, dx, dy, head_change, back_override = SKILLS[cls][pose]
    top, rows = CLASSES[cls]["head"]
    head, head_dy = None, 0
    if head_change == "roar":   # the mouth open under the face (template row 13, column 12)
        rows = rows[:]
        rows[13 - top] = rows[13 - top][:12] + "[" + rows[13 - top][13:]
        head = (top, rows)
    elif head_change == "back":   # turned away: the face becomes hair
        head = (top, ["".join(BACK_VIEW[cls]["face"] if ch in FACE_CHARS else ch for ch in r) for r in rows])
    elif head_change == "down":
        head_dy = 1
    return compose(cls, legs, bob + dy, back_override or back, dx, torso=torso, head=head, head_dy=head_dy)


# Front-hand x/y on the 32x28 canvas and weapon angle in degrees. Negative angles point up.
# Melee rows wind up, strike, then settle. Ranged rows start on release and settle into a ready stance.
STRIKE_PATHS = {
    "KNIGHT": [
        [(20,16,-67), (19,14,-88), (20,13,-70), (22,14,-42), (24,16,-8), (24,18,20), (23,19,37), (22,19,55), (21,19,73)],
        [(20,19,57), (19,20,68), (21,20,42), (23,19,18), (24,16,-25), (23,14,-55), (22,13,-72), (21,15,-58), (21,17,-40)],
        [(21,16,-55), (19,13,-89), (18,11,-106), (21,12,-72), (23,15,-32), (24,18,14), (23,20,44), (22,20,59), (21,19,74)],
    ],
    "ARCHER": [
        [(25,16,0), (25,16,-5), (24,16,-8), (23,17,-3), (22,18,6), (21,19,16), (20,20,24), (20,19,32), (20,18,38)],
        [(25,18,9), (25,18,14), (24,19,19), (23,20,24), (22,20,30), (21,20,39), (21,19,43), (20,19,35), (20,18,26)],
        [(24,14,-18), (25,14,-24), (25,15,-17), (24,16,-9), (23,17,0), (22,18,9), (21,19,17), (20,19,23), (20,18,32)],
    ],
    "THIEF": [
        [(19,18,16), (18,19,25), (20,19,13), (24,18,2), (26,17,-4), (25,17,0), (23,18,11), (21,19,25), (20,19,37)],
        [(19,20,43), (18,20,65), (20,20,38), (23,19,8), (25,16,-24), (24,15,-49), (22,15,-65), (21,16,-40), (20,17,-20)],
        [(20,16,-45), (19,14,-65), (20,13,-77), (23,15,-36), (25,18,24), (24,20,46), (23,20,56), (21,19,45), (20,18,33)],
    ],
}

# Preparing a queued shot brings the hand to the height of that shot's release.
DRAW_PATHS = {
    "ARCHER": [[(22,19,14), (23,17,7), (24,16,0)],
               [(22,20,24), (23,19,15), (24,18,9)],
               [(22,17,2), (23,15,-9), (24,14,-18)]],
}

# The free arm is part of each tunic. The weapon arm is drawn from shoulder through elbow to hand.
STRIKE_CORE = {
    "KNIGHT": T(".......bbbbsbbsbbb", "....hhhsbbbbbbbsss", "...hsssh&bbbbbss", "...hsdsh&bbbbbs",
                "...hsdsh&hhhhhhs", "....hhh.@ssssss", "........sssssdd", top=15),
    "ARCHER": ARCHER_CORE,
    "MAGICIAN": T("......>>:@^^^^^^:", "......>:.@^^^^^^``", ".......:.@@^^^^^``", top=15),
    "THIEF": T(".......pppppppp", ".......-ppppp-", "........<<<<<<X", ".......@<<<<<X", ".......@XXXXXXX",
               ".......h<hhhhhh", "........XXXXXXX", top=15),
}


# Hip and knee centres, back leg then front, in the six gait poses. The soles come from the gait art.
LEG_JOINTS = {
    "STAND": ((14, 13), (18, 18)), "RUN_1": ((14, 12), (17, 19)),
    "RUN_2": ((14, 12), (17, 17)), "RUN_3": ((17, 19), (14, 12)),
    "RUN_4": ((17, 17), (14, 12)), "AIR": ((14, 13), (17, 19)),
}


def pixel_line(grid, x0, y0, x1, y1, color):
    steps = max(1, math.ceil(max(abs(x1 - x0), abs(y1 - y0))))
    for i in range(steps + 1):
        x = round(x0 + (x1 - x0) * i / steps)
        y = round(y0 + (y1 - y0) * i / steps)
        assert 0 <= x < W and 0 <= y < H, (x, y)
        grid[y][x] = color


def strike_legs(cls, legs, load, hip_x, drop):
    """Bend the knees between the moving hips and the feet of the current gait pose."""
    grid = [["."] * W for _ in range(H)]
    magician = cls == "MAGICIAN"
    if load == 0:
        stamp(grid, (ROBE if magician else LEGS)[legs], keys=CLASSES[cls].get("legs"))
        return grid

    lower = [["."] * W for _ in range(H)]
    stamp(lower, (ROBE if magician else LEGS)[legs])
    gait = {"RUN_3": "RUN_1", "RUN_4": "RUN_2"}.get(legs, legs) if magician else legs
    colors = CLASSES[cls].get("legs", dict(T="`", U="^", B="&", F="&", G="&", E="&"))
    rear_side = int(legs in ("RUN_3", "RUN_4"))
    for side, (thigh, shin, sole) in enumerate(("TBG", "UFE")):
        hip, knee = LEG_JOINTS[gait][side]
        hip += hip_x
        archer_rear = cls == "ARCHER" and side == rear_side
        loaded = archer_rear if cls == "ARCHER" else side == 1
        knee += round(load * (0.75 if loaded else 0.25))
        shoe = []
        for y in range(22, H):
            xs = [x for x, ch in enumerate(lower[y]) if ch == ("&" if magician else sole)]
            if magician:
                mid = len(xs) // 2
                xs = xs[mid:] if side else xs[:mid]
            shoe.extend((x, y) for x in xs)
        if archer_rear:
            # Lift the rear foot as it travels and plant it at full draw.
            step = (0, 1, 3, 4, 4)[load]
            lift = int(legs == "STAND" and load < 3)
            knee -= (step + 1) // 2
            shoe = [(x - step, y - lift) for x, y in shoe]
        ankle_y = min(y for x, y in shoe)
        ankle = min(x for x, y in shoe if y == ankle_y) + 1
        knee_y = min(ankle_y - 1, 23 + min(1, drop))
        for width in (0, 1):
            pixel_line(grid, hip + width, 21 + drop, knee + width, knee_y, colors[thigh])
            pixel_line(grid, knee + width, knee_y, ankle + width, ankle_y, colors[shin])
        for x, y in shoe:
            grid[y][x] = colors[sole]

    if magician:
        # The hem hangs lower behind the hips and rises where the leading knee presses into the cloth.
        waist, hem = 18, 25
        for x in range(W):
            front = min(1, max(0, (x - 14) / 8))
            raised_hem = hem - round(load * 0.6 * front)
            for y in range(waist + drop, raised_hem + 1):
                depth = (y - waist - drop) / (raised_hem - waist - drop)
                dx = round(hip_x * (1 - depth) + load * 0.75 * front * depth)
                src_x = x - dx
                src_y = waist + round(depth * (hem - waist))
                if 0 <= src_x < W and lower[src_y][src_x] != ".":
                    grid[y][x] = lower[src_y][src_x]
    return grid


# Knee bend, shoulder turn (degrees), grip x/y and wand tip x/y on the hero grid.
# The bolt crosses row 20 on release. Queued draws occupy the preceding cast's last three poses.
MAGICIAN_CASTS = [
    [(3,12,22,20,29,20), (4,5,24,19,31,19), (3,0,24,18,31,16),
     (2,0,23,18,30,17), (2,0,22,19,29,21), (1,0,21,19,27,23),
     (1,0,21,18,27,20), (1,0,21,17,25,13), (0,0,21,17,22,10)],
    [(3,18,23,20,29,20), (4,26,24,17,30,14), (4,32,23,15,27,9),
     (3,40,22,14,24,7), (2,46,21,14,21,7), (2,54,20,15,18,8),
     (1,98,17,16,14,10), (1,182,11,18,5,16), (0,298,17,17,19,10)],
    [(3,308,22,20,29,20), (4,346,24,20,31,20), (4,375,24,20,31,21),
     (3,400,23,21,29,23), (2,415,21,21,26,24), (2,402,20,20,25,23),
     (1,385,20,19,26,20), (1,368,21,18,26,15), (0,360,21,17,22,10)],
]
MAGICIAN_DRAWS = [
    [(1,18,20,20,24,23), (2,26,20,20,24,20), (2,22,21,20,26,19)],
    [(1,8,21,20,26,24), (2,14,21,21,26,25), (3,20,22,21,28,24)],
    [(2,100,16,17,13,12), (3,182,8,19,1,19), (3,260,14,19,11,18)],
]


def magician_cast(strike, frame, legs, drawing):
    """Draw the flick, lifting crescent and broad turning sweep around the current gait's feet."""
    poses = MAGICIAN_DRAWS[strike] if drawing else MAGICIAN_CASTS[strike]
    index = frame - 6 if drawing else frame
    load, turn, hx, hy, tx, ty = poses[index]
    side = math.cos(math.radians(turn))
    hip = round(load * 0.4)
    drop = round(load * 0.5)
    bob = int(legs in ("RUN_1", "RUN_3"))
    grid = strike_legs("MAGICIAN", legs, load, hip, drop)

    # Each afterimage follows the tip. The lifting arc rises in front and the pivot passes behind the body.
    if drawing and strike == 2:
        traces = [((18,9),(15,9),(13,12)), ((12,13),(6,14),(2,16),(1,19)),
                  ((2,20),(4,21),(7,21),(11,18))]
    elif not drawing and strike == 1:
        traces = [((26,25),(28,23),(29,20)), ((28,23),(30,21),(31,18),(30,14)),
                  ((31,18),(31,14),(30,11),(27,9)), ((30,13),(29,10),(27,8),(24,7))]
    elif not drawing and strike == 2:
        traces = [((11,18),(15,17),(23,17),(27,18),(29,20)), ((22,17),(27,18),(30,19),(31,20)),
                  ((26,18),(29,19),(31,21))]
    else:
        traces = []
    if index < len(traces):
        path = traces[index]
        for k, ((x0, y0), (x1, y1)) in enumerate(zip(path, path[1:])):
            color = "i" if k == len(path) - 2 else "u"
            pixel_line(grid, x0, y0 + bob, x1, y1 + bob, color)

    top = [["."] * W for _ in range(H)]
    costume = CLASSES["MAGICIAN"]
    stamp(top, STRIKE_CORE["MAGICIAN"])
    stamp(top, costume["head"])
    stamp(top, costume["back"]["flutter" if load >= 2 else "rest"], only_empty=True)
    stamp(top, costume["tier"], only_empty=True)
    # Project the shoulders, hat and hair through profile. Hide the face on the back half of the pivot.
    width = (0.52 + 0.48 * abs(side)) * (-1 if side < 0 else 1)
    for y in range(H - drop - bob):
        weight = min(1, max(0, (23 - y) / 13))
        shift = hip + round(load * weight)
        for x in range(W):
            src_x = round(16 + (x - 16 - shift) / width)
            if not 0 <= src_x < W:
                continue
            ch = top[y][src_x]
            if ch == ".":
                continue
            if 95 <= turn <= 290 and y >= 8 and ch in FACE_CHARS:
                ch = ":" if src_x % 5 == 0 else ">"
            grid[y + drop + bob][x] = ch

    sy = 16 + drop + bob
    sx = 16 + hip + round(3 * side)
    # The empty arm counterbalances the sweep. The casting wrist leads the tip.
    free_x = round(15 + hip - (5 + load) * side)
    free_y = sy + (0 if strike == 1 else 3)
    pixel_line(grid, 16 + hip, sy, free_x, free_y, "^")
    pixel_line(grid, free_x, free_y, free_x - round(side), free_y + 1, "@")
    hy += bob
    ty += bob
    elbow_x = round((sx + hx) / 2 - side)
    elbow_y = max(sy, hy) + 1
    for offset in (0, 1):
        pixel_line(grid, sx, sy + offset, elbow_x, elbow_y + offset, "^")
        pixel_line(grid, elbow_x, elbow_y + offset, hx, hy + offset, "`")
    butt_x = hx - (tx - hx) * 0.75
    butt_y = hy - (ty - hy) * 0.75
    pixel_line(grid, butt_x, butt_y, tx, ty, "&")
    grid[hy][hx] = "@"
    grid[ty][tx] = "w"
    grid[ty - 1][tx] = "c"
    grid[ty + 1][tx] = "2"
    return doubled(grid)


def strike_frame(cls, strike, frame, legs, drawing=False):
    """Pose the legs, torso and weapon together on one R1 frame."""
    if cls == "MAGICIAN":
        return magician_cast(strike, frame, legs, drawing)
    ranged = cls in DRAW_PATHS
    hand_x, hand_y, angle = STRIKE_PATHS[cls][strike][frame]
    load = (3, 4, 4, 3, 2, 2, 1, 1, 0) if ranged else (0, 1, 1, 2, 4, 4, 3, 1, 0)
    load = load[frame]
    if drawing:
        hand_x, hand_y, angle = DRAW_PATHS[cls][strike][frame - 6]
        load = (1, 2, 3)[frame - 6]
    lean = -load if cls == "ARCHER" else load
    hip_x = round(lean * 0.4)
    drop = round(load * 0.5)
    bob = int(legs in ("RUN_1", "RUN_3"))
    costume = CLASSES[cls]
    grid = strike_legs(cls, legs, load, hip_x, drop)
    upper = [["."] * W for _ in range(H)]
    stamp(upper, STRIKE_CORE[cls])
    stamp(upper, costume["head"])
    back = "flutter" if bob or load >= 3 else "rest"
    stamp(upper, costume["back"][back], only_empty=True)
    stamp(upper, costume["tier"], only_empty=True)
    # Sample around the belt. The brim, brow and shoulders pitch down together at full extension.
    for y in range(H):
        weight = min(1, max(0, (23 + drop - y) / 13))
        for x in range(W):
            pitch = lean * (x - 16) / 33
            src_y = y - drop - round((bob + pitch) * weight)
            src_x = x - hip_x - round(lean * weight)
            if 0 <= src_x < W and 0 <= src_y < H and upper[src_y][src_x] != ".":
                grid[y][x] = upper[src_y][src_x]

    sx, sy = 18 + hip_x + round(lean * 0.5), 16 + bob + drop
    hx = hand_x + (hip_x if cls == "ARCHER" else 0)
    hy = hand_y + bob
    ex, ey = (sx + hx) / 2 - 1, max(sy, hy) + 1
    sleeve = {"KNIGHT": "s", "ARCHER": "&", "THIEF": "<"}[cls]
    pixel_line(grid, sx, sy, ex, ey, sleeve)
    pixel_line(grid, sx, sy + 1, ex, ey + 1, sleeve)
    pixel_line(grid, ex, ey, hx, hy, "]")
    pixel_line(grid, ex, ey + 1, hx, hy + 1, "@" if cls == "ARCHER" else sleeve)

    # Grip-local coordinates: u runs along the blade or arrow, v across it.
    co, si = math.cos(math.radians(angle)), math.sin(math.radians(angle))
    def stroke(u0, v0, u1, v1, color):
        pixel_line(grid, hx + u0*co - v0*si, hy + u0*si + v0*co,
             hx + u1*co - v1*si, hy + u1*si + v1*co, color)

    if cls == "ARCHER":
        pull = (3, 5, 7)[frame - 6] if drawing else (1, 3, 2, 2, 3, 3, 3, 3, 3)[frame]
        for side in (-1, 1):
            stroke(0, 0, -1, side*3, "/")
            stroke(-1, side*3, -3, side*6, "/")
            stroke(-3, side*6, -pull, 0, ":")
        # The drawing hand snaps back to the cheek on release, then returns towards the string.
        fingers = -pull if drawing else (-8, -10, -9, -7, -6, -5, -4, -3, -3)[frame]
        stroke(-8, 1, fingers, 0, "@")
        if drawing:
            stroke(-pull, 0, 2, 0, "/")
            stroke(3, 0, 3, 0, "w")
    else:
        length = 7 if cls == "KNIGHT" else 4
        stroke(-1, 0, 2, 0, "h" if cls == "KNIGHT" else "2")
        stroke(2, -1, 2, 1, "d")
        stroke(3, -1, length - 1, -1, "b")
        stroke(3, 0, length, 0, "w")
    grid[hy][hx] = "@"
    return doubled(grid)


def all_frames():
    out = []
    for cls in CLASS_NAMES:
        for name in BODY_FRAMES:
            out.append((f"HERO_{cls}_{name}", body_frame(cls, name)))
        for pose in ARM_POSES:
            if pose in DRAWS and cls not in ("ARCHER", "MAGICIAN"):
                continue
            for legs in LEG_POSES:
                if pose in STRIKES:
                    strike, frame = STRIKES[pose]
                    rows = strike_frame(cls, strike, frame, legs)
                elif pose in DRAWS:
                    strike, frame = DRAWS[pose]
                    rows = strike_frame(cls, strike, frame + 6, legs, drawing=True)
                else:
                    rows = arm_frame(cls, pose, legs)
                out.append((f"HERO_{cls}_{pose}_{legs}", rows))
    return out


# --- Hero shots and hit sparks (phase 5b step 8, D-063: class colours, never 'm'), art px, doubled, facing right. ---
ARROW_UP = ["........bw", "......//b.", "....//....", ";;//......", ";;........"]
STAR_A = ["..b..", "..s..", "bswss", "..s..", "..s.."]
QUAKE_BASE = ["..ss.H", ".sdss.", "sdddss", "sddsds", "dddddd"]
FROST_QUADRANT = [
    "........w",
    "........i",
    ".....i..i",
    "i.....i.i",
    ".i.I...wi",
    "..wiI...i",
    ".Iiwii..i",
    ".....iiiI",
    ".......Iw",
]
FROST_HALF = [row + row[::-1] for row in FROST_QUADRANT]
FROST_FLAKE = FROST_HALF + FROST_HALF[::-1]
SHOTS = [
    ("ARROW_FLAT", [";;......b.", "////////bw", ";;......b."]),
    ("ARROW_UP", ARROW_UP),
    ("ARROW_DOWN", ARROW_UP[::-1]),
    ("ARROW_FALL", [";.;", ";/;", "./.", "./.", "./.", "./.", "./.", "./.", "bbb", ".w."]),
    ("ARROW_BIG", ["ii.........c..", "bbbbbbbbbbbcww", "ii.........c.."]),
    ("BOLT_A", [".uuuu.", "uccccu", "ucwwcu", "ucwwcu", "uccccu", ".uuuu."]),
    ("BOLT_B", ["..uu..", ".uccu.", "ucwwcu", "ucwwcu", ".uccu.", "..uu.."]),
    ("ORB_A", FROST_FLAKE),
    ("ORB_B", [row.translate(str.maketrans("wiI", "iIw")) for row in FROST_FLAKE]),
    ("STAR_A", STAR_A),
    ("STAR_B", ["b...b", ".s.s.", "..w..", ".s.s.", "s...s"]),
    ("QUAKE_A", ["..H...", ".HsH.."] + QUAKE_BASE),
    ("QUAKE_B", ["...H..", "H.HsH."] + QUAKE_BASE),
    ("METEOR_ROCK", ["O...........", ".OO.........", "..ONO..O....", "...ONNO.O...", "..O.ONNOO...", ".....ONKKK..",
                     "....ONKDDKV.", ".....KDDDKVV", ".....KDDKVVV", "......KKVVVV", ".......VVVV.", "............"]),
    ("SPARK_1", [".......", ".......", "...w...", "..www..", "...w...", ".......", "......."]),
    ("SPARK_2", ["...w...", "...w...", "..www..", "wwwwwww", "..www..", "...w...", "...w..."]),
    ("SPARK_3", ["...w...", ".......", ".......", "w.....w", ".......", ".......", "...w..."]),
]


def shots():
    out = []
    for name, rows in SHOTS:
        assert all(len(r) == len(rows[0]) for r in rows), name
        doubled_rows = []
        for r in rows:
            line = "".join(ch * 2 for ch in r)
            doubled_rows += [line, line]
        out.append((name, doubled_rows))
    return out


def sprite_rows(name, rows, prefix):
    assert not any("??" in r for r in rows), name + ": '??' would start a C++ trigraph"
    body = "\n".join('    "%s"' % r.replace("\\", "\\\\").replace('"', '\\"') for r in rows)
    return "%s %s = {%d, %d,\n%s};\n" % (prefix, name, len(rows[0]), len(rows), body)


def write_all(write, sprite_text, root):
    frames = all_frames()
    h = ["// quest_sprites_hero.h (generated by docs/work-plan/name-quest/art-tools/heroes.py: edit that script, not this file)",
         "#pragma once", "#include \"sprites.h\"",
         f"const int HERO_CLASSES = 4, HERO_BODY_FRAMES = 15, HERO_ARM_POSES = {len(ARM_POSES)}, HERO_LEG_POSES = 6;",
         "// Heroes: 32x28 art px doubled (64x56 real px), facing right; trim 'h' and tier details '1'-'3' recoloured by job tier.",
         "extern const Sprite* const HERO_BODY[HERO_CLASSES][HERO_BODY_FRAMES];",
         "extern const Sprite* const HERO_ARMS[HERO_CLASSES][HERO_ARM_POSES][HERO_LEG_POSES];",
         "// Hero shots and hit sparks (step 8), doubled, facing right.",
         "extern const Sprite ARROW_FLAT, ARROW_UP, ARROW_DOWN, ARROW_FALL, ARROW_BIG, BOLT_A, BOLT_B, ORB_A, ORB_B, STAR_A, STAR_B,",
         "    QUAKE_A, QUAKE_B, METEOR_ROCK, SPARK_1, SPARK_2, SPARK_3;"]
    write(os.path.join(root, "source", "quest_sprites_hero.h"), "\n".join(h) + "\n")
    cpp = ["// quest_sprites_hero.cpp (generated by docs/work-plan/name-quest/art-tools/heroes.py: edit that script, not this file)",
           "#include \"quest_sprites_hero.h\"", ""]
    cpp += [sprite_rows(name, rows, "static const Sprite") for name, rows in frames]
    cpp += [sprite_rows(name, rows, "extern const Sprite") for name, rows in shots()]
    cpp.append("const Sprite* const HERO_BODY[HERO_CLASSES][HERO_BODY_FRAMES] = {")
    for cls in CLASS_NAMES:
        cpp.append("    {" + ", ".join("&HERO_%s_%s" % (cls, f) for f in BODY_FRAMES) + "},")
    cpp.append("};")
    cpp.append("const Sprite* const HERO_ARMS[HERO_CLASSES][HERO_ARM_POSES][HERO_LEG_POSES] = {")
    for cls in CLASS_NAMES:
        cpp.append("    {")
        for pose in ARM_POSES:
            if pose in DRAWS and cls not in ("MAGICIAN", "ARCHER"):
                strike, frame = DRAWS[pose]
                pose = f"STRIKE_{strike+1}_{frame+7}"
            cpp.append("        {" + ", ".join("&HERO_%s_%s_%s" % (cls, pose, l) for l in LEG_POSES) + "},")
        cpp.append("    },")
    cpp.append("};")
    write(os.path.join(root, "source", "quest_sprites_hero.cpp"), "\n".join(cpp) + "\n")


TRIM_BY_JOB = "hbYN"


def tinted(rows, job):
    """The rows as drawn at a job tier: 'h' in the tier colour, '1'-'3' shown from their tier on."""
    trim = TRIM_BY_JOB[job]
    return ["".join(trim if ch == "h" else (trim if int(ch) <= job else ".") if ch in TIER_CHARS else ch for ch in r)
            for r in rows]


def preview(folder, save_png, global_palette):
    """heroes.png: per class the frames being reviewed, on the reference's grey, at 3x (6 screen px per art px)."""
    pal = global_palette()
    bg = (138, 138, 138)
    frames = dict(all_frames())
    names = ["SKILL_1_STAND", "SKILL_2_STAND", "SKILL_3_STAND", "SKILL_4_STAND", "SKILL_5_STAND", "SKILL_2_RUN_1"]
    cw, ch = W * 2 + 8, H * 2 + 8
    img = [[bg] * (cw * len(names)) for _ in range(ch * len(CLASS_NAMES))]
    for j, cls in enumerate(CLASS_NAMES):
        for i, name in enumerate(names):
            for y, line in enumerate(tinted(frames["HERO_%s_%s" % (cls, name)], 0)):
                for x, c in enumerate(line):
                    if c != ".":
                        img[j * ch + 4 + y][i * cw + 4 + x] = pal[c]
    save_png(os.path.join(folder, "heroes.png"), cw * len(names), ch * len(CLASS_NAMES), img, 3)
    img = [[bg] * (cw * 4) for _ in range(ch * len(CLASS_NAMES))]   # heroes_tiers.png: IDLE_1 at tiers 0-3
    for j, cls in enumerate(CLASS_NAMES):
        for job in range(4):
            for y, line in enumerate(tinted(frames["HERO_%s_IDLE_1" % cls], job)):
                for x, c in enumerate(line):
                    if c != ".":
                        img[j * ch + 4 + y][job * cw + 4 + x] = pal[c]
    save_png(os.path.join(folder, "heroes_tiers.png"), cw * 4, ch * len(CLASS_NAMES), img, 3)
    width, height = cw * 9, ch * 3 * len(CLASS_NAMES)
    img = [[bg] * width for _ in range(height)]
    for j, cls in enumerate(CLASS_NAMES):
        for pose, (strike, frame) in STRIKES.items():
            for y, line in enumerate(tinted(frames[f"HERO_{cls}_{pose}_STAND"], 0)):
                for x, c in enumerate(line):
                    if c != ".":
                        img[(j * 3 + strike) * ch + 4 + y][frame * cw + 4 + x] = pal[c]
    save_png(os.path.join(folder, "basic-combos.png"), width, height, img, 2)
