# NAME game — Game Design Document

**Version:** 0.22 · **Updated:** 2026-11-18 · [Features](#feature-index) · [Decisions](#decisions) · [Change log](#change-log)

## Summary
You pilot a small ship at the bottom of the screen against waves of pixel-art aliens. Hold R1 to spray bullets, and save the scarce R2 rocket for the moment a wave gets out of hand. Waves get harder, and every 5th wave is a boss. The run goes through five levels of 20 waves each, every one harder than the last: level 1 (space), the jungle, the glacier, the volcano and the deep sea. It ends when you beat the deep sea's wave-20 bosses or lose your last life, and you chase the saved high score. Classic plays the five levels as one run; the level picker plays any single level on its own.

| | |
|---|---|
| Genre and camera | 2D fixed-screen arcade shooter (shmup), vertical: enemies above, ship below |
| Audience | casual players with arcade instincts; pick up and play, no tutorial |
| Platforms and input | real PS3 (HEN), one DualShock 3 / Sixaxis; a second one joins for co-op |
| Session | one run is about 15–20 min per level: a level clear or game over |
| Engine | C++ with SDL2 + SDL2_mixer on PSL1GHT (see `CLAUDE.md`) |
| Scope | small homebrew release; a greybox prototype first |
| References | 80s fixed-screen shooters (alien formations that sway and dive), wave-based shmups with bosses (the waves-plus-boss rhythm and weapon upgrades). Characters, art and names are our own |

## Pillars
1. **Readable at a glance.** Chunky pixels at 320×180, enemy shots in a colour nothing else uses; a background or effect that hides a bullet is cut.
2. **The rocket is the panic button.** Bullets handle the routine and the rocket saves you, so rockets stay scarce and every one is decisive; no rocket spam.
3. **Arcade, not a campaign.** 3 lives, a score and a high score; no story, no menus beyond what the screens below need.
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
| Pixel art | implemented | MVP | 320×180 palette art: sprites, font and starfield as text grids in code | [design](features/pixel-art/design.md) |
| Screens | implemented | MVP | Main menu (Classic, Level picker, Settings), spinning level picker, pause menu, settings (music Mizrahit/Dubstep/Off, shake, flashing), high score on the title, game over | [design](features/screens/design.md) |
| High score save | implemented | MVP | Best score saved on the PS3, shown on the title | [design](features/high-score/design.md) |
| Game feel | implemented | MVP | Shake, hit-stop, flashes, particles by event tier; shake and flash options | [design](features/game-feel/design.md) |
| Co-op | implemented | MVP | A second player drops in with Start on a second controller | [design](features/co-op/design.md) |
| Level 1 | implemented | MVP | First world: waves 1-20 of today's game, ends when the wave-20 boss dies; template for later themed levels | [design](features/level-1/design.md) |
| Jungle | implemented | MVP | Level 2: insects, roaming heavy beetles, monkeys and gorillas with bananas, peels and stink mist | [design](features/jungle/design.md) |
| Glacier | implemented | MVP | Level 3: ice monsters, a gliding ship, a sneezing yeti and an ice golem whose ice blocks freeze you | [design](features/glacier/design.md) |
| Volcano | implemented | MVP | Level 4: fire monsters, an imp and a fire lord; lava drops slow you but double your bullets; eruptions | [design](features/volcano/design.md) |
| Deep sea | implemented | MVP | Level 5: scary fish, a current, a bubble-spitting anglerfish and a fisherman fish that hooks you (or throws a boot) | [design](features/deep-sea/design.md) |

## Milestones
1. **M1, greybox on the PS3:** boxes for ship, bullets, rocket and enemies; low-res scaling, music playback and button mapping proven · done when: [prototypes/ship-feel/BRIEF.md](../prototypes/ship-feel/BRIEF.md) has a RESULT line.
2. **M2, core loop (done 2026-11-17):** ship controls, rocket, enemy waves, lives and score, music and sound, pixel art · done when: a run goes from wave 1 to game over on the PS3 at 60 fps.
3. **M3, full game (done 2026-11-17):** bosses, weapon upgrades, screens, high score save, game feel · done when: title → run → game over → title works on the PS3 and the high score survives a reboot.
4. **M4, release polish (built 2026-11-17; balancing playtest and fresh-player install still open):** game name and icon, balancing to wave 20, controller unplug handling, 30-minute stability run, install guide · done when: a fresh player installs the pkg from the guide and plays a full run without help.

The task-by-task plan for each milestone is in the [work plan](work-plan/nameGame-work-plan.pdf) (source: [make_work_plan.py](work-plan/make_work_plan.py)).

## Screens and flow
```
MAIN MENU ─ CLASSIC ─→ level 1 ─→ JUNGLE ─→ GLACIER ─→ VOLCANO ─→ DEEP SEA ─ DEEP SEA CLEAR (3 s) ─→ end screen (win)
          ├ LEVEL PICKER ─→ wheel of SPACE / JUNGLE / GLACIER / VOLCANO / DEEP SEA ─→ that level only ─ <LEVEL> CLEAR ─→ end screen (win)
          └ SETTINGS
Between levels: "<LEVEL> CLEAR" for 3 s, then the next level's name banner and WAVE 1, with the same players.
Boot → MAIN MENU; PLAYING ⇄ (Start) PAUSED ─ RETURN TO MENU ─→ MAIN MENU; game over / <LEVEL> CLEAR end screen: X/Start = play again (same mode and level), Circle = MAIN MENU
```
- **Main menu:** title "NAME GAME", the items Classic, Level picker and Settings and the saved high score. Silent. Classic starts at level 1 and goes through every level.
- **Level picker:** a wheel with the picked level big and yellow in the middle and two levels above and below it smaller; up/down spins it (wrapping), the background previews the picked level, X plays just that level, Circle goes back.
- **Settings:** music Mizrahit / Dubstep / Off, screen shake 0–100 % and reduced flashing, saved to settings.txt. Circle or Back returns to the menu that opened it.
- **Game:** the HUD shows score, wave, lives and rockets. Music loops if enabled.
- **Pause:** Start opens it (Resume, Settings, Return to menu); the music keeps playing.
- **Game over / level clear:** final score and wave; the panel reads GAME OVER, or <LEVEL> CLEAR after the run's last level (deep sea in Classic, the picked level in the picker); X or Start plays again, Circle goes to the main menu.

## Direction
- **Art:** pixel art drawn at 320×180 and scaled up by a whole number (×6 at 1080p, ×4 at 720p) with no smoothing. A small fixed palette; magenta marks only deadly enemy projectiles (shots, bananas, coconuts, snowballs, icicles, fireballs, eruptions, bubbles, the boot); hazards that don't kill (peels, ice blocks, lava drops, the hook) never use it. Dark starfield background that scrolls slowly.
- **Audio:** the chosen song (Mizrahit or Dubstep, OGG copies) loops during gameplay only. Short square-wave and noise SFX for shooting, hits, explosions, pickups and death, mixed under the music.
- **Game feel:** the [game-feel](../.claude/skills/game-feel/SKILL.md) tiers: bullet hits are small, kills and pickups medium, rocket explosions, boss hits and the player's death large. Shooting itself gets almost no feedback so held R1 doesn't buzz.
- **Controls:** left stick or D-pad moves, R1 fires (hold), R2 rocket, Start pauses, X confirms in menus, Circle backs out in menus. Button numbers are measured in M1.
- **Accessibility:** shake strength 0–100 % and reduce flashing in the settings screen; large flashes at most 3 per second.

## Game-level acceptance
- 60 fps on the PS3 with 60 enemies, 100 bullets and 2 explosions on screen. *(PS3 stress test 2026-11-17 with Triangle: 60 fps held up to 60 aliens, 1,900 bullets and 8 explosions; slowest frame 11 ms of 16.7)*

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
| D-028 | 2026-11-18 | Main menu splits into CLASSIC (levels 1-5 as one run) and LEVEL PICKER (a spinning wheel; the picked level is played alone and its clear is the win) | one menu item per level, each run continuing to the end | user's request: one full run plus a way to practise any level | [screens](features/screens/design.md) |

## Open questions
- **Assumptions to confirm:** sprites as text grids in code (D-006); every number marked `[ASSUMPTION]` in the feature notes.
- **Risks:**
  - Frame rate with real sprites and many objects → M1 used ~2 ms of the 16.7 ms frame with ~45 boxes; recheck in M2 with the stress test.
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
