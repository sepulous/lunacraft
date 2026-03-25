
#include <vector>
#include <cstdlib>

#include <glm/glm.hpp>

#include "chunk.h"
#include "constants.h"
#include "simplex_noise.h"
#include "helpers.h"
#include "rng.h"
#include "chunk_generation.h"
#include "moon.h"

//
// Helpers
//

void GenerateCrystal(BlockID *, RNG &, float, int, int, int, int, BlockID, int, int);

struct TreeBlock
{
    BlockID block;
    int local_x;
    int local_y;
    int local_z;

    TreeBlock(BlockID block, int local_x, int local_y, int local_z) : block(block), local_x(local_x), local_y(local_y), local_z(local_z) {}
};

std::vector<std::vector<TreeBlock>> GREEN_LIGHT_TREE_SHAPES = {
    {
        TreeBlock(BlockID::moon_leaf, 8, 0, 0), // max extent

        TreeBlock(BlockID::moon_leaf, 0, 1, 1), TreeBlock(BlockID::moon_leaf, 0, 2, 1), TreeBlock(BlockID::moon_leaf, -1, 1, 1), TreeBlock(BlockID::moon_leaf, -1, 2, 1),
        TreeBlock(BlockID::moon_leaf, 1, 1, 1), TreeBlock(BlockID::moon_leaf, 1, 2, 1), TreeBlock(BlockID::moon_leaf, 2, 2, 1), TreeBlock(BlockID::moon_leaf, 1, 2, 2),
        TreeBlock(BlockID::moon_leaf, 0, 1, 2), TreeBlock(BlockID::moon_leaf, -1, 2, 2), TreeBlock(BlockID::moon_leaf, -1, 3, 2), TreeBlock(BlockID::moon_leaf, -1, 3, 1),
        TreeBlock(BlockID::moon_leaf, -2, 4, 1), TreeBlock(BlockID::moon_leaf, -2, 4, 2), TreeBlock(BlockID::moon_leaf, -2, 3, 2), TreeBlock(BlockID::moon_leaf, -2, 3, 1),
        TreeBlock(BlockID::moon_leaf, -3, 5, 1), TreeBlock(BlockID::moon_leaf, -3, 5, 2), TreeBlock(BlockID::moon_leaf, -4, 6, 2), TreeBlock(BlockID::moon_leaf, -4, 5, 2),
        TreeBlock(BlockID::moon_leaf, -5, 7, 2), TreeBlock(BlockID::moon_leaf, -6, 7, 2), TreeBlock(BlockID::moon_leaf, -7, 7, 2), TreeBlock(BlockID::moon_leaf, -7, 6, 2),
        TreeBlock(BlockID::moon_leaf, -7, 6, 1), TreeBlock(BlockID::moon_leaf, 2, 3, 1), TreeBlock(BlockID::moon_leaf, 2, 3, 2), TreeBlock(BlockID::moon_leaf, 2, 4, 1),
        TreeBlock(BlockID::moon_leaf, 2, 4, 2), TreeBlock(BlockID::moon_leaf, 3, 5, 2), TreeBlock(BlockID::moon_leaf, 3, 5, 1), TreeBlock(BlockID::moon_leaf, 4, 5, 1),
        TreeBlock(BlockID::moon_leaf, 4, 6, 1), TreeBlock(BlockID::moon_leaf, 5, 6, 1), TreeBlock(BlockID::moon_leaf, 6, 6, 1), TreeBlock(BlockID::moon_leaf, 6, 5, 1),
        TreeBlock(BlockID::moon_leaf, 6, 5, 2),

        TreeBlock(BlockID::light, 7, 3, 2), TreeBlock(BlockID::light, 6, 3, 2), TreeBlock(BlockID::light, 7, 3, 1), TreeBlock(BlockID::light, 6, 3, 1),
        TreeBlock(BlockID::light, 7, 4, 2), TreeBlock(BlockID::light, 6, 4, 2), TreeBlock(BlockID::light, 7, 4, 1), TreeBlock(BlockID::light, 6, 4, 1),
        TreeBlock(BlockID::light, -8, 4, 0), TreeBlock(BlockID::light, -8, 5, 0), TreeBlock(BlockID::light, -7, 4, 0), TreeBlock(BlockID::light, -7, 5, 0),
        TreeBlock(BlockID::light, -7, 4, 1), TreeBlock(BlockID::light, -7, 5, 1), TreeBlock(BlockID::light, -8, 5, 1)
    },
    {
        TreeBlock(BlockID::moon_leaf, 9, 0, 0), // max extent

        TreeBlock(BlockID::moon_leaf, 1, 1, 0), TreeBlock(BlockID::moon_leaf, 1, 1, 1), TreeBlock(BlockID::moon_leaf, 1, 1, -1), TreeBlock(BlockID::moon_leaf, 1, 2, 1),
        TreeBlock(BlockID::moon_leaf, 1, 2, -1), TreeBlock(BlockID::moon_leaf, 2, 1, 1), TreeBlock(BlockID::moon_leaf, 2, 1, 0), TreeBlock(BlockID::moon_leaf, 2, 2, 1),
        TreeBlock(BlockID::moon_leaf, 2, 2, 0), TreeBlock(BlockID::moon_leaf, 1, 2, 2), TreeBlock(BlockID::moon_leaf, 2, 2, 2), TreeBlock(BlockID::moon_leaf, 1, 3, 2),
        TreeBlock(BlockID::moon_leaf, 2, 3, 2), TreeBlock(BlockID::moon_leaf, 2, 4, 2), TreeBlock(BlockID::moon_leaf, 1, 4, 3), TreeBlock(BlockID::moon_leaf, 2, 4, 3),
        TreeBlock(BlockID::moon_leaf, 2, 5, 3), TreeBlock(BlockID::moon_leaf, 1, 5, 3), TreeBlock(BlockID::moon_leaf, 2, 5, 4), TreeBlock(BlockID::moon_leaf, 1, 5, 4),
        TreeBlock(BlockID::moon_leaf, 1, 6, 4), TreeBlock(BlockID::moon_leaf, 1, 6, 5), TreeBlock(BlockID::moon_leaf, 1, 6, 6), TreeBlock(BlockID::moon_leaf, 0, 6, 6),
        TreeBlock(BlockID::moon_leaf, 0, 5, 6), TreeBlock(BlockID::moon_leaf, 0, 5, 7), TreeBlock(BlockID::moon_leaf, 1, 2, -2), TreeBlock(BlockID::moon_leaf, 1, 3, -2),
        TreeBlock(BlockID::moon_leaf, 1, 4, -2), TreeBlock(BlockID::moon_leaf, 2, 2, -1), TreeBlock(BlockID::moon_leaf, 2, 3, -1), TreeBlock(BlockID::moon_leaf, 2, 3, -2),
        TreeBlock(BlockID::moon_leaf, 2, 4, -2), TreeBlock(BlockID::moon_leaf, 2, 5, -2), TreeBlock(BlockID::moon_leaf, 2, 4, -3), TreeBlock(BlockID::moon_leaf, 2, 5, -3),
        TreeBlock(BlockID::moon_leaf, 2, 6, -3), TreeBlock(BlockID::moon_leaf, 3, 5, -3), TreeBlock(BlockID::moon_leaf, 3, 6, -3), TreeBlock(BlockID::moon_leaf, 3, 6, -4),
        TreeBlock(BlockID::moon_leaf, 2, 6, -4), TreeBlock(BlockID::moon_leaf, 3, 7, -4), TreeBlock(BlockID::moon_leaf, 2, 7, -4), TreeBlock(BlockID::moon_leaf, 3, 7, -5),
        TreeBlock(BlockID::moon_leaf, 2, 7, -5), TreeBlock(BlockID::moon_leaf, 3, 6, -6), TreeBlock(BlockID::moon_leaf, 3, 7, -6), TreeBlock(BlockID::moon_leaf, 4, 6, -6),
        TreeBlock(BlockID::moon_leaf, 4, 6, -7), TreeBlock(BlockID::moon_leaf, 3, 6, -7), TreeBlock(BlockID::moon_leaf, 4, 5, -7), TreeBlock(BlockID::moon_leaf, 4, 5, -8),

        TreeBlock(BlockID::light, 5, 3, -8), TreeBlock(BlockID::light, 5, 3, -9), TreeBlock(BlockID::light, 4, 3, -8), TreeBlock(BlockID::light, 4, 3, -9),
        TreeBlock(BlockID::light, 5, 4, -8), TreeBlock(BlockID::light, 5, 4, -9), TreeBlock(BlockID::light, 4, 4, -8), TreeBlock(BlockID::light, 4, 4, -9),
        TreeBlock(BlockID::light, -1, 3, 8), TreeBlock(BlockID::light, -1, 3, 7), TreeBlock(BlockID::light, 0, 3, 8), TreeBlock(BlockID::light, 0, 3, 7),
        TreeBlock(BlockID::light, -1, 4, 8), TreeBlock(BlockID::light, -1, 4, 7), TreeBlock(BlockID::light, 0, 4, 8), TreeBlock(BlockID::light, 0, 4, 7)
    },
    {
        TreeBlock(BlockID::moon_leaf, 11, 0, 0), // max extent

        // TODO: Lower max extent to 7
        // x: -6 to 8  ->  -7 to 7
        // z: -11 to 3  ->  -7 to 7

        TreeBlock(BlockID::moon_leaf, 1, 1, 0), TreeBlock(BlockID::moon_leaf, 1, 2, 0), TreeBlock(BlockID::moon_leaf, 1, 3, 0), TreeBlock(BlockID::moon_leaf, 1, 1, -1),
        TreeBlock(BlockID::moon_leaf, 1, 2, -1), TreeBlock(BlockID::moon_leaf, 1, 3, -1), TreeBlock(BlockID::moon_leaf, 0, 1, -1), TreeBlock(BlockID::moon_leaf, 0, 2, -1),
        TreeBlock(BlockID::moon_leaf, 0, 3, -1), TreeBlock(BlockID::moon_leaf, -1, 4, -1), TreeBlock(BlockID::moon_leaf, -1, 3, -1), TreeBlock(BlockID::moon_leaf, -1, 2, -1),
        TreeBlock(BlockID::moon_leaf, 2, 3, 0), TreeBlock(BlockID::moon_leaf, 2, 4, 0), TreeBlock(BlockID::moon_leaf, 2, 3, -1), TreeBlock(BlockID::moon_leaf, 2, 4, -1),
        TreeBlock(BlockID::moon_leaf, 2, 5, -1), TreeBlock(BlockID::moon_leaf, 3, 5, -1), TreeBlock(BlockID::moon_leaf, 3, 5, 0), TreeBlock(BlockID::moon_leaf, 3, 6, 0),
        TreeBlock(BlockID::moon_leaf, 4, 6, 0), TreeBlock(BlockID::moon_leaf, 4, 7, 0), TreeBlock(BlockID::moon_leaf, 5, 7, 0), TreeBlock(BlockID::moon_leaf, 6, 7, 0),
        TreeBlock(BlockID::moon_leaf, 6, 6, 0), TreeBlock(BlockID::moon_leaf, 7, 6, 0), TreeBlock(BlockID::moon_leaf, -1, 3, 0), TreeBlock(BlockID::moon_leaf, -1, 3, -2),
        TreeBlock(BlockID::moon_leaf, -1, 4, -2), TreeBlock(BlockID::moon_leaf, -2, 4, -1), TreeBlock(BlockID::moon_leaf, -2, 4, 0), TreeBlock(BlockID::moon_leaf, -2, 5, 0),
        TreeBlock(BlockID::moon_leaf, -3, 5, 0), TreeBlock(BlockID::moon_leaf, -3, 6, 0), TreeBlock(BlockID::moon_leaf, -3, 5, -1), TreeBlock(BlockID::moon_leaf, -4, 5, -1),
        TreeBlock(BlockID::moon_leaf, -4, 6, -1), TreeBlock(BlockID::moon_leaf, -4, 6, 0), TreeBlock(BlockID::moon_leaf, -4, 7, 0), TreeBlock(BlockID::moon_leaf, -5, 6, 0),
        TreeBlock(BlockID::moon_leaf, -5, 7, 0), TreeBlock(BlockID::moon_leaf, -5, 7, 1), TreeBlock(BlockID::moon_leaf, -6, 7, 0), TreeBlock(BlockID::moon_leaf, -6, 7, 1),
        TreeBlock(BlockID::moon_leaf, -6, 7, 2), TreeBlock(BlockID::moon_leaf, 1, 1, -2), TreeBlock(BlockID::moon_leaf, 1, 1, -3), TreeBlock(BlockID::moon_leaf, 0, 1, -2),
        TreeBlock(BlockID::moon_leaf, 0, 1, -3), TreeBlock(BlockID::moon_leaf, 1, 2, -2), TreeBlock(BlockID::moon_leaf, 1, 2, -3), TreeBlock(BlockID::moon_leaf, 0, 2, -2),
        TreeBlock(BlockID::moon_leaf, 0, 2, -3), TreeBlock(BlockID::moon_leaf, 0, 2, -4), TreeBlock(BlockID::moon_leaf, 1, 3, -3), TreeBlock(BlockID::moon_leaf, 0, 3, -4),
        TreeBlock(BlockID::moon_leaf, 1, 3, -4), TreeBlock(BlockID::moon_leaf, 0, 4, -4), TreeBlock(BlockID::moon_leaf, 1, 4, -4), TreeBlock(BlockID::moon_leaf, 1, 4, -5),
        TreeBlock(BlockID::moon_leaf, 1, 5, -5), TreeBlock(BlockID::moon_leaf, 1, 6, -5), TreeBlock(BlockID::moon_leaf, 0, 4, -5), TreeBlock(BlockID::moon_leaf, 0, 5, -5),
        TreeBlock(BlockID::moon_leaf, 0, 6, -6), TreeBlock(BlockID::moon_leaf, 1, 6, -6), TreeBlock(BlockID::moon_leaf, 1, 7, -6), TreeBlock(BlockID::moon_leaf, 1, 7, -7),
        TreeBlock(BlockID::moon_leaf, 0, 7, -7), TreeBlock(BlockID::moon_leaf, 1, 8, -7), TreeBlock(BlockID::moon_leaf, 1, 8, -9), TreeBlock(BlockID::moon_leaf, 1, 7, -9),
        TreeBlock(BlockID::moon_leaf, 1, 7, -10), TreeBlock(BlockID::moon_leaf, 0, 7, -9), TreeBlock(BlockID::moon_leaf, 0, 7, -10), TreeBlock(BlockID::moon_leaf, -1, 7, -10),
        TreeBlock(BlockID::moon_leaf, 0, 7, -8), TreeBlock(BlockID::moon_leaf, 1, 7, -8), TreeBlock(BlockID::moon_leaf, 1, 8, -8),

        TreeBlock(BlockID::light, 0, 5, -11), TreeBlock(BlockID::light, 0, 6, -11), TreeBlock(BlockID::light, 0, 5, -10), TreeBlock(BlockID::light, 0, 6, -10),
        TreeBlock(BlockID::light, -1, 5, -11), TreeBlock(BlockID::light, -1, 6, -11), TreeBlock(BlockID::light, -1, 6, -10), TreeBlock(BlockID::light, -5, 5, 3),
        TreeBlock(BlockID::light, -5, 5, 2), TreeBlock(BlockID::light, -6, 5, 3), TreeBlock(BlockID::light, -6, 5, 2), TreeBlock(BlockID::light, -5, 6, 3),
        TreeBlock(BlockID::light, -5, 6, 2), TreeBlock(BlockID::light, -6, 6, 3), TreeBlock(BlockID::light, -6, 6, 2), TreeBlock(BlockID::light, 8, 4, 0),
        TreeBlock(BlockID::light, 8, 4, 1), TreeBlock(BlockID::light, 7, 4, 1), TreeBlock(BlockID::light, 7, 5, 1), TreeBlock(BlockID::light, 7, 5, 0),
        TreeBlock(BlockID::light, 8, 5, 0)
    },
    {
        TreeBlock(BlockID::moon_leaf, 9, 0, 0), // max extent

        // x: -5 to 11  ->  -8 to 8 TreeBlock(DONE)
        // z: -10 to 7  ->  -9 to 8 TreeBlock(DONE)

        TreeBlock(BlockID::moon_leaf, -2, 1, 1), TreeBlock(BlockID::moon_leaf, -2, 2, 1), TreeBlock(BlockID::moon_leaf, -2, 3, 1), TreeBlock(BlockID::moon_leaf, -2, 4, 1),
        TreeBlock(BlockID::moon_leaf, -3, 1, 0), TreeBlock(BlockID::moon_leaf, -2, 1, -1), TreeBlock(BlockID::moon_leaf, -1, 1, -1), TreeBlock(BlockID::moon_leaf, -1, 1, 2),
        TreeBlock(BlockID::moon_leaf, 0, 1, 1), TreeBlock(BlockID::moon_leaf, 0, 1, 0), TreeBlock(BlockID::moon_leaf, -1, 1, 1), TreeBlock(BlockID::moon_leaf, -1, 2, 1),
        TreeBlock(BlockID::moon_leaf, -2, 1, 0), TreeBlock(BlockID::moon_leaf, -2, 2, 0), TreeBlock(BlockID::moon_leaf, -2, 3, 0), TreeBlock(BlockID::moon_leaf, -2, 4, 0),
        TreeBlock(BlockID::moon_leaf, -2, 5, 0), TreeBlock(BlockID::moon_leaf, -1, 4, -2), TreeBlock(BlockID::moon_leaf, -2, 4, -2), TreeBlock(BlockID::moon_leaf, -2, 4, -1),
        TreeBlock(BlockID::moon_leaf, -1, 4, -1), TreeBlock(BlockID::moon_leaf, -1, 1, 0), TreeBlock(BlockID::moon_leaf, -1, 2, 0), TreeBlock(BlockID::moon_leaf, -1, 3, 0),
        TreeBlock(BlockID::moon_leaf, -1, 4, 1), TreeBlock(BlockID::moon_leaf, 0, 3, 0), TreeBlock(BlockID::moon_leaf, 0, 4, 0), TreeBlock(BlockID::moon_leaf, 0, 5, 0),
        TreeBlock(BlockID::moon_leaf, 0, 5, -1), TreeBlock(BlockID::moon_leaf, 0, 6, -1), TreeBlock(BlockID::moon_leaf, -2, 5, -2), TreeBlock(BlockID::moon_leaf, 1, 5, 0),
        TreeBlock(BlockID::moon_leaf, 1, 6, 0), TreeBlock(BlockID::moon_leaf, 1, 7, 0), TreeBlock(BlockID::moon_leaf, 1, 6, -1), TreeBlock(BlockID::moon_leaf, 1, 7, -1),
        TreeBlock(BlockID::moon_leaf, 2, 7, 0), TreeBlock(BlockID::moon_leaf, 2, 8, 0), TreeBlock(BlockID::moon_leaf, 2, 8, -1), TreeBlock(BlockID::moon_leaf, 2, 9, -1),
        TreeBlock(BlockID::moon_leaf, 3, 8, -1), TreeBlock(BlockID::moon_leaf, 3, 9, -1), TreeBlock(BlockID::moon_leaf, 4, 9, -1), TreeBlock(BlockID::moon_leaf, 5, 9, -1),
        TreeBlock(BlockID::moon_leaf, 6, 9, -1), TreeBlock(BlockID::moon_leaf, 6, 8, -1), TreeBlock(BlockID::moon_leaf, 7, 8, -1), TreeBlock(BlockID::moon_leaf, 7, 7, -1),
        TreeBlock(BlockID::moon_leaf, 8, 7, -1), TreeBlock(BlockID::moon_leaf, 7, 7, 0), TreeBlock(BlockID::moon_leaf, -1, 4, 2), TreeBlock(BlockID::moon_leaf, -1, 5, 2),
        TreeBlock(BlockID::moon_leaf, -1, 6, 2), TreeBlock(BlockID::moon_leaf, -1, 7, 2), TreeBlock(BlockID::moon_leaf, -2, 5, 2), TreeBlock(BlockID::moon_leaf, -2, 6, 2),
        TreeBlock(BlockID::moon_leaf, -2, 7, 2), TreeBlock(BlockID::moon_leaf, -2, 7, 3), TreeBlock(BlockID::moon_leaf, -2, 8, 3), TreeBlock(BlockID::moon_leaf, -1, 8, 3),
        TreeBlock(BlockID::moon_leaf, -1, 8, 4), TreeBlock(BlockID::moon_leaf, -1, 8, 5), TreeBlock(BlockID::moon_leaf, -1, 8, 6), TreeBlock(BlockID::moon_leaf, -1, 7, 6),
        TreeBlock(BlockID::moon_leaf, 0, 7, 6), TreeBlock(BlockID::moon_leaf, -1, 7, 7), TreeBlock(BlockID::moon_leaf, -1, 6, 7), TreeBlock(BlockID::moon_leaf, 0, 6, 6),
        TreeBlock(BlockID::moon_leaf, -3, 5, -1), TreeBlock(BlockID::moon_leaf, -3, 6, -1), TreeBlock(BlockID::moon_leaf, -3, 6, 0), TreeBlock(BlockID::moon_leaf, -4, 7, -1),
        TreeBlock(BlockID::moon_leaf, -4, 8, -1), TreeBlock(BlockID::moon_leaf, -4, 7, 0), TreeBlock(BlockID::moon_leaf, -5, 8, 0), TreeBlock(BlockID::moon_leaf, -5, 9, 0),
        TreeBlock(BlockID::moon_leaf, -5, 9, -1), TreeBlock(BlockID::moon_leaf, -6, 9, 0), TreeBlock(BlockID::moon_leaf, -7, 9, 0), TreeBlock(BlockID::moon_leaf, -1, 5, -3),
        TreeBlock(BlockID::moon_leaf, -1, 5, -3), TreeBlock(BlockID::moon_leaf, -2, 5, -3), TreeBlock(BlockID::moon_leaf, -1, 6, -3), TreeBlock(BlockID::moon_leaf, -2, 6, -3),
        TreeBlock(BlockID::moon_leaf, -2, 6, -4), TreeBlock(BlockID::moon_leaf, -2, 7, -4), TreeBlock(BlockID::moon_leaf, -1, 7, -4), TreeBlock(BlockID::moon_leaf, -1, 8, -5),
        TreeBlock(BlockID::moon_leaf, -1, 8, -6), TreeBlock(BlockID::moon_leaf, -2, 8, -5), TreeBlock(BlockID::moon_leaf, -2, 8, -6), TreeBlock(BlockID::moon_leaf, -2, 8, -7),
        TreeBlock(BlockID::moon_leaf, -1, 7, -7), TreeBlock(BlockID::moon_leaf, -1, 6, -8), TreeBlock(BlockID::moon_leaf, -1, 7, -8), TreeBlock(BlockID::moon_leaf, -2, 7, -8),
        TreeBlock(BlockID::moon_leaf, -1, 6, -9), TreeBlock(BlockID::moon_leaf, -2, 6, -9),

        TreeBlock(BlockID::light, -7, 7, 0), TreeBlock(BlockID::light, -8, 7, 0), TreeBlock(BlockID::light, -7, 7, 1), TreeBlock(BlockID::light, -8, 7, 1),
        TreeBlock(BlockID::light, -7, 8, 0), TreeBlock(BlockID::light, -8, 8, 0), TreeBlock(BlockID::light, -7, 8, 1), TreeBlock(BlockID::light, -8, 8, 1),
        TreeBlock(BlockID::light, -1, 4, -9), TreeBlock(BlockID::light, -2, 4, -9), TreeBlock(BlockID::light, -1, 4, -8), TreeBlock(BlockID::light, -2, 4, -8),
        TreeBlock(BlockID::light, -1, 5, -9), TreeBlock(BlockID::light, -2, 5, -9), TreeBlock(BlockID::light, -1, 5, -8), TreeBlock(BlockID::light, -2, 5, -8),
        TreeBlock(BlockID::light, 7, 5, 0), TreeBlock(BlockID::light, 7, 5, -1), TreeBlock(BlockID::light, 8, 5, -1), TreeBlock(BlockID::light, 7, 6, 0),
        TreeBlock(BlockID::light, 8, 6, 0), TreeBlock(BlockID::light, 7, 6, -1), TreeBlock(BlockID::light, 8, 6, -1), TreeBlock(BlockID::light, 0, 4, 8),
        TreeBlock(BlockID::light, -1, 4, 8), TreeBlock(BlockID::light, 0, 4, 7), TreeBlock(BlockID::light, -1, 4, 7), TreeBlock(BlockID::light, -1, 5, 8),
        TreeBlock(BlockID::light, 0, 5, 7), TreeBlock(BlockID::light, -1, 5, 7)
    },
    {
        TreeBlock(BlockID::moon_leaf, 8, 0, 0), // max extent

        // x: -7 to 9  ->  -8 to 8 TreeBlock(DONE)
        // z: -7 to 8

        TreeBlock(BlockID::moon_leaf, 0, 2, 0), TreeBlock(BlockID::moon_leaf, 0, 3, 0), TreeBlock(BlockID::moon_leaf, 0, 4, 0), TreeBlock(BlockID::moon_leaf, -1, 2, 0),
        TreeBlock(BlockID::moon_leaf, -1, 3, 0), TreeBlock(BlockID::moon_leaf, -1, 4, 0), TreeBlock(BlockID::moon_leaf, -1, 1, -1), TreeBlock(BlockID::moon_leaf, -1, 2, -1),
        TreeBlock(BlockID::moon_leaf, -1, 3, -1), TreeBlock(BlockID::moon_leaf, -1, 4, -1), TreeBlock(BlockID::moon_leaf, 1, 3, 0), TreeBlock(BlockID::moon_leaf, 1, 4, 0),
        TreeBlock(BlockID::moon_leaf, 1, 3, -1), TreeBlock(BlockID::moon_leaf, 1, 4, -1), TreeBlock(BlockID::moon_leaf, 0, 1, -1), TreeBlock(BlockID::moon_leaf, 0, 2, -1),
        TreeBlock(BlockID::moon_leaf, 0, 3, -1), TreeBlock(BlockID::moon_leaf, 0, 2, -2), TreeBlock(BlockID::moon_leaf, 0, 3, -2), TreeBlock(BlockID::moon_leaf, 0, 4, -2),
        TreeBlock(BlockID::moon_leaf, -1, 2, -2), TreeBlock(BlockID::moon_leaf, -1, 3, -2), TreeBlock(BlockID::moon_leaf, -1, 4, -2), TreeBlock(BlockID::moon_leaf, -1, 5, -2),
        TreeBlock(BlockID::moon_leaf, 0, 4, -3), TreeBlock(BlockID::moon_leaf, 0, 5, -3), TreeBlock(BlockID::moon_leaf, -1, 3, -3), TreeBlock(BlockID::moon_leaf, -1, 4, -3),
        TreeBlock(BlockID::moon_leaf, -1, 5, -3), TreeBlock(BlockID::moon_leaf, -1, 6, -3), TreeBlock(BlockID::moon_leaf, 0, 5, -4), TreeBlock(BlockID::moon_leaf, -1, 5, -4),
        TreeBlock(BlockID::moon_leaf, 0, 6, -4), TreeBlock(BlockID::moon_leaf, -1, 6, -4), TreeBlock(BlockID::moon_leaf, 0, 5, -5), TreeBlock(BlockID::moon_leaf, -1, 5, -5),
        TreeBlock(BlockID::moon_leaf, -1, 4, -5), TreeBlock(BlockID::moon_leaf, -1, 3, -6), TreeBlock(BlockID::moon_leaf, -1, 4, -6), TreeBlock(BlockID::moon_leaf, -2, 4, 0),
        TreeBlock(BlockID::moon_leaf, -2, 4, 1), TreeBlock(BlockID::moon_leaf, -2, 5, 0), TreeBlock(BlockID::moon_leaf, -2, 5, 1), TreeBlock(BlockID::moon_leaf, -3, 5, 1),
        TreeBlock(BlockID::moon_leaf, -3, 6, 1), TreeBlock(BlockID::moon_leaf, -4, 6, 1), TreeBlock(BlockID::moon_leaf, -5, 6, 1), TreeBlock(BlockID::moon_leaf, -4, 6, 2),
        TreeBlock(BlockID::moon_leaf, -5, 6, 2), TreeBlock(BlockID::moon_leaf, -5, 5, 2), TreeBlock(BlockID::moon_leaf, -6, 5, 2), TreeBlock(BlockID::moon_leaf, -6, 5, 1),
        TreeBlock(BlockID::moon_leaf, -6, 4, 2), TreeBlock(BlockID::moon_leaf, -7, 4, 2), TreeBlock(BlockID::moon_leaf, -7, 3, 2), TreeBlock(BlockID::moon_leaf, -7, 2, 2),
        TreeBlock(BlockID::moon_leaf, -7, 1, 2), TreeBlock(BlockID::moon_leaf, -8, 1, 2), TreeBlock(BlockID::moon_leaf, -8, 2, 2), TreeBlock(BlockID::moon_leaf, -8, 1, 3),
        TreeBlock(BlockID::moon_leaf, 0, 3, 1), TreeBlock(BlockID::moon_leaf, 0, 4, 1), TreeBlock(BlockID::moon_leaf, 0, 5, 1), TreeBlock(BlockID::moon_leaf, -1, 4, 1),
        TreeBlock(BlockID::moon_leaf, -1, 5, 1), TreeBlock(BlockID::moon_leaf, -1, 6, 2), TreeBlock(BlockID::moon_leaf, -1, 6, 3), TreeBlock(BlockID::moon_leaf, -1, 6, 4),
        TreeBlock(BlockID::moon_leaf, 0, 6, 2), TreeBlock(BlockID::moon_leaf, 0, 6, 3), TreeBlock(BlockID::moon_leaf, 0, 7, 3), TreeBlock(BlockID::moon_leaf, -1, 7, 3),
        TreeBlock(BlockID::moon_leaf, -1, 7, 4), TreeBlock(BlockID::moon_leaf, -1, 7, 5), TreeBlock(BlockID::moon_leaf, -1, 8, 5), TreeBlock(BlockID::moon_leaf, -1, 8, 6),
        TreeBlock(BlockID::moon_leaf, -1, 8, 7), TreeBlock(BlockID::moon_leaf, -1, 7, 7), TreeBlock(BlockID::moon_leaf, 0, 7, 7), TreeBlock(BlockID::moon_leaf, 2, 4, 0),
        TreeBlock(BlockID::moon_leaf, 2, 4, -1), TreeBlock(BlockID::moon_leaf, 2, 5, 0), TreeBlock(BlockID::moon_leaf, 2, 5, -1), TreeBlock(BlockID::moon_leaf, 2, 6, 0),
        TreeBlock(BlockID::moon_leaf, 2, 6, -1), TreeBlock(BlockID::moon_leaf, 3, 6, -1), TreeBlock(BlockID::moon_leaf, 3, 7, -1), TreeBlock(BlockID::moon_leaf, 3, 7, 0),
        TreeBlock(BlockID::moon_leaf, 4, 7, 0), TreeBlock(BlockID::moon_leaf, 4, 7, -1), TreeBlock(BlockID::moon_leaf, 5, 7, -1), TreeBlock(BlockID::moon_leaf, 4, 8, -1),
        TreeBlock(BlockID::moon_leaf, 5, 8, -1), TreeBlock(BlockID::moon_leaf, 6, 8, -1), TreeBlock(BlockID::moon_leaf, 6, 9, -1), TreeBlock(BlockID::moon_leaf, 7, 8, -1),
        TreeBlock(BlockID::moon_leaf, 7, 8, -2),

        TreeBlock(BlockID::light, 0, 5, 8), TreeBlock(BlockID::light, -1, 5, 8), TreeBlock(BlockID::light, 0, 5, 7), TreeBlock(BlockID::light, -1, 5, 7),
        TreeBlock(BlockID::light, -1, 6, 8), TreeBlock(BlockID::light, 0, 6, 7), TreeBlock(BlockID::light, -1, 6, 7), TreeBlock(BlockID::light, 8, 6, -3),
        TreeBlock(BlockID::light, 7, 6, -3), TreeBlock(BlockID::light, 8, 6, -2), TreeBlock(BlockID::light, 7, 6, -2), TreeBlock(BlockID::light, 8, 7, -3),
        TreeBlock(BlockID::light, 7, 7, -3), TreeBlock(BlockID::light, 8, 7, -2), TreeBlock(BlockID::light, 7, 7, -2), TreeBlock(BlockID::light, -2, 1, -7),
        TreeBlock(BlockID::light, -1, 1, -7), TreeBlock(BlockID::light, -2, 1, -6), TreeBlock(BlockID::light, -1, 1, -6), TreeBlock(BlockID::light, -1, 2, -7),
        TreeBlock(BlockID::light, -2, 2, -6), TreeBlock(BlockID::light, -1, 2, -6)
    }
};

