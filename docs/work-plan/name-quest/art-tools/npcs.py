"""NAME QUEST NPCs on the D-065 hero grid. See spec §22 for proportions, floor anchor and authoring rules."""
import os

from heroes import H, W, T, LEGS, doubled, stamp


# Shared face, looking right. Individual heads are layered over it, with room for hoods, hats and hair.
FACE = T(".........@@@@@@", ".........]]@@]@@", ".........w]@@]@@",
         ".........+@@@+@", "..........$@@", top=10)
BLINK = T(".........@@@@@@", ".........@@@@@@@", ".........]]@@]@@",
          ".........+@@@+@", "..........$@@", top=10)
HAIR = T(".........AAAAA", "........AAAAAAA", ".......AAAAAAAAA", ".......AAAAAAABB",
         ".......AAAAABBBB", ".......AAABBBBBB", ".......AABBBB", ".......AAB", ".......AA", top=3)
JACKET = T(".........AAABBB", "........AAAABBBB", ".......AAAABBBBBB", ".......AAAAABBBB@",
           ".......AABBBBBB@@", "........CCCCCCCC", "........AAAAABBB", top=15)
COAT = T(".........AAABBB", "........AAAABBBB", ".......AAAAABBBBB", ".......AAAAABBBB@",
         ".......AAAABBBB@@", ".......CCCCCCCCCC", "......AAAAAABBBBB",
         "......AAAAAABBBBB", ".....AAAAAABBBBBBB", ".....AAAAAABBBBBBB", top=15)

