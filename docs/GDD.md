# NAME game — Game Design Document

**Version:** 0.33 · **Updated:** 2026-11-21 · [Features](#feature-index) · [Decisions](#decisions) · [Change log](#change-log)

## Summary
You pilot a small ship at the bottom of the screen against waves of pixel-art aliens. Hold R1 to spray bullets, and save the scarce R2 rocket for the moment a wave gets out of hand. Waves get harder, and every 5th wave is a boss. The run goes through five levels of 20 waves each, every one harder than the last: level 1 (space), the jungle, the glacier, the volcano and the deep sea. It ends when you beat the deep sea's wave-20 bosses or lose your last life, and you chase the saved high score. Big bosses drop level buffs that stack until you die. Classic offers a five-level run in 1–2 Players or expanded 1–4 Players, and its Level Picker plays any single level on its own. NAME QUEST is a separate RPG platformer on the main menu, set in the same worlds, with four classes, a hub village, checkpoints and three save slots.

| | |
|---|---|
| Genre and camera | 2D fixed-screen arcade shooter (shmup), vertical: enemies above, ship below |
| Audience | casual players with arcade instincts; pick up and play, no tutorial |
| Platforms and input | real PS3 (HEN), DualShock 3 or Sixaxis, up to four local players in Classic |
| Session | one run is about 15–20 min per level: a level clear or game over |
| Engine | C++ with SDL2 + SDL2_mixer on PSL1GHT (see `CLAUDE.md`) |
| Scope | small homebrew release; a greybox prototype first |
| References | 80s fixed-screen shooters (alien formations that sway and dive), wave-based shmups with bosses (the waves-plus-boss rhythm and weapon upgrades). Characters, art and names are our own |

## Pillars
1. **Readable at a glance.** Chunky pixels and a dedicated enemy-shot colour. Backgrounds and effects keep bullets visible.
2. **The rocket is the panic button.** Bullets handle the routine and the rocket saves you, so rockets stay scarce and every one is decisive; no rocket spam.
3. **Arcade, not a campaign (Classic and the level picker).** 3 lives, a score and a high score; no story, no menus beyond what the screens below need.
4. **60 fps on a real PS3.** A feature or effect that drops frames on the console is simplified or cut.

## Core loop
```
dodge + shoot (R1) → kill aliens → score and pickups → stronger gun / more rockets → harder wave → boss
```
- **Seconds:** [ship controls](features/ship-controls/design.md), [rocket](features/rocket/design.md), dodging enemy shots.
- **Minutes:** [enemy waves](features/enemy-waves/design.md) clear and get harder; a boss every 5th wave.
- **Session and between sessions:** score against the saved high score.

## Feature index

| Feature | Status | Scope | Summary | Docs |
|---|---|---|---|---|
| Ship controls | implemented | MVP | Free movement in the bottom of the screen; hold R1 to auto-fire | [design](features/ship-controls/design.md) |
| Rocket | implemented | MVP | R2 fires a limited rocket that explodes in a large radius | [design](features/rocket/design.md) |
| Enemy waves | implemented | MVP | Alien formations that sway, shoot and dive; harder each wave | [design](features/enemy-waves/design.md) |
| Weapon upgrades | implemented | MVP | Pickups upgrade bullets: double → spread; death loses a level | [design](features/weapon-upgrades/design.md) |
| Lives and score | implemented | MVP | 3 lives, one hit kills, respawn invincibility, extra life every 50,000 | [design](features/lives-and-score/design.md) |
| Bosses | implemented | MVP | A boss on every 5th wave: classic, twin classics, a mage with a power-ball beam, twin mages | [design](features/bosses/design.md) |
| Music and sound | implemented | MVP | Two songs to choose from (Mizrahit, Dubstep) looped during gameplay; synthesized retro SFX | [design](features/music-and-sound/design.md) |
| Pixel art | implemented | MVP | 320×180 palette art: sprites, font and starfield as text grids in code; a ship skin per level | [design](features/pixel-art/design.md) |
| Screens | implemented | MVP | Main menu, Classic variant selection and controller lobby, spinning level picker, pause, settings and game over | [design](features/screens/design.md) |
| High score save | implemented | MVP | Best score saved on the PS3, shown on the title | [design](features/high-score/design.md) |
| Game feel | implemented | MVP | Shake, hit-stop, flashes, particles by event tier; shake and flash options | [design](features/game-feel/design.md) |
| Co-op | implemented | MVP | Two Classic variants with drop-in play and a larger, denser 1–4 player arena | [design](features/co-op/design.md) |
| Level 1 | implemented | MVP | First world: waves 1-20 of today's game, ends when the wave-20 boss dies; template for later themed levels | [design](features/level-1/design.md) |
| Jungle | implemented | MVP | Level 2: insects, roaming heavy beetles, monkeys and gorillas with bananas, peels and stink mist | [design](features/jungle/design.md) |
| Glacier | implemented | MVP | Level 3: ice monsters, a gliding ship, a sneezing yeti and an ice golem whose ice blocks freeze you | [design](features/glacier/design.md) |
| Level buffs | implemented | MVP | Big bosses drop a level buff (wand beam, bananas, frost wave, fire bullets, bubble shield); Square picks the strong attack; death loses all | [design](features/level-buffs/design.md) |
| Volcano | implemented | MVP | Level 4: fire monsters, an imp and a fire lord; lava drops slow you but double your bullets; eruptions | [design](features/volcano/design.md) |
| Deep sea | implemented | MVP | Level 5: scary fish, a current, a bubble-spitting anglerfish and a fisherman fish that hooks you (or throws a boot) | [design](features/deep-sea/design.md) |
| NAME QUEST | in progress | MVP | RPG platformer mode: 4 classes, 6 worlds of 20 areas, Namula hub, souls-like death | [design](features/name-quest/design.md) |

## Milestones
1. **M1, greybox on the PS3:** boxes for ship, bullets, rocket and enemies; low-res scaling, music playback and button mapping proven · done when: [prototypes/ship-feel/BRIEF.md](../prototypes/ship-feel/BRIEF.md) has a RESULT line.
2. **M2, core loop (done 2026-11-17):** ship controls, rocket, enemy waves, lives and score, music and sound, pixel art · done when: a run goes from wave 1 to game over on the PS3 at 60 fps.
3. **M3, full game (done 2026-11-17):** bosses, weapon upgrades, screens, high score save, game feel · done when: title → run → game over → title works on the PS3 and the high score survives a reboot.
4. **M4, release polish (built 2026-11-17; balancing playtest and fresh-player install still open):** game name and icon, balancing to wave 20, controller unplug handling, 30-minute stability run, install guide · done when: a fresh player installs the pkg from the guide and plays a full run without help.

The task-by-task plan for each milestone is in the [work plan](work-plan/nameGame-work-plan.pdf) (source: [make_work_plan.py](work-plan/make_work_plan.py)).

## Screens and flow
```
MAIN MENU ─┬ CLASSIC ─┬ 1–2 PLAYERS ─────────────────→ five-world run
           │          ├ 1–4 PLAYERS ─ CONTROLLERS ─→ five-world run
           │          └ LEVEL PICKER ─→ wheel of SPACE / JUNGLE / GLACIER / VOLCANO / DEEP SEA ─→ that level only ─ <LEVEL> CLEAR ─→ end screen (win)
           ├ NAME QUEST ─→ SLOTS ─ empty slot → CLASS SELECT → INTRO → PLAY in Namula; used slot → PLAY at the last checkpoint
           │               PLAY ⇄ (Start) PAUSED ─ RETURN TO MENU (saves the slot) ─→ MAIN MENU; SLOTS: Circle → MAIN MENU
           └ SETTINGS
Five-world run: level 1 → JUNGLE → GLACIER → VOLCANO → DEEP SEA → end screen (win).
Between levels: "<LEVEL> CLEAR" for 3 s, then the next level's name banner and WAVE 1, with the same players.
Boot → MAIN MENU; PLAYING ⇄ (Start) PAUSED ─ RETURN TO MENU ─→ MAIN MENU; game over / <LEVEL> CLEAR end screen: X/Start = play again (same mode and level), Circle = MAIN MENU
```
- The **main menu** is silent and shows the title "NAME GAME", the items Classic, NAME QUEST and Settings, and the saved high score.
- In **Classic**, either player-count option plays all five levels. Level Picker opens a wheel with the picked level big and yellow in the middle, two smaller rows above it and two below it. Up and Down spin the wheel, and the background previews the selection. X plays that level and Circle returns to Classic.
- **Settings:** music Mizrahit / Dubstep / Off, screen shake 0–100 % and reduced flashing, saved to settings.txt. Circle or Back returns to the menu that opened it.
- **Game:** the HUD shows score, wave, lives and rockets. Music loops if enabled.
- **Pause:** Start opens it (Resume, Settings, Return to menu); the music keeps playing.
- **Game over / level clear:** final score and wave; the panel reads GAME OVER, or <LEVEL> CLEAR after the run's last level (deep sea in Classic, the picked level in the picker); X or Start plays again, Circle goes to the main menu.

## Direction
- **Art:** the shooter uses a 320×180 picture for 1–2 Players and the Level Picker, and 384×216 for Classic 1–4 Players. Both use nearest-neighbour scaling. A small fixed palette; magenta marks only deadly enemy projectiles (shots, bananas, coconuts, snowballs, icicles, fireballs, eruptions, bubbles, the boot); hazards that don't kill (peels, ice blocks, lava drops, the hook) never use it. Dark starfield background that scrolls slowly.
- **Audio:** the chosen song (Mizrahit or Dubstep, OGG copies) loops during gameplay only. Short square-wave and noise SFX for shooting, hits, explosions, pickups and death, mixed under the music.
- **Game feel:** the [game-feel](../.claude/skills/game-feel/SKILL.md) tiers: bullet hits are small, kills and pickups medium, rocket explosions, boss hits and the player's death large. Shooting itself gets almost no feedback so held R1 doesn't buzz.
- **Controls:** left stick or D-pad moves, R1 fires (hold), R2 strong attack (rocket or a buff's), Square picks it, Start pauses, X confirms in menus, Circle backs out in menus. Button numbers are measured in M1.
- **Accessibility:** shake strength 0–100 % and reduce flashing in the settings screen; large flashes at most 3 per second.

## Game-level acceptance
- 60 fps on the PS3 with 60 enemies, 100 bullets and 2 explosions on screen. *(In the historical PS3 stress run on 2026-11-17, the game held 60 fps with up to 60 aliens, 1,900 bullets and 8 explosions. The slowest frame took 11 ms of the 16.7 ms budget.)*

- No frame drops or memory growth over 30 minutes. *(PS3 soak 2026-11-17, about 21 min not 30: autopilot to wave 50, fps ≥ 59.9, worst work 6 ms, heap 5,709 → 5,709 KB)*

## Decisions

| ID | Date | Decision | Alternatives considered | Why | Affects |
|---|---|---|---|---|---|
| D-001 | 2026-11-17 | Original alien enemies; nothing taken from Chicken Invaders | chickens; bugs; robots | the user chose aliens; we don't copy another game's characters | [enemy waves](features/enemy-waves/design.md) |
| D-002 | 2026-11-17 | Waves with a boss every 5th wave, endless until game over | endless without bosses; fixed levels | user's choice | [enemy waves](features/enemy-waves/design.md) |
| D-003 | 2026-11-17 | Rockets are ammo (start 3) refilled by pickups; area explosion | recharge timer; unlimited; column pierce; screen clear | user's choice; keeps the rocket scarce (pillar 2) | [rocket](features/rocket/design.md) |
| D-004 | 2026-11-17 | Logical resolution 320×180 | 480×270; flat shapes at full HD | user chose chunky pixels; 320×180 scales by a whole number to both 720p (×4) and 1080p (×6), 480×270 doesn't fit 720p | all drawing |
| D-005 | 2026-11-17 | Music as OGG Vorbis | keep FLAC; MP3 | the image's SDL2_mixer is built with OGG and WAV only (checked with `nm`) | Music and sound |
| D-006 | 2026-11-17 | Sprites, font and SFX defined in code, no image or sound files except the song | PNG files through libpng | no SDL2_image in the image; tiny pixel art is easy to edit as text grids [ASSUMPTION] | all drawing, Music and sound |
| D-007 | 2026-11-17 | Single player, but players are an array with their own controller | hard-coded single ship; co-op now | user wants co-op to be easy to add later without extra work now | [ship controls](features/ship-controls/design.md) |
| D-008 | 2026-11-17 | Draw the 320×180 picture in a CPU pixel buffer, then one scaled `SDL_RenderCopy` to the screen | SDL render target; scaled sprite textures | the port's `SDL_SetRenderTarget` is a no-op and `SDL_RenderCopy` has no alpha blending (read in SDL_PSL1GHT's renderer source); the scaled copy is a hardware blit with nearest-neighbour | all drawing |
| D-009 | 2026-11-17 | Co-op as drop-in join: runs start with one player, player 2 presses Start on a second controller to join | two players by default; a player-count menu | the user wanted to add co-op now, with a player joining instead of two players by default | [co-op](features/co-op/design.md), [lives and score](features/lives-and-score/design.md) |
| D-010 | 2026-11-17 | A main menu with game modes (only Classic for now) and a Settings screen reachable from the main and pause menus; music on/off saved to settings.txt | title screen with "press X" only | the user wanted room for more modes and settings | [screens](features/screens/design.md), [music and sound](features/music-and-sound/design.md), [co-op](features/co-op/design.md) |
| D-011 | 2026-11-17 | No hit-stop on ordinary alien kills; hit-stop only on rocket blasts, the boss death and the player's death | hit-stop on every kill (the medium tier) | spread fire kills several aliens a second and would stutter; the large events keep their weight | [game feel](features/game-feel/design.md) |
| D-012 | 2026-11-17 | The game is called "NAME game" ("NAME GAME" in the uppercase pixel fonts); the repo, app ID NMGM00001 and file names keep nameGame | ROCKET PANIC; STARSWARM; HIVE STORM | user's choice; a new app ID would install a second game and orphan the saved high score and settings | [screens](features/screens/design.md) |
| D-013 | 2026-11-17 | Difficulty keeps climbing to wave 20: enemy shots 70→90 px/s over waves 11–20, two red and two blue rows from wave 13, two divers at once from wave 15 | keep the wave-12 caps and only retune | every value was capped by wave 12–15, so waves 13–19 played alike | [enemy waves](features/enemy-waves/design.md) |
| D-014 | 2026-11-17 | A lost controller pauses the run; after it reconnects the pause menu stays up and the player resumes | resume at once; 3-2-1 countdown | user's choice: the player resumes when ready | [ship controls](features/ship-controls/design.md), [co-op](features/co-op/design.md), [screens](features/screens/design.md) |
| D-015 | 2026-11-17 | make release builds without the dev aids (Triangle stress, Select autopilot, L1 screenshot), logs to 127.0.0.1 and includes the song | one build for everything | a player pressing Triangle would spawn 60 aliens; the user may share the song | [music and sound](features/music-and-sound/design.md) |
| D-016 | 2026-11-17 | Claude drives PS3 tests remotely: launch (`/play.ps3?NMGM00001`) and quit (`/xmb.ps3$exit`) through webMAN, input through webMAN's virtual pad, screenshots from a dev-only L1 dump of the game's pixel buffer | the user launches and presses buttons; an in-game UDP input listener; webMAN's built-in screenshot | all three webMAN commands worked on the PS3; its own screenshot saved nothing | [music and sound](features/music-and-sound/design.md) |
| D-017 | 2026-11-17 | A run is level 1: killing the wave-20 boss shows LEVEL 1 CLEAR for 3 s, then the end screen, and the high score is saved; enemy shots stay, so losing the last life during the banner is still game over; replaces D-002's endless run | endless waves; clear enemy shots at the kill; lock the win at the kill | user's choice: a level structure for later worlds; the finale stays dangerous to the end | [level 1](features/level-1/design.md), [bosses](features/bosses/design.md), [screens](features/screens/design.md), [high score](features/high-score/design.md) |
| D-018 | 2026-11-17 | Music setting rotates MIZRAHIT, DUBSTEP, OFF; both songs are loaded into memory at boot; settings.txt keeps music=0/1 and adds 2 | music on/off with one song | the user wanted a second song to pick | [music and sound](features/music-and-sound/design.md), [screens](features/screens/design.md) |
| D-019 | 2026-11-17 | Boss line-up per boss wave: 5 one classic (100 HP), 10 two classics (100 HP each), 15 one mage (200 HP), 20 two mages (200 HP each); twins sway in their own half at the full attack rate each; the mage attacks in random order and gaps, and its power ball charges 1.5 s at the hat tip, then beams down that column | one boss with growing HP (100 + 50 per appearance); twins at half rate; ball above the nearest ship | user's choice: each boss wave should feel different | [bosses](features/bosses/design.md), [level 1](features/level-1/design.md) |
| D-020 | 2026-11-18 | Level 2 is the jungle: beating level 1 continues the run into it; JUNGLE on the main menu starts there; play again restarts at the menu's level | separate mode only; reached only through level 1 | user's choice; unlocking it through level 1 may come later | [jungle](features/jungle/design.md), [level 1](features/level-1/design.md), [screens](features/screens/design.md) |
| D-021 | 2026-11-18 | Jungle waves use level 1's numbers for wave n + 5 and add wave / 8 roaming heavy beetles (12 HP) | same curve as level 1; start at wave 11 | user's choice: gentler than the level-1 finale, harder by its end | [jungle](features/jungle/design.md), [enemy waves](features/enemy-waves/design.md) |
| D-022 | 2026-11-18 | Stink mist: a player's bullets heal gorillas while that player's ship is in a mist when the bullet hits; rockets always damage | bullets marked when fired; rockets heal too | simplest rule that matches "enter the mist, your shots heal"; the rocket stays the panic button | [jungle](features/jungle/design.md), [bosses](features/bosses/design.md) |
| D-023 | 2026-11-18 | Magenta marks every deadly enemy projectile (shots, bananas, coconuts); harmless peels, poop and mist never use it | bananas without magenta | keeps pillar 1's "magenta = danger" | [jungle](features/jungle/design.md) |
| D-024 | 2026-11-18 | Levels 3-5 are the glacier, the volcano and the deep sea; each level's clear continues the run into the next, the deep sea's clear is the win; each has its own main-menu item | separate modes; one more level only | user's request ("3 more levels" PDF); names, order and rosters chosen by the planner while the user slept | [glacier](features/glacier/design.md), [volcano](features/volcano/design.md), [deep sea](features/deep-sea/design.md), [screens](features/screens/design.md) |
| D-025 | 2026-11-18 | Difficulty ladder: formation waves play level 1's numbers 5, 8, 11 and 14 waves ahead in levels 2-5, heavies every 8, 6, 5, 4 waves (at most 3), enemy shots +0, 5, 10, 15 px/s, boss HP and rewards rise per level | only the wave offset (it caps out by wave 20); more HP on formation aliens | the level-1 formulas cap at wave 20, so later levels needed their own knobs; heavier counts and faster shots keep each level distinct | [enemy waves](features/enemy-waves/design.md), [bosses](features/bosses/design.md) |
| D-026 | 2026-11-18 | Non-lethal hazards: the ice block freezes 1.6 s (R1 mashing breaks out sooner), the lava drop scorches 3 s (0.6× speed, 2× bullet damage, checked when the bullet hits), the hook lifts 24 px and stuns 1.6 s and ignores blinking ships | lethal ice blocks; scorch from the moment of firing; hook on any ship | the PDF's rules; mashing gives the frozen player something to do; a respawning ship shouldn't be hooked straight away | [glacier](features/glacier/design.md), [volcano](features/volcano/design.md), [deep sea](features/deep-sea/design.md) |
| D-027 | 2026-11-18 | Level-wide twists: the ship glides on the glacier's ice and the deep sea's current pushes it sideways; the volcano's twist is its lava drops | none; twists on every level | small rule changes make each world feel different without new controls | [glacier](features/glacier/design.md), [deep sea](features/deep-sea/design.md), [ship controls](features/ship-controls/design.md) |
| D-028 | 2026-11-18 | Main menu splits into CLASSIC (levels 1-5 as one run) and LEVEL PICKER (a spinning wheel; the picked level is played alone and its clear is the win) → picker location superseded by D-056 | one menu item per level, each run continuing to the end | user's request: one full run plus a way to practise any level | [screens](features/screens/design.md) |
| D-029 | 2026-11-18 | Playtest fixes: gold player-2 ship; bigger mage hat and a wand whose star casts the power ball; brown, bulky gorilla; 24 px see-through ice blocks for both glacier bosses; the imp's lava drip is one big drop straight down; moving ships trail bubbles in the deep sea; the fisherman throws live fish (2 a throw, at most 3 out) that chase the nearest ship | ice blocks bigger for the yeti only; an aimed big drop; fish that flop and spit or hop | user's playtest notes and choices | [co-op](features/co-op/design.md), [bosses](features/bosses/design.md), [jungle](features/jungle/design.md), [glacier](features/glacier/design.md), [volcano](features/volcano/design.md), [deep sea](features/deep-sea/design.md) |
| D-030 | 2026-11-18 | Level buffs: big bosses (waves 15, 20) drop a falling pickup; buffs stack, all lost on death; Square cycles the strong attack; beam 30 per enemy, frost 3 s (+25 % HP), bubbles 2 per hit, volcano fire rate only | wave 20 only; given automatically; all strong attacks at once or newest wins | user's PDF and choices; scarce rockets and the death loss keep pillar 2 | [level buffs](features/level-buffs/design.md), [rocket](features/rocket/design.md), [bosses](features/bosses/design.md), [ship controls](features/ship-controls/design.md) |
| D-031 | 2026-11-18 | A ship skin per level on the same 16×14 body and hitbox; player 2 keeps a gold copy | recolour only | user's PDF; same hitbox keeps dodging fair | [pixel art](features/pixel-art/design.md), [co-op](features/co-op/design.md), [level buffs](features/level-buffs/design.md) |
| D-032 | 2026-11-18 | Beams grow from their ball in 0.1 s, spark, and end in shards; the eruption grows too; beams, the frost wave and the shield pop shake the screen; the player's beam hits when it reaches the top | beams appearing at once; kick-only shake; hit-stop on the shield pop | user's request: beams felt flat and trauma 0.3 rounded to no visible shake | [level buffs](features/level-buffs/design.md), [bosses](features/bosses/design.md), [volcano](features/volcano/design.md), [game feel](features/game-feel/design.md) |
| D-033 | 2026-11-18 | Upgrades drop at 1 % and level n takes n parts, shown by a HUD gauge and a pop-up; formation waves pick a random shape and entry; difficulty waves run 1-20 in level 1 and step back 15 at each new level (to 21-40 in the deep sea), adding HP, sway, divers and a bob past wave 15 | keep 3 % drops; one fixed formation; per-level offsets 5, 8, 11, 14 | user's request: rarer, earned upgrades; varied waves; a classic run that eases off at each new world then climbs higher | [weapon upgrades](features/weapon-upgrades/design.md), [enemy waves](features/enemy-waves/design.md) |
| D-034 | 2026-11-19 | Formation waves also pick from 29 traced shapes (lines, X, heart, <>, circles, zigzag, letters, words...) drawn by spacing the wave's aliens along strokes; a shape that doesn't fit the alien count is redrawn | only the 6 grid shapes; fixed bitmaps with their own alien counts | user's request for many more shapes; keeping 16-40 aliens per wave leaves the difficulty untouched | [enemy waves](features/enemy-waves/design.md) |
| D-035 | 2026-11-20 | NAME QUEST is a separate RPG platformer on the main menu with its own code and saves; the shooter's pillar 3 ("arcade, not a campaign") applies to Classic and the level picker only | a story mode inside the shooter; a separate homebrew app | user's choice | [NAME QUEST](features/name-quest/design.md) |
| D-036 | 2026-11-20 | 6 worlds in a fixed order: the 5 shooter themes, then The Nameless Citadel; 20 areas each; a secret world unlocked by a quest | free world order; fewer, larger worlds | user's choice | [NAME QUEST](features/name-quest/design.md) |
| D-037 | 2026-11-20 | 3 save slots with autosave (rest, level up, purchase, quest step, boss kill, quit); no manual save | one slot; manual saves at checkpoints | user's choice | [NAME QUEST](features/name-quest/design.md) |
| D-038 | 2026-11-20 | Death drops all carried names where you died; touching them recovers them; dying again first loses them | keep names on death; lose them for good | user's choice | [NAME QUEST](features/name-quest/design.md) |
| D-039 | 2026-11-20 | Healing only from bought potions; resting at a checkpoint heals fully and respawns enemies; big bosses stay dead, mini-bosses respawn | regenerating HP; enemies that stay dead | user's choice | [NAME QUEST](features/name-quest/design.md) |
| D-040 | 2026-11-20 | Max level 40, no stat points; jobs at levels 5, 15, 30 through the class mentor | stat points to spend; a higher level cap | user's choice | [NAME QUEST](features/name-quest/design.md) |
| D-041 | 2026-11-20 | The strong attack costs MP | a cooldown only | user's choice | [NAME QUEST](features/name-quest/design.md) |
| D-042 | 2026-11-20 | Controls: X jump, R1 attack (hold to repeat), R2 strong attack, Circle dodge, Square drink potion, Triangle switch potion, Up talk / portal / climb, Start pause | attack on Square or X; jump on a shoulder button | user's choice | [NAME QUEST](features/name-quest/design.md) |
| D-043 | 2026-11-20 | Areas are 2 screens: 80x23 tiles wide or 40x45 tiles tall (8 px tiles); exits at the edges and through portals | single-screen rooms; long scrolling levels | user's choice | [NAME QUEST](features/name-quest/design.md) |
| D-044 | 2026-11-20 | Areas are hand-made ASCII tile maps, checked by a physics-based reachability test | generated areas; an in-game level editor | user's choice | [NAME QUEST](features/name-quest/design.md) |
| D-045 | 2026-11-20 | Single player only | drop-in co-op like the shooter | user's choice | [NAME QUEST](features/name-quest/design.md) |
| D-046 | 2026-11-20 | Pits and deadly liquids kill instantly | pits cost HP and put you back at the edge | user's choice | [NAME QUEST](features/name-quest/design.md) |
| D-047 | 2026-11-20 | A normal enemy hit takes 20 % of the reference hero's max HP; enemy stats derive from a reference hero per world | hand-tuned stats per enemy | user's choice | [NAME QUEST](features/name-quest/design.md) |
| D-048 | 2026-11-20 | The final boss is Claude, never hinted before the citadel's last room; after the credits Claude waits in Namula and offers New Game+ | the Name-Eater as the last boss; no New Game+ | user's choice | [NAME QUEST](features/name-quest/design.md) |
| D-049 | 2026-11-20 | No music in NAME QUEST for now: synthesized ambience per world and sound effects | the shooter's songs; a song per world | user's choice | [NAME QUEST](features/name-quest/design.md) |
| D-050 | 2026-11-20 | Each world's mini-boss holds the key to the gate in front of the world's second half | no gates; keys found in chests | user's choice | [NAME QUEST](features/name-quest/design.md) |
| D-051 | 2026-11-20 | Worlds 1–5 reuse the shooter's enemy and boss sprites where the theme matches (the RPG is set in the same worlds) | all-new sprites | user's choice | [NAME QUEST](features/name-quest/design.md) |
| D-055 | 2026-11-21 | Classic has a 1–2 player variant and an expanded 1–4 player variant with a three-pad entry gate and its own high score | one scale and balance for every party | user requested more room and denser waves for larger parties | [Co-op](features/co-op/design.md), [Screens](features/screens/design.md) |
| D-056 | 2026-11-21 | Put Level Picker under Classic and remove the shooter stress test | Level Picker on the main menu and a Triangle stress-test shortcut | the user grouped all shooter play under Classic and requested removal of the test | [Screens](features/screens/design.md), [Ship controls](features/ship-controls/implementation.md) |

## Open questions
- **Assumptions to confirm:** sprites as text grids in code (D-006); every number marked `[ASSUMPTION]` in the feature notes.
- **Risks:**
  - Check frame rate during dense waves with real sprites on the PS3. M1 used about 2 ms of the 16.7 ms frame budget with about 45 boxes.
  - R2, Triangle and Start button numbers are from the SDL port's source, not yet seen in a PS3 log (left, right and R1 are confirmed).

## Change log
**0.1 (2026-11-17):** first draft from the user's answers.
**0.2 (2026-11-17):** M1 done, prototype kept ([brief](../prototypes/ship-feel/BRIEF.md)); D-008 logged.
**0.3 (2026-11-17):** M2 started: lives and score, music and sound, pixel art designed.
**0.4 (2026-11-17):** XMB icon and backdrop drawn from the game's sprites ([make_xmb_art.py](xmb-art/make_xmb_art.py)); the M4 icon task can reuse it once the name is final.
**0.5 (2026-11-17):** M2 done: six features implemented with implementation and testing notes; stress test passed; the song now plays from memory (restart freeze fix to confirm).
**0.6 (2026-11-17):** co-op built as drop-in join (D-009); Co-op moves from later to MVP.
**0.7 (2026-11-17):** restart freeze fixed on the PS3 (music restart 1 ms, was ~350 ms); co-op join confirmed with a second controller.
**0.8 (2026-11-17):** main menu, pause menu and settings (D-010).
**0.9 (2026-11-17):** weapon upgrades implemented.
**0.10 (2026-11-17):** bosses implemented.
**0.11 (2026-11-17):** high score save implemented.
**0.12 (2026-11-17):** game feel implemented (D-011).
**0.13 (2026-11-17):** M3 done: screens and co-op confirmed on the PS3, M3 numbers confirmed in play.
**0.14 (2026-11-17):** M4 built: name, unplug pause, difficulty to wave 20, release build and install guide (D-012 to D-015).
**0.15 (2026-11-17):** M4 checked on the PS3: unplug pause (single-player and co-op) and a 21-minute soak confirmed; balancing playtest and fresh-player install still open.
**0.16 (2026-11-17):** remote test workflow: make launch, quit, pad and shot (D-016).
**0.17 (2026-11-17):** level 1 designed: waves 1-20 of the current game, ending on the wave-20 boss.
**0.18 (2026-11-17):** level 1 ending implemented: the wave-20 boss ends the run with LEVEL 1 CLEAR (D-017).
**0.19 (2026-11-17):** second song: music setting MIZRAHIT / DUBSTEP / OFF (D-018).
**0.20 (2026-11-17):** boss line-up: classic, twin classics, mage, twin mages (D-019).
**0.21 (2026-11-18):** jungle level (D-020 to D-023).
**0.22 (2026-11-18):** glacier, volcano and deep sea levels (D-024 to D-027).
**0.23 (2026-11-18):** main menu split into Classic and a spinning level picker (D-028).
**0.24 (2026-11-18):** playtest fixes: ships, mage, gorilla, ice, lava, deep sea (D-029).
**0.25 (2026-11-18):** level buffs and a ship skin per level (D-030, D-031).
**0.26 (2026-11-18):** beam build-up, shards and strong-attack shake (D-032).
**0.27 (2026-11-18):** upgrade parts and gauge, random wave patterns, classic difficulty ramp (D-033).
**0.28 (2026-11-19):** 29 traced formation shapes (D-034).
**0.29 (2026-11-20):** NAME QUEST designed (D-035 to D-051); phase 1 engine.
**0.30 (2026-11-20):** NAME QUEST combat: classes, job skills, enemies, damage, death and the name echo.
**0.31 (2026-11-21):** Classic variants and live controller lobby (D-055).
**0.32 (2026-11-21):** NAME QUEST RPG systems: checkpoints and travel, Namula's people, levels, shops, items and buffs, jobs, chests, gates, shortcuts, the map, ambience.
**0.33 (2026-11-21):** NAME QUEST world 1, THE DRIFTING STATION: its look (approved from the style frame), 20 areas, five redrawn enemies, electric panels and vents, the boss framework, THE OVERSEER and THE STAR MAGE; world 2's entry lights when the Star Mage falls.
