# nameGame — Game Design Document

**Version:** 0.7 · **Updated:** 2026-11-17 · [Features](#feature-index) · [Decisions](#decisions) · [Change log](#change-log)

## Summary
You pilot a small ship at the bottom of the screen against waves of pixel-art aliens. Hold R1 to spray bullets, and save the scarce R2 rocket for the moment a wave gets out of hand. Waves get harder, and every 5th wave is a boss. The run ends when you lose your last life, and you chase the saved high score.

| | |
|---|---|
| Genre and camera | 2D fixed-screen arcade shooter (shmup), vertical: enemies above, ship below |
| Audience | casual players with arcade instincts; pick up and play, no tutorial |
| Platforms and input | real PS3 (HEN), one DualShock 3 / Sixaxis; a second one joins for co-op |
| Session | one run is about 5–15 min; play until game over |
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
| Weapon upgrades | idea | MVP | Pickups upgrade bullets: double → spread; death loses a level | — |
| Lives and score | implemented | MVP | 3 lives, one hit kills, respawn invincibility, extra life every 50,000 | [design](features/lives-and-score/design.md) |
| Bosses | idea | MVP | A boss on every 5th wave, stronger each time | — |
| Music and sound | implemented | MVP | Your song looped during gameplay; synthesized retro SFX | [design](features/music-and-sound/design.md) |
| Pixel art | implemented | MVP | 320×180 palette art: sprites, font and starfield as text grids in code | [design](features/pixel-art/design.md) |
| Screens | idea | MVP | Title, pause (Start), game over | — |
| High score save | idea | MVP | Best score saved on the PS3, shown on the title | — |
| Game feel | idea | MVP | Shake, hit-stop, flashes, particles by event tier; shake and flash options | — |
| Co-op | in progress | MVP | A second player drops in with Start on a second controller | [design](features/co-op/design.md) |

## Milestones
1. **M1, greybox on the PS3:** boxes for ship, bullets, rocket and enemies; low-res scaling, music playback and button mapping proven · done when: [prototypes/ship-feel/BRIEF.md](../prototypes/ship-feel/BRIEF.md) has a RESULT line.
2. **M2, core loop (done 2026-11-17):** ship controls, rocket, enemy waves, lives and score, music and sound, pixel art · done when: a run goes from wave 1 to game over on the PS3 at 60 fps.
3. **M3, full game:** bosses, weapon upgrades, screens, high score save, game feel · done when: title → run → game over → title works on the PS3 and the high score survives a reboot.
4. **M4, release polish:** game name and icon, balancing to wave 20, controller unplug handling, 30-minute stability run, install guide · done when: a fresh player installs the pkg from the guide and plays a full run without help.

The task-by-task plan for each milestone is in the [work plan](work-plan/nameGame-work-plan.pdf) (source: [make_work_plan.py](work-plan/make_work_plan.py)).

## Screens and flow
`Boot → Title → Game ⇄ Pause → Game over → Title`
- **Title:** game name, high score, "Press X or Start". No music.
- **Game:** the HUD shows score, wave, lives and rockets. Music loops.
- **Pause:** Start toggles it; resume or quit to the title. Music keeps playing.
- **Game over:** final score and wave, "new high score" when it is one; any button goes back to the title.

## Direction
- **Art:** pixel art drawn at 320×180 and scaled up by a whole number (×6 at 1080p, ×4 at 720p) with no smoothing. A small fixed palette; enemy shots are the only magenta/pink objects. Dark starfield background that scrolls slowly.
- **Audio:** the user's song (an OGG copy of the FLAC) loops during gameplay only. Short square-wave and noise SFX for shooting, hits, explosions, pickups and death, mixed under the music.
- **Game feel:** the [game-feel](../.claude/skills/game-feel/SKILL.md) tiers: bullet hits are small, kills and pickups medium, rocket explosions, boss hits and the player's death large. Shooting itself gets almost no feedback so held R1 doesn't buzz.
- **Controls:** left stick or D-pad moves, R1 fires (hold), R2 rocket, Start pauses, X confirms in menus. Button numbers are measured in M1.
- **Accessibility:** shake strength 0–100 % and reduce flashing in the pause menu; large flashes at most 3 per second.

## Game-level acceptance
- 60 fps on the PS3 with 60 enemies, 100 bullets and 2 explosions on screen. *(PS3 stress test 2026-11-17 with Triangle: 60 fps held up to 60 aliens, 1,900 bullets and 8 explosions; slowest frame 11 ms of 16.7)*

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

## Open questions
- **Assumptions to confirm:** working title "nameGame"; sprites as text grids in code (D-006); every number marked `[ASSUMPTION]` in the feature notes.
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
