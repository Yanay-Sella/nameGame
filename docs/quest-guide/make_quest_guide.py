"""Builds the NAME QUEST player's guide: docs/quest-guide/NAME-quest-guide.pdf.

Pictures and numbers come from the game's own code (render_quest_guide.cpp writes scenes, sprites and facts.json);
the words are written for players from the spec and the feature docs. The pages are HTML printed to PDF by Chrome.
Needs Pillow, a C++ compiler and Google Chrome. Run from anywhere: python3 docs/quest-guide/make_quest_guide.py
"""
import glob
import html
import json
import os
import subprocess

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
WORK = os.path.join(ROOT, "build", "quest-guide")
RAW = os.path.join(WORK, "raw")
PNG = os.path.join(WORK, "png")
PDF = os.path.join(HERE, "NAME-quest-guide.pdf")
CHROME = "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"
KEY = (0x01, 0xFE, 0x02)

# --- Pictures and facts from the game -------------------------------------------------------------------------------

os.makedirs(RAW, exist_ok=True)
os.makedirs(PNG, exist_ok=True)
for old in glob.glob(os.path.join(RAW, "*")) + glob.glob(os.path.join(PNG, "*")):
    os.remove(old)
subprocess.run(["c++", "-std=c++14", "-O2", "-I" + os.path.join(ROOT, "source"), os.path.join(HERE, "render_quest_guide.cpp")] +
               sorted(glob.glob(os.path.join(ROOT, "source", "quest*.cpp"))) + [os.path.join(ROOT, "source", "screen.cpp"),
               "-o", os.path.join(WORK, "render_quest_guide")], check=True)
subprocess.run([os.path.join(WORK, "render_quest_guide"), RAW], check=True, stdout=subprocess.DEVNULL)
with open(os.path.join(RAW, "facts.json")) as f:
    FACTS = json.load(f)


def png(name, scale):
    """The rendered BMP as a PNG magnified with hard pixels (so Chrome never smooths them); KEY pixels see-through."""
    out = os.path.join(PNG, f"{name}@{scale}.png")
    if not os.path.exists(out):
        image = Image.open(os.path.join(RAW, name + ".bmp")).convert("RGBA")
        if name.startswith("area_"):   # scenery without the HUD's top and bottom bands
            image = image.crop((0, 40, 640, 320))
        image.putdata([(0, 0, 0, 0) if p[:3] == KEY else p for p in image.getdata()])
        image.resize((image.width * scale, image.height * scale), Image.NEAREST).save(out)
    return "file://" + out


def scene(name):
    return png(name, 3)


def sprite(name):
    return png(name, 8)


def esc(text):
    return html.escape(str(text), quote=True)


def fmt(n):
    return f"{n:,}"


# --- What the guide says ---------------------------------------------------------------------------------------------
# Facts from docs/work-plan/name-quest/spec.md and docs/features/name-quest/design.md; numbers from facts.json.

CLASS_FILES = ["knight", "archer", "magician", "thief"]
MENTORS = ["SIR ODO", "WREN", "YSOLDE", "KIT"]
HEROES = [
    {"tag": "Heavy armour, a wide sword.", "colour": "#c8ccdc", "accent": "#ffd84a", "bars": (3, 5, 1),
     "about": "The Knight walks in wearing a helm and carrying a shield, and simply refuses to fall over. The most HP and "
              "DEF of the four, a sword that sweeps a wide box in front, and skills that hit everything close by. "
              "The easiest hero to learn: stand your ground, swing, and roll out of the big attacks.",
     "basic": "Sword: a 22 x 18 box in front of you, live for a moment in each 0.40 s swing. Tap R1 again during a "
              "swing for a three-hit combo: a level slash, a rising cut and an overhead chop.",
     "dodge": "Roll (Circle): a quick roll along the ground with a short moment of invincibility.",
     "skills": ["A big horizontal slash, 30 x 20, for 2.4x damage. 10 MP.",
                "An upward cut over your head that hops you into the air. Great against flyers. 3.0x, 12 MP.",
                "Two shockwaves run 80 dp along the ground both ways. Ground enemies only. 3.4x, 16 MP.",
                "A huge golden arc in front that stuns for half a second. 5.0x, 24 MP."],
     "tip": "Melee hits freeze the action for a blink: that tiny hit-stop tells you the sword connected."},
    {"tag": "Arrows from far away.", "colour": "#9bd468", "accent": "#7bd0ff", "bars": (3, 2, 5),
     "about": "The Archer has a ponytail, a long bow and the best reach in the game: arrows fly 170 dp before they fade. "
              "Fragile up close, deadly at range. Keep moving backwards and let nothing touch you. From the first "
              "promotion on, every arrow pierces one extra enemy.",
     "basic": "Arrow: flies 240 dp/s for 170 dp, one every 0.45 s, stopped by walls. The combo is a standard shot, "
              "a low reload shot and a snap shot.",
     "dodge": "Roll (Circle): a quick roll along the ground with a short moment of invincibility.",
     "skills": ["Two arrows side by side. 1.5x each, 8 MP.",
                "Six arrows rain down on a zone 80 dp ahead of you. 1.6x each, 14 MP.",
                "A fast bolt that flies 300 dp and goes through every enemy in a row. 4.0x, 18 MP.",
                "A fan of five arrows, fired three times in under half a second. 1.8x each, 26 MP."],
     "tip": "Ground enemies are drawn tall on purpose so your arrows always hit them. Flyers need a jump shot."},
    {"tag": "Bolts that hit hard. Fragile.", "colour": "#6f8cff", "accent": "#c890ff", "bars": (5, 1, 4),
     "about": "The Magician wears a wide pointed hat and carries a wand. The strongest attacks and by far the most MP "
              "(it also comes back fastest), but the lowest HP and DEF. Instead of rolling, she blinks forward through "
              "danger, even in the air, leaving a puff of blue mist behind.",
     "basic": "Magic bolt: flies 200 dp/s for 130 dp, one every 0.50 s, for 1.1x damage. The combo is a wand flick, "
              "a rising crescent, then a wide spinning sweep; each cast fires one bolt.",
     "dodge": "Blink (Circle): a short teleport forward that stops before walls. Works in mid-air.",
     "skills": ["A burst 40 dp in front of you, radius 20. 2.6x, 10 MP.",
                "A big snowflake that flies forward and bursts into a ring of ice: hits everything within 32 dp "
                "and slows enemies by half for 3 s (bosses by a fifth). 2.8x, 14 MP.",
                "Lightning jumps to the nearest enemy within 140 dp, then on to three more. 3.0x each, 20 MP.",
                "A meteor falls 120 dp ahead 0.6 s after you cast it. Radius 48. 6.0x, 34 MP."],
     "tip": "Every basic hit gives back 1 MP. A Magician who keeps hitting never runs dry."},
    {"tag": "Fast daggers, quick feet.", "colour": "#a070e0", "accent": "#ff9ad0", "bars": (2, 3, 2),
     "about": "The Thief wears a scarf and a mask and is the fastest hero (walk 80). A very quick dagger, the best "
              "critical hits (10 %, x1.8 damage, +5 % per job) and a double jump: press X again in the air. Hold up "
              "while you press R1 to stab straight up at flyers.",
     "basic": "Dagger: a small 16 x 12 box in front, a new stab every 0.26 s for 0.7x damage. The combo is a "
              "straight stab, a reverse cut and a downward jab.",
     "dodge": "Roll (Circle), plus a double jump: X again in mid-air.",
     "skills": ["Three throwing stars in a small fan. 1.2x each, 8 MP.",
                "Dash 72 dp in 0.15 s, invincible, cutting every enemy you pass. 2.6x, 12 MP.",
                "Spin for half a second: five hits on everything within 30 dp. 1.0x each, 16 MP.",
                "For 12 s a dark copy of you stands behind you and repeats every attack at half damage. 30 MP."],
     "tip": "Shadow Dash makes you invincible: dash THROUGH a boss's charge instead of away from it."},
]
SKILL_SHOTS = {"knight": [2, 1, 1, 1], "archer": [1, 4, 1, 3], "magician": [1, 3, 0, 6], "thief": [1, 2, 3, 6]}

