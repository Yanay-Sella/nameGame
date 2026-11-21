# Phase 5b: character art, animation and combat feel

Read first: [`../name-quest-plan.md`](../name-quest-plan.md), then spec §5 (the attack paragraph), §6 (jobs table),
§7 (hit feel), §16 (sound table), §21 (palette, hero sprites, projectiles), §22 (Characters). Phases 1–5 done.
Phase 6 starts only when this phase's "Done when" holds (master plan).

**No world or level work in this phase.** Don't touch areas, enemies, bosses, hazards, NPCs or world art. The only
subjects are the four playable classes: their sprites, how they move while attacking, their R1 and R2 attacks, and
the feel of those attacks.

**Gameplay numbers are frozen.** Damage, multipliers, MP costs, hitboxes, ranges, projectile speeds and sizes, attack
cycles, active windows, `SKILL_TIME` and every `hitBox`/`hitCircle`/`shoot` call keep their values.
Exactly three rules change (decided with the user, D-062 to D-064):
1. **No R1 movement penalty** (step 1, D-062). Spec §5 now says so explicitly. No later phase may bring it back.
2. **Melee basic hits get a 0.025 s hit-stop** (step 7, D-063).
3. **No R2 skill roots the hero** (step 0, D-064, the user's call after this plan listed the locks).

## What the player gets
- Four redrawn heroes in the style of the user's two references: big heads, readable faces (2 px eyes with a white
  highlight and a blush pixel), 2–3 flat shades per material, a soft coloured outline instead of black, and clear
  gear. Each class has its own silhouette, and each job tier visibly adds to it.
- Holding R1 while running keeps full speed for every class, and so does every R2 skill. The legs keep running while
  the arms attack, and jumping, landing and attacking blend without stopping.
- Basic attacks with a readable wind-up, a quick release and follow-through. The bow draws while R1 is held and
  snaps on release. The staff builds a spark and releases it. The sword sweeps a wide crescent, and the dagger
  stabs in a short straight streak. The brightest frame of a slash is exactly its active hit window.
- Real arrows, magic bolts, frost orbs and throwing stars instead of rectangles; every R2 skill of every tier
  is recognisable in motion, shows where it will hit, and lands with a spark, sound and a brief stop.
- Enemy shots, telegraphs and hazards stay readable on top of the hero's effects.

## The references (art direction for every step)
The user supplied two images (the files, if the user saved them: `docs/work-plan/name-quest/art-ref/`; read them
before step 2 if they exist). What to take from them:
- **Archer reference:** a ~16 x 24 px chibi girl. Cream-white hair in a high ponytail with a green band, a face of
  skin with two 2-px dark eyes, one white highlight pixel and a pink blush pixel. A brown leather vest over a sage-green
  tunic, bare arms, brown boots (front boot lighter, back boot darker), and a recurve bow held wide across the body.
  Head height is about 40 % of the figure.
- **Witch reference:** a ~20 x 30 px figure. A wide navy pointed hat with a bent tip and a muted green band, pale
  grey hair falling past the shoulders, green eyes, a long navy robe with 2 shades, small brown boots, and a stack of
  coloured books held in front.
- **Shared rules:** no black outline (we use a coloured one, see below); 2–3 flat shades per material, lit from the
  top-left; muted, slightly desaturated colours; silhouettes readable from their outline alone (ponytail, hat, bow
  limbs). **Original designs only:** take the approach, not these characters (no cream ponytail archer, no navy-hat
  witch with books).

## Step 0: no R2 skill roots the hero (`source/quest.h`, `source/quest_combat.cpp`; the user's decision, D-064)
Three skills froze the hero while they played: Power Slash (0.6 s on the ground), Meteor (0.6 s, and in the air no
steering) and Blade Fury (0.5 s on the ground). The user chose to run during all three. Their damage, hit windows and
timing stay as they are; only the lock goes. Shadow Dash's dash and Rising Slash's hop stay (movement built into the
skill), and so does the rule that no basic attack or dodge starts while a skill plays (`h.skill >= 0`).

`castTimer` only ever held these locks, so remove it entirely:
- `quest.h` › `Hero`: `float attackTimer, strongTimer;   // s left of the basic cycle and of the strong attack`.
- `quest_combat.cpp`: delete `SKILL_CAST`; in `castSkill` delete `h.castTimer = h.onGround ? ...` and Meteor's
  `h.castTimer = SKILL_CAST[cls][job];` line; Meteor's `FX_METEOR` life becomes `METEOR_TIME`, with
  `const float METEOR_TIME = 0.6f;   // Meteor lands 0.6 s after the press (spec §6)` next to `PARTNER_TIME`.
- `updateSkill` › Meteor: `if (h.volleys > 0 && elapsed >= METEOR_TIME - 0.001f) {` (was the cast-timer condition).
- `hurtHero`: `h.rollTimer = 0;`. `updateCombat`: delete the `castTimer` countdown; both `free` lines lose
  `h.castTimer <= 0 &&`; the movement chain's first test becomes `if (h.hurtTimer > 0) {` and the
  `else if (h.castTimer > 0) { ... }` branch is deleted; the skill end is `if (h.strongTimer <= 0) h.skill = -1;`.
- Movement comment: `// Movement: knockback, roll and dash keep their speed; R1 and R2 never slow or root the hero (D-062, D-064).`
- `grep -rn castTimer source tests` must find nothing afterwards.
- Test (step 11): `skillsNeverRootTheHero`.

## Step 1: remove the R1 movement penalty (`source/quest_combat.cpp`)
- Delete `const float BASIC_SLOW` and, in `updateCombat`'s movement `else` branch, the line
  `if (h.attackTimer > 0 && basic.speed > 0) move.moveX *= BASIC_SLOW;` (the movement comment was set in step 0).
- Nothing else in the movement code changes (the drink x0.4 and the feather x1.15 stay).
- Add a test now (step 11 lists it): `basicAttackKeepsFullSpeed`. `make test`.

## Step 2: the hero palette and the new hero art (`source/sprites.h`, `art-tools/make_quest_art.py`, new `art-tools/heroes.py`)