std::vector<std::vector<TreeBlock>> COLOR_WOOD_TREE_SHAPES = {
    {
        TreeBlock(BlockID::moon_bark, 5, 0, 0), // max extent

        TreeBlock(BlockID::moon_bark, -1, 1, 0), TreeBlock(BlockID::moon_bark, -2, 1, 0), TreeBlock(BlockID::moon_bark, -2, 1, -1), TreeBlock(BlockID::moon_bark, -1, 1, -1),
        TreeBlock(BlockID::moon_bark, -2, 2, 0), TreeBlock(BlockID::moon_bark, -1, 2, -1), TreeBlock(BlockID::moon_bark, -2, 2, 1), TreeBlock(BlockID::moon_bark, -2, 3, 1),
        TreeBlock(BlockID::moon_bark, -2, 4, 1), TreeBlock(BlockID::moon_bark, -3, 4, 1), TreeBlock(BlockID::moon_bark, -3, 5, 1), TreeBlock(BlockID::moon_bark, -3, 6, 1),
        TreeBlock(BlockID::moon_bark, -3, 7, 1), TreeBlock(BlockID::moon_bark, -3, 8, 1), TreeBlock(BlockID::moon_bark, -3, 9, 1), TreeBlock(BlockID::moon_bark, -4, 8, 1),
        TreeBlock(BlockID::moon_bark, -4, 9, 1), TreeBlock(BlockID::moon_bark, -4, 10, 1), TreeBlock(BlockID::moon_bark, -4, 10, 2), TreeBlock(BlockID::moon_bark, -4, 11, 2),
        TreeBlock(BlockID::moon_bark, -4, 12, 2), TreeBlock(BlockID::moon_bark, 0, 2, -1), TreeBlock(BlockID::moon_bark, 0, 3, -1), TreeBlock(BlockID::moon_bark, 0, 3, -2),
        TreeBlock(BlockID::moon_bark, 0, 4, -2), TreeBlock(BlockID::moon_bark, 0, 5, -2), TreeBlock(BlockID::moon_bark, 0, 6, -2), TreeBlock(BlockID::moon_bark, 0, 7, -2),
        TreeBlock(BlockID::moon_bark, 0, 8, -2), TreeBlock(BlockID::moon_bark, 1, 7, -2), TreeBlock(BlockID::moon_bark, 1, 8, -2), TreeBlock(BlockID::moon_bark, 1, 9, -2),
        TreeBlock(BlockID::moon_bark, 1, 10, -2), TreeBlock(BlockID::moon_bark, 1, 11, -2), TreeBlock(BlockID::moon_bark, 1, 12, -2), TreeBlock(BlockID::moon_bark, 2, 10, -2),
        TreeBlock(BlockID::moon_bark, 2, 11, -2), TreeBlock(BlockID::moon_bark, 2, 12, -2), TreeBlock(BlockID::moon_bark, 1, 12, -3), TreeBlock(BlockID::moon_bark, 1, 13, -3),
        TreeBlock(BlockID::moon_bark, 1, 14, -3), TreeBlock(BlockID::moon_bark, 1, 15, -3), TreeBlock(BlockID::moon_bark, 1, 16, -3), TreeBlock(BlockID::moon_bark, 0, 16, -3),
        TreeBlock(BlockID::moon_bark, 0, 17, -3), TreeBlock(BlockID::moon_bark, 0, 18, -3), TreeBlock(BlockID::moon_bark, 0, 19, -3),

        TreeBlock(BlockID::light, -1, 21, -4), TreeBlock(BlockID::light, 1, 21, -4), TreeBlock(BlockID::light, -1, 21, -2), TreeBlock(BlockID::light, 0, 21, -2),
        TreeBlock(BlockID::light, -1, 21, -3), TreeBlock(BlockID::light, 0, 21, -3), TreeBlock(BlockID::light, 1, 21, -3), TreeBlock(BlockID::light, 0, 21, -4),
        TreeBlock(BlockID::light, -5, 13, 3), TreeBlock(BlockID::light, -4, 13, 3), TreeBlock(BlockID::light, -3, 13, 3), TreeBlock(BlockID::light, -5, 13, 2),
        TreeBlock(BlockID::light, -4, 13, 2), TreeBlock(BlockID::light, -3, 13, 2), TreeBlock(BlockID::light, -4, 13, 1),

        TreeBlock(BlockID::chalchanthite, 0, 20, -3), TreeBlock(BlockID::chalchanthite, 0, 20, -4), TreeBlock(BlockID::chalchanthite, 1, 20, -4), TreeBlock(BlockID::chalchanthite, 1, 20, -3),
        TreeBlock(BlockID::chalchanthite, 1, 20, -2), TreeBlock(BlockID::chalchanthite, 0, 20, -2), TreeBlock(BlockID::chalchanthite, -1, 20, -2), TreeBlock(BlockID::chalchanthite, -1, 20, -3),
        TreeBlock(BlockID::chalchanthite, 0, 22, -4), TreeBlock(BlockID::chalchanthite, 1, 22, -4), TreeBlock(BlockID::chalchanthite, -1, 22, -3), TreeBlock(BlockID::chalchanthite, 0, 22, -3),
        TreeBlock(BlockID::chalchanthite, 1, 22, -3), TreeBlock(BlockID::chalchanthite, 0, 23, -3), TreeBlock(BlockID::chalchanthite, 1, 23, -3), TreeBlock(BlockID::chalchanthite, 0, 22, -2),
        TreeBlock(BlockID::chalchanthite, 1, 22, -2),

        TreeBlock(BlockID::feldspar, -5, 14, 1), TreeBlock(BlockID::feldspar, -5, 14, 2), TreeBlock(BlockID::feldspar, -5, 14, 3), TreeBlock(BlockID::feldspar, -4, 14, 1),
        TreeBlock(BlockID::feldspar, -4, 14, 2), TreeBlock(BlockID::feldspar, -4, 14, 3), TreeBlock(BlockID::feldspar, -3, 14, 2), TreeBlock(BlockID::feldspar, -3, 14, 3),
        TreeBlock(BlockID::feldspar, -4, 15, 2)
    },
    {
        TreeBlock(BlockID::moon_bark, 5, 0, 0), // max extent

        TreeBlock(BlockID::moon_bark, -1, 1, 0), TreeBlock(BlockID::moon_bark, -2, 1, 0), TreeBlock(BlockID::moon_bark, -2, 1, -1), TreeBlock(BlockID::moon_bark, -1, 1, -1),
        TreeBlock(BlockID::moon_bark, -2, 2, 0), TreeBlock(BlockID::moon_bark, -1, 2, -1), TreeBlock(BlockID::moon_bark, -2, 2, 1), TreeBlock(BlockID::moon_bark, -2, 3, 1),
        TreeBlock(BlockID::moon_bark, -2, 4, 1), TreeBlock(BlockID::moon_bark, -3, 4, 1), TreeBlock(BlockID::moon_bark, -3, 5, 1), TreeBlock(BlockID::moon_bark, -3, 6, 1),
        TreeBlock(BlockID::moon_bark, -3, 7, 1), TreeBlock(BlockID::moon_bark, -3, 8, 1), TreeBlock(BlockID::moon_bark, -3, 9, 1), TreeBlock(BlockID::moon_bark, -4, 8, 1),
        TreeBlock(BlockID::moon_bark, -4, 9, 1), TreeBlock(BlockID::moon_bark, -4, 10, 1), TreeBlock(BlockID::moon_bark, -4, 10, 2), TreeBlock(BlockID::moon_bark, -4, 11, 2),
        TreeBlock(BlockID::moon_bark, -4, 12, 2), TreeBlock(BlockID::moon_bark, 0, 2, -1), TreeBlock(BlockID::moon_bark, 0, 3, -1), TreeBlock(BlockID::moon_bark, 0, 3, -2),
        TreeBlock(BlockID::moon_bark, 0, 4, -2), TreeBlock(BlockID::moon_bark, 0, 5, -2), TreeBlock(BlockID::moon_bark, 0, 6, -2), TreeBlock(BlockID::moon_bark, 0, 7, -2),
        TreeBlock(BlockID::moon_bark, 0, 8, -2), TreeBlock(BlockID::moon_bark, 1, 7, -2), TreeBlock(BlockID::moon_bark, 1, 8, -2), TreeBlock(BlockID::moon_bark, 1, 9, -2),
        TreeBlock(BlockID::moon_bark, 1, 10, -2), TreeBlock(BlockID::moon_bark, 1, 11, -2), TreeBlock(BlockID::moon_bark, 1, 12, -2), TreeBlock(BlockID::moon_bark, 2, 10, -2),
        TreeBlock(BlockID::moon_bark, 2, 11, -2), TreeBlock(BlockID::moon_bark, 2, 12, -2), TreeBlock(BlockID::moon_bark, 1, 12, -3), TreeBlock(BlockID::moon_bark, 1, 13, -3),
        TreeBlock(BlockID::moon_bark, 1, 14, -3), TreeBlock(BlockID::moon_bark, 1, 15, -3), TreeBlock(BlockID::moon_bark, 1, 16, -3), TreeBlock(BlockID::moon_bark, 0, 16, -3),
        TreeBlock(BlockID::moon_bark, 0, 17, -3), TreeBlock(BlockID::moon_bark, 0, 18, -3), TreeBlock(BlockID::moon_bark, 0, 19, -3),

        TreeBlock(BlockID::light, -1, 21, -2), TreeBlock(BlockID::light, 0, 21, -2), TreeBlock(BlockID::light, -1, 21, -3), TreeBlock(BlockID::light, 0, 21, -3),
        TreeBlock(BlockID::light, 1, 21, -3), TreeBlock(BlockID::light, 0, 21, -4), TreeBlock(BlockID::light, -5, 13, 3), TreeBlock(BlockID::light, -4, 13, 3),
        TreeBlock(BlockID::light, -3, 13, 3), TreeBlock(BlockID::light, -5, 13, 2), TreeBlock(BlockID::light, -4, 13, 2), TreeBlock(BlockID::light, -3, 13, 2),
        TreeBlock(BlockID::light, -4, 13, 1),

        TreeBlock(BlockID::feldspar, 0, 20, -3), TreeBlock(BlockID::feldspar, -1, 22, -4), TreeBlock(BlockID::feldspar, 0, 22, -4), TreeBlock(BlockID::feldspar, 1, 22, -4),
        TreeBlock(BlockID::feldspar, -1, 22, -3), TreeBlock(BlockID::feldspar, 0, 22, -3), TreeBlock(BlockID::feldspar, 1, 22, -3), TreeBlock(BlockID::feldspar, -1, 23, -3),
        TreeBlock(BlockID::feldspar, 0, 23, -3), TreeBlock(BlockID::feldspar, -1, 22, -2), TreeBlock(BlockID::feldspar, 0, 22, -2), TreeBlock(BlockID::feldspar, 1, 22, -2),

        TreeBlock(BlockID::chalchanthite, -5, 14, 1), TreeBlock(BlockID::chalchanthite, -5, 14, 2), TreeBlock(BlockID::chalchanthite, -5, 14, 3), TreeBlock(BlockID::chalchanthite, -4, 14, 1),
        TreeBlock(BlockID::chalchanthite, -4, 14, 2), TreeBlock(BlockID::chalchanthite, -4, 14, 3), TreeBlock(BlockID::chalchanthite, -3, 14, 2), TreeBlock(BlockID::chalchanthite, -3, 14, 3),
        TreeBlock(BlockID::chalchanthite, -4, 15, 2)
    },
    {
        TreeBlock(BlockID::moon_bark, 5, 0, 0), // max extent

        TreeBlock(BlockID::moon_bark, -1, 1, 0), TreeBlock(BlockID::moon_bark, -2, 1, 0), TreeBlock(BlockID::moon_bark, -2, 1, -1), TreeBlock(BlockID::moon_bark, -1, 1, -1),
        TreeBlock(BlockID::moon_bark, -2, 2, 0), TreeBlock(BlockID::moon_bark, -1, 2, -1), TreeBlock(BlockID::moon_bark, -2, 2, 1), TreeBlock(BlockID::moon_bark, -2, 3, 1),
        TreeBlock(BlockID::moon_bark, -2, 4, 1), TreeBlock(BlockID::moon_bark, -3, 4, 1), TreeBlock(BlockID::moon_bark, -3, 5, 1), TreeBlock(BlockID::moon_bark, -3, 6, 1),
        TreeBlock(BlockID::moon_bark, -3, 7, 1), TreeBlock(BlockID::moon_bark, -3, 8, 1), TreeBlock(BlockID::moon_bark, -3, 9, 1), TreeBlock(BlockID::moon_bark, -4, 8, 1),
        TreeBlock(BlockID::moon_bark, -4, 9, 1), TreeBlock(BlockID::moon_bark, -4, 10, 1), TreeBlock(BlockID::moon_bark, -4, 10, 2), TreeBlock(BlockID::moon_bark, -4, 11, 2),
        TreeBlock(BlockID::moon_bark, -4, 12, 2), TreeBlock(BlockID::moon_bark, 0, 2, -1), TreeBlock(BlockID::moon_bark, 0, 3, -1), TreeBlock(BlockID::moon_bark, 0, 3, -2),
        TreeBlock(BlockID::moon_bark, 0, 4, -2), TreeBlock(BlockID::moon_bark, 0, 5, -2), TreeBlock(BlockID::moon_bark, 0, 6, -2), TreeBlock(BlockID::moon_bark, 0, 7, -2),
        TreeBlock(BlockID::moon_bark, 0, 8, -2), TreeBlock(BlockID::moon_bark, 1, 7, -2), TreeBlock(BlockID::moon_bark, 1, 8, -2), TreeBlock(BlockID::moon_bark, 1, 9, -2),
        TreeBlock(BlockID::moon_bark, 1, 10, -2), TreeBlock(BlockID::moon_bark, 1, 11, -2), TreeBlock(BlockID::moon_bark, 1, 12, -2), TreeBlock(BlockID::moon_bark, 2, 10, -2),
        TreeBlock(BlockID::moon_bark, 2, 11, -2), TreeBlock(BlockID::moon_bark, 2, 12, -2), TreeBlock(BlockID::moon_bark, 1, 12, -3), TreeBlock(BlockID::moon_bark, 1, 13, -3),
        TreeBlock(BlockID::moon_bark, 1, 14, -3), TreeBlock(BlockID::moon_bark, 1, 15, -3), TreeBlock(BlockID::moon_bark, 1, 16, -3), TreeBlock(BlockID::moon_bark, 0, 16, -3),
        TreeBlock(BlockID::moon_bark, 0, 17, -3), TreeBlock(BlockID::moon_bark, 0, 18, -3), TreeBlock(BlockID::moon_bark, 0, 19, -3),

        TreeBlock(BlockID::light, -1, 21, -2), TreeBlock(BlockID::light, 0, 21, -2), TreeBlock(BlockID::light, -1, 21, -3), TreeBlock(BlockID::light, 0, 21, -3),
        TreeBlock(BlockID::light, 1, 21, -3), TreeBlock(BlockID::light, 0, 21, -4), TreeBlock(BlockID::light, -5, 13, 3), TreeBlock(BlockID::light, -4, 13, 3),
        TreeBlock(BlockID::light, -3, 13, 3), TreeBlock(BlockID::light, -5, 13, 2), TreeBlock(BlockID::light, -4, 13, 2), TreeBlock(BlockID::light, -3, 13, 1),
        TreeBlock(BlockID::light, -3, 13, 2), TreeBlock(BlockID::light, -4, 13, 1),

        TreeBlock(BlockID::chalchanthite, 0, 20, -3), TreeBlock(BlockID::chalchanthite, -1, 22, -4), TreeBlock(BlockID::chalchanthite, 0, 22, -4), TreeBlock(BlockID::chalchanthite, 1, 22, -4),
        TreeBlock(BlockID::chalchanthite, -1, 22, -3), TreeBlock(BlockID::chalchanthite, 0, 22, -3), TreeBlock(BlockID::chalchanthite, 1, 22, -3), TreeBlock(BlockID::chalchanthite, -1, 23, -3),
        TreeBlock(BlockID::chalchanthite, 0, 23, -3), TreeBlock(BlockID::chalchanthite, 1, 23, -3), TreeBlock(BlockID::chalchanthite, -1, 22, -2), TreeBlock(BlockID::chalchanthite, 0, 22, -2),
        TreeBlock(BlockID::chalchanthite, 1, 22, -2),

        TreeBlock(BlockID::sulphur_ore, -5, 14, 2), TreeBlock(BlockID::sulphur_ore, -5, 14, 3), TreeBlock(BlockID::sulphur_ore, -4, 14, 1), TreeBlock(BlockID::sulphur_ore, -4, 14, 2),
        TreeBlock(BlockID::sulphur_ore, -4, 14, 3), TreeBlock(BlockID::sulphur_ore, -3, 14, 2), TreeBlock(BlockID::sulphur_ore, -3, 14, 3), TreeBlock(BlockID::sulphur_ore, -4, 15, 2)
    },
    {
        TreeBlock(BlockID::moon_bark, 5, 0, 0), // max extent

        TreeBlock(BlockID::moon_bark, -1, 1, 0), TreeBlock(BlockID::moon_bark, -2, 1, 0), TreeBlock(BlockID::moon_bark, -2, 1, -1), TreeBlock(BlockID::moon_bark, -1, 1, -1),
        TreeBlock(BlockID::moon_bark, -2, 2, 0), TreeBlock(BlockID::moon_bark, -1, 2, -1), TreeBlock(BlockID::moon_bark, -2, 2, 1), TreeBlock(BlockID::moon_bark, -2, 3, 1),
        TreeBlock(BlockID::moon_bark, -2, 4, 1), TreeBlock(BlockID::moon_bark, -3, 4, 1), TreeBlock(BlockID::moon_bark, -3, 5, 1), TreeBlock(BlockID::moon_bark, -3, 6, 1),
        TreeBlock(BlockID::moon_bark, -3, 7, 1), TreeBlock(BlockID::moon_bark, -3, 8, 1), TreeBlock(BlockID::moon_bark, -3, 9, 1), TreeBlock(BlockID::moon_bark, -4, 8, 1),
        TreeBlock(BlockID::moon_bark, -4, 9, 1), TreeBlock(BlockID::moon_bark, -4, 10, 1), TreeBlock(BlockID::moon_bark, -4, 10, 2), TreeBlock(BlockID::moon_bark, -4, 11, 2),
        TreeBlock(BlockID::moon_bark, -4, 12, 2), TreeBlock(BlockID::moon_bark, 0, 2, -1), TreeBlock(BlockID::moon_bark, 0, 3, -1), TreeBlock(BlockID::moon_bark, 0, 3, -2),
        TreeBlock(BlockID::moon_bark, 0, 4, -2), TreeBlock(BlockID::moon_bark, 0, 5, -2), TreeBlock(BlockID::moon_bark, 0, 6, -2), TreeBlock(BlockID::moon_bark, 0, 7, -2),
        TreeBlock(BlockID::moon_bark, 0, 8, -2), TreeBlock(BlockID::moon_bark, 1, 7, -2), TreeBlock(BlockID::moon_bark, 1, 8, -2), TreeBlock(BlockID::moon_bark, 1, 9, -2),
        TreeBlock(BlockID::moon_bark, 1, 10, -2), TreeBlock(BlockID::moon_bark, 1, 11, -2), TreeBlock(BlockID::moon_bark, 1, 12, -2), TreeBlock(BlockID::moon_bark, 2, 10, -2),
        TreeBlock(BlockID::moon_bark, 2, 11, -2), TreeBlock(BlockID::moon_bark, 2, 12, -2), TreeBlock(BlockID::moon_bark, 1, 12, -3), TreeBlock(BlockID::moon_bark, 1, 13, -3),
        TreeBlock(BlockID::moon_bark, 1, 14, -3), TreeBlock(BlockID::moon_bark, 1, 15, -3), TreeBlock(BlockID::moon_bark, 1, 16, -3), TreeBlock(BlockID::moon_bark, 0, 16, -3),
        TreeBlock(BlockID::moon_bark, 0, 17, -3), TreeBlock(BlockID::moon_bark, 0, 18, -3), TreeBlock(BlockID::moon_bark, 0, 19, -3),

        TreeBlock(BlockID::light, -1, 21, -2), TreeBlock(BlockID::light, 0, 21, -2), TreeBlock(BlockID::light, -1, 21, -3), TreeBlock(BlockID::light, 0, 21, -3),
        TreeBlock(BlockID::light, 1, 21, -3), TreeBlock(BlockID::light, 0, 21, -4), TreeBlock(BlockID::light, -5, 13, 3), TreeBlock(BlockID::light, -4, 13, 3),
        TreeBlock(BlockID::light, -3, 13, 3), TreeBlock(BlockID::light, -5, 13, 2), TreeBlock(BlockID::light, -4, 13, 2), TreeBlock(BlockID::light, -3, 13, 2),
        TreeBlock(BlockID::light, -4, 13, 1),

        TreeBlock(BlockID::sulphur_ore, 0, 20, -3), TreeBlock(BlockID::sulphur_ore, -1, 22, -4), TreeBlock(BlockID::sulphur_ore, 0, 22, -4), TreeBlock(BlockID::sulphur_ore, 1, 22, -4),
        TreeBlock(BlockID::sulphur_ore, -1, 22, -3), TreeBlock(BlockID::sulphur_ore, 0, 22, -3), TreeBlock(BlockID::sulphur_ore, 1, 22, -3), TreeBlock(BlockID::sulphur_ore, 0, 23, -3),
        TreeBlock(BlockID::sulphur_ore, 1, 23, -3), TreeBlock(BlockID::sulphur_ore, -1, 22, -2), TreeBlock(BlockID::sulphur_ore, 0, 22, -2), TreeBlock(BlockID::sulphur_ore, 1, 22, -2),

        TreeBlock(BlockID::chalchanthite, -5, 14, 1), TreeBlock(BlockID::chalchanthite, -5, 14, 2), TreeBlock(BlockID::chalchanthite, -5, 14, 3), TreeBlock(BlockID::chalchanthite, -4, 14, 1),
        TreeBlock(BlockID::chalchanthite, -4, 14, 2), TreeBlock(BlockID::chalchanthite, -4, 14, 3), TreeBlock(BlockID::chalchanthite, -3, 14, 2), TreeBlock(BlockID::chalchanthite, -3, 14, 3),
        TreeBlock(BlockID::chalchanthite, -4, 15, 2)
    },
    {
        TreeBlock(BlockID::moon_bark, 5, 0, 0), // max extent

        TreeBlock(BlockID::moon_bark, -1, 1, 0), TreeBlock(BlockID::moon_bark, -2, 1, 0), TreeBlock(BlockID::moon_bark, -2, 1, -1), TreeBlock(BlockID::moon_bark, -1, 1, -1),
        TreeBlock(BlockID::moon_bark, -2, 2, 0), TreeBlock(BlockID::moon_bark, -1, 2, -1), TreeBlock(BlockID::moon_bark, -2, 2, 1), TreeBlock(BlockID::moon_bark, -2, 3, 1),
        TreeBlock(BlockID::moon_bark, -2, 4, 1), TreeBlock(BlockID::moon_bark, -3, 4, 1), TreeBlock(BlockID::moon_bark, -3, 5, 1), TreeBlock(BlockID::moon_bark, -3, 6, 1),
        TreeBlock(BlockID::moon_bark, -3, 7, 1), TreeBlock(BlockID::moon_bark, -3, 8, 1), TreeBlock(BlockID::moon_bark, -3, 9, 1), TreeBlock(BlockID::moon_bark, -4, 8, 1),
        TreeBlock(BlockID::moon_bark, -4, 9, 1), TreeBlock(BlockID::moon_bark, -4, 10, 1), TreeBlock(BlockID::moon_bark, -4, 10, 2), TreeBlock(BlockID::moon_bark, -4, 11, 2),
        TreeBlock(BlockID::moon_bark, -4, 12, 2), TreeBlock(BlockID::moon_bark, 0, 2, -1), TreeBlock(BlockID::moon_bark, 0, 3, -1), TreeBlock(BlockID::moon_bark, 0, 3, -2),
        TreeBlock(BlockID::moon_bark, 0, 4, -2), TreeBlock(BlockID::moon_bark, 0, 5, -2), TreeBlock(BlockID::moon_bark, 0, 6, -2), TreeBlock(BlockID::moon_bark, 0, 7, -2),
        TreeBlock(BlockID::moon_bark, 0, 8, -2), TreeBlock(BlockID::moon_bark, 1, 7, -2), TreeBlock(BlockID::moon_bark, 1, 8, -2), TreeBlock(BlockID::moon_bark, 1, 9, -2),
        TreeBlock(BlockID::moon_bark, 1, 10, -2), TreeBlock(BlockID::moon_bark, 1, 11, -2), TreeBlock(BlockID::moon_bark, 1, 12, -2), TreeBlock(BlockID::moon_bark, 2, 10, -2),
        TreeBlock(BlockID::moon_bark, 2, 11, -2), TreeBlock(BlockID::moon_bark, 2, 12, -2), TreeBlock(BlockID::moon_bark, 1, 12, -3), TreeBlock(BlockID::moon_bark, 1, 13, -3),
        TreeBlock(BlockID::moon_bark, 1, 14, -3), TreeBlock(BlockID::moon_bark, 1, 15, -3), TreeBlock(BlockID::moon_bark, 1, 16, -3), TreeBlock(BlockID::moon_bark, 0, 16, -3),
        TreeBlock(BlockID::moon_bark, 0, 17, -3), TreeBlock(BlockID::moon_bark, 0, 18, -3), TreeBlock(BlockID::moon_bark, 0, 19, -3),

        TreeBlock(BlockID::light, -1, 21, -4), TreeBlock(BlockID::light, 1, 21, -4), TreeBlock(BlockID::light, -1, 21, -2), TreeBlock(BlockID::light, 0, 21, -2),
        TreeBlock(BlockID::light, -1, 21, -3), TreeBlock(BlockID::light, 0, 21, -3), TreeBlock(BlockID::light, 1, 21, -3), TreeBlock(BlockID::light, 0, 21, -4),
        TreeBlock(BlockID::light, -5, 13, 3), TreeBlock(BlockID::light, -4, 13, 3), TreeBlock(BlockID::light, -3, 13, 3), TreeBlock(BlockID::light, -5, 13, 2),
        TreeBlock(BlockID::light, -4, 13, 2), TreeBlock(BlockID::light, -3, 13, 2), TreeBlock(BlockID::light, -4, 13, 1), TreeBlock(BlockID::light, -3, 13, 1),

        TreeBlock(BlockID::feldspar, 0, 20, -4), TreeBlock(BlockID::feldspar, 1, 20, -4), TreeBlock(BlockID::feldspar, 1, 20, -3), TreeBlock(BlockID::feldspar, 1, 20, -2),
        TreeBlock(BlockID::feldspar, 0, 20, -2), TreeBlock(BlockID::feldspar, -1, 20, -2), TreeBlock(BlockID::feldspar, -1, 20, -3), TreeBlock(BlockID::feldspar, 0, 20, -3),
        TreeBlock(BlockID::feldspar, 0, 22, -4), TreeBlock(BlockID::feldspar, 1, 22, -4), TreeBlock(BlockID::feldspar, -1, 22, -3), TreeBlock(BlockID::feldspar, 0, 22, -3),
        TreeBlock(BlockID::feldspar, 1, 22, -3), TreeBlock(BlockID::feldspar, 0, 23, -3), TreeBlock(BlockID::feldspar, 1, 23, -3), TreeBlock(BlockID::feldspar, 0, 22, -2),
        TreeBlock(BlockID::feldspar, 1, 22, -2),

        TreeBlock(BlockID::sulphur_ore, -5, 14, 2), TreeBlock(BlockID::sulphur_ore, -5, 14, 3), TreeBlock(BlockID::sulphur_ore, -4, 14, 1), TreeBlock(BlockID::sulphur_ore, -4, 14, 2),
        TreeBlock(BlockID::sulphur_ore, -4, 14, 3), TreeBlock(BlockID::sulphur_ore, -3, 14, 2), TreeBlock(BlockID::sulphur_ore, -3, 14, 3), TreeBlock(BlockID::sulphur_ore, -4, 15, 2)
    },
    {
        TreeBlock(BlockID::moon_bark, 5, 0, 0), // max extent

        TreeBlock(BlockID::moon_bark, -1, 1, 0), TreeBlock(BlockID::moon_bark, -2, 1, 0), TreeBlock(BlockID::moon_bark, -2, 1, -1), TreeBlock(BlockID::moon_bark, -1, 1, -1),
        TreeBlock(BlockID::moon_bark, -2, 2, 0), TreeBlock(BlockID::moon_bark, -1, 2, -1), TreeBlock(BlockID::moon_bark, -2, 2, 1), TreeBlock(BlockID::moon_bark, -2, 3, 1),
        TreeBlock(BlockID::moon_bark, -2, 4, 1), TreeBlock(BlockID::moon_bark, -3, 4, 1), TreeBlock(BlockID::moon_bark, -3, 5, 1), TreeBlock(BlockID::moon_bark, -3, 6, 1),
        TreeBlock(BlockID::moon_bark, -3, 7, 1), TreeBlock(BlockID::moon_bark, -3, 8, 1), TreeBlock(BlockID::moon_bark, -3, 9, 1), TreeBlock(BlockID::moon_bark, -4, 8, 1),
        TreeBlock(BlockID::moon_bark, -4, 9, 1), TreeBlock(BlockID::moon_bark, -4, 10, 1), TreeBlock(BlockID::moon_bark, -4, 10, 2), TreeBlock(BlockID::moon_bark, -4, 11, 2),
        TreeBlock(BlockID::moon_bark, -4, 12, 2), TreeBlock(BlockID::moon_bark, 0, 2, -1), TreeBlock(BlockID::moon_bark, 0, 3, -1), TreeBlock(BlockID::moon_bark, 0, 3, -2),
        TreeBlock(BlockID::moon_bark, 0, 4, -2), TreeBlock(BlockID::moon_bark, 0, 5, -2), TreeBlock(BlockID::moon_bark, 0, 6, -2), TreeBlock(BlockID::moon_bark, 0, 7, -2),
        TreeBlock(BlockID::moon_bark, 0, 8, -2), TreeBlock(BlockID::moon_bark, 1, 7, -2), TreeBlock(BlockID::moon_bark, 1, 8, -2), TreeBlock(BlockID::moon_bark, 1, 9, -2),
        TreeBlock(BlockID::moon_bark, 1, 10, -2), TreeBlock(BlockID::moon_bark, 1, 11, -2), TreeBlock(BlockID::moon_bark, 1, 12, -2), TreeBlock(BlockID::moon_bark, 2, 10, -2),
        TreeBlock(BlockID::moon_bark, 2, 11, -2), TreeBlock(BlockID::moon_bark, 2, 12, -2), TreeBlock(BlockID::moon_bark, 1, 12, -3), TreeBlock(BlockID::moon_bark, 1, 13, -3),
        TreeBlock(BlockID::moon_bark, 1, 14, -3), TreeBlock(BlockID::moon_bark, 1, 15, -3), TreeBlock(BlockID::moon_bark, 1, 16, -3), TreeBlock(BlockID::moon_bark, 0, 16, -3),
        TreeBlock(BlockID::moon_bark, 0, 17, -3), TreeBlock(BlockID::moon_bark, 0, 18, -3), TreeBlock(BlockID::moon_bark, 0, 19, -3),

        TreeBlock(BlockID::light, -1, 21, -2), TreeBlock(BlockID::light, 0, 21, -2), TreeBlock(BlockID::light, -1, 21, -3), TreeBlock(BlockID::light, 0, 21, -3),
        TreeBlock(BlockID::light, 1, 21, -3), TreeBlock(BlockID::light, 0, 21, -4), TreeBlock(BlockID::light, -5, 13, 3), TreeBlock(BlockID::light, -4, 13, 3),
        TreeBlock(BlockID::light, -3, 13, 3), TreeBlock(BlockID::light, -5, 13, 2), TreeBlock(BlockID::light, -4, 13, 2), TreeBlock(BlockID::light, -3, 13, 2),
        TreeBlock(BlockID::light, -4, 13, 1),

        TreeBlock(BlockID::sulphur_ore, 0, 20, -3), TreeBlock(BlockID::sulphur_ore, -1, 22, -4), TreeBlock(BlockID::sulphur_ore, 0, 22, -4), TreeBlock(BlockID::sulphur_ore, 1, 22, -4),
        TreeBlock(BlockID::sulphur_ore, -1, 22, -3), TreeBlock(BlockID::sulphur_ore, 0, 22, -3), TreeBlock(BlockID::sulphur_ore, 1, 22, -3), TreeBlock(BlockID::sulphur_ore, -1, 22, -2),
        TreeBlock(BlockID::sulphur_ore, 0, 22, -2), TreeBlock(BlockID::sulphur_ore, 1, 22, -2), TreeBlock(BlockID::sulphur_ore, -1, 23, -3), TreeBlock(BlockID::sulphur_ore, 0, 23, -3),

        TreeBlock(BlockID::feldspar, -5, 14, 1), TreeBlock(BlockID::feldspar, -5, 14, 2), TreeBlock(BlockID::feldspar, -5, 14, 3), TreeBlock(BlockID::feldspar, -4, 14, 1),
        TreeBlock(BlockID::feldspar, -4, 14, 2), TreeBlock(BlockID::feldspar, -4, 14, 3), TreeBlock(BlockID::feldspar, -3, 14, 2), TreeBlock(BlockID::feldspar, -3, 14, 3),
        TreeBlock(BlockID::feldspar, -4, 15, 2)
    },
    {
        TreeBlock(BlockID::moon_bark, 5, 0, 0), // max extent

        TreeBlock(BlockID::moon_bark, -1, 1, 0), TreeBlock(BlockID::moon_bark, -2, 1, 0), TreeBlock(BlockID::moon_bark, -2, 1, -1), TreeBlock(BlockID::moon_bark, -1, 1, -1),
        TreeBlock(BlockID::moon_bark, -2, 2, 0), TreeBlock(BlockID::moon_bark, -1, 2, -1), TreeBlock(BlockID::moon_bark, -2, 2, 1), TreeBlock(BlockID::moon_bark, -2, 3, 1),
        TreeBlock(BlockID::moon_bark, -2, 4, 1), TreeBlock(BlockID::moon_bark, -3, 4, 1), TreeBlock(BlockID::moon_bark, -3, 5, 1), TreeBlock(BlockID::moon_bark, -3, 6, 1),
        TreeBlock(BlockID::moon_bark, -3, 7, 1), TreeBlock(BlockID::moon_bark, -3, 8, 1), TreeBlock(BlockID::moon_bark, -3, 9, 1), TreeBlock(BlockID::moon_bark, -4, 8, 1),
        TreeBlock(BlockID::moon_bark, -4, 9, 1), TreeBlock(BlockID::moon_bark, -4, 10, 1), TreeBlock(BlockID::moon_bark, -4, 10, 2), TreeBlock(BlockID::moon_bark, -4, 11, 2),
        TreeBlock(BlockID::moon_bark, -4, 12, 2), TreeBlock(BlockID::moon_bark, 0, 2, -1), TreeBlock(BlockID::moon_bark, 0, 3, -1), TreeBlock(BlockID::moon_bark, 0, 3, -2),
        TreeBlock(BlockID::moon_bark, 0, 4, -2), TreeBlock(BlockID::moon_bark, 0, 5, -2), TreeBlock(BlockID::moon_bark, 0, 6, -2), TreeBlock(BlockID::moon_bark, 0, 7, -2),
        TreeBlock(BlockID::moon_bark, 0, 8, -2), TreeBlock(BlockID::moon_bark, 1, 7, -2), TreeBlock(BlockID::moon_bark, 1, 8, -2), TreeBlock(BlockID::moon_bark, 1, 9, -2),
        TreeBlock(BlockID::moon_bark, 1, 10, -2), TreeBlock(BlockID::moon_bark, 1, 11, -2), TreeBlock(BlockID::moon_bark, 1, 12, -2), TreeBlock(BlockID::moon_bark, 2, 10, -2),
        TreeBlock(BlockID::moon_bark, 2, 11, -2), TreeBlock(BlockID::moon_bark, 2, 12, -2), TreeBlock(BlockID::moon_bark, 1, 12, -3), TreeBlock(BlockID::moon_bark, 1, 13, -3),
        TreeBlock(BlockID::moon_bark, 1, 14, -3), TreeBlock(BlockID::moon_bark, 1, 15, -3), TreeBlock(BlockID::moon_bark, 1, 16, -3), TreeBlock(BlockID::moon_bark, 0, 16, -3),
        TreeBlock(BlockID::moon_bark, 0, 17, -3), TreeBlock(BlockID::moon_bark, 0, 18, -3), TreeBlock(BlockID::moon_bark, 0, 19, -3),

        TreeBlock(BlockID::light, -1, 21, -2), TreeBlock(BlockID::light, 0, 21, -2), TreeBlock(BlockID::light, -1, 21, -3), TreeBlock(BlockID::light, 0, 21, -3),
        TreeBlock(BlockID::light, 1, 21, -3), TreeBlock(BlockID::light, 0, 21, -4), TreeBlock(BlockID::light, -5, 13, 3), TreeBlock(BlockID::light, -4, 13, 3),
        TreeBlock(BlockID::light, -3, 13, 3), TreeBlock(BlockID::light, -5, 13, 2), TreeBlock(BlockID::light, -4, 13, 2), TreeBlock(BlockID::light, -3, 13, 2),
        TreeBlock(BlockID::light, -4, 13, 1),

        TreeBlock(BlockID::feldspar, 0, 20, -3), TreeBlock(BlockID::feldspar, -1, 22, -4), TreeBlock(BlockID::feldspar, 0, 22, -4), TreeBlock(BlockID::feldspar, 1, 22, -4),
        TreeBlock(BlockID::feldspar, -1, 22, -3), TreeBlock(BlockID::feldspar, 0, 22, -3), TreeBlock(BlockID::feldspar, 1, 22, -3), TreeBlock(BlockID::feldspar, -1, 22, -2),
        TreeBlock(BlockID::feldspar, 0, 22, -2), TreeBlock(BlockID::feldspar, 1, 22, -2), TreeBlock(BlockID::feldspar, -1, 23, -3), TreeBlock(BlockID::feldspar, 0, 23, -3),

        TreeBlock(BlockID::feldspar, -5, 14, 1), TreeBlock(BlockID::feldspar, -5, 14, 2), TreeBlock(BlockID::feldspar, -5, 14, 3), TreeBlock(BlockID::feldspar, -4, 14, 1),
        TreeBlock(BlockID::feldspar, -4, 14, 2), TreeBlock(BlockID::feldspar, -4, 14, 3), TreeBlock(BlockID::feldspar, -3, 14, 2), TreeBlock(BlockID::feldspar, -3, 14, 3),
        TreeBlock(BlockID::feldspar, -4, 15, 2)
    },
    {
        TreeBlock(BlockID::moon_bark, 5, 0, 0), // max extent

        TreeBlock(BlockID::moon_bark, -1, 1, 0), TreeBlock(BlockID::moon_bark, -2, 1, 0), TreeBlock(BlockID::moon_bark, -2, 1, -1), TreeBlock(BlockID::moon_bark, -1, 1, -1),
        TreeBlock(BlockID::moon_bark, -2, 2, 0), TreeBlock(BlockID::moon_bark, -1, 2, -1), TreeBlock(BlockID::moon_bark, -2, 2, 1), TreeBlock(BlockID::moon_bark, -2, 3, 1),
        TreeBlock(BlockID::moon_bark, -2, 4, 1), TreeBlock(BlockID::moon_bark, -3, 4, 1), TreeBlock(BlockID::moon_bark, -3, 5, 1), TreeBlock(BlockID::moon_bark, -3, 6, 1),
        TreeBlock(BlockID::moon_bark, -3, 7, 1), TreeBlock(BlockID::moon_bark, -3, 8, 1), TreeBlock(BlockID::moon_bark, -3, 9, 1), TreeBlock(BlockID::moon_bark, -4, 8, 1),
        TreeBlock(BlockID::moon_bark, -4, 9, 1), TreeBlock(BlockID::moon_bark, -4, 10, 1), TreeBlock(BlockID::moon_bark, -4, 10, 2), TreeBlock(BlockID::moon_bark, -4, 11, 2),
        TreeBlock(BlockID::moon_bark, -4, 12, 2), TreeBlock(BlockID::moon_bark, 0, 2, -1), TreeBlock(BlockID::moon_bark, 0, 3, -1), TreeBlock(BlockID::moon_bark, 0, 3, -2),
        TreeBlock(BlockID::moon_bark, 0, 4, -2), TreeBlock(BlockID::moon_bark, 0, 5, -2), TreeBlock(BlockID::moon_bark, 0, 6, -2), TreeBlock(BlockID::moon_bark, 0, 7, -2),
        TreeBlock(BlockID::moon_bark, 0, 8, -2), TreeBlock(BlockID::moon_bark, 1, 7, -2), TreeBlock(BlockID::moon_bark, 1, 8, -2), TreeBlock(BlockID::moon_bark, 1, 9, -2),
        TreeBlock(BlockID::moon_bark, 1, 10, -2), TreeBlock(BlockID::moon_bark, 1, 11, -2), TreeBlock(BlockID::moon_bark, 1, 12, -2), TreeBlock(BlockID::moon_bark, 2, 10, -2),
        TreeBlock(BlockID::moon_bark, 2, 11, -2), TreeBlock(BlockID::moon_bark, 2, 12, -2), TreeBlock(BlockID::moon_bark, 1, 12, -3), TreeBlock(BlockID::moon_bark, 1, 13, -3),
        TreeBlock(BlockID::moon_bark, 1, 14, -3), TreeBlock(BlockID::moon_bark, 1, 15, -3), TreeBlock(BlockID::moon_bark, 1, 16, -3), TreeBlock(BlockID::moon_bark, 0, 16, -3),
        TreeBlock(BlockID::moon_bark, 0, 17, -3), TreeBlock(BlockID::moon_bark, 0, 18, -3), TreeBlock(BlockID::moon_bark, 0, 19, -3),

        TreeBlock(BlockID::light, -1, 21, -2), TreeBlock(BlockID::light, 0, 21, -2), TreeBlock(BlockID::light, -1, 21, -3), TreeBlock(BlockID::light, 0, 21, -3),
        TreeBlock(BlockID::light, 1, 21, -3), TreeBlock(BlockID::light, 0, 21, -4), TreeBlock(BlockID::light, -5, 13, 3), TreeBlock(BlockID::light, -4, 13, 3),
        TreeBlock(BlockID::light, -3, 13, 3), TreeBlock(BlockID::light, -5, 13, 2), TreeBlock(BlockID::light, -4, 13, 2), TreeBlock(BlockID::light, -3, 13, 1),
        TreeBlock(BlockID::light, -3, 13, 2), TreeBlock(BlockID::light, -4, 13, 1),

        TreeBlock(BlockID::chalchanthite, 0, 20, -3), TreeBlock(BlockID::chalchanthite, -1, 22, -4), TreeBlock(BlockID::chalchanthite, 0, 22, -4), TreeBlock(BlockID::chalchanthite, 1, 22, -4),
        TreeBlock(BlockID::chalchanthite, -1, 22, -3), TreeBlock(BlockID::chalchanthite, 0, 22, -3), TreeBlock(BlockID::chalchanthite, 1, 22, -3), TreeBlock(BlockID::chalchanthite, -1, 23, -3),
        TreeBlock(BlockID::chalchanthite, 0, 23, -3), TreeBlock(BlockID::chalchanthite, 1, 23, -3), TreeBlock(BlockID::chalchanthite, -1, 22, -2), TreeBlock(BlockID::chalchanthite, 0, 22, -2),
        TreeBlock(BlockID::chalchanthite, 1, 22, -2),

        TreeBlock(BlockID::chalchanthite, -5, 14, 2), TreeBlock(BlockID::chalchanthite, -5, 14, 3), TreeBlock(BlockID::chalchanthite, -4, 14, 1), TreeBlock(BlockID::chalchanthite, -4, 14, 2),
        TreeBlock(BlockID::chalchanthite, -4, 14, 3), TreeBlock(BlockID::chalchanthite, -3, 14, 2), TreeBlock(BlockID::chalchanthite, -3, 14, 3), TreeBlock(BlockID::chalchanthite, -4, 15, 2)
    },
    {
        TreeBlock(BlockID::moon_bark, 6, 0, 0), // max extent

        TreeBlock(BlockID::moon_bark, 1, 1, 0), TreeBlock(BlockID::moon_bark, 0, 1, -1), TreeBlock(BlockID::moon_bark, 1, 1, -1), TreeBlock(BlockID::moon_bark, 2, 1, -1),
        TreeBlock(BlockID::moon_bark, 0, 2, -1), TreeBlock(BlockID::moon_bark, 1, 2, -1), TreeBlock(BlockID::moon_bark, 2, 2, -1), TreeBlock(BlockID::moon_bark, 2, 1, -2),
        TreeBlock(BlockID::moon_bark, 1, 1, -2), TreeBlock(BlockID::moon_bark, 1, 2, -2), TreeBlock(BlockID::moon_bark, 0, 2, -2), TreeBlock(BlockID::moon_bark, 0, 3, -2),
        TreeBlock(BlockID::moon_bark, -1, 3, -2), TreeBlock(BlockID::moon_bark, -1, 4, -2), TreeBlock(BlockID::moon_bark, -1, 4, -3), TreeBlock(BlockID::moon_bark, -1, 5, -3),
        TreeBlock(BlockID::moon_bark, -1, 6, -3), TreeBlock(BlockID::moon_bark, -1, 7, -3), TreeBlock(BlockID::moon_bark, -1, 6, -4), TreeBlock(BlockID::moon_bark, -1, 7, -4),
        TreeBlock(BlockID::moon_bark, -1, 8, -4), TreeBlock(BlockID::moon_bark, -1, 9, -4), TreeBlock(BlockID::moon_bark, -2, 7, -4), TreeBlock(BlockID::moon_bark, -2, 8, -4),
        TreeBlock(BlockID::moon_bark, -2, 9, -4), TreeBlock(BlockID::moon_bark, -2, 8, -5), TreeBlock(BlockID::moon_bark, -2, 9, -5), TreeBlock(BlockID::moon_bark, -2, 10, -5),
        TreeBlock(BlockID::moon_bark, -2, 11, -5), TreeBlock(BlockID::moon_bark, -2, 12, -5), TreeBlock(BlockID::moon_bark, -2, 13, -5), TreeBlock(BlockID::moon_bark, -2, 14, -5),
        TreeBlock(BlockID::moon_bark, -2, 15, -5), TreeBlock(BlockID::moon_bark, -1, 9, -5), TreeBlock(BlockID::moon_bark, -1, 10, -5), TreeBlock(BlockID::moon_bark, -1, 11, -5),
        TreeBlock(BlockID::moon_bark, -1, 14, -5), TreeBlock(BlockID::moon_bark, -1, 15, -5), TreeBlock(BlockID::moon_bark, -1, 15, -4), TreeBlock(BlockID::moon_bark, 2, 2, 0),
        TreeBlock(BlockID::moon_bark, 2, 3, 0), TreeBlock(BlockID::moon_bark, 2, 3, 1), TreeBlock(BlockID::moon_bark, 2, 4, 1), TreeBlock(BlockID::moon_bark, 3, 4, 1),
        TreeBlock(BlockID::moon_bark, 3, 5, 1), TreeBlock(BlockID::moon_bark, 3, 6, 1), TreeBlock(BlockID::moon_bark, 3, 7, 1), TreeBlock(BlockID::moon_bark, 3, 7, 2),
        TreeBlock(BlockID::moon_bark, 3, 8, 2), TreeBlock(BlockID::moon_bark, 3, 9, 2), TreeBlock(BlockID::moon_bark, 3, 10, 2), TreeBlock(BlockID::moon_bark, 3, 11, 2),
        TreeBlock(BlockID::moon_bark, 3, 12, 2), TreeBlock(BlockID::moon_bark, 4, 11, 2), TreeBlock(BlockID::moon_bark, 4, 12, 2), TreeBlock(BlockID::moon_bark, 4, 13, 2),
        TreeBlock(BlockID::moon_bark, 4, 14, 2), TreeBlock(BlockID::moon_bark, 5, 12, 2), TreeBlock(BlockID::moon_bark, 5, 13, 2), TreeBlock(BlockID::moon_bark, 5, 14, 2),
        TreeBlock(BlockID::moon_bark, 5, 14, 3), TreeBlock(BlockID::moon_bark, 4, 14, 3),

        TreeBlock(BlockID::light, 5, 16, 4), TreeBlock(BlockID::light, 4, 16, 3), TreeBlock(BlockID::light, 5, 16, 3), TreeBlock(BlockID::light, 6, 16, 3),
        TreeBlock(BlockID::light, 5, 16, 2), TreeBlock(BlockID::light, 4, 16, 2), TreeBlock(BlockID::light, -3, 17, -6), TreeBlock(BlockID::light, -3, 17, -5),
        TreeBlock(BlockID::light, -3, 17, -4), TreeBlock(BlockID::light, -2, 17, -6), TreeBlock(BlockID::light, -2, 17, -5), TreeBlock(BlockID::light, -2, 17, -4),
        TreeBlock(BlockID::light, -1, 17, -5),

        TreeBlock(BlockID::feldspar, -1, 16, -6), TreeBlock(BlockID::feldspar, -1, 16, -5), TreeBlock(BlockID::feldspar, -1, 16, -4), TreeBlock(BlockID::feldspar, -2, 16, -6),
        TreeBlock(BlockID::feldspar, -2, 16, -5), TreeBlock(BlockID::feldspar, -2, 16, -4), TreeBlock(BlockID::feldspar, -3, 16, -5), TreeBlock(BlockID::feldspar, -3, 16, -4),
        TreeBlock(BlockID::feldspar, -1, 18, -5), TreeBlock(BlockID::feldspar, -1, 18, -4), TreeBlock(BlockID::feldspar, -2, 18, -6), TreeBlock(BlockID::feldspar, -2, 18, -5),
        TreeBlock(BlockID::feldspar, -2, 18, -4), TreeBlock(BlockID::feldspar, -3, 18, -6), TreeBlock(BlockID::feldspar, -3, 18, -5),

        TreeBlock(BlockID::chalchanthite, 4, 15, 4), TreeBlock(BlockID::chalchanthite, 4, 15, 3), TreeBlock(BlockID::chalchanthite, 4, 15, 2), TreeBlock(BlockID::chalchanthite, 5, 15, 4),
        TreeBlock(BlockID::chalchanthite, 5, 15, 3), TreeBlock(BlockID::chalchanthite, 5, 15, 2), TreeBlock(BlockID::chalchanthite, 6, 15, 3), TreeBlock(BlockID::chalchanthite, 6, 15, 2),
        TreeBlock(BlockID::chalchanthite, 4, 17, 3), TreeBlock(BlockID::chalchanthite, 5, 17, 4), TreeBlock(BlockID::chalchanthite, 5, 17, 3), TreeBlock(BlockID::chalchanthite, 5, 17, 2),
        TreeBlock(BlockID::chalchanthite, 6, 17, 4), TreeBlock(BlockID::chalchanthite, 6, 17, 3)
    },
    {
        TreeBlock(BlockID::moon_bark, 6, 0, 0), // max extent

        TreeBlock(BlockID::moon_bark, 1, 1, 0), TreeBlock(BlockID::moon_bark, 0, 1, -1), TreeBlock(BlockID::moon_bark, 1, 1, -1), TreeBlock(BlockID::moon_bark, 2, 1, -1),
        TreeBlock(BlockID::moon_bark, 0, 2, -1), TreeBlock(BlockID::moon_bark, 1, 2, -1), TreeBlock(BlockID::moon_bark, 2, 2, -1), TreeBlock(BlockID::moon_bark, 2, 1, -2),
        TreeBlock(BlockID::moon_bark, 1, 1, -2), TreeBlock(BlockID::moon_bark, 1, 2, -2), TreeBlock(BlockID::moon_bark, 0, 2, -2), TreeBlock(BlockID::moon_bark, 0, 3, -2),
        TreeBlock(BlockID::moon_bark, -1, 3, -2), TreeBlock(BlockID::moon_bark, -1, 4, -2), TreeBlock(BlockID::moon_bark, -1, 4, -3), TreeBlock(BlockID::moon_bark, -1, 5, -3),
        TreeBlock(BlockID::moon_bark, -1, 6, -3), TreeBlock(BlockID::moon_bark, -1, 7, -3), TreeBlock(BlockID::moon_bark, -1, 6, -4), TreeBlock(BlockID::moon_bark, -1, 7, -4),
        TreeBlock(BlockID::moon_bark, -1, 8, -4), TreeBlock(BlockID::moon_bark, -1, 9, -4), TreeBlock(BlockID::moon_bark, -2, 7, -4), TreeBlock(BlockID::moon_bark, -2, 8, -4),
        TreeBlock(BlockID::moon_bark, -2, 9, -4), TreeBlock(BlockID::moon_bark, -2, 8, -5), TreeBlock(BlockID::moon_bark, -2, 9, -5), TreeBlock(BlockID::moon_bark, -2, 10, -5),
        TreeBlock(BlockID::moon_bark, -2, 11, -5), TreeBlock(BlockID::moon_bark, -2, 12, -5), TreeBlock(BlockID::moon_bark, -2, 13, -5), TreeBlock(BlockID::moon_bark, -2, 14, -5),
        TreeBlock(BlockID::moon_bark, -2, 15, -5), TreeBlock(BlockID::moon_bark, -1, 9, -5), TreeBlock(BlockID::moon_bark, -1, 10, -5), TreeBlock(BlockID::moon_bark, -1, 11, -5),
        TreeBlock(BlockID::moon_bark, -1, 14, -5), TreeBlock(BlockID::moon_bark, -1, 15, -5), TreeBlock(BlockID::moon_bark, -1, 15, -4), TreeBlock(BlockID::moon_bark, 2, 2, 0),
        TreeBlock(BlockID::moon_bark, 2, 3, 0), TreeBlock(BlockID::moon_bark, 2, 3, 1), TreeBlock(BlockID::moon_bark, 2, 4, 1), TreeBlock(BlockID::moon_bark, 3, 4, 1),
        TreeBlock(BlockID::moon_bark, 3, 5, 1), TreeBlock(BlockID::moon_bark, 3, 6, 1), TreeBlock(BlockID::moon_bark, 3, 7, 1), TreeBlock(BlockID::moon_bark, 3, 7, 2),
        TreeBlock(BlockID::moon_bark, 3, 8, 2), TreeBlock(BlockID::moon_bark, 3, 9, 2), TreeBlock(BlockID::moon_bark, 3, 10, 2), TreeBlock(BlockID::moon_bark, 3, 11, 2),
        TreeBlock(BlockID::moon_bark, 3, 12, 2), TreeBlock(BlockID::moon_bark, 4, 11, 2), TreeBlock(BlockID::moon_bark, 4, 12, 2), TreeBlock(BlockID::moon_bark, 4, 13, 2),
        TreeBlock(BlockID::moon_bark, 4, 14, 2), TreeBlock(BlockID::moon_bark, 5, 12, 2), TreeBlock(BlockID::moon_bark, 5, 13, 2), TreeBlock(BlockID::moon_bark, 5, 14, 2),
        TreeBlock(BlockID::moon_bark, 5, 14, 3), TreeBlock(BlockID::moon_bark, 4, 14, 3),

        TreeBlock(BlockID::light, 5, 16, 4), TreeBlock(BlockID::light, 4, 16, 3), TreeBlock(BlockID::light, 5, 16, 3), TreeBlock(BlockID::light, 6, 16, 3),
        TreeBlock(BlockID::light, 5, 16, 2), TreeBlock(BlockID::light, 4, 16, 2), TreeBlock(BlockID::light, -3, 17, -6), TreeBlock(BlockID::light, -3, 17, -5),
        TreeBlock(BlockID::light, -3, 17, -4), TreeBlock(BlockID::light, -2, 17, -6), TreeBlock(BlockID::light, -2, 17, -5), TreeBlock(BlockID::light, -2, 17, -4),
        TreeBlock(BlockID::light, -1, 17, -5),

        TreeBlock(BlockID::chalchanthite, -1, 16, -6), TreeBlock(BlockID::chalchanthite, -1, 16, -5), TreeBlock(BlockID::chalchanthite, -1, 16, -4), TreeBlock(BlockID::chalchanthite, -2, 16, -6),
        TreeBlock(BlockID::chalchanthite, -2, 16, -5), TreeBlock(BlockID::chalchanthite, -2, 16, -4), TreeBlock(BlockID::chalchanthite, -3, 16, -5), TreeBlock(BlockID::chalchanthite, -3, 16, -4),
        TreeBlock(BlockID::chalchanthite, -1, 18, -5), TreeBlock(BlockID::chalchanthite, -1, 18, -4), TreeBlock(BlockID::chalchanthite, -2, 18, -6), TreeBlock(BlockID::chalchanthite, -2, 18, -5),
        TreeBlock(BlockID::chalchanthite, -2, 18, -4), TreeBlock(BlockID::chalchanthite, -3, 18, -6), TreeBlock(BlockID::chalchanthite, -3, 18, -5),

        TreeBlock(BlockID::sulphur_ore, 4, 15, 4), TreeBlock(BlockID::sulphur_ore, 4, 15, 3), TreeBlock(BlockID::sulphur_ore, 4, 15, 2), TreeBlock(BlockID::sulphur_ore, 5, 15, 4),
        TreeBlock(BlockID::sulphur_ore, 5, 15, 3), TreeBlock(BlockID::sulphur_ore, 5, 15, 2), TreeBlock(BlockID::sulphur_ore, 6, 15, 3), TreeBlock(BlockID::sulphur_ore, 6, 15, 2),
        TreeBlock(BlockID::sulphur_ore, 4, 17, 3), TreeBlock(BlockID::sulphur_ore, 5, 17, 4), TreeBlock(BlockID::sulphur_ore, 5, 17, 3), TreeBlock(BlockID::sulphur_ore, 5, 17, 2),
        TreeBlock(BlockID::sulphur_ore, 6, 17, 4), TreeBlock(BlockID::sulphur_ore, 6, 17, 3)
    },
    {
        TreeBlock(BlockID::moon_bark, 6, 0, 0), // max extent

        TreeBlock(BlockID::moon_bark, 1, 1, 0), TreeBlock(BlockID::moon_bark, 0, 1, -1), TreeBlock(BlockID::moon_bark, 1, 1, -1), TreeBlock(BlockID::moon_bark, 2, 1, -1),
        TreeBlock(BlockID::moon_bark, 0, 2, -1), TreeBlock(BlockID::moon_bark, 1, 2, -1), TreeBlock(BlockID::moon_bark, 2, 2, -1), TreeBlock(BlockID::moon_bark, 2, 1, -2),
        TreeBlock(BlockID::moon_bark, 1, 1, -2), TreeBlock(BlockID::moon_bark, 1, 2, -2), TreeBlock(BlockID::moon_bark, 0, 2, -2), TreeBlock(BlockID::moon_bark, 0, 3, -2),
        TreeBlock(BlockID::moon_bark, -1, 3, -2), TreeBlock(BlockID::moon_bark, -1, 4, -2), TreeBlock(BlockID::moon_bark, -1, 4, -3), TreeBlock(BlockID::moon_bark, -1, 5, -3),
        TreeBlock(BlockID::moon_bark, -1, 6, -3), TreeBlock(BlockID::moon_bark, -1, 7, -3), TreeBlock(BlockID::moon_bark, -1, 6, -4), TreeBlock(BlockID::moon_bark, -1, 7, -4),
        TreeBlock(BlockID::moon_bark, -1, 8, -4), TreeBlock(BlockID::moon_bark, -1, 9, -4), TreeBlock(BlockID::moon_bark, -2, 7, -4), TreeBlock(BlockID::moon_bark, -2, 8, -4),
        TreeBlock(BlockID::moon_bark, -2, 9, -4), TreeBlock(BlockID::moon_bark, -2, 8, -5), TreeBlock(BlockID::moon_bark, -2, 9, -5), TreeBlock(BlockID::moon_bark, -2, 10, -5),
        TreeBlock(BlockID::moon_bark, -2, 11, -5), TreeBlock(BlockID::moon_bark, -2, 12, -5), TreeBlock(BlockID::moon_bark, -2, 13, -5), TreeBlock(BlockID::moon_bark, -2, 14, -5),
        TreeBlock(BlockID::moon_bark, -2, 15, -5), TreeBlock(BlockID::moon_bark, -1, 9, -5), TreeBlock(BlockID::moon_bark, -1, 10, -5), TreeBlock(BlockID::moon_bark, -1, 11, -5),
        TreeBlock(BlockID::moon_bark, -1, 14, -5), TreeBlock(BlockID::moon_bark, -1, 15, -5), TreeBlock(BlockID::moon_bark, -1, 15, -4), TreeBlock(BlockID::moon_bark, 2, 2, 0),
        TreeBlock(BlockID::moon_bark, 2, 3, 0), TreeBlock(BlockID::moon_bark, 2, 3, 1), TreeBlock(BlockID::moon_bark, 2, 4, 1), TreeBlock(BlockID::moon_bark, 3, 4, 1),
        TreeBlock(BlockID::moon_bark, 3, 5, 1), TreeBlock(BlockID::moon_bark, 3, 6, 1), TreeBlock(BlockID::moon_bark, 3, 7, 1), TreeBlock(BlockID::moon_bark, 3, 7, 2),
        TreeBlock(BlockID::moon_bark, 3, 8, 2), TreeBlock(BlockID::moon_bark, 3, 9, 2), TreeBlock(BlockID::moon_bark, 3, 10, 2), TreeBlock(BlockID::moon_bark, 3, 11, 2),
        TreeBlock(BlockID::moon_bark, 3, 12, 2), TreeBlock(BlockID::moon_bark, 4, 11, 2), TreeBlock(BlockID::moon_bark, 4, 12, 2), TreeBlock(BlockID::moon_bark, 4, 13, 2),
        TreeBlock(BlockID::moon_bark, 4, 14, 2), TreeBlock(BlockID::moon_bark, 5, 12, 2), TreeBlock(BlockID::moon_bark, 5, 13, 2), TreeBlock(BlockID::moon_bark, 5, 14, 2),
        TreeBlock(BlockID::moon_bark, 5, 14, 3), TreeBlock(BlockID::moon_bark, 4, 14, 3),

        TreeBlock(BlockID::light, 5, 16, 4), TreeBlock(BlockID::light, 4, 16, 3), TreeBlock(BlockID::light, 5, 16, 3), TreeBlock(BlockID::light, 6, 16, 3),
        TreeBlock(BlockID::light, 5, 16, 2), TreeBlock(BlockID::light, 4, 16, 2), TreeBlock(BlockID::light, -3, 17, -6), TreeBlock(BlockID::light, -3, 17, -5),
        TreeBlock(BlockID::light, -3, 17, -4), TreeBlock(BlockID::light, -2, 17, -6), TreeBlock(BlockID::light, -2, 17, -5), TreeBlock(BlockID::light, -2, 17, -4),
        TreeBlock(BlockID::light, -1, 17, -5),

        TreeBlock(BlockID::sulphur_ore, -1, 16, -6), TreeBlock(BlockID::sulphur_ore, -1, 16, -5), TreeBlock(BlockID::sulphur_ore, -1, 16, -4), TreeBlock(BlockID::sulphur_ore, -2, 16, -6),
        TreeBlock(BlockID::sulphur_ore, -2, 16, -5), TreeBlock(BlockID::sulphur_ore, -2, 16, -4), TreeBlock(BlockID::sulphur_ore, -3, 16, -5), TreeBlock(BlockID::sulphur_ore, -3, 16, -4),
        TreeBlock(BlockID::sulphur_ore, -1, 18, -5), TreeBlock(BlockID::sulphur_ore, -1, 18, -4), TreeBlock(BlockID::sulphur_ore, -2, 18, -6), TreeBlock(BlockID::sulphur_ore, -2, 18, -5),
        TreeBlock(BlockID::sulphur_ore, -2, 18, -4), TreeBlock(BlockID::sulphur_ore, -3, 18, -6), TreeBlock(BlockID::sulphur_ore, -3, 18, -5),

        TreeBlock(BlockID::chalchanthite, 4, 15, 4), TreeBlock(BlockID::chalchanthite, 4, 15, 3), TreeBlock(BlockID::chalchanthite, 4, 15, 2), TreeBlock(BlockID::chalchanthite, 5, 15, 4),
        TreeBlock(BlockID::chalchanthite, 5, 15, 3), TreeBlock(BlockID::chalchanthite, 5, 15, 2), TreeBlock(BlockID::chalchanthite, 6, 15, 3), TreeBlock(BlockID::chalchanthite, 6, 15, 2),
        TreeBlock(BlockID::chalchanthite, 4, 17, 3), TreeBlock(BlockID::chalchanthite, 5, 17, 4), TreeBlock(BlockID::chalchanthite, 5, 17, 3), TreeBlock(BlockID::chalchanthite, 5, 17, 2),
        TreeBlock(BlockID::chalchanthite, 6, 17, 4), TreeBlock(BlockID::chalchanthite, 6, 17, 3)
    },
    {
        TreeBlock(BlockID::moon_bark, 6, 0, 0), // max extent

        TreeBlock(BlockID::moon_bark, 1, 1, 0), TreeBlock(BlockID::moon_bark, 0, 1, -1), TreeBlock(BlockID::moon_bark, 1, 1, -1), TreeBlock(BlockID::moon_bark, 2, 1, -1),
        TreeBlock(BlockID::moon_bark, 0, 2, -1), TreeBlock(BlockID::moon_bark, 1, 2, -1), TreeBlock(BlockID::moon_bark, 2, 2, -1), TreeBlock(BlockID::moon_bark, 2, 1, -2),
        TreeBlock(BlockID::moon_bark, 1, 1, -2), TreeBlock(BlockID::moon_bark, 1, 2, -2), TreeBlock(BlockID::moon_bark, 0, 2, -2), TreeBlock(BlockID::moon_bark, 0, 3, -2),
        TreeBlock(BlockID::moon_bark, -1, 3, -2), TreeBlock(BlockID::moon_bark, -1, 4, -2), TreeBlock(BlockID::moon_bark, -1, 4, -3), TreeBlock(BlockID::moon_bark, -1, 5, -3),
        TreeBlock(BlockID::moon_bark, -1, 6, -3), TreeBlock(BlockID::moon_bark, -1, 7, -3), TreeBlock(BlockID::moon_bark, -1, 6, -4), TreeBlock(BlockID::moon_bark, -1, 7, -4),
        TreeBlock(BlockID::moon_bark, -1, 8, -4), TreeBlock(BlockID::moon_bark, -1, 9, -4), TreeBlock(BlockID::moon_bark, -2, 7, -4), TreeBlock(BlockID::moon_bark, -2, 8, -4),
        TreeBlock(BlockID::moon_bark, -2, 9, -4), TreeBlock(BlockID::moon_bark, -2, 8, -5), TreeBlock(BlockID::moon_bark, -2, 9, -5), TreeBlock(BlockID::moon_bark, -2, 10, -5),
        TreeBlock(BlockID::moon_bark, -2, 11, -5), TreeBlock(BlockID::moon_bark, -2, 12, -5), TreeBlock(BlockID::moon_bark, -2, 13, -5), TreeBlock(BlockID::moon_bark, -2, 14, -5),
        TreeBlock(BlockID::moon_bark, -2, 15, -5), TreeBlock(BlockID::moon_bark, -1, 9, -5), TreeBlock(BlockID::moon_bark, -1, 10, -5), TreeBlock(BlockID::moon_bark, -1, 11, -5),
        TreeBlock(BlockID::moon_bark, -1, 14, -5), TreeBlock(BlockID::moon_bark, -1, 15, -5), TreeBlock(BlockID::moon_bark, -1, 15, -4), TreeBlock(BlockID::moon_bark, 2, 2, 0),
        TreeBlock(BlockID::moon_bark, 2, 3, 0), TreeBlock(BlockID::moon_bark, 2, 3, 1), TreeBlock(BlockID::moon_bark, 2, 4, 1), TreeBlock(BlockID::moon_bark, 3, 4, 1),
        TreeBlock(BlockID::moon_bark, 3, 5, 1), TreeBlock(BlockID::moon_bark, 3, 6, 1), TreeBlock(BlockID::moon_bark, 3, 7, 1), TreeBlock(BlockID::moon_bark, 3, 7, 2),
        TreeBlock(BlockID::moon_bark, 3, 8, 2), TreeBlock(BlockID::moon_bark, 3, 9, 2), TreeBlock(BlockID::moon_bark, 3, 10, 2), TreeBlock(BlockID::moon_bark, 3, 11, 2),
        TreeBlock(BlockID::moon_bark, 3, 12, 2), TreeBlock(BlockID::moon_bark, 4, 11, 2), TreeBlock(BlockID::moon_bark, 4, 12, 2), TreeBlock(BlockID::moon_bark, 4, 13, 2),
        TreeBlock(BlockID::moon_bark, 4, 14, 2), TreeBlock(BlockID::moon_bark, 5, 12, 2), TreeBlock(BlockID::moon_bark, 5, 13, 2), TreeBlock(BlockID::moon_bark, 5, 14, 2),
        TreeBlock(BlockID::moon_bark, 5, 14, 3), TreeBlock(BlockID::moon_bark, 4, 14, 3),

        TreeBlock(BlockID::light, 5, 16, 4), TreeBlock(BlockID::light, 4, 16, 3), TreeBlock(BlockID::light, 5, 16, 3), TreeBlock(BlockID::light, 6, 16, 3),
        TreeBlock(BlockID::light, 5, 16, 2), TreeBlock(BlockID::light, 4, 16, 2), TreeBlock(BlockID::light, -3, 17, -6), TreeBlock(BlockID::light, -3, 17, -5),
        TreeBlock(BlockID::light, -3, 17, -4), TreeBlock(BlockID::light, -2, 17, -6), TreeBlock(BlockID::light, -2, 17, -5), TreeBlock(BlockID::light, -2, 17, -4),
        TreeBlock(BlockID::light, -1, 17, -5),

        TreeBlock(BlockID::feldspar, -1, 16, -6), TreeBlock(BlockID::feldspar, -1, 16, -5), TreeBlock(BlockID::feldspar, -1, 16, -4), TreeBlock(BlockID::feldspar, -2, 16, -6),
        TreeBlock(BlockID::feldspar, -2, 16, -5), TreeBlock(BlockID::feldspar, -2, 16, -4), TreeBlock(BlockID::feldspar, -3, 16, -5), TreeBlock(BlockID::feldspar, -3, 16, -4),
        TreeBlock(BlockID::feldspar, -1, 18, -5), TreeBlock(BlockID::feldspar, -1, 18, -4), TreeBlock(BlockID::feldspar, -2, 18, -6), TreeBlock(BlockID::feldspar, -2, 18, -5),
        TreeBlock(BlockID::feldspar, -2, 18, -4), TreeBlock(BlockID::feldspar, -3, 18, -6), TreeBlock(BlockID::feldspar, -3, 18, -5),

        TreeBlock(BlockID::sulphur_ore, 4, 15, 4), TreeBlock(BlockID::sulphur_ore, 4, 15, 3), TreeBlock(BlockID::sulphur_ore, 4, 15, 2), TreeBlock(BlockID::sulphur_ore, 5, 15, 4),
        TreeBlock(BlockID::sulphur_ore, 5, 15, 3), TreeBlock(BlockID::sulphur_ore, 5, 15, 2), TreeBlock(BlockID::sulphur_ore, 6, 15, 3), TreeBlock(BlockID::sulphur_ore, 6, 15, 2),
        TreeBlock(BlockID::sulphur_ore, 4, 17, 3), TreeBlock(BlockID::sulphur_ore, 5, 17, 4), TreeBlock(BlockID::sulphur_ore, 5, 17, 3), TreeBlock(BlockID::sulphur_ore, 5, 17, 2),
        TreeBlock(BlockID::sulphur_ore, 6, 17, 4), TreeBlock(BlockID::sulphur_ore, 6, 17, 3)
    },
    {
        TreeBlock(BlockID::moon_bark, 6, 0, 0), // max extent

        TreeBlock(BlockID::moon_bark, 1, 1, 0), TreeBlock(BlockID::moon_bark, 0, 1, -1), TreeBlock(BlockID::moon_bark, 1, 1, -1), TreeBlock(BlockID::moon_bark, 2, 1, -1),
        TreeBlock(BlockID::moon_bark, 0, 2, -1), TreeBlock(BlockID::moon_bark, 1, 2, -1), TreeBlock(BlockID::moon_bark, 2, 2, -1), TreeBlock(BlockID::moon_bark, 2, 1, -2),
        TreeBlock(BlockID::moon_bark, 1, 1, -2), TreeBlock(BlockID::moon_bark, 1, 2, -2), TreeBlock(BlockID::moon_bark, 0, 2, -2), TreeBlock(BlockID::moon_bark, 0, 3, -2),
        TreeBlock(BlockID::moon_bark, -1, 3, -2), TreeBlock(BlockID::moon_bark, -1, 4, -2), TreeBlock(BlockID::moon_bark, -1, 4, -3), TreeBlock(BlockID::moon_bark, -1, 5, -3),
        TreeBlock(BlockID::moon_bark, -1, 6, -3), TreeBlock(BlockID::moon_bark, -1, 7, -3), TreeBlock(BlockID::moon_bark, -1, 6, -4), TreeBlock(BlockID::moon_bark, -1, 7, -4),
        TreeBlock(BlockID::moon_bark, -1, 8, -4), TreeBlock(BlockID::moon_bark, -1, 9, -4), TreeBlock(BlockID::moon_bark, -2, 7, -4), TreeBlock(BlockID::moon_bark, -2, 8, -4),
        TreeBlock(BlockID::moon_bark, -2, 9, -4), TreeBlock(BlockID::moon_bark, -2, 8, -5), TreeBlock(BlockID::moon_bark, -2, 9, -5), TreeBlock(BlockID::moon_bark, -2, 10, -5),
        TreeBlock(BlockID::moon_bark, -2, 11, -5), TreeBlock(BlockID::moon_bark, -2, 12, -5), TreeBlock(BlockID::moon_bark, -2, 13, -5), TreeBlock(BlockID::moon_bark, -2, 14, -5),
        TreeBlock(BlockID::moon_bark, -2, 15, -5), TreeBlock(BlockID::moon_bark, -1, 9, -5), TreeBlock(BlockID::moon_bark, -1, 10, -5), TreeBlock(BlockID::moon_bark, -1, 11, -5),
        TreeBlock(BlockID::moon_bark, -1, 14, -5), TreeBlock(BlockID::moon_bark, -1, 15, -5), TreeBlock(BlockID::moon_bark, -1, 15, -4), TreeBlock(BlockID::moon_bark, 2, 2, 0),
        TreeBlock(BlockID::moon_bark, 2, 3, 0), TreeBlock(BlockID::moon_bark, 2, 3, 1), TreeBlock(BlockID::moon_bark, 2, 4, 1), TreeBlock(BlockID::moon_bark, 3, 4, 1),
        TreeBlock(BlockID::moon_bark, 3, 5, 1), TreeBlock(BlockID::moon_bark, 3, 6, 1), TreeBlock(BlockID::moon_bark, 3, 7, 1), TreeBlock(BlockID::moon_bark, 3, 7, 2),
        TreeBlock(BlockID::moon_bark, 3, 8, 2), TreeBlock(BlockID::moon_bark, 3, 9, 2), TreeBlock(BlockID::moon_bark, 3, 10, 2), TreeBlock(BlockID::moon_bark, 3, 11, 2),
        TreeBlock(BlockID::moon_bark, 3, 12, 2), TreeBlock(BlockID::moon_bark, 4, 11, 2), TreeBlock(BlockID::moon_bark, 4, 12, 2), TreeBlock(BlockID::moon_bark, 4, 13, 2),
        TreeBlock(BlockID::moon_bark, 4, 14, 2), TreeBlock(BlockID::moon_bark, 5, 12, 2), TreeBlock(BlockID::moon_bark, 5, 13, 2), TreeBlock(BlockID::moon_bark, 5, 14, 2),
        TreeBlock(BlockID::moon_bark, 5, 14, 3), TreeBlock(BlockID::moon_bark, 4, 14, 3),

        TreeBlock(BlockID::light, 5, 16, 4), TreeBlock(BlockID::light, 4, 16, 3), TreeBlock(BlockID::light, 5, 16, 3), TreeBlock(BlockID::light, 6, 16, 3),
        TreeBlock(BlockID::light, 5, 16, 2), TreeBlock(BlockID::light, 4, 16, 2), TreeBlock(BlockID::light, -3, 17, -6), TreeBlock(BlockID::light, -3, 17, -5),
        TreeBlock(BlockID::light, -3, 17, -4), TreeBlock(BlockID::light, -2, 17, -6), TreeBlock(BlockID::light, -2, 17, -5), TreeBlock(BlockID::light, -2, 17, -4),
        TreeBlock(BlockID::light, -1, 17, -5),

        TreeBlock(BlockID::sulphur_ore, -1, 16, -6), TreeBlock(BlockID::sulphur_ore, -1, 16, -5), TreeBlock(BlockID::sulphur_ore, -1, 16, -4), TreeBlock(BlockID::sulphur_ore, -2, 16, -6),
        TreeBlock(BlockID::sulphur_ore, -2, 16, -5), TreeBlock(BlockID::sulphur_ore, -2, 16, -4), TreeBlock(BlockID::sulphur_ore, -3, 16, -5), TreeBlock(BlockID::sulphur_ore, -3, 16, -4),
        TreeBlock(BlockID::sulphur_ore, -1, 18, -5), TreeBlock(BlockID::sulphur_ore, -1, 18, -4), TreeBlock(BlockID::sulphur_ore, -2, 18, -6), TreeBlock(BlockID::sulphur_ore, -2, 18, -5),
        TreeBlock(BlockID::sulphur_ore, -2, 18, -4), TreeBlock(BlockID::sulphur_ore, -3, 18, -6), TreeBlock(BlockID::sulphur_ore, -3, 18, -5),

        TreeBlock(BlockID::feldspar, 4, 15, 4), TreeBlock(BlockID::feldspar, 4, 15, 3), TreeBlock(BlockID::feldspar, 4, 15, 2), TreeBlock(BlockID::feldspar, 5, 15, 4),
        TreeBlock(BlockID::feldspar, 5, 15, 3), TreeBlock(BlockID::feldspar, 5, 15, 2), TreeBlock(BlockID::feldspar, 6, 15, 3), TreeBlock(BlockID::feldspar, 6, 15, 2),
        TreeBlock(BlockID::feldspar, 4, 17, 3), TreeBlock(BlockID::feldspar, 5, 17, 4), TreeBlock(BlockID::feldspar, 5, 17, 3), TreeBlock(BlockID::feldspar, 5, 17, 2),
        TreeBlock(BlockID::feldspar, 6, 17, 4), TreeBlock(BlockID::feldspar, 6, 17, 3)
    },
    {
        TreeBlock(BlockID::moon_bark, 6, 0, 0), // max extent

        TreeBlock(BlockID::moon_bark, 1, 1, 0), TreeBlock(BlockID::moon_bark, 0, 1, -1), TreeBlock(BlockID::moon_bark, 1, 1, -1), TreeBlock(BlockID::moon_bark, 2, 1, -1),
        TreeBlock(BlockID::moon_bark, 0, 2, -1), TreeBlock(BlockID::moon_bark, 1, 2, -1), TreeBlock(BlockID::moon_bark, 2, 2, -1), TreeBlock(BlockID::moon_bark, 2, 1, -2),
        TreeBlock(BlockID::moon_bark, 1, 1, -2), TreeBlock(BlockID::moon_bark, 1, 2, -2), TreeBlock(BlockID::moon_bark, 0, 2, -2), TreeBlock(BlockID::moon_bark, 0, 3, -2),
        TreeBlock(BlockID::moon_bark, -1, 3, -2), TreeBlock(BlockID::moon_bark, -1, 4, -2), TreeBlock(BlockID::moon_bark, -1, 4, -3), TreeBlock(BlockID::moon_bark, -1, 5, -3),
        TreeBlock(BlockID::moon_bark, -1, 6, -3), TreeBlock(BlockID::moon_bark, -1, 7, -3), TreeBlock(BlockID::moon_bark, -1, 6, -4), TreeBlock(BlockID::moon_bark, -1, 7, -4),
        TreeBlock(BlockID::moon_bark, -1, 8, -4), TreeBlock(BlockID::moon_bark, -1, 9, -4), TreeBlock(BlockID::moon_bark, -2, 7, -4), TreeBlock(BlockID::moon_bark, -2, 8, -4),
        TreeBlock(BlockID::moon_bark, -2, 9, -4), TreeBlock(BlockID::moon_bark, -2, 8, -5), TreeBlock(BlockID::moon_bark, -2, 9, -5), TreeBlock(BlockID::moon_bark, -2, 10, -5),
        TreeBlock(BlockID::moon_bark, -2, 11, -5), TreeBlock(BlockID::moon_bark, -2, 12, -5), TreeBlock(BlockID::moon_bark, -2, 13, -5), TreeBlock(BlockID::moon_bark, -2, 14, -5),
        TreeBlock(BlockID::moon_bark, -2, 15, -5), TreeBlock(BlockID::moon_bark, -1, 9, -5), TreeBlock(BlockID::moon_bark, -1, 10, -5), TreeBlock(BlockID::moon_bark, -1, 11, -5),
        TreeBlock(BlockID::moon_bark, -1, 14, -5), TreeBlock(BlockID::moon_bark, -1, 15, -5), TreeBlock(BlockID::moon_bark, -1, 15, -4), TreeBlock(BlockID::moon_bark, 2, 2, 0),
        TreeBlock(BlockID::moon_bark, 2, 3, 0), TreeBlock(BlockID::moon_bark, 2, 3, 1), TreeBlock(BlockID::moon_bark, 2, 4, 1), TreeBlock(BlockID::moon_bark, 3, 4, 1),
        TreeBlock(BlockID::moon_bark, 3, 5, 1), TreeBlock(BlockID::moon_bark, 3, 6, 1), TreeBlock(BlockID::moon_bark, 3, 7, 1), TreeBlock(BlockID::moon_bark, 3, 7, 2),
        TreeBlock(BlockID::moon_bark, 3, 8, 2), TreeBlock(BlockID::moon_bark, 3, 9, 2), TreeBlock(BlockID::moon_bark, 3, 10, 2), TreeBlock(BlockID::moon_bark, 3, 11, 2),
        TreeBlock(BlockID::moon_bark, 3, 12, 2), TreeBlock(BlockID::moon_bark, 4, 11, 2), TreeBlock(BlockID::moon_bark, 4, 12, 2), TreeBlock(BlockID::moon_bark, 4, 13, 2),
        TreeBlock(BlockID::moon_bark, 4, 14, 2), TreeBlock(BlockID::moon_bark, 5, 12, 2), TreeBlock(BlockID::moon_bark, 5, 13, 2), TreeBlock(BlockID::moon_bark, 5, 14, 2),
        TreeBlock(BlockID::moon_bark, 5, 14, 3), TreeBlock(BlockID::moon_bark, 4, 14, 3),

        TreeBlock(BlockID::light, 5, 16, 4), TreeBlock(BlockID::light, 4, 16, 3), TreeBlock(BlockID::light, 5, 16, 3), TreeBlock(BlockID::light, 6, 16, 3),
        TreeBlock(BlockID::light, 5, 16, 2), TreeBlock(BlockID::light, 4, 16, 2), TreeBlock(BlockID::light, -3, 17, -6), TreeBlock(BlockID::light, -3, 17, -5),
        TreeBlock(BlockID::light, -3, 17, -4), TreeBlock(BlockID::light, -2, 17, -6), TreeBlock(BlockID::light, -2, 17, -5), TreeBlock(BlockID::light, -2, 17, -4),
        TreeBlock(BlockID::light, -1, 17, -5),

        TreeBlock(BlockID::sulphur_ore, -1, 16, -6), TreeBlock(BlockID::sulphur_ore, -1, 16, -5), TreeBlock(BlockID::sulphur_ore, -1, 16, -4), TreeBlock(BlockID::sulphur_ore, -2, 16, -6),
        TreeBlock(BlockID::sulphur_ore, -2, 16, -5), TreeBlock(BlockID::sulphur_ore, -2, 16, -4), TreeBlock(BlockID::sulphur_ore, -3, 16, -5), TreeBlock(BlockID::sulphur_ore, -3, 16, -4),
        TreeBlock(BlockID::sulphur_ore, -1, 18, -5), TreeBlock(BlockID::sulphur_ore, -1, 18, -4), TreeBlock(BlockID::sulphur_ore, -2, 18, -6), TreeBlock(BlockID::sulphur_ore, -2, 18, -5),
        TreeBlock(BlockID::sulphur_ore, -2, 18, -4), TreeBlock(BlockID::sulphur_ore, -3, 18, -6), TreeBlock(BlockID::sulphur_ore, -3, 18, -5),

        TreeBlock(BlockID::sulphur_ore, 4, 15, 4), TreeBlock(BlockID::sulphur_ore, 4, 15, 3), TreeBlock(BlockID::sulphur_ore, 4, 15, 2), TreeBlock(BlockID::sulphur_ore, 5, 15, 4),
        TreeBlock(BlockID::sulphur_ore, 5, 15, 3), TreeBlock(BlockID::sulphur_ore, 5, 15, 2), TreeBlock(BlockID::sulphur_ore, 6, 15, 3), TreeBlock(BlockID::sulphur_ore, 6, 15, 2),
        TreeBlock(BlockID::sulphur_ore, 4, 17, 3), TreeBlock(BlockID::sulphur_ore, 5, 17, 4), TreeBlock(BlockID::sulphur_ore, 5, 17, 3), TreeBlock(BlockID::sulphur_ore, 5, 17, 2),
        TreeBlock(BlockID::sulphur_ore, 6, 17, 4), TreeBlock(BlockID::sulphur_ore, 6, 17, 3)
    },
    {
        TreeBlock(BlockID::moon_bark, 3, 0, 0), // max extent

        TreeBlock(BlockID::moon_bark, -2, 1, 0), TreeBlock(BlockID::moon_bark, -3, 1, -1), TreeBlock(BlockID::moon_bark, -2, 1, -1), TreeBlock(BlockID::moon_bark, -2, 2, -1),
        TreeBlock(BlockID::moon_bark, -1, 2, -1), TreeBlock(BlockID::moon_bark, -1, 3, -1), TreeBlock(BlockID::moon_bark, -1, 3, 0), TreeBlock(BlockID::moon_bark, -1, 4, 0),
        TreeBlock(BlockID::moon_bark, 0, 4, 0), TreeBlock(BlockID::moon_bark, 0, 4, -1), TreeBlock(BlockID::moon_bark, 0, 5, 0), TreeBlock(BlockID::moon_bark, 0, 5, -1),
        TreeBlock(BlockID::moon_bark, 0, 6, 0), TreeBlock(BlockID::moon_bark, 1, 5, 0), TreeBlock(BlockID::moon_bark, 1, 6, 0), TreeBlock(BlockID::moon_bark, 1, 7, 0),
        TreeBlock(BlockID::moon_bark, 1, 8, 0), TreeBlock(BlockID::moon_bark, 1, 9, 0), TreeBlock(BlockID::moon_bark, 1, 7, -1), TreeBlock(BlockID::moon_bark, 1, 8, -1),
        TreeBlock(BlockID::moon_bark, 1, 9, -1), TreeBlock(BlockID::moon_bark, 1, 10, -1), TreeBlock(BlockID::moon_bark, 2, 10, -1), TreeBlock(BlockID::moon_bark, 2, 11, -1),
        TreeBlock(BlockID::moon_bark, 2, 12, -1), TreeBlock(BlockID::moon_bark, 2, 13, -1), TreeBlock(BlockID::moon_bark, 2, 13, 0), TreeBlock(BlockID::moon_bark, 2, 14, 0),
        TreeBlock(BlockID::moon_bark, 2, 15, 0), TreeBlock(BlockID::moon_bark, 3, 15, 0), TreeBlock(BlockID::moon_bark, 3, 16, 0), TreeBlock(BlockID::moon_bark, 3, 17, 0),
        TreeBlock(BlockID::moon_bark, 3, 18, 0), TreeBlock(BlockID::moon_bark, 3, 18, 1), TreeBlock(BlockID::moon_bark, 3, 19, 1), TreeBlock(BlockID::moon_bark, 2, 19, 1),

        TreeBlock(BlockID::feldspar, 1, 20, 0), TreeBlock(BlockID::feldspar, 2, 20, 0), TreeBlock(BlockID::feldspar, 1, 20, 1), TreeBlock(BlockID::feldspar, 2, 20, 1),
        TreeBlock(BlockID::feldspar, 3, 20, 1), TreeBlock(BlockID::feldspar, 1, 20, 2), TreeBlock(BlockID::feldspar, 3, 20, 2), TreeBlock(BlockID::feldspar, 1, 22, 0),
        TreeBlock(BlockID::feldspar, 2, 22, 0), TreeBlock(BlockID::feldspar, 3, 22, 0), TreeBlock(BlockID::feldspar, 1, 22, 1), TreeBlock(BlockID::feldspar, 2, 22, 1),
        TreeBlock(BlockID::feldspar, 3, 22, 1), TreeBlock(BlockID::feldspar, 2, 22, 2), TreeBlock(BlockID::feldspar, 3, 22, 2),

        TreeBlock(BlockID::light, 1, 21, 0), TreeBlock(BlockID::light, 1, 21, 1), TreeBlock(BlockID::light, 1, 21, 2), TreeBlock(BlockID::light, 2, 21, 0),
        TreeBlock(BlockID::light, 2, 21, 1), TreeBlock(BlockID::light, 2, 21, 2), TreeBlock(BlockID::light, 3, 21, 0), TreeBlock(BlockID::light, 3, 21, 1)
    },
    {
        TreeBlock(BlockID::moon_bark, 3, 0, 0), // max extent

        TreeBlock(BlockID::moon_bark, -2, 1, 0), TreeBlock(BlockID::moon_bark, -3, 1, -1), TreeBlock(BlockID::moon_bark, -2, 1, -1), TreeBlock(BlockID::moon_bark, -2, 2, -1),
        TreeBlock(BlockID::moon_bark, -1, 2, -1), TreeBlock(BlockID::moon_bark, -1, 3, -1), TreeBlock(BlockID::moon_bark, -1, 3, 0), TreeBlock(BlockID::moon_bark, -1, 4, 0),
        TreeBlock(BlockID::moon_bark, 0, 4, 0), TreeBlock(BlockID::moon_bark, 0, 4, -1), TreeBlock(BlockID::moon_bark, 0, 5, 0), TreeBlock(BlockID::moon_bark, 0, 5, -1),
        TreeBlock(BlockID::moon_bark, 0, 6, 0), TreeBlock(BlockID::moon_bark, 1, 5, 0), TreeBlock(BlockID::moon_bark, 1, 6, 0), TreeBlock(BlockID::moon_bark, 1, 7, 0),
        TreeBlock(BlockID::moon_bark, 1, 8, 0), TreeBlock(BlockID::moon_bark, 1, 9, 0), TreeBlock(BlockID::moon_bark, 1, 7, -1), TreeBlock(BlockID::moon_bark, 1, 8, -1),
        TreeBlock(BlockID::moon_bark, 1, 9, -1), TreeBlock(BlockID::moon_bark, 1, 10, -1), TreeBlock(BlockID::moon_bark, 2, 10, -1), TreeBlock(BlockID::moon_bark, 2, 11, -1),
        TreeBlock(BlockID::moon_bark, 2, 12, -1), TreeBlock(BlockID::moon_bark, 2, 13, -1), TreeBlock(BlockID::moon_bark, 2, 13, 0), TreeBlock(BlockID::moon_bark, 2, 14, 0),
        TreeBlock(BlockID::moon_bark, 2, 15, 0), TreeBlock(BlockID::moon_bark, 3, 15, 0), TreeBlock(BlockID::moon_bark, 3, 16, 0), TreeBlock(BlockID::moon_bark, 3, 17, 0),
        TreeBlock(BlockID::moon_bark, 3, 18, 0), TreeBlock(BlockID::moon_bark, 3, 18, 1), TreeBlock(BlockID::moon_bark, 3, 19, 1), TreeBlock(BlockID::moon_bark, 2, 19, 1),

        TreeBlock(BlockID::chalchanthite, 1, 20, 0), TreeBlock(BlockID::chalchanthite, 2, 20, 0), TreeBlock(BlockID::chalchanthite, 1, 20, 1), TreeBlock(BlockID::chalchanthite, 2, 20, 1),
        TreeBlock(BlockID::chalchanthite, 3, 20, 1), TreeBlock(BlockID::chalchanthite, 1, 20, 2), TreeBlock(BlockID::chalchanthite, 3, 20, 2), TreeBlock(BlockID::chalchanthite, 1, 22, 0),
        TreeBlock(BlockID::chalchanthite, 2, 22, 0), TreeBlock(BlockID::chalchanthite, 3, 22, 0), TreeBlock(BlockID::chalchanthite, 1, 22, 1), TreeBlock(BlockID::chalchanthite, 2, 22, 1),
        TreeBlock(BlockID::chalchanthite, 3, 22, 1),

        TreeBlock(BlockID::light, 1, 21, 0), TreeBlock(BlockID::light, 1, 21, 1), TreeBlock(BlockID::light, 1, 21, 2), TreeBlock(BlockID::light, 2, 21, 0),
        TreeBlock(BlockID::light, 2, 21, 1), TreeBlock(BlockID::light, 2, 21, 2), TreeBlock(BlockID::light, 3, 21, 0), TreeBlock(BlockID::light, 3, 21, 1)
    },
    {
        TreeBlock(BlockID::moon_bark, 3, 0, 0), // max extent

        TreeBlock(BlockID::moon_bark, -2, 1, 0), TreeBlock(BlockID::moon_bark, -3, 1, -1), TreeBlock(BlockID::moon_bark, -2, 1, -1), TreeBlock(BlockID::moon_bark, -2, 2, -1),
        TreeBlock(BlockID::moon_bark, -1, 2, -1), TreeBlock(BlockID::moon_bark, -1, 3, -1), TreeBlock(BlockID::moon_bark, -1, 3, 0), TreeBlock(BlockID::moon_bark, -1, 4, 0),
        TreeBlock(BlockID::moon_bark, 0, 4, 0), TreeBlock(BlockID::moon_bark, 0, 4, -1), TreeBlock(BlockID::moon_bark, 0, 5, 0), TreeBlock(BlockID::moon_bark, 0, 5, -1),
        TreeBlock(BlockID::moon_bark, 0, 6, 0), TreeBlock(BlockID::moon_bark, 1, 5, 0), TreeBlock(BlockID::moon_bark, 1, 6, 0), TreeBlock(BlockID::moon_bark, 1, 7, 0),
        TreeBlock(BlockID::moon_bark, 1, 8, 0), TreeBlock(BlockID::moon_bark, 1, 9, 0), TreeBlock(BlockID::moon_bark, 1, 7, -1), TreeBlock(BlockID::moon_bark, 1, 8, -1),
        TreeBlock(BlockID::moon_bark, 1, 9, -1), TreeBlock(BlockID::moon_bark, 1, 10, -1), TreeBlock(BlockID::moon_bark, 2, 10, -1), TreeBlock(BlockID::moon_bark, 2, 11, -1),
        TreeBlock(BlockID::moon_bark, 2, 12, -1), TreeBlock(BlockID::moon_bark, 2, 13, -1), TreeBlock(BlockID::moon_bark, 2, 13, 0), TreeBlock(BlockID::moon_bark, 2, 14, 0),
        TreeBlock(BlockID::moon_bark, 2, 15, 0), TreeBlock(BlockID::moon_bark, 3, 15, 0), TreeBlock(BlockID::moon_bark, 3, 16, 0), TreeBlock(BlockID::moon_bark, 3, 17, 0),
        TreeBlock(BlockID::moon_bark, 3, 18, 0), TreeBlock(BlockID::moon_bark, 3, 18, 1), TreeBlock(BlockID::moon_bark, 3, 19, 1), TreeBlock(BlockID::moon_bark, 2, 19, 1),

        TreeBlock(BlockID::sulphur_ore, 2, 20, 0), TreeBlock(BlockID::sulphur_ore, 1, 20, 1), TreeBlock(BlockID::sulphur_ore, 2, 20, 1), TreeBlock(BlockID::sulphur_ore, 3, 20, 1),
        TreeBlock(BlockID::sulphur_ore, 1, 20, 2), TreeBlock(BlockID::sulphur_ore, 3, 20, 2), TreeBlock(BlockID::sulphur_ore, 1, 22, 0), TreeBlock(BlockID::sulphur_ore, 2, 22, 0),
        TreeBlock(BlockID::sulphur_ore, 3, 22, 0), TreeBlock(BlockID::sulphur_ore, 1, 22, 1), TreeBlock(BlockID::sulphur_ore, 2, 22, 1), TreeBlock(BlockID::sulphur_ore, 3, 22, 1),
        TreeBlock(BlockID::sulphur_ore, 2, 22, 2), TreeBlock(BlockID::sulphur_ore, 3, 22, 2),

        TreeBlock(BlockID::light, 1, 21, 0), TreeBlock(BlockID::light, 1, 21, 1), TreeBlock(BlockID::light, 1, 21, 2), TreeBlock(BlockID::light, 2, 21, 0),
        TreeBlock(BlockID::light, 2, 21, 1), TreeBlock(BlockID::light, 2, 21, 2), TreeBlock(BlockID::light, 3, 21, 0), TreeBlock(BlockID::light, 3, 21, 1)
    },
};

