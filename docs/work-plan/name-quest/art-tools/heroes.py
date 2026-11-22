"""The four NAME QUEST heroes (spec §21, §22, phase 5b step 2), on the build of the user's reference archer (approved
2026-11-22): a 32x28 art-px canvas doubled to 64x56 real px, facing right, no outline. Head rows 0-14 (face rows 10-13),
body rows 15-21, legs rows 22-27 (feet on row 27); the 8x14 dp hitbox covers art columns 12-19, rows 14-27, and the
canvas is symmetric around it. Per class: 15 body frames, and 14 arm poses (upper body) over 6 leg poses. Trim 'h' and
tier details '1'-'3' are recoloured by job tier when drawn. Writes source/quest_sprites_hero.h/.cpp; called by
make_quest_art.py. Standard library only."""
import math
import os

W, H = 32, 28
SHIFT = 4   # templates are written in the reference's columns; the canvas puts the body's centre on column 15.5
CLASS_NAMES = ["KNIGHT", "ARCHER", "MAGICIAN", "THIEF"]
BODY_FRAMES = ["IDLE_1", "IDLE_2", "RUN_1", "RUN_2", "RUN_3", "RUN_4", "JUMP", "PEAK", "FALL", "LAND", "CLIMB_1", "CLIMB_2",
               "ROLL_1", "ROLL_2", "HURT"]
BASE_ARM_POSES = ["WINDUP", "ACTIVE", "RECOVER", "SKILL_1", "SKILL_2", "SKILL_3", "SKILL_4", "SKILL_5"]
COMBO_ARM_POSES = ["WINDUP_2", "ACTIVE_2", "RECOVER_2", "WINDUP_3", "ACTIVE_3", "RECOVER_3"]
ARM_POSES = BASE_ARM_POSES + COMBO_ARM_POSES
LEG_POSES = ["STAND", "RUN_1", "RUN_2", "RUN_3", "RUN_4", "AIR"]
# Authored: every body frame, three poses per basic attack and five skill poses.
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

