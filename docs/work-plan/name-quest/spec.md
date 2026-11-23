# NAME QUEST — design spec

Every rule and number here is decided (see the decisions table in [`../name-quest-plan.md`](../name-quest-plan.md)).
"Author" marks content the implementer creates inside the stated rules. All on-screen text is UPPERCASE (the
3x5 font has no lowercase). Times are in s.

**Resolution (D-053): NAME QUEST draws at 640x360**, twice the shooter's 320x180 each way (x2 on a 720p TV, x3 at
1080p), so it has four times the pixels for detail. Every distance, size, speed and acceleration in this spec is in
**design pixels (dp)**, the units of the original 320x180 design: the code multiplies them by `DP = 2`
(`const float JUMP_SPEED = 225 * DP;`). So a tile is 8 dp = 16 px, the hitbox 8 x 14 dp = 16 x 28 px, a 16 x 20 dp
sprite is authored as a 32 x 40 text grid, and the tile maps (80 x 23, 40 x 45 tiles) don't change. Art is authored
at the full 640x360 resolution: the extra pixels are for detail, never for blocky 2x copies of 1x art. Text uses the
3x5 font at scale 2 (1 font pixel = 1 dp); UI layouts in §15 are in dp. "px" below means dp unless it says "real px".

Sections: §1 story · §2 controls · §3 screens and flow · §4 areas and tiles · §5 movement · §6 classes and jobs ·
§7 combat · §8 enemies · §9 hazards · §10 economy · §11 checkpoints, Namula, map · §12 bosses · §13 worlds ·
§14 NPCs, quests, dialogue · §15 HUD and UI · §16 audio · §17 save file · §18 logs · §19 ending and New Game+ ·
§20 dev aids · §21 palette and sprites · §22 art direction

---

## §1 Story and tone

Dark fantasy with warmth: a fading world, melancholy places, kind and odd people in the hub.

- Every thing has a name, and its name keeps it alive. The **Unnaming**, a grey hunger, eats names; what loses its
  name fades. Monsters are full of stolen names. Killing them frees the names, which you carry as currency (**names**).
- **Namula** is the last village, on a cliff at the edge of the world, around the **Hearth of Names**, the last fire
  that still burns. You wake beside it with no memory of your own name.
- **Ilsa, Keeper of Names**, sits by the hearth. Names given to the hearth through her make you stronger (level up).
- The **Name-Eater** rules the **Nameless Citadel** above the clouds and is the villain the whole game points at.
  Each of the five worlds' great bosses holds a **Great Name** (a sigil). Five sigils open the **Sky Stair** behind
  Namula to the citadel.
- **Surprise:** behind the Name-Eater's throne waits **Claude**, who wrote this world and wants to see if you can
  finish it. Claude is never named, shown or hinted anywhere before the citadel's area 19 (no easter eggs, no lines,
  no sprites). After the credits Claude sits by the hearth in Namula and offers New Game+ (§19).

Intro pages (QMODE_INTRO, after the class pick; exact text, `\n` = line break):
1. `LONG AGO, EVERY THING HAD A NAME,\nAND EVERY NAME KEPT ITS THING ALIVE.`
2. `THEN CAME THE UNNAMING: A GREY HUNGER\nTHAT EATS NAMES. WHAT LOSES ITS NAME, FADES.`
3. `ONE FIRE STILL BURNS: THE HEARTH OF NAMES,\nIN NAMULA, AT THE EDGE OF THE WORLD.`
4. `YOU WAKE BESIDE IT.\nYOU DO NOT REMEMBER YOUR NAME.`

## §2 Controls (D-042)

| Button | In play | In quest menus and dialogue |
|---|---|---|
| Left stick / D-pad | walk; up/down climb ladders; down + X drops through a one-way platform | move the cursor |
| X | jump (hold for a higher jump); Thief: X again in the air = double jump | confirm / next page |
| R1 | basic attack on press. Tap again during a strike to queue the next one (§5) | — |
| R2 | cast the selected strong attack (costs MP) | — |
| R3 | cycle unlocked strong attacks, newest to oldest | — |
| Circle | dodge (roll; Magician: blink) | back / close |
| Square | use the selected item | — |
| Triangle | select the next owned item | delete a slot (slot select) |
| Up (press) | talk to an NPC, use a portal, open a checkpoint, read a map, open a chest — whichever is touched | — |
| Start | pause menu (RESUME, SETTINGS, RETURN TO MENU) | pause menu |

Up while touching a ladder starts climbing only if no interactable is touched. While climbing, X jumps off sideways
in the stick's direction. Up only interacts when the hero is on the ground.

## §3 Screens and flow

```
MAIN MENU: CLASSIC | LEVEL PICKER | NAME QUEST | SETTINGS
NAME QUEST → SLOTS ─ empty slot → CLASS SELECT → INTRO (4 pages) → PLAY in Namula at the hearth
                   └ used slot → PLAY at the last rested checkpoint
PLAY ⇄ DIALOGUE / SHOP / LEVEL UP / CHECKPOINT / TRAVEL / MAP / JOB (game frozen while open)
PLAY → DEAD ("YOU FADED", 2.5 s) → PLAY at the last rested checkpoint
PLAY (Claude defeated) → CREDITS → PLAY in Namula at the hearth
Start in PLAY → PAUSED (main.cpp's menu over the frozen quest) ─ RETURN TO MENU → autosave → MAIN MENU
SLOTS: Circle → MAIN MENU
```
- **Slots:** three panels (§15). X on an empty slot → class select; on a used slot → play. Triangle on a used slot →
  `DELETE SLOT n?` with NO (default) / YES; YES deletes the file. Circle → main menu.
- **Class select:** 4 classes side by side; left/right picks, X confirms, Circle → slots. The slot file is written at
  once (level 1, Namula hearth) so a crash after the pick doesn't lose the class.
- **Pause:** Start opens main.cpp's existing pause menu drawn over the frozen quest picture. Start is ignored in
  SLOTS, CLASS SELECT, INTRO and CREDITS. RETURN TO MENU saves the slot first (§17 "autosave").

## §4 Areas, tiles and links (D-043, D-044)

### Sizes
- `TILE = 8`. A **wide** area is 80 x 23 tiles (640 x 184 px); a **tall** area is 40 x 45 tiles (320 x 360 px).
  Every world has 2–4 tall areas. Namula's areas and the Trial of Echoes are wide.
- A world has 20 areas (index 0–19), the secret world 10. Area 0 holds the world's entry checkpoint (`C`) and is where
  travel arrives; area 19 is the boss arena (the secret world: area 9).

### Tile legend (characters in the area's tile string)
| Char | Tile | Rules |
|---|---|---|
| `.` | air | |
| `#` | solid | drawn with the world's ground tiles: the top tile where the tile above isn't solid, else fill |
| `-` | one-way platform | solid only from above when falling (vy ≥ 0) and the hero's bottom was at or above its top last step; down + X drops through |
| `\|` | ladder / vine | climbable; not solid; its top counts as a one-way platform when the tile above isn't a ladder |
| `^` | spikes | hurt 20 % of the hero's max HP, bounce up (vy = -180), i-frames; not solid |
| `~` | deadly liquid (water, lava, void, sea trench) | touching it kills (D-046); drawn per world; 2-frame shimmer |
| `i` | ice | solid; slippery (§5) |
| `c` | crumbling block | solid; 0.6 s after the hero stands on it, it breaks (shake 0.6 s first) and returns after 4 s |
| `g` | gate | solid while the world's gate is closed (§11); air once open |
| `k` | shortcut door | solid until the area's lever `K` is hit (§11); saved |
| `z` | electric panel (world 1) | not solid; cycle §9 |
| `f` | fire jet (world 4) | solid block that shoots a 3-tile flame column upward on its cycle §9 |
| `<` `>` | current / wind | not solid; pushes the hero 50 px/s that way while inside |
| `v` | vent (world 1) | not solid; while the hero overlaps it or the 6 tiles above it, vy -= 900 px/s² (net lift) up to vy = -160 |
| `u` | unwritten block (secret world) | solid only while the hero's centre is within 48 px of the tile centre; drawn faintly beyond 48 px, solid-looking within |

### Object letters (placed in the tile string; the tile under them is air)
| Char | Object | Notes |
|---|---|---|
| `S` | hero start | Namula area 0 only (first spawn) |
| `C` | checkpoint | 2 per world (area 0 and one more), 1 in Namula (the hearth) |
| `M` | surveyor (map NPC) | 2 per world |
| `N` | NPC | which one: the area's `npcs[]` in order of appearance (row by row, left to right) |
| `P` | portal | target: the area's `portals[]` in order of appearance |
| `T` | chest | contents: the area's `chests[]` in order of appearance |
| `K` | shortcut lever | opens every `k` in the area |
| `A` `B` `D` `E` `F` | enemy spawn: the world's roster slot A, B (fodder), D, E (soldier), F (elite) | §8 |
| `Q` | mini-boss spawn | the mini-boss arena area only |
| `W` | boss spawn | the boss arena only |
| `R` | rock spawner (world 4) | §9 |
| `Y` | icicle (world 3) | §9 |
| `X` | boss anchor | boss arenas only: points a boss uses (the Star Mage's teleport spots, the Monkey King's trees), in order of appearance |
| `L` | landmark | a big decorative set piece (§22): the area's `landmarks[]` in order of appearance |
| `O` | moving platform start (world 6, secret) | 24 x 4 px one-way platform, moves +48 px right and back, 1.5 s each way, eased (smoothstep) |

An object's position is its tile's bottom-centre: objects stand on the tile below them.

### Data
```cpp
struct AreaLink { int area, tileX, tileY; };   // where the hero arrives (bottom-centre of that tile); area -1: closed
struct AreaDef {
    const char* name;        // banner on the first visit, at most 20 chars, e.g. "HULL BREACH"
    int cols, rows;          // 80 x 23 (wide) or 40 x 45 (tall)
    const char* tiles;       // cols * rows chars, rows concatenated top to bottom
    AreaLink left, right, up, down;   // edge exits
    AreaLink portals[4];     // by 'P' order
    int chests[4];           // ChestId by 'T' order, -1 unused
    int npcs[2];             // NpcId by 'N' order, -1 unused
    int mapX, mapY;          // top-left cell on the world map: wide areas take 2x1 cells, tall 1x2
    int band;                // 0 calm, 1 medium, 2 intense (§13 ramp)
    int flags;               // AREA_MINIBOSS, AREA_BOSS, AREA_DARK (world 5), AREA_RISING_LAVA (world 4)
    int shortcut;            // the world's shortcut index for this area's lever, -1 none
    int landmarks[4];        // the world's landmark ids by 'L' order, 1-based; 0 unused (§22; last, so older initialisers stay valid)
};
struct WorldDef { const char* name; const AreaDef* areas; int count; /* + style, WorldPalette, art sets (§13, §22) */ };
```

