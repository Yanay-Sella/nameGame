"""Builds the player's level guide: docs/level-guide/NAME-game-level-guide.pdf.

Scenes are drawn by the game's own code (render_scenes.cpp), sprites are read from source/sprites.h,
and the pages are HTML printed to PDF by Google Chrome. Needs Pillow, a C++ compiler and Chrome.
Run from anywhere: python3 docs/level-guide/make_level_guide.py
"""
import html
import os
import re
import subprocess

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.abspath(os.path.join(HERE, "..", ".."))
WORK = os.path.join(ROOT, "build", "level-guide")
SHOTS = os.path.join(WORK, "shots")
ART = os.path.join(WORK, "art")
PDF = os.path.join(HERE, "NAME-game-level-guide.pdf")
CHROME = "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"

# --- Pictures -------------------------------------------------------------------------------------------------------

os.makedirs(SHOTS, exist_ok=True)
os.makedirs(ART, exist_ok=True)
subprocess.run(["c++", "-std=c++14", "-O1", "-I" + os.path.join(ROOT, "source"), os.path.join(HERE, "render_scenes.cpp"),
                os.path.join(ROOT, "source", "game.cpp"), os.path.join(ROOT, "source", "screen.cpp"),
                os.path.join(ROOT, "source", "formations.cpp"),
                "-o", os.path.join(WORK, "render_scenes")], check=True)
subprocess.run([os.path.join(WORK, "render_scenes"), SHOTS], check=True)
for name in os.listdir(SHOTS):   # BMP to PNG, magnified with hard pixels so Chrome never smooths them
    if name.endswith(".bmp"):
        image = Image.open(os.path.join(SHOTS, name)).convert("RGB")
        image.resize((image.width * 6, image.height * 6), Image.NEAREST).save(os.path.join(SHOTS, name[:-4] + ".png"))
        os.remove(os.path.join(SHOTS, name))

with open(os.path.join(ROOT, "source", "sprites.h")) as f:
    SOURCE = f.read()
PALETTE = {c: ((int(v, 16) >> 16) & 255, (int(v, 16) >> 8) & 255, int(v, 16) & 255)
           for c, v in re.findall(r"case '(.)': return 0x([0-9A-Fa-f]{8});", SOURCE)}
for name, w, h, body in re.findall(r'const Sprite (\w+) = \{(\d+), (\d+),((?:\s*"[^"]*")+)\};', SOURCE):
    pixels = "".join(re.findall(r'"([^"]*)"', body))
    image = Image.new("RGBA", (int(w), int(h)), (0, 0, 0, 0))
    image.putdata([PALETTE[c] + (255,) if c in PALETTE and c != "." else (0, 0, 0, 0) for c in pixels])
    image.resize((int(w) * 12, int(h) * 12), Image.NEAREST).save(os.path.join(ART, name + ".png"))


def shot(name):
    return "file://" + os.path.join(SHOTS, name + ".png")


def sprite(name):
    return "file://" + os.path.join(ART, name + ".png")


def esc(text):
    return html.escape(text, quote=True)


# --- The levels -----------------------------------------------------------------------------------------------------
# Facts come from docs/features/*/design.md and source/game.cpp; the words are written for players.

OFFSET = {1: 0, 2: 5, 3: 8, 4: 11, 5: 14}        # formation wave n plays like level 1's wave n + this
HEAVY_EVERY = {1: 0, 2: 8, 3: 6, 4: 5, 5: 4}     # heavies per formation wave: wave / this, at most 3