### Palette
Append to `paletteColor` in `source/sprites.h` (after `'Q'`), each with a short comment:
```
'+' 0xFFE89A88 blush             '[' 0xFF6A3A28 skin outline       ']' 0xFF24160C leather/hair outline
'(' 0xFF23263A steel outline     ')' 0xFF4A1220 cape outline       '{' 0xFF1E3A1C green outline
';' 0xFF6E9A50 sage green        ',' 0xFF46703A sage green shade   '/' 0xFF7A4A2C warm leather
'>' 0xFFE6E2C4 pale hair         ':' 0xFFB4B094 pale hair shade    '_' 0xFF6E6A58 pale hair outline
'^' 0xFF2E5478 navy robe         '`' 0xFF1C3450 navy robe shade    '|' 0xFF0E1A2C navy outline
'<' 0xFF7A7686 cloak light       '-' 0xFF6A44B0 scarf shade        '}' 0xFF2E1A52 scarf outline
```
These are character colours; world palettes never use them (they use `0`–`f` through `drawSpritePal`).
Before adding them, check `paletteColor` again: if a later change took one of these characters, pick an unused
punctuation character (never `"`, `\`, `?`, `#`, `.`) and say so in `implementation.md`.

### Canvas and body box (every hero frame)
- Art grid **24 x 24 art px**, doubled to **48 x 48 real px** (2x2 blocks, like the NPCs and enemies).
- The body (head, torso, legs, outline included) fits a **16 x 22 box: art columns 4–19, rows 2–23**. Weapons,
  cape, ponytail, hat tip and skill poses may use the rest of the canvas.
- The 8 x 14 dp hitbox is **art columns 8–15, rows 9–22**; the feet's lowest pixels are on row 22 and the outline
  under them on row 23. Mirroring keeps the body in place (the canvas is symmetric around column 12).
- Proportions as the references: head (with helm, hood, hat or hair) rows 2–10, torso rows 11–16, legs rows 17–22.
  Faces: two 2-px-tall eyes (`=`) one pixel apart, one `w` highlight pixel in the front eye, one `+` blush pixel under
  it. Light from the top-left: the lightest shade on top and left edges, the darkest on the bottom and back.
- **Coloured outline** (D-061): after composing a frame, every transparent cell that touches an opaque cell (4-way) becomes
  the outline colour of that neighbour, looked up in this order of neighbours: below, front (right), back (left), above:
```
OUTLINE_OF = {b s d w c y: '(',  r R: ')',  @ $ +: '[',  & / h: ']',  ; ,: '{',  > : _: '_',  ^ ` |: '|',  p - }: '}'}
anything else: 'x'
```
  The eyes (`=`) never touch the edge.

### The four designs (original; a distinct silhouette each)
| Class | Silhouette | Colours | Weapon | Tier details |
|---|---|---|---|---|
| KNIGHT | broad: an open-faced round helm with a crest ridge, square pauldrons, a short red cape falling behind | steel `b s d`, cape `r R`, leather `& /`, face `@ $ + =` | a broadsword with crossguard (`b` blade, `w` edge, `h` grip wrap), a small round shield on the back arm (`s d`, `h` rim) | trim `h` on the helm ridge, belt and shield rim; tier 1+: a 3-px plume on the helm; tier 2+: a raised pauldron rim; tier 3+: a crest on the shield (all in the tier colour) |
| ARCHER | lean and tall: a short hood pushed back over dark hair tied in a low side braid, a short cape-let, slim legs, a quiver on the back | sage `; ,`, leather `/ &`, hair `&`, face | a recurve bow (`/` limbs, `&` grip, `w` string) held out front; the arrow `/` shaft, `b` head | trim `h` on the hood edge and quiver band; tier 1+: 3 fletchings above the quiver; tier 2+: a bracer on the bow arm; tier 3+: a feather in the hair |
| MAGICIAN | tall and pointed: a wide navy hat with a bent tip (drawn differently from the reference: a straight brim, the tip bent backwards), long pale hair, a robe flaring at the hem | navy `^`, `` ` ``, `\|`; hair `>` `:`; face; boots `&` | a crooked staff (`/` wood, `&` knots) with a `c` gem and a `w` glint on top | trim `h` on the hat band and robe hem; tier 1+: a star on the hat; tier 2+: the gem grows to 2x2; tier 3+: a mantle collar |
| THIEF | low and compact: shaggy dark hair, a cloth mask over the lower face, a purple scarf whose two long tails fly behind, a slight crouch | cloak `< X x`, scarf `p - }`, hair `&`, face (eyes only) | a short dagger (`b` blade, `w` tip) reversed in the front hand, a second sheathed on the hip | trim `h` on the belt and boot wraps; tier 1+: a third scarf tail; tier 2+: arm wraps; tier 3+: a gold mask edge |

**Tier details** are drawn with the characters `1`, `2`, `3` in the hero grids: a `k` pixel is shown in the tier
colour when `job >= k`, else it's transparent. Put them only where no body pixel is (plume, feather, fletchings, star,
tails, collar edge, crest) so a lower tier just doesn't have them. `h` stays the trim char, recoloured by tier
(`TRIM_BY_JOB` `h b Y N`, unchanged). The outline pass treats `1 2 3` as transparent, so tier details carry no outline.

### Frames
Each class gets **15 body frames** and **8 arm poses x 6 leg poses** (63 frames):
- Body: `IDLE_1 IDLE_2` (breathing: the head and torso sink 1 px), `RUN_1..RUN_4` (contact, pass, contact, pass, with
  a 1 px bob, the cape, tails, hair and braid lagging one frame), `JUMP` (rising, knees up), `PEAK` (arms out,
  legs loose), `FALL` (legs down, cape/hair up), `LAND` (a 1 px squash: knees bent, head 1 px lower, 1 px wider),
  `CLIMB_1 CLIMB_2` (back view; 2 is 1 mirrored with one hand higher), `ROLL_1 ROLL_2` (tucked ball, 2 is half a turn),
  `HURT` (thrown back, eyes shut as a 1-px line).
- Arm poses (upper body: head, torso, arms, weapon):
  - basic: `WINDUP`, `ACTIVE`, `RECOVER`; the melee classes wind up the weapon back (Knight raised high behind,
    Thief pulled back to the hip), strike forward (Knight blade horizontal at full reach, Thief arm fully extended in
    a thrust), and follow through (Knight blade low in front, Thief arm coming back). Ranged: `WINDUP` = the draw (Archer: string pulled to the
    cheek, the bow bent 1 px deeper; Magician: staff pulled back, gem lit with a `w` core), `ACTIVE` = the release
    (Archer: string snapped straight, the drawing hand flung back open; Magician: staff thrust forward), `RECOVER` =
    reaching for the next arrow / staff swinging back up.
  - skill poses `SKILL_1..SKILL_5`, per class:

| Class | SKILL_1 | SKILL_2 | SKILL_3 | SKILL_4 | SKILL_5 |
|---|---|---|---|---|---|
| KNIGHT | RAISE: sword lifted over the head with both hands | CLEAVE: a wide two-handed horizontal strike, body lunging | UPPERCUT: sword vertical above, body stretched up | SLAM: kneeling, sword point in the ground | ROAR: chest out, sword held back low, mouth open |
| ARCHER | FULL DRAW: leaning back, string past the ear | BIG RELEASE: the bow thrust forward, both arms straight | AIM UP: the bow pointing 45° up, drawn | UP RELEASE: the same, string snapped | STORM: the bow held flat-ish (tilted 30°), drawn |
| MAGICIAN | GATHER: the staff held up and back with both hands, gem bright | THRUST: the staff pushed forward in both hands | OVERHEAD: the staff raised straight up, arms high | POINT: the free hand forward with spread fingers, staff behind | PUSH: both palms forward, the staff tucked under the arm |
| THIEF | THROW: the throwing arm fully forward, fingers open | DASH: a low lunge, the dagger forward, scarf streaming flat | SPIN_A: turned three-quarter away, daggers out to both sides | SPIN_B: turned three-quarter toward, daggers crossed | CROSS: arms crossed over the chest, head down |

- Leg poses (hips, legs, feet; also the cape/tails/robe hem below the waist): `STAND`, `RUN_1..RUN_4` (the same
  legs as the body run frames), `AIR` (the `PEAK` legs).
- A frame `<POSE>_<LEGS>` is the arm pose stamped over the leg pose, exactly the way the current generator stamps
  `TORSOS` over `LEGS`. The upper body bobs 1 px down on `RUN_1` and `RUN_3` legs as in the body run frames.
  Because the hero can run during every attack and skill (D-062, D-064), every arm pose needs all six leg poses.

### Generator (new `docs/work-plan/name-quest/art-tools/heroes.py`, called from `make_quest_art.py`)
- Move the hero code out of `make_quest_art.py` into `heroes.py` (like `namula.py`, `world1.py`): templates
  per class as lists of strings in the 24 x 24 canvas (`HEADS`, `TORSO_ARMS[pose]`, `LEGS[pose]`, `BODY_SPECIAL`
  for climb, roll, hurt, jump, peak, fall, land), the key-to-palette tables, the compose function, the outline pass,
  doubling, and `write_all(write, sprite_text, root)` + `preview(folder, save_png, global_palette)`.
- `make_quest_art.py` stops emitting `HERO_*` into `quest_sprites.h` (remove the old `HERO_FRAMES`, `HEADS`, `TORSOS`,
  `LEGS`, `CLIMB`, `ROLL`, `FRAMES`, `COLOURS`, `CLASSES`, `BACKS`, `WEAPON_DIRS`, `hero`, `weapon` and their use in
  `build_objects` and `preview_objects`), updates its docstring and header comment, and calls `heroes.write_all` and
  `heroes.preview`. Check that the NPC code doesn't use any of the removed names before deleting them (grep); keep what it uses.
- `heroes.py` writes `source/quest_sprites_hero.h` and `source/quest_sprites_hero.cpp`:
```cpp
// quest_sprites_hero.h (generated by docs/work-plan/name-quest/art-tools/heroes.py: edit that script, not this file)
#pragma once
#include "sprites.h"
const int HERO_CLASSES = 4, HERO_BODY_FRAMES = 15, HERO_ARM_POSES = 8, HERO_LEG_POSES = 6;
// Heroes: 24x24 art px doubled (48x48 real px), facing right; trim 'h' and tier details '1'-'3' recoloured by job tier.
extern const Sprite* const HERO_BODY[HERO_CLASSES][HERO_BODY_FRAMES];
extern const Sprite* const HERO_ARMS[HERO_CLASSES][HERO_ARM_POSES][HERO_LEG_POSES];
// Hero shots and hit sparks (step 8), doubled, facing right.
extern const Sprite ARROW_FLAT, ARROW_UP, ARROW_DOWN, ARROW_FALL, ARROW_BIG, BOLT_A, BOLT_B, ORB_A, ORB_B, STAR_A, STAR_B,
    QUAKE_A, QUAKE_B, METEOR_ROCK, SPARK_1, SPARK_2, SPARK_3;
```
  The `.cpp` holds every `static const Sprite HERO_<CLASS>_<NAME>` (names: the body frame names, and
  `<POSE>_<LEGS>` for the 48 combinations, e.g. `HERO_KNIGHT_ACTIVE_RUN_3`, `HERO_THIEF_SKILL_2_AIR`), then the two
  tables in this order: classes KNIGHT ARCHER MAGICIAN THIEF; body frames in the order listed above; arm poses
  `WINDUP ACTIVE RECOVER SKILL_1..SKILL_5`; legs `STAND RUN_1 RUN_2 RUN_3 RUN_4 AIR`.
- Step 2 ends with: every body frame and the three basic arm poses authored for all four classes, at all tiers. The
  skill poses `SKILL_1..5` are temporarily the `ACTIVE` pose (step 6 authors them); say so in a comment in `heroes.py`.
- `--preview` writes `heroes.png`: one row per class, at 3x, on the old `(40, 36, 60)` background: the 15 body frames,
  then `WINDUP/ACTIVE/RECOVER` on `STAND`, then `ACTIVE` on `RUN_1`; and `heroes_tiers.png`: `IDLE_1` of each class at
  tiers 0–3 (tier colour and details applied). Run `python3 -I docs/work-plan/name-quest/art-tools/make_quest_art.py --preview shots/art`
  and look at both images.

## Step 3: pose selection and drawing (`source/quest.h`, `source/quest.cpp`, `source/quest_combat.cpp`, `source/quest_draw.cpp`)

### Hero state (`quest.h` › `struct Hero`)
Add after `volleys`:
```cpp
    float landTimer;             // s left of the landing pose (drawing only)
    bool fireHeld;               // R1 held this step (the bow and staff draw while it is)
```