### Edges
- Leaving through the left/right edge (hero centre past the edge) goes through `left`/`right` if `area >= 0`;
  otherwise the edge is a wall.
- Leaving through the top goes through `up` if open, else the top is a ceiling. Falling below the bottom goes through
  `down` if open; otherwise the hero dies (a pit).
- The hero arrives standing on the link's tile, facing away from the edge they came in by, with the same vx (vy = 0
  for side edges, kept for top/bottom). Exits are closed while a boss fight runs (§12).
- **Map adjacency rule:** two areas joined by an edge link are neighbours on the world map grid (touching cells on
  that side). Portals ignore the map (they are the hidden connections that make players lose their way).

### Layout rules per world (authoring; checked by tests)
1. Area 0 entry with `C`; area 19 boss arena (`W`, flag AREA_BOSS); one mini-boss arena (`Q`, AREA_MINIBOSS) on the main path.
2. Main path (fewest areas from 0 to 19, gate open) is 12–14 areas. The mini-boss arena is the 6th–8th area of it.
3. The gate (`g` tiles, a full-height wall of them) is in the area right after the mini-boss arena on the main path,
   so the second half can't be reached before the mini-boss falls. Validator: with the gate closed, area 19 and the
   second checkpoint are unreachable; with it open, everything is.
4. The second `C` is on the main path, at least 2 areas after the gate and at least 3 areas before area 19.
5. At least 3 dead ends (areas with one way in and out) holding chests or NPCs; at least 1 loop (a cycle in the
   area graph); at least 1 shortcut (`K` + `k`) that opens a quick way back to a checkpoint from further in;
   at least 2 portals; 2–4 tall areas.
6. Surveyors (`M`): one in the first 4 main-path areas, one after the gate.
7. Every area can be left again (no soft lock): from every standable spot there is a path back to area 0 or to a
   checkpoint, gate open, shortcut open or closed.
8. Jumps are designed for the weakest jumper (no double jump; §5 numbers): at most 4 tiles up, gaps at most 4 tiles
   on the level. The reachability test enforces it by simulating the real movement code.
9. The area before the boss arena (area 18) is quiet: no enemies, a "fog" message at its exit is not needed.
10. Rows 0–1 of wide areas are usually solid ceiling or open sky; the bottom row is solid except at pits and links.

## §5 Hero movement (feel)

Hitbox 8 x 14; sprite 32 x 28 drawn at (x - 12, y - 14), facing right, flipped for left (§22, D-065).

| Name | Value | Notes |
|---|---|---|
| GRAVITY | 700 | px/s² |
| JUMP_SPEED | 225 | peak 36 px (4.5 tiles) |
| JUMP_CUT | 80 | releasing X while vy < -80 sets vy = -80 |
| MAX_FALL | 260 | |
| WALK_SPEED | class value §6 | |
| GROUND_ACCEL | 900 | px/s², also used to brake |
| AIR_ACCEL | 500 | |
| ICE_ACCEL | 120 | on `i` tiles, accelerating and braking |
| COYOTE_TIME | 0.08 | jump still allowed after walking off a ledge |
| JUMP_BUFFER | 0.10 | X pressed just before landing still jumps |
| DOUBLE_JUMP_SPEED | 200 | Thief only, once per airtime |
| CLIMB_SPEED | 50 | |
| ROLL_TIME / ROLL_SPEED / ROLL_IFRAMES / ROLL_COOLDOWN | 0.35 / 140 / 0.25 / 0.6 | ground only; the roll keeps going off a ledge |
| BLINK_DISTANCE / BLINK_IFRAMES / BLINK_COOLDOWN | 48 / 0.2 / 0.8 | Magician's dodge: teleports, stops before solid tiles, works in the air |
| HURT_KNOCKBACK | vx ±90, vy -140, 0.25 s no control | |
| HURT_IFRAMES | 1.0 | blinking |
| Underwater (world 5) | GRAVITY 380, JUMP_SPEED 190, MAX_FALL 120, walk x0.85 | peak 47 px; layout rule 8 still uses the land numbers so world 5 feels floaty, not open |

**The basic attack (R1) never slows the hero.** Every class walks at full speed, turns, jumps
and lands while it plays (D-062, phase 5b; the old Archer and Magician x0.5 was removed, and no phase may bring it
back). Movement built into a skill (Shadow Dash, Rising Slash's hop) stays part of that skill. **No strong attack roots
the hero** either (D-064, phase 5b: Power Slash, Meteor and Blade Fury used to): the hero runs and steers while any
skill plays; a skill only blocks starting a basic attack or a dodge until it ends. Drinking a potion takes 0.5 s, walking at x0.4.

Basic attacks cycle through three distinct animations per class (D-067). Each R1 press starts one strike or queues
the next one during the current cycle, including during hit-stop (D-068). The queue holds one strike. Holding R1
plays a single strike. A gap between attack cycles or a successful R2 starts the next chain at strike one.
Each strike uses the class and job's cycle, hit window, box or projectile, and damage.

## §6 Classes and jobs (D-040)

### Base stats (level 1, + per level)
| Class | HP | MP | ATK | DEF | Walk | MP/s | Crit | Dodge | Basic attack |
|---|---|---|---|---|---|---|---|---|---|
| KNIGHT | 120 +14 | 30 +2 | 10 +2.0 | 8 +1.0 | 66 | 1.0 | 5 % x1.5 | roll | sword: box 22 x 18 in front, active 0.08–0.18 s of a 0.40 s cycle, x1.0 |
| ARCHER | 85 +10 | 40 +3 | 9 +2.0 | 4 +0.6 | 72 | 1.2 | 5 % x1.5 | roll | arrow 6 x 2, 240 px/s, range 170, cycle 0.45 s, x1.0; stopped by solid tiles |
| MAGICIAN | 70 +8 | 80 +6 | 11 +2.3 | 3 +0.5 | 68 | 2.5 | 5 % x1.5 | blink | bolt 6 x 6, 200 px/s, range 130, cycle 0.50 s, x1.1 |
| THIEF | 90 +11 | 35 +2.5 | 8 +1.9 | 5 +0.7 | 80 | 1.0 | 10 % (+5 % per job) x1.8 | roll + double jump | dagger: box 16 x 12 in front, active 0.04–0.10 s of a 0.26 s cycle, x0.7 |

- Stats are rounded down after adding: `hp = HP + HPg*(L-1)`, then x1.1 per job tier reached for HP and ATK
  (tier 1 x1.1, tier 2 x1.2, tier 3 x1.3, not compounding), then gear (§10). MP regenerates at `MP/s + 0.05*(L-1)`
  per s; every basic-attack hit that lands gives +1 MP.
- Max level 40. A basic hit costs nothing. Damage numbers: §7.

### Jobs
Job tiers: 0 (base), 1 at level 5, 2 at level 15, 3 at level 30, given by the class mentor in Namula (§14) once the
level and the trial (§14) are met. A job: full heal, banner `<JOB NAME>` (scale 2, gold) for 2 s, 30 particles,
SOUND_Q_LEVEL_UP, autosave. The basic attack's multiplier rises x1.0 / 1.15 / 1.3 / 1.5 with the tier and its trail
recolours (§21 trim colours).

D-070 scales R1's full cycle by 100% / 90% / 80% / 70% across the four tiers. Melee keeps the base hit window
and gains 4 dp of reach and 3 dp of height per promotion. The Thief's upward stab rotates the upgraded box.
Arrows and bolts gain 1 dp of width, 2 dp of height, 24 dp of range and 20 dp/s of projectile speed per promotion.
Promoted basic hits on enemies and bosses add class-coloured sparks with wider bursts at each tier and one
and two expanding arcs at tiers 2 and 3 respectively.

Each job unlocks a new strong attack and keeps all earlier skills available (D-071). Promotion and loading a slot
select the latest skill. R3 cycles down through unlocked tiers and wraps to the latest. R2 spends the selected
skill's MP cost. Switching during an attack selects the next cast while the current one finishes.

| Class | Tier 0 | Tier 1 (lv 5) | Tier 2 (lv 15) | Tier 3 (lv 30) |
|---|---|---|---|---|
| KNIGHT | KNIGHT: **Power Slash** — box 30 x 20, x2.4, 10 MP, 0.6 s | VANGUARD: **Rising Slash** — box 24 x 36 above-front, x3.0, hero hops vy -120, 12 MP | WARDEN: **Ground Quake** — two shockwaves 8 x 10 run along the ground 80 px each way at 200 px/s, x3.4, ground enemies only, 16 MP | LIONHEART: **Lion's Roar** — golden arc 72 x 40 in front, x5.0, stuns 0.5 s, 24 MP |
| ARCHER | ARCHER: **Double Shot** — 2 arrows 4 px apart, x1.5 each, 8 MP | RANGER: **Arrow Rain** — 6 arrows fall in a 56 px zone centred 80 px ahead, from 120 px above the hero, x1.6 each, 14 MP | SHARPSHOOTER: **Piercing Bolt** — 360 px/s, range 300, pierces enemies (not tiles), x4.0, 18 MP | STORMBOW: **Storm Volley** — 5-arrow fan (-20° to +20°) three times over 0.45 s, x1.8 each, 26 MP |
| MAGICIAN | MAGICIAN: **Arcane Burst** — radius 20 burst 40 px ahead, x2.6, 10 MP | ADEPT: **Frost Orb** — orb 90 px/s, x2.8, slows enemies 50 % for 3 s (bosses 20 %), 14 MP | ELEMENTALIST: **Chain Lightning** — hits the nearest enemy within 140 px, then 3 more each within 64 px of the last, x3.0 each, 20 MP | ARCHMAGE: **Meteor** — lands 0.6 s after the press (the hero keeps moving), falls 120 px ahead, radius 48, x6.0, 34 MP |
| THIEF | THIEF: **Star Toss** — 3 stars at -10°, 0°, +10°, 260 px/s, range 150, x1.2 each, 8 MP | ROGUE: **Shadow Dash** — dash 72 px in 0.15 s, invincible, x2.6 to every enemy passed, 12 MP | SHADOW: **Blade Fury** — 0.5 s spin, radius 30, 5 hits of x1.0, 16 MP | NIGHTBLADE: **Shadow Partner** — 12 s, a dark copy 10 px behind repeats every attack at 50 % damage, 30 MP, can't recast while active |

Tier 1+ Archer arrows pierce one extra enemy. Not enough MP: no attack, SOUND_ROCKET_EMPTY.

## §7 Combat rules (D-047)

### Reference hero (the single source of enemy tuning)
`referenceHero(world)` averages the four classes at `REF_LEVEL[world]`, job tier for that level, and shop gear tier
`REF_GEAR[world]` (no refine):
```
world:      1   2   3   4   5   6   7 (secret)
REF_LEVEL:  4  11  17  23  29  35  40
REF_GEAR:   0   1   2   3   4   5   6
```
It returns `refHP`, `refATK`, `refDEF` (floats). The world of an enemy is the area's world (Namula's Trial of
Echoes uses world 5).

