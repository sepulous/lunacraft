#ifndef CHUNK_GEN_H
#define CHUNK_GEN_H

#include <vector>
#include <cstdlib>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "block.h"
#include "constants.h"
#include "simplex.h"

std::vector<std::vector<glm::vec3>> CRYSTAL_PLANT_SHAPES = {
    {
        glm::vec3(2, 0, 0), // max extent

        glm::vec3(0, 1, 0), glm::vec3(0, 2, 0), glm::vec3(0, 3, 0), glm::vec3(-1, 2, 0), glm::vec3(1, 2, 0), glm::vec3(0, 2, -1),
        glm::vec3(0, 3, 1), glm::vec3(0, 3, 2), glm::vec3(0, 2, 2), glm::vec3(0, 4, 2), glm::vec3(-1, 3, 2)
    },
    {
        glm::vec3(3, 0, 0), // max extent

        glm::vec3(0, 1, 0), glm::vec3(0, 2, 0), glm::vec3(0, 3, 0), glm::vec3(0, 3, 1), glm::vec3(0, 3, 2), glm::vec3(0, 3, 3),
        glm::vec3(0, 2, 3), glm::vec3(0, 4, 3), glm::vec3(1, 3, 2), glm::vec3(2, 3, 2)
    },
    {
        glm::vec3(2, 0, 0), // max extent

        glm::vec3(0, 1, 0), glm::vec3(0, 2, 0), glm::vec3(0, 3, 0), glm::vec3(1, 3, 0), glm::vec3(2, 3, 0), glm::vec3(0, 2, -1),
        glm::vec3(0, 2, -2), glm::vec3(0, 2, 1), glm::vec3(0, 2, 2), glm::vec3(0, 1, 2), glm::vec3(0, 3, 2), glm::vec3(-1, 2, 2), glm::vec3(-2, 2, 2)
    },
    {
        glm::vec3(3, 0, 0), // max extent

        glm::vec3(0, 1, 0), glm::vec3(0, 2, 0), glm::vec3(0, 3, 0), glm::vec3(0, 4, 0), glm::vec3(-1, 4, 0), glm::vec3(-2, 4, 0),
        glm::vec3(-3, 4, 0), glm::vec3(1, 4, 0), glm::vec3(2, 4, 0), glm::vec3(3, 4, 0), glm::vec3(0, 4, 1), glm::vec3(0, 4, 2), glm::vec3(0, 4, 3)
    },
    {
        glm::vec3(3, 0, 0), // max extent

        glm::vec3(0, 1, 0), glm::vec3(0, 2, 0), glm::vec3(0, 3, 0), glm::vec3(0, 4, 0), glm::vec3(-1, 4, 0), glm::vec3(-2, 4, 0),
        glm::vec3(-3, 4, 0), glm::vec3(-3, 5, 0), glm::vec3(-3, 6, 0), glm::vec3(-3, 4, 1), glm::vec3(-3, 4, 2), glm::vec3(1, 4, 0), glm::vec3(2, 4, 0),
        glm::vec3(2, 4, 1), glm::vec3(2, 4, 2), glm::vec3(2, 4, -1), glm::vec3(2, 4, -2), glm::vec3(2, 5, 0), glm::vec3(2, 6, 0), glm::vec3(2, 3, 0), glm::vec3(2, 2, 0)
    },
    {
        glm::vec3(3, 0, 0), // max extent

        glm::vec3(0, 1, 0), glm::vec3(0, 2, 0), glm::vec3(-1, 2, 0), glm::vec3(-2, 2, 0), glm::vec3(-2, 1, 0), glm::vec3(-2, 2, 1),
        glm::vec3(-2, 2, -1), glm::vec3(1, 2, 0), glm::vec3(2, 2, 0), glm::vec3(2, 2, 1), glm::vec3(2, 2, -1), glm::vec3(2, 3, 0),
        glm::vec3(2, 1, 0), glm::vec3(0, 2, 1), glm::vec3(0, 2, 2), glm::vec3(0, 2, 3), glm::vec3(1, 2, 3), glm::vec3(0, 1, 3), glm::vec3(0, 3, 3)
    },
    {
        glm::vec3(3, 0, 0), // max extent

        glm::vec3(0, 1, 0), glm::vec3(0, 2, 0), glm::vec3(0, 3, 0), glm::vec3(-1, 3, 0), glm::vec3(-2, 3, 0), glm::vec3(-3, 3, 0),
        glm::vec3(-3, 4, 0), glm::vec3(-3, 5, 0), glm::vec3(-3, 2, 0), glm::vec3(-3, 1, 0), glm::vec3(1, 3, 0), glm::vec3(2, 3, 0),
        glm::vec3(3, 3, 0), glm::vec3(3, 3, 1), glm::vec3(3, 3, 2), glm::vec3(3, 3, -1), glm::vec3(3, 3, -2), glm::vec3(0, 3, 1),
        glm::vec3(0, 3, 2), glm::vec3(0, 3, 3), glm::vec3(-1, 3, 3), glm::vec3(-2, 3, 3), glm::vec3(1, 3, 3), glm::vec3(2, 3, 3), glm::vec3(0, 4, 3),
        glm::vec3(0, 5, 3), glm::vec3(0, 2, 3), glm::vec3(0, 1, 3)
    },
    {
        glm::vec3(3, 0, 0), // max extent

        glm::vec3(0, 1, 0), glm::vec3(0, 2, 0), glm::vec3(0, 3, 0), glm::vec3(-1, 3, 0), glm::vec3(-2, 3, 0), glm::vec3(-2, 4, 0),
        glm::vec3(-2, 5, 0), glm::vec3(-2, 2, 0), glm::vec3(-2, 1, 0), glm::vec3(1, 3, 0), glm::vec3(1, 4, 0), glm::vec3(2, 3, 0),
        glm::vec3(2, 2, 0), glm::vec3(0, 3, 1), glm::vec3(0, 3, 2), glm::vec3(0, 3, 3), glm::vec3(1, 3, 3), glm::vec3(0, 4, 3),
        glm::vec3(-1, 3, 3), glm::vec3(-2, 3, 3)
    }
};

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