# --- Basic attack poses: the whole upper body below the head (torso, both arms, the weapon) for each pose. ---
ARMS = {
    "KNIGHT": {   # sword raised high, struck level at full reach, followed through low in front
        "WINDUP": T(".................b", ".................bw", ".................bw", ".................bw",
                    ".................bw", ".................bw", ".................bw", "................ddd",
                    ".................h", ".................@", ".................]", ".......bbbbsbbsbbb",
                    "....hhhsbbbbbbbsss", "...hsssh&bbbbbss", "...hsdsh&bbbbbs", "...hsdsh&hhhhhhs", "....hhh.@ssssss",
                    "........sssssdd", top=4),
        "ACTIVE": T(".......bbbbsbbsbbb", "....hhhsbbbbbbbsss..dbbbbbb", "...hsssh&bbbbbs]]]]@dwwwwwww",
                    "...hsdsh&bbbbbs.....d", "...hsdsh&hhhhhhs", "....hhh.@ssssss", "........sssssdd", top=15),
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
    "MAGICIAN": {   # the staff pulled back (gem lit), thrust forward, swinging back up
        "WINDUP": T("...................w", "..................cwc", "..................2c2", "...................&",
                    "...................&", "...................&", "...................&", "...................&",
                    "...................&", "......>>:@^^^^^^:..&", "......>:.@^^^^^^``@&", ".......:.@@^^^^^``@&",
                    "...................&", "...................&", "...................&", "...................&",
                    "...................&", "...................&", "...................&", top=6),
        "ACTIVE": T(".........................w", "........................c2", ".......................&",
                    "......................&", "......>>:@^^^^^^:....&", "......>:.@^^^^^^``@@&", ".......:.@@^^^^^``.&",
                    "..................&", ".................&", top=11),
        "RECOVER": T("..................w", "..................c2", "..................&", "..................&",
                     "..................&", "..................&", "...................&", "...................&",
                     "......>>:@^^^^^^:..&", "......>:.@^^^^^^``.&", ".......:.@@^^^^^``@&", "....................&",
                     "....................&", "....................&", "....................&", "....................&",
                     ".....................&", ".....................&", ".....................&", ".....................&",
                     ".....................&", top=7),
    },
    "THIEF": {   # the dagger pulled back to the hip, thrust with the arm fully out, arm coming back
        "WINDUP": T(".......pppppppp", ".......-ppppp-", "........<<<<<<X]", ".......@<<<<<X]]", ".......@XXXXX]@bw",
                    ".......h<hhhhhh", "........XXXXXXX", top=15),
        "ACTIVE": T(".......pppppppp", ".......-ppppp-", "........<<<<<<X", ".......@<<<<<X]]]]]@bbw", ".......@XXXXXXX",
                    ".......h<hhhhhh", "........XXXXXXX", top=15),
        "RECOVER": T(".......pppppppp", ".......-ppppp-", "........<<<<<<X]]", ".......@<<<<<X]]]@bw", ".......@XXXXXXX",
                     ".......h<hhhhhh", "........XXXXXXX", top=15),
    },
}

# The second and third basic strikes, in order: a rising cut and overhead chop, a low reload and snap shot,
# a side cast and raised-staff cast, a reverse cut and downward jab.
ARMS["KNIGHT"].update(
    WINDUP_2=T(".......bbbbsbbsbbb", "....hhhsbbbbbbbsss", "...hsssh&bbbbbss", "...hsdsh&bbbbbs..]",
               "...hsdsh&hhhhhhs.]@", "....hhh.@ssssss..hd", "........sssssdd...bw",
               "...................bw", "....................bw", ".....................bw", top=15),
    ACTIVE_2=T("..........................w", ".........................bw", "........................bw",
               ".......................bw", "......................bw", ".....................bw", "....................bw",
               "...................ddd", ".......bbbbsbbsbbb..h", "....hhhsbbbbbbbsss.@@", "...hsssh&bbbbbss]]]",
               "...hsdsh&bbbbbs..]", "...hsdsh&hhhhhhs", "....hhh.@ssssss", "........sssssdd", top=7),
    RECOVER_2=T("....................w", "...................bw", "...................bw", "..................bw",
                "..................bw", ".................ddd", "..................h", "..................@",
                ".......bbbbsbbsbbb]@", "....hhhsbbbbbbbsss]", "...hsssh&bbbbbss]", "...hsdsh&bbbbbs",
                "...hsdsh&hhhhhhs", "....hhh.@ssssss", "........sssssdd", top=7),
    WINDUP_3=T("................b", "...............bw", "..............bw", ".............bw", "............bw",
               "...........bw", "..........bw", ".........ddd", "..........h.@@", "...........@]]]", "..............]]",
               ".......bbbbsbbsbbb", "....hhhsbbbbbbbsss", "...hsssh&bbbbbss", "...hsdsh&bbbbbs",
               "...hsdsh&hhhhhhs", "....hhh.@ssssss", "........sssssdd", top=4),
    ACTIVE_3=T(".......bbbbsbbsbbb", "....hhhsbbbbbbbsss]", "...hsssh&bbbbbss]]@@", "...hsdsh&bbbbbs....hd",
               "...hsdsh&hhhhhhs.....dbw", "....hhh.@ssssss........bw", "........sssssdd..........bw",
               "..........................bw", "...........................w", top=15),
    RECOVER_3=T(".......bbbbsbbsbbb", "....hhhsbbbbbbbsss", "...hsssh&bbbbbss]", "...hsdsh&bbbbbs..]]",
                "...hsdsh&hhhhhhs...@@", "....hhh.@ssssss.....d", "........sssssdd.....bw",
                ".....................bw", "......................bw", "......................b", top=15),
)
ARMS["ARCHER"].update(
    WINDUP_2=T("....................//", ".................../:", "....................:/", ".................@...:/",
               ".........@/&;;&.@////////bw", "........//&&&&&]@@@@@&", "........//&&&&&...:..:/",
               ".........&&&&&&....::/", ".........&;;;;;...../", ".........;;;;;;...../", ".........;;;,,,", top=11),
    ACTIVE_2=T("...................//", ".................../:", "....................:/", "....................:/",
               "......@@..&&;;&......:&", "......./@@&&&&&]@@@@@&", "........//&&&&&.....:/",
               ".........&&&&&&....:/", ".........&;;;;;...:/", ".........;;;;;;.../", ".........;;;,,,", top=11),
    RECOVER_2=T(".........@/&;;&", "........//&&&&&.....//", "........//&&&&&]@@.::/",
                ".........&&&&&&...@:&", ".........&;;;;;..::/", ".........;;;;;;.::/", ".........;;;,,,./", top=15),
    WINDUP_3=T("......................./", "......................:/", ".....................:/", "....................:/",
               "...................:/", ".................@//bw", ".........@/&;;&.@@@@@&",
               "........//&&&&&]@@..:/", "........//&&&&&.....:/", ".........&&&&&&......:/",
               ".........&;;;;;......./", ".........;;;;;;......./", ".........;;;,,,", top=9),
    ACTIVE_3=T("......................./", "......................:/", "......................:/", ".....................:/",
               ".....................:/", ".........@...........:&", ".......@@@/&;;&..@@@@&",
               "........//&&&&&]@....:/", "........//&&&&&......:/", ".........&&&&&&.......:/",
               ".........&;;;;;........//", ".........;;;;;;", ".........;;;,,,", top=9),
    RECOVER_3=T("....................//", "....................:/", "....................:/", "...................:/",
                ".........@/&;;&....:&", "........//&&&&&]@@@&", "........//&&&&&....:/",
                ".........&&&&&&.....:/", ".........&;;;;;.....:/", ".........;;;;;;......//", ".........;;;,,,", top=11),
)
ARMS["MAGICIAN"].update(
    WINDUP_2=T(".....w", ".....Ic", "......2c", ".......&", "........&", ".........&", "..........&", "...........&",
               "......>>:@@^^^^^:", "......>:..@^^^^^\x60\x60", ".......:.@@^^^^^\x60\x60", top=7),
    ACTIVE_2=T(".....................2cw", "....................&&c", "..................&&", "................&&",
               "......>>:@^^^^^@@&", "......>:.@^^^^^@@", ".......:.@@^^^^^\x60\x60", top=11),
    RECOVER_2=T("..................cww", "...................2c", "....................&", ".....................&",
                "......>>:@^^^^^^:....&", "......>:.@^^^^^^\x60\x60@@&", ".......:.@@^^^^^\x60\x60.&",
                "..................&", ".................&", top=11),
    WINDUP_3=T(".....................w", "....................cIc", "....................2c2", ".....................&",
               ".....................&", ".....................&", "...................@@&", "..................@@.&",
               "......>>:@^^^^^:@@@@.&", "......>:.@^^^^^^\x60\x60...&", ".......:.@@^^^^^\x60\x60..&",
               ".....................&", top=7),
    ACTIVE_3=T(".....................w", "....................Icw", "...................&2c", "...................&",
               "...................&", "......>>:@^^^^^^:@@&", "......>:.@^^^^^\x60\x60@@@@", ".......:.@@^^^^^\x60\x60.&",
               "...................&", "...................&", "...................&", top=10),
    RECOVER_3=T("....................cw", "....................2c", "....................&", "....................&",
                "......>>:@^^^^^^:...&", "......>:.@^^^^^^\x60\x60@@&", ".......:.@@^^^^^\x60\x60..&",
                "....................&", "....................&", "....................&", top=11),
)
ARMS["THIEF"].update(
    WINDUP_2=T(".......pppppppp", ".......-ppppp-", "........<<<<<<X]", ".......@<<<<<X.]]", ".......@XXXXXXX..@@",
               ".......h<hhhhhh....b", "........XXXXXXX.....bw", "......................w", top=15),
    ACTIVE_2=T(".....................w", "....................bw", ".......pppppppp....bw", ".......-ppppp-..@@b",
               "........<<<<<<X]]]]", ".......@<<<<<X]", ".......@XXXXXXX", ".......h<hhhhhh", "........XXXXXXX", top=13),
    RECOVER_2=T(".................w", "................bw", "...............bw", ".......pppppppp@", ".......-ppppp-]]",
                "........<<<<<<X]", ".......@<<<<<X]", ".......@XXXXXXX", ".......h<hhhhhh", "........XXXXXXX", top=12),
    WINDUP_3=T("...................b", "...................bw", "...................bw", "...................@",
               ".......pppppppp...]]", ".......-ppppp-...]]", "........<<<<<<X]]", ".......@<<<<<X]", ".......@XXXXXXX",
               ".......h<hhhhhh", "........XXXXXXX", top=11),
    ACTIVE_3=T(".......pppppppp", ".......-ppppp-...]]", "........<<<<<<X]]..@", ".......@<<<<<X].....bw",
               ".......@XXXXXXX......bw", ".......h<hhhhhh.......w", "........XXXXXXX", top=15),
    RECOVER_3=T(".......pppppppp", ".......-ppppp-", "........<<<<<<X]]", ".......@<<<<<X..]]", ".......@XXXXXXX..@",
                ".......h<hhhhhh..bw", "........XXXXXXX...w", top=15),
)


# --- Climb (back view, arms up; CLIMB_2 is CLIMB_1 mirrored) and roll (a tucked ball; ROLL_2 turned half a circle).
# Keys: A the back (cape, vest, robe, cloak), S the sleeves, H head/hair in the ball, B ball shade, F feet. ---
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


def all_frames():
    """Named frames for each class, including the extra basic attack poses over all six leg poses."""
    out = []
    for cls in CLASS_NAMES:
        for name in BODY_FRAMES:
            out.append(("HERO_%s_%s" % (cls, name), body_frame(cls, name)))
        for pose in BASE_ARM_POSES:
            for legs in LEG_POSES:
                out.append(("HERO_%s_%s_%s" % (cls, pose, legs), arm_frame(cls, pose, legs)))
    for cls in CLASS_NAMES:
        for pose in COMBO_ARM_POSES:
            for legs in LEG_POSES:
                out.append(("HERO_%s_%s_%s" % (cls, pose, legs), arm_frame(cls, pose, legs)))
    return out


# --- Hero shots and hit sparks (phase 5b step 8, D-063: class colours, never 'm'), art px, doubled, facing right. ---
ARROW_UP = ["........bw", "......//b.", "....//....", ";;//......", ";;........"]
STAR_A = ["..b..", "..s..", "bswss", "..s..", "..s.."]
QUAKE_BASE = ["..ss.H", ".sdss.", "sdddss", "sddsds", "dddddd"]
SHOTS = [
    ("ARROW_FLAT", [";;......b.", "////////bw", ";;......b."]),
    ("ARROW_UP", ARROW_UP),
    ("ARROW_DOWN", ARROW_UP[::-1]),
    ("ARROW_FALL", [";.;", ";/;", "./.", "./.", "./.", "./.", "./.", "./.", "bbb", ".w."]),
    ("ARROW_BIG", ["ii.........c..", "bbbbbbbbbbbcww", "ii.........c.."]),
    ("BOLT_A", [".uuuu.", "uccccu", "ucwwcu", "ucwwcu", "uccccu", ".uuuu."]),
    ("BOLT_B", ["..uu..", ".uccu.", "ucwwcu", "ucwwcu", ".uccu.", "..uu.."]),
    ("ORB_A", [".IIII.", "IwiiiI", "IiiiiI", "IiiiiI", "IiiiiI", ".IIII."]),
    ("ORB_B", [".IIII.", "IiiwiI", "IiiiiI", "IIiiiI", "IiIiiI", ".IIII."]),
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
    names = ["WINDUP", "ACTIVE", "RECOVER", "WINDUP_2", "ACTIVE_2", "RECOVER_2", "WINDUP_3", "ACTIVE_3", "RECOVER_3"]
    width, height = cw * len(names), ch * len(CLASS_NAMES)
    img = [[bg] * width for _ in range(height)]
    for j, cls in enumerate(CLASS_NAMES):
        for i, name in enumerate(names):
            for y, line in enumerate(tinted(frames["HERO_%s_%s_STAND" % (cls, name)], 0)):
                for x, c in enumerate(line):
                    if c != ".":
                        img[j * ch + 4 + y][i * cw + 4 + x] = pal[c]
    save_png(os.path.join(folder, "basic-combos.png"), width, height, img, 2)
