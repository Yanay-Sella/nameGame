"""Builds nameGame-work-plan.pdf next to this file; this script is the plan's source, so edit it here and rerun.

Needs reportlab (4.0.x on Python 3.9): python3 -m venv /tmp/pdfenv && /tmp/pdfenv/bin/pip install "reportlab==4.0.9"
Run: /tmp/pdfenv/bin/python docs/work-plan/make_work_plan.py
"""
import os

from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.units import mm
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import (BaseDocTemplate, Flowable, Frame, KeepTogether, NextPageTemplate, PageBreak,
                                PageTemplate, Paragraph, Spacer, Table, TableStyle)
from reportlab.platypus.tableofcontents import TableOfContents

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "nameGame-work-plan.pdf")
VERSION = "1.0"
DATE = "17 November 2026"

FONTS = "/System/Library/Fonts/Supplemental/"
pdfmetrics.registerFont(TTFont("Body", FONTS + "Arial.ttf"))
pdfmetrics.registerFont(TTFont("Body-Bold", FONTS + "Arial Bold.ttf"))
pdfmetrics.registerFont(TTFont("Body-Italic", FONTS + "Arial Italic.ttf"))
pdfmetrics.registerFont(TTFont("Mono", FONTS + "Courier New.ttf"))
pdfmetrics.registerFontFamily("Body", normal="Body", bold="Body-Bold", italic="Body-Italic", boldItalic="Body-Bold")

NAVY = colors.HexColor("#141a33")
INK = colors.HexColor("#22263a")
MUTED = colors.HexColor("#5b6178")
ACCENT = colors.HexColor("#e0309a")      # the enemy-shot magenta
GREEN = colors.HexColor("#3fae5a")       # the green alien
RULE = colors.HexColor("#d9dbe6")
ZEBRA = colors.HexColor("#f4f5fa")
DONE = colors.HexColor("#e3f4e7")
NOW = colors.HexColor("#fde7f3")

styles = {
    "body": ParagraphStyle("body", fontName="Body", fontSize=9.5, leading=13.5, textColor=INK, spaceAfter=5),
    "small": ParagraphStyle("small", fontName="Body", fontSize=8.3, leading=11, textColor=INK),
    "smallbold": ParagraphStyle("smallbold", fontName="Body-Bold", fontSize=8.3, leading=11, textColor=colors.white),
    "note": ParagraphStyle("note", fontName="Body-Italic", fontSize=8.5, leading=12, textColor=MUTED, spaceAfter=6),
    "h1": ParagraphStyle("h1", fontName="Body-Bold", fontSize=19, leading=24, textColor=NAVY, spaceBefore=4, spaceAfter=10),
    "h2": ParagraphStyle("h2", fontName="Body-Bold", fontSize=12.5, leading=16, textColor=NAVY, spaceBefore=10, spaceAfter=5),
    "bullet": ParagraphStyle("bullet", fontName="Body", fontSize=9.5, leading=13.5, textColor=INK, leftIndent=12,
                             bulletIndent=2, spaceAfter=2.5),
    "code": ParagraphStyle("code", fontName="Mono", fontSize=8.6, leading=11.5, textColor=INK, backColor=ZEBRA,
                           borderPadding=(4, 6, 4, 6), leftIndent=6, rightIndent=6, spaceBefore=4, spaceAfter=8),
    "toc1": ParagraphStyle("toc1", fontName="Body", fontSize=10.5, leading=17, textColor=INK, leftIndent=8),
}


class Doc(BaseDocTemplate):
    """A4 document with a full-bleed cover page and numbered content pages; h1 headings feed the contents."""

    def __init__(self, path):
        super().__init__(path, pagesize=A4, title="nameGame work plan", author="nameGame project",
                         subject="Game design, technology and milestones", leftMargin=20 * mm, rightMargin=20 * mm,
                         topMargin=20 * mm, bottomMargin=18 * mm)
        frame = Frame(self.leftMargin, self.bottomMargin, self.width, self.height, id="main")
        self.addPageTemplates([PageTemplate("cover", [frame], onPage=draw_cover),
                               PageTemplate("content", [frame], onPage=draw_page_chrome)])

    def afterFlowable(self, flowable):
        if isinstance(flowable, Paragraph) and flowable.style.name == "h1":
            text = flowable.getPlainText()
            key = "h1-" + text.split(".")[0]
            self.canv.bookmarkPage(key)
            self.canv.addOutlineEntry(text, key, level=0)
            self.notify("TOCEntry", (0, text, self.page, key))


# Sprite grids: the same text-grid format the game will use for its pixel art (D-006).
SHIP = [
    ".......cc.......",
    "......cwwc......",
    "......cwwc......",
    ".....cbwwbc.....",
    ".....bbbbbb.....",
    "..r..bbbbbb..r..",
    "..r.bbbbbbbb.r..",
    ".rb.bbbccbbb.br.",
    ".bbbbbbccbbbbbb.",
    "bbbbbbbbbbbbbbbb",
    "bbbrbbbbbbbbrbbb",
    ".bb.rr....rr.bb.",
    "....oo....oo....",
    ".....o....o.....",
]
ALIEN = [
    "..g......g..",
    "...g....g...",
    "..gggggggg..",
    ".ggwkggwkgg.",
    "gggggggggggg",
    "g.gggggggg.g",
    "g.g......g.g",
    "...gg..gg...",
]
SPRITE_COLORS = {"b": "#c0c4d8", "c": "#57d4f0", "w": "#ffffff", "r": "#e04848", "o": "#ffa020",
                 "g": "#3fae5a", "k": "#141a33"}


