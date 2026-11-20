# Phase 8: world 6 (The Nameless Citadel), the Hollow Twins, the Name-Eater, Claude, credits, New Game+

Read first: [`../name-quest-plan.md`](../name-quest-plan.md), the **world authoring recipe** in
[`phase-4-world-1.md`](phase-4-world-1.md), then spec §1, §8 (world 6 roster and traits), §9 (world 6), §10 (world 6
chests), §12 (world 6 rows, §12a), §13 (world 6), §14 (story beats), §19. Phases 1–7 done.

**Keep the secret.** Players must not see or hear anything about Claude before world 6 area 19: no sprite, line or
name on screen anywhere else (the credits and the post-credits Shore come after it). Area 19's banner is
`THE LAST ROOM`. Logs may say `quest: boss CLAUDE start` (players don't see logs).

## What the player gets
The citadel above the clouds: name-stealing wraiths, gargoyles, statue archers, shade hounds and citadel knights over
the void; crumbling bridges, moving platforms and wind. The Hollow Twins guard the gate. The Name-Eater waits in area
18; behind its throne, area 19 holds the true last fight: Claude. Then the credits, Namula again, and Claude by the
fire on the Shore offering New Game+.

## Step 1: world 6 mechanics and enemies
- Moving platforms `O` (spec §4): a list per area, updated before the hero; a hero standing on one moves with it
  (add its dx to the hero's x before `moveHero`'s collision). Reset on area load.
- NAME WRAITH steal (spec §8): the stolen amount is held by that wraith; killing it within 10 s drops them as a pickup
  that flies to the hero; after 10 s they're gone. Show `-n NAMES` in `r` on a steal.
- Sprites: NAME WRAITH 12x12, GARGOYLE 12x12, STATUE ARCHER 12x16, SHADE HOUND 16x10, CITADEL KNIGHT 16x18 (+ wind-up),
  world 6 tiles (grey-gold marble, gilded edges), the void (black with far clouds below), background (spec §13), the
  throne door (closed/open).

## Step 2: bosses
- Hollow Twins (two `Boss` entries, both QBOSS_TWIN_*; the arena locks for both; the bar shows `THE HOLLOW TWINS` with
  their summed HP); the rage rule when one dies. Sprites 20x24 each (author).
- The Name-Eater (spec §12 world 6 row; 48x40 sprite, author: a hooded maw stitched from grey name-ribbons).
  Dialogue before (a DIALOGUE mode page triggered on arena entry, the fight starts when it closes) and at death. Its
  phase-2 steals are recovered as an echo-like pickup at its death spot. Its death opens the throne door (a `k`-style
  door in area 18 opened by code, saved by its `bosses` bit 8) to area 19.
- Claude (spec §12a, every number and line). New sprites: CLAUDE idle A/B, cast, hurt, kneel (20x28), SUBAGENT (10x14).
  TOKEN RAIN glyphs: random A–Z drawn with `drawText` in `m`, each a projectile with a 4x5 hitbox. REFACTOR finds the
  arena's three one-way platform runs at fight start (scan for `-` runs) and swaps their rows in the live `AreaState`
  (restored on reset). CONTEXT WINDOW walls are two solid rectangles handled like trap damage plus a push.
  DEEP THINKING's beam: 12 px tall at floor level, sweeping across in `m`/`w`.
- Claude's death: kneel, the defeat lines (§14), the 2 s whitening (a cut with reduce flashing), bosses bit 9, autosave,
  QMODE_CREDITS.

## Step 3: credits and New Game+ (`source/quest.cpp`, `source/quest_draw.cpp`, `source/quest_story.cpp`)
- QMODE_CREDITS exactly spec §19 (text, speed, `THE END`, X → hearth, KEY_CREDITS_SEEN, autosave). Start doesn't pause here.
- Claude on the Shore (`npcPresent` after KEY_CREDITS_SEEN): the §14 line, then MENU_YES_NO `YES` / `NOT YET`; YES applies
  spec §19's reset list exactly, logs `quest: ng+ %d`, autosaves, and puts the hero at the hearth. NG+ factors already
  come from phase 2's `enemyAttack/Health/Names(…, ngPlus)`; check every call passes `save.ngPlus`.
- The slot screen shows `NG+n` (phase 1 layout).

## Step 4: author world 6 (new `source/quest_world6.cpp`)
Remove the phase-7 placeholder. Personality: a cold, beautiful, empty palace of gold and grey above a sea of clouds;
banners with no names on them; the deeper you go, the more the floors are missing. Area 18 is the Name-Eater's hall,
area 19 Claude's room (`=` walls, 3 one-way platforms, nothing else). Chests: spec §10 world 6. The gate is a gilded
portcullis.

## Step 5: tests
World 6 in the world tests (area 18 and 19 are both AREA_BOSS; the main path ends 18 → 19; the reachability BFS opens the
throne door like a gate). Plus `wraithStealsAndGivesBack`, `movingPlatformCarriesTheHero`, `twinsRageWhenOneFalls`,
`nameEaterOpensTheThroneDoor`, `claudeHasThreePhases`, `refactorSwapsAndRestoresPlatforms`, `contextWindowPushesBack`,
`creditsReturnToNamula`, `ngPlusResetsTheRightThings` (field by field against spec §19), `ngPlusScalesEnemies`,
`claudeNeverAppearsEarly` (scan every world's areas and every `talkTo` page for `CLAUDE` except NPC_CLAUDE's own lines,
world 6 area 19 and the credits).

## Step 6: docs
design.md: world 6, the finale, NG+ — in the design doc, describe area 19 only as "the final boss (see spec §12a)" so the
surprise isn't spoiled by the README later. GDD change log `0.36`.

## Step 7: Mac self-check and PS3 check
- `make quest-maps`; play the three bosses on the Mac at the reference (`level=35 gear=5 job=3`).
- PS3: dev save at area 18 (Name-Eater) and one at area 19 with the Name-Eater's bit set; fight Claude in god mode
  through all three phases and the credits; screenshot each phase. Expect `quest: boss CLAUDE phase 2`, `phase 3`,
  `down after`, then `quest: ng+ 1` after choosing YES on the Shore. fps ≥ 59 during TOKEN RAIN with 3 subagents.
- Ask the user to fight Claude without god mode once and tell you how it felt.

## Done when
- `make test` passes, `make ps3` builds, `check_docs.py` is clean.
- On the PS3: the game can be finished; Claude offers New Game+ on the Shore, and NG+ starts with harder enemies.