BEHAVIOUR = {
    0: "Walks back and forth and turns at walls and ledges. Touching it hurts.",
    1: "Patrols, and when you are in front of it within 96 dp it shakes and flashes for half a second, then charges "
       "at 160 dp/s until it hits a wall. After a charge it is stunned for 0.8 s and takes 50 % more damage.",
    2: "Flies through walls, bobbing near its home. Within 100 dp it pauses for 0.4 s, then swoops at the spot "
       "where you stood, and floats back.",
    3: "Stands and turns to face you. Within 160 dp it glows for half a second, then fires an aimed shot, every "
       "2.4 s. Its shots stop at walls.",
    4: "Hops in place; when you come within 120 dp it hops toward you every 1.2 s.",
    5: "The elite. It walks toward you, winds up for 0.7 s and swings a big blow in front (30 % harder than its "
       "touch). Its guard blocks three quarters of every hit from the front, except while it winds up or recovers, "
       "or when you hit it from above.",
    6: "Hangs from the ceiling. Walk under it and it drops on you, then walks around like a walker.",
}
KIND = ["fodder", "soldier", "elite"]
TRAITS = {1: " When it lands it throws one banana at you."}

WORLDS = {
    1: {
        "accent": "#ff6a5a", "accent2": "#6f8cff", "dark": "#0b0d1c", "cover": "area_1_00_1", "map_fill": ["#3b4a8a", "#5a4a9a", "#8a3a5a"],
        "tagline": "A dead space station drifting at the edge of the world.",
        "intro": "Deep space, a dim red sun and a ringed planet behind broken station spars. Riveted steel floors with "
                 "hazard stripes, grated catwalks, blinking consoles and red emergency lamps. The crew is gone; their "
                 "names were eaten long ago. What is left is little green crawlers, floating jellies, gunners and the "
                 "Sentinels that still guard the halls.",
        "special": ("Pits open to space", "Fall into a pit and you see stars under your feet: that is open space, and it is "
                    "deadly. Your names stay where you last stood safely."),
        "hazards": [("Pits into space", "Instant death; your names wait as an echo on the last safe ground."),
                    ("Electric panels", "Floor panels that stay dark, flicker for 0.4 s, then arc for 2 s (off 1.5 s, only "
                     "1 s in the hard areas). Touching a lit panel costs 15 % of your max HP."),
                    ("Vents", "Pale streaks rising through a shaft: stand in them and they lift you up.")],
        "path": "DOCKING RING → CARGO HOLD → CREW DECK → down the MAINTENANCE SHAFT → REACTOR RING → up the COOLANT "
                "CONDUIT → HANGAR (the Overseer) → THE BULKHEAD (the gate) → SIGNAL ROOM → MED BAY (second checkpoint) "
                "→ COMMAND RING → THE LAST CORRIDOR (quiet: no enemies) → OBSERVATORY DOME (the Star Mage).",
        "secrets": "The Hangar's lever opens a ladder hatch straight back up to the Docking Ring. The Server Farm and "
                   "Cold Storage make a loop under the Med Bay. Three dead ends hide a Vial Shard, a Map Page and the "
                   "STAR IRON that Brann the Smith wants.",
    },
    2: {
        "accent": "#9bd432", "accent2": "#e0b040", "dark": "#0c1a0c", "cover": "area_2_05_0", "map_fill": ["#3d6a2a", "#5a7a20", "#7a5a20"],
        "tagline": "A drowned jungle swallowing an old temple.",
        "intro": "Green-gold light falls through the canopy onto giant trees and temple pillars in the mist. Mossy soil, "
                 "branch platforms, glowing mushrooms, fireflies and falling leaves. The insects of the jungle hop, "
                 "crawl and charge, monkeys wait in the treetops, and somewhere in a grove a monkey wearing a crown "
                 "is waiting for you.",
        "special": ("Vines and dark water", "Vines are this world's ladders: climb them with up and down. The dark green "
                    "water is deadly, so read every jump over it."),
        "hazards": [("Water pits", "Dark green water: falling in kills you at once."),
                    ("Thorn spikes", "Brown thorns cost 20 % of your max HP and bounce you up."),
                    ("Coconut palms", "Walk under a palm and a coconut falls 0.3 s later (20 % of max HP). The palm grows a "
                     "new one in 4 s. Keep walking: it falls where you were.")],
        "path": "THE EDGE OF GREEN → FERN HOLLOW → THE OLD STEPS → down THE SUN WELL → THE THORN WALK → THE DROWNED "
                "TEMPLE → THE KING'S GROVE (the Monkey King) → THE THORN WALL (the gate) → THE ROOT CELLAR → THE SUNKEN "
                "SHRINE (second checkpoint) → PALM TERRACES → THE STILL ROOTS (quiet) → THE SILVER THRONE (the Silverback).",
        "secrets": "Garrick, Tobin's lost father, sits hurt beside a fallen statue's head in THE FALLEN FACE. Bring him "
                   "a RED POTION. A lever in THE ROOT HOARD opens a hatch back up to the Sunken Shrine.",
    },
    3: {
        "accent": "#9fe0ff", "accent2": "#4a9ad8", "dark": "#08142a", "cover": "area_3_05_0", "map_fill": ["#3a6a9a", "#2a5080", "#1a3060"],
        "tagline": "A blue cave under a glacier where even the wind whispers.",
        "intro": "A dim blue cave under the ice: far ice mountains, a frozen waterfall, crystals and frozen trees, snow "
                 "falling at three depths. Snow-capped blue rock, ice-slab platforms and frozen rope ladders. Living "
                 "snowflakes swoop, ice bats hang from the ceiling, and an ice cathedral hides the feather that "
                 "makes heroes into masters.",
        "special": ("Ice and wind", "Ice floors are slippery: you keep sliding after you let go. Late areas blow wind "
                    "that turns around every 3 s; the snow streaks show which way."),
        "hazards": [("Freezing water", "Deadly, like every liquid."),
                    ("Crumbling ice", "Shakes for 0.6 s under you with spreading cracks, breaks, and grows back 4 s after "
                     "you leave it."),
                    ("Icicles", "Fall 0.25 s after you pass under them (20 % of max HP) and stay gone until you rest."),
                    ("Wind", "Pushes 40 dp/s and turns around every 3 s.")],
        "path": "THE COLD MOUTH → THE LONG DRIFT → THE FROZEN STAIR → down THE GLACIER WELL → THE ICE GALLERY → THE "
                "ICE CATHEDRAL → THE YETI'S DEN (the Yeti) → THE ICE WALL (the gate) → THE BLUE HOLLOW → THE DEEP "
                "CHAPEL (second checkpoint) → THE WIND GATE → THE STILL ICE (quiet) → down the ladder to THE GLACIER "
                "THRONE (the Titan).",
        "secrets": "The FROZEN FEATHER waits in THE FEATHER VAULT above the Ice Cathedral, behind an Ice Knight. Your "
                   "mentor needs it for your second job. A lever in THE LEVER CAVE opens a way back to the Deep Chapel.",
    },
}

