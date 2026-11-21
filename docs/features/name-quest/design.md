# NAME QUEST

**Status:** in progress · **Updated:** 2026-11-21 · [GDD](../../GDD.md) · [Implementation](implementation.md) · [Testing](testing.md)

NAME QUEST is a second game on the main menu: a hard 2D side-scrolling action RPG set in the shooter's worlds. You wake beside the Hearth of Names in Namula, the last village, pick one of four classes and go out through six worlds of 20 hand-made areas each, earning names (the currency) from monsters, dropping them when you die and resting at checkpoints, with a hub you keep coming back to. It has its own code and its own three save slots; the shooter's pillar 3 ("arcade, not a campaign") applies to Classic and the level picker only (D-035).

Rules and numbers: see the work-plan spec [`spec.md`](../../work-plan/name-quest/spec.md) until phase 9 folds the final numbers in here. The plan's phases are in [`name-quest-plan.md`](../../work-plan/name-quest-plan.md).

## Rules (phase 1: the engine)
1. NAME QUEST on the [main menu](../screens/design.md) opens the slot screen. There is no music in NAME QUEST (D-049).
2. The slot screen shows three slots. X on an empty slot opens class select; X on a used slot puts the hero at the last rested checkpoint. Triangle on a used slot asks `DELETE SLOT n?` (NO by default); YES deletes it. Circle goes back to the main menu.
3. Class select shows KNIGHT, ARCHER, MAGICIAN and THIEF side by side with POWER, TOUGHNESS and REACH bars; left/right picks, X confirms, Circle goes back. The slot is written at once.
4. Four intro pages are typed out; X completes a page, then turns it. After the last page the hero stands beside the hearth in Namula.
5. Controls in play: walk with the stick or D-pad, X jumps (hold for a higher jump), the Thief jumps again in the air, up/down climbs ladders, down + X drops through a one-way platform, Up at an open portal goes through it, Start pauses.
6. Areas are two screens wide or tall. Walking past an open edge, or Up at an open portal, enters the next area; the first visit shows the area's name.
7. Falling into a pit or a deadly liquid kills at once: `YOU FADED` for 2.5 s, then the hero is back at the last rested checkpoint (Namula's hearth for now) and the slot is saved.
8. RETURN TO MENU from the pause menu saves the slot. Picking the slot again later, even after a reboot, puts the hero back at the checkpoint with the same class.
9. Namula has three calm areas (the village, the cliffs, the shore); the sea on the shore is its only danger. Its portals are closed stone arches for now.

## Rules (phase 2: combat; numbers in spec §5-§8)
10. R1 attacks (hold to repeat) with the class's basic attack: the Knight's sword and the Thief's dagger hit a box in front during a short window, the Archer's arrows and the Magician's bolts fly until a wall or their range. Each landed basic hit gives 1 MP.
11. R2 uses the job tier's strong attack (spec §6 table) for MP; without enough MP nothing happens and the empty click plays. Power Slash, Blade Fury and Meteor root the hero on the ground while they play. R2 cancels a basic attack that is still playing.
12. Circle dodges: the Knight, Archer and Thief roll on the ground with brief i-frames; the Magician blinks forward, in the air too, stopping before walls.
13. HP never regenerates; MP regenerates slowly. The HUD shows LV, the HP and MP bars, `HP n/m` and the carried names.
14. Enemies follow one of seven behaviours (walker, charger, flyer, shooter, jumper, armored, dropper); their stats come from the world's reference hero (spec §7). Walkers turn at walls and never walk into pits.
15. Hits flash the enemy white, knock fodder and soldiers back, float a number (crits orange) and briefly freeze the action on strong hits and kills. A kill bursts into particles and adds its names straight to the counter.
16. Touching an enemy, its swing or its shot, spikes, a lit electric panel or a burning fire jet hurts: knockback (spikes bounce the hero up), 1 s of blinking i-frames, a red number and a shake.
17. Dying (HP 0, a pit or a liquid) drops every carried name as a pale-yellow wisp where the hero last stood on safe ground; an older wisp is lost. Touching the wisp gives the names back (`+n NAMES`). After death every enemy is back and HP and MP are full.
18. The armored elite guards its front (hits from the front do a quarter) except while it winds up or recovers, or from above; a stunned charger takes 50 % more.

## Rules (phase 3: RPG systems; numbers in spec §10, §11, §14-§16)
19. Up on the ground uses the first thing touched: an NPC (within 14 dp, same floor), a chest, a lit checkpoint, a surveyor, a portal; a ladder is climbed only when none is touched. The world stops while a dialogue, menu, shop or the map is open; playtime keeps counting and Start still pauses.
20. Dialogue types at 50 characters a second with a soft blip every third; X completes the page, then turns it; Circle closes. Closing the last page with X opens what follows (a shop, Ilsa's level up, the job question, the map).
21. Touching an unlit brazier lights it (`NAME KINDLED`) and saves. Up at a lit one: REST (HP and MP full, every enemy back, crumbles reset, the respawn point, a short white frame unless Reduce Flashing is on), TRAVEL (NAMULA, then every lit checkpoint; arriving rests there and keeps running buffs), LEAVE. Namula's hearth is always lit.
22. Ilsa's first three talks are an introduction; from the fourth she says one line and opens LEVEL UP (the level, cost, names and the stats before and after). It stays open, so several levels can be bought; level 40 is the last.
23. Marra sells the six items at base price x `PRICE_SCALE[highest world]` up to each carry limit (shards, Sela's locket and Garrick's gift raise them); Brann sells the class's next weapon and the next armor once the hero has entered that tier's world (tier 7 never). Rows that can't be bought are grey and click empty.
24. Triangle selects the next owned item; Square uses it. Potions take a 0.5 s drink at x0.4 walk and heal at its end (RED 35 %, BIG 70 % of max HP, MANA 50 % of max MP); WHETSTONE (ATK x1.2), IRON TONIC (damage taken x0.8) and SWIFT FEATHER (walk x1.15) run 120 s and restart if used again. Death ends them.
25. The class mentor gives tier 1 at level 5 once world 1's gate is open and tier 2 at level 15 for a FROZEN FEATHER (taken), after a YES to `BECOME <JOB>?`: full heal, the gold job banner, 30 sparks. Below the level the mentor says `COME BACK AT LEVEL n.`; other mentors talk about their craft.
26. Chests open with Up: the lid lifts, the contents float up for 1.5 s; a full bag shows `YOUR BAG IS FULL` and the chest stays shut. Opened chests stay open.
27. A world's gate opens with `THE GATE OPENS` (the gate shakes, then vanishes top to bottom); a lever hit by any attack opens its area's shortcut doors for good.
28. A surveyor says one line, then shows the world map: visited areas filled, the rest outlined, the current one blinking, lit checkpoints, closed gates, edge links as gaps.
29. Each world plays a quiet 4 s ambience loop (Namula: the hearth's crackle; TEST GROUNDS: none), also in the pause menu over the quest.

## Edge cases
- Start does nothing on the slot, class and intro screens; it pauses in play and while `YOU FADED` shows.
- A controller unplugged in play opens the pause menu, which waits for it (D-014).
- A save that can't be renamed into place is written directly (logged); a missing save file is an empty slot.

## History
- 2026-11-20: designed with the user (D-035 to D-051); phase 1 engine built.
- 2026-11-20: phase 2 combat built (rules 10-18).
- 2026-11-21: phase 2 reviewed: R2 cancels a basic swing, spikes bounce.
- 2026-11-21: phase 3 RPG systems built (rules 19-29).