# Named entries follow NpcId order. Headgear and props share the body's idle bob.
NPCS = {
    "ILSA": (T(".........AAAAA", "........AAAAAAA", ".......AAAAAAABB", "......AAAAAAAABBB",
               "......AAAAAAABBBBB", "......AAAAABBBBBBB", ".....AAAAABBBBBBBB", ".....AAAAB..BBBBBB",
               ".....AAAAB.......B", ".....AAAAB.......B", ".....AAAAB.......B", ".....AAAAB.......B",
               ".....AAAAAB..BBBBB", top=2),
             COAT, dict(A="7", B="6", C="4", D="&", E="W"),
             T("..................TT", ".................ThhT", "..................oo",
               ".................hNoh", ".................hoNh", ".................ThhT", top=12)),
    "MARRA": (T("..........AA", "........BAAAAB", ".......BAAAAAABB", ".......AAAAAAABB", "......AAAAAAABBB",
                "......AAAABBBBBB", "......AABBBB", "......AAB", "......AA", "......AA", "......AA", top=2),
              T(".........BBAABBB", "........BBBAAABBB", ".......BBhwWhBBB@", ".......AAbbbWBBB@",
                ".......AA8bbWBB@@", ".......AAbbbWBB", ".......AAbbbWBB", "......AAAbbbWBBB",
                "......AAbbbbWBBB", "......AAbbbbWBBB", top=15),
              dict(A="G", B=";", C="S", D="G", E="/"),
              T("..................l...l", "..................l..l", "...................ll",
                "..................lB@", "....................@", top=14)),
    "BRANN": (T("........AAAAAA", ".......AAAAAAAB", ".......AAAAAAABB", ".......AAAABBBB", ".......AABBBB",
                "......AAA", "......AA", ".......A", "", "", "........hh....hh", "........hhhHHhhh",
                ".........hhhhhh", ".........hhhh", top=3),
              T(".........XXX@@XX", ".......XXX%%WW%%X", "......XX@@%WWWW%@@", "......X@@@%WWWW%@@@",
                ".......@@%WWWWWW%@@@", "........WWSWWWWW", "........WWWWWWWW", "........&WWWWWW&", top=15),
              dict(A="&", B="/", D="X", E="&"),
              T("...................ssbbb", "...................ssbbb", "...................ssbbb",
                ".....................h", ".....................h", ".....................h",
                ".....................h", ".....................@", top=12)),
    "LIO": (T("................N", ".................N", "..........AAA...N", "........AAAAAANNN",
              ".....AAAABBBBBBBAAA", "......AAAABBBBBBBB", "........hhHHHHH", "........hHHHH",
              "........hh", top=1),
            JACKET, dict(A="U", B="u", C="z", D="U", E="&"),
            T("............hhh", "...........hhHHh...SS", "..........hhHHHHhSS@@", "..........hHHxHSS@@",
              "..........hHHHHHh", "...........hhHhh", "............hhh", top=15)),
    "PELL": (HAIR, JACKET, dict(A="s", B="b", C="h", D="X", E="&"),
             T("........TTTTTTTTTTT", "........T88>>8>>88T", ".......@T88>Gll>8T@",
               ".......@T::>l>>>8T@", "........T:88>>88TT", "........TTTTTTTTT", top=17)),
    "SELA": (T("..........AAAA", "........AAAAAAAB", ".......AAAAAAAABB", ".......AAAAAAABBB",
               ".......AAAABBBBBB", ".......AABBBB", ".......AA", "......AAA", "......AAA", ".......AA", top=3),
             COAT, dict(A="d", B="p", C="-", D="d", E="&"),
             T("................../", ".................//", "................./@", "................./@",
               "................./", "................./", "................./", "................./", top=17)),
    "ODO": (T(".........bssss", "........bbbbbss", ".......bbbbbbbss", ".......bsssssssd", ".......bsssddddd",
              ".......bsdd", ".......bsd", ".......bsd", ".......bd..>>>>>", "........>:>>>>>>",
              "........::>>>>>", ".........:>>>", top=5),
            T(".........bssssbb", "........bssssbbbss", ".......bssrrRbbbss", ".......sddrrRRssb@",
              ".......sddrRRRsb@@", "........dssssssbb", "........bddddds", top=15),
            dict(D="d", E="X"),
            T("..................bbb", "...................b", "...................h",
              "...................@", "...................h", "...................bw", "...................bw",
              "...................bw", "...................bw", "...................b", top=17)),
    "WREN": (T(".........AAA..L", "........AAAAALL", ".......AAAAALLAA", ".......AAAALAAAAA",
               "......AAAAAAAABBB", "......AAAAAABBBBB", "......AABBBBBBBB", "......AAB", "......AB",
               "......AB", "......AB", top=2),
             JACKET, dict(A="G", B="g", C="&", D=",", E="&"),
             T(".....h", "....h:", "...h.:", "...h.:", "...h.:", "...h.:", "....h:", ".....h", top=13)),
    "YSOLDE": (T(".......A", ".......AA", ".......AAA", "........AAAA", "........AAAAAA",
                 ".......AAAAAAAB", ".......CCCCCCCC", "....BBBBBBBBBBBBBBB", ".....AAAAAAAAAAAAA",
                 "........>>..>>", ".......>>:", ".......>>:", ".......>>:", "........>:", top=0),
               COAT, dict(A="U", B="p", C="c", D="U", E="&"),
               T(".....................w", "....................cIc", ".....................b",
                 "...................../", "...................../", "....................@/",
                 "...................../", "...................../", "...................../",
                 "...................../", "...................../", "...................../", top=11)),
    "KIT": (T("........AAAAAA", ".......AAAAAAAAB", "......AAAAAAABBBB", "......AAAABBBBBBA",
              "......AABBBBBBAAA", "......AAAAAB", ".......AAA", ".......AAA", ".......AAA..RRRRRR",
              ".......RRRRRRRRRRR", top=4),
            JACKET, dict(A="&", B="X", C="R", D="d", E="x"),
            T("......RRRRRRR", "...rRRRRRRRRRRR", ".....RR.......RR",
              "...................@", "...................sdw", "....................sw", top=14)),
    "GATEKEEPER": (T(".........sssss", "........sbbbbss", ".......sbbbbbbss", "......sbbbbbbssss",
                     "......sbbssssssdd", "......sbsdddddsdd", "......sbsccccdsdd", "......sdsdddddsdd",
                     "......sddssdsdd", ".......sdddddss", "........sdddds", ".........ssss", top=3),
                   T(".......sbbssssssbbb", "......sbbssssssbbbdd", ".....ssbbssbdddbbddd",
                     ".....sbbssbdddbbdddd", "......sbbssdddddbdd", ".......bsdddzdss",
                     ".......sdddz!zds", ".......sddddzdds", "......ssddddddds", "......sdddddddds",
                     "......zzzzzzzzzz", ".....XXXXXddddddd", ".....XXXddddddddd", top=15),
                   dict(D="d", E="X"), None),
    "TOBIN": (T(".........AAAAA", "........AAAAAAAB", ".......AAAAAAABBB", ".......AAAAABBBBB", ".......ABBBB",
                ".......AB", ".......AB", ".......A", top=5),
              T("..........AAABB", ".........AAAABBB", "........AAAABBBB@", "........@AABBBB@", ".........AACBB",
                ".........AABBB", top=15),
              dict(A="C", B="o", C="N", D="h", E="&"), None),
    "GARRICK": (T(".........zzzzzN", "........zzzzzzNN", ".......YYYYYYYYYw", ".......zzzzzzzzzo",
                  ".......sbbbb", ".......sbb", ".......sb", "", "", "........:>....>>", "........:>>>>>>>",
                  ".........:>>>>>", "..........::>>", top=3),
                T(".........RRRRRR", "........RRRRrRRR", ".......RRRRrrRRRR",
                  ".......RRRRrRR@@", "........RRRRR@@", "........RRRrrrr", ".......UUUUUUUUU",
                  ".......UUUUUUUUUU888b&&", "......UUUUUUUUUU888bb&&", "......UUUUUUUUuuu88b&&", top=15),
                dict(D="U", E="&"), None),
    "STRANGER": (T("..........AAAA", ".........AAAAAA", "........AAABBBBB", ".......AAABBBBBBB",
                   "......AAABBBBBBBBB", "......AABBBBBBBBBB", "......AABBBBBBBBBB", "......AABB......BB",
                   "......AABB......BB", "......AABB......BB", "......AABB......BB", "......AABBB.....BB",
                   ".....AAABBBB...BBB", top=2),
                 COAT, dict(A="E", B="^", C="s", D="d", E="&"),
                 T("................88>", "...............@8r>", "................8>>", top=18)),
    "CALDER": (T("..........eEE", ".........eEEEE", "........eEEEEEs", ".......eEEEEEess", ".......eEEEeesss",
                 ".......eEsssssdd", ".......ess", ".......eEs", "", "", "........eEssssds", top=3),
               T(".......eEEEseEEE", "......eEEssseEEEs", "......eEsGGGeeEss", "......eEsseeeesss@",
                 "......eEsseeesss@@", ".......eEEEsszzs", ".......eeeeesss", top=15),
               dict(D="e", E="^"),
               T(".....eEEEEs", "....eEGGEEss", "....eEGEEEss", "....eEEEEsds", ".....eEEdds", "......edds", top=16)),
    "CLAUDE": (T(".........AAAAA", "........AAABBBB", ".......AAABBBBBB", "......AAABBBBBBBB", "......AABBBBBBBBBB",
                 "......AABBBBBBBBBB", "......AABB>>>>>BBB", "......AABB......BB", "......AABB......BB", "......AABB......BB",
                 "......AABB......BB", "......AABBB....BBB", "......AABBBB..BBBB", top=2),
               COAT, dict(A="C", B="Q", C="z", D="R", E="&"),
               T("...............z", "..............zNz", "...............z", top=16)),
}