BOSSES = {
    0: {"sprite": 0, "title": "The Overseer", "where": "HANGAR", "tag": "The station's old guardian, still on patrol.",
        "about": "A green saucer-shaped machine with a single blue dome. It hovers above the hangar floor, sweeping from "
                 "wall to wall, and its centre is just within a good jump's reach.",
        "p1": [("Burst", "Three shots aimed at you, 0.15 s apart, after a 0.5 s tell.", "Keep walking: aimed shots go where you were."),
               ("Fan", "Five shots straight down in a fan, after a 0.5 s tell.", "Stand between two shots, or roll through."),
               ("Slam", "Its shadow marks the floor for 0.8 s, then it drops like a stone, sits there for 1.5 s, and rises.",
                "Step out of the shadow, then hit it as hard as you can while it sits: this is your best chance.")],
        "p2": "At half HP it roars and sweeps 30 % faster, and adds SUMMON: two Floaters (at most four at once).",
        "reward": "Its death opens the gate in THE BULKHEAD.", "shots": [("fight_0_07", "A BURST of aimed shots."), ("fight_0_38", "Phase 2: Floaters join the fight.")]},
    1: {"sprite": 1, "title": "The Star Mage", "where": "OBSERVATORY DOME", "tag": "The wizard on the roof of the world.",
        "about": "A small green wizard in a tall blue hat with a star-tipped wand. Before every attack it fades out and "
                 "appears on one of four spots on the dome's platforms.",
        "p1": [("Beam", "A ball grows at the wand's star for 1.2 s, with the column marked below it. Then a 24 dp wide "
                "beam burns down that column for 0.8 s. A big attack.", "The moment you see the ball, leave its column."),
               ("Star Fan", "Seven shots spread over 120 degrees.", "Find a gap, or roll through it."),
               ("Summon", "Two Crawlers (at most four).", "Clear them quickly so you can watch the mage.")],
        "p2": "Phase 2: the Beam fires in two columns at once, 64 dp apart, both marked. And every 8 s the middle six "
              "tiles of the floor flicker for 0.6 s and then electrify for 1.5 s.",
        "reward": "Its death gives the first Great Name and lights the Greendeep's entry.",
        "shots": [("fight_1_08", "BEAM: the column burns."), ("fight_1_38", "Phase 2: two beams at once.")]},
    2: {"sprite": 2, "title": "The Monkey King", "where": "THE KING'S GROVE", "tag": "Wears a crown. Throws fruit. Very proud.",
        "about": "A brown monkey with a little golden crown. It leaps between two trees in its grove (you can climb up "
                 "to it) and throws whatever fruit is at hand.",
        "p1": [("Banana Arc", "Three bananas lobbed at you.", "Watch where they will land and step away."),
               ("Peel Toss", "Two banana peels land on the floor and stay for 6 s. Step on one and you slide for half a "
                "second with no control.", "Remember where the peels are, and jump over them."),
               ("Leap", "Between attacks it leaps to the other tree in a 0.8 s arc; its landing spot is marked for 0.5 s.",
                "Don't stand under the landing mark.")],
        "p2": "Phase 2 adds COCONUT RAIN: five coconuts marked on the ceiling for 0.8 s, then they fall.",
        "reward": "Its death opens the gate of thorny roots in THE THORN WALL.",
        "shots": [("fight_2_14", "BANANA ARC on its way."), ("fight_2_34", "Phase 2: coconuts marked on the ceiling.")]},
    3: {"sprite": 3, "title": "The Silverback", "where": "THE SILVER THRONE", "tag": "The angry king of the jungle floor.",
        "about": "A huge grey-brown gorilla with a silver back and red eyes. It shakes the floor, jumps on you, and its "
                 "stink is a weapon too.",
        "p1": [("Pound", "It pounds the ground for 0.6 s, then a 12 dp tall shockwave runs along the floor both ways.",
                "Jump the wave."),
               ("Leap", "A shadow appears under you for 0.7 s, then it lands right there and shakes the screen.",
                "Move the moment you see the shadow."),
               ("Stink", "It throws poop that leaves a stink mist 40 dp wide for 6 s. Inside it you lose 3 % of your "
                "max HP every second.", "Fight from outside the cloud.")],
        "p2": "Phase 2: it roars and calls two Stag beetles, and every Pound sends two waves, 0.4 s apart.",
        "reward": "Its death gives the second Great Name and lights the White Silence's entry.",
        "shots": [("fight_3_13", "LEAP: here it comes."), ("fight_3_40", "Phase 2: stink mist, Stags and two waves.")]},
    4: {"sprite": 4, "title": "The Yeti", "where": "THE YETI'S DEN", "tag": "Big, white, furry, and it has a cold.",
        "about": "A great white yeti with a blue face. It slides around its den on its belly, throws ice blocks and "
                 "sneezes terribly.",
        "p1": [("Sneeze", "\"AH... AH...\" for half a second, then seven snowballs in a 100 degree fan.",
                "When you read AH..., move to a gap."),
               ("Belly Slide", "It crouches for 0.6 s, then slides across the den at 180 dp/s until it hits a wall. A big attack.",
                "Jump over it, or roll through."),
               ("Ice Block", "Throws a block of ice. If it hits you, you are frozen for 1.2 s but take no damage.",
                "Mash X or R1: every press chips 0.15 s off the ice.")],
        "p2": "Phase 2: it sneezes twice in a row, and snow starts falling in the den.",
        "reward": "Its death opens the wall of blue ice in THE ICE WALL.",
        "shots": [("fight_4_06", "A SNEEZE fan of snowballs."), ("fight_4_27", "An ICE BLOCK on its way.")]},
    5: {"sprite": 5, "title": "The Glacier Titan", "where": "THE GLACIER THRONE", "tag": "A walking mountain of ice.",
        "about": "A giant made of blue ice blocks with a glowing heart. It sits at the bottom of the glacier, and the ice "
                 "itself fights for it.",
        "p1": [("Icicle Rain", "Six icicles are marked on the ceiling for 0.8 s, 32 dp apart around you, then they fall.",
                "Stand between two marks."),
               ("Volley", "Three aimed ice shards.", "Keep moving."),
               ("Spike Line", "It slams the floor; cracks run along the ground, and 16 dp tall ice spikes burst up after "
                "them, from the Titan outward. A big attack.", "Jump over the line of spikes, or be far away.")],
        "p2": "Phase 2: the middle ten tiles of its floor turn into crumbling ice over a void, and a blizzard pushes you "
              "at 40 dp/s, turning around every 4 s.",
        "reward": "Its death gives the third Great Name.",
        "shots": [("fight_5_30", "SPIKE LINE racing across the floor."), ("fight_5_15", "A VOLLEY of ice shards.")]},
}
WORLD_BOSSES = {1: (0, 1), 2: (2, 3), 3: (4, 5)}

NAMULA_PEOPLE = [
    (0, "Sits by the Hearth of Names. Give her the names you carry and you level up. She is a little lost too."),
    (1, "Sells potions and buffs. Prices rise with the furthest world you have reached."),
    (2, "Sells your class's next weapon and the next armour, once you have reached that tier's world. Bring him STAR "
        "IRON to refine your weapon."),
    (3, "Sings one song about the latest Great Name you took."),
    (4, "The wind stole his maps of the five worlds. Map pages lie in dead ends of each world."),
    (5, "By the well. Says only \"...\" until you are a lot stronger."),
    (6, "The Knight's mentor. Your jobs come from your own class's mentor."),
    (7, "The Archer's mentor."),
    (8, "The Magician's mentor."),
    (9, "The Thief's mentor."),
    (10, "A statue by the Sky Stair. It counts the Great Names you hold."),
    (11, "On the shore. His father went into the Greendeep and never came back."),
    (12, "Tobin's father. Hurt somewhere in the Greendeep; later back home on the shore."),
    (13, "Arrives on the shore after a Great Name of the far north. She has a letter to deliver."),
]

# --- HTML ------------------------------------------------------------------------------------------------------------