def draw_sprite(canv, grid, x, y, pixel):
    for row, line in enumerate(grid):
        for col, ch in enumerate(line):
            if ch in SPRITE_COLORS:
                canv.setFillColor(colors.HexColor(SPRITE_COLORS[ch]))
                canv.rect(x + col * pixel, y - (row + 1) * pixel, pixel, pixel, stroke=0, fill=1)


def draw_cover(canv, doc):
    w, h = A4
    canv.saveState()
    canv.setFillColor(NAVY)
    canv.rect(0, 0, w, h, stroke=0, fill=1)
    canv.setFillColor(colors.HexColor("#8a90b0"))
    for i in range(70):   # fixed pseudo-random starfield
        sx, sy = (i * 137 % 997) / 997 * w, (i * 311 % 991) / 991 * h
        canv.rect(sx, sy, 1.6 if i % 5 else 2.6, 1.6 if i % 5 else 2.6, stroke=0, fill=1)
    for i in range(5):
        draw_sprite(canv, ALIEN, 58 * mm + i * 20 * mm, h - 60 * mm, 4)
    canv.setFillColor(ACCENT)
    canv.rect(90 * mm, h - 92 * mm, 3, 9, stroke=0, fill=1)
    canv.rect(128 * mm, h - 80 * mm, 3, 9, stroke=0, fill=1)
    canv.setFillColor(colors.HexColor("#fff080"))
    canv.rect(w / 2 - 1.5, h - 128 * mm, 3, 10, stroke=0, fill=1)
    draw_sprite(canv, SHIP, w / 2 - 8 * 7, h - 140 * mm, 7)
    canv.setFillColor(colors.white)
    canv.setFont("Body-Bold", 44)
    canv.drawCentredString(w / 2, 92 * mm, "nameGame")
    canv.setFont("Body", 15)
    canv.setFillColor(colors.HexColor("#c9cce0"))
    canv.drawCentredString(w / 2, 80 * mm, "Work plan: game design, technology and milestones")
    canv.setFont("Body", 11)
    canv.setFillColor(ACCENT)
    canv.drawCentredString(w / 2, 70 * mm, "A retro arcade shooter for a real PS3 (homebrew, PSL1GHT + SDL2)")
    canv.setFillColor(colors.HexColor("#8a90b0"))
    canv.setFont("Body", 9.5)
    canv.drawCentredString(w / 2, 26 * mm, "Version %s  ·  %s  ·  working title" % (VERSION, DATE))
    canv.restoreState()


def draw_page_chrome(canv, doc):
    w, h = A4
    canv.saveState()
    canv.setFillColor(ACCENT)
    canv.rect(doc.leftMargin, h - 12 * mm, 14 * mm, 1.6, stroke=0, fill=1)
    canv.setFont("Body", 8)
    canv.setFillColor(MUTED)
    canv.drawString(doc.leftMargin + 17 * mm, h - 12.6 * mm, "nameGame  ·  Work plan v%s" % VERSION)
    canv.drawRightString(w - doc.rightMargin, 10 * mm, "%d" % doc.page)
    canv.drawString(doc.leftMargin, 10 * mm, DATE)
    canv.restoreState()


class Pipeline(Flowable):
    """A row of labelled boxes joined by arrows, for the rendering and workflow diagrams."""

    def __init__(self, steps, width, box_height=17 * mm, highlight=None):
        super().__init__()
        self.steps, self.width, self.box_height, self.highlight = steps, width, box_height, highlight or []

    def wrap(self, *args):
        return self.width, self.box_height + 4

    def draw(self):
        n = len(self.steps)
        gap = 7 * mm
        box_w = (self.width - gap * (n - 1)) / n
        for i, (title, sub) in enumerate(self.steps):
            x = i * (box_w + gap)
            self.canv.setFillColor(NOW if i in self.highlight else ZEBRA)
            self.canv.setStrokeColor(ACCENT if i in self.highlight else RULE)
            self.canv.roundRect(x, 2, box_w, self.box_height, 3, stroke=1, fill=1)
            self.canv.setFillColor(NAVY)
            self.canv.setFont("Body-Bold", 8.4)
            self.canv.drawCentredString(x + box_w / 2, self.box_height - 6.5 * mm + 2, title)
            self.canv.setFillColor(MUTED)
            self.canv.setFont("Body", 7.2)
            for j, line in enumerate(sub.split("\n")):
                self.canv.drawCentredString(x + box_w / 2, self.box_height - 10.5 * mm - j * 3.2 * mm + 2, line)
            if i < n - 1:
                ax = x + box_w + 1.2 * mm
                ay = 2 + self.box_height / 2
                self.canv.setStrokeColor(ACCENT)
                self.canv.setLineWidth(1.2)
                self.canv.line(ax, ay, ax + gap - 2.6 * mm, ay)
                self.canv.setFillColor(ACCENT)
                p = self.canv.beginPath()
                p.moveTo(ax + gap - 2.4 * mm, ay)
                p.lineTo(ax + gap - 4.2 * mm, ay + 1.4 * mm)
                p.lineTo(ax + gap - 4.2 * mm, ay - 1.4 * mm)
                p.close()
                self.canv.drawPath(p, stroke=0, fill=1)


def P(text, style="body"):
    return Paragraph(text, styles[style])


def bullets(items):
    return [Paragraph(item, styles["bullet"], bulletText="•") for item in items]