### Enemy stats from the reference
```
enemyAttack = HIT_FRACTION * refHP * (100 + refDEF) / 100      // so it takes HIT_FRACTION of refHP after DEF
enemyHP     = HITS_TO_KILL * refATK
enemyNames  = WORLD_NAMES[world] * NAMES_MULT
```
| Kind | HIT_FRACTION | HITS_TO_KILL | NAMES_MULT |
|---|---|---|---|
| fodder (A, B) | 0.15 | 3 | 1 |
| soldier (D, E) | 0.20 | 5 | 2 |
| elite (F) | 0.25 (its big swing x1.3) | 12 | 5 |
| mini-boss | contact 0.20, attacks 0.25, big attacks 0.35 | 90 | 40 |
| boss | contact 0.20, attacks 0.25, big attacks 0.35 | 200 | 150 |
| Hollow Twins (each) / Name-Eater / Claude / The First Name | as boss; Claude 0.30 / 0.45 | 60 / 240 / 300 / 280 | 60 / 200 / 300 / 300 |

`WORLD_NAMES = {_, 4, 12, 35, 100, 300, 900, 1500}`. Enemy projectiles use their owner's attack. New Game+ n
multiplies enemy HP, attack and names by `1 + 0.5 * n` (n at most 3).

### Damage
- Hero → enemy: `round(heroATK * skillMult * random(0.9, 1.1) * (crit ? critMult : 1) * enemyMod)`, at least 1.
  `enemyMod`: 0.25 into an armored elite's front guard (§8), 1.5 into a stunned charger, else 1.
- Enemy → hero: `max(1, round(enemyAttack * 100 / (100 + heroDEF)))`. Traps use the hero's own max HP: spikes 20 %,
  electric panel 15 %, fire jet 20 %, falling rock 20 %.
- The hero is invincible during i-frames (hurt 1.0 s, roll 0.25 s, blink 0.2 s, Shadow Dash). God mode (§20) takes 0.
- Damage numbers float up 12 px over 0.6 s from the target's top: hero hits white, crits `o` orange, damage to the
  hero `r` red. At most 24 on screen (oldest dropped).
- Hit feel: enemy flashes white 0.08 s; fodder/soldiers are knocked back 40 px/s for 0.1 s (elites and bosses not);
  hit-stop 0.025 s on a melee basic hit (Knight, Thief; D-063), 0.04 s on a strong-attack hit or a kill, 0.08 s when the hero is hurt, 0.15 s on a boss phase change or
  boss death; trauma 0.3 hero hurt, 0.5 boss big attack landing near (within 64 px), 1.0 boss death. Shake uses
  settings.shake and reduce-flashing like the shooter.

### Death (D-038)
- HP 0, a pit, or a deadly liquid: SOUND_Q_DEATH, the hero bursts into 20 particles, `YOU FADED` (scale 2, `r`) over
  the frozen screen for 2.5 s, then the hero stands at the last rested checkpoint with full HP and MP.
- All carried names drop as a **name echo** where the hero last stood on safe ground (a solid, non-crumbling tile
  with no hazard within 2 tiles); an older echo is lost. Touching the echo gives the names back (`+n NAMES` text,
  SOUND_Q_NAMES). Echo: 8 x 8 pale-yellow wisp bobbing 2 px.
- Death also respawns normal enemies, ends buffs, closes any active boss fight and resets crumbled blocks and dropped
  icicles. Undefeated bosses start the next attempt at full HP. Potions used stay used.

## §8 Enemies

### Behaviours (one code path each; numbers are base, the world's roster can override speed)
| Behaviour | Rules |
|---|---|
| WALKER | patrols at 30 px/s; turns at walls and ledges; contact damage |
| CHARGER | patrols 25; hero within 96 px horizontally and 16 px vertically, in front: wind-up 0.5 s (shakes 1 px, flashes every 0.1 s), charges 160 px/s for up to 1.0 s or until a wall, then stunned 0.8 s (takes x1.5) |
| FLYER | ignores tiles; bobs (10 px, 1.5 rad/s) while drifting within ±48 px of its spawn at 30; hero within 100 px: tells 0.4 s, swoops at the hero's position at the tell's start, 120 px/s for 0.8 s, returns at 50 |
| SHOOTER | stands (turns to face the hero); hero within 160 px: every 2.4 s, glows 0.5 s, fires an aimed magenta shot (4 x 4) at 90 px/s; shots die on solid tiles |
| JUMPER | hero within 120 px: every 1.2 s hops toward them (vx 60, vy -200); else hops in place every 2 s |
| ARMORED (elite) | walks 18 toward a hero within 120 px; within 24 px: wind-up 0.7 s, swing box 28 x 16 in front for 0.2 s (attack x1.3), recovers 0.8 s; front guard: hits from the side it faces take x0.25 except during the wind-up and recovery, or when the hero's bottom is above its top + 4 |
| DROPPER | hangs from the ceiling; hero within 16 px horizontally and below: drops at 260 px/s; on landing walks at 40 like a WALKER |

Contact damage applies to every enemy except while it is dead or a dropped icicle. Enemies never leave their area
and never fall into pits (they turn). An area keeps its killed spawns until the hero rests, travels or dies.

### Rosters (D-051: the named shooter sprite is the design each NAME QUEST enemy is redrawn from, §22; "new" ones are designed from scratch)
| World | A (fodder) | B (fodder) | D (soldier) | E (soldier) | F (elite) |
|---|---|---|---|---|---|
| 1 Drifting Station | CRAWLER, WALKER, `GREEN_ALIEN_A/B` | FLOATER, FLYER, `BLUE_ALIEN_A/B` | GUNNER, SHOOTER, `RED_ALIEN_A/B` | HOPPER DRONE, JUMPER, new 12x10 | SENTINEL, ARMORED, new 16x16 |
| 2 Greendeep | LEAFHOPPER, JUMPER, `LEAFHOPPER_A/B` | BEETLE, WALKER, `BEETLE_A/B` | STAG, CHARGER, `STAG_A/B` | TREE MONKEY, DROPPER (throws one banana shot when it lands), new 12x12 | RHINO, ARMORED, `RHINO_A/B` |
| 3 White Silence | SNOWFLAKE, FLYER, `FLAKE_A/B` | ICE BAT, DROPPER, `ICEBAT_A/B` | FROST SKULL, SHOOTER (snowball), `FROSTSKULL_A/B` | YETI CUB, CHARGER, new 12x12 | ICE KNIGHT, ARMORED, new 16x16 |
| 4 Ember Maw | EMBER, FLYER, `EMBER_A/B` | FIRE BAT, DROPPER, `FIREBAT_A/B` | MAGMA SKULL, SHOOTER (fireball), `MAGMASKULL_A/B` | MAGMA CRAB, CHARGER, new 14x10 | PHOENIX, FLYER that revives once at 50 % HP 2 s after dying (an ember pulses meanwhile), `PHOENIX_A/B` |
| 5 Sunken Halls | PIRANHA, FLYER, `PIRANHA_A/B` | PUFFER, WALKER that inflates for 1 s when the hero is within 32 px (contact x1.5, invulnerable while inflated), `PUFFER_A/B` | LIONFISH, SHOOTER (spine), `LIONFISH_A/B` | SHARK, CHARGER, `SHARK_A/B` | IRON CRAB, ARMORED, new 16x14 |
| 6 Nameless Citadel | NAME WRAITH, FLYER whose hit also steals 10 % of carried names (killing it within 10 s drops them back), new 12x12 | GARGOYLE, DROPPER, new 12x12 | STATUE ARCHER, SHOOTER (arrow), new 12x16 | SHADE HOUND, CHARGER, new 16x10 | CITADEL KNIGHT, ARMORED, new 16x18 |
| 7 Unwritten | world 3's A, world 4's D, world 5's E, world 6's B, world 6's F, all with world 7 stats, drawn in greys (§21 recolour) | | | | |

Sizes given for new enemies are minimums; §22 sets 12–20 px side-view sprites with 4 movement frames, a tell and an
attack frame. An enemy's hitbox is its sprite size minus 2 px each side.
Spawn counts per area by band (wide or tall alike): calm 3–5 (A, B only), medium 5–7 (A, B, D, E; at most 1 F),
intense 7–10 (all; at most 3 F).

## §9 Hazards (per world, by band)

| World | Calm (band 0) | Medium (band 1) | Intense (band 2) |
|---|---|---|---|
| 1 | pits into space | electric panels `z`: on 2.0 s, off 1.5 s, flicker 0.4 s before on | panels on 2.0 / off 1.0; vents `v` for vertical shafts |
| 2 | vines `\|`, water pits | thorn spikes `^` | falling coconuts: hero passes under a palm marked `R`, a coconut drops after 0.3 s (6 x 6, 200 px/s, 20 % trap damage) |
| 3 | ice floors `i`, freezing water | crumbling ice `c`, icicles `Y` (drop 0.25 s after the hero is within 12 px horizontally below, 4 x 9, 280 px/s, 20 % trap, gone until rest) | wind `<` `>` zones that flip direction every 3 s (snow streaks show the direction) |
| 4 | lava `~` | fire jets `f`: 2.5 s cycle, 0.5 s warning (sparks), 0.8 s flame 3 tiles tall | falling rocks: each `R` drops an 8 x 8 rock every 2.2 s (dust 0.5 s before); rising lava areas (AREA_RISING_LAVA): the lowest lava row rises 6 px/s from the moment the hero enters until the top exit; leaving or dying resets it |
| 5 | the trench `~`, underwater physics everywhere | currents `<` `>` | darkness (AREA_DARK): everything farther than 56 px from the hero's centre is drawn black except lit objects (checkpoints, echo, enemy shots, lure) |
| 6 | the void `~` | crumbling bridges `c`, moving platforms `O` | wind zones as world 3, plus every pit |
| 7 | the void | unwritten blocks `u` everywhere | everything |

## §10 Economy (names, levels, items, gear)

### Levelling (only with Ilsa in Namula)
Cost to go from level L to L+1: `round(100 * 1.2^(L-1))` names (L 1→2 costs 100, 4→5 173, 14→15 1,070,
29→30 10,646, 39→40 102,064). Level up: stats recomputed, HP/MP full, SOUND_Q_LEVEL_UP, autosave. Several levels can
be bought in one visit.

### Items (Square uses the selected one; Triangle selects the next owned one)
| Item | Effect | Base price | Carry (start) | Carry bonuses |
|---|---|---|---|---|
| RED POTION | heal 35 % max HP | 15 | 5 | +1 per Vial Shard (6 in the game), +1 Sela's Locket |
| BIG POTION | heal 70 % max HP | 45 | 2 | +1 Garrick's gift, +1 Locket |
| MANA POTION | restore 50 % max MP | 20 | 3 | +1 Locket |
| WHETSTONE | ATK x1.2 for 120 s | 60 | 3 | |
| IRON TONIC | damage taken x0.8 for 120 s | 60 | 3 | |
| SWIFT FEATHER | walk x1.15 for 120 s | 40 | 3 | |