CSS = """
@page { size: A4; margin: 0; }
* { box-sizing: border-box; }
html, body { margin: 0; padding: 0; }
body { font-family: 'Avenir Next', 'Helvetica Neue', sans-serif; color: #1d1d28; -webkit-print-color-adjust: exact; print-color-adjust: exact; }
.page { width: 210mm; height: 297mm; position: relative; overflow: hidden; page-break-after: always; break-after: page;
        background-color: #0c0f1e; background-size: cover; background-position: center; image-rendering: pixelated; }
.inner { position: absolute; left: 12mm; right: 12mm; top: 11mm; bottom: 13mm; display: flex; flex-direction: column; gap: 4mm; }
h1, h2, h3, .big { font-family: 'Futura', 'Avenir Next', sans-serif; font-weight: 800; margin: 0; }
h2 { font-size: 21pt; color: white; text-shadow: 0 1mm 0 rgba(0,0,0,.45); letter-spacing: .5px; }
h3 { font-size: 12.5pt; margin: 0 0 1.8mm 0; }
p { margin: 0 0 1.8mm 0; font-size: 10pt; line-height: 1.4; }
.card { background: rgba(255,255,255,.94); border-radius: 4mm; padding: 4mm 5mm; box-shadow: 0 1.5mm 4mm rgba(0,0,0,.35); }
.card.dark { background: rgba(10,12,26,.8); color: #eef; border: .6mm solid rgba(255,255,255,.18); }
.row { display: flex; gap: 4mm; }
.col { flex: 1; display: flex; flex-direction: column; gap: 4mm; min-width: 0; }
.shot { width: 100%; image-rendering: pixelated; border-radius: 2.5mm; display: block; border: .7mm solid rgba(255,255,255,.85); }
.cap { font-size: 8.5pt; color: #eef; margin-top: 1mm; font-style: italic; text-shadow: 0 .4mm 1mm #000; }
.sprite { image-rendering: pixelated; }
table { border-collapse: collapse; width: 100%; font-size: 9pt; }
th { font-family: 'Futura'; font-weight: 700; text-align: left; padding: 1.3mm 1.8mm; color: #111; }
td { padding: 1.2mm 1.8mm; border-top: .3mm solid rgba(0,0,0,.1); vertical-align: middle; }
tr:nth-child(even) td { background: rgba(0,0,0,.035); }
.footer { position: absolute; bottom: 4.5mm; left: 0; right: 0; text-align: center; font-family: 'Futura'; font-size: 8pt;
          color: rgba(255,255,255,.7); letter-spacing: 2px; }
.cover .inner { justify-content: flex-end; }
.chapter-no { font-family: 'Futura'; font-weight: 700; letter-spacing: 4px; font-size: 11pt; opacity: .9; }
.cover h1 { font-size: 44pt; line-height: 1; color: white; text-shadow: 0 1.5mm 0 rgba(0,0,0,.5); }
.tagline { font-size: 14pt; color: white; font-style: italic; margin-top: 2mm; text-shadow: 0 .5mm 1mm #000; }
.parade { display: flex; justify-content: center; align-items: flex-end; gap: 7mm; margin-bottom: 3mm; }
.parade img { filter: drop-shadow(0 1.2mm 0 rgba(0,0,0,.6)); image-rendering: pixelated; }
.enemy { display: flex; align-items: center; gap: 4mm; padding: 2.2mm 0; border-top: .3mm solid rgba(0,0,0,.08); }
.enemy:first-of-type { border-top: none; }
.poses { flex: none; width: 46mm; display: flex; align-items: flex-end; justify-content: center; gap: 1.5mm; }
.poses img { image-rendering: pixelated; }
.name { font-family: 'Futura'; font-weight: 700; font-size: 11.5pt; }
.meta { font-size: 8.5pt; color: #555; margin: .5mm 0 .8mm 0; }
.chip { display: inline-block; font-family: 'Futura'; font-weight: 700; font-size: 7.5pt; padding: .4mm 2mm; border-radius: 5mm;
        color: #111; margin-right: 1mm; letter-spacing: .5px; }
.move { display: flex; gap: 3mm; padding: 2.2mm 0; border-top: .3mm solid rgba(0,0,0,.08); }
.move:first-of-type { border-top: none; }
.move .tag { flex: none; width: 27mm; font-family: 'Futura'; font-weight: 700; font-size: 11pt; }
.move .what { font-size: 9.8pt; line-height: 1.38; }
.move .tip { font-size: 9.2pt; color: #2a6a2a; margin-top: .6mm; }
.move .tip::before { content: "TIP  "; font-family: 'Futura'; font-weight: 700; letter-spacing: 1px; }
ul { margin: 0; padding-left: 4.5mm; } li { font-size: 9.6pt; line-height: 1.38; margin-bottom: 1mm; }
.bars span { display: inline-block; width: 4mm; height: 2.6mm; border-radius: .8mm; margin-right: .6mm; background: #ddd; }
.atlas { display: grid; grid-template-columns: repeat(4, 1fr); gap: 2.6mm; }
.atlas .tile { background: rgba(10,12,26,.82); border-radius: 2mm; padding: 1.2mm; color: #eef; }
.atlas img { width: 100%; display: block; border-radius: 1.2mm; image-rendering: pixelated; }
.atlas .t { font-family: 'Futura'; font-weight: 700; font-size: 7.6pt; margin-top: .8mm; white-space: nowrap; overflow: hidden; }
.atlas .n { font-size: 6.8pt; color: #b8bdd8; white-space: nowrap; overflow: hidden; }
.person { display: flex; align-items: center; gap: 3mm; padding: .9mm 0; border-top: .3mm solid rgba(0,0,0,.08); }
.person img { width: 10mm; flex: none; image-rendering: pixelated; }
.person .pn { font-family: 'Futura'; font-weight: 700; font-size: 9.5pt; }
.person .pd { font-size: 8.6pt; line-height: 1.3; color: #333; }
.statgrid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 2mm; }
.stat { background: rgba(0,0,0,.05); border-radius: 2mm; padding: 1.5mm 2mm; }
.stat .k { font-size: 7.5pt; color: #666; letter-spacing: 1px; font-family: 'Futura'; }
.stat .v { font-family: 'Futura'; font-weight: 800; font-size: 13pt; }
"""


def footer(text):
    return f'<div class="footer">{esc(text)}</div>'


def picture(name, caption, style=""):
    return f'<div style="{style}"><img class="shot" src="{scene(name)}"><div class="cap">{esc(caption)}</div></div>'


def bg(name, top, bottom):
    return (f"background-image: linear-gradient(rgba(5,6,14,{top}), rgba(5,6,14,{bottom})), url('{scene(name)}');")


def img(name, height_mm, extra=""):
    return f'<img class="sprite" src="{sprite(name)}" style="height:{height_mm}mm;{extra}">'


def bars(n, colour):
    return '<span class="bars">' + "".join(f'<span style="background:{colour if i < n else "#ddd"}"></span>' for i in range(5)) + "</span>"


def cover_page():
    heroes = "".join(img(f"hero_{c}_3", 34) for c in CLASS_FILES)
    return f"""
<section class="page cover" style="{bg('area_0_00_0', 0.0, 0.55)}">
  <div class="inner">
    <div class="parade">{heroes}</div>
    <div class="card dark">
      <div class="chapter-no" style="color:#ffb050">NAME GAME · THE PLAYER'S GUIDE</div>
      <h1>NAME QUEST</h1>
      <div class="tagline">Every thing has a name, and its name keeps it alive.</div>
      <p style="margin-top:4mm;font-size:11pt;color:#eef">The heroes, the worlds and their maps, every enemy and every boss
      of NAME QUEST, the action RPG on NAME game's main menu. This edition covers Namula and the first three worlds:
      The Drifting Station, The Greendeep and The White Silence.</p>
    </div>
  </div>
  {footer('NAME QUEST · PLAYER GUIDE')}
</section>"""