SURVEYOR_HATS = (
    T(".........AAAAAA", "........AAAAAACC", ".......AAAAAACCC", ".......AAAAACCCC", ".....CCAAAAAAAAACCC",
      "......CCCBBBBBCCC", "........&&&&", "........&&", top=4),
    T("..........CCC", ".........CAACC", ".........AAACC", "........AAAAACC", ".......AAAAAACCC",
      ".......CAAAACCCC", ".......CCCBBBCCC", "........&&&&", "........&&", top=3),
    T("...........CC", ".........CAAACCC", "........CAAAACCCC", ".......CAAAAACCC", "......CCCCAAACCCCC",
      ".......CCCBBBCCCC", "........&&&&", "........&&", top=4),
)
SURVEYOR_MAP = T(".....hzzz", "....WWSzWW", "....WWWSz", "....WWWSW......8888",
                 "....WWWWS.....@8>l>", ".....WWW.......8>>", top=16)


def frames():
    surveyors = {}
    palettes = ("UPz", "PSh", "G;8", "^Ez", "GCN", "Rrb", "Eds", "GSh", "-p8", "RQN", "Uuz", "7Gs")
    for i, (a, b, c) in enumerate(palettes):
        surveyors["SURVEYOR_%d" % i] = (SURVEYOR_HATS[i % len(SURVEYOR_HATS)], COAT,
                                         dict(A=a, B=b, C=c, D=a, E="&"), SURVEYOR_MAP)
    for name, (head, torso, colours, prop) in {**NPCS, **surveyors}.items():
        for i, frame in enumerate("AB"):
            g = [["."] * W for _ in range(H)]
            stone = name == "GATEKEEPER"
            dy = 0 if stone else i + (3 if name in ("TOBIN", "GARRICK") else 0)
            if not stone and name != "GARRICK":
                leg_colours = dict(T=colours["D"], U=colours["D"], B=colours["E"],
                                   F=colours["E"], G=colours["E"], E=colours["E"])
                stand = LEGS["STAND"]
                if name == "TOBIN":
                    stand = T(".........TT..UU", ".........BB..FF", "........GG..EEE", top=25)
                stamp(g, stand, keys=leg_colours)
            stamp(g, torso, 3 if name == "GARRICK" else dy, keys=colours)
            if not stone:
                stamp(g, BLINK if i else FACE, dy)
            stamp(g, head, dy, keys=colours)
            if stone and i:
                stamp(g, T("..........ww..w", top=9))
            if prop:
                stamp(g, prop, dy)
            yield "NPC_%s_%s" % (name, frame), doubled(g)


def preview(npc_frames, folder, save_png, palette):
    cell_w, cell_h, cols = W * 2 + 4, H * 2 + 6, 8
    width, height = cols * cell_w, ((len(npc_frames) + cols - 1) // cols) * cell_h
    rgb = [[(40, 36, 60)] * width for _ in range(height)]
    for i, (_, rows) in enumerate(npc_frames):
        x0, y0 = (i % cols) * cell_w + 2, (i // cols) * cell_h + 2
        for y, row in enumerate(rows):
            for x, c in enumerate(row):
                if c != ".":
                    rgb[y0 + y][x0 + x] = palette[c]
    save_png(os.path.join(folder, "npcs.png"), width, height, rgb, 2)
