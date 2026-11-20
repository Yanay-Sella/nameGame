# Phase 1b: the art system and Namula's finished look

Sizes in this file are in dp like the spec (x2 in real px) unless they say "real px".

Read first: [`../name-quest-plan.md`](../name-quest-plan.md), then spec "Resolution" (top), §22 (all of it), §13 (the art brief rows for
Namula, 0), §21, §4 (`L` and `AreaDef::landmarks`). Phase 1 is done and committed (`138cdff`); this phase replaces its
placeholder look. Phase 2 starts after the user approves Namula's style frame (step 9).

Why this phase exists: phase 1 draws tiles as flat fills with a 2 px top line, platforms 2 px thin, and Namula's
background as two sine-wave silhouettes. The user asked for many more details: rich backgrounds, thicker platforms,
small things everywhere (leaves, stones...). Every later world is built on what this phase makes.

## What the player gets
NAME QUEST draws at 640x360, and Namula looks finished: a starry dusk sky, far sea cliffs with a lighthouse, rooftops with smoking chimneys and lanterns,
near fences and trees; cobbled ground with moss in the cracks and thick grassy caps that hang over edges; plank
platforms with rope lashings; grass, flowers, pebbles, crates and a cat scattered around; the well, market stalls and
the hearth's stone ring as landmarks; the hearth and lanterns glowing warm; a detailed 16x20 hero with outline and
16 animation frames per class. 60 fps on the PS3.

## Step 0a: speed test at 640x360 (needs the PS3; do this before anything else)
NAME QUEST moves to 640x360 (spec "Resolution", D-053): four times the pixels of today's frame. Prove the PS3 can
present it before building on it.
- Temporary dev code in `main.cpp` behind `DEV_AIDS` and an env-free trigger (hold L1 + R1 for 2 s on the main menu):
  for 300 frames, fill a 640x360 buffer with 4 full-screen layer copies (`memcpy` rows from a 1280x480 test buffer),
  2,000 16x16 tile blits through a palette, 16 lights of radius 48 dp, then `SDL_UpdateTexture` a 640x360 streaming
  texture and `SDL_RenderCopy` it to the screen at x3 (1080p) or x2 (720p). Log
  `speed test 640x360: fill %.1f ms, upload %.1f ms, present %.1f ms, fps %.1f` (worst of each).
- Run it on the PS3 (locked command, `make pad BUTTONS="l1+r1:2.2|8"`), read the log, then delete the test code.
- Pass: fps ≥ 59.5 and fill + upload ≤ 8 ms. If it fails, stop and tell the user the numbers and the two ways out
  (30 fps for NAME QUEST only, or back to 320x180); don't continue without their answer.

## Step 0b: switch NAME QUEST to 640x360 (`source/screen.h/.cpp`, `source/main.cpp`, `source/quest*.cpp`, `Makefile`, tests)
- `screen.h/.cpp`: drawing goes to a current target: `extern uint32_t* screenTarget; extern int screenW, screenH;`
  and `void screenSetTarget(uint32_t* pixels, int w, int h);`. Every function in screen.cpp (fill, sprites, text,
  `drawMenu`, `screenSaveBmp`) clips and writes to the target. `screenPixels[SCREEN_W * SCREEN_H]` stays as the
  shooter's target and the default; add `extern uint32_t questPixels[QUEST_W * QUEST_H];` with
  `const int QUEST_W = 640, QUEST_H = 360;`. `drawMenu` scales every size and its text by `u = screenW / SCREEN_W`
  (1 for the shooter, 2 over the quest), so the pause and settings menus look the same size on both.
- The shooter (`game.cpp`) keeps using `screenPixels` and `SCREEN_W/H` directly; don't change it. The shooter's tests
  must still pass untouched.
- `main.cpp`: a second streaming texture `questPicture` 640x360 and `questRect` scaled by
  `min(outputW / 640, outputH / 360)`. Before drawing a frame: `screenSetTarget(questPixels, QUEST_W, QUEST_H)` when the
  scene is QUEST or paused/settings from the quest, else `screenSetTarget(screenPixels, SCREEN_W, SCREEN_H)`. Upload and
  copy the texture that matches. The visible-pixel check and the screenshot use the current target. Clear the
  borders (both buffers) whenever the target changes, not only on the first two frames.
