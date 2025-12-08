#ifndef MESHER_H
#define MESHER_H

#include <iostream>
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

#include "constants.h"
#include "block.h"
#include "helpers.h"
#include "chunk.h"

struct BlockQuad
{
    BlockID block;
    glm::vec3 base; // Local coordinates
    glm::vec3 du;
    glm::vec3 dv;
    bool back_face;
};

/*
    The algorithm works as follows.
    
    For each axis, we consider all planes perpendicular to that axis slicing through the block faces (if N is the length of the chunk along the axis, there
    are N+1 such planes). It should be clear that each plane represents a set of block faces that could be rendered. For each of these planes, we first construct
    a mask that indicates which block face, if any, should be rendered at each position, as well as whether it's a front or back face (encoded by the sign).

    Then, for each column of this mask, we move up to the first renderable face. If none is found, we move on to the next column. If one is found, we begin with
    a 1x1 quad, and extend its height until a different mask value is reached. We then extend this new quad's width as far as possible to get the final quad.
    Finally, all mask positions covered by the final quad are marked as non-renderable, and we continue iterating through this column.
*/
std::vector<BlockQuad> GreedyMesh(uint16_t *blocks, std::vector<Chunk *> neighbor_chunks)
{
    std::vector<BlockQuad> quads;

    //
    // X axis
    //

    // For each ZY plane
    for (int block_x = -1; block_x < CHUNK_SIZE; block_x++)
    {
        std::vector<int32_t> mask(CHUNK_SIZE * WORLD_HEIGHT_LIMIT);

        // Build mask
        for (int block_z = 0; block_z < CHUNK_SIZE; block_z++)
        {
            for (int block_y = 0; block_y < WORLD_HEIGHT_LIMIT; block_y++)
            {
                uint16_t current_block, next_block;
                if (block_x == -1) // Check left neighbor chunk
                {
                    current_block = blocks[GetChunkIndex(0, block_y, block_z)];
                    next_block = neighbor_chunks[3]->GetBlocks()[GetChunkIndex(CHUNK_SIZE - 1, block_y, block_z)];
                }
                else if (block_x == CHUNK_SIZE - 1) // Check right neighbor chunk
                {
                    current_block = blocks[GetChunkIndex(CHUNK_SIZE - 1, block_y, block_z)];
                    next_block = neighbor_chunks[1]->GetBlocks()[GetChunkIndex(0, block_y, block_z)];
                }
                else
                {
                    current_block = blocks[GetChunkIndex(block_x, block_y, block_z)];
                    next_block = blocks[GetChunkIndex(block_x + 1, block_y, block_z)];
                }

                int mask_index = block_y + block_z * WORLD_HEIGHT_LIMIT;
                if (ShouldRenderFace((BlockID)current_block, (BlockID)next_block) && !ShouldRenderFace((BlockID)next_block, (BlockID)current_block))
                    mask[mask_index] = current_block;
                else if (ShouldRenderFace((BlockID)next_block, (BlockID)current_block) && !ShouldRenderFace((BlockID)current_block, (BlockID)next_block))
                    mask[mask_index] = -next_block; // - sign indicates a back face
                else
                    mask[mask_index] = 0; // Don't render
            }
        }

        // Use mask to construct quads
        for (int block_z = 0; block_z < CHUNK_SIZE; block_z++)
        {
            int y = 0;
            while (y < WORLD_HEIGHT_LIMIT)
            {
                int32_t value = mask[y + block_z * WORLD_HEIGHT_LIMIT];

                // Skip empty faces
                if (value == 0)
                {
                    y++;
                    continue;
                }

                // Determine quad height (along Y)
                int quad_height = 1;
                while (y + quad_height < WORLD_HEIGHT_LIMIT && mask[(y + quad_height) + block_z * WORLD_HEIGHT_LIMIT] == value)
                    quad_height++;

                // Determine quad width (along Z)
                int quad_width = 1;
                for (int z2 = block_z + 1; z2 < CHUNK_SIZE; z2++)
                {
                    bool column_matches = true;
                    for (int y2 = y; y2 < y + quad_height; y2++)
                    {
                        if (mask[y2 + z2 * WORLD_HEIGHT_LIMIT] != value)
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
                        mask[y2 + z2 * WORLD_HEIGHT_LIMIT] = 0;

                bool back_face = value < 0;
                uint16_t block = (uint16_t)(back_face ? -value : value);

                // Push quad
                quads.push_back({
                    (BlockID)block,
                    {block_x, y, block_z},
                    {0, 0, quad_width},
                    {0, quad_height, 0},
                    back_face
                });

                // Advance y past this quad
                y += quad_height;
            }
        }
    }


    //
    // Z axis
    //

    // For each XY plane
    for (int block_z = -1; block_z < CHUNK_SIZE; block_z++)
    {
        std::vector<int32_t> mask(CHUNK_SIZE * WORLD_HEIGHT_LIMIT);

        // Build mask
        for (int block_x = 0; block_x < CHUNK_SIZE; block_x++)
        {
            for (int block_y = 0; block_y < WORLD_HEIGHT_LIMIT; block_y++)
            {
                uint16_t current_block, next_block;
                if (block_z == -1) // Check back neighbor chunk
                {
                    current_block = blocks[GetChunkIndex(block_x, block_y, 0)];
                    next_block = neighbor_chunks[2]->GetBlocks()[GetChunkIndex(block_x, block_y, CHUNK_SIZE - 1)];
                }
                else if (block_z == CHUNK_SIZE - 1) // Check front neighbor chunk
                {
                    current_block = blocks[GetChunkIndex(block_x, block_y, CHUNK_SIZE  -1)];
                    next_block = neighbor_chunks[0]->GetBlocks()[GetChunkIndex(block_x, block_y, 0)];
                }
                else
                {
                    current_block = blocks[GetChunkIndex(block_x, block_y, block_z)];
                    next_block = blocks[GetChunkIndex(block_x, block_y, block_z + 1)];
                }

                int mask_index = block_y + block_x * WORLD_HEIGHT_LIMIT;
                if (ShouldRenderFace((BlockID)current_block, (BlockID)next_block) && !ShouldRenderFace((BlockID)next_block, (BlockID)current_block))
                    mask[mask_index] = current_block;
                else if (ShouldRenderFace((BlockID)next_block, (BlockID)current_block) && !ShouldRenderFace((BlockID)current_block, (BlockID)next_block))
                    mask[mask_index] = -next_block; // - sign indicates a back face
                else
                    mask[mask_index] = 0; // Don't render
            }
        }

        // Use mask to construct quads
        for (int block_x = 0; block_x < CHUNK_SIZE; ++block_x)
        {
            int y = 0;
            while (y < WORLD_HEIGHT_LIMIT)
            {
                int32_t value = mask[y + block_x * WORLD_HEIGHT_LIMIT];

                // Skip empty cells
                if (value == 0)
                {
                    y++;
                    continue;
                }

                // Determine quad height (along Y)
                int quad_height = 1;
                while (y + quad_height < WORLD_HEIGHT_LIMIT && mask[(y + quad_height) + block_x * WORLD_HEIGHT_LIMIT] == value)
                    quad_height++;

                // Determine quad width (along X)
                int quad_width = 1;
                for (int x2 = block_x + 1; x2 < CHUNK_SIZE; x2++)
                {
                    bool column_matches = true;
                    for (int y2 = y; y2 < y + quad_height; y2++)
                    {
                        if (mask[y2 + x2 * WORLD_HEIGHT_LIMIT] != value)
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
                        mask[y2 + x2 * WORLD_HEIGHT_LIMIT] = 0;

                bool back_face = value < 0;
                uint16_t block = (uint16_t)(back_face ? -value : value);

                // Push quad
                quads.push_back({
                    (BlockID)block,
                    {block_x, y, block_z},
                    {quad_width, 0, 0},
                    {0, quad_height, 0},
                    back_face
                });

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
        std::vector<int32_t> mask(CHUNK_SIZE * CHUNK_SIZE);

        // Build mask
        for (int block_x = 0; block_x < CHUNK_SIZE; block_x++)
        {
            for (int block_z = 0; block_z < CHUNK_SIZE; block_z++)
            {
                int mask_index = block_z + block_x * CHUNK_SIZE;
                if (block_y < WORLD_HEIGHT_LIMIT - 1) // Skip rendering top of chunk
                {
                    uint16_t current_block = blocks[GetChunkIndex(block_x, block_y, block_z)];  // These blocks are on opposite sides of the plane
                    uint16_t next_block = blocks[GetChunkIndex(block_x, block_y + 1, block_z)]; //

                    if (ShouldRenderFace((BlockID)current_block, (BlockID)next_block) && !ShouldRenderFace((BlockID)next_block, (BlockID)current_block))
                        mask[mask_index] = current_block;
                    else if (ShouldRenderFace((BlockID)next_block, (BlockID)current_block) && !ShouldRenderFace((BlockID)current_block, (BlockID)next_block))
                        mask[mask_index] = -next_block; // - sign indicates a back face
                    else
                        mask[mask_index] = 0; // Don't render

                }
                else
                {
                    mask[mask_index] = 0;
                }
            }
        }

        // Use mask to construct quads
        for (int block_x = 0; block_x < CHUNK_SIZE; block_x++)
        {
            int z = 0;
            while (z < CHUNK_SIZE)
            {
                int32_t value = mask[z + block_x * CHUNK_SIZE];

                // Skip empty cells
                if (value == 0)
                {
                    z++;
                    continue;
                }

                // Determine quad height (along Z)
                int quad_height = 1;
                while (z + quad_height < CHUNK_SIZE && mask[(z + quad_height) + block_x * CHUNK_SIZE] == value)
                    quad_height++;

                // Determine quad width (along X)
                int quad_width = 1;
                for (int x2 = block_x + 1; x2 < CHUNK_SIZE; x2++)
                {
                    bool column_matches = true;
                    for (int z2 = z; z2 < z + quad_height; z2++)
                    {
                        if (mask[z2 + x2 * CHUNK_SIZE] != value)
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
                        mask[z2 + x2 * CHUNK_SIZE] = 0;

                bool back_face = value < 0;
                uint16_t block = (uint16_t)(back_face ? -value : value);

                // Push quad
                quads.push_back({
                    (BlockID)block,
                    {block_x, block_y, z},
                    {quad_width, 0, 0},
                    {0, 0, quad_height},
                    back_face
                });

                // Advance z past this quad
                z += quad_height;
            }
        }
    }

    return quads;
}

#endif