def story_page():
    lc = FACTS["levelCost"]
    return f"""
<section class="page" style="{bg('area_0_01_1', 0.45, 0.8)}">
  <div class="inner">
    <h2>The story so far</h2>
    <div class="card dark"><p style="font-size:11pt">Long ago, every thing had a name, and every name kept its thing alive.
      Then came <b>the Unnaming</b>: a grey hunger that eats names. What loses its name, fades. Monsters are full of
      stolen names. One fire still burns: <b>the Hearth of Names</b>, in <b>Namula</b>, the last village, on a cliff at
      the edge of the world. You wake beside it. You do not remember your name.</p>
      <p style="font-size:11pt">Somewhere above the clouds, the <b>Name-Eater</b> rules the Nameless Citadel. Each world's
      great boss holds a <b>Great Name</b>; five of them open the Sky Stair behind Namula.</p></div>
    <div class="row">
      <div class="col"><div class="card"><h3>Names are your money</h3>
        <p>Every monster you beat frees the names it stole, and you carry them. Ilsa turns them into levels by the hearth:
        level 2 costs {lc[0]} names, level 5 costs {lc[3]}, level 10 costs {fmt(lc[8])}, and each level costs 20 % more
        than the one before. Marra and Brann take names too.</p></div>
        <div class="card"><h3>When you fade</h3>
        <p>HP 0, a pit or a deadly liquid: <b>YOU FADED</b>. You wake at the last checkpoint where you rested, with full
        HP and MP. Every name you carried stays behind as a pale-yellow wisp, the <b>name echo</b>, on the last safe
        ground you stood on. Touch it to get them all back. Fade again first and the old echo is gone for good.</p></div>
      </div>
      <div class="col"><div class="card"><h3>Checkpoints</h3>
        <p>Stone braziers. Touch a cold one to light it (<b>NAME KINDLED</b>, and the game saves). Press up at a lit one:
        <b>REST</b> fills HP and MP and brings ordinary monsters back; <b>TRAVEL</b> takes you to any lit checkpoint in any
        world. Each world has one at its entry, one half way, and one that appears in the great boss's arena.</p></div>
        <div class="card"><h3>Gates, bosses and HP</h3>
        <p>Half way through every world a gate blocks the path: beat the world's <b>mini-boss</b> to open it. At the end
        waits the <b>great boss</b>. Bosses stay beaten. HP never comes back by itself: only potions, resting and
        levelling heal you. MP refills slowly, and every basic hit that lands gives 1 MP.</p></div>
      </div>
    </div>
  </div>
  {footer('NAME QUEST · PLAYER GUIDE')}
</section>"""


def controls_page():
    rows = [("Left stick / D-pad", "Walk. Up/down climbs ladders and vines. Down + X drops through a thin platform."),
            ("X", "Jump; hold it for a higher jump. Thief: X again in the air for a double jump."),
            ("R1", "Basic attack. Tap again during a strike for a three-hit combo."),
            ("R2", "Strong attack (costs MP)."),
            ("R3", "Switch between the strong attacks you have unlocked."),
            ("Circle", "Dodge: roll (Magician: blink). A moment of invincibility."),
            ("Square", "Use the selected item (potions take half a second to drink)."),
            ("Triangle", "Select the next item you own."),
            ("Up", "Talk, open a chest, use a checkpoint, a portal or a surveyor's map."),
            ("Start", "Pause: resume, settings, or return to the menu (the game saves).")]
    table = "".join(f"<tr><td style='font-family:Futura;font-weight:700;white-space:nowrap'>{esc(k)}</td><td>{esc(v)}</td></tr>" for k, v in rows)
    items = [("RED POTION", "Heals 35 % of max HP. You start with 5."), ("BIG POTION", "Heals 70 % of max HP."),
             ("MANA POTION", "Restores half your MP."), ("WHETSTONE", "ATK x1.2 for 120 s."),
             ("IRON TONIC", "Damage taken x0.8 for 120 s."), ("SWIFT FEATHER", "Walk x1.15 for 120 s.")]
    itable = "".join(f"<tr><td style='font-weight:700;white-space:nowrap'>{esc(k)}</td><td>{esc(v)}</td></tr>" for k, v in items)
    return f"""
<section class="page" style="{bg('area_1_02_0', 0.45, 0.8)}">
  <div class="inner">
    <h2>How to play</h2>
    <div class="card" style="padding:3mm 5mm"><table><tr style="background:#ffb050"><th>Button</th><th>In play</th></tr>{table}</table></div>
    <div class="row">
      <div class="col"><div class="card"><h3>Items (Marra's shop)</h3><table>{itable}</table>
        <p style="margin-top:2mm;font-size:8.8pt">Buffs end when you fade. Vial Shards hidden in each world let you carry one
        more Red Potion.</p></div></div>
      <div class="col"><div class="card"><h3>Seven golden rules</h3><ul>
        <li><b>Every boss attack is announced.</b> A pose, a flash, a shadow or a marked line comes first. Learn the tells.</li>
        <li><b>Magenta hurts.</b> Enemy shots and boss markers are magenta.</li>
        <li><b>After a big attack the boss is open</b> for at least 0.6 s. That is when you hit.</li>
        <li><b>Rest before a boss arena.</b> Walking into the arena closes every exit until someone falls.</li>
        <li><b>Bank your names</b> with Ilsa before a risky area.</li>
        <li><b>Surveyors</b> (a telescope and a satchel of maps) are the only way to see a world's map: two per world.</li>
        <li><b>The elite's guard</b> blocks hits from the front. Hit it while it winds up, or jump on it.</li></ul></div></div>
    </div>
  </div>
  {footer('NAME QUEST · PLAYER GUIDE')}
</section>"""


def heroes_overview_page():
    cards = ""
    for c, hero in enumerate(HEROES):
        k = FACTS["classes"][c]
        cards += f"""
      <div class="card" style="display:flex;gap:4mm;align-items:center">
        <div style="flex:none;width:30mm;text-align:center">{img(f'hero_{CLASS_FILES[c]}_0', 30)}</div>
        <div style="flex:1"><div class="name" style="font-size:15pt">{k['name']}</div>
          <div class="meta">{esc(hero['tag'])} &nbsp;Mentor: {MENTORS[c]}</div>
          <table style="font-size:8.6pt;width:auto"><tr><td>POWER</td><td>{bars(hero['bars'][0], hero['accent'])}</td>
            <td>HP</td><td>{int(k['hp'][0])} (+{k['hp'][1]:g}/lv)</td><td>ATK</td><td>{k['atk'][0]:g} (+{k['atk'][1]:g})</td></tr>
            <tr><td>TOUGHNESS</td><td>{bars(hero['bars'][1], hero['accent'])}</td>
            <td>MP</td><td>{int(k['mp'][0])} (+{k['mp'][1]:g}/lv)</td><td>DEF</td><td>{k['def'][0]:g} (+{k['def'][1]:g})</td></tr>
            <tr><td>REACH</td><td>{bars(hero['bars'][2], hero['accent'])}</td>
            <td>WALK</td><td>{k['walk']:g}</td><td>CRIT</td><td>{k['crit'] * 100:g} % x{k['critMult']:g}</td></tr></table>
          <div class="meta" style="margin-top:1.2mm">Jobs: {' → '.join(k['jobs'])}</div></div>
      </div>"""
    return f"""
<section class="page" style="{bg('area_0_00_1', 0.5, 0.82)}">
  <div class="inner">
    <h2>Choose your hero</h2>
    <div class="card dark"><p>Pick one of four classes when you start a save slot (three slots). You keep it for the
      whole game, but you grow: your class's mentor on Namula's cliffs promotes you to a new <b>job</b> at level 5 (once
      world 1's gate is open), at level 15 (bring the FROZEN FEATHER from the White Silence) and at level 30. Each job
      heals you fully, adds a trim colour to your outfit, unlocks a new strong attack and makes your basic attack faster,
      bigger and harder (x1.0, 1.15, 1.3, 1.5). Earlier skills stay: R3 switches between them.</p></div>
    {cards}
  </div>
  {footer('NAME QUEST · HEROES')}
</section>"""


