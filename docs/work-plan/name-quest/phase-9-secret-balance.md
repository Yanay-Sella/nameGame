# Phase 9: the secret world (The Unwritten), The First Name, the balance pass, docs, release

Read first: [`../name-quest-plan.md`](../name-quest-plan.md), the **world authoring recipe** in
[`phase-4-world-1.md`](phase-4-world-1.md), then spec §4 (`u` tiles), §7, §8 (world 7), §10, §12b, §13 (world 7),
§19, §21 (grey recolour); and every `docs/features/name-quest/testing.md` playtest note the user gave in phases 4–8.
Phases 1–8 done. This is the only phase allowed to change numbers (master plan rule 2).

## What the player gets
After Pell's quest, the Unwritten: 10 grey areas where the floor only exists near you, the hardest enemies, and the
First Name, the hero who failed before you, fighting with the other three classes' skills. The class's secret weapon
and a `*` on the save slot reward it. The whole game is tuned to the 15-hour target and ships as a release build.

## Step 1: the Unwritten (`source/quest_world7.cpp`, `source/quest.cpp`, `source/screen.h/.cpp`)
- `u` tiles (spec §4): solid only within 48 px of the hero's centre; drawn as a faint dotted outline (`n`) beyond 48 px
  and as solid `w`-edged `k` blocks within. The reachability test treats `u` as solid (already); add a test that a hero
  far away falls through.
- `void drawSpriteGrey(const Sprite& sprite, int x, int y, bool flip);` — each colour mapped by luminance to `k n d s b w`.
  World 7 enemies and the First Name use it.
- Author the 10 areas (spec §13 "Areas that are special": 1 checkpoint, main path 7–8, area 9 the arena, all band 2,
  chests: spec §10 world 7). Personality: blank pages, ruled lines, half-drawn ruins of all the earlier worlds. The
  world-layout test for world 7 uses its own rules (no gate, no mini-boss, 1 `C`, 1–2 surveyors, ≥ 1 dead end).

## Step 2: The First Name (`source/quest_bosses.cpp`)
Spec §12b. It cycles through the three classes the player isn't (the order fixed by class index), drawing that class's
hero sprites in grey at 1x like the hero, with a 1 px white outline so it reads as a boss. Its skills are the
heroes' tier-3 skills mirrored as enemy attacks with magenta projectiles and the boss's `atk` / `bigAtk`. Death: lines
(author 2 pages: it remembers its name, and thanks you), KEY_SECRET_DONE, the tier-7 weapon (`weapon = 7`), autosave.

## Step 3: the balance tool (`Makefile`, `tests/quest_tests.cpp`)
`make quest-balance` runs `quest_tests --balance` and prints, per world 1–7:
1. **Names on one clear**: sum over every spawn in the world (by kind and world) + chests of names + mini-boss + boss.
2. **Names needed**: level costs from `REF_LEVEL[w-1]` (1 for world 1) to `REF_LEVEL[w]`, plus the world's gear tier
   (weapon + armor) and 10 RED POTION at that world's price.
3. **Clears needed** = needed / one clear.
4. Per class at the reference level and gear: hits to kill fodder / soldier / elite with the basic attack; % of max HP
   lost per soldier hit; boss time-to-kill estimate = boss HP / (class DPS), where DPS = basic damage / cycle + the
   strong skill's damage x (MP regen + 1 MP per basic hit) / its MP cost.

## Step 4: tune to the targets (only these knobs: `WORLD_NAMES`, `HITS_TO_KILL`, `HIT_FRACTION`, class base and growth
numbers within ±20 %, `PRICE_SCALE`, boss gaps within ±25 %)
| Target | Value |
|---|---|
| Clears needed per world | 1.6–2.2 (the PDF: "not easy by just going over the map once") |
| Soldier hit on the reference hero | 18–22 % of max HP (D-047) |
| Each class's boss time-to-kill | 90–200 s, and no class above 1.35x the fastest |
| Each class's fodder hits | 2–4 |
| Playtime estimate | (main path areas x 90 s + side areas x 60 s + clears needed x 15 min + 3 boss tries x TTK) summed ≥ 15 h for worlds 1–6 |
Apply the user's playtest notes first (they win over the tool where they disagree: a boss the user found unfair gets a
longer telegraph, not less HP). Record every change: the spec's numbers, the code, and one GDD decision
`D-052 | <date> | NAME QUEST balance pass: <the changes in one line> | ... | the targets above and the playtest notes`.
Add `balanceTargetsHold` to the tests (asserts the table above from the same computation) so later edits can't drift.

## Step 5: final docs
- `docs/features/name-quest/design.md`: fold the final rules and numbers in (status `implemented`), keep the spec link as
  "the original plan". The final boss is described only as "a surprise final boss" there and in the README.
- `docs/features/name-quest/testing.md`: every PS3 criterion with proof (log lines, dates) or left open with a note.
- `docs/GDD.md`: feature row status `implemented`, the M5 milestone line `M5, NAME QUEST (done <date>)`, change log `0.37`.
- `README.md`: a NAME QUEST section for players: what it is, the controls table (spec §2), "your progress saves at
  checkpoints and when you quit to the menu", three save slots. No spoilers.
- Bump `VERSION` in the `Makefile` to `1.1`.
- `check_docs.py` clean.

## Step 6: PS3 checks and release
1. A 30-minute soak in NAME QUEST on the PS3 (the user plays, or Claude runs a dev save in god mode with a pad script
   walking world 6 back and forth): heap in the `fps` line flat (± 64 KB), worst work < 12 ms, no watchdog line.
2. Fresh-install check with the user: `make release`, install `dist/NAME-game-1.1.pkg` on the PS3 over the dev build, open
   NAME QUEST, new slot, play 10 minutes, quit to menu, relaunch, continue. Slots from the dev build still load
   (same app ID, same files).
3. The user plays the Unwritten and the First Name once without god mode.

## Done when
- `make test` passes (including `balanceTargetsHold`), `make release` builds, `check_docs.py` is clean.
- On the PS3: the release build plays NAME QUEST from a new slot; the soak is clean; the secret world and its boss work.
