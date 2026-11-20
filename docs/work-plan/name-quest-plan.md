# Plan: NAME QUEST, an RPG platformer inside NAME game (master plan)

Source: the user's PDF "Name game inside NAME game" (3 pages) plus their answers to 30 design questions (2026-11-20).
NAME QUEST is a second game on the main menu: a hard 2D side-scrolling action RPG (MapleStory movement and classes,
Dark Souls hub, death and checkpoints, Hollow Knight-style getting lost), built in its own files next to the shooter.

This file is the entry point. The design lives in [`name-quest/spec.md`](name-quest/spec.md); the work is cut into
nine phase plans, one Opus 5.5 session each. Each phase ends playable and checked on the Mac and the PS3.

## For the implementer (Opus 5.5): how to work through a phase

1. Start a fresh session per phase. Read this file, then the phase file, then only the `spec.md` sections that phase
   names (the spec is long; each phase lists its `§` sections). Read the source files each step names before editing them.
2. **Decisions are final.** Every rule, number, name, button and string in `spec.md` was decided with the user.
   Don't redesign them. Where the spec says "author" (tile maps, sprites, dialogue lines, backgrounds), the content is
   yours to create inside the stated rules; the validators in `tests/quest_tests.cpp` decide whether it is acceptable.
   If a number turns out wrong in play, don't change it outside phase 9: write it in the phase's report instead.
3. **Code style:** `CLAUDE.md` and the existing code. Plain C++14, no new libraries, no classes with virtual methods,
   no scene or entity frameworks, no `new`/`delete` (use `std::vector` and fixed arrays), short one-line comments.
   Keep each update and draw function readable from top to bottom; one `switch` per state machine. Don't touch the
   shooter's rules (`game.cpp`, `formations.cpp`) except where a step says so.
4. **No SDL or PS3 headers in quest files** (`source/quest*.{h,cpp}`), so `make test` builds them on the Mac.
   Only `main.cpp`, `input.cpp`, `audio.cpp` talk to SDL.