### Pose (`quest.h`, after `struct Hero`)
```cpp
// The hero's drawn pose (spec §22): a body frame, or an arm pose over legs; t: s into the basic attack / skill, -1 none.
enum HeroBody { BODY_IDLE_1, BODY_IDLE_2, BODY_RUN_1, BODY_RUN_2, BODY_RUN_3, BODY_RUN_4, BODY_JUMP, BODY_PEAK, BODY_FALL,
                BODY_LAND, BODY_CLIMB_1, BODY_CLIMB_2, BODY_ROLL_1, BODY_ROLL_2, BODY_HURT };
enum HeroArms { ARMS_NONE = -1, ARMS_WINDUP, ARMS_ACTIVE, ARMS_RECOVER, ARMS_SKILL_1, ARMS_SKILL_2, ARMS_SKILL_3, ARMS_SKILL_4, ARMS_SKILL_5 };
enum HeroLegs { LEGS_STAND, LEGS_RUN_1, LEGS_RUN_2, LEGS_RUN_3, LEGS_RUN_4, LEGS_AIR };
struct HeroPose { int body, arms, legs; float attackT, skillT; bool slashLit; };   // slashLit: the melee active window
HeroPose heroPose(const Quest& q);                          // quest_combat.cpp (it owns the attack tables)
```

### `heroPose` (`quest_combat.cpp`, after `updateSkill`)
Pure, reads only `q.hero`, `q.save`, `q.time`, `q.mode`. In order:
1. `attackT = h.attackTimer > 0 ? BASIC[cls].cycle - h.attackTimer : -1`; `skillT = h.skill >= 0 ? SKILL_TIME[cls][h.skill] - h.strongTimer : -1`.
2. `legs`: `LEGS_AIR` if not on ground; else `LEGS_RUN_1 + (int)(q.time / RUN_FRAME) % 4` if `fabsf(h.vx) > 10 * DP`; else `LEGS_STAND`.
   `const float RUN_FRAME = 0.1f;` (the old `WALK_FRAME`).
3. Body (used when `arms == ARMS_NONE`), first match: dead or `hurtTimer > 0` → `BODY_HURT`; `rollTimer > 0` →
   `ROLL_1/2` by `(int)(q.time / 0.1f) % 2`; climbing → `CLIMB_2` if `vy != 0` and `(int)(q.time / 0.2f) % 2`, else
   `CLIMB_1`; in the air: `vy < -60 * DP` → `JUMP`, `vy > 40 * DP` → `FALL`, else `PEAK`; `landTimer > 0` → `LAND`;
   running → `BODY_RUN_1 + (int)(q.time / RUN_FRAME) % 4`; else `IDLE_1/2` by `(int)(q.time / 0.6f) % 2`.
4. Arms: `ARMS_NONE` if hurt, rolling, climbing or dead. Else, if a skill plays: the skill table below. Else, if
   `attackT >= 0`: the basic table below (ranged: `ACTIVE` first, then `WINDUP`, else `RECOVER`). Else, for the Archer
   and Magician only: `ARMS_WINDUP` if `h.fireHeld` (the draw held before the first shot is fine: the shot fires on the
   next free step). Else `ARMS_NONE`.