std::vector<std::vector<TreeBlock>> SPIRAL_LIGHT_TREE_SHAPES = {
    {
        TreeBlock(BlockID::moon_wood, 5, 0, 0), // max extent

        TreeBlock(BlockID::moon_wood, -1, 1, 0), TreeBlock(BlockID::moon_wood, -2, 1, 0), TreeBlock(BlockID::moon_wood, -3, 1, 0), TreeBlock(BlockID::moon_wood, -4, 1, 0),
        TreeBlock(BlockID::moon_wood, 0, 1, 1), TreeBlock(BlockID::moon_wood, -1, 1, 1), TreeBlock(BlockID::moon_wood, -2, 1, 1), TreeBlock(BlockID::moon_wood, -3, 1, 1),
        TreeBlock(BlockID::moon_wood, -1, 2, 1), TreeBlock(BlockID::moon_wood, -2, 2, 1), TreeBlock(BlockID::moon_wood, -3, 2, 1), TreeBlock(BlockID::moon_wood, -4, 2, 1),
        TreeBlock(BlockID::moon_wood, 0, 1, 2), TreeBlock(BlockID::moon_wood, -1, 1, 2), TreeBlock(BlockID::moon_wood, -2, 1, 2), TreeBlock(BlockID::moon_wood, -3, 1, 2),
        TreeBlock(BlockID::moon_wood, 0, 2, 2), TreeBlock(BlockID::moon_wood, -1, 2, 2), TreeBlock(BlockID::moon_wood, -2, 1, 3), TreeBlock(BlockID::moon_wood, -2, 2, 3),
        TreeBlock(BlockID::moon_wood, -2, 3, 3), TreeBlock(BlockID::moon_wood, -2, 4, 3), TreeBlock(BlockID::moon_wood, -2, 5, 3), TreeBlock(BlockID::moon_wood, -2, 6, 3),
        TreeBlock(BlockID::moon_wood, -2, 7, 3), TreeBlock(BlockID::moon_wood, -3, 4, 3), TreeBlock(BlockID::moon_wood, -3, 5, 3), TreeBlock(BlockID::moon_wood, -3, 6, 3),
        TreeBlock(BlockID::moon_wood, -2, 3, 4), TreeBlock(BlockID::moon_wood, -2, 4, 4), TreeBlock(BlockID::moon_wood, -2, 5, 4), TreeBlock(BlockID::moon_wood, -2, 6, 4),
        TreeBlock(BlockID::moon_wood, -1, 2, 4), TreeBlock(BlockID::moon_wood, -1, 3, 4), TreeBlock(BlockID::moon_wood, -1, 4, 4), TreeBlock(BlockID::moon_wood, -1, 1, 3),
        TreeBlock(BlockID::moon_wood, -1, 2, 3), TreeBlock(BlockID::moon_wood, -1, 3, 3), TreeBlock(BlockID::moon_wood, -1, 4, 3), TreeBlock(BlockID::moon_wood, 0, 2, 3),
        TreeBlock(BlockID::moon_wood, 0, 3, 3), TreeBlock(BlockID::moon_wood, -2, 6, 2), TreeBlock(BlockID::moon_wood, -1, 7, 2), TreeBlock(BlockID::moon_wood, -1, 8, 2),
        TreeBlock(BlockID::moon_wood, 0, 8, 2), TreeBlock(BlockID::moon_wood, 0, 8, 3), TreeBlock(BlockID::moon_wood, 0, 9, 3), TreeBlock(BlockID::moon_wood, -1, 6, 3),
        TreeBlock(BlockID::moon_wood, 1, 9, 3), TreeBlock(BlockID::moon_wood, 0, 9, 4), TreeBlock(BlockID::moon_wood, 0, 10, 4), TreeBlock(BlockID::moon_wood, 1, 10, 4),
        TreeBlock(BlockID::moon_wood, 0, 10, 5), TreeBlock(BlockID::moon_wood, 0, 11, 5), TreeBlock(BlockID::moon_wood, -1, 11, 5), TreeBlock(BlockID::moon_wood, -2, 11, 5),
        TreeBlock(BlockID::moon_wood, -2, 12, 5), TreeBlock(BlockID::moon_wood, -2, 12, 4), TreeBlock(BlockID::moon_wood, -2, 13, 4), TreeBlock(BlockID::moon_wood, -2, 13, 3),
        TreeBlock(BlockID::moon_wood, -3, 13, 4), TreeBlock(BlockID::moon_wood, -3, 13, 3), TreeBlock(BlockID::moon_wood, -2, 14, 3), TreeBlock(BlockID::moon_wood, -2, 14, 2),
        TreeBlock(BlockID::moon_wood, -4, 3, 1), TreeBlock(BlockID::moon_wood, -4, 1, -1), TreeBlock(BlockID::moon_wood, -3, 1, -1), TreeBlock(BlockID::moon_wood, -4, 2, 0),
        TreeBlock(BlockID::moon_wood, -4, 3, 0), TreeBlock(BlockID::moon_wood, -4, 4, 0), TreeBlock(BlockID::moon_wood, -3, 2, 0), TreeBlock(BlockID::moon_wood, -3, 3, 0),
        TreeBlock(BlockID::moon_wood, -3, 4, 0), TreeBlock(BlockID::moon_wood, -3, 4, -1), TreeBlock(BlockID::moon_wood, -4, 4, -1), TreeBlock(BlockID::moon_wood, -3, 5, -1),
        TreeBlock(BlockID::moon_wood, -4, 5, -1), TreeBlock(BlockID::moon_wood, -3, 5, -2), TreeBlock(BlockID::moon_wood, -4, 5, -2), TreeBlock(BlockID::moon_wood, -3, 6, -2),
        TreeBlock(BlockID::moon_wood, -2, 6, -2), TreeBlock(BlockID::moon_wood, -3, 6, -3), TreeBlock(BlockID::moon_wood, -2, 6, -3), TreeBlock(BlockID::moon_wood, -2, 7, -3),
        TreeBlock(BlockID::moon_wood, -1, 7, -3), TreeBlock(BlockID::moon_wood, -1, 8, -3), TreeBlock(BlockID::moon_wood, 0, 8, -3), TreeBlock(BlockID::moon_wood, 0, 9, -3),
        TreeBlock(BlockID::moon_wood, 0, 9, -2), TreeBlock(BlockID::moon_wood, 1, 9, -3), TreeBlock(BlockID::moon_wood, 1, 9, -2), TreeBlock(BlockID::moon_wood, 1, 10, -2),
        TreeBlock(BlockID::moon_wood, 1, 10, -1), TreeBlock(BlockID::moon_wood, 1, 11, -1), TreeBlock(BlockID::moon_wood, 2, 11, -1), TreeBlock(BlockID::moon_wood, 1, 11, 0),
        TreeBlock(BlockID::moon_wood, 2, 11, 0), TreeBlock(BlockID::moon_wood, 1, 12, 0), TreeBlock(BlockID::moon_wood, 2, 12, 0), TreeBlock(BlockID::moon_wood, 1, 12, 1),
        TreeBlock(BlockID::moon_wood, 0, 13, 1), TreeBlock(BlockID::moon_wood, 0, 13, 2), TreeBlock(BlockID::moon_wood, 0, 14, 1),

        TreeBlock(BlockID::light, -1, 14, 3), TreeBlock(BlockID::light, 0, 14, 3), TreeBlock(BlockID::light, -1, 15, 3), TreeBlock(BlockID::light, 0, 15, 3),
        TreeBlock(BlockID::light, 1, 15, 3), TreeBlock(BlockID::light, -2, 15, 2), TreeBlock(BlockID::light, -1, 15, 2), TreeBlock(BlockID::light, 0, 15, 2),
        TreeBlock(BlockID::light, 1, 15, 2), TreeBlock(BlockID::light, 0, 15, 1), TreeBlock(BlockID::light, 0, 14, 2), TreeBlock(BlockID::light, -1, 14, 2),
        TreeBlock(BlockID::light, 0, 16, 2), TreeBlock(BlockID::light, -1, 16, 2), TreeBlock(BlockID::light, 1, 14, 2)
    },
    {
        TreeBlock(BlockID::moon_wood, 3, 0, 0), // max extent

        TreeBlock(BlockID::moon_wood, -1, 1, -1), TreeBlock(BlockID::moon_wood, -1, 2, -1), TreeBlock(BlockID::moon_wood, 0, 1, -2), TreeBlock(BlockID::moon_wood, 0, 2, -2),
        TreeBlock(BlockID::moon_wood, 0, 1, -3), TreeBlock(BlockID::moon_wood, -1, 1, -3), TreeBlock(BlockID::moon_wood, -2, 1, -3), TreeBlock(BlockID::moon_wood, -1, 2, -3),
        TreeBlock(BlockID::moon_wood, -2, 2, -3), TreeBlock(BlockID::moon_wood, -1, 1, -2), TreeBlock(BlockID::moon_wood, -2, 1, -2), TreeBlock(BlockID::moon_wood, -1, 2, -2),
        TreeBlock(BlockID::moon_wood, -2, 2, -2), TreeBlock(BlockID::moon_wood, -1, 3, -2), TreeBlock(BlockID::moon_wood, -2, 3, -2), TreeBlock(BlockID::moon_wood, -1, 4, -2),
        TreeBlock(BlockID::moon_wood, -1, 5, -2), TreeBlock(BlockID::moon_wood, -1, 6, -2), TreeBlock(BlockID::moon_wood, -1, 7, -2), TreeBlock(BlockID::moon_wood, -1, 8, -2),
        TreeBlock(BlockID::moon_wood, -1, 9, -2), TreeBlock(BlockID::moon_wood, -2, 4, -3), TreeBlock(BlockID::moon_wood, -1, 4, -3), TreeBlock(BlockID::moon_wood, -1, 5, -3),
        TreeBlock(BlockID::moon_wood, 0, 5, -3), TreeBlock(BlockID::moon_wood, 0, 4, -2), TreeBlock(BlockID::moon_wood, 0, 5, -2), TreeBlock(BlockID::moon_wood, 0, 6, -2),
        TreeBlock(BlockID::moon_wood, 0, 5, -1), TreeBlock(BlockID::moon_wood, 0, 6, -1), TreeBlock(BlockID::moon_wood, 0, 7, -1), TreeBlock(BlockID::moon_wood, -1, 6, -1),
        TreeBlock(BlockID::moon_wood, -1, 7, -1), TreeBlock(BlockID::moon_wood, -1, 8, -1), TreeBlock(BlockID::moon_wood, 0, 7, 0), TreeBlock(BlockID::moon_wood, 0, 7, 1),
        TreeBlock(BlockID::moon_wood, 0, 8, 1), TreeBlock(BlockID::moon_wood, 0, 8, 2), TreeBlock(BlockID::moon_wood, 0, 9, 2), TreeBlock(BlockID::moon_wood, -2, 7, -1),
        TreeBlock(BlockID::moon_wood, -2, 8, -1), TreeBlock(BlockID::moon_wood, -2, 9, -1), TreeBlock(BlockID::moon_wood, -3, 8, -1), TreeBlock(BlockID::moon_wood, -2, 8, -2),
        TreeBlock(BlockID::moon_wood, -2, 9, -2), TreeBlock(BlockID::moon_wood, -2, 10, -2), TreeBlock(BlockID::moon_wood, -3, 9, -2), TreeBlock(BlockID::moon_wood, -2, 9, -3),
        TreeBlock(BlockID::moon_wood, -2, 10, -3), TreeBlock(BlockID::moon_wood, -1, 10, -3), TreeBlock(BlockID::moon_wood, -1, 11, -3), TreeBlock(BlockID::moon_wood, 0, 11, -3),
        TreeBlock(BlockID::moon_wood, 0, 11, -2), TreeBlock(BlockID::moon_wood, 0, 12, -2), TreeBlock(BlockID::moon_wood, 0, 12, -1), TreeBlock(BlockID::moon_wood, 0, 13, -1),
        TreeBlock(BlockID::moon_wood, 0, 13, 0), TreeBlock(BlockID::moon_wood, -1, 13, 0), TreeBlock(BlockID::moon_wood, -2, 15, 0), TreeBlock(BlockID::moon_wood, -1, 14, 0),
        TreeBlock(BlockID::moon_wood, -2, 14, 0),

        TreeBlock(BlockID::light, -1, 8, 1), TreeBlock(BlockID::light, 1, 8, 1), TreeBlock(BlockID::light, -1, 9, 1), TreeBlock(BlockID::light, 0, 9, 1),
        TreeBlock(BlockID::light, 1, 9, 1), TreeBlock(BlockID::light, -1, 9, 2), TreeBlock(BlockID::light, 1, 9, 2), TreeBlock(BlockID::light, -1, 9, 3),
        TreeBlock(BlockID::light, 0, 9, 3), TreeBlock(BlockID::light, 1, 9, 3), TreeBlock(BlockID::light, -1, 8, 2), TreeBlock(BlockID::light, 0, 8, 3),
        TreeBlock(BlockID::light, 1, 8, 2), TreeBlock(BlockID::light, 1, 10, 2), TreeBlock(BlockID::light, 0, 10, 2), TreeBlock(BlockID::light, -1, 10, 2),
        TreeBlock(BlockID::light, 1, 10, 3), TreeBlock(BlockID::light, 0, 10, 3), TreeBlock(BlockID::light, 0, 10, 1), TreeBlock(BlockID::light, -1, 15, -1),
        TreeBlock(BlockID::light, -1, 15, 0), TreeBlock(BlockID::light, -1, 15, 1), TreeBlock(BlockID::light, -2, 15, -1), TreeBlock(BlockID::light, -2, 15, 1),
        TreeBlock(BlockID::light, -3, 15, -1), TreeBlock(BlockID::light, -3, 15, 0), TreeBlock(BlockID::light, -3, 15, 1), TreeBlock(BlockID::light, -1, 14, 1),
        TreeBlock(BlockID::light, -2, 14, 1), TreeBlock(BlockID::light, -3, 14, 1), TreeBlock(BlockID::light, -2, 14, -1), TreeBlock(BlockID::light, -3, 14, 0),
        TreeBlock(BlockID::light, -1, 16, 0), TreeBlock(BlockID::light, -2, 16, 0), TreeBlock(BlockID::light, -3, 16, 0), TreeBlock(BlockID::light, -2, 16, -1),
        TreeBlock(BlockID::light, -3, 16, -1), TreeBlock(BlockID::light, -2, 16, 1)
    },
    {
        TreeBlock(BlockID::moon_wood, 5, 0, 0), // max extent

        TreeBlock(BlockID::moon_wood, 2, 1, -1), TreeBlock(BlockID::moon_wood, 2, 2, -1), TreeBlock(BlockID::moon_wood, 3, 2, -1), TreeBlock(BlockID::moon_wood, 3, 3, -1),
        TreeBlock(BlockID::moon_wood, 4, 3, -1), TreeBlock(BlockID::moon_wood, 4, 4, -1), TreeBlock(BlockID::moon_wood, 4, 4, 0), TreeBlock(BlockID::moon_wood, 4, 5, 0),
        TreeBlock(BlockID::moon_wood, 4, 5, 1), TreeBlock(BlockID::moon_wood, 3, 5, 1), TreeBlock(BlockID::moon_wood, 3, 6, 1), TreeBlock(BlockID::moon_wood, 2, 6, 1),
        TreeBlock(BlockID::moon_wood, 2, 7, 1), TreeBlock(BlockID::moon_wood, 2, 7, 2), TreeBlock(BlockID::moon_wood, 2, 8, 2), TreeBlock(BlockID::moon_wood, 2, 8, 3),
        TreeBlock(BlockID::moon_wood, 2, 8, 4), TreeBlock(BlockID::moon_wood, 2, 9, 4), TreeBlock(BlockID::moon_wood, 1, 9, 4), TreeBlock(BlockID::moon_wood, 0, 10, 4),
        TreeBlock(BlockID::moon_wood, -1, 10, 4), TreeBlock(BlockID::moon_wood, 1, 7, 1), TreeBlock(BlockID::moon_wood, 1, 7, 0), TreeBlock(BlockID::moon_wood, 1, 8, 0),
        TreeBlock(BlockID::moon_wood, 1, 8, -1), TreeBlock(BlockID::moon_wood, 1, 9, -1), TreeBlock(BlockID::moon_wood, 2, 9, -1), TreeBlock(BlockID::moon_wood, 2, 10, -1),
        TreeBlock(BlockID::moon_wood, 3, 10, -1), TreeBlock(BlockID::moon_wood, 3, 11, -1), TreeBlock(BlockID::moon_wood, 3, 12, -1), TreeBlock(BlockID::moon_wood, 3, 12, 0),
        TreeBlock(BlockID::moon_wood, 3, 13, 0), TreeBlock(BlockID::moon_wood, 2, 13, 0), TreeBlock(BlockID::moon_wood, 2, 14, 0), TreeBlock(BlockID::moon_wood, 2, 14, 1),
        TreeBlock(BlockID::moon_wood, 3, 1, 1), TreeBlock(BlockID::moon_wood, 2, 1, 1), TreeBlock(BlockID::moon_wood, 2, 2, 1), TreeBlock(BlockID::moon_wood, 1, 2, 1),
        TreeBlock(BlockID::moon_wood, 1, 3, 1), TreeBlock(BlockID::moon_wood, 0, 3, 1), TreeBlock(BlockID::moon_wood, -1, 3, 1), TreeBlock(BlockID::moon_wood, 0, 4, 1),
        TreeBlock(BlockID::moon_wood, -1, 4, 1), TreeBlock(BlockID::moon_wood, 1, 2, 0), TreeBlock(BlockID::moon_wood, 1, 3, 0), TreeBlock(BlockID::moon_wood, 0, 3, 0),
        TreeBlock(BlockID::moon_wood, -1, 3, 0), TreeBlock(BlockID::moon_wood, -1, 4, 0), TreeBlock(BlockID::moon_wood, -2, 4, 0), TreeBlock(BlockID::moon_wood, -2, 4, 1),
        TreeBlock(BlockID::moon_wood, -2, 5, 1), TreeBlock(BlockID::moon_wood, -3, 5, 1), TreeBlock(BlockID::moon_wood, -3, 5, 0), TreeBlock(BlockID::moon_wood, -3, 6, 0),
        TreeBlock(BlockID::moon_wood, -4, 6, 1), TreeBlock(BlockID::moon_wood, -5, 6, 1), TreeBlock(BlockID::moon_wood, -5, 7, 1), TreeBlock(BlockID::moon_wood, -5, 7, 0),
        TreeBlock(BlockID::moon_wood, -5, 8, 0), TreeBlock(BlockID::moon_wood, -5, 8, -1), TreeBlock(BlockID::moon_wood, -5, 9, -1), TreeBlock(BlockID::moon_wood, -4, 6, 0),
        TreeBlock(BlockID::moon_wood, -4, 7, 0), TreeBlock(BlockID::moon_wood, -4, 8, -1), TreeBlock(BlockID::moon_wood, -4, 9, -1), TreeBlock(BlockID::moon_wood, -4, 9, -2),
        TreeBlock(BlockID::moon_wood, -4, 10, -2), TreeBlock(BlockID::moon_wood, -4, 10, -3), TreeBlock(BlockID::moon_wood, -3, 10, -3), TreeBlock(BlockID::moon_wood, -3, 11, -3),
        TreeBlock(BlockID::moon_wood, 3, 1, 0), TreeBlock(BlockID::moon_wood, 3, 2, 0), TreeBlock(BlockID::moon_wood, 3, 3, 0), TreeBlock(BlockID::moon_wood, 3, 4, 0),
        TreeBlock(BlockID::moon_wood, 3, 5, 0), TreeBlock(BlockID::moon_wood, 3, 6, 0), TreeBlock(BlockID::moon_wood, 3, 7, 0), TreeBlock(BlockID::moon_wood, 2, 1, 0),
        TreeBlock(BlockID::moon_wood, 2, 2, 0), TreeBlock(BlockID::moon_wood, 2, 3, 0), TreeBlock(BlockID::moon_wood, 2, 4, 0), TreeBlock(BlockID::moon_wood, 2, 5, 0),
        TreeBlock(BlockID::moon_wood, 2, 6, 0), TreeBlock(BlockID::moon_wood, 2, 7, 0), TreeBlock(BlockID::moon_wood, 2, 8, 0),

        TreeBlock(BlockID::light, -1, 9, 3), TreeBlock(BlockID::light, -1, 9, 4), TreeBlock(BlockID::light, -1, 9, 5), TreeBlock(BlockID::light, 0, 9, 4),
        TreeBlock(BlockID::light, 0, 9, 5), TreeBlock(BlockID::light, 0, 10, 3), TreeBlock(BlockID::light, 0, 10, 5), TreeBlock(BlockID::light, -1, 10, 3),
        TreeBlock(BlockID::light, -1, 10, 5), TreeBlock(BlockID::light, 0, 11, 4), TreeBlock(BlockID::light, -1, 11, 3), TreeBlock(BlockID::light, -1, 11, 4),
        TreeBlock(BlockID::light, -1, 11, 5), TreeBlock(BlockID::light, -2, 11, 3), TreeBlock(BlockID::light, -2, 11, 4), TreeBlock(BlockID::light, -2, 11, 5),
        TreeBlock(BlockID::light, -2, 10, 4), TreeBlock(BlockID::light, -2, 12, -3), TreeBlock(BlockID::light, -3, 12, -3), TreeBlock(BlockID::light, -1, 13, -3),
        TreeBlock(BlockID::light, -2, 13, -3), TreeBlock(BlockID::light, -3, 13, -3), TreeBlock(BlockID::light, -1, 12, -2), TreeBlock(BlockID::light, -2, 12, -2),
        TreeBlock(BlockID::light, -3, 12, -2), TreeBlock(BlockID::light, -1, 13, -2), TreeBlock(BlockID::light, -2, 13, -2), TreeBlock(BlockID::light, -3, 13, -2),
        TreeBlock(BlockID::light, -2, 14, -3), TreeBlock(BlockID::light, -2, 14, -2), TreeBlock(BlockID::light, -3, 14, -2), TreeBlock(BlockID::light, -1, 14, -1),
        TreeBlock(BlockID::light, -2, 14, -1), TreeBlock(BlockID::light, -1, 13, -1), TreeBlock(BlockID::light, -2, 13, -1), TreeBlock(BlockID::light, -3, 13, -1),
        TreeBlock(BlockID::light, -2, 12, -1), TreeBlock(BlockID::light, 2, 15, 0), TreeBlock(BlockID::light, 1, 15, 0), TreeBlock(BlockID::light, 3, 16, 0),
        TreeBlock(BlockID::light, 2, 16, 0), TreeBlock(BlockID::light, 1, 16, 0), TreeBlock(BlockID::light, 3, 16, 1), TreeBlock(BlockID::light, 2, 16, 1),
        TreeBlock(BlockID::light, 1, 16, 1), TreeBlock(BlockID::light, 3, 16, 2), TreeBlock(BlockID::light, 2, 16, 2), TreeBlock(BlockID::light, 1, 16, 2),
        TreeBlock(BlockID::light, 3, 17, 0), TreeBlock(BlockID::light, 2, 17, 0), TreeBlock(BlockID::light, 3, 17, 1), TreeBlock(BlockID::light, 2, 17, 1),
        TreeBlock(BlockID::light, 1, 17, 1), TreeBlock(BlockID::light, 2, 17, 2), TreeBlock(BlockID::light, 1, 17, 2), TreeBlock(BlockID::light, 1, 15, 1),
        TreeBlock(BlockID::light, 3, 15, 1), TreeBlock(BlockID::light, 3, 15, 2), TreeBlock(BlockID::light, 2, 15, 1), TreeBlock(BlockID::light, 2, 15, 2)
    },
    {
        TreeBlock(BlockID::moon_wood, 6, 0, 0), // max extent

        TreeBlock(BlockID::moon_wood, 0, 1, 0), TreeBlock(BlockID::moon_wood, -1, 1, 0), TreeBlock(BlockID::moon_wood, 0, 1, 1), TreeBlock(BlockID::moon_wood, 1, 1, 1),
        TreeBlock(BlockID::moon_wood, 1, 2, 2), TreeBlock(BlockID::moon_wood, 1, 3, 2), TreeBlock(BlockID::moon_wood, 2, 3, 2), TreeBlock(BlockID::moon_wood, 1, 4, 3),
        TreeBlock(BlockID::moon_wood, 1, 5, 3), TreeBlock(BlockID::moon_wood, 0, 5, 3), TreeBlock(BlockID::moon_wood, 0, 6, 4), TreeBlock(BlockID::moon_wood, -1, 6, 4),
        TreeBlock(BlockID::moon_wood, -1, 7, 4), TreeBlock(BlockID::moon_wood, -2, 7, 4), TreeBlock(BlockID::moon_wood, -2, 8, 4), TreeBlock(BlockID::moon_wood, -3, 8, 4),
        TreeBlock(BlockID::moon_wood, -3, 9, 4), TreeBlock(BlockID::moon_wood, -3, 9, 3), TreeBlock(BlockID::moon_wood, -4, 9, 3), TreeBlock(BlockID::moon_wood, -4, 10, 3),
        TreeBlock(BlockID::moon_wood, -5, 10, 3), TreeBlock(BlockID::moon_wood, -5, 10, 4), TreeBlock(BlockID::moon_wood, -1, 2, -1), TreeBlock(BlockID::moon_wood, -1, 3, -1),
        TreeBlock(BlockID::moon_wood, 0, 3, -1), TreeBlock(BlockID::moon_wood, 0, 4, -1), TreeBlock(BlockID::moon_wood, 0, 4, 0), TreeBlock(BlockID::moon_wood, 0, 5, 0),
        TreeBlock(BlockID::moon_wood, -1, 5, 0), TreeBlock(BlockID::moon_wood, -1, 6, 0), TreeBlock(BlockID::moon_wood, -2, 6, 0), TreeBlock(BlockID::moon_wood, -2, 7, 0),
        TreeBlock(BlockID::moon_wood, -3, 7, 0), TreeBlock(BlockID::moon_wood, -3, 8, 0), TreeBlock(BlockID::moon_wood, -3, 8, 1),

        TreeBlock(BlockID::light, -4, 8, 0), TreeBlock(BlockID::light, -4, 8, 1), TreeBlock(BlockID::light, -4, 9, 0), TreeBlock(BlockID::light, -4, 9, 1),
        TreeBlock(BlockID::light, -5, 9, 0), TreeBlock(BlockID::light, -5, 9, 1), TreeBlock(BlockID::light, -3, 11, 3), TreeBlock(BlockID::light, -4, 11, 3),
        TreeBlock(BlockID::light, -5, 11, 3), TreeBlock(BlockID::light, -6, 11, 3), TreeBlock(BlockID::light, -3, 12, 3), TreeBlock(BlockID::light, -4, 12, 3),
        TreeBlock(BlockID::light, -5, 12, 3), TreeBlock(BlockID::light, -6, 12, 3), TreeBlock(BlockID::light, -4, 13, 3), TreeBlock(BlockID::light, -5, 13, 3),
        TreeBlock(BlockID::light, -4, 11, 2), TreeBlock(BlockID::light, -5, 11, 2), TreeBlock(BlockID::light, -4, 12, 2), TreeBlock(BlockID::light, -5, 12, 2),
        TreeBlock(BlockID::light, -4, 11, 4), TreeBlock(BlockID::light, -5, 11, 4), TreeBlock(BlockID::light, -5, 12, 4)
    },
    {
        TreeBlock(BlockID::moon_wood, 4, 0, 0), // max extent

        TreeBlock(BlockID::moon_wood, -1, 1, 1), TreeBlock(BlockID::moon_wood, 0, 1, 1), TreeBlock(BlockID::moon_wood, -1, 1, 0), TreeBlock(BlockID::moon_wood, 0, 1, 0),
        TreeBlock(BlockID::moon_wood, 0, 2, 0), TreeBlock(BlockID::moon_wood, 1, 1, 0), TreeBlock(BlockID::moon_wood, 1, 2, 0), TreeBlock(BlockID::moon_wood, 1, 3, 0),
        TreeBlock(BlockID::moon_wood, 1, 2, -1), TreeBlock(BlockID::moon_wood, 1, 3, -1), TreeBlock(BlockID::moon_wood, 0, 1, -1), TreeBlock(BlockID::moon_wood, 0, 2, -1),
        TreeBlock(BlockID::moon_wood, 0, 3, -1), TreeBlock(BlockID::moon_wood, 0, 3, -2), TreeBlock(BlockID::moon_wood, 0, 4, -2), TreeBlock(BlockID::moon_wood, -1, 4, -2),
        TreeBlock(BlockID::moon_wood, -1, 3, -1), TreeBlock(BlockID::moon_wood, -1, 4, -1), TreeBlock(BlockID::moon_wood, -1, 5, -2), TreeBlock(BlockID::moon_wood, -1, 5, -3),
        TreeBlock(BlockID::moon_wood, -1, 6, -3), TreeBlock(BlockID::moon_wood, -1, 7, -3), TreeBlock(BlockID::moon_wood, -2, 6, -3), TreeBlock(BlockID::moon_wood, -2, 7, -3),
        TreeBlock(BlockID::moon_wood, -2, 8, -3), TreeBlock(BlockID::moon_wood, -2, 9, -3), TreeBlock(BlockID::moon_wood, -1, 7, -4), TreeBlock(BlockID::moon_wood, -1, 8, -4),
        TreeBlock(BlockID::moon_wood, -1, 9, -4),  TreeBlock(BlockID::moon_wood, -1, 10, -4), TreeBlock(BlockID::moon_wood, -1, 10, -3), TreeBlock(BlockID::moon_wood, -1, 11, -3),
        TreeBlock(BlockID::moon_wood, -1, 11, -2), TreeBlock(BlockID::moon_wood, -1, 12, -2), TreeBlock(BlockID::moon_wood, 0, 11, -3), TreeBlock(BlockID::moon_wood, 0, 12, -3),
        TreeBlock(BlockID::moon_wood, 1, 13, -2), TreeBlock(BlockID::moon_wood, 0, 13, -2), TreeBlock(BlockID::moon_wood, 0, 13, -1), TreeBlock(BlockID::moon_wood, 1, 12, -2),
        TreeBlock(BlockID::moon_wood, 0, 12, -2), TreeBlock(BlockID::moon_wood, 0, 12, -1), TreeBlock(BlockID::moon_wood, 1, 14, -1), TreeBlock(BlockID::moon_wood, 1, 14, 0),
        TreeBlock(BlockID::moon_wood, 1, 15, 0),

        TreeBlock(BlockID::light, 1, 13, 1), TreeBlock(BlockID::light, 2, 13, 1), TreeBlock(BlockID::light, 1, 14, 1), TreeBlock(BlockID::light, 2, 14, 1),
        TreeBlock(BlockID::light, 1, 13, 0), TreeBlock(BlockID::light, 1, 13, -1), TreeBlock(BlockID::light, 2, 12, 0), TreeBlock(BlockID::light, 2, 13, 0),
        TreeBlock(BlockID::light, 2, 14, 0), TreeBlock(BlockID::light, 2, 15, 0), TreeBlock(BlockID::light, 1, 15, -1), TreeBlock(BlockID::light, 2, 13, -1),
        TreeBlock(BlockID::light, 2, 14, -1), TreeBlock(BlockID::light, 3, 13, 0), TreeBlock(BlockID::light, 3, 14, 0)
    },
    {
        TreeBlock(BlockID::moon_wood, 4, 0, 0), // max extent

        TreeBlock(BlockID::moon_wood, -1, 1, 1), TreeBlock(BlockID::moon_wood, 0, 1, 1), TreeBlock(BlockID::moon_wood, 0, 1, 0), TreeBlock(BlockID::moon_wood, 1, 1, 0),
        TreeBlock(BlockID::moon_wood, -1, 1, -1), TreeBlock(BlockID::moon_wood, 0, 2, 2), TreeBlock(BlockID::moon_wood, -1, 2, 2), TreeBlock(BlockID::moon_wood, -1, 3, 2),
        TreeBlock(BlockID::moon_wood, -2, 3, 2), TreeBlock(BlockID::moon_wood, -2, 4, 2), TreeBlock(BlockID::moon_wood, -2, 4, 1), TreeBlock(BlockID::moon_wood, -2, 5, 1),
        TreeBlock(BlockID::moon_wood, -3, 5, 1), TreeBlock(BlockID::moon_wood, -3, 6, 1), TreeBlock(BlockID::moon_wood, 0, 2, -1), TreeBlock(BlockID::moon_wood, 0, 3, -2),
        TreeBlock(BlockID::moon_wood, 0, 4, -2), TreeBlock(BlockID::moon_wood, -1, 4, -2), TreeBlock(BlockID::moon_wood, -1, 5, -2), TreeBlock(BlockID::moon_wood, -2, 5, -2),
        TreeBlock(BlockID::moon_wood, -2, 6, -1), TreeBlock(BlockID::moon_wood, -2, 7, 0), TreeBlock(BlockID::moon_wood, -2, 8, 0), TreeBlock(BlockID::moon_wood, -3, 8, 0),
        TreeBlock(BlockID::moon_wood, -2, 8, 1), TreeBlock(BlockID::moon_wood, -2, 9, 1), TreeBlock(BlockID::moon_wood, -1, 9, 1), TreeBlock(BlockID::moon_wood, -1, 10, 0),
        TreeBlock(BlockID::moon_wood, -2, 10, 0), TreeBlock(BlockID::moon_wood, -1, 11, 0), TreeBlock(BlockID::moon_wood, 0, 11, 0), TreeBlock(BlockID::moon_wood, -2, 11, -1),
        TreeBlock(BlockID::moon_wood, -2, 11, -2), TreeBlock(BlockID::moon_wood, 0, 12, 1), TreeBlock(BlockID::moon_wood, -1, 12, 1), TreeBlock(BlockID::moon_wood, -1, 12, 2),
        TreeBlock(BlockID::moon_wood, -1, 13, 2), TreeBlock(BlockID::moon_wood, -2, 13, 2), TreeBlock(BlockID::moon_wood, -2, 13, 1), TreeBlock(BlockID::moon_wood, -2, 14, 1),
        TreeBlock(BlockID::moon_wood, -3, 14, 1), TreeBlock(BlockID::moon_wood, -2, 14, 0),

        TreeBlock(BlockID::light, -3, 6, 0), TreeBlock(BlockID::light, -3, 7, 0), TreeBlock(BlockID::light, -4, 6, 0), TreeBlock(BlockID::light, -4, 7, 0),
        TreeBlock(BlockID::light, -3, 6, -1), TreeBlock(BlockID::light, -3, 7, -1), TreeBlock(BlockID::light, -4, 6, -1), TreeBlock(BlockID::light, -1, 11, -2),
        TreeBlock(BlockID::light, -1, 12, -2), TreeBlock(BlockID::light, -2, 11, -3), TreeBlock(BlockID::light, -2, 12, -3), TreeBlock(BlockID::light, -2, 12, -2),
        TreeBlock(BlockID::light, -1, 12, -3), TreeBlock(BlockID::light, -1, 15, 0), TreeBlock(BlockID::light, -1, 15, -1), TreeBlock(BlockID::light, -2, 15, 0),
        TreeBlock(BlockID::light, -2, 15, -1), TreeBlock(BlockID::light, -1, 16, 0), TreeBlock(BlockID::light, -1, 16, -1), TreeBlock(BlockID::light, -2, 16, 0),
        TreeBlock(BlockID::light, -2, 16, -1)
    },
    {
        TreeBlock(BlockID::moon_wood, 6, 0, 0), // max extent

        TreeBlock(BlockID::moon_wood, 0, 1, 0), TreeBlock(BlockID::moon_wood, 1, 1, 0), TreeBlock(BlockID::moon_wood, -1, 1, 0), TreeBlock(BlockID::moon_wood, 0, 1, 1),
        TreeBlock(BlockID::moon_wood, 1, 1, 1), TreeBlock(BlockID::moon_wood, 1, 1, 2), TreeBlock(BlockID::moon_wood, 2, 1, 1), TreeBlock(BlockID::moon_wood, 2, 1, 2),
        TreeBlock(BlockID::moon_wood, 2, 2, 1), TreeBlock(BlockID::moon_wood, 2, 2, 2), TreeBlock(BlockID::moon_wood, 3, 2, 2), TreeBlock(BlockID::moon_wood, 3, 3, 2),
        TreeBlock(BlockID::moon_wood, 4, 3, 2), TreeBlock(BlockID::moon_wood, 4, 4, 2), TreeBlock(BlockID::moon_wood, 5, 5, 2), TreeBlock(BlockID::moon_wood, 6, 6, 2),
        TreeBlock(BlockID::moon_wood, 6, 6, 3), TreeBlock(BlockID::moon_wood, 6, 6, 4), TreeBlock(BlockID::moon_wood, 6, 7, 4), TreeBlock(BlockID::moon_wood, 5, 7, 4),
        TreeBlock(BlockID::moon_wood, 0, 2, -1), TreeBlock(BlockID::moon_wood, -1, 2, -1), TreeBlock(BlockID::moon_wood, -1, 3, -2), TreeBlock(BlockID::moon_wood, -1, 4, -2),
        TreeBlock(BlockID::moon_wood, -2, 3, -2), TreeBlock(BlockID::moon_wood, 0, 4, -2), TreeBlock(BlockID::moon_wood, 0, 5, -2), TreeBlock(BlockID::moon_wood, 1, 5, -2),
        TreeBlock(BlockID::moon_wood, 1, 6, -3), TreeBlock(BlockID::moon_wood, 2, 6, -3), TreeBlock(BlockID::moon_wood, 1, 7, -2), TreeBlock(BlockID::moon_wood, 2, 7, -2),
        TreeBlock(BlockID::moon_wood, 1, 8, -2), TreeBlock(BlockID::moon_wood, 0, 8, -2), TreeBlock(BlockID::moon_wood, 0, 9, -2), TreeBlock(BlockID::moon_wood, -1, 9, -2),
        TreeBlock(BlockID::moon_wood, -1, 10, -2), TreeBlock(BlockID::moon_wood, -1, 10, -3), TreeBlock(BlockID::moon_wood, -1, 11, -3), TreeBlock(BlockID::moon_wood, -1, 11, -4),
        TreeBlock(BlockID::moon_wood, 0, 11, -3), TreeBlock(BlockID::moon_wood, 0, 11, -4), TreeBlock(BlockID::moon_wood, 0, 12, -3), TreeBlock(BlockID::moon_wood, 0, 12, -2),
        TreeBlock(BlockID::moon_wood, 0, 13, -2), TreeBlock(BlockID::moon_wood, 1, 13, -2), TreeBlock(BlockID::moon_wood, 1, 14, -2), TreeBlock(BlockID::moon_wood, 1, 15, -2),
        TreeBlock(BlockID::moon_wood, 1, 16, -2), TreeBlock(BlockID::moon_wood, 1, 16, -3), TreeBlock(BlockID::moon_wood, 2, 14, -2), TreeBlock(BlockID::moon_wood, 2, 15, -2),
        TreeBlock(BlockID::moon_wood, 2, 14, -1), TreeBlock(BlockID::moon_wood, 1, 14, -1),

        TreeBlock(BlockID::light, 4, 8, 5), TreeBlock(BlockID::light, 5, 8, 5), TreeBlock(BlockID::light, 4, 8, 4), TreeBlock(BlockID::light, 5, 8, 4),
        TreeBlock(BlockID::light, 4, 9, 5), TreeBlock(BlockID::light, 5, 9, 5), TreeBlock(BlockID::light, 4, 9, 4), TreeBlock(BlockID::light, 5, 9, 4),
        TreeBlock(BlockID::light, 0, 16, -3), TreeBlock(BlockID::light, 0, 16, -2), TreeBlock(BlockID::light, 0, 17, -3), TreeBlock(BlockID::light, 0, 17, -2),
        TreeBlock(BlockID::light, -1, 17, -3), TreeBlock(BlockID::light, -1, 17, -2)
    }
};

