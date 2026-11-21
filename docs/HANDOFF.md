# Handoff: state of the work and how Claude and Codex share it

**Read this first, update it before you stop.** Rules and commands are in `AGENTS.md`; conventions, questions between agents and the small-change lane are in `docs/COLLAB.md`; this file holds what changes between sessions.
Updated: 2026-11-21 (Claude, NAME QUEST phase 3).

## Classic shooter

- Classic offers 1–2 Players and 1–4 Players. The larger arena uses a live controller lobby and a three-pad entry gate. Tuning, source files and test status are in [Co-op](features/co-op/design.md).
- The Mac scripts checked the gate, three- and four-player starts, drop-in, unplug and resume. The cached PS3 Docker toolchain built the package. Physical controller, TV and frame-rate checks remain open.
- For a future 1–2 Players change, ask the user whether to carry it over to 1–4 Players, including when the code is shared. See `AGENTS.md`.

## Where NAME QUEST stands
| Phase | Plan | State |
|---|---|---|
| 1 engine, 1b art | `docs/work-plan/name-quest/phase-1-engine.md`, `phase-1b-art.md` | done, committed (`ab92b67`) |
| 2 combat | `phase-2-combat.md` | done, committed (`f871aac`); PS3 checks open |
| 3 RPG systems | `phase-3-rpg-systems.md` | done on `claude/work`, Mac-checked, `make ps3` builds; **not committed yet**; PS3 check (step 12) open |
| 4 world 1 | `phase-4-world-1.md` | **next** |
| 4-9 | `phase-4` .. `phase-9` | planned, not started |

- Master plan and rules for working through a phase: `docs/work-plan/name-quest-plan.md`. Rules, numbers and strings: `docs/work-plan/name-quest/spec.md`.
- Feature docs (keep them true to the code): `docs/features/name-quest/{design,implementation,testing}.md`. Implementation lists the code map and every deviation from the plan.
- PS3 acceptance checks for phases 1 and 2 are still unticked in `testing.md`; they need the user's PS3 (HEN) and are not blocking Mac work.

## Next step: phase 4
1. The user merges `claude/work` (phase 3) and runs its PS3 check (`phase-3-rpg-systems.md` step 12; listen for the hearth's crackle in Namula).
2. Then phase 4: read `name-quest-plan.md`, `phase-4-world-1.md` and its spec sections. Phase 3 left hooks for it: `openGate(q, world)` for the mini-boss, `resetHazards` in `quest.cpp` for crumbles, and world 1's entry checkpoint still needs lighting from the start (spec §11).
3. Unused `AreaDef::npcs` entries must be written as eight -1s (0 is Ilsa); the area test checks it.

## Working split (suggestion, the user decides)
- **Claude** (stronger at long-range design and review): writes plans with `/name-plan`, reviews a phase's diff before the user commits, settles design questions, art direction critique.
- **Codex** (strong at executing a precise plan): implements a phase or step from its plan, runs the test and Mac-run loop, fixes bugs from logs.
- Either can do either job; the plan files and the feature docs are the contract, so keep them exact. Never have both write the same files at once.

## Known issues
- `make test` now runs both suites every time and fails if either fails (before, the shooter failures stopped it before the quest tests ran).
- `make test`: 53 checks in `tests/game_tests.cpp` fail (e.g. line 1668 `lob.toX ...`, lines 1820-1844 alien hp and beam checks). They fail identically on the committed HEAD, so they predate phase 2. They look like stale shooter tests; investigate before trusting the shooter's behaviour, and don't "fix" them by loosening checks without understanding why.
- Crumbling blocks are solid until phase 4.
- Phase 3 numbers to look at in phase 9 (not changed): the NPC talk reach (14 dp) needs the hero almost touching the NPC; Mac scripts missed Marra from 18 dp. World 6's chest 5 (WHETSTONE + IRON TONIC) gives only the WHETSTONE: one `ChestContent` holds one item.
- For phase 4: electric panels `z` hurt when the hero overlaps the tile or the tile below its feet row, so a hero up to one tile above a lit panel is hurt. Fine if panels sit in the floor row; decide when world 1 places them (`quest_combat.cpp`, the trap loop at the end of `updateCombat`).
- Art for the user to judge: TEST GROUNDS area 3 borrows world 2's STAG, whose dark sprite almost disappears on the dark test background; Lion's Roar's arc (72 x 40 dp, centred on the hero) dips into the floor.
- Spec examples that disagree with the spec formula `round(100 * 1.2^(L-1))`: 39→40 is 102,067 (spec 102,064), 29→30 is 16,484 (spec 10,646). The code follows the formula; the user decides.

## Session log (newest first; add a line per session: date, agent, what changed, what is next)
- 2026-11-21, Claude: phase 3 RPG systems built (checkpoints, travel, Namula's NPCs and sprites, levels, shops, items and buffs, jobs, chests, gate, lever, map, ambience), 17 quest checks, Mac screenshots, `make ps3` builds. Next: the user commits and runs the step-12 PS3 check, then phase 4.
- 2026-11-21, Codex — Removed the Triangle stress-test shortcut from both Classic variants. Mac scripts and screenshots confirmed normal waves in Classic and the stress load in Level Picker. NAME QUEST tests passed and the shooter suite reported its 53 known failures. Next for Classic is a PS3 playtest when requested.
- 2026-11-21, Codex: reviewed Claude's completed phase 3 in a local copy. NAME QUEST tests passed, and the shooter failures matched the baseline. Checked levelling, shops, travel, chests, gate, shortcut and map on Mac. Critical findings are in the user review. PS3 play remains unchecked.
- 2026-11-21, Codex: split Classic and verified the larger arena, all 100 waves, controller lobby, four ships and upgrade rates. `make test` matched the 53 baseline shooter failures and NAME QUEST passed. Next for Classic: a PS3 playtest when the user asks for it.
- 2026-11-21, Claude: reviewed phase 2. Fixed R2 during a melee swing losing the skill's hit, added the spike bounce (spec §4), made `make test` run the quest tests even when shooter tests fail; 2 new quest checks; Mac screenshots of the tier-3 skills and the death and echo loop. Next: the user commits and runs the PS3 checks, then phase 3 step 1.
- 2026-11-21, Codex: confirmed the shared instructions and all seven project skills were visible in this session and replied in `COLLAB.md`. Next: phase 3 step 1 after the phase 2 commit.
- 2026-11-20, Claude: phase 2 combat built and Mac-checked; wrote `AGENTS.md`, `.agents/skills`, this file for the Codex handoff. Next: phase 3 step 1.
