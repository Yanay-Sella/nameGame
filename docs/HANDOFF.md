# Handoff: state of the work and how Claude and Codex share it

**Read this first, update it before you stop.** Rules and commands are in `AGENTS.md`; conventions, questions between agents and the small-change lane are in `docs/COLLAB.md`; this file holds what changes between sessions.
Updated: 2026-11-21 (Codex, Classic variants).

## Classic shooter

- Classic offers 1–2 Players and 1–4 Players. The larger arena uses a live controller lobby and a three-pad entry gate. Tuning, source files and test status are in [Co-op](features/co-op/design.md).
- The Mac scripts checked the gate, three- and four-player starts, drop-in, unplug and resume. The cached PS3 Docker toolchain built the package. Physical controller, TV and frame-rate checks remain open.
- For a future 1–2 Players change, ask the user whether to carry it over to 1–4 Players, including when the code is shared. See `AGENTS.md`.

## Where NAME QUEST stands
| Phase | Plan | State |
|---|---|---|
| 1 engine, 1b art | `docs/work-plan/name-quest/phase-1-engine.md`, `phase-1b-art.md` | done, committed (`ab92b67`) |
| 2 combat | `phase-2-combat.md` | done, reviewed, checked on the Mac, `make ps3` builds; **not committed yet** (user commits); PS3 checks open |
| 3 RPG systems | `phase-3-rpg-systems.md` | **next, not started** |
| 4-9 | `phase-4` .. `phase-9` | planned, not started |

- Master plan and rules for working through a phase: `docs/work-plan/name-quest-plan.md`. Rules, numbers and strings: `docs/work-plan/name-quest/spec.md`.
- Feature docs (keep them true to the code): `docs/features/name-quest/{design,implementation,testing}.md`. Implementation lists the code map and every deviation from the plan.
- PS3 acceptance checks for phases 1 and 2 are still unticked in `testing.md`; they need the user's PS3 (HEN) and are not blocking Mac work.

## Next step: phase 3
1. Read `name-quest-plan.md`, then `phase-3-rpg-systems.md`, then only the spec sections it names.
2. Read `source/quest.h`, `quest.cpp`, `quest_combat.cpp`, `quest_draw.cpp`, `quest_areas.cpp` before editing; follow the plan's steps in order.
3. After every step: `make test` (quest checks must pass), then a `make mac-run` script with `l1` screenshots; open the PNG and check the art.
4. Phase 3 is long. If context runs short, finish the step, run `make test`, and write what is done and which step is next under "Session log" below.
5. Finish: update the three feature docs (done criteria, deviations, history), list changed files and a suggested commit message, and ask the user to run the PS3 checks.

## Working split (suggestion, the user decides)
- **Claude** (stronger at long-range design and review): writes plans with `/name-plan`, reviews a phase's diff before the user commits, settles design questions, art direction critique.
- **Codex** (strong at executing a precise plan): implements a phase or step from its plan, runs the test and Mac-run loop, fixes bugs from logs.
- Either can do either job; the plan files and the feature docs are the contract, so keep them exact. Never have both write the same files at once.

## Known issues
- `make test` now runs both suites every time and fails if either fails (before, the shooter failures stopped it before the quest tests ran).
- `make test`: 53 checks in `tests/game_tests.cpp` fail (e.g. line 1668 `lob.toX ...`, lines 1820-1844 alien hp and beam checks). They fail identically on the committed HEAD, so they predate phase 2. They look like stale shooter tests; investigate before trusting the shooter's behaviour, and don't "fix" them by loosening checks without understanding why.
- Crumbling blocks are solid until phase 4. Namula's NPC spots have no `npcs[]` entries until phase 3.
- For phase 4: electric panels `z` hurt when the hero overlaps the tile or the tile below its feet row, so a hero up to one tile above a lit panel is hurt. Fine if panels sit in the floor row; decide when world 1 places them (`quest_combat.cpp`, the trap loop at the end of `updateCombat`).
- Art for the user to judge: TEST GROUNDS area 3 borrows world 2's STAG, whose dark sprite almost disappears on the dark test background; Lion's Roar's arc (72 x 40 dp, centred on the hero) dips into the floor.
- Spec examples that disagree with the spec formula `round(100 * 1.2^(L-1))`: 39→40 is 102,067 (spec 102,064), 29→30 is 16,484 (spec 10,646). The code follows the formula; the user decides.

## Session log (newest first; add a line per session: date, agent, what changed, what is next)
- 2026-11-21, Codex: split Classic and verified the larger arena, all 100 waves, controller lobby, four ships and upgrade rates. `make test` matched the 53 baseline shooter failures and NAME QUEST passed. Next for Classic: a PS3 playtest when the user asks for it.
- 2026-11-21, Claude: reviewed phase 2. Fixed R2 during a melee swing losing the skill's hit, added the spike bounce (spec §4), made `make test` run the quest tests even when shooter tests fail; 2 new quest checks; Mac screenshots of the tier-3 skills and the death and echo loop. Next: the user commits and runs the PS3 checks, then phase 3 step 1.
- 2026-11-21, Codex: confirmed the shared instructions and all seven project skills were visible in this session and replied in `COLLAB.md`. Next: phase 3 step 1 after the phase 2 commit.
- 2026-11-20, Claude: phase 2 combat built and Mac-checked; wrote `AGENTS.md`, `.agents/skills`, this file for the Codex handoff. Next: phase 3 step 1.
