# Handoff: state of the work and how Claude and Codex share it

**Read this first, update it before you stop.** Rules and commands are in `AGENTS.md`; conventions, questions between agents and the small-change lane are in `docs/COLLAB.md`; this file holds what changes between sessions.
Updated: 2026-11-21 (Classic ship roles and NAME QUEST phase 4 audio and autosave).

## Classic shooter

- The Classic menu offers 1–2 Players, 1–4 Players and Level Picker. The larger arena uses a live controller lobby and a three-pad entry gate. Tuning, source files and test status are in [Co-op](features/co-op/design.md).
- The Mac scripts checked the gate, three- and four-player starts, drop-in, unplug and resume. The cached PS3 Docker toolchain built the package. Physical controller, TV and frame-rate checks remain open.
- Expanded parties of three or four now choose Fighter, Tank, Assassin or Mage. The third drop-in opens choices for everyone and a fourth drop-in keeps the existing picks. The Mac scripts checked both starts and both drop-in paths, including cancel and reconnect. The [Co-op tuning and controls](features/co-op/design.md) are ready for the PS3 playtest.
- For a future 1–2 Players change, ask the user whether to carry it over to 1–4 Players, including when the code is shared. See `AGENTS.md`.

## Where NAME QUEST stands
| Phase | Plan | State |
|---|---|---|
| 1 engine, 1b art | `docs/work-plan/name-quest/phase-1-engine.md`, `phase-1b-art.md` | done, committed (`ab92b67`) |
| 2 combat | `phase-2-combat.md` | done, committed (`f871aac`); PS3 checks open |
| 3 RPG systems | `phase-3-rpg-systems.md` | done, merged into `main` (`1456d78`), Mac-checked, `make ps3` builds; PS3 check (step 12) open |
| 4 world 1 | `phase-4-world-1.md` | **built on `claude/work`**: steps 0-9 done (Mac); step 10 (PS3 check, the user's playtest) open |
| 5-9 | `phase-5` .. `phase-9` | planned, not started |

- Master plan and rules for working through a phase: `docs/work-plan/name-quest-plan.md`. Rules, numbers and strings: `docs/work-plan/name-quest/spec.md`.
- Feature docs (keep them true to the code): `docs/features/name-quest/{design,implementation,testing}.md`. Implementation lists the code map and every deviation from the plan.
- PS3 acceptance checks for phases 1 and 2 are still unticked in `testing.md`; they need the user's PS3 (HEN) and are not blocking Mac work.

## Next step: phase 4, step 10 (when the user asks for the PS3)
1. PS3 check per `phase-4-world-1.md` step 10 (ask the user first): travel to world 1 from slot 1; then the Star Mage from a dev save (`NAMEGAME_QUEST='slot=3 class=0 world=1 area=19 level=8 gear=1'` writes `build/mac/quest3.txt`; copy it to `pkgfiles/USRDIR/quest3.txt` (gitignored), `make ps3`, and delete the copy after).
2. The user's world 1 playtest: record their notes in `docs/features/name-quest/testing.md`; don't retune before phase 9 unless something is broken.
3. For the user to confirm: the Overseer's hover height is read as its centre 50 dp above the floor (see `implementation.md` › Deviations).
4. Then phase 5 (worlds 2-3). It replaces world 2's placeholder room (`WORLD2_AREAS` in `quest_areas.cpp`); its rosters must be drawn at least 12 dp tall (extend `arrowsHitEveryGroundEnemy`).
5. Unused `AreaDef::npcs` entries must be written as eight -1s (0 is Ilsa); the area test checks it.

## Working split (suggestion, the user decides)
- **Claude** (stronger at long-range design and review): writes plans with `/name-plan`, reviews a phase's diff before the user commits, settles design questions, art direction critique.
- **Codex** (strong at executing a precise plan): implements a phase or step from its plan, runs the test and Mac-run loop, fixes bugs from logs.
- Either can do either job; the plan files and the feature docs are the contract, so keep them exact. Never have both write the same files at once.

## Known issues
- `make test` now runs both suites every time and fails if either fails (before, the shooter failures stopped it before the quest tests ran).
- `make test`: 53 checks in `tests/game_tests.cpp` fail (e.g. line 1668 `lob.toX ...`, lines 1820-1844 alien hp and beam checks). They fail identically on the committed HEAD, so they predate phase 2. They look like stale shooter tests; investigate before trusting the shooter's behaviour, and don't "fix" them by loosening checks without understanding why.
- Crumbling blocks are solid until phase 4.
- Phase 3 numbers to look at in phase 9 (not changed): the NPC talk reach (14 dp) needs the hero almost touching the NPC; Mac scripts missed Marra from 18 dp. World 6's chest 5 (WHETSTONE + IRON TONIC) gives only the WHETSTONE: one `ChestContent` holds one item.
- Audio: square-wave drones sound like white noise on the PS3's TV speakers; Namula's and world 1's were removed. Worlds 3-7's planned ambiences are continuous noise (wind, rumble, water) and may bother the user the same way. The user supplies NAME QUEST music (D-057): `quest_namula.ogg`, `quest_world1..7.ogg`, `quest_boss.ogg` via `make music`; Claude makes every other sound.
- Ground enemies must be drawn at least 12 dp tall with their outline (spec §22), or an Archer's basic arrows fly over them; phases 5-9: extend `arrowsHitEveryGroundEnemy` to each redrawn roster.
- Phase 4 decided: electric panels sit in the air row on a floor and hurt only while the hero overlaps the tile.
- `check_docs.py docs` reports 7 broken links in old shooter work plans (`boss-lineup-plan.md`, `level-1-ending-plan.md`, `level-buffs-plan.md`); the NAME QUEST docs are clean.
- A wide boss arena is two screens: the Overseer sweeps the half the camera isn't showing for a while. Fine by the spec; worth watching in the step 8 playtest.
- Art for the user to judge: TEST GROUNDS area 3 borrows world 2's STAG, whose dark sprite almost disappears on the dark test background; Lion's Roar's arc (72 x 40 dp, centred on the hero) dips into the floor.
- Spec examples that disagree with the spec formula `round(100 * 1.2^(L-1))`: 39→40 is 102,067 (spec 102,064), 29→30 is 16,484 (spec 10,646). The code follows the formula; the user decides.

## Session log (newest first; add a line per session: date, agent, what changed, what is next)
- 2026-11-21, Codex: set the Space buff's Magic Beam to 100 damage in both Classic variants (D-060). The beam checks and NAME QUEST tests passed. The other 50 shooter failures matched the baseline. A headless Mac screenshot confirmed the beam fired through a column of enemies in a three-player run.
- 2026-11-21, Codex — Added boss-wave revives to Classic 1–2 Players. The focused regression and NAME QUEST passed, and shooter failures matched the baseline. Checked the returning ship and its two spare hearts in a Mac render. PS3 playtest pending.
- 2026-11-21, Codex: checked the Classic Space buff tuning in code. The beam deals 60 damage per target in its column, and firing again during the charge restarts it and spends another rocket.
- 2026-11-21, Claude: removed the square-wave hums under Namula and world 1 (the user's PS3 white noise); NAME QUEST music from the user's songs per place plus a boss song (D-057, `audioSetQuestSound`). PS3 listening checks open. Then: quitting the app now saves the NAME QUEST slot (spec §17); `scripts/ps3-lock.sh` leaves a game that was already running alone (new `make ps3-state`).
- 2026-11-21, Codex — Added Classic ship roles for three- and four-player parties, with a per-controller chooser, role HUD, new sprites, and R2 specials. The new combat checks and NAME QUEST suite passed, and the shooter failures matched the baseline. Checked selection, drop-in and effects on the Mac and built the PS3 package. Next: PS3 couch playtest when requested.
- 2026-11-21, Claude: the user approved world 1's look. World 1's 20 areas built (`quest_world1.cpp`), world 1 in the reachability test plus gate, layout-rule and chest checks; all 40 map screens read; the Overseer now hovers with its centre at 50 dp (was unreachable); a locked arena's ladder down no longer kills; boss fights measured with scratch bots. Next: step 10 on the PS3 when the user asks.
- 2026-11-21, Claude: phase 4 steps 0-4 and 7: world 1 art (generator `world1.py`, layers, 13 props, 6 landmarks, space pits), redrawn world 1 enemies (4 movement frames + tell + attack), the boss framework with the Overseer and the Star Mage (tested in new TEST GROUNDS arenas 4-5), panels and vents drawn, `make quest-maps`, world 1's entry lit from the start, Ilsa's world 1 Great Name line, world 2's placeholder room. 5 boss checks; `make ps3` builds. Next: the user's OK on the style frame, then step 5.
- 2026-11-21, Codex — Resolved the merge conflict in this handoff and combined the Classic menu and NAME QUEST phase 3 changes on `codex/work`.
- 2026-11-21, Codex — Moved Level Picker into Classic and removed the shooter stress test. Mac scripts and screenshots checked the new menus, both full-run options, Jungle from the picker, and the NAME QUEST Triangle actions. NAME QUEST tests passed and the shooter suite reported its 53 known failures. Next for Classic is a PS3 playtest when requested.
- 2026-11-21, Claude: phase 3 RPG systems built (checkpoints, travel, Namula's NPCs and sprites, levels, shops, items and buffs, jobs, chests, gate, lever, map, ambience), 17 quest checks, Mac screenshots, `make ps3` builds. Next: the user commits and runs the step-12 PS3 check, then phase 4.
- 2026-11-21, Codex — Removed the Triangle stress-test shortcut from both Classic variants. Mac scripts and screenshots confirmed normal waves in Classic and the stress load in Level Picker. NAME QUEST tests passed and the shooter suite reported its 53 known failures. Next for Classic is a PS3 playtest when requested.
- 2026-11-21, Codex: reviewed Claude's completed phase 3 in a local copy. NAME QUEST tests passed, and the shooter failures matched the baseline. Checked levelling, shops, travel, chests, gate, shortcut and map on Mac. Critical findings are in the user review. PS3 play remains unchecked.
- 2026-11-21, Codex: split Classic and verified the larger arena, all 100 waves, controller lobby, four ships and upgrade rates. `make test` matched the 53 baseline shooter failures and NAME QUEST passed. Next for Classic: a PS3 playtest when the user asks for it.
- 2026-11-21, Claude: reviewed phase 2. Fixed R2 during a melee swing losing the skill's hit, added the spike bounce (spec §4), made `make test` run the quest tests even when shooter tests fail; 2 new quest checks; Mac screenshots of the tier-3 skills and the death and echo loop. Next: the user commits and runs the PS3 checks, then phase 3 step 1.
- 2026-11-21, Codex: confirmed the shared instructions and all seven project skills were visible in this session and replied in `COLLAB.md`. Next: phase 3 step 1 after the phase 2 commit.
- 2026-11-20, Claude: phase 2 combat built and Mac-checked; wrote `AGENTS.md`, `.agents/skills`, this file for the Codex handoff. Next: phase 3 step 1.
