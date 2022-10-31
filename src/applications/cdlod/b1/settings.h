
// Also see F.Strugar's paper and code !

#pragma once

#include "glad/glad.h"
#include "omath/vec3.h"
#include <string>

namespace terrain {

namespace settings {

const GLuint HEIGHTMAP_TEXTURE_UNIT = 0;
const GLuint AABB_DRAWING_VERTEX_BUFFER_BINDING_INDEX = 0;
const GLuint GRIDMESH_VERTEX_BUFFER_BINDING_INDEX = 11;
// Skybox vertex buffer: 12
// UIOverlay vertex buffer: ??
// UIOverlay Font texture unit = 20;

/* The size of the quadtree in raster units. .y ist the height.
 * The quadtree can get very large. Its origin (usually 0,0,0) and size are defined here.
 * Must be power of two. */
const omath::uvec3 RASTER_MIN = { 0,0,0 };
const omath::uvec3 RASTER_MAX = { 4096,0,4096 };
/* Doesn't determine the absolute highst/lowest value from the heightmap for each node, but instead looks
 * up 4 cornerpoints and center height and builds bounding box from that. */
const bool FAST_TREE_GENERATION = false;
/* A multiplier to apply for the conversion between raster space and world space.
 * Can be seen as the distance between posts in m if height steps are 1m */
const float RASTER_TO_WORLD_X = 1.0f;
const float RASTER_TO_WORLD_Z = 1.0f;
// Use half floats for the heightmap textures. Faster, less, memory on the GPU, evtl. precision problems.
// FIXME CPU values are still 32bit floats.
const bool USE_HALF_FLOATS = true;
/* This is applied directly to the value reead from the heightmap data (heightmap::getHeightAt()).
 * Should correspond to raster size and raster to world conversion, or else too steep/flat the terrain. */
const float HEIGHT_FACTOR = 0.05f;
/* The size of a leaf node of the quadtree. Together with the raster size and number of levels this determines
 * how many nodes there will be. On a quadtree of size 256 (unrealistic, just an example) with 4 levels the
 * leaf nodes would have a size of 32. Also has performance implications. Many small nodes will render slower.
 * Must be power of 2 and > 2. Recommendation: keep it somewhere between 16 and 256. */
const unsigned int LEAF_NODE_SIZE = 64;
/* This times leaf node size is the raster size of one patch of terrain, the gridmesh dimension.
 * Affects performance, try out how it works best for you. Recommendation not make it larger than 1024.
 * TODO Can have good effects when used with noise textures (not implemented yet) and dynamically selecting
 * gridmeshes for rendering. But performance ... */
const unsigned int RENDER_GRID_RESULUTION_MULT = 1;
const unsigned int GRIDMESH_DIMENSION = LEAF_NODE_SIZE * RENDER_GRID_RESULUTION_MULT;
// The maximum depth of the quadtree.
const unsigned int MAX_LOD_LEVEL_COUNT = 15;
/* Number of lod levels used in this case. Big quadtrees and small node sizes require more levels.
 * Should fit the size of the quadtree and the camera depth range. Calculation of lod level ranges
 * and the ranges for the smooth transitions between them is done on their base.
 * Extreme example: If there 10 levels on a camera range of 1000 that won't work.
 * TODO implement a mechanism to show nodes that fall out of clean transition */
const unsigned int NUMBER_OF_LOD_LEVELS = 5;
// For performance, memory is reserved for so many nodes. More will not be rendered.
const unsigned int MAX_NUMBER_SELECTED_NODES = 1024;
// TODO seperate view range for LOD and camera to be able to select shorter near/far planes
// and still have LODding capabilities.
/* The minimum view range that covers clean transitions. Can be situation dependent.
 * Set higher when there are incorrect transitions and visible seams. */
const float MIN_VIEW_RANGE = 1000.0f;
// The maximum view range that can be set. Usually limited by precision problems.
const float MAX_VIEW_RANGE = 100000.0f;
/* Determines level distribution based on distance from the viewer. Value of 2.0 should result in about an equal
 * number of triangles displayed on screen (in average) for all distances. Values above 2.0 will result in
 * more triangles on more distant areas, below 2.0 in less. */
const float LOD_LEVEL_DISTANCE_RATIO = 2.0f;
/* The part of the distance over a level that is stable. 1-this is used for transitioning to the next level.
 * That is 0.66 means the first 0.66 are rendered with fixed resolution, 0.34 are used to linearly transition
 * to the next level. */
const float MORPH_START_RATIO = 0.66f;
/* Sort selection by camera distance. Can speed up rendering.
 * TODO: Sort by level first, then distance. */
const bool SORT_SELECTION = true;
// Not implemented yet
const bool SHADOW_MAP_ENABLED = false;
const unsigned int SHADOW_MAP_TEXTURE_RESOLUTION = 4096;	// 2048, 8192

}

}