def hero_page(c):
    hero, k, f = HEROES[c], FACTS["classes"][c], CLASS_FILES[c]
    trims = ["#c8c0a0", "#6f9cff", "#ffd84a", "#ff6a5a"]
    tiers = "".join(f"<div style='text-align:center'>{img(f'hero_{f}_{j}', 26)}<div class='meta' style='color:#eef'>"
                    f"<span class='chip' style='background:{trims[j]}'>{'LV 1' if j == 0 else ['', 'LV 5', 'LV 15', 'LV 30'][j]}</span>"
                    f"<br><b style='font-family:Futura;font-size:10pt;color:white'>{k['jobs'][j]}</b></div></div>" for j in range(4))
    skills = ""
    for j in range(4):
        skills += f"""
        <div style="display:flex;gap:2.5mm;align-items:flex-start">
          <img class="shot" style="width:46mm;flex:none;border-color:{trims[j]}" src="{scene(f'skill_{f}_{j}_{SKILL_SHOTS[f][j]}')}">
          <div><div class="name" style="font-size:10.5pt">{esc(k['skills'][j])}</div><div class="meta">{k['jobs'][j]} · R2</div>
          <div style="font-size:8.6pt;line-height:1.3">{esc(hero['skills'][j])}</div></div></div>"""
    weapons = ", ".join(" ".join(word.capitalize() for word in name.split()) for name in k["weapons"][:6])
    return f"""
<section class="page" style="background:linear-gradient(160deg, #141833, #0a0b16)">
  <div class="inner">
    <div style="display:flex;justify-content:space-between;align-items:flex-end">
      <div><div class="chapter-no" style="color:{hero['accent']}">HERO · MENTOR {MENTORS[c]}</div><h2 style="font-size:30pt">{k['name']}</h2></div>
      <div class="tagline" style="font-size:12pt">{esc(hero['tag'])}</div></div>
    <div class="card dark" style="display:flex;justify-content:space-around;align-items:flex-end;padding:3mm">{tiers}</div>
    <div class="row">
      <div class="col" style="flex:1.05"><div class="card"><p>{esc(hero['about'])}</p>
        <h3 style="margin-top:2mm">Basic attack (R1)</h3><p>{esc(hero['basic'])}</p>
        <h3>Dodge</h3><p>{esc(hero['dodge'])}</p>
        <h3>Hint</h3><p>{esc(hero['tip'])}</p></div></div>
      <div class="col" style="flex:.95"><div class="card"><h3>Stats</h3><div class="statgrid">
        <div class="stat"><div class="k">HP</div><div class="v">{int(k['hp'][0])}</div><div class="meta">+{k['hp'][1]:g} per level</div></div>
        <div class="stat"><div class="k">MP</div><div class="v">{int(k['mp'][0])}</div><div class="meta">+{k['mp'][1]:g} per level</div></div>
        <div class="stat"><div class="k">MP / S</div><div class="v">{k['mpRegen']:g}</div><div class="meta">+0.05 per level</div></div>
        <div class="stat"><div class="k">ATK</div><div class="v">{k['atk'][0]:g}</div><div class="meta">+{k['atk'][1]:g} per level</div></div>
        <div class="stat"><div class="k">DEF</div><div class="v">{k['def'][0]:g}</div><div class="meta">+{k['def'][1]:g} per level</div></div>
        <div class="stat"><div class="k">WALK</div><div class="v">{k['walk']:g}</div><div class="meta">dp per second</div></div></div>
        <p style="margin-top:2mm;font-size:8.8pt">Each job adds 10 % HP and ATK. Weapons from Brann: {esc(weapons)}.</p></div></div>
    </div>
    <div class="card" style="padding:3mm 4mm"><h3>Strong attacks</h3>
      <div style="display:grid;grid-template-columns:1fr 1fr;gap:3mm">{skills}</div></div>
  </div>
  {footer('NAME QUEST · HEROES · ' + k['name'])}
</section>"""


def namula_page():
    people = ""
    for npc, text in NAMULA_PEOPLE:
        people += (f"<div class='person'><img src='{sprite(f'npc_{npc}')}'><div><div class='pn'>{esc(FACTS['npcs'][npc])}</div>"
                   f"<div class='pd'>{esc(text)}</div></div></div>")
    half = (len(NAMULA_PEOPLE) + 1) // 2
    split = people.split("<div class='person'>")
    left = "<div class='person'>".join([""] + split[1:half + 1])
    right = "<div class='person'>".join([""] + split[half + 1:])
    return f"""
<section class="page" style="background:linear-gradient(170deg, #2a1a10, #120a08)">
  <div class="inner">
    <div><div class="chapter-no" style="color:#ffb050">THE HUB · WORLD 0</div><h2 style="font-size:30pt">Namula</h2></div>
    <div class="card dark"><p>The last village, on a cliff at the edge of the world: warm stone, lanterns and firelight. No
      monsters, no traps; only the sea on the shore can hurt you. Three areas side by side: <b>NAMULA SHORE</b> on the left,
      the village <b>NAMULA</b> with the hearth in the middle, and <b>NAMULA CLIFFS</b> on the right, where the mentors stand and
      the Sky Stair waits. The hearth is your first checkpoint and is always lit.</p></div>
    <div class="row">{picture('area_0_02_0', 'Namula Shore: Tobin, the hut and the closed portals.')}{picture('area_0_00_0', 'Namula: the Hearth of Names, Ilsa, Marra, Brann and Lio.')}</div>
    <div class="row">{picture('area_0_01_0', 'Namula Cliffs: the four mentors, Pell and Sela by the well.')}{picture('area_0_01_1', 'The Gatekeeper and the Sky Stair, still closed.')}</div>
    <div class="card" style="padding:2.5mm 5mm"><div class="row"><div class="col" style="gap:0">{left}</div><div class="col" style="gap:0">{right}</div></div></div>
  </div>
  {footer('NAME QUEST · NAMULA')}
</section>"""


def world_cover(w):
    W, wd = WORLDS[w], FACTS["worlds"][w]
    mini, big = WORLD_BOSSES[w]
    parade = img(f"boss_{mini}_idle", 26) + img(f"boss_{big}_idle", 36)
    return f"""
<section class="page cover" style="{bg(W['cover'], 0.0, 0.5)}">
  <div class="inner">
    <div class="parade">{parade}</div>
    <div class="card dark">
      <div class="chapter-no" style="color:{W['accent']}">WORLD {w}</div>
      <h1>{esc(wd['name'].title())}</h1>
      <div class="tagline">{esc(W['tagline'])}</div>
      <p style="margin-top:4mm;font-size:11pt;color:#eef">{esc(W['intro'])}</p>
      <p style="font-size:10pt;color:{W['accent']}"><b>{esc(W['special'][0])}.</b> <span style="color:#eef">{esc(W['special'][1])}</span></p>
    </div>
  </div>
  {footer('NAME QUEST · WORLD ' + str(w))}
</section>"""


def area_tags(w, i, a):
    t = a["tiles"]
    tags = []
    if a["flags"] & 1:
        tags.append(("MINI-BOSS", "#c060ff"))
    if a["flags"] & 2:
        tags.append(("BOSS", "#ff3a3a"))
    if t["C"] or t["b"]:
        tags.append(("CHECKPOINT", "#ff8a2a"))
    if t["g"]:
        tags.append(("GATE", "#c0c0c0"))
    if t["M"]:
        tags.append(("SURVEYOR", "#6fe0c0"))
    if t["T"]:
        tags.append(("CHEST", "#ffe14a"))
    if t["K"]:
        tags.append(("LEVER", "#a0a8c0"))
    if a["npcs"]:
        tags.append(("PERSON", "#ff9ad0"))
    return tags


