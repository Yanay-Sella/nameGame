// NAME QUEST's art system (spec §22): world palettes, pre-rendered parallax layers, autotiles, props, landmarks, lights.
// Drawing-side only: quest.cpp never includes it, so the rules stay art-free.
#pragma once
#include "quest.h"
#include "screen.h"
#include <vector>

// Autotile shapes from the 4 neighbours (bit 1 up solid, 2 right, 4 down, 8 left).
enum TileShape { SHAPE_TOP, SHAPE_TOP_LEFT, SHAPE_TOP_RIGHT, SHAPE_LEFT, SHAPE_RIGHT, SHAPE_FILL, SHAPE_BOTTOM,
                 SHAPE_BOTTOM_LEFT, SHAPE_BOTTOM_RIGHT, SHAPE_COLUMN_TOP, SHAPE_COLUMN, SHAPE_SINGLE, TILE_SHAPE_COUNT };   // (the shooter's game.h has a SHAPE_COUNT)
enum PropWhere { PROP_TOP, PROP_CEILING, PROP_WALL };
// weight: how often this prop is picked among the props of its `where` (grass more often than a cat).
struct PropDef { const Sprite* frames[4]; int frameCount; float frameTime; int where; bool light; int weight; };
struct LandmarkDef { const Sprite* frames[4]; int frameCount; float frameTime; int lightX, lightY, lightRadius; uint32_t lightColor; };
struct LightTint { uint32_t color; int radius; };
// Everything a world looks like; worlds without art (worldArt nullptr) keep phase 1's flat drawing.
struct WorldArt {
    WorldPalette palette;
    const Sprite* ground[TILE_SHAPE_COUNT][3];   // up to 3 variants; nullptr = use variant 0
    const Sprite* fringe[3];                // drawn over the cap, 1-3 dp above the top line (grass blades, snow lip)
    const Sprite* drapeLeft; const Sprite* drapeRight;   // hang down the side at an edge
    const Sprite* platformLeft; const Sprite* platformMid; const Sprite* platformRight;   // 8 x 6 dp
    const Sprite* bracket; const Sprite* rope; const Sprite* post;   // platform supports: into a wall, up to a ceiling, down to the ground
    const Sprite* ladder[2];                // body, top
    const Sprite* liquid[3];                // 3-frame surface line; the depth gradient is drawn in code
    const PropDef* props; int propCount;
    const LandmarkDef* landmarks; int landmarkCount;   // AreaDef::landmarks ids 1..landmarkCount
    void (*paintLayers)(uint32_t* layers[4], int w, int h);   // pre-renders L0-L3 (spec §22) with a fixed seed
    void (*addDetails)(const Quest& q, int camX, int camY);   // the animated details over the layers
    float propDensityTop, propDensityCeiling, propDensityWall;      // spec §22: 0.35, 0.25, 0.10 (Namula top 0.45)
    LightTint propLight;                    // the glow of props marked light
    const Sprite* pit;                      // drawn on air in the bottom 2 rows over a pit (world 1's space), nullptr: none
};
const WorldArt* worldArt(int world);   // nullptr: no art yet (worlds 1-8 until their phase)

const int LAYER_W = 1280, LAYER_H = 480;   // real px; layers wrap horizontally
void prepareWorldArt(int world);           // paints the world's layers once (again only when the world changes)
const uint32_t* layerPixels(int layer);    // the painted layer, LAYER_W x LAYER_H, 0 = transparent (L1-L3)
void drawLayers(int world, int camX, int camY);

// A scattered prop: its kind (index in WorldArt::props), its tile, and its sprite's top-left in area px.
struct PlacedProp { int kind, tx, ty, x, y; bool flip; };
void placeProps(const AreaState& area, std::vector<PlacedProp>& out);   // deterministic: hash of world, area, tile
bool propTileExcluded(const AreaState& area, int tx, int ty);           // spec §22: near spots, objects and hazards

struct Light { int x, y, radius; uint32_t color; };   // screen px; color is added in 4 dithered quarter steps
void applyLights(const std::vector<Light>& lights, int nearX, int nearY);   // the 16 nearest to (nearX, nearY)
void applyVignette();                                                     // corners darker by dither (not world 7)
