# Phase 7: quests, NPC stories, job trials, Trial of Echoes, Sky Stair

Read first: [`../name-quest-plan.md`](../name-quest-plan.md), then spec §1, §6 "Jobs", §11 (Namula table, portals),
§14 (all of it), §17 (keys, quests), §18. Phases 1–6 done. Search `quest_story.cpp` for `// phase 7` markers first:
every one must be gone at the end of this phase.

## What the player gets
Every quest of spec §14 works end to end, with no quest menu: NPCs re-explain their current stage when talked to again.
The Trial of Echoes gives the tier-3 job. The Gatekeeper opens the Sky Stair after five Great Names (world 6's entry
is lit; world 6 itself comes in phase 8, so until then the Sky Stair leads to a one-area placeholder like phase 4's).
Pell's five map pages open the portal to The Unwritten (placeholder area until phase 9).

## Step 1: quest effects (`source/quest_story.cpp`)
`talkTo` grows into the full table of spec §14. Rules:
- Each NPC's case reads only its quest stage and the save, picks the pages for the current stage, and applies at most
  one stage change per talk (items taken or given, the stage, a log `quest: quest %s stage %d`, autosave).
- Give/take helpers only where reused: `bool takeItem(SaveData&, ItemType)`, `void giveKey(Quest&, KeyItem)` (pop-up text
  like a chest). Don't add a quest engine, scripting tables or callbacks; a `switch` per NPC is the design.
- Effects of key items: TOBIN'S CHARM (HP x1.05, already in `heroStats`), GARRICK's gift (BIG POTION carry +1), RING OF
  THE STRANGER (names x1.15), SELA'S LOCKET (+1 carry for every potion), refines (Brann). Check `carryLimit` and
  `heroStats` read the keys.
- Author every remaining line in the spec §1 tone, using the spec's key lines verbatim. Each NPC has a line for every
  stage, including "done" (a warm closing line they repeat).

## Step 2: the mentor's trials (`source/quest_story.cpp`, `source/quest.cpp`, `source/quest_areas.cpp`)
- Tier 3 (spec §14): at level ≥ 30 the mentor opens the TRIAL OF ECHOES portal on the Shore. Author the trial as Namula
  area 3 (wide arena, `C`-less; flag it with a new `AREA_TRIAL = 16`), entered by that portal, left by the same portal.
- Trial run: entering locks the exits and starts wave 1; each wave spawns from authored `X` anchors when the last is
  dead (spec §14 waves; enemies at world 5 stats: build them with world-5 numbers and the listed rosters; the Monkey
  King and the Yeti reuse their boss functions at world 5 stats). Clearing wave 3: KEY_TRIAL_DONE, exits open, banner
  `TRIAL CLEARED`. Death inside: no echo, no names lost, back at the hearth (log `quest: hero died in 0.3 (trial)`).
- The mentor then gives tier 3 on the next talk.

## Step 3: Sky Stair and the placeholders
The Gatekeeper's count = popcount of `bosses` bits 1–5. At 5: the Sky Stair portal (Namula cliffs' `P`) opens and
lights world 6's entry. Until phase 8, world 6 is a one-area placeholder room with its `C`; until phase 9, world 7 too.

## Step 4: tests
`tobinQuestRunsToTheEnd` (drive stages with a fake save: talk to each NPC in order, check items and stages),
`pellNeedsAllFivePages`, `strangerAppearsAfterWorldThree`, `strangerQuestGivesTheRing`, `smithRefinesInOrder`,
`selaSpeaksAtTwentyAndGivesAtThirtyFive`, `trialDeathCostsNothing`, `trialGivesTierThree`, `skyStairNeedsFiveNames`,
`npcRepeatsTheCurrentStage` (talking twice changes nothing), `noPhase7MarkersLeft` (the test reads
`source/quest_story.cpp` and fails if it contains `phase 7`).

## Step 5: docs
design.md: the quests as numbered rules (stage tables); testing.md; GDD change log `0.35`.

## Step 6: Mac self-check and PS3 check
- Mac: dev starts with the quest stages and key items set (`NAMEGAME_QUEST='slot=3 class=3 world=0 area=2 level=35 keys=<hex> quest1=4'`,
  spec §20): run each quest's last talks and read the pop-ups.
- PS3: a dev save at level 30 on the Shore: open and clear the Trial in god mode; expect `quest: quest JOB stage 3`,
  `quest: job 3 <name>`. Ask the user to read three NPC conversations on the TV for text fit (no line runs off the box).

## Done when
- `make test` passes (no `// phase 7` left), `make ps3` builds, `check_docs.py` is clean.
- On the PS3: the Trial gives the tier-3 job; the Gatekeeper opens the Sky Stair with five Great Names.