uint32_t GetStructureSeed(uint64_t world_seed, int chunk_x, int chunk_z)
{
    uint64_t x = (uint64_t)((uint32_t)chunk_x);
    uint64_t z = (uint64_t)((uint32_t)chunk_z);

    uint64_t hash = world_seed;
    hash ^= x * 0x517CC1B727220A95UL;
    hash ^= z * 0x9E3779B97F4A7C15UL;
    hash ^= (hash >> 33);
    hash *= 0xFF51AFD7ED558CCDUL;
    hash ^= (hash >> 33);
    hash *= 0xC4CEB9FE1A85EC53UL;
    hash ^= (hash >> 33);

    return (int)(hash & 0x7FFFFFFF);
}

void GenerateHeightMap(uint8_t *height_map, int chunk_x, int chunk_z, uint64_t seed, float amplitude, float frequency, float persistence, int octaves, float roughness)
{
    float frequency0 = frequency;
    float amplitude0 = amplitude;
    float divFactor = 32.0f - 4.0f * roughness;
    float height_limit;
    double offset_x = (double)(SplitMix64(seed) & 0xFFFFFFFF);
    double offset_z = (double)(SplitMix64(seed) & 0xFFFFFFFF);

    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            frequency = frequency0;
            amplitude = amplitude0;
            height_limit = 0.0f;
            for (int i = 0; i < octaves; i++)
            {
                double xArg = (((x + chunk_x * CHUNK_SIZE) + offset_x) / divFactor) * frequency;
                double zArg = (((z + chunk_z * CHUNK_SIZE) + offset_z) / divFactor) * frequency;
                height_limit += SimplexNoise(xArg, zArg) * amplitude;
                frequency *= 3.0f;
                amplitude *= persistence;
            }
            height_map[z + CHUNK_SIZE * x] = (uint8_t)height_limit;
        }
    }
}

