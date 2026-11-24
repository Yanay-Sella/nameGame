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
7. Falling into a pit or a deadly liquid kills at once: `YOU FADED` for 2.5 s while the hero evaporates into drifting motes of its own colours and the picture sinks to black, then the hero is back at the last rested checkpoint (Namula's hearth for now) and the slot is saved.
8. RETURN TO MENU from the pause menu saves the slot. Picking the slot again later, even after a reboot, puts the hero back at the checkpoint with the same class.
9. Namula has three calm areas (the village, the cliffs, the shore); the sea on the shore is its only danger. Its portals are closed stone arches for now.

## Rules (phase 2: combat; numbers in spec §5-§8)
10. Press R1 for one basic attack. The Knight's sword and the Thief's dagger hit a box in front during a short window. The Archer's arrows and the Magician's bolts fly until a wall or their range. Each landed basic hit gives 1 MP.
11. R2 casts the selected strong attack for its MP cost (spec §6). The empty click plays when MP is too low. Each promotion selects the new skill and keeps earlier skills available. R3 cycles from newest to oldest and wraps. Switching during a cast selects the next one. A successful R2 cancels a basic attack in progress (D-071).
12. Circle dodges: the Knight, Archer and Thief roll on the ground with brief i-frames; the Magician blinks forward, in the air too, stopping before walls. Blue mist lingers where the Magician vanished and disperses into tiny blue pixels in about 0.4 s.
13. HP never regenerates; MP regenerates slowly. The HUD shows LV, the HP and MP bars, `HP n/m` and the carried names. After the first promotion it also shows the selected R2 skill and the R3 switch control.
14. Enemies follow one of seven behaviours (walker, charger, flyer, shooter, jumper, armored, dropper); their stats come from the world's reference hero (spec §7). Walkers turn at walls and never walk into pits.
15. Hits flash the enemy white, knock fodder and soldiers back, float a number (crits orange) and briefly freeze the action on strong hits and kills. A kill bursts into particles and adds its names straight to the counter.
16. Touching an enemy, its swing or its shot, spikes, a lit electric panel or a burning fire jet hurts: knockback (spikes bounce the hero up), 1 s of blinking i-frames, a red number and a shake.
17. Dying (HP 0, a pit or a liquid) drops every carried name as a pale-yellow wisp where the hero last stood on safe ground; an older wisp is lost. Touching the wisp gives the names back (`+n NAMES`). After death ordinary enemies return and HP and MP are full.
18. The armored elite guards its front (hits from the front do a quarter) except while it winds up or recovers, or from above; a stunned charger takes 50 % more.

## Rules (phase 3: RPG systems; numbers in spec §10, §11, §14-§16)
19. Up on the ground uses the first thing touched: an NPC (within 14 dp, same floor), a chest, a lit checkpoint, a surveyor, a portal; a ladder is climbed only when none is touched. The world stops while a dialogue, menu, shop or the map is open; playtime keeps counting and Start still pauses.
20. Dialogue types at 50 characters a second with a soft blip every third; X completes the page, then turns it; Circle closes. Closing the last page with X opens what follows (a shop, Ilsa's level up, the job question, the map).
21. Touching an unlit brazier lights it (`NAME KINDLED`) and saves. Up at a lit one: REST (HP and MP full, ordinary enemies return, crumbles reset, the respawn point, a short white frame unless Reduce Flashing is on), TRAVEL (highlight a world to preview its landscape in a small, centered menu over the current area, then choose a lit checkpoint by area name; arriving rests there and keeps running buffs; the arrival fades in from black instead of the white frame), LEAVE. Namula's hearth is always lit. Every entry at a checkpoint (death, travel, loading a slot) shows 0.2 s of black, then the picture eases in over 1 s.
22. Ilsa's first three talks are an introduction; from the fourth she says one line and opens LEVEL UP (the level, cost, names and the stats before and after). It stays open, so several levels can be bought; level 40 is the last.
23. Marra sells the six items at base price x `PRICE_SCALE[highest world]` up to each carry limit (shards, Sela's locket and Garrick's gift raise them); Brann sells the class's next weapon and the next armor once the hero has entered that tier's world (tier 7 never). Rows that can't be bought are grey and click empty.
24. Triangle selects the next owned item; Square uses it. Potions take a 0.5 s drink at x0.4 walk and heal at its end (RED 35 %, BIG 70 % of max HP, MANA 50 % of max MP); WHETSTONE (ATK x1.2), IRON TONIC (damage taken x0.8) and SWIFT FEATHER (walk x1.15) run 120 s and restart if used again. Death ends them.
25. The class mentor gives tier 1 at level 5 once world 1's gate is open and tier 2 at level 15 for a FROZEN FEATHER (taken), after a YES to `BECOME <JOB>?`: full heal, the gold job banner, 30 sparks. Below the level the mentor says `COME BACK AT LEVEL n.`; other mentors talk about their craft.
26. Chests open with Up: the lid lifts, the contents float up for 1.5 s; a full bag shows `YOUR BAG IS FULL` and the chest stays shut. Opened chests stay open.
27. A world's gate opens with `THE GATE OPENS` (the gate shakes, then vanishes top to bottom); a lever hit by any attack opens its area's shortcut doors for good.
28. A surveyor says one line, then shows the world map: visited areas filled, the rest outlined, the current one blinking, lit checkpoints, closed gates, edge links as gaps.
29. (D-057 since phase 4) Namula and each world play the user's song for that place (`quest_namula.ogg`, `quest_world1.ogg`..), and the boss song (`quest_boss.ogg`) while a boss fight runs; without the song, or with Settings' music OFF, the place's ambience plays. Before: each world plays a quiet 4 s ambience loop (Namula: the hearth's crackle; TEST GROUNDS: none), also in the pause menu over the quest.

## Rules (phase 4: world 1 and the bosses; numbers in spec §9, §12, §13)
30. THE DRIFTING STATION has its own look (spec §22): deep space with a dim red sun and a ringed planet behind station spars, riveted plating with hazard stripes on every ledge, grated catwalks, cables, consoles and red lamps. Pits show open space. Its entry checkpoint is lit from the start.
31. Electric panels sit on the floor: dark, then flickering 0.4 s, then arcing for 2 s (off 1.5 s; 1 s in intense areas); touching a lit one costs 15 % of max HP. Vents show pale streaks rising through the 6 tiles they lift.
32. World 1's five enemies are redrawn side-view with 4 movement frames, a tell pose and an attack pose.
33. Entering a mini-boss or boss area whose boss is due starts the fight: the boss's name as a banner, its bar at the bottom, every exit closed (a grey shimmering wall at linked edges; portals do nothing). If the hero dies during the fight, the undefeated boss starts at full HP on the next attempt.
34. Every boss attack is telegraphed (a tell pose for 0.5 s; slams and beams mark their area with magenta dashes for 0.8 s and 1.2 s, blinking faster at the end) and followed by at least 0.6 s open. Attacks are random, never the same twice in a row. At half HP the boss roars, shakes the screen, holds an invulnerable pose for 0.8 s and switches to its phase-2 set.
35. THE OVERSEER hovers and sweeps (BURST, FAN, SLAM; phase 2 faster, plus 2 FLOATERS). THE STAR MAGE teleports between 4 anchors before each attack (BEAM, STAR FAN, 2 CRAWLERS; phase 2 two beams and an electric floor every 8 s). Summons give no names.
36. A mini-boss's death gives its names, opens the world's gate and saves. A big boss's death gives its names, shows `GREAT NAME TAKEN`, lights the next world's entry and saves. A lit brazier and a separate onward portal appear in the cleared arena. The portal names the next world and opens when its entry is playable. Defeated bosses and mini-bosses stay dead in that slot (D-072). Ilsa greets the hero differently after world 1's Great Name.
37. World 1 has 20 areas: the main path runs DOCKING RING, CARGO HOLD, CREW DECK, the maintenance shaft down, REACTOR RING, the coolant conduit up, the HANGAR (the Overseer), THE BULKHEAD (the gate), SIGNAL ROOM, MED BAY (the second checkpoint), COMMAND RING, THE LAST CORRIDOR (no enemies) and the OBSERVATORY DOME (the Star Mage). Off it: three dead ends with the VIAL SHARD, MAP PAGE 1 and STAR IRON, a side room, a loop through the server farm and cold storage, and a vent shaft. The hangar's lever opens a ladder straight up to the entry. Two surveyors: in the crew deck and the signal room.

## Rules (phase 5: worlds 2 and 3; numbers in spec §8, §9, §12, §13)
38. THE GREENDEEP has its own look: a drowned jungle swallowing an old temple, green-gold light through the canopy, giant trees and temple pillars in mist, mossy soil, branch platforms, fireflies and falling leaves. Vines are its ladders; its water is dark green; its thorns are brown spikes; its gate is a wall of thorny roots.
39. World 2's hazards: water pits early, thorns in the middle areas, coconut palms late. Passing under a palm drops its coconut 0.3 s later (20 % of max HP); the palm regrows it in 4 s. The tree monkey hangs from the ceiling, drops on a hero below and throws one banana when it lands.
40. THE MONKEY KING leaps between two trees you can climb to (BANANA ARC, PEEL TOSS; phase 2 a coconut rain marked on the ceiling). A peel on the floor slides the hero 0.5 s without control. THE SILVERBACK pounds the floor (a shockwave you jump), leaps onto the hero's spot and throws a stink that leaves a mist taking 3 % of max HP per second; phase 2 adds two stags and a second pair of waves.
41. World 2 has 20 areas: the main path runs THE EDGE OF GREEN, FERN HOLLOW, THE OLD STEPS, the sun well down, THE THORN WALK, THE DROWNED TEMPLE, THE KING'S GROVE (the Monkey King), THE THORN WALL (the gate), THE ROOT CELLAR, THE SUNKEN SHRINE (the second checkpoint), PALM TERRACES, THE STILL ROOTS (no enemies) and THE SILVER THRONE (the Silverback). Garrick sits hurt in a dead end beside a fallen statue's head and asks for a RED POTION. A lever opens a hatch from the root hoard back up to the shrine.
42. THE WHITE SILENCE has its own look: a dim blue cave under a glacier, far ice mountains and a frozen waterfall, crystals and frozen trees, snow-capped blue rock, ice-slab platforms, frozen rope ladders, snow falling at three depths. Its gate is a wall of blue ice.
43. World 3's hazards: ice floors and freezing water early; crumbling ice and icicles in the middle areas; wind late. Crumbling blocks (every world) shake 0.6 s under the hero, with cracks spreading, then break and come back 4 s later once the hero is off them. An icicle falls 0.25 s after the hero passes under it (20 %) and stays gone until a rest. Wind zones push 40 px/s and turn around every 3 s; snow streaks show which way.
44. THE YETI sneezes (`AH... AH...`, then 7 snowballs in a fan; twice in phase 2, with snow falling), belly-slides across its den, and throws an ice block that freezes the hero 1.2 s; each X or R1 press chips 0.15 s off. THE GLACIER TITAN rains 6 icicles marked on the ceiling, fires 3 ice shards, and slams: cracks run along the floor ahead of ice spikes erupting outward. In phase 2 the middle 10 tiles of its floor turn to crumbling ice over a void, and a blizzard pushes the hero, turning every 4 s.
45. World 3 has 20 areas: the main path runs THE COLD MOUTH, THE LONG DRIFT, THE FROZEN STAIR, the glacier well down, THE ICE GALLERY, THE ICE CATHEDRAL, THE YETI'S DEN, THE ICE WALL (the gate), THE BLUE HOLLOW, THE DEEP CHAPEL (the second checkpoint), THE WIND GATE, THE STILL ICE (no enemies) and THE GLACIER THRONE (the Titan, down a ladder). The FROZEN FEATHER waits behind an ice knight in a dead end above the cathedral.
46. After the Great Names of worlds 2 and 3, Ilsa greets the hero differently and Lio adds a verse about the latest one after his song.

## Rules (phase 5b: heroes and combat feel; spec §5, §6, §21, §22)
47. The four heroes are drawn on the build of the user's reference archer: big heads with 2 x 2 eyes and blush, flat shades, no outline (D-065). Each class has its own silhouette (helm and shield; ponytail and bow; wide hat and staff; scarf and mask) and each job tier adds a trim colour and one detail.
48. Every class runs, turns and jumps at full speed during R1 attacks and R2 skills (D-062, D-064). Skills with their own movement (Shadow Dash, Rising Slash's hop) keep it.
49. Each R1 strike plays nine drawings. The Knight, Magician and Thief bend their lead knee and lean into the attack. The Archer steps back with the rear foot and bends that knee while aiming forward. The front foot stays planted when standing, and running legs continue their stride (D-069). The bow and staff draw for the next shot if it is queued.
50. A melee slash is brightest exactly while it can hit; the Knight's skills draw their crescents from the same timing. Melee basic hits stop the action for 0.025 s (D-063).
51. Hero shots are sprites in class colours (wood and steel arrows, blue bolts, icy orbs, steel stars), never magenta; every hit throws a spark, gold for skills.
52. Basic hit bursts draw over enemies and bosses. Enemy shots and telegraphs draw on top of impact effects. Large hero fills are dithered, and at most 200 particles live at once.
53. The full NPC cast uses the hero art style (D-066). NPCs have two idle frames and recognizable silhouettes matched to their roles. The authoring rules for new humanoid NPCs live in spec §22.
54. Tap R1 during a basic attack to queue the next of three strikes (D-067, D-068). Holding R1 plays a single strike. Knight uses a level slash, a rising cut and an overhead chop. Archer uses a standard shot, a low reload shot and a snap shot. The Magician flicks her wand forward, sweeps a rising crescent, then pivots through a wide waist-high sweep. Each cast fires one horizontal bolt. Thief uses a straight stab, a reverse cut and a downward jab. The combo uses the attack cycles and damage in spec §5–§6.
55. Each promotion shortens the basic attack cycle and makes swings taller and longer or shots bigger, faster and longer-ranged. The hit bursts grow from class-coloured sparks to sparks with one and then two expanding arcs. See spec §6 for the tuning.
56. Adept’s Frost Orb flies as a snowflake three times the old orb’s width and height. It bursts into an expanding ice ring and shards on impact or at maximum range, damaging and slowing nearby enemies and bosses. See spec §6 for tuning (D-074).

## Dev playtesting

- In dev builds, talk to the NPC marked DEV just left of Namula's hearth or near the Proving Floor spawn. Use a spare slot for changes you want to keep separate from your main playthrough.
- Choose any of the four classes and any job tier, including the tier 3 skills. Choosing a job raises the level to its threshold if needed. The menu also grants one free level at a time, adds 10,000 Names, fills item stacks to their carry limits, refills HP and MP, and toggles invincibility for the session.
- Warp lists the worlds and areas that exist in the build. Hold Select for 2 s to return to the Proving Floor from anywhere in play.

## Edge cases
- Start does nothing on the slot, class and intro screens; it pauses in play and while `YOU FADED` shows.
- A controller unplugged in play opens the pause menu, which waits for it (D-014).
- A save that can't be renamed into place is written directly (logged); a missing save file is an empty slot.
- A crumbled block waits to come back while the hero stands in its tile. Icicles stay fallen across area changes until a rest or death.

## History
- 2026-11-20: designed with the user (D-035 to D-051); phase 1 engine built.
- 2026-11-20: phase 2 combat built (rules 10-18).
- 2026-11-21: phase 2 reviewed: R2 cancels a basic swing, spikes bounce.
- 2026-11-21: phase 3 RPG systems built (rules 19-29).
- 2026-11-22: phase 5b heroes and combat feel built (rules 47-52).
- 2026-11-21: phase 4 started: world 1's art and style frame, the boss framework, both world 1 bosses (rules 30-36).
- 2026-11-21: music from the user's songs per place plus a boss song (D-057, rule 29); the ambience hums removed.
- 2026-11-21: the user approved world 1's style frame; world 1's 20 areas built (rule 37). PS3 check and the user's playtest open.
- 2026-11-21: phase 5: worlds 2 and 3 (looks approved from their style frames), their hazards, four bosses, 40 areas, Garrick, Ilsa and Lio's lines (rules 38-46). PS3 checks and the user's playtest open.
- 2026-11-22: extended the hero style to NPCs and added three R1 attack animations per class (D-066, D-067).
- 2026-11-22: R1 combos advance with repeated presses (D-068).
- 2026-11-22: R1 gains speed, reach, hitbox size and impact effects at each promotion (D-070).
- 2026-11-22: Job promotions retain earlier R2 skills. R3 cycles through them (D-071).
- 2026-11-23: Defeated mini-bosses stay dead after rest, death and save reload (D-072).
- 2026-11-23: Added world boss arena checkpoints, onward portals and world previews for checkpoint travel (D-073).
- 2026-11-23: Reworked Adept’s Frost Orb into a larger snowflake with an area burst (D-074).