5. `slashLit`: Knight or Thief with `attackT` inside `[BASIC.from, BASIC.to]` (the exact `hitBox` window), or a
   Knight melee skill inside its hit window (Power Slash `[0.1, 0.25]`, Rising Slash and Lion's Roar `[0, 0.15]`).

Basic table (`c` = cycle, `from`/`to` = the melee window):
| Class | WINDUP | ACTIVE | RECOVER | after that |
|---|---|---|---|---|
| Knight, Thief | `t < from` | `from <= t <= to` | `to < t < to + 0.6 * (c - to)` | `ARMS_NONE` |
| Archer, Magician (the shot leaves at t = 0) | `t >= c - 0.20` and `fireHeld` | `t < 0.08` | `0.08 <= t < c - 0.20`, or later without `fireHeld` | — |

Skill table (`t` = `skillT`; `RECOVER` = `ARMS_RECOVER`):
| Skill | poses |
|---|---|
| K0 Power Slash | `SKILL_1` t < 0.1, `SKILL_2` t ≤ 0.25, then `RECOVER` |
| K1 Rising Slash | `SKILL_3` t ≤ 0.15, then `RECOVER` |
| K2 Ground Quake | `SKILL_4` |
| K3 Lion's Roar | `SKILL_5` t ≤ 0.15, then `RECOVER` |
| A0 Double Shot, A2 Piercing Bolt | `SKILL_2` t < 0.12, then `RECOVER` |
| A1 Arrow Rain | `SKILL_4` t < 0.12, then `SKILL_3` |
| A3 Storm Volley | `u = fmodf(t, 0.225f)`: `SKILL_2` (release) if u < 0.08, else `SKILL_5` (drawn, bow tilted) |
| M0 Arcane Burst | `SKILL_2` t < 0.12, then `RECOVER` |
| M1 Frost Orb | `SKILL_5` t < 0.12, then `RECOVER` |
| M2 Chain Lightning | `SKILL_4` |
| M3 Meteor | `SKILL_1` t < 0.3, then `SKILL_3` |
| T0 Star Toss | `SKILL_1` t < 0.1, then `RECOVER` |
| T1 Shadow Dash | `SKILL_2` |
| T2 Blade Fury | `SKILL_3` if `(int)(t / 0.05f) % 2 == 0`, else `SKILL_4` |
| T3 Shadow Partner | `SKILL_5` |

### Timers (`quest.cpp` › `questUpdate`, `quest_combat.cpp` › `updateCombat`)
- `updateCombat`, at the top after the hit-stop return: `h.fireHeld = a.fire;` and `h.landTimer = fmaxf(0, h.landTimer - dt);`.
- `questUpdate`, next to the land sound: `if (!wasOnGround && h.onGround) h.landTimer = LAND_TIME;` with
  `const float LAND_TIME = 0.08f;` at the top of `quest.cpp`. Drawing only; it never blocks input.

### Drawing (`quest_draw.cpp`)
- Include `quest_sprites_hero.h`. Delete `enum HeroFrame`, `HERO_ROW`, `HERO_FRAMES`, `WALK_FRAME`, `IDLE_FRAME`,
  `CLIMB_FRAME`, `ATTACK_FRAME`, `ROLL_FRAME`, `FALL_SPEED` if nothing else uses them (check with grep).
- `drawHero`: `HeroPose p = heroPose(q);` sprite = `p.arms == ARMS_NONE ? HERO_BODY[cls][p.body] : HERO_ARMS[cls][p.arms][p.legs]`.
  Position `x = floorf(h.x - 8 * DP) - camX`, `y = floorf(h.y - 9 * DP) - camY`. The partner silhouette and i-frame
  blink stay as they are (`drawSpriteSolid` draws the tier-detail pixels too, which is fine).
- The hero itself is drawn by a plain pixel loop (like the Star Mage's dither loop in `drawCombat`), because it needs
  two recolours: `'.'` skip; `'1'..'3'` skip if `job < c - '0'`, else the tier colour; `'h'` the tier colour
  (`TRIM_BY_JOB[job]`); anything else `paletteColor(c)`; mirrored when `h.facingLeft`; clipped to the screen.
- Put that loop in `static void drawHeroSprite(const Sprite& s, int x, int y, bool flip, int job)` (two call sites:
  `drawHero`, and `drawClassSelect`, which draws `HERO_BODY[i][BODY_IDLE_1]` at job 0 at `y + 1 * DP`, since the taller
  canvas would touch the class name at `y + 26`). `drawSprite` can't be used there: it would draw the `1`–`3` pixels.
- `tests/quest_tests.cpp`: remove the 64 `&HERO_*` entries from `ALL_QUEST_SPRITES`; `questSpritesHaveTheirSize` also
  checks every `HERO_BODY` and `HERO_ARMS` entry is 48 x 48 and has `w * h` characters.

Then `make test`, `make mac-run` per class (`NAMEGAME_QUEST='slot=3 class=<0..3> world=8 area=0'`, `BUTTONS='2|right:1|l1|1|quit'`), look at the shot.

## Step 4: animation sheets and the style preview tool (`tests/quest_tests.cpp`, `Makefile`, new `art-tools/flipbook.py`)
A deterministic renderer (game clips can't be timed reliably with real-time scripts).
- `tests/quest_tests.cpp`: `static void renderAnims()`, run by `quest_tests --anims` (in `main` next to `--maps`):
  - **Lineups** (the in-game style frame): for world 1 area 0 and Namula area 0 (`loadArea(q.area, w, 0, 0, 0)`, the
    camera at the area's arrival screen, enemies spawned, `q.time = 1`), draw the scene 4 times with the hero being
    each class at tier 0, standing on the arrival tile and the 3 tiles to its right of it, spaced 40 dp (x = arrival
    x + 40 dp * class), facing right; copy only each pass's 24 x 24 dp box around its hero into the first pass's
    picture (the backgrounds are identical because camera and time are the same). Save `build/anims/lineup_w1.bmp`,
    `lineup_w0.bmp`. A second pair `lineup_w1_t3.bmp`, `lineup_w0_t3.bmp` with every hero at tier 3.
  - **Sheets:** for each class `c` and tier `j` (16 runs): a `TestArea` with a flat floor on row 19 and an `A` at column 30
    and a `D` at column 36, `startQuest(q, t, c, 30, j, 14)`, `q.godMode = true`, enemies held in place: after every step
    put each enemy back at `homeX, homeY` with `vx = vy = 0`, `state = ES_PATROL`, `stateTimer = 0`, `hp = maxHp`.
    Script at 60 steps/s with `play`: 0.5 s `moveX = 1` + `fire` held (run + basic), 0.3 s `fire` held standing, 0.2 s
    nothing, then `rocket` pressed once and 1.0 s nothing (the skill and its aftermath), with MP refilled before the press.
    Capture every 2nd step (30 fps) a 160 x 90 dp crop centred on the hero (clamped to the picture) into a sheet of 8
    columns, 1 dp gap, background `k`; save `build/anims/<class>_t<j>.bmp` (class lowercase, e.g. `archer_t2.bmp`).
  - Uses `screenSetTarget`/`screenSaveBmp` like `renderMaps`, and a second `std::vector<uint32_t>` for the sheet.
- `Makefile`: target `quest-anims` (and add it to `.PHONY`), shaped like `quest-maps`: build the test binary, `mkdir -p build/anims shots/anims`,
  run `build/tests/quest_tests --anims`, convert every BMP to `shots/anims/<name>.png` with `sips`, then
  `python3 -I docs/work-plan/name-quest/art-tools/flipbook.py build/anims shots/anims/flipbook.html`.
- `flipbook.py` (standard library only): reads the 16 sheet BMPs (24-bit, bottom-up rows; parse the header with
  `struct`), slices the cells (8 columns, the cell size from the crop), encodes each frame as a PNG (reuse the
  `save_png` logic, written to bytes) in base64, and writes one self-contained HTML page: a class/tier picker, the clip
  playing at 30 fps scaled 3x with `image-rendering: pixelated`, play/pause, step back/forward, and a speed toggle
  (1x, 0.25x). No external scripts or fonts.

## Step 5: style preview (needs the user; stop here until they approve)
- Run `make quest-anims` and the art preview from step 2.
- Send with `SendUserFile`: `shots/anims/lineup_w1.png`, `lineup_w0.png`, `lineup_w1_t3.png`, `shots/art/heroes.png`,
  `shots/art/heroes_tiers.png`. Say in one line per class what its silhouette is.
- Wait for the user's OK or changes. Apply changes, repeat. Record the approval and notes in `testing.md` (Phase 5b
  section). **Don't author the skill poses or effects before this OK.**

## Step 6: skill poses (`art-tools/heroes.py`)
Author `SKILL_1..SKILL_5` for every class (table in step 2) on all six leg poses; remove the temporary `ACTIVE` mapping
and its comment. Regenerate, `make test`, `make quest-anims`, and look at every sheet: each skill's pose sequence must
read as wind-up → release → follow-through.

## Step 7: melee slashes, impacts and hit-stop (`source/quest_combat.cpp`, `source/quest_draw.cpp`, `source/quest.h`)
### Rules for every hero effect (readability, pillar 1)
- Never `m` (enemy shots only) and nothing that resembles magenta.
- Large fills (more than 12 dp across) are drawn 50 % dithered (`bayer4(px, py) < 8`) with a solid 1-dp edge, so
  enemies and shots behind stay visible.
- Draw order in `drawCombat`: hero effects and hero shots **first**, then enemies and bosses, then enemy shots and
  telegraph markers **last** (move the `!p.fromHero` projectile drawing and the markers loop after the effects).
- With `q.reduceFlashing`, white cores and sparks use `b` instead of `w`.
- At most `PARTICLE_CAP = 200` particles: `burst` and every new particle push skip when `q.particles.size() >= PARTICLE_CAP`.

### Basic melee (drawn from `heroPose`, not from effects)
- Remove the `FX_SWING` `addEffect` call from `updateCombat` (and the `FX_SWING` case from `drawCombat`; keep the enum value).
- New `static void drawHeroSlash(const Quest& q, const HeroPose& p, int camX, int camY)` in `quest_draw.cpp`, called in
  `drawCombat` before the enemies. Box = the exact `hitBox` box (`frontBox` geometry: in front of the hitbox, centred
  on its middle; Knight 22 x 18 dp, Thief 16 x 12 dp). Colours: edge = the tier colour (`h b Y N` → for tier 0 use `b`),
  core `w`.
  - **Knight:** `slashLit`: a crescent filling the box (the existing `drawArc` shape) with a solid `w` inner band 2 dp
    thick and the tier-colour outer band 2 dp, the rest 50 % dithered; plus a 1-frame `w` streak line along the blade's
    path. RECOVER's first 0.06 s: the crescent's outer band only, 50 % dithered. WINDUP: nothing (the glint is in the sprite).
  - **Thief:** `slashLit`: a straight stab streak, two lines 1 dp apart along the box's middle, from the hand to the box's
    far end, tapering (3 dp tall at the tip, 1 dp at the hand), `w` core, tier-colour edge; RECOVER's first 0.04 s: the
    tip only (the last third).
- The Shadow Partner draws the same slash 10 dp behind, in `x`/`p` instead of `w`/tier.

### Knight skills (drawn from `heroPose.skillT`)
Remove the `FX_ARC` `addEffect` calls from `castSkill` (Power Slash, Rising Slash, Lion's Roar). In `drawHeroSlash`:
- Power Slash: `t < 0.1`: a gathering glint (3 `w` sparkle pixels near the raised blade); `0.1 <= t <= 0.25`: a big
  crescent filling the 30 x 20 box, 3 dp `w` band + 2 dp tier band, dithered fill, plus 6 speed lines behind it; then
  0.1 s of the outer band fading (dithered, then every other dp).
- Rising Slash: `t <= 0.15`: a vertical crescent in the 24 x 36 box above-front (the arc rotated: bulging upward and
  forward), `w` core; then 0.08 s fading.
- Ground Quake (projectiles, step 8): at cast, `burst` 10 `H`/`s` dust particles at the sword point and a 2-frame dust
  ring (FX_BURST in `s`, radius 10 dp, life 0.15).
- Lion's Roar: `t <= 0.15`: three concentric golden crescents (`N`, `Y`, `o`) inside the 72 x 40 box, the outer one
  growing from 40 % to 100 % of the box over 0.15 s, 50 % dithered fill between them; a mane of 8 short `N` rays from
  the hero; then 0.1 s fading. **Clip to non-solid tiles:** skip every pixel whose tile (`tileAt(q.area, px / TILE, py / TILE)`
  in world coordinates) is solid (the HANDOFF note: the arc dipped into the floor).

### Impacts (`quest_combat.cpp`)
- `hitEnemy` and `hitBoss` add a hit spark at the contact point: enemies `(clamp(sourceX, e.x, e.x + e.w), e.y + e.h / 2)`;
  `hitBoss` gains no new parameter: it uses the boss box edge nearest the hero's centre. `addEffect(q, FX_SPARK, px, py,
  0, 0, SPARK_TIME, strong ? 'N' : 'w')` with `const float SPARK_TIME = 0.12f;`, plus 4 particles in that colour
  (speed 40–80 dp/s, life 0.15 s). Add `FX_SPARK` to `EffectKind` (append; keep the order). Drawn: `SPARK_1/2/3` by
  `age / life` thirds, centred, with `drawSpriteEx(..., 'w', color)` if `strong`.
- Melee basic hit-stop (D-063): in `updateCombat`'s melee window, after `int hits = hitBox(...)`:
  `if (hits > 0) q.hitStop = fmaxf(q.hitStop, BASIC_HIT_STOP);` with `const float BASIC_HIT_STOP = 0.025f;` next to the
  other hit-feel constants. Not for the partner's copy, not for ranged basics.
- Shake: Meteor impact `q.trauma = fminf(1, q.trauma + 0.25f)`, Ground Quake cast `+0.15f`, Lion's Roar cast `+0.2f`.
  These use `shakeStrength` like every shake.
- Log: `castSkill` after the MP is paid: `logLine(q, "quest: skill %s", SKILL_NAMES[cls][job]);` (for the PS3 check).

## Step 8: shots and ranged skills (`art-tools/heroes.py`, `source/quest_draw.cpp`, `source/quest_combat.cpp`)
Class colours (D-063; never `m`):

| Sprite | Art px (doubled) | Look |
|---|---|---|
| `ARROW_FLAT` | 10 x 3 | `;` fletching (2 px, top and bottom), `/` shaft, `b` head with a `w` tip |
| `ARROW_UP`, `ARROW_DOWN` | 10 x 5 | the same arrow at about 20°, stepping 1 px per 2 columns |
| `ARROW_FALL` | 3 x 10 | the arrow pointing down |
| `ARROW_BIG` | 14 x 3 | Piercing Bolt: `b` shaft, a `c`/`w` glowing head, `i` fletching |
| `BOLT_A/B` | 6 x 6 | magic bolt: `u` rim, `c` ring, `w` core; B is A turned 45° (twinkle) |
| `ORB_A/B` | 6 x 6 | frost orb: `I` rim, `i` body, `w` highlight top-left; B rotates the facets |
| `STAR_A/B` | 5 x 5 | 4-point star: `s` body, `b` light edge, `w` glint; B turned 45° |
| `QUAKE_A/B` | 6 x 7 | a rock crest: `d`/`s` rocks, `H` dust tips; B shifted dust |
| `METEOR_ROCK` | 12 x 12 | `V K D` rock, `O`/`N` flames streaming up-back |
| `SPARK_1/2/3` | 7 x 7 | hit spark: a `w` 4-point star growing, then an open ring of 4 dots |

Drawing (`drawCombat`, hero shots): the sprite centred on the hitbox centre (`p.x + p.w / 2`, `p.y + p.h / 2`), mirrored when
`p.vx < 0`. Frame `A/B` by `(int)(q.time / 0.08f) % 2`.
- `PROJ_ARROW`: `ARROW_BIG` if `p.w >= 10 * DP`; else `ARROW_UP` if `p.vy < -0.12f * fabsf(p.vx)`, `ARROW_DOWN` if
  `p.vy > 0.12f * fabsf(p.vx)`, else `ARROW_FLAT`. A speed line behind: 6 dp of `w`, 1 dp tall, 50 % dithered.
- `PROJ_RAIN`: `ARROW_FALL`. `PROJ_BOLT`: `BOLT`, plus a trail of 3 dots behind along `-v`, at 3, 6, 9 dp, sizes 2, 2, 1 dp,
  colours `c`, `u`, `U`. `PROJ_ORB`: `ORB` and a trail of `i`/`I` dots like the bolt. `PROJ_STAR`: `STAR`. `PROJ_WAVE`:
  `QUAKE`; its dust is emitted in `updateCombat`'s shot loop (drawing never adds particles): 1 `H` particle per wave
  every 3 steps.
- `ARROW_BIG` trail: 3 `w`/`c`/`I` streak segments of 4 dp behind it; and when it pierces an enemy, an extra `c` spark (step 7's spark in `c`).

Skills:
- **Arrow Rain zone:** in `castSkill` A1, find the floor under the zone centre like Meteor does (the first `holdsFeet`
  row below the hero's middle, up to 8 rows) and `addEffect(q, FX_ZONE, zoneCentreX, floorY, 56 * DP, 0, 0.5f, 'w')`.
  `FX_ZONE` (append to `EffectKind`): dashes of 2 dp every 4 dp along the floor line, 1 dp tall, across `w`, blinking
  faster in the last 0.15 s (like markers). The falling arrows reach the floor at about 0.5 s.
- **Storm Volley:** its arrows are `ARROW_*` with a `c` glint (draw with `drawSpriteEx(..., 'w', paletteColor('c'))` when
  `p.strong && p.mult < 2`), and each fan adds 4 `c` particles at the bow.
- **Arcane Burst** (`FX_BURST`, redrawn for every use): first 30 % of its life a solid `w` disc (radius 40 % of `f.w`),
  then a ring expanding to exactly `f.w`, 2 dp thick, in `color`, with the inside 50 % dithered; the last 30 % only every
  other ring pixel. The hit circle is radius 20 dp, so the ring's final radius shows its reach.
- **Frost Orb hit:** when a `PROJ_ORB` hits, 6 `i` particles; slowed enemies (`slowTimer > 0`) are drawn with 3 `i`
  frost pixels on their top edge (in `drawCombat`'s enemy loop, after the sprite).
- **Chain Lightning** (`FX_CHAIN`): a jagged line between the points with a 1-dp `w` core and `c` sides, re-jagged
  every 0.05 s (seed the offsets with `(int)(q.time * 20)`); a 2-frame `c` glow disc (radius 4 dp) at each target.
- **Meteor:** at cast, `q.soundRequested[SOUND_Q_CHARGE]`; during its 0.6 s, 2 `o`/`N` particles per step spiral in
  toward the staff tip (start 16 dp away, move toward it; the hero may be running, so aim at the tip's current position).
  `FX_METEOR`: the target marker is an ellipse outline of radius `f.w` (48 dp) by 6 dp on the floor in `o`, blinking
  4 times a second, then 12 in the last 0.2 s; the rock is `METEOR_ROCK` falling from 120 dp above along a slight
  diagonal, with 2 `O` trail particles per step. Impact: the existing `FX_BURST` in `o` (radius 48 dp) plus 16 `V`/`O`
  debris particles and the step 7 shake. The target stays where it was cast, even if the hero runs away.
- **Star Toss:** `SOUND_Q_THROW` instead of `SOUND_Q_STAB`.
- **Shadow Dash:** 3 afterimages: in `updateCombat`, while the dash plays, every 0.05 s add `FX_GHOST` (append) at the
  hero's position with life 0.2; drawn as the current hero sprite silhouette in `x`, 50 % dithered, fading to 25 %. On
  each enemy the dash hits: a horizontal `w` slash line 12 dp long through the spark.
- **Blade Fury:** `FX_SPIN` redrawn: a ring of radius `f.w` (30 dp, the hit radius) around the hero's current centre (it
  moves with the running hero) in the tier colour, 50 % dithered, with a bright `w` 90° segment rotating once per 0.1 s
  (one turn per hit tick), and a `w` pulse (the full ring solid for one frame) at each of the 5 ticks (`h.volleys` changes).
- **Shadow Partner:** at cast, 12 `x`/`p` smoke particles where the partner appears; the partner silhouette stays
  `drawSpriteSolid` in `x`, now drawn 50 % dithered so the hero in front stays clear.

Then `make test`, `make quest-anims`, read every sheet.

## Step 9: sounds (`source/game.h`, `source/audio.cpp`)
Append before `SOUND_COUNT`: `SOUND_Q_HEAVY, SOUND_Q_CHARGE, SOUND_Q_THROW`. Tones (`{start, seconds, startHz, endHz, volume, noise}`):
```cpp
sounds[SOUND_Q_HEAVY] = synthesize({{0, 0.16f, 1800, 400, 0.25f, true}, {0, 0.10f, 120, 80, 0.15f, false}});
sounds[SOUND_Q_CHARGE] = synthesize({{0, 0.60f, 200, 900, 0.08f, false}, {0, 0.60f, 2000, 6000, 0.05f, true}});
sounds[SOUND_Q_THROW] = synthesize({{0, 0.05f, 4000, 6000, 0.08f, false}, {0, 0.06f, 3000, 1500, 0.12f, true}});
```
Use: Power Slash and Rising Slash play `SOUND_Q_HEAVY` (instead of `SOUND_Q_SWING`); Meteor's cast `SOUND_Q_CHARGE`
(instead of `SOUND_Q_SPELL`; the impact keeps `SOUND_EXPLOSION`); Star Toss `SOUND_Q_THROW`. Check that
`host/audio_silent.cpp` needs no change (it takes any `Sound`).

## Step 10: Mac checks (no PS3)
- **Movement while holding R1**, for each class `c` 0–3 (`NAMEGAME_QUEST='slot=3 class=<c> world=8 area=0' make mac-run BUTTONS=...`):
  `2|right+r1:1.5|l1|0.3|left+r1:0.8|cross+left+r1:0.15|left+r1:0.8|right+r1:0.2|quit`. Check `make mac-run`'s stdout
  for no errors; the `l1` shot must show the run legs with an attack arm pose (not a standing pose sliding), and the
  hero reached as far as without R1 (compare with the same script without `+r1`: the shot positions must match within
  a tile). A jump while R1 is held must jump the full height (step 11's tests prove the numbers).
- **Running during R2:** for each class at job 3 (`... level=30 job=3`) and the Knight at job 0, Thief at job 2:
  `2|right:0.5|right+r2:0.1|right:0.8|l1|1|quit`; the shot shows the hero further right than the cast point, mid-run.
- **Every class and tier at gameplay scale:** `make quest-anims`; open each of the 16 sheets and the 4 lineups;
  check each row against "What the player gets" and the readability rules (step 7). Fix and rerun until they hold.
- Send the user (`SendUserFile`, `display: render`) `shots/anims/flipbook.html` plus the 16 sheets, with one line per
  class on what changed. Note anything you think still reads poorly.

## Step 11: tests (`tests/quest_tests.cpp`; `make test` must pass with only the 53 known shooter failures)
- `basicAttackKeepsFullSpeed`: for each class, on a flat floor, two runs of 60 steps with `moveX = 1` (one with `fire`
  held, one without), from standstill: the end `x` is equal within 0.5 dp. A third run with `fire` held and X pressed
  at step 10 reaches the same jump peak (lowest `y`) within 0.5 dp as without `fire`.
- `skillsNeverRootTheHero`: Knight tier 0, Magician tier 3 and Thief tier 2 (level 30, full MP), on a flat floor: two runs
  of 45 steps holding `moveX = 1` from a running start (30 steps first), one pressing R2 on the first counted step: the
  end `x` is equal within 0.5 dp. Meteor in the air (cast during a jump while holding right) moves the same as a plain jump.
- `slashLitMatchesHitWindow`: Knight and Thief tier 0, R1 pressed once; for each step record `heroPose(q).slashLit` and
  whether the enemy (placed in the box as in `knightSwingHitsOnlyInItsWindow`) took damage that step. Every damage step
  has `slashLit`, and `slashLit` is never true more than one step before the window's first damage step or after its
  last frame (`BASIC.to`).
- `meleeBasicHitStops`: Knight hits an enemy; on the step the damage lands, `q.hitStop` is `0.025f` (within 0.001);
  Archer's arrow hit leaves `q.hitStop == 0` for a non-killing hit.
- `rangedDrawShowsWhileHeld`: Archer tier 0 holding R1: within one cycle `heroPose(q).arms` takes `ARMS_ACTIVE` right
  after the shot and `ARMS_WINDUP` in the last 0.2 s; after releasing R1 it ends at `ARMS_NONE`.
- `skillTimingUnchanged`: Knight 0, 1, 3, Magician 0, 2, 3 and Thief 2 (tier = the skill's job, level 30, full MP,
  `godMode`), one `D` enemy placed at the skill's reach: in front (`x = hero.x + HERO_W + 2 * DP`) for Power Slash,
  Rising Slash, Lion's Roar and Blade Fury; centred 40 dp ahead for Arcane Burst; 60 dp ahead for Chain Lightning;
  centred 120 dp ahead for Meteor. Press R2 once; the first damage step must be (s after the press, ±1 step):
  Power Slash 0.1, Meteor 0.6, all others 0. (Projectile skills are covered by the existing tests.)
- `heroSpritesFitTheirBox`: for each class's `IDLE_1`, `IDLE_2`, `RUN_1..4` and `LAND`: no opaque pixel in real-px
  columns 0–3 or 44–47 (art columns 0–1, 22–23), and the outline row (real rows 46–47) has an opaque pixel in real
  columns 16–31 (the feet stand on the hitbox's bottom line).
- Keep `arrowsHitEveryGroundEnemy`, `knightSwingHitsOnlyInItsWindow` and `skillCastMidSwingStillHits` unchanged and passing.

## Step 12: docs
- `spec.md`: §5, §6, §7, §16, §21 and §22 were already updated when this phase was planned (D-061 to D-064); fix them only
  where the built result differs, and say why in `implementation.md` › Deviations.
- `docs/features/name-quest/design.md`: a "Rules (phase 5b: heroes and combat feel)" section (6–8 bullets: no R1 slow,
  no R2 root, pose rules, slash window lighting, hit-stop, shot looks, readability order).
- `implementation.md`: code map rows for `quest_sprites_hero.{h,cpp}`, `art-tools/heroes.py`, `flipbook.py`,
  `heroPose`, `make quest-anims`; `castTimer` and `SKILL_CAST` removed (D-064).
- `testing.md`: a Phase 5b section: the style preview approval, the Mac checks, and the PS3 checks below (unticked).
- `docs/GDD.md`: a change log entry `**<next version> (date):** NAME QUEST phase 5b: redrawn heroes, animation and combat feel; no R1 slowdown, no R2 roots (D-061 to D-064).`; bump the version line.
- `docs/HANDOFF.md`: the phase table row and the session log.
- Run `python3 .claude/skills/game-docs/scripts/check_docs.py docs`: no new problems.

## Step 13: PS3 readability and feel check (only when the user asks; ask first)
One locked command per run (AGENTS.md). Build with `make ps3`, then:
```
scripts/ps3-lock.sh bash -c 'make log > /tmp/nq5b.log & LOG=$!; make install launch && make pad BUTTONS="..." && make shot; kill $LOG'
```
(the hero is chosen in the slot menu: use dev saves as in phase 4 step 10, one per class at job 3, written by the Mac
with `NAMEGAME_QUEST='slot=3 class=<c> world=1 area=0 level=30 job=3'` and copied to `pkgfiles/USRDIR/quest3.txt`;
delete the copy afterwards). Expect `quest: skill <NAME>` for every R2 pressed and `fps` lines ≥ 59 while effects play.
Ask the user to play each class for a minute on the TV: R1 held while running and jumping, every skill of the tier while
running, and whether enemy shots and hazards stay readable. Record the notes in `testing.md`; don't retune numbers
(master plan rule 2).

## Done when
- `make test` passes (only the 53 known shooter failures), `make ps3` builds, `check_docs.py` has no new problems.
- The user approved the style preview (step 5) and has the flipbook of every class and tier (step 10).
- On the Mac, every class runs at full speed with R1 held and during every R2 skill, attack arms over running legs,
  and jumps and lands without a stop.
- No damage, hitbox, range, MP or timing value changed except D-062's R1 speed, D-063's melee hit-stop and D-064's removed roots.
- On the PS3 (when the user asks): every skill logs `quest: skill`, fps stays ≥ 59, and the user's notes are recorded.