def map_svg(w):
    W, areas = WORLDS[w], FACTS["worlds"][w]["areas"]
    cw, ch = 17, 11      # mm per map cell
    cols = max(a["mapX"] + (2 if a["wide"] else 1) for a in areas)
    top = min(a["mapY"] for a in areas)
    rows = max(a["mapY"] + (1 if a["wide"] else 2) for a in areas) - top

    def box(a):
        x, y = a["mapX"] * cw, (a["mapY"] - top) * ch
        return x, y, (2 if a["wide"] else 1) * cw, (1 if a["wide"] else 2) * ch

    def centre(a):
        x, y, bw, bh = box(a)
        return x + bw / 2, y + bh / 2
    lines = ""
    for i, a in enumerate(areas):
        for t in a["links"]:
            if t > i:
                (x1, y1), (x2, y2) = centre(a), centre(areas[t])
                lines += f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="#fff" stroke-width="1.2" opacity=".75"/>'
        for t in a["portals"]:
            if t > i or (t >= 0 and i not in areas[t]["portals"]):
                (x1, y1), (x2, y2) = centre(a), centre(areas[t])
                lines += (f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="#d08aff" stroke-width=".9" '
                          f'stroke-dasharray="1.6 1.2"/>')
    boxes = ""
    for i, a in enumerate(areas):
        x, y, bw, bh = box(a)
        fill = W["map_fill"][a["band"]]
        boxes += f'<rect x="{x + .8}" y="{y + .8}" width="{bw - 1.6}" height="{bh - 1.6}" rx="1.4" fill="{fill}" stroke="#fff" stroke-width=".35"/>'
        boxes += f'<text x="{x + 2}" y="{y + 4.6}" font-family="Futura" font-weight="700" font-size="3.6" fill="#fff">{i}</text>'
        for k, (label, colour) in enumerate(area_tags(w, i, a)):
            boxes += f'<circle cx="{x + bw - 3 - k * 3.2}" cy="{y + bh - 3}" r="1.25" fill="{colour}" stroke="#000" stroke-width=".25"/>'
    return (f'<svg viewBox="-1 -1 {cols * cw + 2} {rows * ch + 2}" style="width:100%;height:auto;display:block">'
            f'{lines}{boxes}</svg>')


def map_page(w):
    W, wd = WORLDS[w], FACTS["worlds"][w]
    legend_items = [("CHECKPOINT", "#ff8a2a"), ("MINI-BOSS", "#c060ff"), ("BOSS", "#ff3a3a"), ("GATE", "#c0c0c0"),
                    ("SURVEYOR", "#6fe0c0"), ("CHEST", "#ffe14a"), ("LEVER", "#a0a8c0"), ("PERSON", "#ff9ad0")]
    legend = "".join(f"<span style='margin-right:3mm;white-space:nowrap'><span style='display:inline-block;width:2.6mm;height:2.6mm;"
                     f"border-radius:50%;background:{c};border:.2mm solid #000;vertical-align:middle'></span> {l}</span>" for l, c in legend_items)
    bands = "".join(f"<span style='margin-right:3mm'><span style='display:inline-block;width:4mm;height:2.6mm;border-radius:.6mm;"
                    f"background:{c};vertical-align:middle'></span> {n}</span>" for n, c in zip(["calm", "medium", "intense"], W["map_fill"]))
    chests, surveyors = [], []
    s = 0
    for i, a in enumerate(wd["areas"]):
        for what, count in a["chests"]:
            label = f"{count} {what}S" if count > 1 and what != "NAMES" else what
            if what == "NAMES":
                label = f"{fmt(count * FACTS['worlds'][w]['roster'][0]['names'])} NAMES"   # a fodder's names x 30
            if what == "MAP PAGE":
                label = f"MAP PAGE {w}"
            chests.append(f"<tr><td>{i}</td><td>{esc(a['name'])}</td><td>{esc(label)}</td></tr>")
        if a["tiles"]["M"]:
            surveyors.append(f"{esc(FACTS['npcs'][16 + (w - 1) * 2 + s].replace('SURVEYOR ', '').title())} ({esc(a['name'].title())})")
            s += 1
    hazards = "".join(f"<li><b>{esc(n)}.</b> {esc(t)}</li>" for n, t in W["hazards"])
    return f"""
<section class="page" style="background:linear-gradient(170deg, {W['dark']}, #05060c)">
  <div class="inner">
    <div><div class="chapter-no" style="color:{W['accent']}">WORLD {w} · THE MAP</div><h2>{esc(wd['name'].title())}: 20 areas</h2></div>
    <div class="card dark" style="padding:3mm">{map_svg(w)}
      <div style="font-size:7.8pt;margin-top:2mm;color:#dde">{bands} &nbsp; {legend} &nbsp; <span style="color:#d08aff">- - portal</span></div></div>
    <div class="row">
      <div class="col" style="flex:1.1"><div class="card"><h3>The main path</h3><p style="font-size:9.4pt">{esc(W['path'])}</p>
        <h3>Off the path</h3><p style="font-size:9.4pt">{esc(W['secrets'])}</p>
        <p style="font-size:9.4pt"><b>Surveyors:</b> {', '.join(surveyors)}.</p></div></div>
      <div class="col" style="flex:.9"><div class="card"><h3>Hazards</h3><ul>{hazards}</ul></div>
        <div class="card" style="padding:3mm 4mm"><h3>Chests</h3><table style="font-size:8.4pt">{''.join(chests)}</table></div></div>
    </div>
  </div>
  {footer('NAME QUEST · WORLD ' + str(w) + ' · MAP')}
</section>"""


def atlas_page(w):
    W, wd = WORLDS[w], FACTS["worlds"][w]
    tiles = ""
    band_names = ["calm", "medium", "intense"]
    for i, a in enumerate(wd["areas"]):
        t = a["tiles"]
        foes = sum(t[k] for k in "ABDEF")
        notes = [band_names[a["band"]], "tall" if not a["wide"] else "wide"]
        notes.append(f"{foes} foes" if foes else "no foes")
        notes += [l.lower() for l, _ in area_tags(w, i, a)]
        shotname = f"area_{w}_{i:02d}_{1 if a['flags'] & 3 or (a['tiles']['C'] == 0 and a['tiles']['b']) else 0}"
        tiles += (f"<div class='tile'><img src='{png(shotname, 1)}'><div class='t'>{i} · {esc(a['name'])}</div>"
                  f"<div class='n'>{esc(' · '.join(notes))}</div></div>")
    return f"""
<section class="page" style="background:linear-gradient(170deg, {W['dark']}, #05060c)">
  <div class="inner" style="gap:3mm">
    <div><div class="chapter-no" style="color:{W['accent']}">WORLD {w} · EVERY AREA</div><h2>Area by area</h2></div>
    <div class="atlas">{tiles}</div>
  </div>
  {footer('NAME QUEST · WORLD ' + str(w) + ' · AREAS')}
</section>"""


def enemies_page(w):
    W, wd = WORLDS[w], FACTS["worlds"][w]
    rows = ""
    for slot, e in enumerate(wd["roster"]):
        size = f"max-height:{13 if e['kind'] < 2 else 17}mm;max-width:14.5mm;width:auto;height:auto"
        poses = f'<img src="{sprite(f"enemy_{w}_{slot}")}" style="{size}">'
        for pose in ("tell", "attack"):
            if os.path.exists(os.path.join(RAW, f"enemy_{w}_{slot}_{pose}.bmp")):
                poses += f'<img src="{sprite(f"enemy_{w}_{slot}_{pose}")}" style="{size}">'
        text = BEHAVIOUR[e["behaviour"]] + TRAITS.get(e["trait"], "")
        kind_colour = ["#b8e0a0", "#ffd080", "#ff9a8a"][e["kind"]]
        rows += f"""
      <div class="enemy"><div class="poses">{poses}</div><div>
        <div class="name">{esc(e['name'])} <span class="chip" style="background:{kind_colour}">{KIND[e['kind']].upper()}</span></div>
        <div class="meta">HP {e['hp']} &nbsp;·&nbsp; hits for {e['atk']} before armour &nbsp;·&nbsp; {e['names']} names</div>
        <div style="font-size:9.4pt;line-height:1.35">{esc(text)}</div></div></div>"""
    ref = wd["ref"]
    busiest = sorted(range(len(wd["areas"])), key=lambda i: -sum(wd["areas"][i]["tiles"][k] for k in "DEF"))[:2]
    crowd = "".join(picture(f"area_{w}_{i:02d}_0", f"{wd['areas'][i]['name'].title()}: an intense area.", "flex:1") for i in busiest)
    return f"""
<section class="page" style="background:linear-gradient(170deg, {W['dark']}, #05060c)">
  <div class="inner">
    <div><div class="chapter-no" style="color:{W['accent']}">WORLD {w} · THE MONSTERS</div><h2>Who lives here</h2></div>
    <div class="card" style="padding:2mm 5mm">{rows}</div>
    <div class="row">
      <div class="col"><div class="card dark"><h3 style="color:{W['accent']}">Tuned for a level {['', 4, 11, 17][w]} hero</h3>
        <p>The world's numbers are made for a hero of about level {['', 4, 11, 17][w]} with this world's shop gear
        (about {ref[0]:.0f} HP and {ref[1]:.0f} ATK). Fodder falls in about 3 hits from such a hero, soldiers in 5,
        elites in 12. A fodder hit takes about 15 % of your HP, a soldier's 20 %, an elite's 25 %.</p></div></div>
      <div class="col"><div class="card dark"><h3 style="color:{W['accent']}">Where they appear</h3>
        <p>Calm areas (the first third of the path) hold only fodder. Medium areas add soldiers and at most one elite;
        intense areas the most monsters, up to three elites. Beaten monsters stay gone until you rest, travel or fade.
        The small poses beside each monster are its tell and its attack: learn them.</p></div></div>
    </div>
    <div class="row">{crowd}</div>
  </div>
  {footer('NAME QUEST · WORLD ' + str(w) + ' · MONSTERS')}
</section>"""