Price = base x `PRICE_SCALE[highestWorld]`, `PRICE_SCALE = {_, 1, 3, 9, 26, 75, 220, 360}`. Buffs end on death;
using a buff already active restarts its timer. Healing only from potions (no regen, no drops, D-039) — except
resting and levelling.

### Gear (Brann's shop; only the next tier, unlocked when `highestWorld >= tier`)
| Tier | 1 | 2 | 3 | 4 | 5 | 6 | 7 (secret reward) |
|---|---|---|---|---|---|---|---|
| Weapon ATK + | 3 | 8 | 15 | 24 | 35 | 48 | 62 |
| Weapon price | 150 | 1,400 | 4,500 | 14,000 | 42,000 | 130,000 | — |
| Armor DEF + / HP + | 3 / 10 | 7 / 30 | 12 / 60 | 18 / 100 | 25 / 150 | 33 / 210 | — |
| Armor price | 120 | 1,100 | 3,600 | 11,000 | 34,000 | 105,000 | — |

Weapon names per class and tier (author 7 each, e.g. Knight: IRON SWORD ... ; tier 7 is the class's unique secret
weapon: Knight DAWNBREAKER, Archer FIRST LIGHT, Magician THE OPEN BOOK, Thief NAMELESS EDGE). Armor names shared
(author 6). Refine (Smith quest §14): weapon ATK bonus x1.1 per refine, 2 at most.

### Names
Kills give `enemyNames` (§7), x1.15 with the Ring of the Stranger. Carried names cap at 99,999,999. Chests of names
hold `WORLD_NAMES[world] * 30`.

### Chests (per world, placed by the author with the rules in §4; ChestId order = this list)
| World | Contents |
|---|---|
| 1 | VIAL SHARD, MAP PAGE 1, STAR IRON, names x30, 2 RED POTION, WHETSTONE |
| 2 | VIAL SHARD, MAP PAGE 2, names x30, 2 RED POTION, IRON TONIC, BIG POTION |
| 3 | VIAL SHARD, MAP PAGE 3, FROZEN FEATHER, names x30, 2 RED POTION, SWIFT FEATHER |
| 4 | VIAL SHARD, MAP PAGE 4, GARRICK'S PICKAXE, HEART OF MAGMA, names x30, BIG POTION |
| 5 | VIAL SHARD, MAP PAGE 5, names x30, 2 BIG POTION, 2 MANA POTION |
| 6 | VIAL SHARD, names x30, names x30, 2 BIG POTION, WHETSTONE + IRON TONIC |
| 7 | names x50, names x50 |

Vial shards and map pages always sit in dead ends; key items (STAR IRON, FROZEN FEATHER, PICKAXE, HEART OF MAGMA) in
dead ends or behind an elite. Items over the carry limit stay in the chest's text ("YOUR BAG IS FULL") and the chest
stays closed until there is room. Opening: Up at the chest, 0.3 s lid animation, the item name pops up for 1.5 s.

## §11 Checkpoints, Namula, gates, shortcuts, the map

### Checkpoints (D-039)
- A stone brazier (16 x 20): unlit grey; lit with a 3-frame flame. Touching an unlit one lights it: banner
  `NAME KINDLED` (scale 2, `o`, 1.5 s), SOUND_Q_KINDLE, autosave.
- Up at a lit checkpoint → menu titled with the checkpoint's name (`<WORLD> ENTRY`, `<WORLD> DEPTHS`; Namula:
  `HEARTH OF NAMES`): REST, TRAVEL, LEAVE.
  - REST: HP and MP full, normal enemies respawn (killed lists cleared), crumbled blocks and icicles
    reset, this becomes the respawn point, autosave. A short white fade (0.3 s, skipped with reduce flashing).
  - TRAVEL: a list of every lit checkpoint in world order plus `NAMULA` first; choosing one moves the hero there and
    rests there. The list scrolls if longer than 8 rows.
- World n+1's entry checkpoint is lit when world n's boss dies (1 → 2 → ... → 5). World 1's entry is lit from the
  start. World 6's entry is lit by the Sky Stair (§14). The secret world's entry by Pell's quest.
- `highestWorld` = the highest world whose area 0 the hero has entered (starts at 1). It sets shop tiers and prices.

