# Plan: level buffs and a ship skin per level

For the implementer: follow the steps in order. Every design choice below is final, so don't redesign anything.
Read only the files each step names, and don't re-read a file after editing it. Code style: follow `CLAUDE.md` and
the existing code (plain C++14, no new libraries, short one-line comments, no helpers for one-off logic).
Don't commit: the user commits. When done, list the changed files and suggest a one-line commit message.
The working tree already has uncommitted playtest fixes (D-029); build on top of them and don't revert anything.

Source: the user's "Level based stuff" PDF (2026-11-18). Choices the user made in the planning session are marked
*(user)*; the rest were taken by the planner. Ship preview drawn while planning: `docs/work-plan/level-buffs/ship-skins-draft.png`.

## What the player gets

### Ship skin per level
Each level draws the player's ship in its own look, on the same 16×14 body with the same 6×6 hitbox (fairness:
nothing about dodging changes). The cyan cockpit stays on player 1 in every level; player 2 gets a gold copy of
each skin with an orange cockpit, as today. Space: today's ship. Jungle: wooden hull with leaf wings. Glacier: pale
ice-crystal ship. Volcano: slate obsidian with glowing lava seams. Deep sea: a grey submarine with yellow portholes
that trails bubbles instead of flames. *(user)*

### Level buffs
- **Drop:** every big boss kill (waves 15 and 20, so the twins of wave 20 drop one each) drops a **buff pickup**
  for that level *(user)*: an 11×11 blinking yellow frame with the level's icon, falling at 30 px/s like other pickups.
  The ship that touches it gets the buff; a missed pickup is gone *(user)*. A buff you already hold pays 5,000 points.
- **Stacking:** buffs from different levels stack. In Classic they carry over to the next level with the score.
  **Dying loses every buff** (and the armed strong attack, a raised shield and a charging beam).
- **Strong attack:** R2 is now the "strong attack". By default it is the rocket. The space, glacier and deep-sea
  buffs each add a strong attack; **Square cycles** which one R2 fires *(user)*, in the order
  rocket → magic beam → frost wave → bubble shield (skipping ones not held, wrapping). Collecting one of these three
  buffs arms it at once. Every strong attack costs 1 rocket and shares the rocket's 0.5 s cooldown and R2 buffer;
  with 0 rockets R2 plays the empty click as today.
- **HUD:** each player's held buffs are 5×5 icons above their rocket icons, in level order; the armed strong
  attack's icon is underlined in yellow.