5. **Verify as you go, cheapest first:** `make test` after every step that touches rules or data; `make mac-run`
   with a `BUTTONS=` script plus `l1` screenshots (read `shots/shot.png`) to see the result yourself; the PS3 last,
   in one locked command (see the phase's PS3 step). Fix what you see before moving on.
6. **Logs prove behaviour on the PS3.** Every step that adds an event (death, level up, boss phase...) adds the
   `debugNetPrintf(INFO, ...)` line the phase's PS3 step greps for. Keep the exact formats given.
7. **Don't commit.** The user commits. End the phase by listing the changed files and a suggested one-line commit
   message, plus a short report: what was built, what the PS3 check showed, anything that didn't work or that you
   think should be retuned (with numbers).
8. Long phases (4–6, 8): if your context gets long, finish the current step, run `make test`, and write a
   `docs/work-plan/name-quest/progress.md` note (what is done, what step is next) so a new session can resume.
   Delete that note when the phase is done.

## Phases

| # | Plan | Builds | Playable result |
|---|---|---|---|
| 1 | [phase-1-engine.md](name-quest/phase-1-engine.md) | menu item, save slots, class select, tiles, platformer physics, camera, area links, pit death, HUD, reachability validator | walk, jump and climb through 4 test areas on the PS3; saves survive a relaunch |
| 2 | [phase-2-combat.md](name-quest/phase-2-combat.md) | 4 classes x 4 job tiers of attacks, MP, dodge, 8 enemy behaviours, damage formulas, names drop and recovery, hit feel | fight test enemies with every class; die, recover your names |
| 3 | [phase-3-rpg-systems.md](name-quest/phase-3-rpg-systems.md) | checkpoints (rest, travel), Namula hub (3 areas), Ilsa's level up, two shops, potions and buffs, jobs, chests, gates, shortcuts, map NPC and map screen, NPC dialogue box, ambience and SFX | the full RPG loop in the test areas: earn, die, rest, level, shop |
| 4 | [phase-4-world-1.md](name-quest/phase-4-world-1.md) | intro, world 1 (The Drifting Station): 20 areas, enemies, hazards, the boss framework, The Overseer, The Star Mage; first Namula dialogue | world 1 start to finish |
| 5 | [phase-5-worlds-2-3.md](name-quest/phase-5-worlds-2-3.md) | world 2 (The Greendeep), world 3 (The White Silence), 4 bosses | worlds 1–3 |
| 6 | [phase-6-worlds-4-5.md](name-quest/phase-6-worlds-4-5.md) | world 4 (The Ember Maw), world 5 (The Sunken Halls, underwater physics, darkness), 4 bosses | worlds 1–5 |
| 7 | [phase-7-quests.md](name-quest/phase-7-quests.md) | every quest chain and NPC, job trials, Trial of Echoes, Sky Stair | the whole Namula story so far |
| 8 | [phase-8-citadel-finale.md](name-quest/phase-8-citadel-finale.md) | world 6 (The Nameless Citadel), Hollow Twins, Name-Eater, Claude, credits, New Game+ | the game can be finished |
| 9 | [phase-9-secret-balance.md](name-quest/phase-9-secret-balance.md) | secret world (The Unwritten), The First Name, balance pass with a simulated playthrough, docs, release | release candidate |

Phases depend on the ones before them; don't start phase n+1 before phase n's "Done when" holds.

## Decisions taken with the user (record them in the GDD in phase 1)

| ID | Decision |
|---|---|
| D-035 | NAME QUEST is a separate RPG platformer on the main menu with its own code and saves; the shooter's pillar 3 ("arcade, not a campaign") applies to Classic and the level picker only |
| D-036 | 6 worlds in a fixed order: the 5 shooter themes, then The Nameless Citadel; 20 areas each; a secret world unlocked by a quest |
| D-037 | 3 save slots with autosave (rest, level up, purchase, quest step, boss kill, quit); no manual save |
| D-038 | Death drops all carried names where you died; touching them recovers them; dying again first loses them |
| D-039 | Healing only from bought potions; resting at a checkpoint heals fully and respawns enemies; big bosses stay dead, mini-bosses respawn |
| D-040 | Max level 40, no stat points; jobs at levels 5, 15, 30 through the class mentor |
| D-041 | The strong attack costs MP |
| D-042 | Controls: X jump, R1 attack (hold to repeat), R2 strong attack, Circle dodge, Square drink potion, Triangle switch potion, Up talk / portal / climb, Start pause |
| D-043 | Areas are 2 screens: 80x23 tiles wide or 40x45 tiles tall (8 px tiles); exits at the edges and through portals |
| D-044 | Areas are hand-made ASCII tile maps, checked by a physics-based reachability test |
| D-045 | Single player only |
| D-046 | Pits and deadly liquids kill instantly |
| D-047 | A normal enemy hit takes 20 % of the reference hero's max HP; enemy stats derive from a reference hero per world |
| D-048 | The final boss is Claude, never hinted before the citadel's last room; after the credits Claude waits in Namula and offers New Game+ |
| D-049 | No music in NAME QUEST for now: synthesized ambience per world and sound effects |
| D-050 | Each world's mini-boss holds the key to the gate in front of the world's second half |
| D-051 | Worlds 1–5 reuse the shooter's enemy and boss sprites where the theme matches (the RPG is set in the same worlds) |

## Research that shaped the spec

- Dark Souls 2's Majula and the Emerald Herald: the only way to level is to spend souls with her by the hub bonfire,
  costs rise each level, and souls drop on death with one chance to recover them ([Fextralife](https://darksouls2.wiki.fextralife.com/Level),
  [Gamer Guides](https://www.gamerguides.com/dark-souls-2/beginners-guide/characters/levelling-up), [Wikipedia](https://wikipedia.com/wiki/Dark_Souls_2)).
  → Ilsa by the hearth, rising level costs, names dropped on death, the "talk to her a few times first" nod.
- Hollow Knight's cartography: the map is earned, Cornifer is found in each zone, and getting lost is the point
  ([Super Jump](https://www.superjumpmagazine.com/getting-lost-by-design-in-hollow-knight/), [A.V. Club](https://www.avclub.com/hollow-knights-ingenious-map-system-ensures-theres),
  [PC Gamer](https://pcgamer.com/how-to-design-a-great-metroidvania-map)). → surveyors as the only map, dead ends with rewards,
  loops and one-way shortcuts back to checkpoints, and Pell the cartographer's lost pages as the secret-world quest.
- MapleStory's first job at level 10 from a class instructor in town, four base classes and branching jobs
  ([StrategyWiki](https://StrategyWiki.org/wiki/MapleStory/Jobs), [u4n](https://www.u4n.com/news/maplestory-classic-world-first-and-second-job-advancement-guide.html)).
  → class mentors in Namula, three job tiers with our own names and skills.
- Boss design: every attack telegraphed (wind-up, glow, marker), telegraph → attack → recovery, 2–3 phases that add
  patterns instead of only HP ([molbv](https://molbv.github.io/blog/2025-07-08-boss-phases.html),
  [Game Developer, Psychonauts 2](https://www.gamedeveloper.com/marketing/using-a-modular-system-of-maneuvers-to-design-i-psychonauts-2-i-s-boss-fights-in-a-hurry),
  [Wayline](https://www.wayline.io/blog/crafting-legendary-boss-battles)). → spec §12 boss rules.
- Platformer feel (common practice): coyote time, jump buffering, variable jump height, i-frames after a hit.