- `quest.h`: `const float DP = 2;` Every dp number in `quest.cpp` and `quest_draw.cpp` becomes `<value> * DP`
  (physics, walk speeds, camera lead, `HERO_W/H`, `TILE = 16`, shake, HUD and screen layouts, text scales x2). The
  camera keeps the hero around the screen centre (320, 180 real px); areas are 1280 x 368 (wide) or 640 x 720 (tall) real px.
- Tests: `quest_tests.cpp` checks in real px now (the jump peak 72 ± 2 px, etc.); write each expectation as
  `<dp value> * DP` so the spec number is visible. The reachability test needs no change beyond the constants.
- `Makefile`: `shot` accepts both BMP sizes (`SHOT_SIZE := 172854` for the shooter, `QUEST_SHOT_SIZE := 691254` =
  54 + 640*360*3); `shot` and `mac-run` convert with `--resampleWidth 1280` (x4 and x2, both whole numbers).
- Until step 5 redraws them, the phase-1 sprites are drawn at 2x with a temporary `drawSpriteScaled2` in
  `quest_draw.cpp`; delete it in step 5 (no 2x copies of 1x art in the result, spec "Resolution").
- `make test`, `make mac-run` (read a screenshot: everything twice as big and crisp, same layout), then continue.

## Step 1: palette drawing (`source/screen.h/.cpp`)
```cpp
struct WorldPalette { uint32_t c[16]; };   // a world's 16 colours: four ramps of four (spec §22)
// Like drawSpriteEx, but the sprite's '0'-'9' 'a'-'f' characters index pal ('.' transparent).
void drawSpritePal(const Sprite& sprite, int x, int y, bool flip, const WorldPalette& pal);
// drawSpriteEx plus a 1 px outline in `outline` around every opaque pixel (characters, spec §22).
void drawSpriteOutlined(const Sprite& sprite, int x, int y, bool flip, uint32_t outline, char swapFrom = 0, uint32_t swapTo = 0);
// Ordered 4x4 Bayer threshold for (x, y), 0..15: dithered gradients, glows and fog.
inline int bayer4(int x, int y) { static const int M[16] = {0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5}; return M[(y & 3) * 4 + (x & 3)]; }
```
`drawSpriteOutlined`: first pass writes the outline colour at the 4-neighbours of opaque pixels that are themselves
transparent, second pass draws the sprite (so the outline never covers it).