void GenerateChunk(BlockID *chunk, int chunk_x, int chunk_z, MoonSettings settings)
{
    // Seed RNG so structure placement is deterministic
    uint64_t structure_seed = settings.seed ^ ((uint64_t)chunk_x * 73856093ull) ^ ((uint64_t)chunk_z * 19349663ull);
    RNG rng{structure_seed};

    thread_local uint8_t height_maps[CHUNK_SIZE * CHUNK_SIZE * 4];

    //
    // Generate base terrain
    //

    const int ROCK_OFFSET   = 0;
    const int GRAVEL_OFFSET = 1 * CHUNK_SIZE * CHUNK_SIZE;
    const int DIRT_OFFSET   = 2 * CHUNK_SIZE * CHUNK_SIZE;
    const int SAND_OFFSET   = 3 * CHUNK_SIZE * CHUNK_SIZE;
    GenerateHeightMap(&height_maps[ROCK_OFFSET], chunk_x, chunk_z, settings.seed, 14, 0.2, 0.4, 4, settings.terrain_roughness);
    GenerateHeightMap(&height_maps[GRAVEL_OFFSET], chunk_x, chunk_z, settings.seed, 4, 0.2, 0.6, 2, settings.terrain_roughness);
    GenerateHeightMap(&height_maps[DIRT_OFFSET], chunk_x, chunk_z, settings.seed, 3, 0.2, 0.6, 3, settings.terrain_roughness);
    GenerateHeightMap(&height_maps[SAND_OFFSET], chunk_x, chunk_z, settings.seed, 3, 0.2, 0.8, 2, settings.terrain_roughness * 0.6f);

    //
    // Crater
    //

    bool spawn_crater = RNG{}.Range(0.0f, 1.0f) < 0.22f; // 22% chance (to match original average crater density)
    if (spawn_crater)
    {
        int radius = RNG{}.Range(7, 14); // was [8, 15]

        float outer_radius = (float)radius;
        float inner_radius = outer_radius * 0.75f;

        int center_x = RNG{}.Range(radius, CHUNK_SIZE - 1 - radius);
        int center_z = RNG{}.Range(radius, CHUNK_SIZE - 1 - radius);

        for (int x = center_x - radius; x <= center_x + radius; x++)
        {
            for (int z = center_z - radius; z <= center_z + radius; z++)
            {
                int dx = x - center_x;
                int dz = z - center_z;

                float dist = glm::sqrt((float)(dx*dx + dz*dz));

                if (dist > outer_radius) continue;

                float t;
                if (dist < inner_radius)
                {
                    float u = dist / inner_radius;
                    t = u * u;  // inner bowl
                }
                else
                {
                    t = 1.0f - (dist - inner_radius) / (outer_radius - inner_radius); // rim falloff
                }

                float h = (&height_maps[DIRT_OFFSET])[z + x * CHUNK_SIZE];
                h += (outer_radius * t) / 3.0f;

                (&height_maps[DIRT_OFFSET])[z + x * CHUNK_SIZE] = (uint8_t)glm::min(h, 126.0f);
            }
        }
    }

    //
    // Layer determination
    //

    // Initial generation
    int chunk_index = 0;
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            int rock_height   = height_maps[ROCK_OFFSET   + z + CHUNK_SIZE * x];
            int gravel_height = height_maps[GRAVEL_OFFSET + z + CHUNK_SIZE * x];
            int dirt_height   = height_maps[DIRT_OFFSET   + z + CHUNK_SIZE * x];
            int sand_height   = height_maps[SAND_OFFSET   + z + CHUNK_SIZE * x];
            int y = 0;

            // Base rock
            while (y < 50 + rock_height)
            {
                chunk[chunk_index++] = BlockID::rock;
                y++;
            }

            // Base gravel
            while (y < 50 + rock_height + gravel_height)
            {
                chunk[chunk_index++] = BlockID::gravel;
                y++;
            }

            // Base dirt
            while (y < 50 + rock_height + gravel_height + dirt_height)
            {
                chunk[chunk_index++] = BlockID::dirt;
                y++;
            }

            if (50 + rock_height + gravel_height + dirt_height < GROUND_LEVEL) // Below ground level; fill rest in with sand/water
            {
                int sand_limit = glm::min(GROUND_LEVEL, 50 + rock_height + gravel_height + dirt_height + sand_height);
                while (y < sand_limit)
                {
                    chunk[chunk_index++] = BlockID::sand;
                    y++;
                }

                while (y < GROUND_LEVEL)
                {
                    chunk[chunk_index++] = BlockID::water;
                    y++;
                }
            }
            else // Above ground level; finish terrain by placing topsoil
            {
                chunk[chunk_index++] = BlockID::topsoil;
                y++;
            }

            while (y < WORLD_HEIGHT_LIMIT)
            {
                chunk[chunk_index++] = BlockID::air;
                y++;
            }
        }
    }

    //
    // Ores
    //

    int ore_spawn_chance = rng.Range(1, 10);
    if (ore_spawn_chance <= 4)
    {
        int seed_block_x = rng.Range(5, CHUNK_SIZE - 7);
        int seed_block_z = rng.Range(5, CHUNK_SIZE - 7);
        int seed_block_y = -1;
        for (int y = 63; y < WORLD_HEIGHT_LIMIT; y++)
        {
            chunk_index = GetChunkIndex(seed_block_x, y + 1, seed_block_z);
            if (chunk[chunk_index] == BlockID::air)
            {
                seed_block_y = y;
                break;
            }
        }

        int ore = rng.Range(1, 100);
        int vein_size;
        BlockID ore_id;
        if (ore <= 36) // 36%
        {
            ore_id = BlockID::magnetite;
            vein_size = rng.Range(2, 6);
        }
        else if (ore <= 60) // 24%
        {
            ore_id = BlockID::aluminum_ore;
            vein_size = rng.Range(2, 6);
        }
        else if (ore <= 78) // 18%
        {
            ore_id = BlockID::titanium_ore;
            vein_size = rng.Range(2, 6);
        }
        else if (ore <= 91) // 13%
        {
            ore_id = BlockID::gold_ore;
            vein_size = rng.Range(1, 4);
        }
        else if (ore <= 98) // 7%
        {
            ore_id = BlockID::notchium_ore;
            vein_size = rng.Range(1, 4);
        }
        else // 2%
        {
            ore_id = BlockID::blue_crystal;
            vein_size = rng.Range(1, 4);
        }

        int current_block_x = seed_block_x;
        int current_block_y = seed_block_y;
        int current_block_z = seed_block_z;
        chunk_index = GetChunkIndex(seed_block_x, seed_block_y, seed_block_z);
        chunk[chunk_index] = ore_id;
        for (int count = 0; count < vein_size; count++)
        {
            int next_direction = rng.Range(1, 5);
            if (next_direction == 1) // Forward
            {
                current_block_z++;
            }
            else if (next_direction == 2) // Backward
            {
                current_block_z--;
            }
            else if (next_direction == 3) // Right
            {
                current_block_x++;
            }
            else if (next_direction == 4) // Left
            {
                current_block_x--;
            }
            else // Down
            {
                current_block_y--;
            }

            chunk_index = GetChunkIndex(current_block_x, current_block_y, current_block_z);
            if (chunk[chunk_index] != BlockID::air)
                chunk[chunk_index] = ore_id;
        }
    }

    //
    // Trees
    //

    // These calculations were reverse-engineered from the latest version of the original game (v2.01).
    // Charlie's chunks were much bigger than mine, so I did my best to make the densities match.
    int green_tree_count = settings.tree_cover * (rng.Range(0.0f, 1.0f) * rng.Range(0.0f, 1.0f) - 0.1f) * 9;
    int wood_tree_count = settings.tree_cover * (rng.Range(0.0f, 1.0f) * rng.Range(0.0f, 1.0f) - 0.2f) * 7;
    int color_tree_count = settings.tree_cover * (rng.Range(0.0f, 1.0f) * rng.Range(0.0f, 1.0f) - 0.4f) * 4;
    for (int i = 0; i < 3; i++)
    {
        int tree_count;
        if (i == 0)
            tree_count = green_tree_count;
        else if (i == 1)
            tree_count = wood_tree_count;
        else
            tree_count = color_tree_count;

        for (int j = 0; j < tree_count; j++)
        {
            std::vector<TreeBlock> tree_data;
            if (i == 0)
                tree_data = GREEN_LIGHT_TREE_SHAPES[rng.Range(0, GREEN_LIGHT_TREE_SHAPES.size() - 1)];
            else if (i == 1)
                tree_data = SPIRAL_LIGHT_TREE_SHAPES[rng.Range(0, SPIRAL_LIGHT_TREE_SHAPES.size() - 1)];
            else
                tree_data = COLOR_WOOD_TREE_SHAPES[rng.Range(0, COLOR_WOOD_TREE_SHAPES.size() - 1)];

            int tree_orientation = rng.Range(1, 4);
            int padding_needed = tree_data[0].local_x;
            int base_block_x = rng.Range(padding_needed, (CHUNK_SIZE - 1) - padding_needed);
            int base_block_z = rng.Range(padding_needed, (CHUNK_SIZE - 1) - padding_needed);
            int base_block_y;
            for (int y = 63; y < WORLD_HEIGHT_LIMIT; y++)
            {
                chunk_index = GetChunkIndex(base_block_x, y, base_block_z);
                if (chunk[chunk_index + 1] == BlockID::air)
                {
                    if (chunk[chunk_index] == BlockID::topsoil)
                    {
                        base_block_y = y;
                        for (int j = 1; j < tree_data.size(); j++)
                        {
                            TreeBlock tree_block = tree_data[j];
                            if (tree_orientation == 2) // 90 degrees
                            {
                                int temp = tree_block.local_x;
                                tree_block.local_x = -tree_block.local_z;
                                tree_block.local_z = temp;
                            }
                            else if (tree_orientation == 3) // 180 degrees
                            {
                                tree_block.local_x *= -1;
                                tree_block.local_z *= -1;
                            }
                            else if (tree_orientation == 4) // 270 degrees
                            {
                                int temp = tree_block.local_x;
                                tree_block.local_x = tree_block.local_z;
                                tree_block.local_z = -temp;
                            }

                            chunk_index = GetChunkIndex(base_block_x + tree_block.local_x, base_block_y + tree_block.local_y, base_block_z + tree_block.local_z);
                            chunk[chunk_index] = tree_block.block;

                            // Partial fix for overhanging trees when placed on an edge
                            if (tree_block.local_y == 1 && chunk[chunk_index - 1] == BlockID::air && chunk[chunk_index - 2] != BlockID::air)
                                chunk[chunk_index - 1] = tree_block.block;
                        }
                    }
                    break;
                }
            }
        }
    }

    //
    // Crystal plants
    //

    constexpr int CRYSTAL_WEIGHTS[] = {65, 30, 5};
    constexpr int TOTAL_CRYSTAL_WEIGHT = 100;
    constexpr BlockID CRYSTALS[] = {BlockID::sulphur_crystal, BlockID::boron_crystal, BlockID::blue_crystal};

    bool spawn_crystal = rng.Range(0.0f, 1.0f) < 0.05f;
    if (spawn_crystal)
    {
        int rand_weight = rng.Range(0, TOTAL_CRYSTAL_WEIGHT - 1);
        int remaining_weight = TOTAL_CRYSTAL_WEIGHT;

        int index = 0;
        while (remaining_weight > rand_weight)
        {
            remaining_weight -= CRYSTAL_WEIGHTS[index];
            if (remaining_weight > rand_weight)
                index++;
        }

        int x = rng.Range(4, (CHUNK_SIZE - 1) - 4);
        int z = rng.Range(4, (CHUNK_SIZE - 1) - 4);

        // The total height isn't determined by a single heightmap, so I have to do this...
        int y = -1;
        for (int j = 50; j < WORLD_HEIGHT_LIMIT - 1; j++) // j=50 to skip section guaranteed to be solid
        {
            if (chunk[GetChunkIndex(x, j + 1, z)] == BlockID::air)
            {
                y = j;
                break;
            }
        }

        if (y != -1)
        {
            float w = rng.Range(2.0, 4.0);
            GenerateCrystal(
                chunk,
                rng,
                w,
                x,
                z,
                y,
                rng.Range(2, 4),
                CRYSTALS[index],
                (int)w,
                4
            );
        }
    }

    //
    // Astronaut lairs
    //

    bool spawn_astronaut_lair = rng.Range(1, 100) == 69; // 1% chance, each chunk
    const int lair_depth = 31;
    if (spawn_astronaut_lair)
    {
        int center_block_x = (int)(CHUNK_SIZE / 2);
        int center_block_z = (int)(CHUNK_SIZE / 2);
        int center_block_y = -1;
        for (int y = 64; y < WORLD_HEIGHT_LIMIT; y++)
        {
            chunk_index = GetChunkIndex(center_block_x, y + 1, center_block_z);
            if (chunk[chunk_index] == BlockID::air)
            {
                center_block_y = y;
                break;
            }
        }

        // Place gravel block at center of bottom so I can easily check whether a chunk contains an astronaut lair
        chunk_index = GetChunkIndex(center_block_x, 0, center_block_z);
        chunk[chunk_index] = BlockID::gravel;

        // Carve main shaft
        for (int x_offset = -3; x_offset <= 3; x_offset++)
        {
            for (int z_offset = -3; z_offset <= 3; z_offset++)
            {
                for (int y_offset = -6; y_offset <= lair_depth; y_offset++)
                {
                    chunk_index = GetChunkIndex(center_block_x + x_offset, center_block_y - y_offset, center_block_z + z_offset);
                    chunk[chunk_index] = BlockID::air;
                }
            }
        }

        // Decorate main shaft with polymer and light
        bool left_side_done = false;
        bool right_side_done = false;
        bool front_side_done = false;
        bool back_side_done = false;
        for (int y = center_block_y - lair_depth; y < WORLD_HEIGHT_LIMIT; y++)
        {
            chunk_index = GetChunkIndex(center_block_x - 4, y, center_block_z);
            if (chunk[chunk_index] != BlockID::air && !left_side_done)
            {
                chunk[chunk_index] = BlockID::polymer;
                if ((y - (center_block_y - lair_depth)) / 8.0f == 3)
                    chunk[chunk_index] = BlockID::light;
            }
            else
            {
                left_side_done = true;
            }

            chunk_index = GetChunkIndex(center_block_x + 4, y, center_block_z);
            if (chunk[chunk_index] != BlockID::air && !right_side_done)
            {
                chunk[chunk_index] = BlockID::polymer;
                if ((y - (center_block_y - lair_depth)) / 8.0f == 3)
                    chunk[chunk_index] = BlockID::light;
            }
            else
            {
                right_side_done = true;
            }

            chunk_index = GetChunkIndex(center_block_x, y, center_block_z + 4);
            if (chunk[chunk_index] != BlockID::air && !front_side_done)
            {
                chunk[chunk_index] = BlockID::polymer;
                if ((y - (center_block_y - lair_depth)) / 8.0f == 3)
                    chunk[chunk_index] = BlockID::light;
            }
            else
            {
                front_side_done = true;
            }

            chunk_index = GetChunkIndex(center_block_x, y, center_block_z - 4);
            if (chunk[chunk_index] != BlockID::air && !back_side_done)
            {
                chunk[chunk_index] = BlockID::polymer;
                if ((y - (center_block_y - lair_depth)) / 8.0f == 3)
                    chunk[chunk_index] = BlockID::light;
            }
            else
            {
                back_side_done = true;
            }

            if (left_side_done && right_side_done && front_side_done && back_side_done)
                break;
        }

        // Polymer at bottom center
        chunk_index = GetChunkIndex(center_block_x, center_block_y - lair_depth - 1, center_block_z);
        chunk[chunk_index] = BlockID::polymer;

        // Extra front shaft(s)
        if (rng.Range(1, 4) == 1)
        {
            for (int dz = 0; dz < 10; dz++)
            {
                for (int dx = -2; dx <= 2; dx++)
                {
                    for (int dy = -2; dy <= 2; dy++)
                    {
                        int shaft_x = center_block_x + dx;
                        int shaft_y = (center_block_y - lair_depth + 3) + dy;
                        int shaft_z = (center_block_z + 4) + dz;
                        chunk_index = GetChunkIndex(shaft_x, shaft_y, shaft_z);
                        chunk[chunk_index] = BlockID::air;

                        if (dy == 0)
                        {
                            // Left polymer
                            chunk_index = GetChunkIndex(center_block_x - 3, shaft_y, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;

                            // Right polymer
                            chunk_index = GetChunkIndex(center_block_x + 3, shaft_y, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;
                        }

                        if (dx == 0)
                        {
                            // Top polymer
                            chunk_index = GetChunkIndex(shaft_x, center_block_y - lair_depth + 6, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;

                            // Bottom polymer
                            chunk_index = GetChunkIndex(shaft_x, center_block_y - lair_depth, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;
                        }
                    }
                }
            }

            // Polymer on back wall
            chunk_index = GetChunkIndex(center_block_x, center_block_y - lair_depth + 3, center_block_z + 4 + 10);
            chunk[chunk_index] = BlockID::polymer;

            // Extra vertical shaft?
            if (rng.Range(1, 4) == 1)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        for (int dy = 0; dy <= 20; dy++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x + dx, center_block_y - lair_depth - dy, center_block_z + 4 + 8 + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dz == 0 && dx != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x + dx + dx, center_block_y - lair_depth - dy, center_block_z + 4 + 8);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dx == 0 && dz != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x, center_block_y - lair_depth - dy, center_block_z + 4 + 8 + dz + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (left)?
            if (rng.Range(1, 4) == 1)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dx = 0; dx < 7; dx++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x - 3 - dx, center_block_y - lair_depth + 3 + dy, center_block_z + 12 + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dy == 0 && dz == -1) // left
                            {
                                chunk_index = GetChunkIndex(center_block_x - 3 - dx, center_block_y - lair_depth + 3 + dy, center_block_z + 12 + dz - 1);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dz == 0 && dy != 0) // top and bottom
                            {
                                chunk_index = GetChunkIndex(center_block_x - 3 - dx, center_block_y - lair_depth + 3 + dy + dy, center_block_z + 12 + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (right)?
            if (rng.Range(1, 4) == 1)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dx = 0; dx < 7; dx++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x + 3 + dx, center_block_y - lair_depth + 3 + dy, center_block_z + 12 + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dy == 0 && dz == -1) // right
                            {
                                chunk_index = GetChunkIndex(center_block_x + 3 + dx, center_block_y - lair_depth + 3 + dy, center_block_z + 12 + dz - 1);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dz == 0 && dy != 0) // top and bottom
                            {
                                chunk_index = GetChunkIndex(center_block_x + 3 + dx, center_block_y - lair_depth + 3 + dy + dy, center_block_z + 12 + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }
        }

        // Extra back shaft(s)
        if (rng.Range(1, 4) == 1)
        {
            for (int dz = 0; dz < 10; dz++)
            {
                for (int dx = -2; dx <= 2; dx++)
                {
                    for (int dy = -2; dy <= 2; dy++)
                    {
                        int shaft_x = center_block_x + dx;
                        int shaft_y = (center_block_y - lair_depth + 3) + dy;
                        int shaft_z = (center_block_z - 4) - dz;
                        chunk_index = GetChunkIndex(shaft_x, shaft_y, shaft_z);
                        chunk[chunk_index] = BlockID::air;

                        if (dy == 0)
                        {
                            // Left polymer
                            chunk_index = GetChunkIndex(center_block_x - 3, shaft_y, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;

                            // Right polymer
                            chunk_index = GetChunkIndex(center_block_x + 3, shaft_y, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;
                        }

                        if (dx == 0)
                        {
                            // Top polymer
                            chunk_index = GetChunkIndex(shaft_x, center_block_y - lair_depth + 6, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;

                            // Bottom polymer
                            chunk_index = GetChunkIndex(shaft_x, center_block_y - lair_depth, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;
                        }
                    }
                }
            }

            // Polymer on back wall
            chunk_index = GetChunkIndex(center_block_x, center_block_y - lair_depth + 3, center_block_z - 4 - 10);
            chunk[chunk_index] = BlockID::polymer;

            // Extra vertical shaft?
            if (rng.Range(1, 4) == 1)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        for (int dy = 0; dy <= 20; dy++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x + dx, center_block_y - lair_depth - dy, center_block_z - 12 + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dz == 0 && dx != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x + dx + dx, center_block_y - lair_depth - dy, center_block_z - 12);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dx == 0 && dz != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x, center_block_y - lair_depth - dy, center_block_z - 12 + dz + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (left)?
            if (rng.Range(1, 4) == 1)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dx = 0; dx < 7; dx++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x - 3 - dx, center_block_y - lair_depth + 3 + dy, center_block_z - 12 + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dy == 0 && dz == 1) // left
                            {
                                chunk_index = GetChunkIndex(center_block_x - 3 - dx, center_block_y - lair_depth + 3 + dy, center_block_z - 12 + dz + 1);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dz == 0 && dy != 0) // top and bottom
                            {
                                chunk_index = GetChunkIndex(center_block_x - 3 - dx, center_block_y - lair_depth + 3 + dy + dy, center_block_z - 12 + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (right)?
            if (rng.Range(1, 4) == 1)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dx = 0; dx < 7; dx++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x + 3 + dx, center_block_y - lair_depth + 3 + dy, center_block_z - 12 + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dy == 0 && dz == 1) // right
                            {
                                chunk_index = GetChunkIndex(center_block_x + 3 + dx, center_block_y - lair_depth + 3 + dy, center_block_z - 12 + dz + 1);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dz == 0 && dy != 0) // top and bottom
                            {
                                chunk_index = GetChunkIndex(center_block_x + 3 + dx, center_block_y - lair_depth + 3 + dy + dy, center_block_z - 12 + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }
        }

        // Extra right shaft(s)
        if (rng.Range(1, 4) == 1)
        {
            for (int dx = 0; dx < 10; dx++)
            {
                for (int dz = -2; dz <= 2; dz++)
                {
                    for (int dy = -2; dy <= 2; dy++)
                    {
                        int shaft_x = (center_block_x + 4) + dx;
                        int shaft_y = (center_block_y - lair_depth + 3) + dy;
                        int shaft_z = center_block_z + dz;
                        chunk_index = GetChunkIndex(shaft_x, shaft_y, shaft_z);
                        chunk[chunk_index] = BlockID::air;

                        if (dy == 0)
                        {
                            // Left polymer
                            chunk_index = GetChunkIndex(shaft_x, shaft_y, center_block_z - 3);
                            chunk[chunk_index] = BlockID::polymer;

                            // Right polymer
                            chunk_index = GetChunkIndex(shaft_x, shaft_y, center_block_z + 3);
                            chunk[chunk_index] = BlockID::polymer;
                        }

                        if (dz == 0)
                        {
                            // Top polymer
                            chunk_index = GetChunkIndex(shaft_x, center_block_y - lair_depth + 6, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;

                            // Bottom polymer
                            chunk_index = GetChunkIndex(shaft_x, center_block_y - lair_depth, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;
                        }
                    }
                }
            }

            // Polymer on back wall
            chunk_index = GetChunkIndex(center_block_x + 4 + 10, center_block_y - lair_depth + 3, center_block_z);
            chunk[chunk_index] = BlockID::polymer;

            // Extra vertical shaft?
            if (rng.Range(1, 4) == 1)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        for (int dy = 0; dy <= 20; dy++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x + 4 + 8 + dx, center_block_y - lair_depth - dy, center_block_z + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dz == 0 && dx != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x + 4 + 8 + dx + dx, center_block_y - lair_depth - dy, center_block_z);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dx == 0 && dz != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x + 4 + 8, center_block_y - lair_depth - dy, center_block_z + dz + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (left)?
            if (rng.Range(1, 4) == 1)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dz = 0; dz < 7; dz++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x + 12 + dx, center_block_y - lair_depth + 3 + dy, center_block_z + 3 + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dy == 0 && dx == -1)
                            {
                                chunk_index = GetChunkIndex(center_block_x + 12 + dx - 1, center_block_y - lair_depth + 3 + dy, center_block_z + 3 + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dx == 0 && dy != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x + 12 + dx, center_block_y - lair_depth + 3 + dy + dy, center_block_z + 3 + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (right)?
            if (rng.Range(1, 4) == 1)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dz = 0; dz < 7; dz++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x + 12 + dx, center_block_y - lair_depth + 3 + dy, center_block_z - 3 - dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dy == 0 && dx == -1)
                            {
                                chunk_index = GetChunkIndex(center_block_x + 12 + dx - 1, center_block_y - lair_depth + 3 + dy, center_block_z - 3 - dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dx == 0 && dy != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x + 12 + dx, center_block_y - lair_depth + 3 + dy + dy, center_block_z - 3 - dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }
        }

        // Extra left shaft(s)
        if (rng.Range(1, 4) == 1)
        {
            for (int dx = 0; dx < 10; dx++)
            {
                for (int dz = -2; dz <= 2; dz++)
                {
                    for (int dy = -2; dy <= 2; dy++)
                    {
                        int shaft_x = (center_block_x - 4) - dx;
                        int shaft_y = (center_block_y - lair_depth + 3) + dy;
                        int shaft_z = center_block_z + dz;
                        chunk_index = GetChunkIndex(shaft_x, shaft_y, shaft_z);
                        chunk[chunk_index] = BlockID::air;

                        if (dy == 0)
                        {
                            // Left polymer
                            chunk_index = GetChunkIndex(shaft_x, shaft_y, center_block_z - 3);
                            chunk[chunk_index] = BlockID::polymer;

                            // Right polymer
                            chunk_index = GetChunkIndex(shaft_x, shaft_y, center_block_z + 3);
                            chunk[chunk_index] = BlockID::polymer;
                        }

                        if (dz == 0)
                        {
                            // Top polymer
                            chunk_index = GetChunkIndex(shaft_x, center_block_y - lair_depth + 6, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;

                            // Bottom polymer
                            chunk_index = GetChunkIndex(shaft_x, center_block_y - lair_depth, shaft_z);
                            chunk[chunk_index] = BlockID::polymer;
                        }
                    }
                }
            }

            // Polymer on back wall
            chunk_index = GetChunkIndex(center_block_x - 4 - 10, center_block_y - lair_depth + 3, center_block_z);
            chunk[chunk_index] = BlockID::polymer;

            // Extra vertical shaft?
            if (rng.Range(1, 4) == 1)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        for (int dy = 0; dy <= 20; dy++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x - 4 - 8 + dx, center_block_y - lair_depth - dy, center_block_z + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dz == 0 && dx != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x - 4 - 8 + dx + dx, center_block_y - lair_depth - dy, center_block_z);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dx == 0 && dz != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x - 4 - 8, center_block_y - lair_depth - dy, center_block_z + dz + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (left)?
            if (rng.Range(1, 4) == 1)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dz = 0; dz < 7; dz++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x - 12 + dx, center_block_y - lair_depth + 3 + dy, center_block_z + 3 + dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dy == 0 && dx == 1)
                            {
                                chunk_index = GetChunkIndex(center_block_x - 12 + dx + 1, center_block_y - lair_depth + 3 + dy, center_block_z + 3 + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dx == 0 && dy != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x - 12 + dx, center_block_y - lair_depth + 3 + dy + dy, center_block_z + 3 + dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (right)?
            if (rng.Range(1, 4) == 1)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dz = 0; dz < 7; dz++)
                        {
                            // Carve
                            chunk_index = GetChunkIndex(center_block_x - 12 + dx, center_block_y - lair_depth + 3 + dy, center_block_z - 3 - dz);
                            chunk[chunk_index] = BlockID::air;

                            // Side polymers
                            if (dy == 0 && dx == 1)
                            {
                                chunk_index = GetChunkIndex(center_block_x - 12 + dx + 1, center_block_y - lair_depth + 3 + dy, center_block_z - 3 - dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                            else if (dx == 0 && dy != 0)
                            {
                                chunk_index = GetChunkIndex(center_block_x - 12 + dx, center_block_y - lair_depth + 3 + dy + dy, center_block_z - 3 - dz);
                                chunk[chunk_index] = BlockID::polymer;
                            }
                        }
                    }
                }
            }
        }
    }
}

//
// This was reverse-engineered from Lunacraft v2.01
//
void GenerateCrystal(BlockID *chunk, RNG &rng, float radius, int base_x, int base_z, int base_y, int segment_length, BlockID crystal_type, int recursion_depth, int direction)
{
    constexpr int directions[] = {
    //  x   z   y
        0, -1,  0,
        1,  0,  0,
        0,  1,  0,
       -1,  0,  0,
        0,  0,  1,
        0,  0, -1,
    };

    constexpr int direction_map[6][4] = { // Allowed next directions for each direction
        {1, 3, 4, 5},
        {0, 2, 4, 5},
        {1, 3, 4, 5},
        {0, 2, 4, 5},
        {0, 1, 2, 3},
        {0, 1, 2, 3},
    };

    if (segment_length <= 0 || radius < 0.1f || recursion_depth <= 0)
        return;

    int voxel_x = base_x;
    int voxel_z = base_z;
    int voxel_y = base_y;

    int dx = directions[direction * 3];
    int dz = directions[direction * 3 + 1];
    int dy = directions[direction * 3 + 2];

    for (int i = 0; i < segment_length; i++)
    {
        voxel_x += dx;
        voxel_z += dz;
        voxel_y += dy;

        if (BlockIsInChunk(voxel_x, voxel_y, voxel_z))
        {
            BlockID &current_block = chunk[GetChunkIndex(voxel_x, voxel_y, voxel_z)];
            if (current_block == BlockID::air)
                current_block = crystal_type;
        }
    }

    if (radius > 0.0f)
    {
        int branches = static_cast<int>(radius);

        for (int i = 1; i < branches; i++)
        {
            int index = (i + rng.Range(0, 3)) % 4;
            int next_direction = direction_map[direction][index];

            GenerateCrystal(
                chunk,
                rng,
                radius * 0.8f,
                voxel_x,
                voxel_z,
                voxel_y,
                segment_length - 1,
                crystal_type,
                recursion_depth - 1,
                next_direction
            );
        }
    }
}
