# Phase 2: combat (classes, jobs, enemies, damage, death and names)

Read first: [`../name-quest-plan.md`](../name-quest-plan.md), then spec §5 (attack and dodge rows), §6, §7, §8, §9
(the hazard damage only), §15 (HUD HP/MP/names), §18, §21 (hero frames, projectile colours). Phase 1 must be done.

## What the player gets
- Every class attacks with R1 (hold to repeat), R2 casts the tier's strong skill (MP), Circle dodges (roll / blink),
  the Thief double-jumps. HP and MP bars and the names counter work.
- TEST GROUNDS gains enemies of all 7 behaviours (world 1's roster stats for now); hits flash, knock back, float
  damage numbers and shake; kills drop names straight into the counter.
- Dying (HP, pit, liquid) drops every carried name as an echo; touching it brings them back; dying again first loses
  them. Enemies respawn on death.
- Dev: `NAMEGAME_QUEST='... job=3'` shows the tier-3 skills.

## Step 1: stats and formulas (`source/quest.h`, new `source/quest_stats.cpp`)
Pure functions and tables, no state, all testable:
```cpp
struct ClassInfo { const char* name; float hp, hpG, mp, mpG, atk, atkG, def, defG, walk, mpRegen, crit, critMult; };
extern const ClassInfo CLASSES[CLASS_COUNT];               // spec §6 base table
extern const char* const JOB_NAMES[CLASS_COUNT][4];        // KNIGHT VANGUARD WARDEN LIONHEART, ...
extern const char* const SKILL_NAMES[CLASS_COUNT][4];      // POWER SLASH ... SHADOW PARTNER
struct HeroStats { int maxHp, maxMp, atk, def; float walk, mpRegen, crit, critMult; };
HeroStats heroStats(const SaveData& save);                 // class, level, job, weapon, armor, refine, charm (spec §6, §10)
struct RefStats { float hp, atk, def; };
RefStats referenceHero(int world);                         // spec §7: average class at REF_LEVEL with REF_GEAR and the level's job
enum EnemyKind { KIND_FODDER, KIND_SOLDIER, KIND_ELITE, KIND_MINIBOSS, KIND_BOSS };
int enemyAttack(int world, float hitFraction, int ngPlus);
int enemyHealth(int world, float hitsToKill, int ngPlus);
int enemyNames(int world, float namesMult, int ngPlus, bool strangerRing);
int levelCost(int level);                                  // names to go from level to level + 1 (spec §10)
int damageToHero(int enemyAtk, int heroDef);               // spec §7
float jobMultiplier(int job);                              // 1, 1.1, 1.2, 1.3
```
- `heroStats`: `floor` each stat after adding the level part; HP and ATK x `jobMultiplier`; then weapon ATK
  (x1.1^refine) and armor DEF/HP from the spec §10 tables (`WEAPON_ATK[7]`, `ARMOR_DEF[6]`, `ARMOR_HP[6]`); then
  Tobin's charm (HP x1.05) if the key bit is set. `walk` from the class (buffs are applied by the caller).
- `referenceHero`: average of the four classes' raw `heroStats` with level `REF_LEVEL[world]`, job = 3 if level ≥ 30,
  2 if ≥ 15, 1 if ≥ 5, else 0; weapon = armor = `REF_GEAR[world]`; no refine, no charm. Return floats (no flooring of the average).
- NG+ factor `1 + 0.5 * min(ngPlus, 3)`.

## Step 2: enemies (`source/quest.h`, new `source/quest_combat.cpp`)
```cpp
enum Behaviour { BEH_WALKER, BEH_CHARGER, BEH_FLYER, BEH_SHOOTER, BEH_JUMPER, BEH_ARMORED, BEH_DROPPER };
enum EnemyTrait { TRAIT_NONE, TRAIT_BANANA_ON_LAND, TRAIT_REVIVE, TRAIT_INFLATE, TRAIT_STEAL_NAMES };
// One roster slot of a world (spec §8 rosters).
struct EnemyType { const char* name; Behaviour behaviour; EnemyKind kind; EnemyTrait trait;
                   const Sprite* frames[2]; float speedScale; Sound attackSound; };
const EnemyType& rosterType(int world, int slot);          // slot 0..4 = A, B, D, E, F
struct Enemy {
    int type;                 // roster slot
    int spawn;                // index of its letter in the area (bit in the killed list)
    float x, y, vx, vy;       // top-left of the hitbox
    float homeX, homeY;       // spawn point (flyers drift around it)
    int hp, maxHp, atk;
    int state; float stateTimer;   // behaviour state machine (patrol, tell, act, recover, stunned...)
    bool facingLeft, alive;
    float flash, knockTimer, slowTimer;
    bool revived;             // phoenix
};
struct Projectile { float x, y, vx, vy; int w, h; int damage; bool fromHero; int pierce; float range; int kind; };
struct DamageNumber { float x, y, age; int value; uint32_t color; };
struct Particle;   // reuse game.h's Particle
struct NameEcho { int names, world, area; float x, y; };
```
Add to `Quest`: `std::vector<Enemy> enemies; std::vector<Projectile> projectiles; std::vector<DamageNumber> numbers;
std::vector<Particle> particles; std::vector<uint64_t> killed[WORLD_COUNT];` (per area, bits by spawn index; size
the inner vector to the world's area count on open), `float hitStop;`.
- Rosters: TEST GROUNDS uses world 1's roster but world 1's stats. World 1–6 rosters per spec §8 (all 6 now: the table
  is small, and later phases only author maps). New sprites listed there are authored in the world's phase; until then
  use the shooter sprite of the same slot or a 12x12 placeholder box sprite `PLACEHOLDER_ENEMY` in `n`/`s`.
- `loadArea` (caller side, in quest.cpp) spawns an `Enemy` for each `A B D E F` whose killed bit is clear: stats from
  `enemyAttack/Health/Names` with the kind's spec §7 numbers.
- One `updateEnemy(Quest&, Enemy&, float dt)` with a `switch (behaviour)`; the numbers and states of spec §8 exactly.
  Patrol turn rule: a walker turns when the tile ahead at foot level is solid or the tile ahead below the feet is
  air/liquid (never walks into pits). Enemies use the same solid-tile rules as the hero (flyers ignore tiles).
- Enemy shots: `Projectile` with `fromHero = false`, 4x4, magenta, die on solid tiles or after 3 s.

## Step 3: hero combat (`source/quest.h`, `source/quest_combat.cpp`, `source/quest.cpp`)
Add to `Hero`: `int hp, mp; float mpFraction;` (MP regen accumulates) `float attackTimer, strongTimer, castTimer;
float rollTimer, rollCooldown, iframes, hurtTimer; float partnerTimer; float buffTimers[3];
float drinkTimer;` (drink comes in phase 3; declare now).
- Basic attack (R1 held, `attackTimer` ≤ 0): starts the class's cycle (spec §6). Melee: one hitbox, active in its
  window, each enemy hit once per swing. Ranged: spawn the projectile at the hand (6 px ahead, 6 px down from the top).
  Sounds: SWING (Knight), ARROW, SPELL, STAB. A landed basic hit gives +1 MP.
- Strong attack (R2 pressed): the tier's skill from spec §6, exactly as written; not enough MP → SOUND_ROCKET_EMPTY.
  Skills with a cast time root the hero on the ground for it. Meteor, Arrow Rain, Ground Quake, Chain Lightning, Storm
  Volley and Shadow Partner each get a small block in the strong-attack `switch`; reuse `Projectile` for arrows,
  stars, orbs, waves (a ground wave is a projectile that follows the floor and ends at a wall or ledge).
- Dodge (Circle = `a.back`): roll or blink per spec §5. Thief double jump already exists (phase 1).
- Damage to enemies per spec §7, with crits, damage numbers, flash, knockback, hit-stop (strong-attack hits and
  kills 0.04 s), SOUND_Q_HIT / SOUND_Q_CRIT. Kill: 8 particles in the enemy's main colour, names added (x1.15 with the
  Stranger's ring), `kills += 1`, the spawn's killed bit set. Phoenix revive (spec §8).
- Damage to the hero: contact with a living enemy's hitbox, an enemy projectile, spikes, electric panel (when on),
  fire jet (when burning): §7 numbers; knockback, i-frames, SOUND_Q_HURT, hit-stop 0.08, trauma 0.3, red damage
  number. No damage while `iframes > 0` or `godMode`.
- `hitStop` freezes enemies, projectiles and the hero (timers too) but not particles or damage numbers, like the shooter.
- MP regen per spec §6; HP never regenerates.

## Step 4: death and the name echo (`source/quest.cpp`)
- HP ≤ 0, a pit or a liquid → death per spec §7: the echo is created at `lastSafeX/Y` (HP deaths: the hero's position
  if grounded safely, else `lastSafe`), replacing any older echo; `save.names = 0`; log
  `quest: hero died in %d.%d (%s), dropped %d names`; DEAD mode; then respawn: clear every `killed` list, full HP/MP,
  buffs off, autosave.
- Test mode (`godMode`): a pit or liquid puts the hero at `lastSafeX/Y` instead (spec §20).
- The echo is drawn (8 x 8 wisp, `y`/`w`, bobbing) in its world and area; touching it: names back, `+n NAMES` (`y`,
  rising like a damage number but 1.2 s), SOUND_Q_NAMES, log `quest: names recovered %d`, echo cleared, autosave.

## Step 5: drawing (`source/quest_draw.cpp`)
Enemies (2 frames every 0.25 s, white silhouette while flashing via `drawSpriteSolid`, tells: flashing or a 1 px
shake as the behaviour says), projectiles (hero `y` lines/dots, enemy `m` 4x4), skills' visuals (arcs and waves as
short-lived filled shapes in the class colours; Meteor's marker and fall; Shadow Partner as the hero sprite drawn
solid `x` 10 px behind), the hero blinking during i-frames (every 0.1 s), damage numbers, particles, HUD (LV, HP and MP
bars, `HP n/m`, names with the wisp icon). Hero ATTACK frame while an attack plays; ROLL during a roll.

## Step 6: TEST GROUNDS enemies (`source/quest_areas.cpp`)
Place spawns: area 0 two `A` and one `B`; area 1 (tall) one `B` and one `E`; area 2 `D` near the spikes, one `F`;
area 3 one of each `A B D E F`. Keep the reachability test green (enemies don't affect it).

## Step 7: tests (`tests/quest_tests.cpp`)
- `levelCostMatchesSpec`: `levelCost(1) == 100`, `levelCost(4) == 173`, `levelCost(14) == 1070`, `levelCost(39) == 102064`.
- `referenceHitIsTwentyPercent`: for worlds 1–7, `damageToHero(enemyAttack(w, 0.20, 0), round(ref.def))` is within
  ±1 % of `0.20 * ref.hp` (and 0.15, 0.25 for fodder and elite).
- `classesDifferAsDesigned`: at level 10 the Knight has the most HP and DEF, the Magician the most MP and ATK, the
  Thief the fastest walk; every class's tier-3 skill exists (`SKILL_NAMES` not empty).
- `basicAttackKillsFodderInThreeHits` (reference-level hero vs world-1 fodder, no crit: seed the RNG so it doesn't crit,
  or force `crit = 0` in the test hero).
- `knightSwingHitsOnlyInItsWindow`, `arrowsStopAtWalls`, `chargerTellsThenCharges`, `armoredGuardsItsFront`,
  `dropperFallsWhenHeroBelow`, `walkersNeverWalkIntoPits`.
- `iframesBlockDamage`, `rollGivesIframes`, `blinkStopsBeforeWalls`, `strongAttackNeedsMp`.
- `deathDropsNamesAtLastSafeGround`, `secondDeathLosesTheOldEcho`, `echoReturnsNames`, `deathRespawnsEnemies`.
- `make test` passes.

## Step 8: Mac self-check
Script each class against TEST GROUNDS area 3 with `job=0` and `job=3`, take `l1` shots mid-attack and read them:
```
NAMEGAME_QUEST='slot=3 class=0 world=8 area=3 level=10 job=3' make mac-run BUTTONS='1|r1:2|r2|1|l1|1|circle|1|quit'
```
Check: the skill's shape is visible and readable at 320x180, numbers float, the HUD's MP drops. Walk into an elite
without blocking and die on purpose; confirm the echo and recovery in stdout.

## Step 9: docs
- `docs/features/name-quest/design.md`: add the combat rules (numbered, short, pointing to spec §6–§8 for numbers);
  implementation.md: the new files; testing.md: this phase's PS3 checks.
- GDD change log `0.30 (date): NAME QUEST combat`. Run `check_docs.py`.

## Step 10: PS3 check
First add the dev rule of spec §20 "Select held 2 s → TEST GROUNDS area 0" to the dev aids in `quest.cpp` (a held-time
counter on `Actions::autopilot` is not enough since it is a press: add `bool selectHeld;` to `Actions`, filled like
`jumpHeld` in both input files).
```
scripts/ps3-lock.sh bash -c 'make install launch && (make log > /tmp/nq-log.txt &) && make pad BUTTONS="down|down|cross|1|cross|2|select:2.2|1|select|1|select|1|select|2|r1:3|r2|1|l1" && make shot; make pad BUTTONS="start|1|down|down|cross|3"; make quit'
```
(loads slot 1 from phase 1, goes to TEST GROUNDS, warps to area 3, attacks, casts, screenshot, back to the menu.)
Expect: `quest: area 8.x` lines, `quest: hero died ...` if the hero dies, fps ≈ 60 with 8 enemies and skills on
screen (worst work < 10 ms). The screenshot shows an attack. Tick the testing.md criteria.

## Done when
- `make test` passes, `make ps3` builds, `check_docs.py` is clean.
- On the PS3: an Archer fights TEST GROUNDS enemies at 60 fps; death drops names and touching the echo returns them.
