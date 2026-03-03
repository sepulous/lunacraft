#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "constants.h"
#include "block.h"
#include "helpers.h"
#include "chunk.h"

struct BlockQuad
{
    BlockID block;
    glm::ivec3 base_coords; // Local voxel position
    glm::vec3 du;
    glm::vec3 dv;
    bool back_face;
};

/*
    The algorithm works as follows.
    
    For each axis, we consider all planes perpendicular to that axis slicing through the block faces (if N is the length of the chunk along the axis, there
    are N+1 such planes). It should be clear that each plane represents a set of block faces that could be rendered. For each of these planes, we first construct
    a mask that indicates which block face, if any, should be rendered at each position, as well as whether it's a front or back face.

    Then, for each column of this mask, we move up to the first renderable face. If none is found, we move on to the next column. If one is found, we begin with
    a 1x1 quad, and extend its height until a different mask value is reached. We then extend this new quad's width as far as possible to get the final quad.
    Finally, all mask positions covered by the final quad are marked as non-renderable, and we continue iterating through this column.
*/
std::vector<BlockQuad> GreedyMesh(BlockID *blocks, const Lightmap &lightmap, std::array<Chunk *, 4> neighbors)
{
    struct MaskEntry
    {
        BlockID block;
        uint8_t light;
        bool back_face;

        MaskEntry() { block = BlockID::air, back_face = false; }
        MaskEntry(BlockID block, uint8_t light, bool back_face) : block(block), light(light), back_face(back_face) {}
        bool operator==(const MaskEntry& other) { return block == other.block && light == other.light && back_face == other.back_face; }
        bool operator!=(const MaskEntry& other) { return block != other.block || light != other.light || back_face != other.back_face; }
    };

    std::vector<MaskEntry> mask;
    mask.reserve(CHUNK_SIZE * WORLD_HEIGHT_LIMIT);

    std::vector<BlockQuad> quads;
    quads.reserve(1500); // 1500 = empirically determined maximum (with padding). Depends on generation and meshing algorithms.

    auto front_neighbor = neighbors[0];
    auto right_neighbor = neighbors[1];
    auto back_neighbor = neighbors[2];
    auto left_neighbor = neighbors[3];

    //
    // X axis
    //

    // For each ZY plane
    for (int block_x = 0; block_x < CHUNK_SIZE; block_x++)
    {
        mask.clear();

        // Build mask
        for (int block_z = 0; block_z < CHUNK_SIZE; block_z++)
        {
            for (int block_y = 0; block_y < WORLD_HEIGHT_LIMIT; block_y++)
            {
                glm::ivec3 left_block_coords;
                BlockID left_block;

                glm::ivec3 right_block_coords = {block_x, block_y, block_z};
                BlockID right_block = blocks[GetChunkIndex(right_block_coords)];

                if (block_x == 0)
                {
                    left_block_coords = {CHUNK_SIZE - 1, block_y, block_z};
                    left_block = left_neighbor->GetBlocks()[GetChunkIndex(left_block_coords)];
                }
                else
                {
                    left_block_coords = {block_x - 1, block_y, block_z};
                    left_block = blocks[GetChunkIndex(left_block_coords)];
                }

                /*
                 *  Need to rethink how to sample light here. If we render the left block,
                 *  we sample where the right block is, and vice versa. We use neighbor
                 *  lightmaps on edges.
                 */

                if (ShouldRenderFace(left_block, right_block) && !ShouldRenderFace(right_block, left_block))
                {
                    uint8_t light = lightmap.GetCombinedLight(right_block_coords);
                    mask.emplace_back(left_block, light, false);
                }
                else if (ShouldRenderFace(right_block, left_block) && !ShouldRenderFace(left_block, right_block))
                {
                    uint8_t light;
                    if (block_x == 0)
                        light = left_neighbor->GetLightmap().GetCombinedLight(left_block_coords);
                    else
                        light = lightmap.GetCombinedLight(left_block_coords);

                    mask.emplace_back(right_block, light, true);
                }
                else
                {
                    mask.emplace_back(BlockID::air, 0, false); // Don't render
                }
            }
        }

        // Use mask to construct quads
        for (int block_z = 0; block_z < CHUNK_SIZE; block_z++)
        {
            int y = 0;
            while (y < WORLD_HEIGHT_LIMIT)
            {
                MaskEntry base_entry = mask[y + block_z * WORLD_HEIGHT_LIMIT];

                // Skip empty faces
                if (base_entry.block == BlockID::air)
                {
                    y++;
                    continue;
                }

                // Determine quad height (along Y)
                int quad_height = 1;
                while (y + quad_height < WORLD_HEIGHT_LIMIT && mask[(y + quad_height) + block_z * WORLD_HEIGHT_LIMIT] == base_entry)
                    quad_height++;

                // Determine quad width (along Z)
                int quad_width = 1;
                for (int z2 = block_z + 1; z2 < CHUNK_SIZE; z2++)
                {
                    bool column_matches = true;
                    for (int y2 = y; y2 < y + quad_height; y2++)
                    {
                        if (mask[y2 + z2 * WORLD_HEIGHT_LIMIT] != base_entry)
                        {
                            column_matches = false;
                            break;
                        }
                    }

                    if (!column_matches)
                        break;

                    quad_width++;
                }

                // Mark mask entries covered by this quad as "handled"
                for (int z2 = block_z; z2 < block_z + quad_width; z2++)
                    for (int y2 = y; y2 < y + quad_height; y2++)
                        mask[y2 + z2 * WORLD_HEIGHT_LIMIT].block = BlockID::air;

                // Push quad
                quads.emplace_back(
                    base_entry.block,
                    glm::ivec3{block_x, y, block_z},
                    glm::vec3{0, 0, quad_width},
                    glm::vec3{0, quad_height, 0},
                    base_entry.back_face
                );

                // Advance y past this quad
                y += quad_height;
            }
        }
    }

    //
    // Z axis
    //

    // For each XY plane
    for (int block_z = 0; block_z < CHUNK_SIZE; block_z++)
    {
        mask.clear();

        // Build mask
        for (int block_x = 0; block_x < CHUNK_SIZE; block_x++)
        {
            for (int block_y = 0; block_y < WORLD_HEIGHT_LIMIT; block_y++)
            {
                glm::ivec3 back_block_coords;
                BlockID back_block;

                glm::ivec3 front_block_coords = {block_x, block_y, block_z};
                BlockID front_block = blocks[GetChunkIndex(front_block_coords)];

                if (block_z == 0)
                {
                    back_block_coords = {block_x, block_y, CHUNK_SIZE - 1};
                    back_block = back_neighbor->GetBlocks()[GetChunkIndex(back_block_coords)];
                }
                else
                {
                    back_block_coords = {block_x, block_y, block_z - 1};
                    back_block = blocks[GetChunkIndex(back_block_coords)];
                }

                if (ShouldRenderFace(back_block, front_block) && !ShouldRenderFace(front_block, back_block))
                {
                    uint8_t light = lightmap.GetCombinedLight(front_block_coords);
                    mask.emplace_back(back_block, light, false);
                }
                else if (ShouldRenderFace(front_block, back_block) && !ShouldRenderFace(back_block, front_block))
                {
                    uint8_t light;
                    if (block_z == 0)
                        light = back_neighbor->GetLightmap().GetCombinedLight(back_block_coords);
                    else
                        light = lightmap.GetCombinedLight(back_block_coords);

                    mask.emplace_back(front_block, light, true);
                }
                else
                {
                    mask.emplace_back(BlockID::air, 0, false); // Don't render
                }
            }
        }

        // Use mask to construct quads
        for (int block_x = 0; block_x < CHUNK_SIZE; ++block_x)
        {
            int y = 0;
            while (y < WORLD_HEIGHT_LIMIT)
            {
                MaskEntry base_entry = mask[y + block_x * WORLD_HEIGHT_LIMIT];

                // Skip empty cells
                if (base_entry.block == BlockID::air)
                {
                    y++;
                    continue;
                }

                // Determine quad height (along Y)
                int quad_height = 1;
                while (y + quad_height < WORLD_HEIGHT_LIMIT && mask[(y + quad_height) + block_x * WORLD_HEIGHT_LIMIT] == base_entry)
                    quad_height++;

                // Determine quad width (along X)
                int quad_width = 1;
                for (int x2 = block_x + 1; x2 < CHUNK_SIZE; x2++)
                {
                    bool column_matches = true;
                    for (int y2 = y; y2 < y + quad_height; y2++)
                    {
                        if (mask[y2 + x2 * WORLD_HEIGHT_LIMIT] != base_entry)
                        {
                            column_matches = false;
                            break;
                        }
                    }

                    if (!column_matches)
                        break;

                    quad_width++;
                }

                // Mark mask entries covered by this quad as "handled"
                for (int x2 = block_x; x2 < block_x + quad_width; x2++)
                    for (int y2 = y; y2 < y + quad_height; y2++)
                        mask[y2 + x2 * WORLD_HEIGHT_LIMIT].block = BlockID::air;

                // Push quad
                quads.emplace_back(
                    base_entry.block,
                    glm::ivec3{block_x, y, block_z},
                    glm::vec3{quad_width, 0, 0},
                    glm::vec3{0, quad_height, 0},
                    !base_entry.back_face
                );

                // Advance y past this quad
                y += quad_height;
            }
        }
    }

    //
    // Y axis
    //

    // For each XZ plane
    for (int block_y = 0; block_y < WORLD_HEIGHT_LIMIT; block_y++)
    {
        mask.clear();

        // Build mask
        for (int block_x = 0; block_x < CHUNK_SIZE; block_x++)
        {
            for (int block_z = 0; block_z < CHUNK_SIZE; block_z++)
            {
                if (block_y < WORLD_HEIGHT_LIMIT - 1) // Skip rendering top of chunk
                {
                    BlockID bottom_block = blocks[GetChunkIndex(block_x, block_y, block_z)];  // These blocks are on opposite sides of the plane
                    BlockID top_block = blocks[GetChunkIndex(block_x, block_y + 1, block_z)]; //

                    if (ShouldRenderFace(bottom_block, top_block) && !ShouldRenderFace(top_block, bottom_block))
                    {
                        uint8_t light = lightmap.GetCombinedLight(block_x, block_y + 1, block_z);
                        mask.emplace_back(bottom_block, light, false);
                    }
                    else if (ShouldRenderFace(top_block, bottom_block) && !ShouldRenderFace(bottom_block, top_block))
                    {
                        uint8_t light = lightmap.GetCombinedLight(block_x, block_y, block_z);
                        mask.emplace_back(top_block, light, true);
                    }
                    else
                    {
                        mask.emplace_back(BlockID::air, 0, false); // Don't render
                    }
                }
                else
                {
                    mask.emplace_back(BlockID::air, 0, false);
                }
            }
        }

        // Use mask to construct quads
        for (int block_x = 0; block_x < CHUNK_SIZE; block_x++)
        {
            int z = 0;
            while (z < CHUNK_SIZE)
            {
                MaskEntry base_entry = mask[z + block_x * CHUNK_SIZE];

                // Skip empty cells
                if (base_entry.block == BlockID::air)
                {
                    z++;
                    continue;
                }

                // Determine quad height (along Z)
                int quad_height = 1;
                while (z + quad_height < CHUNK_SIZE && mask[(z + quad_height) + block_x * CHUNK_SIZE] == base_entry)
                    quad_height++;

                // Determine quad width (along X)
                int quad_width = 1;
                for (int x2 = block_x + 1; x2 < CHUNK_SIZE; x2++)
                {
                    bool column_matches = true;
                    for (int z2 = z; z2 < z + quad_height; z2++)
                    {
                        if (mask[z2 + x2 * CHUNK_SIZE] != base_entry)
                        {
                            column_matches = false;
                            break;
                        }
                    }

                    if (!column_matches)
                        break;

                    quad_width++;
                }

                // Mark mask entries covered by this quad as "handled"
                for (int x2 = block_x; x2 < block_x + quad_width; x2++)
                    for (int z2 = z; z2 < z + quad_height; z2++)
                        mask[z2 + x2 * CHUNK_SIZE].block = BlockID::air;

                // Push quad
                quads.emplace_back(
                    base_entry.block,
                    glm::ivec3{block_x, block_y, z},
                    glm::vec3{quad_width, 0, 0},
                    glm::vec3{0, 0, quad_height},
                    base_entry.back_face
                );

                // Advance z past this quad
                z += quad_height;
            }
        }
    }

    return quads;
}
