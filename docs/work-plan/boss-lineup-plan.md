# Plan: a different boss line-up on every boss wave (classic, twin classics, mage, twin mages)

For the implementer: follow the steps in order. Every design choice below is final, so don't redesign anything.
Read only the files each step names, and don't re-read a file after editing it. Code style: follow `CLAUDE.md` and
the existing code (plain C++14, no new libraries, short one-line comments, no helpers for one-off logic).
Don't commit: the user commits. When done, list the changed files and suggest a one-line commit message.

## What the player gets
| Wave | Bosses | HP each | Sway | Attacks (each boss, own timer) |
|---|---|---|---|---|
| 5 | 1 classic (today's sprite) | 100 | 40 px/s, full width | burst, fan, alternating, every 2.5 s (as today) |
| 10 | 2 classics | 100 | 50 px/s, each in its own half | burst, fan, alternating, every 2.2 s **each** |
| 15 | 1 mage (new sprite: pointed hat, wand) | 200 | 60 px/s, full width | random burst / fan / power ball, random gaps 0.6–1.4 × 1.9 s |
| 20 | 2 mages (level boss) | 200 | 70 px/s, each in its own half | random, gaps 0.6–1.4 × 1.6 s **each** |

- **Sway and attack-interval formulas are unchanged** (`40 + 10·(n−1)` px/s capped at 80, `max(2.5 − 0.3·(n−1), 1.2)` s, n = wave / 5).
  Only HP, count and type change; the old "100 + 50 per appearance" HP rule is gone.
- **Twins:** the left twin sways in x 0…128, the right one in x 160…288 (each half minus the 32 px sprite), starting in
  the middle of its half and moving apart (left goes left, right goes right). Both attack at the full rate; the second
  twin's first attack comes at 1.5 × the interval so they don't fire on the same frame. The wave ends when both are dead.
- **Each boss kill** gives 5,000 · n points, the big explosion, an upgrade pickup and a rocket pickup (so twin waves drop two of each).
  The wave-clear bonus comes once, when the last boss dies. Wave 20's last mage still ends level 1.
- **Health bar:** one bar (same place and look) showing the summed HP of all of the wave's bosses over their summed max HP.
- **Mage attacks:** each time its timer runs out it rolls: 35 % aimed burst, 35 % fan, 30 % power ball. If it rolls the
  power ball while its own ball or beam is still active, it does a fan instead. The next gap is the interval × a random
  0.6…1.4. Burst and fan are exactly the classic boss's.
- **Power ball:** appears at the wand tip (sprite pixel 28, 1) where the mage is at that moment and stays there while the
  mage sways on. It charges for 3 s: a magenta circle growing from radius 1 to 5 with a white centre, and a dotted magenta
  guide line straight down from it to the bottom of the screen. Then it fires a beam: a 12 px wide magenta column with a
  4 px white core, from the top of the screen to the bottom, centred on the ball's x, lasting 0.8 s, with a small shake
  (trauma 0.3) and `SOUND_EXPLOSION`. A ship whose hitbox touches the beam dies (respawn invincibility protects it, as
  with shots). The ball disappears when the beam starts.
- A charging ball still fires if every ship has died meanwhile; the mage casts no new attack without a living ship (as today).
  A mage that dies takes its ball and beam with it.

## Step 1: data (`source/game.h`, `source/sprites.h`)
`game.h`:
- Above `struct Boss`, add `enum BossType { BOSS_CLASSIC, BOSS_MAGE };`
- Replace `struct Boss` with (keep the comment line above it):
```cpp
struct Boss {
    BossType type;
    float x, y, vx;          // top-left of the sprite; vx is the signed sway speed
    float minX, maxX;        // sway range: the full width alone, its own half as a twin
    float attackTimer;       // s until the next attack
    float burstTimer;        // s until the next shot of a burst
    int hp, maxHp;
    int burstShotsLeft;
    int nextAttack;          // classic: 0 aimed burst, 1 fan, alternating
    float flash;             // s of white hit flash left
    float ballX, ballY;      // mage: centre of the power ball, fixed where it was cast
    float ballTimer;         // s until the ball fires its beam; 0 when no ball
    float beamTimer;         // s of beam left; 0 when no beam
    bool alive;
};
```
- In `struct Game`, replace `Boss boss;` with `std::vector<Boss> bosses;   // this wave's bosses; empty on formation waves`.
- Replace the `bossHealth` declaration and add two:
```cpp
int bossHealth(int wave);           // HP of each boss: 100 classic, 200 mage
int bossCount(int wave);            // 2 on waves 10 and 20, else 1
BossType bossType(int wave);        // mage from wave 15
```
`sprites.h`: after `BOSS_B`, add the mage (wand tip glows on frame A, pupils look left on frame B). Don't rerun `make_xmb_art.py`.
```cpp
const Sprite MAGE_A = {32, 20,
    "...............U............o..."
    "..............UuU..........owo.."
    ".............UuuuU..........o..."
    "............UuucuuU.........b..."
    "...........UuucccuuU........b..."
    "..........UuuuucuuuuU.......b..."
    ".........UuuuuuuuuuuuU......b..."
    ".....UUUUUUUUUUUUUUUUUUUUU..b..."
    ".........GgggggggggggG......b..."
    ".........GgwwRgggwwRgG......b..."
    ".........GgwwRgggwwRgG......b..."
    ".........GgggggggggggG......b..."
    ".........GgggrrrrrgggG......b..."
    ".........GGgggggggggGG......b..."
    "........UuuuuuuuuuuuuuUUUUUgb..."
    ".......UuuuuuuuuuuuuuuuU....b..."
    "......UuuuuuuuuuuuuuuuuuU...b..."
    ".....UuuuuuuuuuuuuuuuuuuuU..b..."
    "....UuuuuuuuuuuuuuuuuuuuuuU....."
    "...UcccccccccccccccccccccccU...."};
const Sprite MAGE_B = {32, 20,
    "...............U................"
    "..............UuU...........w..."
    ".............UuuuU.............."
    "............UuucuuU.........b..."
    "...........UuucccuuU........b..."
    "..........UuuuucuuuuU.......b..."
    ".........UuuuuuuuuuuuU......b..."
    ".....UUUUUUUUUUUUUUUUUUUUU..b..."
    ".........GgggggggggggG......b..."
    ".........GgRwwgggRwwgG......b..."
    ".........GgRwwgggRwwgG......b..."
    ".........GgggggggggggG......b..."
    ".........GgggrrrrrgggG......b..."
    ".........GGgggggggggGG......b..."
    "........UuuuuuuuuuuuuuUUUUUgb..."
    ".......UuuuuuuuuuuuuuuuU....b..."
    "......UuuuuuuuuuuuuuuuuuU...b..."
    ".....UuuuuuuuuuuuuuuuuuuuU..b..."
    "....UuuuuuuuuuuuuuuuuuuuuuU....."
    "...UcccccccccccccccccccccccU...."};
```

## Step 2: rules (`source/game.cpp`)
Constants, in the `// Bosses` block: delete `BOSS_HP_PER_APPEARANCE` (keep `BOSS_BASE_HP = 100`) and add:
```cpp
const int MAGE_WAVE = 15, MAGE_HP = 200;   // waves 15 and 20 are mages
const float TWIN_FIRST_ATTACK = 1.5f;      // x the interval: the second twin's first attack
const float MAGE_BURST_CHANCE = 0.35f, MAGE_FAN_CHANCE = 0.35f;   // the rest is a power ball
const float MAGE_GAP_MIN = 0.6f, MAGE_GAP_MAX = 1.4f;             // x the attack interval
const int WAND_TIP_X = 28, WAND_TIP_Y = 1;    // px in the mage sprite
const float BALL_CHARGE_TIME = 3, BEAM_TIME = 0.8f;   // s
const int BALL_MAX_RADIUS = 5, BEAM_W = 12, BEAM_CORE_W = 4;   // px
const float TRAUMA_BEAM = 0.3f;
```
Formulas (replace `bossHealth`, add the two others next to it):
- `bossHealth`: `return wave >= MAGE_WAVE ? MAGE_HP : BOSS_BASE_HP;`
- `bossCount`: `return wave % 10 == 0 ? 2 : 1;`
- `bossType`: `return wave >= MAGE_WAVE ? BOSS_MAGE : BOSS_CLASSIC;`

`startWave`: replace `game.boss = Boss();` and the `if (isBossWave(wave))` block with:
```cpp
    game.bosses.clear();
    if (isBossWave(wave)) {
        int count = bossCount(wave);
        float speed = std::min(BOSS_BASE_SPEED + BOSS_SPEED_PER_APPEARANCE * (wave / 5 - 1), BOSS_MAX_SPEED);
        for (int i = 0; i < count; i++) {
            Boss boss = {};
            boss.type = bossType(wave);
            boss.minX = count == 1 ? 0 : i * SCREEN_W / 2.0f;
            boss.maxX = count == 1 ? SCREEN_W - BOSS_W : boss.minX + SCREEN_W / 2.0f - BOSS_W;
            boss.x = (boss.minX + boss.maxX) / 2;
            boss.y = -BOSS_H;
            boss.vx = count == 2 && i == 0 ? -speed : speed;   // twins move apart first
            boss.hp = boss.maxHp = bossHealth(wave);
            boss.attackTimer = bossAttackInterval(wave) * (i == 0 ? 1 : TWIN_FIRST_ATTACK);
            boss.alive = true;
            game.bosses.push_back(boss);
        }
        rows = 0;
    }
```
`damageBoss`: new signature `static void damageBoss(Game& game, Boss& boss, int damage, int owner)`; drop the
`Boss& boss = game.boss;` line; next to `boss.burstShotsLeft = 0;` add `boss.ballTimer = boss.beamTimer = 0;`. Everything else stays
(reward `BOSS_REWARD * (game.wave / 5)` per boss, both pickups per boss).

`explode`: wrap the boss part in `for (Boss& boss : game.bosses) { ... }`, using `boss` instead of `game.boss` and
`damageBoss(game, boss, EXPLOSION_DAMAGE, owner)`.

`updateBoss`: new signature `static void updateBoss(Game& game, Boss& boss, float dt)`; drop the `Boss& boss = game.boss;` line.
Update its one-line comment to `// Flies in, sways in its range and attacks; holds new attacks with no living ship.`
- Sway edge test becomes the boss's range:
```cpp
    if (boss.x < boss.minX || boss.x > boss.maxX) {
        boss.x = std::max(boss.minX, std::min(boss.x, boss.maxX));
        boss.vx = -boss.vx;
    }
```
- Right after the sway (before the target search), the ball and beam tick, so a charging ball fires even with no living ship:
```cpp
    if (boss.ballTimer > 0) {
        boss.ballTimer -= dt;
        if (boss.ballTimer <= 0) {
            boss.ballTimer = 0;
            boss.beamTimer = BEAM_TIME;
            feedback(game, TRAUMA_BEAM, 0, 0);
            game.soundRequested[SOUND_EXPLOSION] = true;
        }
    } else {
        boss.beamTimer = std::max(0.0f, boss.beamTimer - dt);
    }
```
- The attack choice replaces the `if (boss.attackTimer <= 0) { ... }` block (the burst-shot block after it is unchanged):
```cpp
    if (boss.attackTimer <= 0) {
        int attack = boss.nextAttack;   // 0 burst, 1 fan, 2 power ball
        boss.attackTimer = bossAttackInterval(game.wave);
        if (boss.type == BOSS_CLASSIC) {
            boss.nextAttack = 1 - boss.nextAttack;
        } else {   // mage: random order and random gaps
            float roll = randomUnit(game);
            attack = roll < MAGE_BURST_CHANCE ? 0 : roll < MAGE_BURST_CHANCE + MAGE_FAN_CHANCE ? 1 : 2;
            if (attack == 2 && (boss.ballTimer > 0 || boss.beamTimer > 0)) attack = 1;
            boss.attackTimer *= MAGE_GAP_MIN + (MAGE_GAP_MAX - MAGE_GAP_MIN) * randomUnit(game);
        }
        if (attack == 0) {
            <today's burst start: burstShotsLeft = BOSS_BURST_SHOTS, burstTimer = 0>
        } else if (attack == 1) {
            <today's fan loop, unchanged>
        } else {
            boss.ballX = boss.x + WAND_TIP_X;
            boss.ballY = boss.y + WAND_TIP_Y;
            boss.ballTimer = BALL_CHARGE_TIME;
        }
    }
```

`updateGame`, every `game.boss` use (keep the logic, loop over the vector):
- Update: `for (Boss& boss : game.bosses) if (boss.alive) updateBoss(game, boss, dt);`
- Bullet loop, replace the boss `if` with:
```cpp
        for (Boss& boss : game.bosses) {
            if (bullet.y <= -50 || !boss.alive || boss.y + BOSS_H < 0 || !overlap(bullet.x, bullet.y, 2, 4, boss.x, boss.y, BOSS_W, BOSS_H)) continue;
            damageBoss(game, boss, 1, bullet.owner);
            bullet.y = -100;   // spent; removed below
        }
```
- Rocket loop: `for (const Boss& boss : game.bosses) if (boss.alive && boss.y + BOSS_H >= 0 && overlap(...boss...)) hit = true;`
- Player hit block, replace the boss-body line with:
```cpp
            for (const Boss& boss : game.bosses) {
                if (boss.alive && overlap(boss.x, boss.y, BOSS_W, BOSS_H, hx, hy, HITBOX, HITBOX)) hit = true;
                if (boss.alive && boss.beamTimer > 0 && overlap(boss.ballX - BEAM_W / 2.0f, 0, BEAM_W, SCREEN_H, hx, hy, HITBOX, HITBOX)) hit = true;
            }
```
- `anyAlienAlive`: `std::any_of(game.bosses.begin(), game.bosses.end(), [](const Boss& b) { return b.alive; }) || std::any_of(...aliens as today...)`.

## Step 3: drawing (`source/game.cpp` › `drawGame`)
- Boss sprites: replace the `if (game.boss.alive) { ... }` block with a loop over `game.bosses` (skip dead ones), sprite
  `boss.type == BOSS_MAGE ? (frame ? MAGE_B : MAGE_A) : (frame ? BOSS_B : BOSS_A)`, flash as today.
- Ball and beam: right after the enemy-shot line, before the explosions:
```cpp
    for (const Boss& boss : game.bosses) {
        if (!boss.alive) continue;
        int bx = (int)boss.ballX, by = (int)boss.ballY;
        if (boss.ballTimer > 0) {
            for (int y = by + BALL_MAX_RADIUS + 2; y < SCREEN_H; y += 4) fillRect(bx, y, 1, 2, paletteColor('m'));   // guide line
            int radius = 1 + (int)((1 - boss.ballTimer / BALL_CHARGE_TIME) * (BALL_MAX_RADIUS - 1));
            fillCircle(bx, by, radius, paletteColor('m'));
            if (radius >= 3) fillCircle(bx, by, radius - 2, paletteColor('w'));
        }
        if (boss.beamTimer > 0) {
            fillRect(bx - BEAM_W / 2, 0, BEAM_W, SCREEN_H, paletteColor('m'));
            fillRect(bx - BEAM_CORE_W / 2, 0, BEAM_CORE_W, SCREEN_H, paletteColor('w'));
        }
    }
```
- Health bar: replace the `if (game.boss.alive) { ... }` HUD block with a sum over all bosses of the wave:
```cpp
    int bossHp = 0, bossMaxHp = 0;
    bool anyBossAlive = false;
    for (const Boss& boss : game.bosses) {
        bossHp += std::max(boss.hp, 0);
        bossMaxHp += boss.maxHp;
        anyBossAlive |= boss.alive;
    }
    if (anyBossAlive) { <the two fillRects as today, the fill width BOSS_HEALTH_BAR_W * bossHp / bossMaxHp> }
```

## Step 4: logs (`source/main.cpp`)
Only the boss bookkeeping around the fixed-step loop changes.
- Replace `bool bossWasAlive = game.boss.alive;` with:
```cpp
        int bossesBefore = 0;
        bool beamBefore[2] = {};
        for (size_t i = 0; i < game.bosses.size(); i++) {
            bossesBefore += game.bosses[i].alive;
            if (i < 2) beamBefore[i] = game.bosses[i].beamTimer > 0;
        }
```
- Replace the two boss log blocks after the loop with:
```cpp
        int bossesAlive = 0;
        for (const Boss& boss : game.bosses) bossesAlive += boss.alive;
        if (bossesAlive > 0 && bossesBefore == 0) {
            bossStart = game.time;
            debugNetPrintf(INFO, "boss wave %d: %d %s, hp %d\n", game.wave, bossesAlive,
                           game.bosses[0].type == BOSS_MAGE ? "mage" : "classic", game.bosses[0].hp);
        }
        if (bossesAlive < bossesBefore)
            debugNetPrintf(INFO, "boss down at wave %d after %.0f s, %d left\n", game.wave, game.time - bossStart, bossesAlive);
        for (size_t i = 0; i < game.bosses.size() && i < 2; i++)
            if (game.bosses[i].beamTimer > 0 && !beamBefore[i]) debugNetPrintf(INFO, "mage beam at x %.0f\n", game.bosses[i].ballX);
```

## Step 5: tests (`tests/game_tests.cpp`)
The `Makefile` `test` target needs no change. In the existing tests replace every `game.boss` with `game.bosses[0]`, plus:
- `ALL_SPRITES`: add `&MAGE_A, &MAGE_B` after `&BOSS_B`.
- `startBossGame`: last line becomes `for (Boss& boss : game.bosses) boss.attackTimer = 100;`.
- `everyFifthWaveIsABoss`: first check uses `game.bosses.size() == 1 && game.bosses[0].alive`; after `game.bosses[0].alive = false;`
  the wave-6 check uses `game.bosses.empty()` instead of `!game.boss.alive`.
- `bossHealthGrowsEachAppearance` → rename to `bossWavesFollowTheTable` (also in `main()`), body:
```cpp
    CHECK(bossHealth(5) == 100 && bossHealth(10) == 100 && bossHealth(15) == 200 && bossHealth(20) == 200);
    CHECK(bossCount(5) == 1 && bossCount(10) == 2 && bossCount(15) == 1 && bossCount(20) == 2);
    CHECK(bossType(5) == BOSS_CLASSIC && bossType(10) == BOSS_CLASSIC && bossType(15) == BOSS_MAGE && bossType(20) == BOSS_MAGE);
    <keep the two isBossWave CHECK lines>
```
- `killingTheBossScoresAndDropsBothPickups`: use wave 5 instead of 10; expected score `before + 5000 + 5 * 1000` (comment
  `// 5,000 x 1 and the wave 5 clear bonus`).
- `killingTheLevelBossClearsTheLevel`: the hp check becomes
  `CHECK(game.bosses.size() == 2 && game.bosses[0].type == BOSS_MAGE && game.bosses[0].hp == 200 && game.bosses[1].hp == 200);`
  followed by `game.bosses[1].alive = false;   // its twin is already down`. The score expectation stays.
- `deathDuringTheClearOnlyLosesOnTheLastLife`: after `startBossGame(game, 20);` add `game.bosses[1].alive = false;`.

New tests, added after `bossWaitsForALivingShip` and called in that order from `main()` right after it.
Never keep a `Boss&` past a wave change (`startWave` clears the vector): every new test stays inside its boss wave.
- `twinBossesSwayInTheirOwnHalves`: `startBossGame(game, 10)`; `left = game.bosses[0]`, `right = game.bosses[1]` (const refs);
  CHECK `size() == 2`, `left.minX == 0 && left.maxX == 128 && right.minX == 160 && right.maxX == 288`, `fabsf(fabsf(left.vx) - 50) < 0.01f`.
  Then 600 steps of `updateGame(game, &NO_INPUT, DT)`, and-ing into one bool that `left.x` is in 0…128 and `right.x` in 160…288; CHECK it.
- `twinBossesBothMustDie`: `startBossGame(game, 10)`, `before` = score. Set `bosses[0].hp = 1`, push a bullet
  `{bosses[0].x + 10, bosses[0].y + 5, 0, 0, 0}`, one `updateGame`: CHECK `!bosses[0].alive && bosses[1].alive && score == before + 10000`.
  `run(game, NO_INPUT, 0.3f)`: CHECK `game.nextWaveTimer == 0 && game.wave == 10`. Same kill on `bosses[1]` (bullet at its
  current position): CHECK `score == before + 10000 + 10000 + 10 * 1000`, and counting `game.pickups` 2 rockets and 2 upgrades.
  `run(game, NO_INPUT, 0.1f)`: CHECK `game.nextWaveTimer > 0`.
- `mageBallBeamsItsColumnAfterThreeSeconds`: `for (int offset : {0, 20})`: `startBossGame(game, 15)`, `Boss& mage = game.bosses[0]`,
  `Player& p = game.players[0]`; CHECK `game.bosses.size() == 1 && mage.type == BOSS_MAGE && mage.hp == 200`. Set `p.invincibleTimer = 0`,
  `mage.ballX = p.x + 8 + offset`, `mage.ballY = 21`, `mage.ballTimer = 3`. `run(2.9f)`: CHECK `p.alive && mage.beamTimer == 0`.
  `run(0.2f)`: CHECK `mage.beamTimer > 0 && p.alive == (offset == 20)`. `run(1)`: CHECK `mage.beamTimer == 0`.
- `deadMageCastsNoBeam`: `startBossGame(game, 15)`, `p.invincibleTimer = 0`, `mage.ballX = p.x + 8`, `ballY = 21`, `ballTimer = 1`,
  `mage.hp = 1`, bullet on the mage as above. `run(1.5f)` (the next wave starts after 2 s): CHECK `game.wave == 15 && !mage.alive && mage.beamTimer == 0 && p.alive`.
- `mageAttacksInRandomOrderAndGaps`: `startBossGame(game, 15)` (ship invincible), `mage.attackTimer = 0.01f`. Run 7200 steps of
  `updateGame`; after each step: if `mage.attackTimer` rose above its value before the step, an attack happened (record the gap in
  `game.time` since the previous attack, keep min and max); count casts as rising edges of `mage.ballTimer > 0`; note if
  `game.enemyShots` was ever non-empty. CHECK `casts >= 5`, shots seen, `minGap >= 0.6f * 1.9f - 0.02f`, `maxGap <= 1.4f * 1.9f + 0.02f`,
  `minGap < 1.5f`, `maxGap > 2.3f`.
- `beamDrawsAMagentaColumn`: `startBossGame(game, 15)`, `mage.ballX = 100`, `mage.beamTimer = 0.5f`; `picture = pictureOf(game)`;
  CHECK `picture[170 * SCREEN_W + 95] == paletteColor('m')` and `picture[170 * SCREEN_W + 100] == paletteColor('w')`.
  (`pictureOf` is defined below the boss tests: place this test after `pictureOf`, and call it from `main()` after `mageAttacksInRandomOrderAndGaps`.)

Run `make test`: all pass.

## Step 6: docs
- `docs/features/bosses/design.md`: rewrite the intro, Rules, Numbers and Edge cases to "What the player gets" above
  (the per-wave table as the first Numbers rows; the mage numbers: 35/35/30 %, gaps 0.6–1.4 ×, ball 3 s at wand tip (28, 1),
  radius 1→5, beam 12 px (core 4) × full height for 0.8 s, trauma 0.3). Edge cases to add: ball fires with no living ship;
  a dead mage's ball and beam vanish; a second ball is never cast while one is active (fan instead); each twin drops both pickups.
  Keep the existing edge cases that still hold. Status stays `implemented`; History: `- 2026-11-17: boss line-up per wave: classic, twin classics, mage, twin mages (D-019).`
- `docs/features/bosses/implementation.md`: code map rows for `Game::bosses`, `BossType`, `bossCount`, `bossType`, `MAGE_A`/`MAGE_B`,
  the ball/beam in `updateBoss`/`drawGame`, and main.cpp's new log lines; History line as above.
- `docs/features/bosses/testing.md`: replace the HP criterion's test name with `bossWavesFollowTheTable` and its text with the
  new HP table; add one ticked criterion per new test (with its `tests/game_tests.cpp › name`), and unchecked PS3 criteria:
  "Waves 10 and 20 show two bosses, each in its half" · "The mage's ball charges at the wand tip and the beam reads clearly; 60 fps holds" ·
  "Twin mages at wave 20 are beatable but threatening (player report)". The old "hp 150/200/250" PS3 evidence stays in History, not in criteria.
- `docs/features/level-1/design.md`: intro "same bosses" → "the bosses differ per boss wave ([Bosses](../features/bosses/design.md))";
  bundle row Bosses → "One classic at wave 5 (100 HP), two at 10 (100 HP each), a mage at 15 (200 HP), two mages at 20 (200 HP each)";
  wave-table rows:
  - `| 5 | Mid boss: 1 classic, 100 HP, sway 40 px/s, attack every 2.5 s | boss | 0.75 | climax |`
  - `| 10 | Mid boss: 2 classics, 100 HP each, each in its half, sway 50 px/s, each attacks every 2.2 s | boss | 0.85 | climax |`
  - `| 15 | Mid boss: 1 mage, 200 HP, sway 60 px/s, random attack every 1.1–2.7 s, power-ball beam | boss | 0.9 | climax |`
  - `| 20 | Level boss: 2 mages, 200 HP each, sway 70 px/s, each attacks every 1.0–2.2 s, reward 20,000 each | boss | 1.0 | climax |`

  Numbers row Level boss → `| Level boss | 2 mages, 200 HP and 20,000 points each | | clearly the finale |`. History line as above.
  Don't touch `docs/features/level-1/testing.md` (the user has uncommitted edits there).
- Run `grep -rn "250\|100 + 50\|bossHealth\|150 HP" docs/features docs/GDD.md` and fix any other line that describes current
  boss HP (not History lines or dated test evidence).
- `docs/GDD.md`: Version `0.20`; feature index Bosses summary → "A boss on every 5th wave: classic, twin classics, a mage with a
  power-ball beam, twin mages"; decision row after D-018:
  `| D-019 | 2026-11-17 | Boss line-up per boss wave: 5 one classic (100 HP), 10 two classics (100 HP each), 15 one mage (200 HP), 20 two mages (200 HP each); twins sway in their own half at the full attack rate each; the mage attacks in random order and gaps, and its power ball charges 3 s at the wand tip, then beams down that column | one boss with growing HP (100 + 50 per appearance); twins at half rate; ball above the nearest ship | user's choice: each boss wave should feel different | [bosses](features/bosses/design.md), [level 1](features/level-1/design.md) |`;
  change log `**0.20 (2026-11-17):** boss line-up: classic, twin classics, mage, twin mages (D-019).`
- Don't edit older decision rows, `docs/work-plan/make_work_plan.py` or the PDF.
- Run `python3 .claude/skills/game-docs/scripts/check_docs.py docs` and fix what it reports.

## Step 7: PS3 check (Claude drives it remotely; the user only re-enables HEN if the PS3 doesn't answer)
The autopilot run overwrites the saved high score on the PS3. The autopilot keeps the ship invincible, so this checks
spawns, beams, looks and fps, not whether the beam kills (the Mac tests cover that).
1. `make ps3` (outside the lock) builds without warnings in the changed code.
2. If `lsof -nP -iUDP:18195` shows a stale python, kill it. Then run everything that touches the PS3 as ONE locked command
   (`CLAUDE.md`), in the background, with the log in your scratchpad (`$LOG` below), and wait for it with Monitor:
```bash
scripts/ps3-lock.sh bash -c '
  make log > "$LOG" 2>&1 & LOGPID=$!
  make install launch && sleep 15 && make pad BUTTONS="cross|2|select" &&
  until grep -q "boss wave 10" "$LOG"; do sleep 2; done && sleep 2 && make shot && cp shots/shot.png shots/wave10.png &&
  until grep -q "boss wave 15" "$LOG"; do sleep 2; done &&
  for i in 1 2 3 4 5 6; do sleep 1; make shot && cp shots/shot.png shots/wave15-$i.png; done &&
  until grep -q "boss wave 20" "$LOG"; do sleep 2; done && sleep 2 && make shot && cp shots/shot.png shots/wave20.png &&
  until grep -q "level clear at\|game over at" "$LOG"; do sleep 5; done
  kill $LOGPID'
```
   (`cross` picks CLASSIC at boot, `select` turns the autopilot on; the run takes about 8 min.)
3. The log must show, in order: `boss wave 5: 1 classic, hp 100`, `boss down at wave 5 ... 0 left`;
   `boss wave 10: 2 classic, hp 100`, `... 1 left`, `... 0 left`; `boss wave 15: 1 mage, hp 200`; `boss wave 20: 2 mage, hp 200`;
   at least one `mage beam at x N` during waves 15–20; `level 1 clear at`; no `wave 21 start`; the fps lines stay ≈ 60.
4. Read the shots: `wave10.png` shows two classic bosses, one in each half; at least one `wave15-*.png` shows the mage with a
   charging ball and its guide line, or the beam; `wave20.png` shows two mages and the health bar.
5. If a line is missing: no `mage beam` → check the ball tick sits before the no-target `return` in `updateBoss`; twins overlapping
   → check `minX`/`maxX` in `startWave`. Fix, `make test`, `make ps3`, repeat from 2.
6. Tick the testing.md criteria the log and shots prove ("PS3 log 2026-11-17" / "PS3 screenshot 2026-11-17"); leave the
   player-report criterion unchecked and ask the user to play waves 15 and 20 without the autopilot.

## Done when
- `make test` passes, `make ps3` builds, `check_docs.py` is clean.
- On the PS3: an autopilot run meets one classic at wave 5, two at 10, a beam-casting mage at 15 and two mages at 20, then clears level 1 at 60 fps.