## Step 2: the world art data (new `source/quest_art.h`, `source/quest_art.cpp`)
Keep art separate from rules: `quest.cpp` never includes it; `quest_draw.cpp` and the tests do.
```cpp
// Autotile shapes from the 4 neighbours (bit 1 up solid, 2 right, 4 down, 8 left).
enum TileShape { SHAPE_TOP, SHAPE_TOP_LEFT, SHAPE_TOP_RIGHT, SHAPE_LEFT, SHAPE_RIGHT, SHAPE_FILL, SHAPE_BOTTOM,
                 SHAPE_BOTTOM_LEFT, SHAPE_BOTTOM_RIGHT, SHAPE_COLUMN_TOP, SHAPE_COLUMN, SHAPE_SINGLE, SHAPE_COUNT };
struct PropDef { const Sprite* frames[4]; int frameCount; float frameTime; int where; bool light; };   // where: PROP_TOP, PROP_CEILING, PROP_WALL
struct LandmarkDef { const Sprite* frames[4]; int frameCount; float frameTime; int lightX, lightY, lightRadius; uint32_t lightColor; };
struct LightTint { uint32_t color; int radius; };
// Everything a world looks like; worlds without art (count 0 sprites) keep phase 1's drawing.
struct WorldArt {
    WorldPalette palette;
    const Sprite* ground[SHAPE_COUNT][3];   // up to 3 variants; nullptr = use variant 0
    const Sprite* fringe[3];                // drawn over the cap, 1-3 px above the top line (grass blades, snow lip)
    const Sprite* drapeLeft; const Sprite* drapeRight;   // hang down the side at an edge
    const Sprite* platformLeft; const Sprite* platformMid; const Sprite* platformRight;   // 8 x 6
    const Sprite* bracket; const Sprite* rope;            // platform supports: into a wall, up to a ceiling
    const Sprite* ladder[2];                // body, top
    const Sprite* liquid[3];                // 3-frame surface line; the depth gradient is drawn in code
    const PropDef* props; int propCount;
    const LandmarkDef* landmarks; int landmarkCount;   // AreaDef::landmarks ids 1..landmarkCount
    void (*paintLayers)(uint32_t* layers[4], int w, int h);   // pre-renders L0-L3 (spec §22) with a fixed seed
    void (*addDetails)(const struct Quest& q, int camX, int camY);   // the animated details over the layers
    float propDensityTop, propDensityCeiling, propDensityWall;      // spec §22: 0.35, 0.25, 0.10 (Namula top 0.45)
};
const WorldArt* worldArt(int world);   // nullptr: no art yet (worlds 1-8 until their phase)
```
- `const int LAYER_W = 1280, LAYER_H = 480;` (real px) Layer buffers are owned by `quest_art.cpp` (static, allocated once with
  `std::vector<uint32_t>`), re-painted when the world changes (`void prepareWorldArt(int world)` called from
  `questDraw` when `q.area.world` differs from the last painted world). After painting, build the per-row opaque runs
  for L1–L3 (`struct Run { uint16_t x, len; };` per row) so drawing is `memcpy` per run.
- `void drawLayers(int world, int camX, int camY)` — L0 copied whole (its x scroll is 0; its y offset is `camY * 0.05`),
  L1–L3 at x factors 0.15, 0.35, 0.6 and y factors 0.1, 0.2, 0.35, wrapping horizontally at 1280.
- Prop placement (spec §22 rules): `void placeProps(const AreaState& area, std::vector<PlacedProp>& out)` in
  `quest_art.cpp`, run on area entry (store the result in a draw-side cache keyed by world+area, not in `Quest`, so
  rules stay art-free). Hash: `h = (world * 73856093) ^ (area * 19349663) ^ (tx * 83492791) ^ (ty * 2654435761u)`,
  then `xorshift` once. Exclusions: arrival tiles of every link into the area, `S C M N P T K Q W X L O` tiles and their
  neighbours, hazard tiles (`^ ~ z f`) within 2 tiles.

## Step 3: tiles in the new look (`source/quest_draw.cpp`)
In `drawTile`, when `worldArt(world)` exists:
- `#` (and `c`, `i` with their own overlays): compute the neighbour mask with `tileSolid`, map it to a `TileShape`,
  pick the variant with the tile hash % 3, `drawSpritePal`. Then, if the tile above is open: the `fringe` sprite at
  `sy - 3` (variant by hash). At a left/right edge of a top tile: the drape sprite hanging over the side (1–4 px into the
  neighbouring air column).
- `-`: platform sprites 6 px tall (left end, middle, right end by the neighbours), then supports: if a solid wall is
  within 2 tiles left/right at the same row, a `bracket` into it; else if a solid ceiling is within 6 tiles above, a
  `rope` column up to it at both ends; else posts (the ladder body sprite cut to 3 px wide) down to the ground within 3.
  Collision is unchanged (top line only).
- `|`, `~`: the art's ladder and liquid (the surface line animates every 0.2 s; below it a dithered gradient from the
  palette's light ramp to its dark ramp over 16 px).
- Draw order inside `drawPlay`: layers → animated details → tiles → props (behind) → landmarks → objects → NPCs/enemies
  → hero → fringes (in front of feet) → projectiles/particles → lights → vignette → HUD. Keep `drawPlay` reading top to
  bottom; each item is one call.