LEVELS = [
    {
        "n": 1, "key": "space", "title": "Deep Space", "accent": "#ffd84a", "accent2": "#6f8cff",
        "tagline": "Where every pilot's story begins.",
        "intro": "A calm, dark sky full of drifting stars. Rows of little aliens float in, line up at the top of the "
                 "screen and sway from side to side, dropping pink shots. It starts gently, so you can learn to dodge "
                 "and shoot, then climbs all the way to two hat-wearing mages at wave 20.",
        "mood": "Quiet, dark, classic arcade", "scenery": "A starfield in three layers that drift down at different speeds, so the sky feels deep",
        "twist": ("No tricks here", "Space has no special rule. It is the place to learn the basics: hold R1, keep moving, "
                  "and save your rockets for the bosses."),
        "enemies": [
            ("GREEN_ALIEN_A", "Green alien", 1, 100, "The friendly-looking rookie. One hit and it pops."),
            ("BLUE_ALIEN_A", "Blue alien", 2, 200, "A bit tougher; arrives from wave 4 in the upper rows."),
            ("RED_ALIEN_A", "Red armoured alien", 4, 400, "Wears armour and sits at the top. Shows up from wave 7."),
        ],
        "heavy": None,
        "tips": ["From wave 3, an alien sometimes breaks away and swoops down at you in a curve. Step aside, it flies off "
                 "the bottom and comes back to its place.",
                 "From wave 16, two aliens can dive at the same time.",
                 "Shots get a little faster every wave after wave 10. Waves 16 to 19 are the busiest of the level."],
        "small": {
            "name": "The Classic", "sprite": "BOSS_A", "waves": "5 and 10",
            "about": "A big, round alien commander with a wide grin. On wave 5 it comes alone and sweeps across the "
                     "whole screen. On wave 10 two of them arrive, and each one guards its own half of the screen.",
            "stats": [("Wave 5", "1 Classic", "100", "slow", "every 2.5 s", "5,000"),
                      ("Wave 10", "2 Classics", "100 each", "medium", "every 2.2 s", "10,000 each")],
            "rhythm": "Never surprises you: it always alternates its two attacks, one, then the other.",
            "moves": [("Triple shot", "Three quick pink shots aimed right at your ship.", "Keep moving sideways; the shots go where you WERE."),
                      ("Fan of five", "Five shots spread out downward like an open hand.", "Fly through one of the gaps between the shots.")],
            "shots": [("space-classic", "The Classic opens its fan of five."), ("space-twins", "Wave 10: twin Classics, each in its own half.")],
        },
        "big": {
            "name": "The Mage", "sprite": "MAGE_A", "waves": "15 and 20",
            "about": "The Classic alien put on a pointed wizard's hat, and with it learned magic. It mixes its attacks "
                     "in a random order with random pauses, so you can never quite guess what comes next. Two mages "
                     "at wave 20 are the boss of the whole level.",
            "stats": [("Wave 15", "1 Mage", "200", "fast", "every 1.1-2.7 s", "15,000"),
                      ("Wave 20", "2 Mages", "200 each", "very fast", "every 1.0-2.2 s", "20,000 each")],
            "rhythm": "Random: 35% triple shot, 35% fan of five, 30% Power Ball.",
            "moves": [("Triple shot", "Three aimed pink shots, like the Classic's.", "Keep sliding sideways."),
                      ("Fan of five", "Five shots fanning downward.", "Find the gap."),
                      ("Power Ball", "A pink ball appears at the tip of the hat and grows for 1.5 s, with a dotted line "
                       "showing where it will strike. Then a huge pink beam shoots straight down for almost a second. "
                       "The ball stays where it was cast, even when the mage moves on.",
                       "Watch the dotted line and get out of that column. The beam is wide!")],
            "shots": [("space-mage-charge", "The Power Ball charges. See the dotted line?"), ("space-mage-beam", "...and the beam fires. Nobody home, phew.")],
        },
    },
    {
        "n": 2, "key": "jungle", "title": "The Jungle", "accent": "#9bd432", "accent2": "#d8a868",
        "tagline": "Humid, buzzing, and very cheeky.",
        "intro": "A dark rainforest of giant tree trunks, rope bridges and vines, lit by blinking fireflies. "
                 "The aliens are gone; now it is insects, a big horned beetle that wanders around, and two "
                 "troublemakers who throw bananas: a monkey and a gorilla.",
        "mood": "Humid, buzzing, cheeky", "scenery": "Tall trunks, leafy clumps, rope bridges and vines, with fireflies blinking in the dark",
        "twist": ("Banana peels and stink", "The bosses throw banana peels that make you slip, and the gorilla throws "
                  "poop that turns into a smelly green cloud. Nothing here is deadly unless it is pink, but these can "
                  "still ruin your day."),
        "enemies": [
            ("LEAFHOPPER_A", "Leafhopper", 1, 100, "A lime-green hopping bug. Easy pickings."),
            ("BEETLE_A", "Brown beetle", 2, 200, "A shiny brown shell takes two hits."),
            ("STAG_A", "Stag beetle", 4, 400, "Big black jaws and a tough shell."),
        ],
        "heavy": ("RHINO_A", "Rhinoceros beetle", "A huge horned beetle that does not stay in line. It wanders around the "
                  "upper part of the screen, never dives, and shoots twice as often as the others. Arrives from wave 8."),
        "tips": ["The jungle starts where space left off: wave 1 here already feels like space wave 6.",
                 "Shoot the rhinoceros beetle when it wanders into a clear spot. It takes 12 hits, so a rocket is fair game.",
                 "Two divers at once start at wave 11 in the jungle."],
        "small": {
            "name": "The Monkey", "sprite": "MONKEY_A", "waves": "5 and 10",
            "about": "A brown monkey with a banana in its raised hand and a very loud screech. It swings across the top "
                     "of the screen throwing bananas, and loves to leave banana peels lying around where you fly.",
            "stats": [("Wave 5", "1 Monkey", "150", "medium", "every 2.2 s", "25,000"),
                      ("Wave 10", "2 Monkeys", "150 each", "fast", "every 2.0 s", "30,000 each")],
            "rhythm": "Takes turns: banana throw, then banana bunch, then banana throw again.",
            "moves": [("Banana throw", "Three bananas drop straight down, one after another. At the same time it lobs a "
                       "banana peel that lands somewhere in your area a moment later.", "Step out from under the monkey, and remember where the peel lands."),
                      ("Banana bunch", "Five bananas thrown in a fan.", "Fly through a gap, or stay near the edges.")],
            "shots": [("jungle-monkey", "A banana bunch on its way down."), ("jungle-monkeys", "Twin monkeys: bananas in the air and peels on the ground.")],
        },
        "big": {
            "name": "The Gorilla", "sprite": "GORILLA_A", "waves": "15 and 20",
            "about": "A slate-grey gorilla with a silver back and angry red eyes. It throws bananas, pounds its chest "
                     "until coconuts fall, and throws poop that leaves a stink cloud. The worst part: if you shoot it "
                     "from inside the stink, you HEAL it, and it laughs at you.",
            "stats": [("Wave 15", "1 Gorilla", "300", "slow", "every 1.1-2.5 s", "35,000"),
                      ("Wave 20", "2 Gorillas", "300 each", "medium", "every 1.0-2.2 s", "40,000 each")],
            "rhythm": "Random: 30% banana throw, 30% poop, 20% banana bunch, 20% chest pound.",
            "moves": [("Banana throw", "Three bananas down and a peel lobbed into your area.", "Move aside; watch where the peel lands."),
                      ("Poop throw", "Lobbed right at you. It splats into a green stink cloud that lasts 6 seconds "
                       "(at most 3 clouds at a time).", "Get out of the cloud before you shoot!"),
                      ("Banana bunch", "Five bananas in a fan.", "Find the gap."),
                      ("Chest pound", "It beats its chest for a moment, then five coconuts fall from the trees, one in "
                       "each fifth of the screen.", "When you see the pounding, look up and find the coconut-free lane.")],
            "shots": [("jungle-pound", "Chest pound: coconuts start falling."), ("jungle-gorillas", "Inside the stink: the gorilla laughs HA HA and its bar turns lime.")],
        },
        "hazards": [
            ("PEEL", "Banana peel", "Lies on the ground for 5 seconds (it blinks before it goes). Touch it and you slide "
             "for half a second the way you were steering, then you are dizzy for 1 second: no moving, no shooting, "
             "no rockets. It does not kill you."),
            ("POOP", "Poop and stink cloud", "While your ship is inside a stink cloud, your bullets turn lime green and "
             "HEAL the gorilla instead of hurting it. Rockets still hurt it, stink or no stink."),
        ],
    },
    {
        "n": 3, "key": "glacier", "title": "The Glacier", "accent": "#9fe0ff", "accent2": "#4a9ad8",
        "tagline": "Cold, quiet, and a little bit silly.",
        "intro": "A frozen cave of ice pillars and icicle shelves, with snow drifting down. The ice is so slippery that "
                 "your ship glides around. The bosses are a yeti with a terrible cold and a walking pile of ice blocks "
                 "that can freeze you solid.",
        "mood": "Cold, quiet, a little comic", "scenery": "Crystal ice pillars, jagged ice walls and icicle shelves, with gently falling snow",
        "twist": ("Slippery ice", "Your ship slides! It speeds up slowly when you push the stick and keeps gliding a little "
                  "after you let go. Start your dodges a moment earlier than usual."),
        "enemies": [
            ("FLAKE_A", "Snowflake creature", 1, 100, "A living snowflake with tiny eyes."),
            ("ICEBAT_A", "Ice bat", 2, 200, "Flaps its frosty wings; takes two hits."),
            ("FROSTSKULL_A", "Frost skull", 4, 400, "A spooky frozen skull. Tough!"),
        ],
        "heavy": ("YETICLOUD_A", "Little yeti on a cloud", "A baby yeti riding its own little cloud. It roams the sky, never "
                  "dives and shoots twice as often. Takes 12 hits. Arrives from wave 6; two from wave 12, three from wave 18."),
        "tips": ["Because you glide, small taps on the stick work better than holding it all the way.",
                 "Two divers at once from wave 7.",
                 "Pink shots are a little faster here than in the jungle."],
        "small": {
            "name": "The Yeti", "sprite": "YETI_A", "waves": "5 and 10",
            "about": "A big white furry yeti with a blue face. It throws snowballs and ice blocks, and it has a cold. "
                     "When it says \"AH...\" get ready: an enormous \"ACHOO!\" sprays snowballs everywhere.",
            "stats": [("Wave 5", "1 Yeti", "180", "medium", "every 2.1 s", "45,000"),
                      ("Wave 10", "2 Yetis", "180 each", "fast", "every 1.9 s", "50,000 each")],
            "rhythm": "Always in the same order: snowball volley, ice block, sneeze, and again.",
            "moves": [("Snowball volley", "Three snowballs aimed at you.", "Keep sliding sideways."),
                      ("Ice block", "Throws a block of ice straight at you. It does not kill, but it freezes you.", "Dodge it, or mash R1 if it gets you."),
                      ("The sneeze", "\"AH...\" for half a second, then \"ACHOO!\" and a fan of 7 snowballs.", "When you see AH..., move to a gap early.")],
            "shots": [("glacier-sneeze", "ACHOO! Seven snowballs in a wide fan."), ("glacier-yeti", "Caught by an ice block: frozen in a cube.")],
        },
        "big": {
            "name": "The Ice Golem", "sprite": "GOLEM_A", "waves": "15 and 20",
            "about": "A giant made of blocks of ice, with glowing cyan eyes. It throws ice blocks two at a time and can "
                     "make the ceiling rain icicles.",
            "stats": [("Wave 15", "1 Ice Golem", "340", "slow", "every 1.0-2.4 s", "55,000"),
                      ("Wave 20", "2 Ice Golems", "340 each", "medium", "every 0.9-2.1 s", "60,000 each")],
            "rhythm": "Random: 30% snowball volley, 25% two ice blocks, 20% snowball fan, 25% icicle rain.",
            "moves": [("Snowball volley", "Three aimed snowballs.", "Keep moving."),
                      ("Two ice blocks", "One thrown at you, one at a random spot in your area.", "Watch both, they go to different places."),
                      ("Snowball fan", "Five snowballs fanning down.", "Find the gap."),
                      ("Icicle rain", "Six icicles appear at the top, shaking, for almost a second. Then they all drop fast.", "Use the shaking time to move under a gap.")],
            "shots": [("glacier-golem", "Icicle rain: six icicles hang shaking, then drop."), ("glacier-golems", "Two golems and their flying ice blocks.")],
        },
        "hazards": [
            ("ICE_BLOCK", "Ice block", "Touch it and your ship is frozen in an ice cube for 1.6 seconds: no moving, no "
             "shooting, no rockets. Every fresh press of R1 chips some ice away, so mash it to break free sooner. "
             "It does not kill you, but you can be shot while frozen."),
            ("ICICLE", "Icicle", "Pink-tipped and deadly. They hang and shake before they fall, so you get a warning."),
        ],
    },
    {
        "n": 4, "key": "volcano", "title": "The Volcano", "accent": "#ff6a10", "accent2": "#ffd84a",
        "tagline": "Hot, angry, and loud.",
        "intro": "A deep chasm of dark rock with rivers of lava winding through it and glowing embers rising into the air. "
                 "The bosses throw fireballs and drops of lava, and the Fire Lord can make the ground erupt right under you.",
        "mood": "Hot, angry, loud", "scenery": "Dark rock, winding lava rivers, glowing cracks, and embers floating upward",
        "twist": ("Lava drops: a risky power-up", "Lava drops are not deadly. If one touches you, your ship gets SCORCHED for "
                  "3 seconds: it moves slower, but every bullet hits twice as hard, as orange fire bolts. \"HOT!\" appears "
                  "and flames trail your ship. Grab one on purpose when you need extra damage!"),
        "enemies": [
            ("EMBER_A", "Living ember", 1, 100, "A little flame with a face."),
            ("FIREBAT_A", "Fire bat", 2, 200, "Bat wings made of fire."),
            ("MAGMASKULL_A", "Magma skull", 4, 400, "A skull of glowing magma. Very tough."),
        ],
        "heavy": ("PHOENIX_A", "Phoenix", "A blazing firebird that flies around the sky instead of staying in line, shooting "
                  "twice as often. 12 hits. Arrives from wave 5; two from wave 11, three from wave 16."),
        "tips": ["From the very first wave, the volcano is busy: 5 rows with armoured aliens on top.",
                 "Two divers at once from wave 4.",
                 "Pink shots are faster again than on the glacier."],
        "small": {
            "name": "The Fire Imp", "sprite": "IMP_A", "waves": "5 and 10",
            "about": "A grinning little devil with flaming hair, small horns and bat wings. It throws fireballs and lets "
                     "lava drip down on you.",
            "stats": [("Wave 5", "1 Fire Imp", "200", "medium", "every 2.0 s", "65,000"),
                      ("Wave 10", "2 Fire Imps", "200 each", "fast", "every 1.8 s", "70,000 each")],
            "rhythm": "Always in the same order: fireball volley, lava drip, fireball fan.",
            "moves": [("Fireball volley", "Three fireballs aimed at you.", "Keep sliding sideways."),
                      ("Lava drip", "Three lava drops falling slowly in a small fan. Not deadly: they scorch you.", "Dodge them, or grab one for double damage."),
                      ("Fireball fan", "Five fireballs fanning downward.", "Find the gap.")],
            "shots": [("volcano-imp", "The imp's lava drip and fireballs."), ("volcano-twins", "Scorched! HOT! and double-damage fire bolts.")],
        },
        "big": {
            "name": "The Fire Lord", "sprite": "FIRELORD_A", "waves": "15 and 20",
            "about": "A horned head in a mane of flame, with a molten core and fists of magma. It rains lava across the "
                     "whole screen and opens volcanic eruptions right below your ship.",
            "stats": [("Wave 15", "1 Fire Lord", "380", "medium", "every 1.0-2.2 s", "75,000"),
                      ("Wave 20", "2 Fire Lords", "380 each", "fast", "every 0.8-2.0 s", "80,000 each")],
            "rhythm": "Random: 30% fireball volley, 20% fireball fan, 25% lava rain, 25% eruption.",
            "moves": [("Fireball volley", "Three aimed fireballs.", "Keep moving."),
                      ("Fireball fan", "Five fireballs fanning down.", "Find the gap."),
                      ("Lava rain", "Five lava drops, one in each fifth of the screen.", "Dodge, or grab one for a scorch."),
                      ("Eruption", "A bubbling pool and a dotted pink line appear under you for 1 second. Then a pillar of "
                       "fire shoots from the bottom to the top of the screen.", "As soon as you see the bubbling, move sideways!")],
            "shots": [("volcano-warn", "The warning: a bubbling pool and a dotted line..."), ("volcano-eruption", "...then the eruption. Missed!")],
        },
        "hazards": [
            ("LAVA_DROP", "Lava drop", "Scorches you for 3 seconds: slower ship, double-damage bullets. A new drop restarts "
             "the 3 seconds. Rockets are not affected. Dying puts the fire out."),
            ("FIREBALL_A", "Fireball", "Pink-centred and deadly, like every shot in the game."),
        ],
    },
    {
        "n": 5, "key": "sea", "title": "The Deep Sea", "accent": "#3fb8a0", "accent2": "#ffd84a",
        "tagline": "Dark, scary... and one very silly fisherman.",
        "intro": "The last and hardest level. Deep, dark water with swaying kelp, faint beams of light and bubbles "
                 "rising from below. Scary fish fill the screen, a current pushes you around, and at the end waits "
                 "a fish in a big yellow fisherman's hat who wants to catch YOU.",
        "mood": "Dark and scary, with one silly fisherman", "scenery": "Light rays from far above, rock walls, swaying kelp and rising bubbles",
        "twist": ("The current", "The water slowly pushes your ship to one side, then back to the other, about once every "
                  "14 seconds. You will see the background bubbles drift too. Lean against it with the stick."),
        "enemies": [
            ("PIRANHA_A", "Piranha", 1, 100, "Small, but look at those teeth."),
            ("PUFFER_A", "Pufferfish", 2, 200, "Round, spiky, and yellow."),
            ("LIONFISH_A", "Lionfish", 4, 400, "Red stripes and poison spines. Tough."),
        ],
        "heavy": ("SHARK_A", "Shark", "A shark that patrols the upper water, never dives, and shoots twice as often. 12 hits. "
                  "One from wave 4, two from wave 8, three from wave 12."),
        "tips": ["The busiest level: 5 full rows with lots of armour from wave 1.",
                 "Two divers at once from the very first wave.",
                 "Pink shots are at their fastest here."],
        "small": {
            "name": "The Anglerfish", "sprite": "ANGLER_A", "waves": "5 and 10",
            "about": "A gaping mouth full of fangs, red eyes and a glowing lure dangling in front of its face. "
                     "It spits bubbles: in streams, in fans, and one giant bubble that bursts.",
            "stats": [("Wave 5", "1 Anglerfish", "220", "medium", "every 1.9 s", "85,000"),
                      ("Wave 10", "2 Anglerfish", "220 each", "fast", "every 1.7 s", "90,000 each")],
            "rhythm": "Always in the same order: bubble stream, big bubble, bubble fan.",
            "moves": [("Bubble stream", "Five wobbly bubbles aimed at you, one right after another.", "Keep moving; they wobble, so give them room."),
                      ("Big bubble", "A big slow bubble that floats toward you, then pops into 8 small bubbles flying in every direction.", "Stay far from it when it pops."),
                      ("Bubble fan", "Seven bubbles in a wide fan.", "Find the gap.")],
            "shots": [("sea-angler", "A bubble stream heading for the ship."), ("sea-twins", "Twin anglerfish: a big bubble and a fan.")],
        },
        "big": {
            "name": "The Fisherman Fish", "sprite": "FISHERMAN_A", "waves": "15 and 20",
            "about": "A fish wearing a big yellow fisherman's hat and holding a fishing rod. It casts its hook down at "
                     "you. If it misses, it sometimes reels up an old boot instead... and throws that at you. "
                     "Two fishermen at wave 20 are the final boss of the whole game.",
            "stats": [("Wave 15", "1 Fisherman", "420", "medium", "every 0.9-2.1 s", "95,000"),
                      ("Wave 20", "2 Fishermen", "420 each", "fast", "every 0.8-1.8 s", "100,000 each")],
            "rhythm": "Random: 35% bubble stream, 15% big bubble, 20% bubble fan, 30% hook cast.",
            "moves": [("Bubble stream", "Five wobbly aimed bubbles.", "Keep moving."),
                      ("Big bubble", "The giant bubble that pops into 8.", "Keep your distance."),
                      ("Bubble fan", "Seven bubbles in a fan.", "Find the gap."),
                      ("Hook cast", "The hook drops where you are, waits at the bottom for a moment, then reels up. If it "
                       "catches you: \"GOT ONE!\" Your ship is lifted up and dangles for 1.6 seconds, unable to move or "
                       "shoot. If it catches nothing, 35% of the time it brings up an OLD BOOT and throws it at you.",
                       "Move away from the line as soon as it drops. The boot is pink: it is deadly!")],
            "shots": [("sea-hook", "The hook comes down, and an old boot flies."), ("sea-fishermen", "The final battle: two fishermen.")],
        },
        "hazards": [
            ("HOOK", "The hook", "Not deadly, but it holds you helpless for 1.6 seconds, and you can still be shot "
             "while dangling. A ship that is blinking after a respawn is never hooked."),
            ("BOOT", "The old boot", "Thrown straight at you. It is pink, so yes, it kills."),
        ],
    },
]