| Buff | Effect |
|---|---|
| Space | A wand sits on the ship's right wing. Strong attack **magic beam**: a magic ball grows at the wand's star for 0.2 s, then a 12 px beam (half the mage's 24 px) shoots from the star to the top of the screen for 0.4 s. At the moment it fires, every alien, boss and live fish touching the column takes **30 damage** once *(user)*. Cyan edges, white 4 px core (never magenta). |
| Jungle | Bullets are drawn as bananas. No other effect. |
| Glacier | Strong attack **frost wave**: an ice ring sweeps out from the ship; **every enemy freezes for 3 s** *(user)* (no moving, swaying, diving, shooting or attacking; still deadly to touch; still takes damage); every living alien and boss heals **+25 % of its max HP** (rounded up, capped at max); every enemy projectile and in-flight attack is destroyed (enemy shots, thrown hazards, lobs in flight, live fish, a charging or firing mage ball/beam or eruption, bursts, a gorilla pound or yeti sneeze wind-up, the fisherman's hook; a hooked ship is let go). Peels and stink mists on the ground stay. Clearing the wave ends the freeze. |
| Volcano | Bullets burn (flame bullets; fire bananas with the jungle buff) and fire **25 % faster** (interval × 0.8). Damage unchanged *(user)*. |
| Deep sea | Strong attack **bubble shield**: a bubble ring around the ship. The next deadly hit pops it instead of killing (1 s of blinking invincibility after, so overlapping shots or a beam can't kill straight away), and 3 big bubbles burst out. A bubble flies off in a random direction for 0.2 s (it can't hit yet), then homes on a random living enemy. When it hits, it does **2 damage** *(user)* and splits into 3 bubbles one size smaller; size 3 (8 px) → 2 (4 px) → 1 (2 px); a size-1 hit stops the chain. Up to 39 hits, 78 damage. R2 with a shield already up does nothing and spends no rocket. Non-lethal hazards (ice block, lava drop, hook, peels) don't touch the shield. |

### Why it stays fair and not too easy
Buffs come only from big bosses and must be caught; all are lost on death; each strong attack spends a scarce rocket;
the frost wave heals what it freezes (a big boss gets back 85-105 HP); the shield needs a hit to deal damage; the
volcano buff adds speed, not damage. Level 1's numbers and every enemy stay unchanged.

## Input rule
- Square is button 8, known only from the SDL port's source. It is used only during play. If it turns out wrong on the
  PS3, R2 still fires the armed attack (the last collected strong buff), so nothing becomes unreachable.

## Step 1: sprites (`source/sprites.h`)
Add at the end of the sprites, just before the `// 3x5 font` comment, with the comments shown.
No sprite below may contain `m`. Don't rerun `make_xmb_art.py` (the XMB art keeps `SHIP_A`).

Ship skins, frame A for player 1 (16×14 each):
```cpp
// Ship skins per level (same 16x14 body and hitbox): jungle wood and leaves, glacier ice, volcano obsidian, deep-sea submarine.
const Sprite SHIP_JUNGLE_A = {16, 14,
    ".......cc......."
    "......cwwc......"
    "......hwwh......"
    ".....hHHHHh....."
    ".....hBhhBh....."
    "..l..hBhhBh..l.."
    ".llg.hBhhBh.gll."
    "lggghhBccBhhgggl"
    ".lgghHHccHHhggl."
    "..lghhhhhhhhgl.."
    "...hhhhhhhhhh..."
    "....hh....hh...."
    "....oo....oo...."
    ".....o....o....."};
const Sprite SHIP_GLACIER_A = {16, 14,
    ".......cc......."
    "......cwwc......"
    "......cwwc......"
    ".....iIwwIi....."
    "....iIIIIIIi...."
    ".i..IIqIIqII..i."
    ".iI.IqIIIIqI.Ii."
    "iIIiIIIccIIIiIIi"
    ".IIIIIIccIIIIII."
    "..qIIIIIIIIIIq.."
    "...qIiIIIIiIq..."
    "....qq....qq...."
    "....oo....oo...."
    ".....o....o....."};
const Sprite SHIP_VOLCANO_A = {16, 14,
    ".......cc......."
    "......cwwc......"
    "......cwwc......"
    ".....XXwwXX....."
    ".....XOXXOX....."
    "..N..XXOOXX..N.."
    "..O.XXXXXXXX.O.."
    ".OX.XXXccXXX.XO."
    ".XXXOXXccXXOXXX."
    "XXXXXOXXXXOXXXXX"
    "XXXOXXXXXXXXOXXX"
    ".XX.OO....OO.XX."
    "....NO....ON...."
    ".....O....O....."};
const Sprite SHIP_SEA_A = {16, 14,
    ".......cc......."
    "......cwwc......"
    "......bccb......"
    "....bbbbbbbb...."
    "...bbNbbbbNbb..."
    "..bbNwNbbNwNbb.."
    "..bbbNbbbbNbbb.."
    ".IbbbbbccbbbbbI."
    "IIbbbbbccbbbbbII"
    ".IbbbbbbbbbbbbI."
    "..bbbbbbbbbbbb.."
    "....bb....bb...."
    "....ii....ii...."
    ".....i....i....."};
```
- Frame B of each (`SHIP_JUNGLE_B`, `SHIP_GLACIER_B`, `SHIP_VOLCANO_B`, `SHIP_SEA_B`) is frame A with only rows 12 and 13 replaced:
  jungle and glacier `"....yo....oy...."` / `"................"` (as `SHIP_B`); volcano `"....ON....NO...."` / `"................"`;
  sea `".....i....i....."` / `"....i......i...."`.
- Player 2's copies `SHIP2_JUNGLE_A/B`, `SHIP2_GLACIER_A/B`, `SHIP2_VOLCANO_A/B`, `SHIP2_SEA_A/B`: the player-1 grid with
  every `c` replaced by `o` and the skin's hull character replaced by `Y`: jungle `h`, glacier `I`, volcano `X`, sea `b`.
  Nothing else changes. Comment: `// Player 2's skins: the hull gold and the cockpit orange, as SHIP2_A.`

Buff art:
```cpp
// Level buffs: the wand on a ship with the space buff (star centre at 1, 1), banana and fire bullets, shield bubbles.
const Sprite WAND = {3, 7, ".N.NwN.N..h..h..h..h."};
const Sprite PLAYER_BANANA = {3, 5, ".h...Y..Y.YYYY."};
const Sprite FIRE_BANANA = {3, 5, ".N.N.YO.Y.YYYYO"};
const Sprite FIRE_BULLET = {2, 4, "NNNOOOOZ"};
const Sprite SHIELD_BUBBLE_3 = {8, 8,
    "..iiii.."
    ".i....i."
    "iww....i"
    "iw.....i"
    "i......i"
    "i......i"
    ".i....i."
    "..iiii.."};
const Sprite SHIELD_BUBBLE_2 = {4, 4, ".ii.iw.ii..i.ii."};
const Sprite SHIELD_BUBBLE_1 = {2, 2, "wiii"};
// Level buff icons (HUD and pickup), by level: wand star, banana, snowflake, flame, bubble.
const Sprite BUFF_ICON_SPACE = {5, 5, "..N..NNwNN.NwN..N.N.N...N"};
const Sprite BUFF_ICON_JUNGLE = {5, 5, "....h...Y...YY.YYY...Y..."};
const Sprite BUFF_ICON_GLACIER = {5, 5, "i.i.i.iii.iiwii.iii.i.i.i"};
const Sprite BUFF_ICON_VOLCANO = {5, 5, "..O...OO...ONO.ONNNO.OOO."};
const Sprite BUFF_ICON_SEA = {5, 5, ".iii.iw..ii...ii...i.iii."};
```
(The one-line grids are the rows concatenated; write them one row per line like the other sprites, same characters.
The icons are `BUFF_ICON_*` because `game.h` uses `BUFF_*` for the buff bits.)

## Step 2: declarations (`source/game.h`)
- `struct Actions`: add after `godMode`: `bool cycle;                  // Square pressed: arm the next held strong attack (level buffs)`.
- `enum PickupType`: add `PICKUP_BUFF`. `struct Pickup`: becomes
  `struct Pickup { float x, y; PickupType type; int level; };   // level: the buff's level, for PICKUP_BUFF`.
- Add before `struct Player`:
  ```cpp
  // Level buffs: bit (level - 1) of Player::buffs (docs/features/level-buffs).
  const int BUFF_SPACE = 1, BUFF_JUNGLE = 2, BUFF_GLACIER = 4, BUFF_VOLCANO = 8, BUFF_SEA = 16, BUFF_ALL = 31;
  ```
- `struct Player`: add at the end:
  ```cpp
  int buffs;               // level buffs held: bit (level - 1); all lost on death
  int strongAttack;        // what R2 fires: 0 rocket, or the buff's level: 1 magic beam, 3 frost wave, 5 bubble shield
  bool shield;             // deep-sea bubble shield up: the next deadly hit pops it instead of killing
  float beamCharge;        // s left of the magic ball growing at the wand; the beam fires at 0
  float beamTimer;         // s the magic beam stays drawn
  float beamX, beamBottom; // the beam's centre x and the y it rises from
  ```
- Add after `struct Fish`:
  ```cpp
  // A bubble from a popped shield, centred at x, y; size 3, 2, 1; homes on alien index `alien` or boss index `boss` (-1: none).
  struct ShieldBubble { float x, y, vx, vy, age; int size, owner, alien, boss; };
  ```
- `struct Game`: add `std::vector<ShieldBubble> shieldBubbles;   // homing bubbles from popped shields` after `fish`, and after `sceneryTime`:
  ```cpp
  float freezeTimer;     // s left of a frost wave: no enemy moves, shoots or attacks
  float frostAge = -1;   // s since the frost wave's ring left the ship, or -1
  float frostX, frostY;  // where the ring started
  ```

## Step 3: input (`source/input.cpp`)
- Add `BTN_SQUARE = 8` to the button enum. `a.cycle = pressed[player][BTN_SQUARE];` next to `a.godMode`.

## Step 4: rules (`source/game.cpp`)
Constants, a new block after the deep-sea block (before `// Pixel art`):
```cpp
// Level buffs (level-buffs/design.md)
const int BUFF_PICKUP_SIZE = 11, BUFF_HELD_POINTS = 5000;   // px; points for a buff already held
const float MAGIC_CHARGE_TIME = 0.2f, MAGIC_BEAM_TIME = 0.4f;   // s
const int MAGIC_BEAM_W = BEAM_W / 2, MAGIC_BEAM_CORE_W = 4, MAGIC_DAMAGE = 30;   // px, px, HP per enemy
const int SHIP_WAND_X = 14, SHIP_WAND_Y = 3;   // px in the ship: the wand's star (the WAND sprite is drawn at +13, +2)
const float FROST_TIME = 3, FROST_RING_TIME = 0.4f, FROST_RING_SPEED = 450, FROST_THAW_BLINK = 0.5f;   // s, s, px/s, s
const float VOLCANO_FIRE_FACTOR = 0.8f;   // x the fire interval: 25 % faster
const int SHIELD_RADIUS = 11, SHIELD_SPLIT = 3, BUBBLE_DAMAGE = 2;   // px, bubbles per burst, HP per hit
const float SHIELD_POP_INVINCIBLE = 1, SHIELD_BUBBLE_SPREAD = 0.2f, SHIELD_BUBBLE_LIFE = 3;   // s
const float SHIELD_BUBBLE_SPEED[4] = {0, 150, 130, 110};   // px/s by size
const int SHIELD_BUBBLE_D[4] = {0, 2, 4, 8};               // px by size
```
After `THROWN_SPRITES`, add the skin table:
```cpp
const Sprite* const SHIP_SKINS[LEVEL_COUNT][2][2] = {{{&SHIP_A, &SHIP_B}, {&SHIP2_A, &SHIP2_B}},
    {{&SHIP_JUNGLE_A, &SHIP_JUNGLE_B}, {&SHIP2_JUNGLE_A, &SHIP2_JUNGLE_B}}, {{&SHIP_GLACIER_A, &SHIP_GLACIER_B}, {&SHIP2_GLACIER_A, &SHIP2_GLACIER_B}},
    {{&SHIP_VOLCANO_A, &SHIP_VOLCANO_B}, {&SHIP2_VOLCANO_A, &SHIP2_VOLCANO_B}}, {{&SHIP_SEA_A, &SHIP_SEA_B}, {&SHIP2_SEA_A, &SHIP2_SEA_B}}};   // [level - 1][player][frame]
const Sprite* const BUFF_ICONS[LEVEL_COUNT] = {&BUFF_ICON_SPACE, &BUFF_ICON_JUNGLE, &BUFF_ICON_GLACIER, &BUFF_ICON_VOLCANO, &BUFF_ICON_SEA};
const Sprite* const SHIELD_BUBBLE_SPRITES[4] = {NULL, &SHIELD_BUBBLE_1, &SHIELD_BUBBLE_2, &SHIELD_BUBBLE_3};
```

Edits, in file order:
1. `damageAlien`: the pickup push becomes `{cx - 4, cy - 4, roll < PICKUP_CHANCE ? PICKUP_ROCKET : PICKUP_UPGRADE, 0}`.
2. `damageBoss`: the two pushes get a trailing `, 0`; then add
   `if (game.wave >= BIG_BOSS_WAVE) game.pickups.push_back({cx - BUFF_PICKUP_SIZE / 2.0f, cy + 8, PICKUP_BUFF, game.level});   // big bosses drop the level buff`.
3. `killPlayer`: add `player.buffs = player.strongAttack = 0; player.shield = false; player.beamCharge = player.beamTimer = 0;` (dying loses every buff).
4. New function after `killPlayer` (it touches many lists, so it gets a name):
   ```cpp
   // The glacier buff's strong attack: every enemy freezes and heals a quarter of its max HP; every incoming attack shatters.
   static void frostWave(Game& game, const Player& player) {
       game.freezeTimer = FROST_TIME;
       game.frostAge = 0;
       game.frostX = player.x + SHIP_W / 2.0f;
       game.frostY = player.y + SHIP_H / 2.0f;
       for (Alien& alien : game.aliens)
           if (alien.alive) alien.hp = std::min(alien.hp + (ALIEN_HP[alien.type] + 3) / 4, ALIEN_HP[alien.type]);
       for (Boss& boss : game.bosses) {
           if (!boss.alive) continue;
           boss.hp = std::min(boss.hp + (boss.maxHp + 3) / 4, boss.maxHp);
           boss.burstShotsLeft = 0;
           boss.ballTimer = boss.beamTimer = boss.windupTimer = 0;
           boss.hookState = HOOK_NONE;
       }
       for (Player& p : game.players) p.hookedTimer = 0;
       for (const Shot& s : game.enemyShots) spawnParticles(game, s.x + 1, s.y + 2, 2, paletteColor('i'), 30);
       for (const Thrown& t : game.thrown) spawnParticles(game, t.x + THROWN_W[t.kind] / 2.0f, t.y + THROWN_H[t.kind] / 2.0f, 3, paletteColor('i'), 30);
       for (const Fish& f : game.fish) spawnParticles(game, f.x + FISH_W / 2.0f, f.y + FISH_H / 2.0f, 3, paletteColor('i'), 30);
       game.enemyShots.clear();
       game.thrown.clear();
       game.lobs.clear();
       game.fish.clear();
       feedback(game, TRAUMA_BEAM, 0, FLASH_BLAST);
       game.soundRequested[SOUND_FREEZE] = true;
   }
   ```
5. `updatePlayer`, right after the `if (!player.alive) { … return; }` block and before the scorch lines, the magic beam
   (it runs even while the ship is stunned or frozen):
   ```cpp
   player.beamTimer = std::max(0.0f, player.beamTimer - dt);
   if (player.beamCharge > 0) {   // the magic ball grows at the wand, then beams up its column (space buff)
       player.beamX = player.x + SHIP_WAND_X;
       player.beamCharge -= dt;
       if (player.beamCharge <= 0) {
           player.beamCharge = 0;
           player.beamTimer = MAGIC_BEAM_TIME;
           player.beamBottom = player.y + SHIP_WAND_Y;
           float left = player.beamX - MAGIC_BEAM_W / 2.0f;
           for (Alien& alien : game.aliens)
               if (alien.alive && overlap(left, 0, MAGIC_BEAM_W, player.beamBottom, alien.x, alien.y, ALIEN_WIDTH[alien.type], ALIEN_HEIGHT[alien.type]))
                   damageAlien(game, alien, MAGIC_DAMAGE, index);
           for (Boss& boss : game.bosses)
               if (boss.alive && overlap(left, 0, MAGIC_BEAM_W, player.beamBottom, boss.x, boss.y, BOSS_W, BOSS_H)) damageBoss(game, boss, MAGIC_DAMAGE, index);
           for (Fish& f : game.fish) {
               if (f.life <= 0 || !overlap(left, 0, MAGIC_BEAM_W, player.beamBottom, f.x, f.y, FISH_W, FISH_H)) continue;
               f.life = 0;   // removed in updateGame
               addScore(game, player, FISH_SCORE);
               spawnParticles(game, f.x + FISH_W / 2.0f, f.y + FISH_H / 2.0f, 8, paletteColor('O'), 50);
           }
           feedback(game, TRAUMA_BEAM, 0, 0);
           game.soundRequested[SOUND_EXPLOSION] = true;
       }
   }
   ```
6. `updatePlayer`, the fire interval line becomes
   `player.fireTimer += (player.weaponLevel == MAX_WEAPON_LEVEL ? FAST_FIRE_INTERVAL : FIRE_INTERVAL) * (player.buffs & BUFF_VOLCANO ? VOLCANO_FIRE_FACTOR : 1);`
7. `updatePlayer`, the R2 block: replace the body of `if (player.rocketPressedAt >= 0 && player.rocketCooldown == 0)` with:
   ```cpp
   player.rocketPressedAt = -1;
   bool shieldUp = player.strongAttack == 5 && player.shield;   // a second shield can't stack and costs nothing
   if (player.rockets > 0 && !shieldUp) {
       player.rockets--;
       player.rocketCooldown = ROCKET_COOLDOWN;
       if (player.strongAttack == 1) {
           player.beamCharge = MAGIC_CHARGE_TIME;
           player.beamTimer = 0;
           player.beamX = player.x + SHIP_WAND_X;
           game.soundRequested[SOUND_CAST] = true;
       } else if (player.strongAttack == 3) {
           frostWave(game, player);
       } else if (player.strongAttack == 5) {
           player.shield = true;
           game.soundRequested[SOUND_BUBBLE] = true;
       } else {
           <today's rocket push and SOUND_ROCKET_LAUNCH, unchanged>
       }
   } else {
       game.soundRequested[SOUND_ROCKET_EMPTY] = true;
   }
   ```
8. New function after `startDive`:
   ```cpp
   // Three bubbles of `size` burst from (x, y) in random directions (deep-sea buff); they pick a target once they home.
   static void spawnShieldBubbles(Game& game, float x, float y, int size, int owner) {
       for (int k = 0; k < SHIELD_SPLIT; k++) {
           float angle = (k + randomUnit(game)) * 6.2831853f / SHIELD_SPLIT;
           game.shieldBubbles.push_back({x, y, cosf(angle) * SHIELD_BUBBLE_SPEED[size], sinf(angle) * SHIELD_BUBBLE_SPEED[size], 0, size, owner, -1, -1});
       }
   }
   ```
9. `updateGame`, right after `game.flashTimer = …` and **before** the hit-stop check (so a Square press during hit-stop isn't lost):
   ```cpp
   for (size_t i = 0; i < game.players.size(); i++) {   // Square arms the next held strong attack: rocket, beam, frost, shield
       Player& p = game.players[i];
       if (!actions[i].cycle || !p.alive) continue;
       const int ORDER[4] = {0, 1, 3, 5};
       int at = 0;
       while (ORDER[at] != p.strongAttack) at++;
       for (int k = 1; k <= 4; k++) {
           int next = ORDER[(at + k) % 4];
           if (next == 0 || (p.buffs & (1 << (next - 1)))) {
               p.strongAttack = next;
               break;
           }
       }
       game.soundRequested[SOUND_PICKUP] = true;
   }
   ```
10. `updateGame`, after the mist loop (`player.inMist` loop), the freeze:
    - `game.freezeTimer = std::max(0.0f, game.freezeTimer - dt); bool frozen = game.freezeTimer > 0;`
    - `if (game.frostAge >= 0 && (game.frostAge += dt) > FROST_RING_TIME) game.frostAge = -1;`
    - Wrap the sway update (`swaySpeed` … the `SWAY_RANGE` clamp) in `if (!frozen) { … }` (keep the `dw` line outside; it is used below).
    - Alien loop: `if (alien.alive) { if (frozen) alien.flash = std::max(0.0f, alien.flash - dt); else updateAlien(game, alien, dt); }`.
    - Boss loop: the same with `boss.flash` and `updateBoss`.
    - Dive condition: add `!frozen &&` at the front.
11. `updateGame`, after the rockets loop, the shield bubbles (by index: a hit adds bubbles; copy the bubble first because
    `push_back` may move the vector):
    ```cpp
    int living = 0;
    for (const Alien& alien : game.aliens) living += alien.alive;
    for (const Boss& boss : game.bosses) living += boss.alive;
    for (size_t i = 0; i < game.shieldBubbles.size(); i++) {
        ShieldBubble b = game.shieldBubbles[i];
        b.age += dt;
        bool targetAlive = (b.alien >= 0 && b.alien < (int)game.aliens.size() && game.aliens[b.alien].alive) ||
                           (b.boss >= 0 && b.boss < (int)game.bosses.size() && game.bosses[b.boss].alive);
        if (!targetAlive && living > 0) {   // a random living enemy
            int pick = (int)(randomUnit(game) * living);
            b.alien = b.boss = -1;
            for (size_t k = 0; k < game.aliens.size() && b.alien < 0; k++) if (game.aliens[k].alive && pick-- == 0) b.alien = (int)k;
            for (size_t k = 0; k < game.bosses.size() && b.alien < 0 && b.boss < 0; k++) if (game.bosses[k].alive && pick-- == 0) b.boss = (int)k;
            targetAlive = b.alien >= 0 || b.boss >= 0;
        }
        if (b.age >= SHIELD_BUBBLE_SPREAD && targetAlive) {   // homes on the target's centre
            float tx = b.alien >= 0 ? game.aliens[b.alien].x + ALIEN_WIDTH[game.aliens[b.alien].type] / 2.0f : game.bosses[b.boss].x + BOSS_W / 2.0f;
            float ty = b.alien >= 0 ? game.aliens[b.alien].y + ALIEN_HEIGHT[game.aliens[b.alien].type] / 2.0f : game.bosses[b.boss].y + BOSS_H / 2.0f;
            float dx = tx - b.x, dy = ty - b.y, length = sqrtf(dx * dx + dy * dy) + 0.001f;
            b.vx = dx / length * SHIELD_BUBBLE_SPEED[b.size];
            b.vy = dy / length * SHIELD_BUBBLE_SPEED[b.size];
        }
        b.x += b.vx * dt;
        b.y += b.vy * dt;
        float d = (float)SHIELD_BUBBLE_D[b.size], bx = b.x - d / 2, by = b.y - d / 2;
        bool hit = false;
        if (b.age >= SHIELD_BUBBLE_SPREAD) {   // spreading bubbles can't hit yet, so a split doesn't land on the same enemy at once
            for (Alien& alien : game.aliens) {
                if (hit || !alien.alive || !overlap(bx, by, d, d, alien.x, alien.y, ALIEN_WIDTH[alien.type], ALIEN_HEIGHT[alien.type])) continue;
                damageAlien(game, alien, BUBBLE_DAMAGE, b.owner);
                hit = true;
            }
            for (Boss& boss : game.bosses) {
                if (hit || !boss.alive || !overlap(bx, by, d, d, boss.x, boss.y, BOSS_W, BOSS_H)) continue;
                damageBoss(game, boss, BUBBLE_DAMAGE, b.owner);
                hit = true;
            }
            for (Fish& f : game.fish) {
                if (hit || f.life <= 0 || !overlap(bx, by, d, d, f.x, f.y, FISH_W, FISH_H)) continue;
                f.life = 0;   // removed below
                addScore(game, game.players[b.owner], FISH_SCORE);
                hit = true;
            }
        }
        if (hit) {   // pops, and splits into three smaller ones until size 1
            spawnParticles(game, b.x, b.y, 3, paletteColor('i'), 30);
            game.soundRequested[SOUND_BUBBLE] = true;
            b.age = SHIELD_BUBBLE_LIFE;   // removed below
            if (b.size > 1) spawnShieldBubbles(game, b.x, b.y, b.size - 1, b.owner);
        }
        game.shieldBubbles[i] = b;
    }
    ```
    Add to the erase block: remove bubbles with `age >= SHIELD_BUBBLE_LIFE` or a centre more than 20 px outside the screen (a big bubble spreading
    straight down from the ship travels 22 px before it homes).
12. `updateGame`, the ship-hit block: replace `if (hit) { killPlayer(game, player); continue; }` with:
    ```cpp
    if (hit && player.shield) {   // the bubble shield takes the hit and bursts into three big bubbles
        player.shield = false;
        player.invincibleTimer = SHIELD_POP_INVINCIBLE;
        spawnShieldBubbles(game, player.x + SHIP_W / 2.0f, player.y + SHIP_H / 2.0f, 3, (int)(&player - &game.players[0]));
        spawnParticles(game, player.x + SHIP_W / 2.0f, player.y + SHIP_H / 2.0f, 10, paletteColor('i'), 50);
        game.soundRequested[SOUND_BUBBLE] = true;
    } else if (hit) {
        killPlayer(game, player);
        continue;
    }
    ```
13. Pickup collection: the box is `pickup.type == PICKUP_BUFF ? BUFF_PICKUP_SIZE : 8` square. The effect chain becomes
    rocket → `else if (pickup.type == PICKUP_BUFF)` → upgrade → upgrade points. For a buff:
    `int bit = 1 << (pickup.level - 1);` if held, `addScore(game, player, BUFF_HELD_POINTS)`; else `player.buffs |= bit;`
    and if `pickup.level` is 1, 3 or 5, `player.strongAttack = pickup.level;`. Particle colour `'y'` for buffs; sound
    `SOUND_EXTRA_LIFE` for buffs, `SOUND_PICKUP` for the others.
14. Wave-clear branch (`if (!anyAlienAlive && game.nextWaveTimer == 0 && !game.levelClear)`): first line `game.freezeTimer = 0;   // the freeze ends with the wave`.

## Step 5: drawing (`source/game.cpp` › `drawGame`)
All new drawing uses only existing palette characters; nothing the player makes uses `m`.
- **Frozen enemies:** after drawing each living alien, if `game.freezeTimer > 0` and not thawing-blink-off
  (`game.freezeTimer < FROST_THAW_BLINK && fmodf(game.freezeTimer, 0.2f) < 0.1f` hides the ice), draw `ICE_CUBE`
  centred on the alien: `(int)(alien.x + ALIEN_WIDTH[t] / 2.0f) - 9, (int)(alien.y + ALIEN_HEIGHT[t] / 2.0f) - 8`.
  After each living boss, same condition: a 1 px `'i'` rectangle outline at `boss.x - 2, boss.y - 2`, size `BOSS_W + 4` × `BOSS_H + 4`
  (four `fillRect` calls), plus `'w'` pixels at `(boss.x - 1, boss.y - 1)` and `(boss.x + BOSS_W, boss.y + BOSS_H)`.
- **Buff pickups:** in the pickup loop, for `PICKUP_BUFF`: an 11×11 frame, 1 px border in `'y'` (or `'w'` when
  `(int)(game.time * 4) % 2`), filled `'k'` inside, and `BUFF_ICONS[pickup.level - 1]` at `+3, +3`.
- **Bullets:** keep the `inMist` branch first. Then with `banana = owner.buffs & BUFF_JUNGLE` and
  `fiery = (owner.buffs & BUFF_VOLCANO) || owner.scorchTimer > 0`: banana → `drawSprite(fiery ? FIRE_BANANA : PLAYER_BANANA, (int)bullet.x, (int)bullet.y - 1)`;
  else volcano buff → `drawSprite(FIRE_BULLET, …)`; else today's scorch bolt; else `BULLET`.
- **Magic beam and ball**, after the rockets and before the ships, for each living player:
  charging → `fillCircle((int)player.beamX, (int)player.y + SHIP_WAND_Y, 1 + (int)((1 - player.beamCharge / MAGIC_CHARGE_TIME) * 2), paletteColor('c'))`
  and a `'w'` pixel at its centre; firing (`beamTimer > 0`) → `fillRect(beamX - MAGIC_BEAM_W / 2, 0, MAGIC_BEAM_W, beamBottom, paletteColor('c'))`
  then `fillRect(beamX - MAGIC_BEAM_CORE_W / 2, 0, MAGIC_BEAM_CORE_W, beamBottom, paletteColor('w'))`.
- **Ships:** the sprite becomes `*SHIP_SKINS[game.level - 1][i == 0 ? 0 : 1][frame]`. When drawn and `buffs & BUFF_SPACE`,
  draw `WAND` at `(int)player.x + 13 + <same shake offset>, (int)player.y + 2`. When `player.shield` and alive, a ring:
  for `k` in 0..47, angle `k * 6.2831853f / 48 + game.feelTime`, pixel at `centre + SHIELD_RADIUS·(cos, sin)` in `'i'`,
  skipping every pixel where `(k + (int)(game.feelTime * 8)) % 6 == 0` (shimmer). Centre = ship centre.
- **Shield bubbles:** after the ships, `drawSprite(*SHIELD_BUBBLE_SPRITES[b.size], (int)(b.x - SHIELD_BUBBLE_D[b.size] / 2.0f), (int)(b.y - SHIELD_BUBBLE_D[b.size] / 2.0f))`.
- **Frost ring:** before the explosions, if `game.frostAge >= 0`: `r = FROST_RING_SPEED * game.frostAge`; for `k` in
  `0 .. (int)(6.3f * r)`, angle `k / r`, an `'i'` pixel at radius `r` and a `'w'` pixel at radius `r - 2` around `frostX, frostY`
  (`fillRect` clips off-screen pixels).
- **HUD**, in the per-player loop: for each held buff in level order, the `k`-th held icon goes at
  `x = right ? SCREEN_W - 7 - k * 7 : 2 + k * 7`, `y = SCREEN_H - 14`; if that buff's level equals `player.strongAttack`,
  `fillRect(x, SCREEN_H - 8, 5, 1, paletteColor('y'))` under it.

## Step 6: wiring and logs (`source/main.cpp`)
- The step loop's reset `for (Actions& a : actions) a.rocket = false;` becomes `a.rocket = a.cycle = false;` (a press counts once).
- Dev aid: in the L3 test-mode toggle, when it turns **on**, also `for (Player& player : game.players) player.buffs = BUFF_ALL;`
  (buffs stay when it turns off, so dying can be tested). Log line stays `test mode on/off`.
- Logs, using the existing before/after pattern (snapshot before the step loop, compare after, next to the other per-player logs):
  ```cpp
  const char* const STRONG_NAMES[6] = {"rocket", "magic beam", "", "frost wave", "", "bubble shield"};   // by Player::strongAttack
  ```
  - buffs changed → `player %d buffs 0x%02x, strong attack %s` (also covers the death loss and the test-mode grant).
  - only the strong attack changed (Square) → `player %d strong attack %s`.
  - `beamTimer > 0` and wasn't → `player %d magic beam at x %.0f`.
  - `freezeTimer > 0` and wasn't → `frost wave: %d aliens, %d bosses frozen`.
  - shield on and wasn't → `player %d shield up`; shield was on, now off, ship alive → `player %d shield popped`.
  - number of `PICKUP_BUFF` pickups grew → `buff pickup dropped: level %d at wave %d`.
  - No per-bubble log (a chain is up to 39 hits).

## Step 7: tests (`tests/game_tests.cpp`)
- Every `Pickup` brace-init in the file gets a trailing `, 0` (the new `level` field).
- Add all new sprites (20 ship skins incl. player 2, `WAND`, `PLAYER_BANANA`, `FIRE_BANANA`, `FIRE_BULLET`, 3 shield
  bubbles, 5 buff icons) to `ALL_SPRITES`; `spritesHaveTheirSize` and `onlyDeadlyShotsUseMagenta` then cover them.
- New tests, called from `main()` after `liveFishKillsAndDies` group (anywhere at the end is fine):
  - `shipSkinsKeepTheBody`: every `SHIP_SKINS`-listed sprite (list them in the test) is 16×14, and each player-2 skin
    contains no `c`.
  - `bigBossesDropTheLevelBuff`: `startBossGame(game, 15, 3)`, boss hp 1, a bullet kills it → exactly one pickup with
    `type == PICKUP_BUFF && level == 3`. `startBossGame(game, 10, 3)`, kill both twins → no `PICKUP_BUFF`.
  - `buffPickupGivesTheBuffOnce`: level-2 safe game; a `{p.x + 4, p.y, PICKUP_BUFF, 2}` pickup → `buffs == BUFF_JUNGLE`,
    `strongAttack == 0`, `SOUND_EXTRA_LIFE`; the same again → buffs unchanged, score +5,000; a level-1 buff →
    `buffs == BUFF_JUNGLE | BUFF_SPACE`, `strongAttack == 1`.
  - `dyingLosesEveryBuff`: safe game, then `invincibleTimer = 0`, `buffs = BUFF_ALL`, `strongAttack = 3`; an enemy shot
    `{p.x + 7, p.y + 5, 0, -1, 0}` → dead, `buffs == 0`, `strongAttack == 0`.
  - `squareCyclesHeldStrongAttacks`: `buffs = BUFF_SPACE | BUFF_JUNGLE | BUFF_SEA`; one-step presses of `cycle` give
    1, 5, 0, 1. With `buffs = 0` a press keeps 0.
  - `magicBeamHitsItsColumnOnce`: level-1 safe game, aliens cleared, `swayDirection = 0`, `diveTimer = 1e9f`; parked
    aliens at (158, 20), (158, 40), (158, 60) and (100, 40), each `hp = 40`; ship untouched (x 152, star at 166).
    `buffs = BUFF_SPACE`, `strongAttack = 1`; one R2 step → `rockets == 2`, `beamCharge > 0`; `run 0.25 s` → the three
    column aliens have hp 10, the fourth 40, `beamTimer > 0`; `run 0.5 s` → still 10 (hit once).
  - `frostWaveFreezesHealsAndShatters`: `startBossGame(game, 15, 3)`; boss `hp = 100`; push 2 enemy shots and a
    snowball; `buffs = BUFF_GLACIER`, `strongAttack = 3`; one R2 step → `freezeTimer > 2.9f`, boss hp 185
    (100 + 340 / 4), shots and thrown empty, rockets 2, `SOUND_FREEZE`. Then `boss.attackTimer = 0.05f`, remember
    `boss.x`; `run 1 s` → x unchanged, no new shots or thrown. A bullet on the boss → hp 184. `run 2.1 s` → `freezeTimer == 0`;
    `run 0.5 s` → x changed.
  - `volcanoBuffFiresAQuarterFaster`: hold fire one step → `fireTimer ≈ 0.12 - DT`; new game with `buffs = BUFF_VOLCANO`
    → `fireTimer ≈ 0.096 - DT` (tolerance 0.001).
  - `bubbleShieldTakesAHitAndChains`: level-5 safe game, aliens cleared, one parked alien at (100, 30) with `hp = 200`,
    `swayDirection = 0`, `diveTimer = 1e9f`; `buffs = BUFF_SEA`, `strongAttack = 5`; R2 → `shield`, rockets 2;
    `run 0.6 s`, R2 again → rockets still 2. Then `invincibleTimer = 0` and an enemy shot on the ship → alive, `!shield`,
    `invincibleTimer > 0.9f`, `shieldBubbles.size() == 3`; then `invincibleTimer = 1e9f`, `run 4 s` → alien hp
    `200 - 78` and `shieldBubbles` empty. (The level-5 current moves the ship; that doesn't matter here.)
  - `classicKeepsBuffsIntoTheNextLevel`: `startBossGame(game, 20, 1)`, `buffs = BUFF_SPACE`, kill both bosses as
    `eachLevelClearContinuesToTheNext` does, `run 3.4 s` → level 2 and `buffs & BUFF_SPACE`.
- Run `make test`: all pass. If `bubbleShieldTakesAHitAndChains` lands a few hits short in 4 s, print the hp and fix the
  cause (target picking or the spread rule), don't loosen the check.

## Step 8: docs (game-docs conventions; copy the shape of `docs/features/volcano/*.md`)
- New `docs/features/level-buffs/design.md`, `implementation.md`, `testing.md`. Design: "What the player gets" and
  "Why it stays fair" above as numbered rules, the numbers in a table, edge cases (buff pickup during the level-clear
  banner still falls and can be caught; a picker run keeps only that level's buff; co-op: per player, frost freezes for
  both; Square unconfirmed). Implementation: code map like the volcano's. Testing: the Mac tests above as checked
  criteria, the PS3 checks of step 9 unchecked.
- `docs/GDD.md`: version 0.25 and today's date; feature index row
  `| Level buffs | implemented | MVP | Big bosses drop a level buff (wand beam, bananas, frost wave, fire bullets, bubble shield); Square picks the strong attack; death loses all | [design](features/level-buffs/design.md) |`;
  Ship controls summary unchanged; Pixel art row: add "a ship skin per level". Summary paragraph: add one sentence
  "Big bosses drop level buffs that stack until you die." Controls line: "R2 strong attack (rocket or a buff's), Square
  picks it". Decisions:
  - `D-030 | 2026-11-18 | Level buffs: big bosses (waves 15, 20) drop a falling pickup; buffs stack, all lost on death; Square cycles the strong attack; beam 30 per enemy, frost 3 s (+25 % HP), bubbles 2 per hit, volcano fire rate only | wave 20 only; given automatically; all strong attacks at once or newest wins | user's PDF and choices; scarce rockets and the death loss keep pillar 2 | level buffs, rocket, bosses, ship controls`
  - `D-031 | 2026-11-18 | A ship skin per level on the same 16×14 body and hitbox; player 2 keeps a gold copy | recolour only | user's PDF; same hitbox keeps dodging fair | pixel art, co-op, every level`
  - Change log `0.25 (2026-11-18): level buffs and a ship skin per level (D-030, D-031).`
- "Ship skin" rows in `jungle`, `glacier`, `volcano`, `deep-sea` `design.md`: the new skin in a few words (deep sea keeps
  "; a moving ship trails bubbles"). `level-1/design.md` row: "The current ship; later levels have their own skins (D-031)".
- `rocket/design.md`: one rule: "R2 fires the armed strong attack: the rocket, or a level buff's ([level buffs](../features/level-buffs/design.md))".
- Don't touch `docs/level-guide/` (its PDF is made separately).
- Run `python3 .claude/skills/game-docs/scripts/check_docs.py docs`: clean.

## Step 9: PS3 check
Build outside the lock with `make ps3`, then everything that touches the PS3 in ONE locked command, with `make log`
running in the background inside it (log to `shots/buffs-log.txt`, not `build/`). Menu: main menu `down|1|cross` opens
the level picker; X starts the picked level; check the screenshot to know which level is picked before pressing X.
1. Level picker → each of the five levels in turn, `make shot` after the `WAVE 1` banner: the ship skin matches the level.
2. In one run: `l3` (test mode on, all buffs) → log `player 1 buffs 0x1f, strong attack …`; hold R1 and `make shot`:
   wand on the ship, fire bananas, five HUD icons.
3. `square` presses → log `strong attack magic beam`, `frost wave`, `bubble shield`, `rocket` in turn; with each armed,
   `r2` and `make shot`: the beam column, the frost ring and frozen enemies (log `frost wave: …`), the bubble ring (log `shield up`).
4. `l3` (test mode off), wait for an enemy shot: log `shield popped`, then on a later hit `player 1 died …` and
   `player 1 buffs 0x00, strong attack rocket`.
5. Drop check: level picker SPACE, `select` (autopilot) only, wait for wave 15 (about 7 min): log `buff pickup dropped: level 1 at wave 15`.
6. Watch the log's fps lines during a full bubble chain and a frost wave: 60 fps holds.
If Square never shows in the log, read the SDL port's button numbers again (`CLAUDE.md`) and report it; don't remap blindly.
The user plays a real run afterwards and reports whether the buffs feel fair; record that in `testing.md`.

## Done when
- `make test` passes, `make ps3` builds, `check_docs.py` is clean.
- On the PS3: each level shows its ship skin, and a big boss's buff changes the ship's attacks as described, until the ship dies.