- Keep phase 1's flat drawing as the `else` branch for worlds without art (TEST GROUNDS stays plain on purpose: it's
  for tests).

## Step 4: lights and vignette (`source/quest_art.cpp`)
- `void applyLights(const std::vector<Light>& lights)`; `struct Light { int x, y, radius; uint32_t color; };` gathered
  each frame from lit props and landmarks on screen, liquids marked as glowing (lava; Namula: none), and the hearth.
  For each pixel within the radius: `strength = (1 - d/r)^2`, quantised to 4 steps by comparing `strength * 16` with
  `bayer4(x, y)` per step, each step adding a quarter of the light's colour (channel-wise, clamped). Nearest 16 lights.
- Vignette: a precomputed `uint8_t` mask (0–3 steps) built once; darken the playfield pixel by `step * 1/8` per channel.
  It applies in every world except 7 (it isn't a flash, so reduce-flashing doesn't turn it off).
- Dev timing: `extern double (*questClock)();` (seconds) declared in `quest.h`, set by main.cpp to
  `[]{ return SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency(); }`; when set, `drawPlay` measures
  layers, tiles+props, lights+vignette and keeps the worst of each over 5 s; main.cpp appends them to its 5-second `fps`
  line as `draw bg %.1f tiles %.1f light %.1f ms` while the scene is QUEST.

## Step 5: the hero and the existing objects (`source/quest_sprites.h`, `source/quest_draw.cpp`)
- Redraw the hero at 16 x 20 with the 16 frames of spec §22 per class (`HERO_<CLASS>_IDLE_1..2, WALK_1..4, JUMP, FALL,
  ATTACK_1..3, CLIMB_1..2, ROLL_1..2, HURT`); `HERO_FRAMES` becomes `[CLASS_COUNT][16]` with a `HeroFrame` enum of those;
  draw at `(x - 4, y - 6)` with `drawSpriteOutlined` (outline = the sprite's darkest colour per class, given as a table)
  and the job trim swap. Walk cycles at 0.1 s per frame; JUMP while rising, FALL while vy > 40; idle breath every 0.6 s.
  Cape/hair 1–2 px of secondary motion between frames (spec §22). Class select cards use IDLE_1.
- Redraw the brazier (16 x 20, unlit + 3 lit frames with a light), the hearth (24 x 28, stone ring, 3 flame frames, a
  big warm light radius 48), portal and closed arch (16 x 28 carved stone), chest (12 x 10, closed/open), all using the
  global palette with outlines.
- Delete the old 12x16 hero sprites and update `ALL_QUEST_SPRITES` in the tests.

## Step 6: Namula's art (`source/quest_art.cpp`, `source/quest_sprites.h`, `source/quest_areas.cpp`)
Spec §13 row 0 and §22. Concretely:
- Palette (author 16 colours as 4 ramps): stone (cool grey-brown), foliage (moss green), night (deep blue-violet),
  firelight (amber to warm white, used for lights and windows only).
- Layers: L0 the dusk sky (a 6-band dithered gradient from violet to deep blue, 120 stars of 3 brightnesses, a thin moon);
  L1 the sea cliffs and a lighthouse silhouette with a lit window, the sea at the horizon with 2-row wave highlights;
  L2 rooftops (8+ distinct houses with windows, some lit), chimneys, hanging lantern strings; L3 dark fence posts, a
  tree trunk with branches and laundry lines, at most 25 % coverage.
- Animated details: chimney smoke (puffs rising 6 px/s, fading by dither), lantern flicker, fireflies (10, slow
  wandering), waves moving in L1.
- Tiles: cobble top (3 variants), packed earth fill with buried stones (3 variants), the 12 shapes, grass fringe (3),
  moss drapes; planks 6 px with rope lashings; rope ladder; sea surface (3 frames).
- Props (≥ 12): grass tufts (2, swaying), small flowers (2), pebbles (2), crate, barrel, bucket, stool, a sleeping cat
  (tail animates), a dropped basket; Namula top density 0.45.
- Landmarks (≥ 6, place 1–3 per area with `L` and set `landmarks[]`): the hearth's stone ring (behind the hearth), the
  well (cliffs), market stalls with awnings (village, 2 kinds), a signpost, the statue of a nameless hero, boats pulled
  up on the shore, Tobin and Garrick's hut (shore). Lights: lanterns on stalls, the hut's window.
- Re-author Namula's three tile maps where needed so the ground has shape (steps, a raised market, a terrace under the
  cliffs, a pier on the shore) and the `L` spots have room. Keep every rule from phase 1 (the reachability test, links,
  spots) green.