def wave_rows(level):
    """Who comes on each of the level's 20 waves, worked out the same way as startWave in game.cpp."""
    rows = []
    roles = level["enemies"]
    for wave in range(1, 21):
        if wave % 5 == 0:
            boss = level["small"] if wave < 15 else level["big"]
            count = 2 if wave % 10 == 0 else 1
            rows.append((wave, None, boss, count))
            continue
        dw = wave + OFFSET[level["n"]]
        total = min(2 + dw // 2, 5)
        red = 2 if dw >= 13 else 1 if dw >= 7 else 0
        blue_end = 4 if dw >= 13 else 2 if dw >= 4 else 0
        counts = [0, 0, 0]
        for r in range(total):
            counts[2 if r < red else 1 if r < blue_end else 0] += 8
        every = HEAVY_EVERY[level["n"]]
        heavies = min(wave // every, 3) if every else 0
        divers = 2 if dw >= 15 else 1 if dw >= 3 else 0
        danger = max(1, min(5, -(-(dw + 2 * heavies + 1.5 * (level["n"] - 1)) // 7)))
        rows.append((wave, (counts, heavies, divers, int(danger)), None, 0))
    return rows


# --- HTML -----------------------------------------------------------------------------------------------------------

CSS = """
@page { size: A4; margin: 0; }
* { box-sizing: border-box; }
html, body { margin: 0; padding: 0; }
body { font-family: 'Avenir Next', 'Helvetica Neue', sans-serif; color: #1d1d28; -webkit-print-color-adjust: exact; print-color-adjust: exact; }
.page { width: 210mm; height: 297mm; position: relative; overflow: hidden; page-break-after: always; break-after: page;
        background-color: #0c0f1e; background-size: cover; background-position: center; }
.inner { position: absolute; left: 13mm; right: 13mm; top: 12mm; bottom: 12mm; display: flex; flex-direction: column; gap: 5mm; }
h1, h2, h3, .big { font-family: 'Futura', 'Avenir Next', sans-serif; font-weight: 800; margin: 0; }
h2 { font-size: 21pt; color: white; text-shadow: 0 1mm 0 rgba(0,0,0,.45); letter-spacing: .5px; }
h3 { font-size: 13pt; margin: 0 0 2mm 0; }
p { margin: 0 0 2mm 0; font-size: 10.5pt; line-height: 1.42; }
.card { background: rgba(255,255,255,.94); border-radius: 5mm; padding: 5mm 6mm; box-shadow: 0 1.5mm 4mm rgba(0,0,0,.35); }
.card.dark { background: rgba(10,12,26,.78); color: #eef; border: .6mm solid rgba(255,255,255,.18); }
.pill { display: inline-block; font-family: 'Futura'; font-weight: 700; font-size: 9pt; padding: .8mm 3mm; border-radius: 10mm; color: #111; }
.row { display: flex; gap: 5mm; }
.col { flex: 1; display: flex; flex-direction: column; gap: 5mm; }
.shot { width: 100%; image-rendering: pixelated; border-radius: 3mm; display: block; border: .8mm solid rgba(255,255,255,.85); }
.cap { font-size: 9pt; color: #eef; margin-top: 1.3mm; font-style: italic; text-shadow: 0 .4mm 1mm #000; }
.sprite { image-rendering: pixelated; }
table { border-collapse: collapse; width: 100%; font-size: 9.5pt; }
th { font-family: 'Futura'; font-weight: 700; text-align: left; padding: 1.6mm 2mm; color: #111; }
td { padding: 1.5mm 2mm; border-top: .3mm solid rgba(0,0,0,.1); vertical-align: middle; }
tr:nth-child(even) td { background: rgba(0,0,0,.035); }
.danger span { display: inline-block; width: 3.2mm; height: 3.2mm; border-radius: 1mm; margin-right: .7mm; background: #ddd; }
.footer { position: absolute; bottom: 5mm; left: 0; right: 0; text-align: center; font-family: 'Futura'; font-size: 8pt;
          color: rgba(255,255,255,.7); letter-spacing: 2px; }
/* Chapter covers */
.cover .inner { justify-content: flex-end; }
.chapter-no { font-family: 'Futura'; font-weight: 700; letter-spacing: 4px; font-size: 12pt; color: white; opacity: .85; }
.cover h1 { font-size: 48pt; line-height: 1; color: white; text-shadow: 0 1.5mm 0 rgba(0,0,0,.5); }
.tagline { font-size: 15pt; color: white; font-style: italic; margin-top: 2mm; text-shadow: 0 .5mm 1mm #000; }
.boss-parade { display: flex; justify-content: center; align-items: flex-end; gap: 8mm; margin-bottom: 4mm; }
.boss-parade img { height: 30mm; filter: drop-shadow(0 1.5mm 0 rgba(0,0,0,.6)); }
.enemy { display: flex; align-items: center; gap: 4mm; padding: 2.4mm 0; border-top: .3mm solid rgba(0,0,0,.08); }
.enemy:first-of-type { border-top: none; }
.enemy img { width: 22mm; image-rendering: pixelated; flex: none; }
.enemy .name { font-family: 'Futura'; font-weight: 700; font-size: 11.5pt; }
.enemy .meta { font-size: 8.5pt; color: #555; margin-bottom: .8mm; }
.hits span { display: inline-block; width: 2.3mm; height: 2.3mm; border-radius: 50%; margin-right: .5mm; }
.chip { display: inline-flex; align-items: center; margin-right: 2.2mm; font-weight: 600; font-size: 9pt; }
.chip img { height: 4.2mm; image-rendering: pixelated; margin-right: .6mm; }
.waves td { padding: 1.05mm 2mm; }
.bossrow td { background: #fff3b0 !important; font-weight: 700; }
.bossrow img { height: 6mm; image-rendering: pixelated; vertical-align: middle; margin-right: 1.5mm; }
.portrait { width: 72mm; image-rendering: pixelated; filter: drop-shadow(0 2mm 0 rgba(0,0,0,.5)); }
.move { display: flex; gap: 3mm; padding: 3mm 0; border-top: .3mm solid rgba(0,0,0,.08); }
.move:first-of-type { border-top: none; }
.move .tag { flex: none; width: 34mm; font-family: 'Futura'; font-weight: 700; font-size: 12pt; }
.move .what { font-size: 10.8pt; line-height: 1.42; }
.move .tip { font-size: 10pt; color: #2a6a2a; margin-top: .8mm; }
.move .tip::before { content: "TIP  "; font-family: 'Futura'; font-weight: 700; letter-spacing: 1px; }
ul { margin: 0; padding-left: 5mm; } li { font-size: 10pt; line-height: 1.4; margin-bottom: 1.2mm; }
.hazard { display: flex; gap: 4mm; align-items: flex-start; margin-top: 2.5mm; }
.hazard img { width: auto; max-width: 13mm; max-height: 11mm; image-rendering: pixelated; flex: none; margin-top: 1mm; }
.toc-row { display: flex; align-items: center; gap: 5mm; padding: 2.6mm 0; border-top: .3mm solid rgba(255,255,255,.15); color: white; }
.toc-row img.thumb { width: 52mm; border-radius: 2mm; image-rendering: pixelated; border: .6mm solid rgba(255,255,255,.6); }
"""


def danger(level, n):
    return '<span class="danger">' + "".join(
        f'<span style="background:{level["accent"] if i < n else "#dcdce4"}"></span>' for i in range(5)) + "</span>"


def hits(level, count):
    return '<span class="hits">' + "".join(f'<span style="background:{level["accent2"]}"></span>' for _ in range(count)) + "</span>"


def bg(level, dim):
    top, bottom = dim
    return (f"background-image: linear-gradient(rgba(5,6,14,{top}), rgba(5,6,14,{bottom})), "
            f"url('{shot(level['key'] + '-bg')}');")


def picture(name, caption):
    return f'<div><img class="shot" src="{shot(name)}"><div class="cap">{esc(caption)}</div></div>'


def footer(text):
    return f'<div class="footer">{esc(text)}</div>'


def cover(level):
    parade = "".join(f'<img class="sprite" src="{sprite(s)}">' for s in
                     [level["small"]["sprite"], level["big"]["sprite"]])
    return f"""
<section class="page cover" style="{bg(level, (0.05, 0.55))}">
  <div class="inner">
    <div class="boss-parade">{parade}</div>
    <div class="card dark">
      <div class="chapter-no" style="color:{level['accent']}">LEVEL {level['n']} OF 5</div>
      <h1>{esc(level['title'])}</h1>
      <div class="tagline">{esc(level['tagline'])}</div>
      <p style="margin-top:4mm;font-size:11.5pt;color:#eef">{esc(level['intro'])}</p>
    </div>
  </div>
</section>"""


def world_page(level):
    enemies = "".join(f"""
      <div class="enemy"><img src="{sprite(s)}"><div>
        <div class="name">{esc(name)}</div>
        <div class="meta">{hits(level, hp)} {hp} hit{'s' if hp > 1 else ''} &nbsp;·&nbsp; {pts} points</div>
        <div style="font-size:9.6pt">{esc(text)}</div></div></div>""" for s, name, hp, pts, text in level["enemies"])
    if level["heavy"]:
        s, name, text = level["heavy"]
        enemies += f"""
      <div class="enemy"><img src="{sprite(s)}" style="width:26mm"><div>
        <div class="name">{esc(name)} <span class="pill" style="background:{level['accent']}">ROAMER</span></div>
        <div class="meta">{hits(level, 12)} 12 hits &nbsp;·&nbsp; 1,000 points</div>
        <div style="font-size:9.6pt">{esc(text)}</div></div></div>"""
    title, text = level["twist"]
    glance = [("Mood", level["mood"]), ("Scenery", level["scenery"]), ("Special twist", title),
              ("Mid bosses", f"{level['small']['name']} (wave 5), two of them (wave 10)"),
              ("Big bosses", f"{level['big']['name']} (wave 15), two of them (wave 20, the level boss)")]
    rows = "".join(f'<tr><td style="font-weight:700;width:32mm">{esc(k)}</td><td>{esc(v)}</td></tr>' for k, v in glance)
    return f"""
<section class="page" style="{bg(level, (0.35, 0.6))}">
  <div class="inner">
    <h2>Welcome to {esc(level['title'])}</h2>
    <div class="card"><table><tr><th colspan=2 style="background:{level['accent']};border-radius:2mm">At a glance</th></tr>{rows}</table></div>
    <div class="row">
      <div class="col" style="flex:1.15">
        <div class="card"><h3>Who lives here</h3>{enemies}</div>
      </div>
      <div class="col">
        <div class="card" style="border-left:3mm solid {level['accent']}"><h3>{esc(title)}</h3><p style="font-size:10pt">{esc(text)}</p></div>
        {picture(level['key'] + '-wave', 'A busy formation wave in ' + level['title'] + '.')}
        {picture(level['key'] + '-early', 'Wave 2: the calm before the storm.')}
      </div>
    </div>
  </div>
  {footer('NAME GAME · LEVEL GUIDE · ' + level['title'].upper())}
</section>"""


def waves_page(level):
    pics = [s for s, *_ in level["enemies"]]
    body = ""
    for wave, formation, boss, count in wave_rows(level):
        if boss:
            label = ("LEVEL BOSS: " if wave == 20 else "MID BOSS: ") + (f"two {boss['name'][4:]}s" if count == 2 and boss["name"].startswith("The ")
                                                                        else boss["name"])
            if count == 2 and boss["name"] == "The Anglerfish":
                label = label.replace("Anglerfishs", "Anglerfish")
            if count == 2 and boss["name"] == "The Fisherman Fish":
                label = label.replace("Fisherman Fishs", "Fisherman Fish")
            body += (f'<tr class="bossrow"><td>{wave}</td><td colspan=3><img src="{sprite(boss["sprite"])}">'
                     f'{esc(label)}</td><td>{danger(level, 5)}</td></tr>')
            continue
        counts, heavies, divers, d = formation
        chips = "".join(f'<span class="chip"><img src="{sprite(pics[i])}">×{c}</span>' for i, c in enumerate(counts) if c)
        extra = []
        if heavies:
            extra.append(f'<span class="chip"><img src="{sprite(level["heavy"][0])}">×{heavies}</span>')
        body += (f'<tr><td>{wave}</td><td>{chips}</td><td>{"".join(extra) or "&ndash;"}</td>'
                 f'<td>{["none", "one at a time", "two at once"][divers]}</td><td>{danger(level, d)}</td></tr>')
    tips = "".join(f"<li>{esc(t)}</li>" for t in level["tips"])
    hazards = ""
    if level.get("hazards"):
        items = "".join(f"""<div class="hazard"><img src="{sprite(s)}"><div><b style="font-family:Futura">{esc(name)}</b>
            <div style="font-size:8.8pt;line-height:1.33">{esc(text)}</div></div></div>""" for s, name, text in level["hazards"])
        hazards = f'<div class="card col" style="flex:1.25;gap:0;border-left:3mm solid #ff40c0;padding:4mm 5mm"><h3 style="margin:0">Boss hazards to know</h3>{items}</div>'

    return f"""
<section class="page" style="{bg(level, (0.45, 0.65))}">
  <div class="inner">
    <h2>Wave by wave</h2>
    <div class="card" style="padding:4mm 5mm">
      <table class="waves">
        <tr style="background:{level['accent']}"><th style="width:12mm">Wave</th><th>The formation</th><th style="width:20mm">Roamers</th>
            <th style="width:28mm">Divers</th><th style="width:27mm">Danger</th></tr>
        {body}
      </table>
    </div>
    <div class="row"><div class="card col" style="gap:0;border-left:3mm solid {level['accent']}"><h3>Good to know</h3><ul>{tips}</ul></div>
    {hazards}</div>
  </div>
  {footer('NAME GAME · LEVEL GUIDE · ' + level['title'].upper())}
</section>"""


def boss_page(level, boss, kind):
    stat_rows = "".join(f"<tr><td style='font-weight:700'>{a}</td><td>{b}</td><td>{c}</td><td>{d}</td><td>{e}</td><td>{f}</td></tr>"
                        for a, b, c, d, e, f in boss["stats"])
    moves = "".join(f"""<div class="move"><div class="tag" style="color:#111">{esc(m)}</div>
        <div><div class="what">{esc(what)}</div><div class="tip">{esc(tip)}</div></div></div>""" for m, what, tip in boss["moves"])
    pics = "".join(f'<div class="col" style="gap:0">{picture(n, c)}</div>' for n, c in boss["shots"])
    label = "MID BOSS" if kind == "small" else "BIG BOSS"
    return f"""
<section class="page" style="{bg(level, (0.4, 0.7))}">
  <div class="inner">
    <div class="row" style="align-items:center">
      <img class="portrait" src="{sprite(boss['sprite'])}">
      <div class="col" style="gap:2mm">
        <span class="pill" style="background:{level['accent']};align-self:flex-start">{label} · WAVES {boss['waves']}</span>
        <h2 style="font-size:28pt">{esc(boss['name'])}</h2>
        <p style="color:#eef;text-shadow:0 .4mm 1mm #000;font-size:11.5pt">{esc(boss['about'])}</p>
      </div>
    </div>
    <div class="card" style="padding:3.5mm 5mm">
      <table style="font-size:10.5pt"><tr style="background:{level['accent']}"><th>Wave</th><th>Who</th><th>Health</th><th>Moves side to side</th><th>Attacks</th><th>Points per kill</th></tr>{stat_rows}</table>
      <p style="font-size:9pt;margin:2mm 0 0 0;color:#444">Every bullet takes 1 health away; a rocket blast takes 20. {esc(boss['rhythm'])}</p>
    </div>
    <div class="card" style="padding:4mm 6mm"><h3 style="font-size:15pt">Its moves</h3>{moves}</div>
    <div class="row" style="margin-top:auto">{pics}</div>
  </div>
  {footer('NAME GAME · LEVEL GUIDE · ' + level['title'].upper())}
</section>"""


def intro_pages():
    space = LEVELS[0]
    toc = "".join(f"""
      <div class="toc-row"><img class="thumb" src="{shot(l['key'] + '-wave')}">
        <div style="flex:1"><div class="chapter-no" style="color:{l['accent']};font-size:9pt">LEVEL {l['n']}</div>
          <div class="big" style="font-size:19pt">{esc(l['title'])}</div>
          <div style="font-size:9.5pt;opacity:.85">{esc(l['tagline'])}</div></div>
        <img class="sprite" style="height:13mm" src="{sprite(l['small']['sprite'])}">
        <img class="sprite" style="height:13mm" src="{sprite(l['big']['sprite'])}"></div>""" for l in LEVELS)
    title = f"""
<section class="page cover" style="{bg(space, (0.0, 0.35))}">
  <div class="inner" style="justify-content:center;align-items:center;text-align:center">
    <img class="sprite" style="width:44mm;filter:drop-shadow(0 2mm 0 rgba(0,0,0,.6))" src="{sprite('SHIP_A')}">
    <h1 style="font-size:64pt;margin-top:6mm;background:linear-gradient(#fff,#ffd84a 45%,#ff6a10 80%,#e04848);
        -webkit-background-clip:text;color:transparent;text-shadow:none;filter:drop-shadow(0 1.5mm 0 #000)">NAME GAME</h1>
    <div class="tagline" style="font-size:20pt">The Pilot's Guide to All Five Worlds</div>
    <div class="row" style="margin-top:14mm;gap:7mm;align-items:flex-end">
      {''.join(f'<img class="sprite" style="height:16mm;filter:drop-shadow(0 1mm 0 #000)" src="{sprite(l["big"]["sprite"])}">' for l in LEVELS)}
    </div>
    <p style="color:#dde;margin-top:12mm;font-size:11pt">Every level, every enemy, every boss and every trick they have up their sleeves.</p>
  </div>
</section>"""
    journey = f"""
<section class="page" style="{bg(space, (0.45, 0.7))}">
  <div class="inner">
    <h2>Your journey</h2>
    <div class="card dark" style="padding:3mm 6mm">{toc}</div>
    <div class="card"><h3>How the adventure works</h3>
      <p>Each world has <b>20 waves</b>. Every 5th wave there is no formation, only a <b>boss</b>: a mid boss at waves 5 and 10
      (two of them at wave 10), and the level boss at waves 15 and 20 (two of them at wave 20). Beat the wave-20 boss and
      the world is clear!</p>
      <p><b>Classic</b> plays all five worlds in a row, in the order above, keeping your score, lives, rockets and gun.
      Win the deep sea and you have beaten the game. The <b>Level picker</b> lets you play any one world on its own,
      perfect for practice.</p></div>
  </div>
  {footer('NAME GAME · LEVEL GUIDE')}
</section>"""
    controls = [("Left stick or D-pad", "Fly around the bottom part of the screen"), ("R1 (hold)", "Shoot, shoot, shoot"),
                ("R2", "Fire a rocket: a big explosion"), ("Start", "Pause (and Player 2 joins with Start on a second controller)"),
                ("X / Circle", "Choose / go back in the menus")]
    guns = [("Single", "One stream of bullets"), ("Double", "Two streams side by side"), ("Spread", "Three bullets: one straight, two angled"),
            ("Fast spread", "The spread, fired even faster")]
    toolbox = f"""
<section class="page" style="{bg(space, (0.45, 0.7))}">
  <div class="inner">
    <h2>Before you take off</h2>
    <div class="row">
      <div class="card col" style="gap:0"><h3>The controls</h3><table>
        {''.join(f"<tr><td style='font-weight:700;width:34mm'>{esc(a)}</td><td>{esc(b)}</td></tr>" for a, b in controls)}</table></div>
    </div>
    <div class="card" style="border-left:3mm solid #ff40c0">
      <div class="row" style="align-items:center">
        <div style="flex:none"><img class="sprite" style="height:16mm" src="{sprite('ENEMY_SHOT')}"></div>
        <div><h3>The golden rule: PINK MEANS DANGER</h3>
        <p>Anything pink kills your ship on touch: shots, bananas, coconuts, snowballs, icicles, fireballs, eruptions,
        bubbles, even the old boot. Things that are NOT pink, like banana peels, ice blocks, lava drops and the fishing
        hook, will not kill you, but they will cause trouble. Bumping into an enemy or a boss is deadly too.</p></div>
      </div>
    </div>
    <div class="row">
      <div class="card col" style="gap:1mm">
        <h3>Your gun grows</h3>
        <p style="font-size:9.6pt">Catch the falling <img class="sprite" style="height:5mm;vertical-align:middle" src="{sprite('UPGRADE_PICKUP')}">
        upgrade to power up your gun. Losing a life takes one step back.</p>
        <table>{''.join(f"<tr><td style='font-weight:700;width:6mm'>{i + 1}</td><td style='font-weight:700'>{a}</td><td>{b}</td></tr>" for i, (a, b) in enumerate(guns))}</table>
        <p style="font-size:9pt;margin-top:2mm">Already at full power? Each upgrade gives 1,000 points instead.</p>
      </div>
      <div class="card col" style="gap:1mm">
        <h3>Rockets: the panic button</h3>
        <p style="font-size:9.6pt"><img class="sprite" style="height:7mm;vertical-align:middle;float:right;margin-left:2mm" src="{sprite('ROCKET')}">
        You start with 3 rockets and can carry up to 9. A rocket climbs, speeds up, and blows up everything near the first
        thing it hits. Against a boss, one rocket is worth 20 bullets. Catch the falling
        <img class="sprite" style="height:5mm;vertical-align:middle" src="{sprite('ROCKET_PICKUP')}"> for another.</p>
        <p style="font-size:9.6pt">Save them for when a wave gets out of hand, or for a boss.</p>
      </div>
    </div>
    <div class="row">
      <div class="card col"><h3>Lives and score</h3>
        <p style="font-size:9.6pt">You have <b>3 lives</b>, and one hit costs one. After losing one, your ship comes back blinking
        and can't be hurt for a moment. Every <b>50,000 points</b> gives an extra life. Your best score is saved and shown on the title screen.</p></div>
      <div class="card col"><h3>Points</h3><table>
        <tr><td>Small enemy</td><td style="text-align:right">100 / 200 / 400</td></tr>
        <tr><td>Roaming heavy</td><td style="text-align:right">1,000</td></tr>
        <tr><td>Clearing a wave</td><td style="text-align:right">1,000 × wave</td></tr>
        <tr><td>Bosses</td><td style="text-align:right">5,000 to 100,000</td></tr></table></div>
    </div>
  </div>
  {footer('NAME GAME · LEVEL GUIDE')}
</section>"""
    return title + journey + toolbox


def summary_page():
    sea = LEVELS[4]
    rows = ""
    for l in LEVELS:
        for kind in ("small", "big"):
            b = l[kind]
            moves = ", ".join(m for m, *_ in b["moves"])
            rows += (f"<tr><td><img class='sprite' style='height:8mm;vertical-align:middle' src='{sprite(b['sprite'])}'></td>"
                     f"<td style='font-weight:700'>{esc(b['name'])}</td><td>{esc(l['title'])}</td><td style='white-space:nowrap'>{b['waves']}</td>"
                     f"<td>{b['stats'][0][2]}</td><td>{esc(moves)}</td></tr>")
    return f"""
<section class="page" style="{bg(sea, (0.5, 0.75))}">
  <div class="inner">
    <h2>Boss cheat sheet</h2>
    <div class="card" style="padding:4mm 5mm"><table>
      <tr style="background:#ffd84a"><th></th><th>Boss</th><th>World</th><th>Waves</th><th>Health</th><th>Moves</th></tr>{rows}</table></div>
    <div class="card"><h3>Five golden tips</h3><ul>
      <li><b>Pink kills.</b> Everything else just gets in your way.</li>
      <li><b>Keep moving.</b> Aimed shots fly to where you were, not where you are going.</li>
      <li><b>Every warning is a gift:</b> the dotted line, the "AH...", the chest pound, the shaking icicles, the bubbling pool.</li>
      <li><b>Rockets are for emergencies and bosses.</b> One rocket does the work of 20 bullets on a boss.</li>
      <li><b>Bring a friend.</b> A second controller can join any time with Start.</li></ul></div>
    <div class="card dark" style="text-align:center"><div class="big" style="font-size:20pt;color:#ffd84a">Good luck, pilot!</div>
      <p style="color:#dde;margin-top:2mm">Beat the two fishermen at the bottom of the deep sea and you have beaten NAME game.</p></div>
  </div>
  {footer('NAME GAME · LEVEL GUIDE')}
</section>"""


pages = intro_pages()
for level in LEVELS:
    pages += cover(level) + world_page(level) + waves_page(level) + boss_page(level, level["small"], "small") + boss_page(level, level["big"], "big")
pages += summary_page()

page_html = os.path.join(WORK, "guide.html")
with open(page_html, "w") as f:
    f.write(f"<!DOCTYPE html><html><head><meta charset='utf-8'><title>NAME game level guide</title><style>{CSS}</style></head>"
            f"<body>{pages}</body></html>")
subprocess.run([CHROME, "--headless=new", "--disable-gpu", "--no-pdf-header-footer", "--allow-file-access-from-files",
                "--print-to-pdf=" + PDF, "file://" + page_html], check=True, stderr=subprocess.DEVNULL)
print("wrote", os.path.relpath(PDF, ROOT))