def table(rows, widths, header=True, row_colors=None):
    """A styled table; cells are strings (wrapped as Paragraphs) or flowables."""
    data = []
    for r, row in enumerate(rows):
        style = "smallbold" if header and r == 0 else "small"
        data.append([Paragraph(c, styles[style]) if isinstance(c, str) else c for c in row])
    t = Table(data, colWidths=widths, repeatRows=1 if header else 0)
    cmds = [("VALIGN", (0, 0), (-1, -1), "TOP"),
            ("TOPPADDING", (0, 0), (-1, -1), 3.5), ("BOTTOMPADDING", (0, 0), (-1, -1), 3.5),
            ("LEFTPADDING", (0, 0), (-1, -1), 5), ("RIGHTPADDING", (0, 0), (-1, -1), 5),
            ("LINEBELOW", (0, 0), (-1, -1), 0.5, RULE)]
    if header:
        cmds += [("BACKGROUND", (0, 0), (-1, 0), NAVY)]
    for r in range(1 if header else 0, len(rows)):
        if r % 2 == 0:
            cmds.append(("BACKGROUND", (0, r), (-1, r), ZEBRA))
    for r, color in (row_colors or {}).items():
        cmds.append(("BACKGROUND", (0, r), (-1, r), color))
    t.setStyle(TableStyle(cmds))
    return t


def section(title):
    return [PageBreak(), P(title, "h1")]


FULL = A4[0] - 40 * mm