## Step 7: tests (`tests/quest_tests.cpp`)
- `questSpritesHaveTheirSize` covers every new sprite; `palSpritesUseOnlyPaletteChars` (`0-9a-f.` only) for every
  sprite in every `WorldArt`.
- `everyGroundShapeHasArt` (Namula), `propsNeverBlockSpots` (no placed prop on an excluded tile, for every Namula area),
  `propPlacementIsStable` (same area twice → same list), `layersObeyTheValueRules` (paint Namula's layers, check the
  luminance caps of spec §22 per layer and that no layer pixel is `m` or `y`'s colour), `outlineSurroundsTheSprite`,
  `areaLandmarksExist` (every `L` has a non-zero id within the world's landmark count).
- `make test` passes, including the phase-1 tests.

## Step 8: Mac check
`make mac`, then screenshots of each Namula area (walk the hero across with `BUTTONS`, `l1` at 3–4 places) and the
class select. Read them at full size and fix: anything in the background that reads as a platform or a projectile,
grass hiding the hero's feet by more than 2 px, a light that blooms over the HUD, repeated patterns visible across
the screen (raise variant counts or hash spread).

## Step 9: style frame approval (needs the user)
Send the user (SendUserFile, PNGs at 1280 px wide) three Namula screenshots and the class select, with
one line each. Ask: "Is this the level of detail you want for every world?" Apply their changes and resend until they
approve. Write their notes and the approval date in `docs/features/name-quest/testing.md`. Don't start phase 2 before it.

## Step 10: docs
- `docs/features/name-quest/design.md`: an "Art" section: the §22 rules in short numbered form.
- `docs/GDD.md`: decision `D-052 | 2026-11-20 | Rich pixel art for NAME QUEST (spec §22): 16-colour world palettes, 4
  pre-rendered parallax layers, autotiled ground with thick overhanging caps, 6 px platforms, scattered props and
  landmarks, software lights, outlined 16x20 characters; enemies and bosses redrawn side-view from the shooter's designs
  (replaces D-051's reuse); each world's look approved from a style frame | phase 1's flat tiles | the user wanted
  much more detail | NAME QUEST`, and `D-053 | 2026-11-20 | NAME QUEST renders at 640x360 (x2 at 720p, x3 at 1080p) with
  every design number x2; the shooter stays 320x180 (D-004) | keep 320x180 with richer art | the user chose more pixels
  for detail | NAME QUEST, all drawing`; change log entry with the next version number: `NAME QUEST art system, Namula
  finished at 640x360 (D-052, D-053)`; bump `**Version:**` to match.
- `check_docs.py` clean.

## Step 11: PS3 check
```
scripts/ps3-lock.sh bash -c 'make install launch && (make log > /tmp/nq-log.txt &) && make pad BUTTONS="down|down|cross|1|cross|3|right:3|l1" && make shot; make pad BUTTONS="left:6|1|start|1|down|down|cross|3"; make quit'
```
Expect: the `fps` line ≥ 59.5 with `draw bg ... tiles ... light ...` and the three together ≤ 8 ms worst on the PS3,
total worst work < 12 ms. The screenshot matches the Mac's. If the budget is blown: first measure which part, then
(in this order) shrink light radii, drop L3's runs that are under 4 px, halve the prop animation rate — never drop the
art the user approved without asking them.

## Done when
- `make test` passes, `make ps3` builds, `check_docs.py` is clean.
- The user approved Namula's style frame; on the PS3 Namula runs at 60 fps with the full art.