def boss_page(w, bid):
    W, B = WORLDS[w], BOSSES[bid]
    facts = next(b for b in FACTS["worlds"][w]["bosses"] if b["id"] == bid)
    moves = "".join(f"<div class='move'><div class='tag'>{esc(n)}</div><div><div class='what'>{esc(d)}</div>"
                    f"<div class='tip'>{esc(t)}</div></div></div>" for n, d, t in B["p1"])
    kind = "GREAT BOSS" if facts["big"] else "MINI-BOSS"
    shots = "".join(picture(n, c, "flex:1") for n, c in B["shots"])
    poses = "".join(f"<div style='text-align:center'>{img(f'boss_{bid}_{pose}', 22)}<div class='meta' style='color:#cdd'>{label}</div></div>"
                    for pose, label in (("idle", "IDLE"), ("tell", "TELL: GET READY"), ("attack", "ATTACK"), ("phase2", "PHASE 2")))
    return f"""
<section class="page" style="background:linear-gradient(170deg, {W['dark']}, #05060c)">
  <div class="inner">
    <div style="display:flex;align-items:flex-end;gap:5mm">
      <div style="flex:none">{img(f'boss_{bid}_idle', 40)}</div>
      <div style="flex:1"><div class="chapter-no" style="color:{W['accent']}">WORLD {w} · {kind} · {esc(B['where'])}</div>
        <h2 style="font-size:28pt">{esc(B['title'])}</h2><div class="tagline" style="font-size:12pt">{esc(B['tag'])}</div></div>
    </div>
    <div class="row">
      <div class="col" style="flex:1.25"><div class="card"><p>{esc(B['about'])}</p>
        <h3>Phase 1</h3>{moves}
        <h3 style="margin-top:2mm">Phase 2 (below half HP)</h3><p>{esc(B['p2'])} It changes with a roar and a short
        invulnerable pose; then it picks from the new list.</p></div></div>
      <div class="col" style="flex:.75"><div class="card"><h3>Numbers</h3><table>
        <tr><td>HP</td><td><b>{fmt(facts['hp'])}</b></td></tr>
        <tr><td>Touch</td><td>{facts['contact']}</td></tr>
        <tr><td>Attacks</td><td>{facts['atk']}</td></tr>
        <tr><td>Big attacks</td><td>{facts['bigAtk']}</td></tr>
        <tr><td>Pause between</td><td>{facts['gap']:g} s</td></tr>
        <tr><td>Names</td><td><b>{fmt(facts['names'])}</b></td></tr></table>
        <p style="font-size:8.5pt;margin-top:1.5mm">Damage before your DEF. Phase 2 shortens the pause to 75 %.</p></div>
        <div class="card dark"><h3 style="color:{W['accent']}">Reward</h3><p>{esc(B['reward'])}</p>
        <p style="font-size:8.8pt">Never the same attack twice in a row. If you fade, it comes back at full HP.</p></div></div>
    </div>
    <div class="row">{shots}</div>
    <div class="card dark" style="display:flex;justify-content:space-around;align-items:flex-end;padding:3mm">{poses}</div>
  </div>
  {footer('NAME QUEST · WORLD ' + str(w) + ' · ' + B['title'].upper())}
</section>"""


def closing_page():
    rows = ""
    for w in (1, 2, 3):
        for bid in WORLD_BOSSES[w]:
            facts = next(b for b in FACTS["worlds"][w]["bosses"] if b["id"] == bid)
            B = BOSSES[bid]
            attacks = ", ".join(n for n, _, _ in B["p1"])
            rows += (f"<tr><td>{img(f'boss_{bid}_idle', 9)}</td><td style='font-weight:700'>{esc(B['title'])}</td>"
                     f"<td>{w}</td><td>{'great' if facts['big'] else 'mini'}</td><td>{fmt(facts['hp'])}</td><td>{fmt(facts['names'])}</td>"
                     f"<td>{esc(attacks)}</td></tr>")
    ahead = "".join(f"<li><b>{esc(FACTS['worlds'][w]['name'].title())}</b></li>" for w in (4, 5, 6))
    return f"""
<section class="page" style="{bg('area_3_19_1', 0.55, 0.85)}">
  <div class="inner">
    <h2>Boss cheat sheet</h2>
    <div class="card" style="padding:3mm 4mm"><table>
      <tr style="background:#ffb050"><th></th><th>Boss</th><th>World</th><th>Kind</th><th>HP</th><th>Names</th><th>Phase 1</th></tr>{rows}</table></div>
    <div class="row">
      <div class="col"><div class="card"><h3>Before every boss</h3><ul>
        <li>Rest at the nearest checkpoint and bank your names with Ilsa.</li>
        <li>Fill up on Red Potions; buy a Whetstone or an Iron Tonic for a hard one.</li>
        <li>Watch one full round of attacks before you go in hard.</li>
        <li>Punish the open moment after a big attack.</li></ul></div></div>
      <div class="col"><div class="card dark"><h3 style="color:#ffb050">The road ahead</h3>
        <p>Three Great Names taken, two still to find. Beyond the White Silence the map goes on, still being written:</p>
        <ul style="color:#eef">{ahead}</ul>
        <p style="margin-top:2mm">... and whatever waits behind the Name-Eater's throne.</p></div></div>
    </div>
    <div class="card dark" style="text-align:center"><div class="big" style="font-size:19pt;color:#ffb050">Go and find your name.</div></div>
  </div>
  {footer('NAME QUEST · PLAYER GUIDE')}
</section>"""


pages = cover_page() + story_page() + controls_page() + heroes_overview_page()
pages += "".join(hero_page(c) for c in range(4))
pages += namula_page()
for w in (1, 2, 3):
    mini, big = WORLD_BOSSES[w]
    pages += world_cover(w) + map_page(w) + atlas_page(w) + enemies_page(w) + boss_page(w, mini) + boss_page(w, big)
pages += closing_page()

page_html = os.path.join(WORK, "guide.html")
with open(page_html, "w") as f:
    f.write(f"<!DOCTYPE html><html><head><meta charset='utf-8'><title>NAME QUEST guide</title><style>{CSS}</style></head>"
            f"<body>{pages}</body></html>")
subprocess.run([CHROME, "--headless=new", "--disable-gpu", "--no-pdf-header-footer", "--allow-file-access-from-files",
                "--print-to-pdf=" + PDF, "file://" + page_html], check=True, stderr=subprocess.DEVNULL)
print("wrote", os.path.relpath(PDF, ROOT))