def build():
    story = [NextPageTemplate("content"), PageBreak(), Paragraph("Contents", ParagraphStyle("contents", parent=styles["h1"]))]
    toc = TableOfContents()
    toc.levelStyles = [styles["toc1"]]
    toc.dotsMinLevel = 0
    story.append(toc)
    story.append(Spacer(1, 10))
    story.append(P("How to read this plan: sections 1–4 describe the game and how it is designed; section 5 is the "
                   "technology; sections 6–8 are the milestones with every task; sections 9–12 cover testing, risks, "
                   "decisions and open questions. The living design lives in <font name='Mono'>docs/GDD.md</font> and "
                   "<font name='Mono'>docs/features/</font>; when they disagree with this PDF, the GDD wins and this "
                   "plan is regenerated.", "note"))

    # 1. Status
    story += section("1. Where the project stands")
    story.append(P("nameGame is an original fixed-screen arcade shooter for a real PS3, in the spirit of 80s alien-"
                   "formation shooters and later wave-based shmups. You fly a small ship at the bottom of the screen, "
                   "hold R1 to spray bullets and fire a scarce, powerful rocket with R2. Waves get harder, every 5th "
                   "wave is a boss, and a run ends when the last life is lost."))
    story.append(P("Milestone 1 is done: a throwaway grey-box prototype ran on the console and the verdict was "
                   "<b>keep</b>.", "body"))
    story.append(table([
        ["Question the prototype answered", "Result on the PS3 (17 Nov 2026)"],
        ["Does flying and shooting feel tight?", "Yes: the player played it and said it feels good"],
        ["Steady 60 fps?", "60 fps every second; 1.0 ms of work per frame (max 2.0 ms) of a 16.7 ms budget"],
        ["Chunky 320×180 pixels scaled to the TV?", "1920×1080 output, scaled ×6 exactly, no borders"],
        ["Does the song play?", "music.ogg loaded and played at 48 kHz stereo, looping"],
        ["Which controller buttons are which?", "Confirmed: 0 left, 2 right, 12 R1; others from the SDL port source"],
        ["Not exercised yet", "R2 rocket, Triangle stress test, Start pause (no presses in the log)"],
    ], [62 * mm, FULL - 62 * mm]))
    story.append(Spacer(1, 8))
    story.append(P("Milestone status", "h2"))
    story.append(table([
        ["Milestone", "Goal", "Status"],
        ["M1 Greybox on the PS3", "Prove controls, scaling, music and frame rate on the console", "Done, kept"],
        ["M2 Core loop", "A full run from wave 1 to game over, real pixel art and sound", "Next (section 7)"],
        ["M3 Full game", "Bosses, upgrades, screens, high score save, game feel", "Planned (section 8)"],
        ["M4 Release polish", "Name, icon, balancing, stability, install guide", "Planned (section 8)"],
    ], [42 * mm, FULL - 42 * mm - 32 * mm, 32 * mm], row_colors={1: DONE, 2: NOW}))

    # 2. The game
    story += section("2. The game at a glance")
    story.append(table([
        ["<b>Genre and camera</b>", "2D fixed-screen vertical shooter: aliens above, your ship below"],
        ["<b>Player</b>", "Casual players with arcade instincts; pick up and play, no tutorial"],
        ["<b>Platform and input</b>", "Real PS3 running HEN; one DualShock 3 / Sixaxis; single player, co-op-ready code"],
        ["<b>Session</b>", "One run of 5–15 minutes; play until game over, chase the saved high score"],
        ["<b>Look</b>", "Chunky pixel art drawn at 320×180, scaled ×6 (1080p) or ×4 (720p), no smoothing"],
        ["<b>Sound</b>", "The user's song looping during gameplay, plus synthesized 8-bit sound effects"],
    ], [38 * mm, FULL - 38 * mm], header=False))
    story.append(P("Pillars", "h2"))
    story.append(P("Pillars settle design arguments. When two ideas compete, the one that serves the pillars wins.", "note"))
    story += bullets([
        "<b>Readable at a glance.</b> Chunky pixels; enemy shots are the only magenta objects. Anything that hides a "
        "bullet is cut.",
        "<b>The rocket is the panic button.</b> Bullets handle routine; rockets stay scarce and decisive. No rocket spam.",
        "<b>Arcade, not a campaign.</b> 3 lives, a score and a high score. No story, no menus beyond what is needed.",
        "<b>60 fps on a real PS3.</b> Any feature or effect that drops frames on the console is simplified or cut.",
    ])
    story.append(P("Core loop", "h2"))
    story.append(Pipeline([("Dodge + shoot", "R1 held, stick moves"), ("Kill aliens", "score, drops"),
                           ("Pick up", "rockets, upgrades"), ("Harder wave", "faster, more shots"),
                           ("Boss", "every 5th wave")], FULL))
    story.append(Spacer(1, 6))
    story += bullets(["<b>Seconds:</b> moving, shooting, dodging shots, timing the rocket.",
                      "<b>Minutes:</b> waves clear and escalate; a boss every 5th wave.",
                      "<b>Between runs:</b> beat the saved high score."])
    story.append(P("Controls", "h2"))
    story.append(table([
        ["Action", "Button", "SDL index", "Read as"],
        ["Move", "Left stick or D-pad", "axes 0/1; buttons 0–3", "continuous, radial deadzone 0.2"],
        ["Fire", "R1", "12 (confirmed)", "held: auto-fire every 0.12 s"],
        ["Rocket", "R2", "14", "press, buffered 0.15 s"],
        ["Pause / resume", "Start", "4", "press"],
        ["Confirm in menus", "Cross (X)", "9", "press"],
        ["Back in menus", "Circle", "10", "press"],
    ], [34 * mm, 34 * mm, 42 * mm, FULL - 110 * mm]))
    story.append(P("Screens and flow", "h2"))
    story.append(Pipeline([("Boot", "logs, audio"), ("Title", "high score\nno music"), ("Game", "HUD, music loops"),
                           ("Pause", "Start toggles"), ("Game over", "score, wave\nnew best?")], FULL))

    # 3. Game design
    story += section("3. Game design")
    story.append(P("Every feature, its milestone and its status. Status words are the same everywhere: idea → "
                   "designed → in progress → implemented (plus changed — code pending, and cut).", "note"))
    story.append(table([
        ["Feature", "Summary", "Milestone", "Status"],
        ["Ship controls", "Free movement in the bottom zone; hold R1 to auto-fire", "M2", "designed"],
        ["Rocket", "R2 fires a limited rocket that explodes in a 40 px radius", "M2", "designed"],
        ["Enemy waves", "Formations that fly in, sway, shoot and dive; harder each wave", "M2", "designed"],
        ["Lives and score", "3 lives, one hit kills, respawn invincibility, extra life every 50,000", "M2", "idea"],
        ["Music and sound", "Song loops in gameplay; synthesized retro SFX", "M2", "idea"],
        ["Pixel art", "Palette, sprites and font as text grids in code", "M2", "idea"],
        ["Bosses", "A boss on every 5th wave, stronger each time", "M3", "idea"],
        ["Weapon upgrades", "Pickups: single → double → spread → fast spread; death loses a level", "M3", "idea"],
        ["Screens", "Title, pause menu (with options), game over", "M3", "idea"],
        ["High score save", "Best score saved on the PS3, shown on the title", "M3", "idea"],
        ["Game feel", "Shake, hit-stop, flashes, particles by tier; shake and flash options", "M3", "idea"],
        ["Co-op", "Second ship on a second controller", "later", "idea"],
    ], [32 * mm, FULL - 32 * mm - 20 * mm - 24 * mm, 20 * mm, 24 * mm]))
    story.append(P("Starting numbers for the core features", "h2"))
    story.append(P("All values are first guesses marked [ASSUMPTION] in the feature notes, except those confirmed in the "
                   "M1 playtest (ship speed and fire rate felt good). Tuning changes one number at a time.", "note"))
    story.append(table([
        ["Feature", "Value", "Number"],
        ["Ship", "speed / hitbox / movement zone", "150 px/s · 6×6 px · y 100–180"],
        ["Ship", "fire interval / bullet speed / damage", "0.12 s · 320 px/s · 1 HP"],
        ["Rocket", "start / max ammo / cooldown / buffer", "3 · 9 · 0.5 s · 0.15 s"],
        ["Rocket", "speed / blast radius / blast damage", "60 → 260 px/s · 40 px · 20 HP"],
        ["Rocket", "pickup drop chance", "2 % per kill (about one per 2 waves)"],
        ["Aliens", "types (HP / score)", "green 1 / 100 · blue 2 / 200 (wave 4+) · red 4 / 400 (wave 7+)"],
        ["Aliens", "enemy shot speed / wave clear bonus", "70 px/s · 1,000 × wave"],
        ["Lives", "lives / respawn invincibility / extra life", "3 · 2 s · every 50,000 points (max 9)"],
    ], [24 * mm, 66 * mm, FULL - 90 * mm]))
    story.append(P("Difficulty curve, waves 1–20", "h2"))
    rows = [["Wave", "Aliens", "Sway px/s", "Shots per alien/s", "Dive every", "Types", "Clear bonus"]]
    highlight = {}
    for n in range(1, 21):
        if n % 5 == 0:
            rows.append([str(n), "BOSS", "—", "—", "—", "boss %d" % (n // 5), "—"])
            highlight[len(rows) - 1] = NOW
            continue
        alien_rows = min(2 + n // 2, 5)
        types = "green" + (" blue" if n >= 4 else "") + (" red" if n >= 7 else "")
        dive = "%.1f s" % max(4 - 0.2 * n, 1.5) if n >= 3 else "—"
        rows.append([str(n), str(8 * alien_rows), str(min(20 + 2 * n, 50)), "%.3f" % min(0.02 + 0.005 * n, 0.08),
                     dive, types, "{:,}".format(1000 * n)])
    story.append(table(rows, [13 * mm, 17 * mm, 20 * mm, 30 * mm, 20 * mm, FULL - 125 * mm, 25 * mm],
                       row_colors=highlight))
    story.append(P("Formulas from the enemy-waves note: rows = min(2 + wave/2, 5) of 8; sway = min(20 + 2·wave, 50); "
                   "shot chance = min(0.02 + 0.005·wave, 0.08); dives from wave 3, interval max(4 − 0.2·wave, 1.5) s.",
                   "note"))

    story.append(P("Art direction", "h2"))
    story += bullets([
        "Everything is drawn at 320×180 and scaled by a whole number, so pixels stay square and sharp.",
        "A fixed 16-colour palette. Magenta/pink is reserved for enemy shots; the player's bullets are pale yellow.",
        "Sprites are text grids in code (one character per pixel), like the ship and alien on the cover.",
        "Sprite list for M2: ship 16×16 (2 thruster frames), 3 aliens 12×12 (2 frames each), bullet 2×4, enemy shot "
        "2×4, rocket 4×8, explosion 4 frames, rocket pickup 8×8, HUD icons (life, rocket), 3×5 font, 3-layer starfield.",
    ])
    story.append(P("Audio direction", "h2"))
    story += bullets([
        "The user's song (converted to OGG Vorbis, 48 kHz) loops only during gameplay; the title screen is silent.",
        "Sound effects are synthesized at startup (square waves, noise, pitch sweeps), so there are no sound files.",
        "M2 sound list: shoot, alien hit, alien death, rocket launch, explosion, pickup, player death, empty-rocket "
        "click, wave start jingle. Effects sit under the music; shooting is the quietest.",
    ])
    story.append(P("Game feel (M3)", "h2"))
    story.append(table([
        ["Tier", "Shake (trauma)", "Hit-stop", "Particles", "Events"],
        ["small", "0.10–0.20", "none", "0–4", "bullet hits an alien, pickup collected"],
        ["medium", "0.30–0.50", "0.04–0.06 s", "6–12", "alien dies, extra life"],
        ["large", "0.70–1.00", "0.10–0.15 s", "20–40", "rocket explosion, boss hit/death, player death"],
    ], [18 * mm, 28 * mm, 24 * mm, 20 * mm, FULL - 90 * mm]))
    story.append(P("Shake moves only the picture, never collisions. Hit-stop slows game time but not shake decay, "
                   "menus or the music. Shooting gets almost no feedback so holding R1 doesn't buzz. Options: shake "
                   "strength 0–100 % and reduce flashing; big flashes at most 3 per second.", "note"))

    # 4. Design process
    story += section("4. How the game is designed")
    story.append(P("Each feature goes through the same steps. Docs stay short and are edited in place, so they are "
                   "always true to the code."))
    story.append(Pipeline([("1 Idea", "row in GDD\nfeature index"), ("2 Design note", "rules + numbers\n[ASSUMPTION]"),
                           ("3 Prototype", "only if risky\nthrowaway"), ("4 Build", "in source/\nreal code"),
                           ("5 Playtest", "on the PS3\nuser reports"), ("6 Tune + sync", "one value a time\ndocs match code")],
                          FULL, box_height=19 * mm))
    story.append(Spacer(1, 8))
    story.append(table([
        ["Step", "What happens", "Output"],
        ["1 Idea", "The feature gets one row in the GDD feature index with a one-line summary.", "status: idea"],
        ["2 Design note", "Rules (numbered, testable), a numbers table with units, edge cases (pause, death, spam), "
                          "open questions. Guesses are marked [ASSUMPTION].",
         "features/&lt;slug&gt;/design.md, status: designed"],
        ["3 Prototype", "Only for real risk (tech or 'is it fun?'). Written brief with keep / retune / kill signals, "
                        "grey boxes, debug HUD, its own folder.", "prototypes/&lt;idea&gt;/BRIEF.md with RESULT"],
        ["4 Build", "Real code in source/, with the note's numbers as named constants. implementation.md and "
                    "testing.md are written against the real code.", "status: in progress"],
        ["5 Playtest", "Deploy, run make log, the user plays without coaching and reports: did it read, did it feel "
                       "good, what felt off. Logs give fps and events.", "notes for tuning"],
        ["6 Tune + sync", "Change one constant, replay. When it's right: update design.md numbers, tick proven "
                          "criteria, log decisions, run the docs checker.", "status: implemented"],
    ], [26 * mm, FULL - 26 * mm - 50 * mm, 50 * mm]))
    story.append(P("Where each fact lives", "h2"))
    story.append(table([
        ["Content", "Home"],
        ["Pitch, pillars, core loop, art/audio direction, milestones", "docs/GDD.md"],
        ["A feature's rules, numbers, edge cases", "docs/features/&lt;slug&gt;/design.md"],
        ["Which files a feature touches, its constants, deviations", "docs/features/&lt;slug&gt;/implementation.md"],
        ["Acceptance criteria and their proof, playtest steps", "docs/features/&lt;slug&gt;/testing.md"],
        ["Decisions that matter beyond today", "GDD → Decisions (D-001 …)"],
        ["PS3 setup, commands, SDL port quirks", "CLAUDE.md, docs/ps3-new-project-guide.md"],
        ["The milestone task plan", "this PDF (docs/work-plan/)"],
    ], [FULL - 70 * mm, 70 * mm]))
    story.append(P("Playtest protocol", "h2"))
    story += bullets([
        "Start <font name='Mono'>make log</font> before launching, so every event and the fps line is recorded.",
        "Play without being told what to look for; afterwards answer: did you understand it within 30 s, did a "
        "death feel deserved, what felt slow or unfair, did you want another run?",
        "Watch the log for fps below 60, the watchdog line, and work time above 8 ms (half the frame budget).",
        "Retune one value per round; a value that keeps changing gets a decision row explaining why.",
    ])

    # 5. Technology
    story += section("5. Technology")
    story.append(P("The stack is fixed by the platform: open-source PS3 homebrew. Sony's SDK and the Unity or Unreal "
                   "PS3 exports are not available and are not used."))
    story.append(table([
        ["Layer", "What we use", "Notes"],
        ["Hardware", "PS3, firmware 4.93, PS3HEN + webMAN MOD", "HEN must be re-enabled after every PS3 reboot"],
        ["Language / compiler", "C++ (GCC for the Cell PPU)", "single-threaded game loop plus a watchdog thread"],
        ["SDK", "PSL1GHT via the ps3dev toolchain", "in Docker image namegame-ps3dev"],
        ["Graphics / input", "SDL2 (shagkur/SDL_PSL1GHT) + our render patch", "patch fixes fill-rect and colour bugs"],
        ["Audio", "SDL2_mixer with libvorbis (OGG) and MikMod", "the port's mixer has no FLAC or MP3"],
        ["Logging", "debugnet over UDP to the Mac (make log)", "the Mac's IP is baked in at build time"],
        ["Build", "Docker on the Mac, make ps3 → build/nameGame.pkg", "always a full rebuild"],
        ["Deploy", "FTP upload + webMAN install (make deploy)", "refuses if a game runs or webMAN is silent"],
        ["Assets", "ffmpeg (make music) for the song; sprites and SFX in code", "music.ogg is gitignored"],
        ["Docs", "Markdown + game-docs checker; this PDF via reportlab", "check_docs.py must exit 0"],
    ], [30 * mm, 70 * mm, FULL - 100 * mm]))
    story.append(P("What the SDL port can and can't do", "h2"))
    story.append(P("Learned from the port's source code and the M1 test. These limits shaped the architecture.", "note"))
    story += bullets([
        "<b>No render targets</b> (<font name='Mono'>SDL_SetRenderTarget</font> does nothing) and no rotation or flip.",
        "<b>SDL_RenderCopy has no alpha blending</b>: transparent sprite pixels come out as solid boxes.",
        "<b>SDL_RenderCopy is a hardware scaled blit</b> with nearest-neighbour filtering: ideal for scaling one "
        "320×180 picture to the TV.",
        "Clear and fill-rect are CPU writes into video memory, so full-screen clears every frame are avoided.",
        "Audio runs at 48 kHz stereo float; the mixer opens at 48 kHz to avoid resampling.",
    ])
    story.append(P("Rendering pipeline (decision D-008)", "h2"))
    story.append(Pipeline([("Game draws", "sprites, text, stars\nwith transparency"),
                           ("CPU pixel buffer", "320×180 ARGB\n225 KB"),
                           ("Upload", "SDL_UpdateTexture\nto video memory"),
                           ("Scaled copy", "SDL_RenderCopy\nhardware, ×6 nearest"),
                           ("Present", "vsync flip\n60 fps")], FULL, box_height=19 * mm, highlight=[1]))
    story.append(Spacer(1, 6))
    story.append(P("All transparency, sprite drawing and text happen in our own code on a small buffer, which costs "
                   "about 1 ms per frame; the PS3's hardware does the enlargement for free."))
    story.append(KeepTogether([
        P("Planned code structure for M2", "h2"),
        P("Few files with clear jobs; the game rules don't include any PS3 or SDL header, so they also "
          "compile on the Mac for tests.", "note"),
        table([
            ["File", "Job"],
            ["source/main.cpp", "Startup, main loop, screen states, watchdog, frame timing"],
            ["source/screen.cpp/.h", "320×180 pixel buffer: rects, sprites with transparency, text, stars; scaled present"],
            ["source/sprites.h", "Palette and all sprites and font glyphs as text grids"],
            ["source/input.cpp/.h", "Named actions per player (move, fire, rocket, pause, confirm), deadzone, rocket buffer"],
            ["source/game.cpp/.h", "The game rules: players, bullets, aliens, waves, rockets, pickups, score, lives (no SDL)"],
            ["source/audio.cpp/.h", "Music start/stop and the SFX synthesizer"],
            ["tests/game_tests.cpp", "Mac-side tests of the rules (make test), no framework needed"],
        ], [42 * mm, FULL - 42 * mm]),
    ]))
    story.append(KeepTogether([
        P("Everyday commands", "h2"),
        table([
            ["Command", "What it does"],
            ["<font name='Mono'>make deploy</font>", "Build in Docker and install on the PS3"],
            ["<font name='Mono'>make log</font>", "Print the game's log lines; start it before launching the game"],
            ["<font name='Mono'>make test</font>", "Run the game-rule tests on the Mac (added in M2)"],
            ["<font name='Mono'>make music MUSIC_SRC=song.flac</font>", "Convert a song into the package as OGG"],
            ["<font name='Mono'>make deploy SOURCES=prototypes/ship-feel</font>", "Install a prototype instead of the game"],
        ], [80 * mm, FULL - 80 * mm]),
    ]))

    # 6. Roadmap
    story += section("6. Roadmap")
    story.append(table([
        ["Milestone", "Contents", "Done when"],
        ["M1 Greybox (done)", "Grey-box ship, bullets, rocket, formation; scaling, music and buttons proven",
         "BRIEF.md has a RESULT line: keep, 17 Nov 2026"],
        ["M2 Core loop", "Ship controls, rocket, enemy waves, lives and score, music and sound, pixel art, Mac tests",
         "A run from wave 1 to game over on the PS3 at 60 fps with real art and sound"],
        ["M3 Full game", "Bosses, weapon upgrades, screens, high score save, game feel and its options",
         "Title → run → game over → title works on the PS3; the high score survives a reboot"],
        ["M4 Release polish", "Name and icon, balancing to wave 20, unplug handling, stability run, install guide",
         "A fresh player installs from the guide and plays a full run without help"],
        ["Later", "Co-op on a second controller; button remapping menu", "Only if wanted after M4"],
    ], [34 * mm, FULL - 34 * mm - 62 * mm, 62 * mm], row_colors={1: DONE, 2: NOW}))
    story.append(Spacer(1, 8))
    story.append(P("Rules for every milestone", "h2"))
    story += bullets([
        "Each task ends in something that runs on the PS3; nothing waits until the end to be tried.",
        "One branch per milestone (for example <font name='Mono'>m2/core-loop</font>), committed after each "
        "working task, merged into main when the milestone's 'done when' is met.",
        "Design notes come before code: a feature at 'idea' gets its design.md first.",
        "The milestone ends with a docs sync: statuses, implementation.md, testing.md, decisions, checker at exit 0.",
    ])

    # 7. M2
    story += section("7. Milestone 2: the core loop")
    story.append(P("<b>Goal:</b> a complete run on the PS3, from wave 1 to game over, with real pixel art, the song "
                   "and sound effects, at 60 fps. The prototype is not copied: its lessons (pixel buffer + scaled "
                   "copy, button numbers, tuning values) are rewritten cleanly in <font name='Mono'>source/</font>."))
    m2 = [
        ["#", "Task", "Delivers", "Done when", "Size"],
        ["1", "Design notes for the M2 features still at 'idea'",
         "lives-and-score, music-and-sound, pixel-art design.md", "statuses 'designed'; checker exit 0", "S"],
        ["2", "Code skeleton in source/", "main loop, watchdog, screen buffer + scaled copy, input actions, "
         "Player array (co-op-ready)", "ship box moves on the PS3; actions in the log", "M"],
        ["3", "Pixel art and font", "palette, all M2 sprites, 3×5 font, sprite drawing with transparency, starfield",
         "a sprite test screen on the PS3 looks right on the TV", "M"],
        ["4", "Ship controls", "movement zone, deadzone, D-pad, auto-fire, bullets", "matches its design note; "
         "R1, D-pad and stick confirmed", "S"],
        ["5", "Enemy waves", "fly-in, formation sway, enemy shots, dives from wave 3, 3 alien types, wave banner",
         "waves 1–9 play as the difficulty table says", "L"],
        ["6", "Rocket", "ammo + HUD icons, cooldown, input buffer, blast, pickups, empty click",
         "R2 confirmed on the PS3; blast kills inside 40 px only", "M"],
        ["7", "Lives and score", "one-hit death, respawn with 2 s blink, extra lives, score HUD, game over → "
         "X restarts", "a run ends at 0 lives and restarts cleanly", "M"],
        ["8", "Music and sound", "song loops in gameplay only; SFX synthesizer and the 9 sounds; volume balance",
         "no stutter; every event has its sound", "M"],
        ["9", "Mac tests", "make test: deadzone, rocket buffer, collisions, wave formulas, extra-life thresholds",
         "all pass; each test fails when its value is broken", "S"],
        ["10", "PS3 performance and playtest", "stress test 60 aliens + 100 bullets + 2 blasts; 2 full runs by "
         "the user; one tuning round", "60 fps, work under 8 ms; tuning logged", "M"],
        ["11", "Docs sync", "implementation.md + testing.md per feature; statuses; GDD change log",
         "checker exit 0; M2 marked done", "S"],
    ]
    story.append(table(m2, [8 * mm, 34 * mm, FULL - 8 * mm - 34 * mm - 48 * mm - 11 * mm, 48 * mm, 11 * mm]))
    story.append(P("Size: S = a short session, M = about a day of work, L = several sessions. Tasks 4–8 each deploy "
                   "to the PS3 and get a quick check from the user.", "note"))
    story.append(P("Order and dependencies", "h2"))
    story.append(Pipeline([("1 Notes", "design first"), ("2 Skeleton", "loop, screen,\ninput"),
                           ("3 Pixel art", "sprites, font"), ("4–7 Gameplay", "ship, waves,\nrocket, lives"),
                           ("8 Sound", "music + SFX"), ("9–11 Verify", "tests, PS3,\ndocs")], FULL,
                          box_height=19 * mm, highlight=[3]))
    story.append(Spacer(1, 6))
    story += bullets([
        "Tasks 2 and 3 are the foundation; every later task draws with sprites and reads named actions.",
        "Tasks 4–7 go in that order because each needs the previous one to be testable (no waves without shooting, "
        "no game over without enemy shots).",
        "Task 9's tests are written alongside 4–7, then completed in task 9.",
    ])
    story.append(P("Decisions to make during M2", "h2"))
    story += bullets([
        "Working title stays 'nameGame' until M4, unless you pick a name sooner.",
        "Sprites stay as text grids in code (D-006) unless you'd rather draw PNGs in an editor (libpng is available).",
        "Fixed 60 Hz update vs variable time step: plan is a fixed 1/60 s step, since the PS3 locks to vsync.",
    ])

    # 8. M3 + M4
    story += section("8. Milestones 3 and 4")
    story.append(P("Milestone 3: the full game", "h2"))
    story.append(table([
        ["#", "Task", "Done when"],
        ["1", "Design notes: bosses, weapon upgrades, screens, high score save, game feel", "all 'designed'"],
        ["2", "Weapon upgrades: 4 levels, upgrade pickups, lose a level on death", "each level visible and distinct"],
        ["3", "Bosses: one boss design scaled per appearance; health bar; attack patterns; big reward",
         "waves 5, 10, 15 bosses beatable but threatening"],
        ["4", "Screens: title (high score, press X/Start), pause menu (resume, options, quit), game over",
         "full loop with no restart of the app"],
        ["5", "High score save: a small file on the PS3 hard drive, written on game over",
         "survives quitting and a PS3 reboot"],
        ["6", "Game feel: shake, hit-stop, flashes, particles, score pop-ups by tier; shake and flash options",
         "hits read clearly; 60 fps holds"],
        ["7", "Playtest, tuning and docs sync", "checker exit 0; M3 marked done"],
    ], [8 * mm, FULL - 8 * mm - 60 * mm, 60 * mm]))
    story.append(P("Milestone 4: release polish", "h2"))
    story.append(table([
        ["#", "Task", "Done when"],
        ["1", "Real game name; XMB icon (ICON0.PNG) and background (PIC1.PNG)", "shows correctly in the Game menu"],
        ["2", "Balancing pass to wave 20 with at least one other player", "difficulty curve tuned and logged"],
        ["3", "Controller unplug pauses; reconnect resumes", "checked on the PS3"],
        ["4", "Stability: a 30-minute run with no frame drops or memory growth", "log shows steady fps and no errors"],
        ["5", "Install guide (README) and a versioned release package", "a fresh player installs from it"],
        ["6", "Final docs sync and work-plan update", "all MVP features 'implemented'"],
    ], [8 * mm, FULL - 8 * mm - 60 * mm, 60 * mm]))

    # 9. Testing
    story += section("9. Testing and quality")
    story.append(table([
        ["Layer", "What is checked", "How"],
        ["Game rules on the Mac", "deadzone maths, rocket buffer, collisions, wave formulas, scoring, extra lives",
         "make test (plain C++, no PS3 needed)"],
        ["Build", "compiles without warnings, package contains EBOOT.BIN and music.ogg", "make ps3"],
        ["On the PS3", "each feature after it lands: buttons, visuals, sound", "make deploy + make log + user check"],
        ["Performance", "60 fps; work under 8 ms per frame; stress test", "fps line in the log every second"],
        ["Freezes and crashes", "main loop stuck or crashed", "watchdog logs where it stopped, then exits to XMB"],
        ["Docs", "statuses agree, links work, numbers match the code", "check_docs.py exit 0 after each change"],
    ], [34 * mm, FULL - 34 * mm - 52 * mm, 52 * mm]))
    story.append(P("Performance budget", "h2"))
    story += bullets([
        "Frame: 16.7 ms at 60 fps. Target game work under 8 ms, leaving room for spikes. M1 measured 1–2 ms.",
        "Stress case to hold at 60 fps: 60 aliens, 100 bullets, 2 explosions, full HUD.",
        "Package size: about 5 MB today (the song is 3.9 MB); sprites and sounds in code add almost nothing.",
    ])
    story.append(P("Playtest checklist for the end of M2", "h2"))
    story += bullets([
        "A first-time player understands moving and shooting within 30 seconds, without explanation.",
        "Every death feels deserved: the shot that killed you was visible.",
        "The rocket feels powerful and the player saved one for a hard moment.",
        "Waves 1–3 feel calm, waves 7–9 feel busy, and the player wants another run.",
        "The music never stutters; sound effects never drown it out.",
    ])

    # 10. Risks
    story += section("10. Risks")
    story.append(table([
        ["Risk", "Impact", "Mitigation"],
        ["Frame rate drops with real sprites, many objects and effects", "High",
         "Stress test after each feature; budget 8 ms; effects are the first thing simplified (pillar 4)"],
        ["SDL port surprises (more no-op functions)", "Medium",
         "We use only the proven minimum: our own pixel buffer and one scaled copy"],
        ["R2, Start and other buttons differ from the source mapping", "Low",
         "Every press is logged; the mapping table is one place in input.cpp"],
        ["Audio stutter when many sounds play over the song", "Medium",
         "Limit channels; short SFX; stress test with sound on"],
        ["PS3 freezes during development", "Medium",
         "Watchdog exits to XMB; never install while a game runs; webMAN restart; hold power ~10 s, never unplug"],
        ["HEN off after a reboot (no FTP/HTTP)", "Low", "make deploy stops with a clear message; re-enable HEN"],
        ["Difficulty wrong (too easy or brutal)", "Medium", "All numbers in one design note each; tune one at a time"],
        ["Scope creep (co-op, remapping, extra modes)", "Medium", "Kept under 'Later' until M4 is done"],
    ], [58 * mm, 18 * mm, FULL - 76 * mm]))

    # 11. Decisions
    story += section("11. Decisions so far")
    story.append(table([
        ["ID", "Decision", "Why"],
        ["D-001", "Original alien enemies; nothing taken from another game", "user's choice; our own characters"],
        ["D-002", "Waves with a boss every 5th wave, endless until game over", "user's choice"],
        ["D-003", "Rockets are ammo (start 3) refilled by pickups; area explosion",
         "user's choice; keeps the rocket scarce"],
        ["D-004", "Logical resolution 320×180", "scales by a whole number to 720p (×4) and 1080p (×6)"],
        ["D-005", "Music as OGG Vorbis", "the PS3's SDL2_mixer plays OGG and WAV only"],
        ["D-006", "Sprites, font and SFX defined in code", "no SDL2_image; tiny pixel art is easy to edit as text"],
        ["D-007", "Single player, players kept in an array with their own controller",
         "co-op later without a rewrite"],
        ["D-008", "Draw into a CPU pixel buffer, then one scaled SDL_RenderCopy",
         "render targets are a no-op and copies have no blending; proven at 60 fps in M1"],
    ], [16 * mm, FULL - 16 * mm - 72 * mm, 72 * mm]))

    # 12. Open questions
    story += section("12. Open questions")
    story.append(P("None of these block M2; each has a default that will be used unless you decide otherwise."))
    story.append(table([
        ["Question", "Default"],
        ["What is the game's real name?", "'nameGame' until M4"],
        ["Sprites as text grids in code, or PNG files drawn in an editor?", "text grids in code (D-006)"],
        ["Should there be a button-remapping menu?", "no; later only if wanted"],
        ["Co-op after M4?", "later; code is ready for a second player"],
        ["Should the song also play on the title screen?", "no (your answer: gameplay only)"],
        ["Are the starting numbers right?", "tuned in M2 playtests; marked [ASSUMPTION] until then"],
    ], [FULL - 70 * mm, 70 * mm]))

    doc = Doc(OUT)
    doc.multiBuild(story)
    print("wrote", OUT)


if __name__ == "__main__":
    build()