int GetStructureSeed(uint64_t worldSeed, int chunkX, int chunkZ)
{
    // Convert signed coords to unsigned to avoid sign extension issues
    // uint64_t x = (uint64_t)((uint32_t)chunkX) + 341873128712UL;
    // uint64_t z = (uint64_t)((uint32_t)chunkZ) + 132897987541UL;
    uint64_t x = (uint64_t)((uint32_t)chunkX);
    uint64_t z = (uint64_t)((uint32_t)chunkZ);

    uint64_t hash = worldSeed;
    hash ^= x * 0x517CC1B727220A95UL;
    hash ^= z * 0x9E3779B97F4A7C15UL;
    hash ^= (hash >> 33);
    hash *= 0xFF51AFD7ED558CCDUL;
    hash ^= (hash >> 33);
    hash *= 0xC4CEB9FE1A85EC53UL;
    hash ^= (hash >> 33);

    return (int)(hash & 0x7FFFFFFF);
}

uint64_t splitmix64(uint64_t& x)
{
    x += 0x9E3779B97F4A7C15ULL;
    uint64_t z = x;
    z = (z ^ (z >> 30ULL)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27ULL)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31ULL);
}

int *GenerateHeightMap(int heightMapIndex, int chunkX, int chunkZ, uint64_t seed, float amplitude, float frequency, float persistence, int octaves, int terrainRoughness)
{
    int *heightMap = new int[(CHUNK_SIZE + 2)*(CHUNK_SIZE + 2)];
    float frequency0 = frequency;
    float amplitude0 = amplitude;
    float divFactor = 32.0f - 4.0f * terrainRoughness;
    float heightLimit;
    double offset_x = (double)((splitmix64(seed) << 32) >> 32);
    double offset_z = (double)((splitmix64(seed) << 32) >> 32);

    for (int x = -1; x < CHUNK_SIZE + 1; x++)
    {
        for (int z = -1; z < CHUNK_SIZE + 1; z++)
        {
            frequency = frequency0;
            amplitude = amplitude0;
            heightLimit = 0.0f;
            for (int i = 0; i < octaves; i++)
            {
                double xArg = (((x + chunkX * CHUNK_SIZE) + offset_x) / divFactor) * frequency;
                double zArg = (((z + chunkZ * CHUNK_SIZE) + offset_z) / divFactor) * frequency;
                heightLimit += SimplexNoise(xArg, zArg) * amplitude;
                frequency *= 3.0f;
                amplitude *= persistence;
            }
            heightMap[(z + 1) + (CHUNK_SIZE + 2) * (x + 1)] = (int)heightLimit;
        }
    }

    return heightMap;
}

int RandomRange(int minInclusive, int maxExclusive)
{
    int range = maxExclusive - minInclusive - 1;
    return (int)((float)std::rand() * range / RAND_MAX) + minInclusive;
}