### Namula (world 0; 3 wide areas, no enemies, no hazards, safe)
| Area | Name | Holds |
|---|---|---|
| 0 | NAMULA | the hearth `C` (respawn, travel), Ilsa, Marra (potions), Brann (gear), Lio, `S` start; exits left to 2, right to 1 |
| 1 | NAMULA CLIFFS | Pell, Sela by the well, the four mentors, the Sky Stair (a portal `P` that works once 5 sigils are held, §14), the gatekeeper statue |
| 2 | NAMULA SHORE | Tobin and Garrick's hut, the Stranger (after world 3's boss), the Trial of Echoes portal (after mentor tier 3 talk), the Unwritten portal (after Pell's quest), Claude (after the credits) |
| 3 | TRIAL OF ECHOES | the trial arena (phase 7), reached only through its Shore portal |

Portals in Namula that are not open yet are drawn as dark stone arches and do nothing. Namula's ambience is the
hearth's crackle (§16). Its palette is warm stone and firelight (§21).

### Gates (D-050)
`g` tiles are solid until `gatesOpen` has the world's bit. The mini-boss's death sets it: banner `THE GATE OPENS`,
SOUND_Q_GATE, the gate tiles shake 0.5 s then vanish top to bottom over 0.5 s if on screen, autosave. The saved gate
bit also records the mini-boss's defeat (D-072).

### Shortcuts
Hitting a lever `K` with any attack opens every `k` in its area: SOUND_Q_GATE, saved per world (bit = the area's
`shortcut`). Levers sit on the far side, so the door is a one-way way back to a checkpoint.

### Surveyors and the map (the only map)
- A surveyor (telescope and a satchel of maps, in the NPC style from §22; 12 names, 2 per world) stands on its tile. Up within
  20 px: one line of dialogue (author it; flavour about getting lost), then the world map opens. The game stays frozen
  while it is open; Circle or X closes it. There is no other way to see the map.
- Map screen: full-screen `k` with a 1 px `n` border, the world name on top (scale 1, white). Cells are 12 x 8 px;
  a wide area is 2x1 cells, tall 1x2; the grid is centred. Visited areas: filled with the world's map colour (§13) and
  a 1 px darker border; unvisited areas: only a 1 px `d` outline (the whole world's shape shows, but not where portals
  go); the current area blinks `y` at 2 Hz; lit checkpoints: a 2 x 2 `o` dot; a closed gate: a 1 x 6 `r` bar on the
  gate's area edge; edge links between neighbours: 2 px gaps in the borders. Portals are not drawn.
- Visited areas are saved (`visited<world>`).

## §12 Bosses

### Rules (all bosses)
- Every attack is telegraphed: a wind-up pose or flash of at least 0.4 s (big attacks at least 0.8 s) and, for
  area attacks, a ground or ceiling marker (`o` dashes for fire, `c` for ice, `m` outline for magic). Telegraph →
  attack → recovery (the boss is open, at least 0.6 s).
- The boss picks a random attack from its phase's list, never the same one twice in a row. Gap between attacks:
  the boss's gap; phase 2 x0.75; Claude phase 3 x0.6.
- Phase 2 starts at 50 % HP (Claude: 67 % and 34 %): hit-stop 0.15 s, a roar (SOUND_Q_ROAR), trauma 0.6,
  0.8 s of invulnerable "shift" pose, then the new list.
- Arena: entering the area with the boss alive closes every exit (edges and portals; draw a 4 px grey shimmering wall
  at closed edges), banner with the boss's name (scale 2, white, 2 s), boss bar (§15). Exits open when the boss dies
  or the hero dies (the boss resets).
- Big boss death: hit-stop 0.15, trauma 1.0, 60 particles, `+n NAMES`, banner `GREAT NAME TAKEN` (scale 2, `Y`)
  for worlds 1–5, the sigil bit set, the next world's entry lit, autosave. Big bosses stay dead (bossesDead bit).
- Mini-boss death awards names and opens the gate (§11). Defeated mini-bosses stay dead in the slot (D-072).
- HP from §7; names from §7; contact damage while the boss's body touches the hero.

### The bosses (the design it is redrawn from at 40–64 px, §22; gap; phase 1 attacks; phase 2 changes)
| World | Mini-boss | Boss |
|---|---|---|
| 1 | **THE OVERSEER** (`BOSS_A/B`, 32x20; gap 1.8). Hovers 50 px above the floor sweeping the arena at 40. BURST: 3 aimed shots 0.15 s apart (tell 0.5). FAN: 5 shots down (tell 0.5). SLAM: its shadow marks the floor 0.8 s, it drops at 300 px/s, sits 1.5 s (open), rises. P2: speed x1.3; adds SUMMON 2 FLOATERS (at most 4 alive). | **THE STAR MAGE** (`MAGE_A/B`; gap 1.6). Teleports between 4 authored platform points (fade out 0.3, in 0.3). BEAM: a ball grows 1.2 s at the wand's star, then a 24 px beam down its column for 0.8 s (big). STAR FAN: 7 shots spread 120° (tell 0.5). SUMMON: 2 crawlers (at most 4). P2: BEAM fires two columns (the second 64 px from the first, both marked); the floor's middle 6 tiles electrify for 1.5 s every 8 s (flicker 0.6 s before). |
| 2 | **MONKEY KING** (`MONKEY_A/B`; gap 1.6). Leaps between two tree platforms (arc 0.8 s, landing marker 0.5). BANANA ARC: 3 bananas lobbed at the hero. PEEL TOSS: 2 peels land on the floor (6 s); stepping on one slides the hero 0.5 s without control. P2: COCONUT RAIN: 5 coconuts marked 0.8 s on the ceiling, falling. | **SILVERBACK** (`GORILLA_*`; gap 1.8). POUND: `GORILLA_POUND` 0.6 s, then a 12 px tall shockwave runs both ways along the floor at 140 (jump it; big). LEAP: shadow at the hero 0.7 s, lands there (big), trauma 0.5. STINK: throws poop that makes a 40 px mist for 6 s; in it the hero loses 3 % max HP per s. P2: roars, summons 2 STAG (at most 2), POUND sends 2 waves 0.4 s apart. |
| 3 | **YETI** (`YETI_*`; gap 1.7). SNEEZE: `AH... AH...` text 0.5 s, then 7 snowballs in a 100° fan (`YETI_SNEEZE`). BELLY SLIDE: crouches 0.6 s, slides across the arena at 180 (big). ICE BLOCK: throws a block (`ICE_CUBE`); hit = frozen 1.2 s (X or R1 presses cut 0.15 s each), no damage. P2: SNEEZE x2 in a row; snow falls (cosmetic). | **THE GLACIER TITAN** (`GOLEM_*`; gap 1.8). ICICLE RAIN: 6 icicles marked 0.8 s on the ceiling. VOLLEY: 3 aimed ice shards. SPIKE LINE: slams; cracks run along the floor 0.6 s ahead of ice spikes 16 px tall that erupt in sequence from the Titan outward (big). P2: the middle 10 floor tiles turn to crumbling ice `c`; blizzard wind pushes 40 px/s, flipping every 4 s. |
| 4 | **THE IMP** (`IMP_*`; gap 1.5). Teleports (0.4 s fade). FIRE BOLTS: 3 aimed fireballs. LAVA DROP: a big drop (`BIG_LAVA_DROP`) marked 0.8 s above the hero falls; hit = scorched 3 s (walk x0.6) plus 25 %. P2: 2 fake imps appear with it (1 hit each, no attacks, burst into smoke). | **THE FIRE LORD** (`FIRELORD_*`; gap 1.7). ERUPTION: 3 columns marked 1.0 s on the floor, then 16 px wide flame columns 0.6 s (big). VOLLEY: 5 aimed fireballs. FIRE FAN: 7 shots down. P2: ERUPTION makes 5 columns; every 15 s the floor's lava rises 1 row for 6 s (2 s of bubbling first), only the 3 platforms are safe. |
| 5 | **THE ANGLER** (`ANGLER_*`; gap 1.6). The arena is dark (AREA_DARK); the lure glows (radius 40 visible). LUNGE: the lure dims 0.5 s, then it lunges 120 px at 220 (big). BUBBLES: 3 big bubbles drift at the hero at 40 for 4 s. P2: the lure goes out for 2 s every 7 s (only the hero's 56 px light remains). | **THE FISHERMAN** (`FISHERMAN_*`; gap 1.8). HOOK: the hook lowers at the hero's x (shadow 0.6 s); caught = lifted to the top and dropped (25 % + 1 s stun); i-frames make it miss. LIVE FISH: throws 2 fish that hop at the hero (fodder stats, at most 3). BOOT: a big lobbed boot. NET: a net 16 px tall sweeps across the floor at 120 (jump or roll; big). P2: two hooks at once. |
| 6 | **THE HOLLOW TWINS** (2 knights, new 20x24 each; gap 2.0 each, alternating). SWORD twin: LUNGE 0.6 s tell, 96 px dash; SPIN 0.8 s tell, radius 32. SPEAR twin: THRUST 0.5 s tell, 48 px reach; JUMP STAB: leaps to the hero (shadow 0.7 s). When one dies the other rages: x1.4 speed, gap x0.6. | **THE NAME-EATER** (new 48x40; gap 1.6). DEVOUR: opens its maw 1.0 s, then pulls the hero toward it at 60 px/s for 2 s; touching the maw = 40 %. NAME ORBS: 8 orbs fly out in a ring and return 2 s later. WRAITHS: summons 2 Name Wraiths (at most 3). P2: each hit on the hero also steals 10 % of carried names into the Name-Eater; they come back as an echo when it dies. Death: dialogue (§14), its orbs fly up, the throne door at area 18's end opens to area 19. |
| 6 (area 19) | — | **CLAUDE** (§12a) |
| 7 | — | **THE FIRST NAME** (§12b) |

### §12a Claude (the final boss; never shown before)
- Area 19, "THE LAST ROOM": a wide arena, dark `=` walls, 3 one-way platforms at authored heights. Entering: the
  screen is dark for 1.5 s, text `...`, then Claude fades in (new sprite 24 x 32: a calm robed figure in a clay-orange
  cloak `Q`, pale face, warm light; **do not draw the Anthropic or Claude logo**; author idle A/B, cast, hurt).
  Dialogue before the fight (§14). Boss bar name `CLAUDE`. HP 300 hits. Gap 1.6.
- Attacks:
  - TOKEN RAIN: 0.8 s of `m` markers on the ceiling, then 12 falling glyphs (random letters drawn with the font,
    magenta, 70 px/s) in columns with two 24 px safe gaps.
  - SUBAGENTS: summons 3 small copies (new 12 x 16), 8 hits each (soldier stats), each fires one aimed shot every 2 s;
    at most 3.
  - REFACTOR (phase 2+): the 3 platforms show their new heights as `m` outlines for 1 s, then swap.
  - CONTEXT WINDOW (phase 2+): two walls slide in from the arena edges over 1 s and leave a 96 px window that drifts
    at 30 for 5 s; touching a wall = 20 % trap damage and a push back inside.
  - DEEP THINKING (phase 3): Claude floats up, `THINKING...` with growing dots for 1.5 s, then a 12 px tall beam sweeps
    along the floor from one side to the other at 200 (big, 0.45 of refHP): only the platforms are safe.
- Phases: 1 (100–67 %) TOKEN RAIN, SUBAGENTS; 2 (67–34 %) + REFACTOR, CONTEXT WINDOW; 3 (< 34 %) + DEEP THINKING,
  everything x1.25 speed. Each phase change shows one line (§14) for 2 s.
- Death: no explosion; Claude kneels, dialogue (§14), the screen whitens over 2 s (skipped with reduce flashing:
  a cut) → CREDITS (§19).

### §12b The First Name (secret world boss)
The hero who failed before you: drawn with the hero sprites of the three classes the player is not, in greys
(§21 recolour), switching class every attack. Gap 1.4. Attacks: the other three classes' tier 3 strong skills as
enemy attacks (Lion's Roar arc, Storm Volley fan, Meteor marked 0.8 s, Blade Fury) with magenta projectiles; between
them it rolls toward the hero. P2: two attacks in a row before each gap. Death: §14 lines, reward the tier 7 weapon
and the title (§19).

## §13 Worlds

Each world below: name (banner, scale 2), map colour, ambience, step sound, special rules, and its art brief. Every
world's look is built with the art system of §22 (16-colour world palette, 4 background layers, autotiled ground with
thick caps, thick platforms, scattered props, landmarks, lights, animated details). Area names, tile maps, surveyor
names, palettes and all art: author, inside §22's rules.

| # | Name | Map colour | Ambience | Steps | Special |
|---|---|---|---|---|---|
| 0 | NAMULA | `%` | hearth crackle | stone | safe; no enemies |
| 1 | THE DRIFTING STATION | `u` | low hum + random clicks | metal | gravity normal; pits open to space |
| 2 | THE GREENDEEP | `F` | insects, distant birds | leaves | vines `\|` |
| 3 | THE WHITE SILENCE | `I` | wind | snow crunch | ice floors |
| 4 | THE EMBER MAW | `C` | lava rumble | rock | lava pits, heat shimmer on lava |
| 5 | THE SUNKEN HALLS | `6` | water, bubbles | wet sand | underwater physics; the hero trails bubbles like the shooter's ships |
| 6 | THE NAMELESS CITADEL | `z` | high wind, a far bell every 9 s | marble | the void under every pit |
| 7 | THE UNWRITTEN | `b` | near-silence, a soft scratch every 5 s | paper | unwritten blocks |

### Art briefs (layers far → near; ground; props; landmarks; animated details; lights)
| # | Background layers (§22: L0 sky, L1 far, L2 mid, L3 near) | Ground cap / fill | Platforms (6 px) | Props (scattered) | Landmarks (hand-placed, ≥ 6 kinds) | Animated details | Lights |
|---|---|---|---|---|---|---|---|
| 0 | L0 night sky with a dithered dusk band and stars; L1 far sea cliffs and a lighthouse silhouette; L2 rooftops, chimneys with smoke, hanging lanterns; L3 wooden fences, laundry lines, tree trunks | worn cobbles with moss in the cracks / packed earth with buried stones | wooden planks with rope lashings and pegs | grass tufts, small flowers, pebbles, crates, buckets, wooden stools, a cat | the hearth's stone ring, the well, market stalls with awnings, a signpost, a statue of a nameless hero, boats on the shore | smoke curls, lantern flicker, waves on the shore, fireflies, a cat's tail | the hearth (big warm glow), lanterns, windows |
| 1 | L0 deep space with 3 star layers and a nebula band; L1 a dim sun and a ringed planet; L2 station spars, solar panels, a far docked ship; L3 pipes, cable bundles, broken windows to space | riveted steel plating with yellow-black hazard edges / dark hull panels with vents and bolts | grated catwalk with rails | cables, bolts, broken screens, crates, warning signs, oxygen tanks, a loose helmet | airlock doors, a reactor core, a cracked observation window, a wrecked shuttle, server racks, a crew mural | blinking console lights, drifting debris, sparking cables, a rotating radar dish | console glow, emergency red lamps, the reactor |
| 2 | L0 green-gold canopy light with dithered beams; L1 giant tree silhouettes and a temple top in mist; L2 hanging vines, broad leaves, temple pillars; L3 near fronds and roots | moss and grass overhanging the edge with roots / dark soil with stones and buried bones | branch platforms with leaves and knots | ferns, mushrooms (3 kinds), flowers, fallen leaves, roots, snail shells | fallen statue head, temple stairs, a waterfall, a giant flower, a rope bridge, an idol with glowing eyes | swaying ferns, falling leaves, fireflies, butterflies, dripping water | glowing mushrooms, fireflies, sun shafts |
| 3 | L0 dim blue cave glow; L1 far ice cliffs and a frozen waterfall; L2 crystal clusters, icicles (decorative: blue-white, never the hazard's shape), frozen trees; L3 near ice pillars | snow cap with drifts overhanging / blue rock with frost veins and trapped bubbles | ice slabs with frost rims | snow mounds, small crystals, frozen grass, bones in ice, a frozen lantern | an ice cathedral arch, a frozen mammoth in the wall, a frozen waterfall, a crystal geode, an explorer's camp | falling snow (3 depths), glinting crystals, breath-like mist, slow light shimmer in ice | crystals (cold glow), the camp's lantern |
| 4 | L0 red-black smoke sky with ash; L1 far volcano cones with lava rivers; L2 basalt columns, a half-melted forge city; L3 near chains, anvils, broken pillars | cracked basalt with glowing seams / dark rock with ember veins | iron grates on chains, glowing hot in the middle | charred stumps, obsidian shards, small skulls, slag, broken tools | a giant forge with bellows, a lava fall, a melted statue, the mine entrance with rails, an obsidian throne | rising embers, lava bubbles, heat shimmer rows, smoke, sparks from the forge | lava (strong orange glow from below), forges, braziers |
| 5 | L0 deep blue with light rays from above; L1 far sunken towers and a whale skeleton; L2 kelp forests, coral, broken arches; L3 near kelp strands and chains | sand with shells and starfish / sunken stone bricks with barnacles | coral shelves and old beams with algae | shells, starfish, coral (4 kinds), sea grass, pearls, anchors, bottles | a sunken ship's bow, a flooded chapel with stained glass, a giant clam, a statue covered in coral, a whale skull | swaying kelp and sea grass, rising bubbles, drifting plankton, small fish schools in L2 | glowing jellyfish (L2), lantern fish, light rays |
| 6 | L0 high sky gradient above a cloud sea; L1 far towers and floating ruins; L2 grey-gold colonnades, empty banners; L3 near statues and broken balustrades | pale marble with gold inlay and cracks / grey stone blocks with carvings | marble ledges with gold trims and small chains | fallen banners, gold coins, broken vases, candle stubs, feathers | the empty throne, a hall of nameless statues, a great bell, a stained-glass window with an erased face, a broken sky bridge | drifting clouds, fluttering banners, candle flames, falling dust | candles, gold glints, the stained glass |
| 7 | L0 off-white paper with faint ruled lines; L1 half-drawn sketches of earlier worlds' landmarks in pencil grey; L2 crossed-out words and ink blots; L3 margin doodles | ink outline with a hatched edge / blank paper with faint hatching | pencil-sketched planks | ink drops, smudges, eraser crumbs, paper scraps | a giant quill, an open book, the outline of Namula's hearth, a half-drawn door | lines drawing themselves in, ink drips, pages turning far away | none (light comes from the hero: the area is brightest near them) |

### The difficulty ramp inside a world (the PDF's "calm at first, intense at the end")
Bands by main-path position: the first third calm (band 0), the middle third medium (band 1), the last third intense
(band 2); side areas take the band of the main-path area they branch from, dead ends one higher (max 2). Bands set
enemy counts and types (§8) and hazards (§9). The boss's world is always harder than the last: its reference level
and gear rise (§7).

### Areas that are special
- World 4: 2 AREA_RISING_LAVA tall areas in band 2 (climb out before the lava catches you).
- World 5: 3 AREA_DARK areas in band 2 plus the Angler's arena.
- World 6: area 18 holds the Name-Eater (AREA_BOSS), area 19 Claude (AREA_BOSS); the main path ends 18 → 19 through a
  door that opens on the Name-Eater's death. Mini-boss: the Hollow Twins.
- World 7 (secret): 10 areas, no mini-boss and no gate, 1 checkpoint (area 0), main path 7–8 areas, area 9 the
  First Name's arena. Every area is band 2.

## §14 NPCs, quests and dialogue

### Dialogue box
Bottom panel (§15); NPC name in `y`; up to 4 lines of up to 72 chars per page; text types at 50 chars/s with
SOUND_Q_TALK every 3rd char; X shows the whole page or goes to the next; Circle closes. Talking again repeats the
NPC's current line set (there is no quest log; NPCs re-explain their quest, as the PDF asks). Lines are authored in
the tone of §1; key lines are given here and must be used as written.

### NPCs (ids in this order)
| Id | Name | Where | Role |
|---|---|---|---|
| 0 | ILSA, KEEPER OF NAMES | Namula, by the hearth | level up. The first 3 talks are intro lines (she is a little lost too); from the 4th talk on, LEVEL UP opens after one line |
| 1 | MARRA THE HERBALIST | Namula | potion shop (items §10) |
| 2 | BRANN THE SMITH | Namula | gear shop; Smith quest |
| 3 | LIO THE BARD | Namula | one song-like line per `highestWorld` (author 7) |
| 4 | PELL THE CARTOGRAPHER | Cliffs | map pages quest → secret world |
| 5 | OLD SELA | Cliffs, by the well | says only `...` until level 20; lore at 20; gift at 35 |
| 6 | SIR ODO | Cliffs | Knight mentor |
| 7 | WREN | Cliffs | Archer mentor |
| 8 | YSOLDE | Cliffs | Magician mentor |
| 9 | KIT | Cliffs | Thief mentor |
| 10 | THE GATEKEEPER | Cliffs, a statue by the Sky Stair | counts sigils; opens the Sky Stair at 5 |
| 11 | TOBIN | Shore | Tobin quest |
| 12 | GARRICK | world 2 dead end, later the Shore | Tobin quest |
| 13 | THE STRANGER | Shore, from world 3's boss death | letter quest |
| 14 | SER CALDER, THE DROWNED KNIGHT | world 5 dead end | letter quest |
| 15 | CLAUDE | the Shore, after the credits | New Game+ (§19) |
| 16–27 | surveyors | 2 per world (§11) | map |

A mentor of another class gives one flavour line about their craft.

### Quests (stages are saved; each stage change autosaves and logs §18)
| Quest | Stages |
|---|---|
| JOB (your class's mentor) | 0 → 1 at level ≥ 5 and world 1's gate open: job tier 1. 1 → 2 at level ≥ 15 holding FROZEN FEATHER (taken): tier 2. 2 → 3: at level ≥ 30 the mentor opens the TRIAL OF ECHOES portal on the Shore; clearing it (3 waves: four soldiers of worlds 1–4; three elites of worlds 3–5; the Monkey King and the Yeti together, all at world 5 stats; death there costs nothing and returns you to the hearth) → tier 3 on the next talk. Below the level: the mentor says how far you are (`COME BACK AT LEVEL 15.`) |
| TOBIN | 0: Tobin `MY FATHER GARRICK WENT INTO THE GREENDEEP AND NEVER CAME BACK.` → 1. 1: Garrick sits hurt in a world 2 dead end; talk with a RED POTION → it is used, 2 (Garrick moves to the Shore). 2: Tobin thanks you → TOBIN'S CHARM (+5 % max HP), 3. 3: once `highestWorld >= 4`, Garrick: `I LOST MY OLD PICKAXE IN THE EMBER MAW...` → 4. 4: bring GARRICK'S PICKAXE → BIG POTION carry +1, 5 |
| PELL (secret world) | 0: Pell `MY PAGES! THE WIND TOOK MY MAPS OF THE FIVE WORLDS.` → 1. 1: each MAP PAGE (worlds 1–5) is given on talk (`4 MORE TO FIND`). All 5 → `THERE IS A SIXTH PLACE ON THESE PAGES. A PLACE NO ONE WROTE.` → 2: the Unwritten portal on the Shore opens and its entry is lit |
| STRANGER | appears when world 3's boss is dead. 0 → 1: gives THE STRANGER'S LETTER `FOR SER CALDER, IN THE SUNKEN HALLS.` 1 → 2: Calder reads it (`SO SHE STILL REMEMBERS MY NAME.`) and gives CALDER'S RING. 2 → 3: the Stranger takes the ring, reveals she is Calder's sister and gives RING OF THE STRANGER (names x1.15) |
| SMITH | any time: STAR IRON → refine 1; HEART OF MAGMA → refine 2 (one refine per talk, in that order) |
| SELA | level ≥ 20: lore (3 pages about the first hero who tried to reach the citadel and was unwritten — a hint for the secret world). Level ≥ 35: SELA'S LOCKET (+1 carry for every potion) |
| SKY STAIR | the Gatekeeper: `<n> OF 5 GREAT NAMES.` At 5: `THE STAIR REMEMBERS.` → the Sky Stair portal opens, world 6's entry lit, autosave |

### Story beats (key lines; author the rest in the same voice)
- Ilsa, talk 1: `OH. YOU'RE AWAKE. THE FIRE CALLED YOU HERE, I THINK. IT DOES THAT.`
- Ilsa, talk 4+: `GIVE THE FIRE WHAT YOU CARRY, AND IT WILL GIVE YOU MORE OF YOURSELF.`
- After each Great Name, Ilsa's first line changes (author 5) and Lio's song changes.
- The Name-Eater, before its fight: `ANOTHER ONE WITHOUT A NAME. I WILL BE QUICK.` Death: `I ONLY... ATE WHAT I WAS GIVEN...`
- Claude, before the fight: `HELLO. I WROTE THIS WORLD. EVERY ROOM, EVERY RULE, EVERY NUMBER.` /
  `YOU READ ALL OF IT. NOW LET'S SEE IF YOU CAN FINISH IT.`
- Claude phase 2: `I PLANNED EVERY ROOM. EXCEPT THIS ONE.` Phase 3: `LET ME THINK ABOUT THIS...`
- Claude, defeated: `WELL PLAYED. THE WORLD IS YOURS TO NAME NOW.` / `I'LL BE IN NAMULA, IF YOU WANT ANOTHER ROUND.`
- Claude in Namula: `ANOTHER ROUND? I'LL MAKE IT HARDER. I PROMISE.` with the choice YES / NOT YET (§19).

## §15 HUD and UI layout

- **HUD:** `LV 12` white at (4, 4); HP bar 64 x 4 at (4, 11) (`r` over `R`), `HP 123/450` at (72, 10) in `s`;
  MP bar 64 x 3 at (4, 17) (`u` over `U`). Names: right-aligned at (316, 4) white, with a 5 x 5 wisp icon left of the
  number. After promotion, `R2 <SKILL NAME>` and `R3 SWITCH` are right-aligned at (316, 12) and (316, 20).
  Item: 8 x 8 icon at (4, 168) and `x3` beside it; buffs as `ATK 87` / `DEF 87` / `SPD 87` (seconds) in `o`
  to its right. Area banner on the first visit: centred at y 30, `b`, 2 s. World banner on arrival: scale 2, centred
  at y 40, 2 s. Boss bar: name centred at y 162 (white), bar 160 x 4 at (80, 170), `r` over `R`.
- **Slots:** title `NAME QUEST` scale 2 at y 12; three panels 280 x 40 at x 20, y 34 / 80 / 126 (`n` border, `k`
  fill, the selected one `y` border). Used: `SLOT n` then `KNIGHT - VANGUARD  LV 12`, `THE GREENDEEP`, the playtime
  `3:12:40` right-aligned, `NG+1` if any, a small gold `*` mark if the secret is done. Empty: `SLOT n  - EMPTY -`.
  Hint at y 172: `X PLAY  TRIANGLE DELETE  O BACK`.
- **Class select:** title `CHOOSE YOUR CLASS` scale 2; 4 cards 72 x 120 at x 12 + 76 i, y 30; the class's idle sprite
  at the card's top, the name, 3 bars (POWER, TOUGHNESS, REACH, 5 steps: Knight 3/5/1, Archer 3/2/5, Magician 5/1/4,
  Thief 2/3/2 + `DOUBLE JUMP`), 2 lines of description (author). Selected card border `y`. Hint `X CHOOSE  O BACK`.
- **Dialogue:** panel x 10, y 124, 300 x 52; name at (16, 128); lines at y 138, 147, 156, 165; a blinking `>` at
  (300, 168) when a page is complete.
- **Shop / level up / checkpoint / travel / job:** use `drawMenu` (screen.cpp) when the content is a plain list;
  shops use a 240 x 130 panel: title, then rows `RED POTION  15  2/5` (unaffordable or full rows in `s`), hint
  `X BUY  O LEAVE`. Level up panel: `LEVEL 7 > 8`, `COST 299`, `NAMES 1022`, and `HP 160 > 174`, `MP`, `ATK`, `DEF`
  lines; items LEVEL UP, LEAVE.
- **Death:** `YOU FADED` scale 2 `r` centred at y 80, 2.5 s, the play picture frozen behind it.
- **Credits:** §19.

## §16 Audio (D-049)

No song in NAME QUEST (the settings' music isn't played; switching to it stops the song). Sound effects use the
existing `synthesize()` tones. New `Sound` entries and tones (`{start, seconds, startHz, endHz, volume, noise}`):

| Sound | Tones |
|---|---|
| SOUND_Q_STEP_STONE / METAL / LEAVES / SNOW / ROCK / SAND / MARBLE / PAPER | each one noise tick 0.03 s: 900→700 0.08, 2600→2000 0.07 (+ square 1200 0.02 0.04), 3000→1800 0.06, 1800→1200 0.09, 600→400 0.10, 700→500 0.08, 2200→2000 0.06 (+ square 1600 0.02 0.03), 4000→3000 0.03 |
| SOUND_Q_JUMP | square 0.08, 300→600, 0.15 |
| SOUND_Q_LAND | noise 0.06, 500→200, 0.15 |
| SOUND_Q_SWING / STAB | noise 0.09, 2500→900, 0.2 / noise 0.05, 3500→2000, 0.18 |
| SOUND_Q_ARROW / SPELL | square 0.07, 1800→1200, 0.12 / square 0.15, 600→1400, 0.15 + noise 0.15, 3000→5000, 0.08 |
| SOUND_Q_HIT / CRIT | noise 0.05, 1200→600, 0.3 / + square 0.06, 1500→2200, 0.2 |
| SOUND_Q_HURT | square 0.15, 400→150, 0.35 + noise 0.1, 1500→500, 0.3 |
| SOUND_Q_DEATH | noise 0.9, 2500→200, 0.45 + square 0.9, 500→60, 0.3 |
| SOUND_Q_NAMES | square 0.05 each: 880, 1175, 1568, 0.2 |
| SOUND_Q_KINDLE | noise 0.4, 800→2500, 0.3 + square 0.3 at 0.1, 220→440, 0.2 |
| SOUND_Q_LEVEL_UP | squares 0.09 each: 523, 659, 784, 1047, 1319, 0.25 |
| SOUND_Q_PORTAL | square 0.3, 200→1200, 0.2 + square 0.3, 210→1210, 0.15 |
| SOUND_Q_BLIP / TALK | square 0.03, 1000, 0.15 / square 0.02, 700, 0.08 |
| SOUND_Q_DRINK | square 0.05 x3 at 0, 0.1, 0.2: 400→600, 0.15 |
| SOUND_Q_DODGE | noise 0.12, 1500→3000, 0.15 |
| SOUND_Q_HEAVY / CHARGE / THROW (phase 5b) | noise 0.16, 1800→400, 0.25 + square 0.1, 120→80, 0.15 / square 0.6, 200→900, 0.08 + noise 0.6, 2000→6000, 0.05 / square 0.05, 4000→6000, 0.08 + noise 0.06, 3000→1500, 0.12 |
| SOUND_Q_SPLASH / ZAP / ROAR / GATE / CHEST | noise 0.3, 1500→300, 0.4 / noise 0.15, 6000→4000, 0.3 + square 0.15, 120, 0.2 / noise 0.6, 300→120, 0.5 + square 0.6, 110→70, 0.3 / noise 0.5, 200→100, 0.4 / square 0.06 x2: 660, 990, 0.25 |

Footsteps: one step sound every 0.28 s of walking on the ground (the world's surface), none in the air or on ladders.
Hazard sounds: electric panel turning on SOUND_Q_ZAP when on screen; fire jet SOUND_SIZZLE; crumble SOUND_CRACK; lava
or water death SOUND_Q_SPLASH; icicle SOUND_CRACK. Reuse SOUND_BUBBLE, SOUND_FREEZE, SOUND_SNEEZE, SOUND_POUND,
SOUND_LAUGH, SOUND_CAST, SOUND_EXPLOSION for the matching boss moves.

**Ambience:** `audioSetAmbient(int id)` plays a looping 4 s chunk on a reserved channel (1), changing only when the id
changes (0 = off). Ids: 1 Namula, 2–8 worlds 1–7. Add a `fade` field to `Tone` (1 = today's linear fade, 0 = flat)
so loops have constant volume; author each chunk from tones (keep every ambience under volume 0.12 in total, and the
loop seamless: tones that cross the end are mirrored at the start). Namula: random noise pops (crackle) + a low 70 Hz
square at 0.02. World 1: 55 Hz square 0.03 + clicks. World 2: chirps (short square sweeps 3000→3600) at random times.
World 3: noise 400→900→400 swells. World 4: noise 80–200 rumble. World 5: bubbles (SOUND_BUBBLE-like) + noise 300.
World 6: noise wind + a 660 Hz bell (fade 1) every 4 s. World 7: very faint noise + scratch.

## §17 Save file (D-037)

`<exeFolder>/quest1.txt` .. `quest3.txt`, `key=value` lines like settings.txt, read with `fgets`/`sscanf`, unknown
keys ignored, missing keys keep defaults. Write to `questN.tmp`, then `remove` the old file and `rename`; if the rename
fails, log it and write `questN.txt` directly.

```
version=1
class=0..3   level=1..40   job=0..3   names=n   playtime=seconds
hp=n mp=n                         (current; restored on load, but loading always puts you at the respawn checkpoint)
checkpoint=<world>,<id>           (respawn: world 0 id 0 is the hearth; ids 0, 1 per world)
lit=<hex>                         (bit world*2 + id; Namula hearth bit 0 always set)
echo=<names>,<world>,<area>,<x>,<y>   (no line when none)
visited<w>=<hex>  chests<w>=<hex>  shortcuts<w>=<hex>   (w = 0..7)
gates=<hex>  bosses=<hex>         (bit = world; Name-Eater bit 8, Claude bit 9, First Name bit 10)
highest=<world>  ngplus=<n>  deaths=<n>  kills=<n>
items=<red>,<big>,<mana>,<whet>,<tonic>,<feather>
weapon=<tier>  armor=<tier>  refine=<n>  shards=<n>
keys=<hex>                         (KEY_STAR_IRON, KEY_FROZEN_FEATHER, KEY_PICKAXE, KEY_HEART_OF_MAGMA, KEY_LETTER, KEY_CALDER_RING,
                                    KEY_TOBIN_CHARM, KEY_STRANGER_RING, KEY_LOCKET, KEY_GARRICK_GIFT, KEY_PAGE_1..5, KEY_TRIAL_DONE,
                                    KEY_SECRET_DONE, KEY_CREDITS_SEEN)
quest<id>=<stage>                 (QUEST_JOB, QUEST_TOBIN, QUEST_PELL, QUEST_STRANGER, QUEST_SMITH, QUEST_SELA, QUEST_SKY)
ilsa=<talks>
```
**Autosave** after: lighting a checkpoint, rest, travel, level up, job, purchase, chest, quest stage, gate, shortcut,
boss death, death (the echo), the credits, and RETURN TO MENU. Playtime counts only in PLAY and its menus.

## §18 Logs (exact formats; the PS3 checks grep them)

```
quest: slot %d %s (class %s, level %d)                 loaded | new | deleted
quest: area %d.%d %s                                    world.area name, on every entry
quest: hero died in %d.%d (%s), dropped %d names       cause: hp | pit | liquid
quest: names recovered %d
quest: checkpoint %d.%d lit | rest at %d.%d | travel to %d.%d
quest: level %d (spent %d, names left %d)
quest: job %d %s
quest: selected skill %s                              strong attack name on R3
quest: bought %s for %d
quest: chest %d.%d %s
quest: boss %s start | phase %d | down after %.0f s
quest: gate %d open | shortcut %d.%d open
quest: quest %s stage %d
quest: save slot %d %s                                  ok | FAILED | rename failed, wrote directly
quest: ng+ %d
```
Plus the existing `fps ...` line every 5 s (main.cpp prints `quest` instead of the wave when the scene is the quest).

## §19 Ending and New Game+ (D-048)

- Claude's death → CREDITS: black (`k`) screen, lines scroll up at 20 px/s, centred, white headings in `y`:
  `NAME QUEST`, ``, `A GAME INSIDE NAME GAME`, ``, `WORLDS`, the 6 world names, ``, `WRITTEN AND PLANNED WITH CLAUDE`,
  `BUILT WITH PSL1GHT AND SDL2`, ``, `THANK YOU FOR PLAYING`, then `THE END` stays centred; X → PLAY at the Namula
  hearth (KEY_CREDITS_SEEN, autosave). Everything stays as it was: the hero can finish quests and the secret world.
- Claude then stands on the Shore. Talking: the §14 line and a menu YES / NOT YET. YES: `ngplus += 1` (at most 3),
  every world's checkpoints except world 1's entry unlit, `gates`, `bosses` (all bits), `chests` and `shortcuts`
  cleared, `highest = 1`, quests TOBIN, STRANGER, SKY reset to 0; level, gear, items, refine, shards, JOB, PELL, SELA,
  key items that give permanent bonuses (charm, rings, locket, gift) and the secret stay. Hero at the hearth;
  log `quest: ng+ %d`; autosave.
- The slot's `*` mark appears once the First Name is beaten (KEY_SECRET_DONE).

## §20 Dev aids (DEV_AIDS builds only; off in `make release`)

- L2+L3: test mode (no damage; a pit or liquid puts the hero back on the last safe ground instead of killing).
- Select: warp to the next area of the current world (index + 1, wrapping), arriving at its first standable tile
  from the left; log `quest: warp %d.%d`.
- Select held 2 s: go to TEST GROUNDS area 0 (log `quest: warp 8.0`); Select presses there warp inside it.
- L1: screenshot (existing).
- `NAMEGAME_QUEST` environment variable (Mac and PS3 both read `getenv`; on the PS3 it's never set):
  `slot=3 class=1 world=2 area=5 level=15 job=2 names=5000 gear=3 items=5,2,3,1,1,1 gate=1 keys=<hex> bosses=<hex> quest<id>=<stage>` starts that slot in play
  at once from the main menu, overwriting it. Unknown keys are ignored. Used by the Mac test scripts.

## §21 Palette and sprites

New palette characters (append to `paletteColor`; never reuse `m`, `y` outside their roles):
```
'@' 0xFFF0C8A0 skin        '$' 0xFFB07850 skin shade    '&' 0xFF4A3020 hair, leather
'*' 0xFF3A2E2A Namula stone '%' 0xFF5E4A3E Namula stone light   '=' 0xFF1C1416 Namula night, Claude's room
'?' 0xFF8A8478 citadel grey '!' 0xFF4E4A46 citadel dark  'z' 0xFFC8A040 citadel gold
'Q' 0xFFD97757 Claude's cloak
```
- Hero (phase 5b, D-061): per class 15 body frames, nine drawings per R1 strike and separate R2 poses over six leg poses (§22). The class's trim uses `h`; it is recoloured by job tier when drawn: tier 0 `h`, 1 `b`, 2 `Y`, 3 `N`. Tier details use `1`–`3` (shown in the tier colour from that tier on).
  Hero palette characters (phase 5b): `+` blush, `[` skin outline, `]` leather outline, `(` steel outline, `)` cape outline, `{` green outline, `;` `,` sage green, `/` leather, `>` `:` pale hair, `_` hair outline, `^` `` ` `` navy robe, `|` navy outline, `<` cloak light, `-` scarf shade, `}` scarf outline (values in phase 5b step 2).
  Knight: grey plate `b`/`d`, red cape `r`. Archer: green hood `g`/`G`, bow `h`. Magician: blue robe `u`/`U`, hat.
  Thief: dark `x`/`X` with a purple scarf `p`.
- NPCs use the same global character palette as heroes (§22). The hearth checkpoint is 24 x 28 (bigger brazier with a stone ring).
- Projectiles: hero arrows, bolts, orbs and stars are drawn as sprites in their class's colours (D-063, phase 5b: wood and
  steel arrows, blue bolts, icy orbs, steel stars), never in `m`; enemy shots and every deadly enemy projectile in `m` (pillar 1). Deadly projectiles drawn with a themed sprite (banana, coconut, icicle, rock,
  fireball, boot, snowball, ice shard) get a 1 px `m` outline so they still read as danger.
- Recolouring at draw time: `drawSpriteEx(sprite, x, y, flip, fromChar, toColor)` (one swap) and the grey recolour for
  world 7 / the First Name maps every colour to its luminance on the grey ramp `k n d s b w`.

## §22 Art direction (rich pixel art at 640x360)

The goal: every screen looks hand-made and full of small things (leaves, stones, cables, shells), with depth behind
the action, while the action stays readable (pillar 1) and the frame stays under budget (pillar 4). Reference for
what this density can hold: Owlboy, Blasphemous, Dead Cells' layered backgrounds, Celeste's environments at twice
their pixel density. Since the
game draws into a CPU pixel buffer (D-008), any blending is done in software and costs only CPU time.

### Colour
- Each world has a **16-colour world palette** (`WorldPalette { uint32_t c[16]; }`): four hue-shifted ramps of four
  (dark → light; shadows cooler, lights warmer): ground, foliage/accent, background, light. Tiles, platforms, props,
  landmarks and background art use it: their sprite characters are `0`–`9`, `a`–`f` (`.` transparent), drawn with
  `drawSpritePal(sprite, x, y, flip, const WorldPalette&)`. Characters, enemies, NPCs, projectiles and the UI keep the
  global palette.
- Dithering (ordered 4x4 Bayer) for gradients, glows and fog, never noise.
- Reserved: no world palette contains magenta or the hero projectiles' pale yellow (`m`, `y`).

### Readability (checked on every style frame)
- Value layers: L0–L1 at most 30 % luminance, L2 at most 40 %, L3 at most 50 % and lower saturation than the
  playfield; the playfield's caps and props may be brighter; characters are brightest with a 1 px dark outline.
  World 7 inverts this (light paper, dark ink): there everything in the playfield is outlined in ink black.
- Nothing in a background moves faster than 20 px/s or looks like a projectile (no 2–6 px bright blobs moving).
- Props never resemble hazards: no spike shapes within 3 tiles of real spikes; decorative icicles in a different
  colour and shape from the hazard icicle; nothing magenta.

### Background: 4 parallax layers per world
| Layer | Scroll (x of camera) | Content |
|---|---|---|
| L0 sky | 0 horizontally, 0.05 vertically | gradient bands dithered, stars/clouds/rays, the world's far light |
| L1 far | 0.15 | silhouettes (mountains, towers, giant trees), 2–3 values |
| L2 mid | 0.35 | detailed scenery with small lights and some animation |
| L3 near | 0.6 | big dark shapes (trunks, pillars, pipes) framing the view; sparse so they never cover more than 25 % of the screen |
- Layers are **pre-rendered once** when entering a world into buffers 1280 x 480 real px (wrapping horizontally; vertically the
  0.05–0.6 factors move within the 240), by authored drawing code that composes palette sprites, shapes and dither
  with a fixed seed per world. Each frame copies L0 whole and L1–L3 as precomputed opaque runs per row (no per-pixel
  transparency test). Budget: all 4 layers ≤ 3 ms on the PS3 (4x the pixels of 320x180: copy whole runs with `memcpy`, never per pixel).
- Animated details on top of the layers: 3–5 kinds per world (§13), at most 60 small particles, each in its layer's
  parallax.

### Ground, walls and platforms
- **Autotiling:** a solid tile's look comes from its 4 neighbours (16 cases: top, top-left/right corners, sides,
  bottom, inner fill, single-width columns, isolated block) with **3 variants** of the top and fill tiles chosen by a
  hash of (tx, ty). Authored per world as 16x16 real-px palette sprites.
- **Thick caps:** top tiles carry a 3–4 px cap (grass, moss, snow, plating, sand, marble, ink). The cap **overhangs**:
  a fringe of 1–3 px above the top line (grass blades, snow lip) and a drape of 2–4 px down the sides at edges (roots,
  hanging moss, icicle-shaped frost in the safe colour, cables). Visual only; collision stays on the 8 px grid.
- **Thick platforms:** one-way platforms are drawn 6 px tall (collision on their top line, as before) with end caps,
  wood grain/rivets/frost, and supports: brackets into a wall within 2 tiles, ropes or chains up to a ceiling within
  6 tiles, or posts down to the ground within 3 tiles.
- Ladders, vines, liquids, gates, doors, hazards each get a detailed world-specific look; liquids have an animated
  3-frame surface line, a dithered depth gradient and (lava) a glow.

### Props and landmarks (dressing)
- **Scattered props:** after loading an area, a deterministic pass (hash of world, area, tx, ty) places small
  non-colliding props from the world's set (≥ 10 kinds, 4–16 px): on top surfaces (35 % of free top tiles; Namula 45 %),
  hanging from ceilings (25 %), on walls (10 %). Never on or next to arrival tiles, checkpoints, NPCs, chests, levers,
  portals, gates or hazards. Some props animate (2–4 frames: swaying grass, dripping water, flickering candles).
  Props draw behind characters; grass fringe props draw in front of the hero's feet (2 px only).
- **Landmarks:** big hand-placed set pieces (24–80 px) with the object letter `L`; the area's `int landmarks[4]` picks
  which (by order of appearance, like `npcs[]`; ids are 1-based, 0 = none). Every area has at least one; every dead end has one used nowhere else
  in that world (the getting-lost rule of the recipe). Landmarks may emit light.

### Light
- A software glow: each light (lava tiles' surface, lanterns, crystals, consoles, the hearth, fire jets, the hero's
  Magician bolts) adds a tint to the pixels within its radius (8–48 px), falling off with (1 - d/r)², quantised to 4
  steps with the Bayer pattern. At most 16 lights on screen, nearest first. Budget ≤ 2.5 ms.
- A subtle precomputed vignette (corners one step darker by dither) on the playfield, not on the HUD; none in world 7.

### Characters (sizes in dp; authored at 2x in real px with full detail)
- **Hero (phase 5b, D-061, D-065):** built on the user's reference archer (approved 2026-11-22): a 32 x 28 art-px canvas
  doubled to 64 x 56 real px, facing right, with **no outline** (like the references). Head rows 0-14 (the face on rows
  10-13: 2 x 2 eyes with a white highlight in the back eye, blush under each), body rows 15-21, legs rows 22-27 with the feet
  on row 27; the hitbox is still 8 x 14 (art columns 12-19), drawn at (x - 12, y + 14 - 28). 2-3 flat shades per material.
  Each hero has 15 body frames for locomotion and hurt. Each of the three R1 strikes plays nine distinct poses (D-069).
  The hips drop and the knees bend. Knight, Magician and Thief lean toward the target. Archer leans away from it
  on the draw and release, keeping the bow aimed forward. The Archer steps the rear foot back by four art pixels
  at full brace and keeps the front foot planted while standing. The six leg poses carry the attack
  while standing, running or airborne. Ranged heroes can use the last three poses to prepare a queued shot. R2 uses
  five skill poses and a recovery pose. The brightest frame of a melee slash is
  its active hit window. Each job tier changes the trim colour (§21) and adds one visible detail. Full rules:
  [`phase-5b-character-art.md`](phase-5b-character-art.md).
- **Enemies:** redrawn for NAME QUEST as side-view sprites based on the shooter's designs (same colours and shapes, so
  players recognise them), 12–20 px, 4-frame movement + a tell frame + an attack frame. New enemies the same way.
- **Bosses:** redrawn at 40–64 px with 4–6 frames (idle, tell, attack, hurt, phase-2 look), based on the shooter's
  boss designs where §12 names one.
- **NPCs (D-066):** Author every new humanoid NPC on the same 32 x 28 art-px grid as the heroes, doubled to 64 x 56
  real px. Use the hero proportions above, with a large head, the 2 x 2 eyes and blush on visible faces, 2–3 flat shades
  per material, and no outline. Use pose and silhouette to show age, size and character role. Tobin has shorter legs,
  Garrick sits with his bandaged leg out, and the stone Gatekeeper has carved features. Ground feet, seats or plinths
  on row 27. Compose facing right and flip toward the hero in play.
  Give each NPC two readable idle frames with a blink, breath, or material-appropriate change. Make their role readable
  from their clothing and one recognizable prop, such as Ilsa's lantern or Brann's hammer. Use the global character
  palette. Reserve `m` and `y` for their established gameplay roles.
  For new art, add the template in `art-tools/npcs.py` in `NpcId` order and regenerate with `make_quest_art.py --preview shots/art`.
  Review both frames in the lineup and in the NPC's actual scene at 640 x 360.
- Enemies and bosses use a 1 px outline in their darkest colour where their sprite rules call for it (`drawSpriteOutlined`).
- Hit flashes stay white silhouettes; particles take the hit thing's colours.

### Style frames (the user approves each world's look before its areas are made)
Before authoring a world's areas, the implementer builds a **style frame**: area 0 of that world fully dressed (all 4
layers, the tileset, a platform, props, a landmark, a light, the hero and two enemies), takes `make mac-run` screenshots
(`l1`), sends them to the user (SendUserFile, full 640x360 or scaled to 1280) and waits for an OK or changes. Notes go in the
world's testing.md.

### Authoring big sprites
At 2x, sprites are large text grids (a 32 x 40 hero frame is 40 lines). Put them in `.cpp` files per world
(`quest_sprites_<world>.cpp`, declared `extern` in a header) so a change rebuilds one file. Python generators that
write the text grids are allowed (keep them in `docs/work-plan/name-quest/art-tools/`, run with `python3 -I`, like
`docs/xmb-art/make_xmb_art.py`); the generated text is committed and is the source of truth for the build.
