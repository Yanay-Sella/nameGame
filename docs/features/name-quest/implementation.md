# NAME QUEST: implementation

**Updated:** 2026-11-20 · [Design](design.md) · [Testing](testing.md)

## Approach
NAME QUEST lives in its own `source/quest*` files with no SDL or PS3 headers, so `make test` builds them on the Mac. `main.cpp` owns one `Quest`, opens it from the main menu, steps it at 60 Hz with player 1's `Actions`, plays its requested sounds, prints its log lines and draws it. The quest is one mode machine (`QuestMode`: slots, delete, class, intro, play, dead) in `questUpdate`, with one `switch`.

Movement is `moveHero`: grace timers, ladders, walking, jumping (buffered, coyote time, drop-through, the Thief's double jump), the jump cut, gravity, currents and vents, then an x move and a y move each resolved against the tiles. The reachability test runs the same function.

## Code map
| File / module | Role in this feature |
|---|---|
| `source/quest.h` | constants, area and save data, `Hero`, `Quest`, and every function the game and tests call |
| `source/quest.cpp` | `tileAt`, `loadArea`, `moveHero`, `heroInLiquid`; `questOpen`, `questUpdate`, `questSave`, `questStartDev` |
| `source/quest_areas.cpp` › `worldDef` | the world table (name, style, areas); Namula and the TEST GROUNDS tile maps |
| `source/quest_save.cpp` | `saveWrite` (tmp then rename), `saveRead`, `saveDelete` |
| `source/quest_draw.cpp` › `questDraw` | slot, class and intro screens; play: background, tiles, objects, hero, HUD, banner, shake |
| `source/quest_sprites.h` | hero frames for the four classes, checkpoint, hearth, portal, closed arch, chest |
| `source/screen.cpp` › `drawSpriteEx` | mirrored sprites with one recoloured palette character (the hero's job trim) |
| `source/game.h` › `Actions`, `Sound` | `jump`, `jumpHeld`, `item`; the `SOUND_Q_*` effects |
| `source/audio.cpp` | the `SOUND_Q_*` tones |
| `source/main.cpp` | `SCENE_QUEST`, the menu item, pause over the quest, `NAMEGAME_QUEST`, sounds, logs, the fps line |

## How main.cpp runs the quest
- The main menu's NAME QUEST item stops the music, calls `questOpen` with the EBOOT's folder and sets `quest.devAids`.
- `questUpdate` runs only in frames that started in `SCENE_QUEST`, so the press that opened the quest or resumed it from pause never also acts in it. Presses read in a frame that ran no game step carry over to the next frame.
- After every step the press fields of player 1's actions are cleared, so a press acts once.
- Start (or a lost controller) in play opens main.cpp's pause menu with `questPaused` set; the pause and settings screens draw `questDraw` behind them and never start a song. RETURN TO MENU calls `questSave`.
- `quest.logLines` are printed with `debugNetPrintf` every frame and cleared; `quest.wantsMainMenu` returns to the main menu with the NAME QUEST item selected.
- The 5-second fps line shows `quest <world>.<area>` instead of the wave and score.

## Data and config
- **Config constants:** movement numbers at the top of `quest.cpp` (spec §5); `WALK_SPEED` per class; death, banner and camera timings below them.
- **Persistence:** `<EBOOT folder>/quest1.txt` .. `quest3.txt`, key=value lines (spec §17).
- **Dev aids:** `NAMEGAME_QUEST='slot=3 class=0 world=8 area=0'` starts straight into play; L2+L3 test mode; Select warps to the next area.

## Deviations from design
- `Hero` has two fields the plan didn't list: `dropTimer` (down + X drop-through) and `jumped` (for the jump sound).
- `saveWrite` takes an optional `renameFailed` flag so `questSave` can log `rename failed, wrote directly`.
- Holding Select 2 s to go to the TEST GROUNDS isn't built: `Actions` has no held-Select field. Use `NAMEGAME_QUEST='... world=8'`.
- Vents give a net lift of 900 px/s² (gravity is cancelled, then 900 more), up to a rise of 160 px/s.
- Crumbling blocks stay solid and spikes don't hurt yet; hazards come with combat and hazards (phases 2 and 4).
- The HUD shows `LV n` and a full HP bar, without the `HP n/n` numbers, until stats come in phase 2.
- Namula's background has no hearth glow on the near walls: a glowing edge read as a platform.
- Namula's `N` spots have no `npcs[]` entries (an area holds more NPCs than `npcs[2]`); phase 3 places them.

## History
- 2026-11-20: phase 1 implemented.
