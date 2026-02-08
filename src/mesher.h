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
    glm::ivec3 base_coords; // Global voxel position
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
std::vector<BlockQuad> GreedyMesh(BlockID *blocks, std::array<Chunk *, 4> neighbors)
{
    struct MaskEntry
    {
        BlockID block;
        glm::ivec3 coords;
        bool back_face;

        MaskEntry() { block = BlockID::air, back_face = false; }
        MaskEntry(BlockID block, glm::ivec3 coords, bool back_face) : block(block), coords(coords), back_face(back_face) {}
        bool operator==(const MaskEntry& other) { return block == other.block && coords == other.coords && back_face == other.back_face; }
        bool operator!=(const MaskEntry& other) { return block != other.block || coords != other.coords || back_face != other.back_face; }
    };

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
    for (int block_x = 0; block_x < CHUNK_SIZE + 1; block_x++)
    {
        std::vector<MaskEntry> mask;
        mask.reserve(CHUNK_SIZE * WORLD_HEIGHT_LIMIT);

        // Build mask
        for (int block_z = 0; block_z < CHUNK_SIZE; block_z++)
        {
            for (int block_y = 0; block_y < WORLD_HEIGHT_LIMIT; block_y++)
            {
                if (block_x == 0)
                {
                    BlockID neighbor_block = (BlockID)left_neighbor->GetBlocks()[GetChunkIndex(CHUNK_SIZE - 1, block_y, block_z)];
                    BlockID current_block = (BlockID)blocks[GetChunkIndex(0, block_y, block_z)];

                    glm::ivec3 neighbor_chunk_pos = left_neighbor->GetCoords();
                    glm::ivec3 current_chunk_pos = {neighbor_chunk_pos.x + 1, 0, neighbor_chunk_pos.z};

                    if (ShouldRenderFace(current_block, neighbor_block) && !ShouldRenderFace(neighbor_block, current_block))
                        mask.emplace_back(current_block, LocalToGlobalVoxel({0, block_y, block_z}, current_chunk_pos), true);
                    else if (ShouldRenderFace(neighbor_block, current_block) && !ShouldRenderFace(current_block, neighbor_block))
                        mask.emplace_back(neighbor_block, LocalToGlobalVoxel({CHUNK_SIZE - 1, block_y, block_z}, neighbor_chunk_pos), false);
                    else
                        mask.emplace_back(BlockID::air, glm::ivec3{0, 0, 0}, false); // Don't render
                }
                else if (block_x == CHUNK_SIZE)
                {
                    BlockID neighbor_block = (BlockID)right_neighbor->GetBlocks()[GetChunkIndex(0, block_y, block_z)];
                    BlockID current_block = (BlockID)blocks[GetChunkIndex(CHUNK_SIZE - 1, block_y, block_z)];

                    glm::ivec3 neighbor_chunk_pos = right_neighbor->GetCoords();
                    glm::ivec3 current_chunk_pos = {neighbor_chunk_pos.x - 1, 0, neighbor_chunk_pos.z};

                    if (ShouldRenderFace(current_block, neighbor_block) && !ShouldRenderFace(neighbor_block, current_block))
                        mask.emplace_back(current_block, LocalToGlobalVoxel({CHUNK_SIZE - 1, block_y, block_z}, current_chunk_pos), true);
                    else if (ShouldRenderFace(neighbor_block, current_block) && !ShouldRenderFace(current_block, neighbor_block))
                        mask.emplace_back(neighbor_block, LocalToGlobalVoxel({0, block_y, block_z}, neighbor_chunk_pos), false);
                    else
                        mask.emplace_back(BlockID::air, glm::ivec3{0, 0, 0}, false); // Don't render
                }
                else
                {
                    BlockID current_block = (BlockID)blocks[GetChunkIndex(block_x - 1, block_y, block_z)];
                    BlockID next_block = (BlockID)blocks[GetChunkIndex(block_x, block_y, block_z)];

                    glm::ivec3 neighbor_chunk_pos = right_neighbor->GetCoords();
                    glm::ivec3 current_chunk_pos = {neighbor_chunk_pos.x - 1, 0, neighbor_chunk_pos.z};

                    if (ShouldRenderFace(current_block, next_block) && !ShouldRenderFace(next_block, current_block))
                        mask.emplace_back(current_block, LocalToGlobalVoxel({block_x - 1, block_y, block_z}, current_chunk_pos), false);
                    else if (ShouldRenderFace(next_block, current_block) && !ShouldRenderFace(current_block, next_block))
                        mask.emplace_back(next_block, LocalToGlobalVoxel({block_x, block_y, block_z}, current_chunk_pos), true);
                    else
                        mask.emplace_back(BlockID::air, glm::ivec3{0, 0, 0}, false); // Don't render
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
                        mask[y2 + z2 * WORLD_HEIGHT_LIMIT] = {BlockID::air, {0, 0, 0}, false};

                // Push quad
                quads.emplace_back(
                    base_entry.block,
                    base_entry.coords,
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
    for (int block_z = 0; block_z < CHUNK_SIZE + 1; block_z++)
    {
        std::vector<MaskEntry> mask;
        mask.reserve(CHUNK_SIZE * WORLD_HEIGHT_LIMIT);

        // Build mask
        for (int block_x = 0; block_x < CHUNK_SIZE; block_x++)
        {
            for (int block_y = 0; block_y < WORLD_HEIGHT_LIMIT; block_y++)
            {
                if (block_z == 0)
                {
                    BlockID neighbor_block = (BlockID)back_neighbor->GetBlocks()[GetChunkIndex(block_x, block_y, CHUNK_SIZE - 1)];
                    BlockID current_block = (BlockID)blocks[GetChunkIndex(block_x, block_y, 0)];

                    glm::ivec3 neighbor_chunk_pos = back_neighbor->GetCoords();
                    glm::ivec3 current_chunk_pos = {neighbor_chunk_pos.x, 0, neighbor_chunk_pos.z + 1};

                    if (ShouldRenderFace(current_block, neighbor_block) && !ShouldRenderFace(neighbor_block, current_block))
                        mask.emplace_back(current_block, LocalToGlobalVoxel({block_x, block_y, 0}, current_chunk_pos), true);
                    else if (ShouldRenderFace(neighbor_block, current_block) && !ShouldRenderFace(current_block, neighbor_block))
                        mask.emplace_back(neighbor_block, LocalToGlobalVoxel({block_x, block_y, CHUNK_SIZE - 1}, neighbor_chunk_pos), false);
                    else
                        mask.emplace_back(BlockID::air, glm::ivec3{0, 0, 0}, false); // Don't render
                }
                else if (block_z == CHUNK_SIZE)
                {
                    BlockID neighbor_block = (BlockID)front_neighbor->GetBlocks()[GetChunkIndex(block_x, block_y, 0)];
                    BlockID current_block = (BlockID)blocks[GetChunkIndex(block_x, block_y, CHUNK_SIZE - 1)];

                    glm::ivec3 neighbor_chunk_pos = front_neighbor->GetCoords();
                    glm::ivec3 current_chunk_pos = {neighbor_chunk_pos.x, 0, neighbor_chunk_pos.z - 1};

                    if (ShouldRenderFace(current_block, neighbor_block) && !ShouldRenderFace(neighbor_block, current_block))
                        mask.emplace_back(current_block, LocalToGlobalVoxel({block_x, block_y, CHUNK_SIZE - 1}, current_chunk_pos), true);
                    else if (ShouldRenderFace(neighbor_block, current_block) && !ShouldRenderFace(current_block, neighbor_block))
                        mask.emplace_back(neighbor_block, LocalToGlobalVoxel({block_x, block_y, 0}, neighbor_chunk_pos), false);
                    else
                        mask.emplace_back(BlockID::air, glm::ivec3{0, 0, 0}, false); // Don't render
                }
                else
                {
                    BlockID current_block = (BlockID)blocks[GetChunkIndex(block_x, block_y, block_z - 1)];
                    BlockID next_block = (BlockID)blocks[GetChunkIndex(block_x, block_y, block_z)];

                    glm::ivec3 neighbor_chunk_pos = right_neighbor->GetCoords();
                    glm::ivec3 current_chunk_pos = {neighbor_chunk_pos.x - 1, 0, neighbor_chunk_pos.z};

                    if (ShouldRenderFace(current_block, next_block) && !ShouldRenderFace(next_block, current_block))
                        mask.emplace_back(current_block, LocalToGlobalVoxel({block_x, block_y, block_z - 1}, current_chunk_pos), false);
                    else if (ShouldRenderFace(next_block, current_block) && !ShouldRenderFace(current_block, next_block))
                        mask.emplace_back(next_block, LocalToGlobalVoxel({block_x, block_y, block_z}, current_chunk_pos), true);
                    else
                        mask.emplace_back(BlockID::air, glm::ivec3{0, 0, 0}, false); // Don't render
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
                        mask[y2 + x2 * WORLD_HEIGHT_LIMIT] = {BlockID::air, {0, 0, 0}, false};

                // Push quad
                quads.emplace_back(
                    base_entry.block,
                    base_entry.coords,
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
        std::vector<MaskEntry> mask;
        mask.reserve(CHUNK_SIZE * CHUNK_SIZE);

        // Build mask
        for (int block_x = 0; block_x < CHUNK_SIZE; block_x++)
        {
            for (int block_z = 0; block_z < CHUNK_SIZE; block_z++)
            {
                if (block_y < WORLD_HEIGHT_LIMIT - 1) // Skip rendering top of chunk
                {
                    BlockID current_block = (BlockID)blocks[GetChunkIndex(block_x, block_y, block_z)];  // These blocks are on opposite sides of the plane
                    BlockID next_block = (BlockID)blocks[GetChunkIndex(block_x, block_y + 1, block_z)]; //

                    glm::ivec3 neighbor_chunk_pos = right_neighbor->GetCoords();
                    glm::ivec3 current_chunk_pos = {neighbor_chunk_pos.x - 1, 0, neighbor_chunk_pos.z};

                    if (ShouldRenderFace(current_block, next_block) && !ShouldRenderFace(next_block, current_block))
                        mask.emplace_back(current_block, LocalToGlobalVoxel({block_x, block_y, block_z}, current_chunk_pos), false);
                    else if (ShouldRenderFace(next_block, current_block) && !ShouldRenderFace(current_block, next_block))
                        mask.emplace_back(next_block, LocalToGlobalVoxel({block_x, block_y + 1, block_z}, current_chunk_pos), true);
                    else
                        mask.emplace_back(BlockID::air, glm::ivec3{0, 0, 0}, false); // Don't render
                }
                else
                {
                    mask.emplace_back(BlockID::air, glm::ivec3{0, 0, 0}, false);
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
                        mask[z2 + x2 * CHUNK_SIZE] = {BlockID::air, {0, 0, 0}, false};

                // Push quad
                quads.emplace_back(
                    base_entry.block,
                    base_entry.coords,
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