void GenerateChunk(uint16_t *chunk, int chunkX, int chunkZ, uint64_t seed)
{
    //
    // Generate terrain
    //
    int *rockHeightMap = GenerateHeightMap(0, chunkX, chunkZ, seed, 16, 0.4, 0.4, 4, 2);
    int *gravelHeightMap = GenerateHeightMap(1, chunkX, chunkZ, seed, 4, 0.4, 0.6, 2, 2);
    int *dirtHeightMap = GenerateHeightMap(2, chunkX, chunkZ, seed, 3, 0.4, 0.4, 3, 2);
    int *sandHeightMap = GenerateHeightMap(3, chunkX, chunkZ, seed, 2, 0.4, 0.8, 2, 2);

    int chunkIndex = 0;
    for (int x = 0; x < CHUNK_SIZE + 2; x++)
    {
        for (int z = 0; z < CHUNK_SIZE + 2; z++)
        {
            int rockHeightLimit = rockHeightMap[z + (CHUNK_SIZE + 2) * x];
            int gravelHeightLimit = gravelHeightMap[z + (CHUNK_SIZE + 2) * x];
            int dirtHeightLimit = dirtHeightMap[z + (CHUNK_SIZE + 2) * x];
            int sandHeightLimit = sandHeightMap[z + (CHUNK_SIZE + 2) * x];
            int y = 0;

            // Base rock
            while (y < 50 + rockHeightLimit)
            {
                chunk[chunkIndex++] = (uint16_t)BlockID::rock;
                y++;
            }

            // Base gravel
            while (y < 50 + rockHeightLimit + gravelHeightLimit)
            {
                chunk[chunkIndex++] = (uint16_t)BlockID::gravel;
                y++;
            }

            // Base dirt
            while (y < 50 + rockHeightLimit + gravelHeightLimit + dirtHeightLimit)
            {
                chunk[chunkIndex++] = (uint16_t)BlockID::dirt;
                y++;
            }

            if (50 + rockHeightLimit + gravelHeightLimit + dirtHeightLimit < GROUND_LEVEL) // Below ground level; fill rest in with sand/water
            {
                while (y < GROUND_LEVEL && y < 50 + rockHeightLimit + gravelHeightLimit + dirtHeightLimit + sandHeightLimit)
                {
                    chunk[chunkIndex++] = (uint16_t)BlockID::sand;
                    y++;
                }

                while (y < GROUND_LEVEL)
                {
                    chunk[chunkIndex++] = (uint16_t)BlockID::water;
                    y++;
                }
            }
            else // Above ground level; finish terrain by placing topsoil
            {
                chunk[chunkIndex++] = (uint16_t)BlockID::topsoil;
                y++;
            }

            while (y < WORLD_HEIGHT_LIMIT)
            {
                chunk[chunkIndex++] = (uint16_t)BlockID::air;
                y++;
            }
        }
    }
    free(rockHeightMap);
    free(gravelHeightMap);
    free(dirtHeightMap);
    free(sandHeightMap);

    // Ensure the same seed puts the same structures in the same places
    //UnityEngine.Random.State initialRandomState = UnityEngine.Random.state;
    //int structureSeed = (int)((69 >> 32) ^ (69 & 0xFFFFFFFF)) ^ chunkX ^ chunkZ;
    // int structureSeed = GetStructureSeed(69, chunkX, chunkZ);
    int structureSeed = GetStructureSeed(seed, chunkX, chunkZ);
    std::srand(structureSeed);

    //
    // Ores
    //
    //int oreSpawnChance = RandomRange(0, 10);
    int oreSpawnChance = RandomRange(0, 10);
    if (oreSpawnChance <= 3)
    {
        int seedBlockX = RandomRange(5, CHUNK_SIZE - 6);
        int seedBlockZ = RandomRange(5, CHUNK_SIZE - 6);
        int seedBlockY = -1;
        for (int y = 63; y < WORLD_HEIGHT_LIMIT; y++)
        {
            chunkIndex = GetChunkIndex(seedBlockX, y + 1, seedBlockZ);
            if (chunk[chunkIndex] == (uint16_t)BlockID::air)
            {
                seedBlockY = y;
                break;
            }
        }

        int ore = RandomRange(1, 101);
        int veinSize;
        BlockID oreID;
        if (ore <= 36) // 36%
        {
            oreID = BlockID::magnetite;
            veinSize = RandomRange(2, 7);
        }
        else if (ore <= 60) // 24%
        {
            oreID = BlockID::aluminum_ore;
            veinSize = RandomRange(2, 7);
        }
        else if (ore <= 78) // 18%
        {
            oreID = BlockID::titanium_ore;
            veinSize = RandomRange(2, 7);
        }
        else if (ore <= 91) // 13%
        {
            oreID = BlockID::gold_ore;
            veinSize = RandomRange(1, 5);
        }
        else if (ore <= 98) // 7%
        {
            oreID = BlockID::notchium_ore;
            veinSize = RandomRange(1, 5);
        }
        else // 2%
        {
            oreID = BlockID::blue_crystal;
            veinSize = RandomRange(1, 5);
        }

        int currentBlockX = seedBlockX;
        int currentBlockY = seedBlockY;
        int currentBlockZ = seedBlockZ;
        chunkIndex = GetChunkIndex(seedBlockX, seedBlockY, seedBlockZ);
        chunk[chunkIndex] = (uint16_t)oreID;
        for (int count = 0; count < veinSize; count++)
        {
            int nextDirection = RandomRange(1, 6);
            if (nextDirection == 1) // Forward
            {
                currentBlockZ++;
            }
            else if (nextDirection == 2) // Backward
            {
                currentBlockZ--;
            }
            else if (nextDirection == 3) // Right
            {
                currentBlockX++;
            }
            else if (nextDirection == 4) // Left
            {
                currentBlockX--;
            }
            else // Down
            {
                currentBlockY--;
            }

            chunkIndex = GetChunkIndex(currentBlockX, currentBlockY, currentBlockZ);
            if (chunk[chunkIndex] != (uint16_t)BlockID::air)
                chunk[chunkIndex] = (uint16_t)oreID;
        }
    }

    //
    // Astronaut lairs
    //
    bool spawnAstronautLair = RandomRange(0, 100) == 69; // 1% chance, each chunk
    const int lairDepth = 26;
    if (spawnAstronautLair)
    {
        int centerBlockX = (int)(CHUNK_SIZE / 2);
        int centerBlockZ = (int)(CHUNK_SIZE / 2);
        int centerBlockY = -1;
        for (int y = 64; y < WORLD_HEIGHT_LIMIT; y++)
        {
            chunkIndex = GetChunkIndex(centerBlockX, y + 1, centerBlockZ);
            if (chunk[chunkIndex] == (uint16_t)BlockID::air)
            {
                centerBlockY = y;
                break;
            }
        }

        // Place gravel block at center of bottom so I can easily check whether a chunk contains an astronaut lair
        chunkIndex = GetChunkIndex(centerBlockX, 0, centerBlockZ);
        chunk[chunkIndex] = (uint16_t)BlockID::gravel;

        // Carve main shaft
        for (int xOffset = -3; xOffset <= 3; xOffset++)
        {
            for (int zOffset = -3; zOffset <= 3; zOffset++)
            {
                for (int yOffset = -6; yOffset <= lairDepth; yOffset++)
                {
                    chunkIndex = GetChunkIndex(centerBlockX + xOffset, centerBlockY - yOffset, centerBlockZ + zOffset);
                    chunk[chunkIndex] = (uint16_t)BlockID::air;
                }
            }
        }

        // Decorate main shaft with polymer and light
        bool leftSideDone = false;
        bool rightSideDone = false;
        bool frontSideDone = false;
        bool backSideDone = false;
        for (int y = centerBlockY - lairDepth; y < WORLD_HEIGHT_LIMIT; y++)
        {
            chunkIndex = GetChunkIndex(centerBlockX - 4, y, centerBlockZ);
            if (chunk[chunkIndex] != (uint16_t)BlockID::air && !leftSideDone)
            {
                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                if ((y - (centerBlockY - lairDepth)) / 8.0f == 3)
                    chunk[chunkIndex] = (uint16_t)BlockID::light;
            }
            else
            {
                leftSideDone = true;
            }

            chunkIndex = GetChunkIndex(centerBlockX + 4, y, centerBlockZ);
            if (chunk[chunkIndex] != (uint16_t)BlockID::air && !rightSideDone)
            {
                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                if ((y - (centerBlockY - lairDepth)) / 8.0f == 3)
                    chunk[chunkIndex] = (uint16_t)BlockID::light;
            }
            else
            {
                rightSideDone = true;
            }

            chunkIndex = GetChunkIndex(centerBlockX, y, centerBlockZ + 4);
            if (chunk[chunkIndex] != (uint16_t)BlockID::air && !frontSideDone)
            {
                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                if ((y - (centerBlockY - lairDepth)) / 8.0f == 3)
                    chunk[chunkIndex] = (uint16_t)BlockID::light;
            }
            else
            {
                frontSideDone = true;
            }

            chunkIndex = GetChunkIndex(centerBlockX, y, centerBlockZ - 4);
            if (chunk[chunkIndex] != (uint16_t)BlockID::air && !backSideDone)
            {
                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                if ((y - (centerBlockY - lairDepth)) / 8.0f == 3)
                    chunk[chunkIndex] = (uint16_t)BlockID::light;
            }
            else
            {
                backSideDone = true;
            }

            if (leftSideDone && rightSideDone && frontSideDone && backSideDone)
                break;
        }

        // Polymer at bottom center
        chunkIndex = GetChunkIndex(centerBlockX, centerBlockY - lairDepth - 1, centerBlockZ);
        chunk[chunkIndex] = (uint16_t)BlockID::polymer;

        // Extra front shaft(s)
        if (RandomRange(0, 4) == 0)
        {
            for (int dz = 0; dz < 10; dz++)
            {
                for (int dx = -2; dx <= 2; dx++)
                {
                    for (int dy = -2; dy <= 2; dy++)
                    {
                        int shaftX = centerBlockX + dx;
                        int shaftY = (centerBlockY - lairDepth + 3) + dy;
                        int shaftZ = (centerBlockZ + 4) + dz;
                        chunkIndex = GetChunkIndex(shaftX, shaftY, shaftZ);
                        chunk[chunkIndex] = (uint16_t)BlockID::air;

                        if (dy == 0)
                        {
                            // Left polymer
                            chunkIndex = GetChunkIndex(centerBlockX - 3, shaftY, shaftZ);
                            chunk[chunkIndex] = (uint16_t)BlockID::polymer;

                            // Right polymer
                            chunkIndex = GetChunkIndex(centerBlockX + 3, shaftY, shaftZ);
                            chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                        }

                        if (dx == 0)
                        {
                            // Top polymer
                            chunkIndex = GetChunkIndex(shaftX, centerBlockY - lairDepth + 6, shaftZ);
                            chunk[chunkIndex] = (uint16_t)BlockID::polymer;

                            // Bottom polymer
                            chunkIndex = GetChunkIndex(shaftX, centerBlockY - lairDepth, shaftZ);
                            chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                        }
                    }
                }
            }

            // Polymer on back wall
            chunkIndex = GetChunkIndex(centerBlockX, centerBlockY - lairDepth + 3, centerBlockZ + 4 + 10);
            chunk[chunkIndex] = (uint16_t)BlockID::polymer;

            // Extra vertical shaft?
            if (RandomRange(0, 4) == 0)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        for (int dy = 0; dy <= 20; dy++)
                        {
                            // Carve
                            chunkIndex = GetChunkIndex(centerBlockX + dx, centerBlockY - lairDepth - dy, centerBlockZ + 4 + 8 + dz);
                            chunk[chunkIndex] = (uint16_t)BlockID::air;

                            // Side polymers
                            if (dz == 0 && dx != 0)
                            {
                                chunkIndex = GetChunkIndex(centerBlockX + dx + dx, centerBlockY - lairDepth - dy, centerBlockZ + 4 + 8);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                            else if (dx == 0 && dz != 0)
                            {
                                chunkIndex = GetChunkIndex(centerBlockX, centerBlockY - lairDepth - dy, centerBlockZ + 4 + 8 + dz + dz);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (left)?
            if (RandomRange(0, 4) == 0)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dx = 0; dx < 7; dx++)
                        {
                            // Carve
                            chunkIndex = GetChunkIndex(centerBlockX - 3 - dx, centerBlockY - lairDepth + 3 + dy, centerBlockZ + 12 + dz);
                            chunk[chunkIndex] = (uint16_t)BlockID::air;

                            // Side polymers
                            if (dy == 0 && dz == -1) // left
                            {
                                chunkIndex = GetChunkIndex(centerBlockX - 3 - dx, centerBlockY - lairDepth + 3 + dy, centerBlockZ + 12 + dz - 1);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                            else if (dz == 0 && dy != 0) // top and bottom
                            {
                                chunkIndex = GetChunkIndex(centerBlockX - 3 - dx, centerBlockY - lairDepth + 3 + dy + dy, centerBlockZ + 12 + dz);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (right)?
            if (RandomRange(0, 4) == 0)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dx = 0; dx < 7; dx++)
                        {
                            // Carve
                            chunkIndex = GetChunkIndex(centerBlockX + 3 + dx, centerBlockY - lairDepth + 3 + dy, centerBlockZ + 12 + dz);
                            chunk[chunkIndex] = (uint16_t)BlockID::air;

                            // Side polymers
                            if (dy == 0 && dz == -1) // right
                            {
                                chunkIndex = GetChunkIndex(centerBlockX + 3 + dx, centerBlockY - lairDepth + 3 + dy, centerBlockZ + 12 + dz - 1);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                            else if (dz == 0 && dy != 0) // top and bottom
                            {
                                chunkIndex = GetChunkIndex(centerBlockX + 3 + dx, centerBlockY - lairDepth + 3 + dy + dy, centerBlockZ + 12 + dz);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                        }
                    }
                }
            }
        }

        // Extra back shaft(s)
        if (RandomRange(0, 4) == 0)
        {
            for (int dz = 0; dz < 10; dz++)
            {
                for (int dx = -2; dx <= 2; dx++)
                {
                    for (int dy = -2; dy <= 2; dy++)
                    {
                        int shaftX = centerBlockX + dx;
                        int shaftY = (centerBlockY - lairDepth + 3) + dy;
                        int shaftZ = (centerBlockZ - 4) - dz;
                        chunkIndex = GetChunkIndex(shaftX, shaftY, shaftZ);
                        chunk[chunkIndex] = (uint16_t)BlockID::air;

                        if (dy == 0)
                        {
                            // Left polymer
                            chunkIndex = GetChunkIndex(centerBlockX - 3, shaftY, shaftZ);
                            chunk[chunkIndex] = (uint16_t)BlockID::polymer;

                            // Right polymer
                            chunkIndex = GetChunkIndex(centerBlockX + 3, shaftY, shaftZ);
                            chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                        }

                        if (dx == 0)
                        {
                            // Top polymer
                            chunkIndex = GetChunkIndex(shaftX, centerBlockY - lairDepth + 6, shaftZ);
                            chunk[chunkIndex] = (uint16_t)BlockID::polymer;

                            // Bottom polymer
                            chunkIndex = GetChunkIndex(shaftX, centerBlockY - lairDepth, shaftZ);
                            chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                        }
                    }
                }
            }

            // Polymer on back wall
            chunkIndex = GetChunkIndex(centerBlockX, centerBlockY - lairDepth + 3, centerBlockZ - 4 - 10);
            chunk[chunkIndex] = (uint16_t)BlockID::polymer;

            // Extra vertical shaft?
            if (RandomRange(0, 4) == 0)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        for (int dy = 0; dy <= 20; dy++)
                        {
                            // Carve
                            chunkIndex = GetChunkIndex(centerBlockX + dx, centerBlockY - lairDepth - dy, centerBlockZ - 12 + dz);
                            chunk[chunkIndex] = (uint16_t)BlockID::air;

                            // Side polymers
                            if (dz == 0 && dx != 0)
                            {
                                chunkIndex = GetChunkIndex(centerBlockX + dx + dx, centerBlockY - lairDepth - dy, centerBlockZ - 12);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                            else if (dx == 0 && dz != 0)
                            {
                                chunkIndex = GetChunkIndex(centerBlockX, centerBlockY - lairDepth - dy, centerBlockZ - 12 + dz + dz);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (left)?
            if (RandomRange(0, 4) == 0)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dx = 0; dx < 7; dx++)
                        {
                            // Carve
                            chunkIndex = GetChunkIndex(centerBlockX - 3 - dx, centerBlockY - lairDepth + 3 + dy, centerBlockZ - 12 + dz);
                            chunk[chunkIndex] = (uint16_t)BlockID::air;

                            // Side polymers
                            if (dy == 0 && dz == 1) // left
                            {
                                chunkIndex = GetChunkIndex(centerBlockX - 3 - dx, centerBlockY - lairDepth + 3 + dy, centerBlockZ - 12 + dz + 1);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                            else if (dz == 0 && dy != 0) // top and bottom
                            {
                                chunkIndex = GetChunkIndex(centerBlockX - 3 - dx, centerBlockY - lairDepth + 3 + dy + dy, centerBlockZ - 12 + dz);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (right)?
            if (RandomRange(0, 4) == 0)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dx = 0; dx < 7; dx++)
                        {
                            // Carve
                            chunkIndex = GetChunkIndex(centerBlockX + 3 + dx, centerBlockY - lairDepth + 3 + dy, centerBlockZ - 12 + dz);
                            chunk[chunkIndex] = (uint16_t)BlockID::air;

                            // Side polymers
                            if (dy == 0 && dz == 1) // right
                            {
                                chunkIndex = GetChunkIndex(centerBlockX + 3 + dx, centerBlockY - lairDepth + 3 + dy, centerBlockZ - 12 + dz + 1);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                            else if (dz == 0 && dy != 0) // top and bottom
                            {
                                chunkIndex = GetChunkIndex(centerBlockX + 3 + dx, centerBlockY - lairDepth + 3 + dy + dy, centerBlockZ - 12 + dz);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                        }
                    }
                }
            }
        }

        // Extra right shaft(s)
        if (RandomRange(0, 4) == 0)
        {
            for (int dx = 0; dx < 10; dx++)
            {
                for (int dz = -2; dz <= 2; dz++)
                {
                    for (int dy = -2; dy <= 2; dy++)
                    {
                        int shaftX = (centerBlockX + 4) + dx;
                        int shaftY = (centerBlockY - lairDepth + 3) + dy;
                        int shaftZ = centerBlockZ + dz;
                        chunkIndex = GetChunkIndex(shaftX, shaftY, shaftZ);
                        chunk[chunkIndex] = (uint16_t)BlockID::air;

                        if (dy == 0)
                        {
                            // Left polymer
                            chunkIndex = GetChunkIndex(shaftX, shaftY, centerBlockZ - 3);
                            chunk[chunkIndex] = (uint16_t)BlockID::polymer;

                            // Right polymer
                            chunkIndex = GetChunkIndex(shaftX, shaftY, centerBlockZ + 3);
                            chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                        }

                        if (dz == 0)
                        {
                            // Top polymer
                            chunkIndex = GetChunkIndex(shaftX, centerBlockY - lairDepth + 6, shaftZ);
                            chunk[chunkIndex] = (uint16_t)BlockID::polymer;

                            // Bottom polymer
                            chunkIndex = GetChunkIndex(shaftX, centerBlockY - lairDepth, shaftZ);
                            chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                        }
                    }
                }
            }

            // Polymer on back wall
            chunkIndex = GetChunkIndex(centerBlockX + 4 + 10, centerBlockY - lairDepth + 3, centerBlockZ);
            chunk[chunkIndex] = (uint16_t)BlockID::polymer;

            // Extra vertical shaft?
            if (RandomRange(0, 4) == 0)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        for (int dy = 0; dy <= 20; dy++)
                        {
                            // Carve
                            chunkIndex = GetChunkIndex(centerBlockX + 4 + 8 + dx, centerBlockY - lairDepth - dy, centerBlockZ + dz);
                            chunk[chunkIndex] = (uint16_t)BlockID::air;

                            // Side polymers
                            if (dz == 0 && dx != 0)
                            {
                                chunkIndex = GetChunkIndex(centerBlockX + 4 + 8 + dx + dx, centerBlockY - lairDepth - dy, centerBlockZ);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                            else if (dx == 0 && dz != 0)
                            {
                                chunkIndex = GetChunkIndex(centerBlockX + 4 + 8, centerBlockY - lairDepth - dy, centerBlockZ + dz + dz);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (left)?
            if (RandomRange(0, 4) == 0)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dz = 0; dz < 7; dz++)
                        {
                            // Carve
                            chunkIndex = GetChunkIndex(centerBlockX + 12 + dx, centerBlockY - lairDepth + 3 + dy, centerBlockZ + 3 + dz);
                            chunk[chunkIndex] = (uint16_t)BlockID::air;

                            // Side polymers
                            if (dy == 0 && dx == -1)
                            {
                                chunkIndex = GetChunkIndex(centerBlockX + 12 + dx - 1, centerBlockY - lairDepth + 3 + dy, centerBlockZ + 3 + dz);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                            else if (dx == 0 && dy != 0)
                            {
                                chunkIndex = GetChunkIndex(centerBlockX + 12 + dx, centerBlockY - lairDepth + 3 + dy + dy, centerBlockZ + 3 + dz);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (right)?
            if (RandomRange(0, 4) == 0)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dz = 0; dz < 7; dz++)
                        {
                            // Carve
                            chunkIndex = GetChunkIndex(centerBlockX + 12 + dx, centerBlockY - lairDepth + 3 + dy, centerBlockZ - 3 - dz);
                            chunk[chunkIndex] = (uint16_t)BlockID::air;

                            // Side polymers
                            if (dy == 0 && dx == -1)
                            {
                                chunkIndex = GetChunkIndex(centerBlockX + 12 + dx - 1, centerBlockY - lairDepth + 3 + dy, centerBlockZ - 3 - dz);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                            else if (dx == 0 && dy != 0)
                            {
                                chunkIndex = GetChunkIndex(centerBlockX + 12 + dx, centerBlockY - lairDepth + 3 + dy + dy, centerBlockZ - 3 - dz);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                        }
                    }
                }
            }
        }

        // Extra left shaft(s)
        if (RandomRange(0, 4) == 0)
        {
            for (int dx = 0; dx < 10; dx++)
            {
                for (int dz = -2; dz <= 2; dz++)
                {
                    for (int dy = -2; dy <= 2; dy++)
                    {
                        int shaftX = (centerBlockX - 4) - dx;
                        int shaftY = (centerBlockY - lairDepth + 3) + dy;
                        int shaftZ = centerBlockZ + dz;
                        chunkIndex = GetChunkIndex(shaftX, shaftY, shaftZ);
                        chunk[chunkIndex] = (uint16_t)BlockID::air;

                        if (dy == 0)
                        {
                            // Left polymer
                            chunkIndex = GetChunkIndex(shaftX, shaftY, centerBlockZ - 3);
                            chunk[chunkIndex] = (uint16_t)BlockID::polymer;

                            // Right polymer
                            chunkIndex = GetChunkIndex(shaftX, shaftY, centerBlockZ + 3);
                            chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                        }

                        if (dz == 0)
                        {
                            // Top polymer
                            chunkIndex = GetChunkIndex(shaftX, centerBlockY - lairDepth + 6, shaftZ);
                            chunk[chunkIndex] = (uint16_t)BlockID::polymer;

                            // Bottom polymer
                            chunkIndex = GetChunkIndex(shaftX, centerBlockY - lairDepth, shaftZ);
                            chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                        }
                    }
                }
            }

            // Polymer on back wall
            chunkIndex = GetChunkIndex(centerBlockX - 4 - 10, centerBlockY - lairDepth + 3, centerBlockZ);
            chunk[chunkIndex] = (uint16_t)BlockID::polymer;

            // Extra vertical shaft?
            if (RandomRange(0, 4) == 0)
            {
                for (int dz = -1; dz <= 1; dz++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        for (int dy = 0; dy <= 20; dy++)
                        {
                            // Carve
                            chunkIndex = GetChunkIndex(centerBlockX - 4 - 8 + dx, centerBlockY - lairDepth - dy, centerBlockZ + dz);
                            chunk[chunkIndex] = (uint16_t)BlockID::air;

                            // Side polymers
                            if (dz == 0 && dx != 0)
                            {
                                chunkIndex = GetChunkIndex(centerBlockX - 4 - 8 + dx + dx, centerBlockY - lairDepth - dy, centerBlockZ);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                            else if (dx == 0 && dz != 0)
                            {
                                chunkIndex = GetChunkIndex(centerBlockX - 4 - 8, centerBlockY - lairDepth - dy, centerBlockZ + dz + dz);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (left)?
            if (RandomRange(0, 4) == 0)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dz = 0; dz < 7; dz++)
                        {
                            // Carve
                            chunkIndex = GetChunkIndex(centerBlockX - 12 + dx, centerBlockY - lairDepth + 3 + dy, centerBlockZ + 3 + dz);
                            chunk[chunkIndex] = (uint16_t)BlockID::air;

                            // Side polymers
                            if (dy == 0 && dx == 1)
                            {
                                chunkIndex = GetChunkIndex(centerBlockX - 12 + dx + 1, centerBlockY - lairDepth + 3 + dy, centerBlockZ + 3 + dz);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                            else if (dx == 0 && dy != 0)
                            {
                                chunkIndex = GetChunkIndex(centerBlockX - 12 + dx, centerBlockY - lairDepth + 3 + dy + dy, centerBlockZ + 3 + dz);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                        }
                    }
                }
            }

            // Extra horizontal shaft (right)?
            if (RandomRange(0, 4) == 0)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        for (int dz = 0; dz < 7; dz++)
                        {
                            // Carve
                            chunkIndex = GetChunkIndex(centerBlockX - 12 + dx, centerBlockY - lairDepth + 3 + dy, centerBlockZ - 3 - dz);
                            chunk[chunkIndex] = (uint16_t)BlockID::air;

                            // Side polymers
                            if (dy == 0 && dx == 1)
                            {
                                chunkIndex = GetChunkIndex(centerBlockX - 12 + dx + 1, centerBlockY - lairDepth + 3 + dy, centerBlockZ - 3 - dz);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                            else if (dx == 0 && dy != 0)
                            {
                                chunkIndex = GetChunkIndex(centerBlockX - 12 + dx, centerBlockY - lairDepth + 3 + dy + dy, centerBlockZ - 3 - dz);
                                chunk[chunkIndex] = (uint16_t)BlockID::polymer;
                            }
                        }
                    }
                }
            }
        }
    }

    //
    // Crystal plants
    //
    int spawnCrystalPlant = RandomRange(0, 40);
    if (true || spawnCrystalPlant == 0)
    {
        int crystalPlantType = RandomRange(0, 5);
        BlockID crystal;
        if (crystalPlantType == 0)
            crystal = BlockID::blue_crystal;
        else if (crystalPlantType < 3)
            crystal = BlockID::sulphur_crystal;
        else
            crystal = BlockID::boron_crystal;

        int crystalPlantOrientation = RandomRange(1, 5);
        int crystalPlantShape = RandomRange(0, CRYSTAL_PLANT_SHAPES.size());
        auto shapeOffsets = CRYSTAL_PLANT_SHAPES[crystalPlantShape];

        int paddingNeeded = shapeOffsets[0].x;
        int baseBlockX = RandomRange(paddingNeeded, CHUNK_SIZE - paddingNeeded);
        int baseBlockZ = RandomRange(paddingNeeded, CHUNK_SIZE - paddingNeeded);
        int baseBlockY;
        for (int y = 63; y < WORLD_HEIGHT_LIMIT; y++)
        {
            chunkIndex = GetChunkIndex(baseBlockX, y, baseBlockZ);
            if (chunk[chunkIndex + 1] == (uint16_t)BlockID::air)
            {
                if (chunk[chunkIndex] == (uint16_t)BlockID::topsoil || chunk[chunkIndex] == (uint16_t)BlockID::sand)
                {
                    baseBlockY = y;
                    for (int i = 1; i < shapeOffsets.size(); i++)
                    {
                        glm::vec3 offset = shapeOffsets[i];
                        if (crystalPlantOrientation == 2) // 90 degrees
                            (offset.x, offset.z) = (-offset.z, offset.x);
                        else if (crystalPlantOrientation == 3) // 180 degrees
                            (offset.x, offset.z) = (-offset.x, -offset.z);
                        else if (crystalPlantOrientation == 4) // 270 degrees
                            (offset.x, offset.z) = (offset.z, -offset.x);

                        chunkIndex = GetChunkIndex(baseBlockX + offset.x, baseBlockY + offset.y, baseBlockZ + offset.z);
                        chunk[chunkIndex] = (uint16_t)crystal;
                    }
                }
                break;
            }
        }
    }

    //
    // Trees
    //
    int numberOfTrees = RandomRange(1, 4);
    for (int i = 0; i < numberOfTrees; i++)
    {
        int treeType = RandomRange(0, 10);
        int treeOrientation = RandomRange(1, 5);

        int treeShape;
        std::vector<TreeBlock> treeData;
        if (treeType < 5) // Green light tree
        {
            treeShape = RandomRange(0, GREEN_LIGHT_TREE_SHAPES.size());
            treeData = GREEN_LIGHT_TREE_SHAPES[treeShape];
        }
        else if (treeType < 8) // Color wood tree
        {
            treeShape = RandomRange(0, COLOR_WOOD_TREE_SHAPES.size());
            treeData = COLOR_WOOD_TREE_SHAPES[treeShape];
        }
        else // Spiral light tree
        {
            treeShape = RandomRange(0, SPIRAL_LIGHT_TREE_SHAPES.size());
            treeData = SPIRAL_LIGHT_TREE_SHAPES[treeShape];
        }

        int paddingNeeded = treeData[0].local_x;
        int baseBlockX = RandomRange(paddingNeeded, CHUNK_SIZE - paddingNeeded);
        int baseBlockZ = RandomRange(paddingNeeded, CHUNK_SIZE - paddingNeeded);
        int baseBlockY;
        for (int y = 63; y < WORLD_HEIGHT_LIMIT; y++)
        {
            chunkIndex = GetChunkIndex(baseBlockX, y, baseBlockZ);
            if (chunk[chunkIndex + 1] == (uint16_t)BlockID::air)
            {
                if (chunk[chunkIndex] == (uint16_t)BlockID::topsoil || chunk[chunkIndex] == (uint16_t)BlockID::sand)
                {
                    baseBlockY = y;
                    for (int j = 1; j < treeData.size(); j++)
                    {
                        TreeBlock treeBlock = treeData[j];
                        if (treeOrientation == 2) // 90 degrees
                            (treeBlock.local_x, treeBlock.local_z) = (-treeBlock.local_z, treeBlock.local_x);
                        else if (treeOrientation == 3) // 180 degrees
                            (treeBlock.local_x, treeBlock.local_z) = (-treeBlock.local_x, -treeBlock.local_z);
                        else if (treeOrientation == 4) // 270 degrees
                            (treeBlock.local_x, treeBlock.local_z) = (treeBlock.local_z, -treeBlock.local_x);

                        chunkIndex = GetChunkIndex(baseBlockX + treeBlock.local_x, baseBlockY + treeBlock.local_y, baseBlockZ + treeBlock.local_z);
                        chunk[chunkIndex] = (uint16_t)treeBlock.block;

                        // Partial fix for overhanging trees when placed on an edge
                        if (treeBlock.local_y == 1 && chunk[chunkIndex - 1] == (uint16_t)BlockID::air && chunk[chunkIndex - 2] != (uint16_t)BlockID::air)
                            chunk[chunkIndex - 1] = (uint16_t)treeBlock.block;
                    }
                }
                break;
            }
        }
    }

    //UnityEngine.Random.state = initialRandomState; // Reset so we don't interfere with anything else (but maybe every use of Random should be based on the seed?)
}

#endif
