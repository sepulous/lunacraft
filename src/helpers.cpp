
#include "helpers.h"
#include "constants.h"
#include "block.h"

int GetChunkIndex(int x, int y, int z)
{
    // This formula is tied to the loop order when chunks are generated (see chunk_gen.h)
    return y + (WORLD_HEIGHT_LIMIT * z) + (WORLD_HEIGHT_LIMIT * CHUNK_SIZE * x);
}

bool BlockIsOpaque(BlockID block)
{
    return !(block == BlockID::air || block == BlockID::water || block == BlockID::sulphur_crystal || block == BlockID::boron_crystal || block == BlockID::blue_crystal || block == BlockID::glass);
}

bool ShouldRenderFace(BlockID face, BlockID neighbor_face)
{
    // TODO: Add minilights (after figuring out how to just have one minilight block)
    return !(face == BlockID::air || face == neighbor_face || BlockIsOpaque(neighbor_face));
}

uint64_t ChunkCoordsToID(glm::ivec3 chunk_coords)
{
    uint64_t combined = (uint64_t)((uint32_t)chunk_coords.x);
    combined <<= (sizeof(uint64_t) * 8 / 2);
    combined |= (uint64_t)((uint32_t)chunk_coords.z);
    return combined;
}

glm::ivec3 ChunkIDToCoords(uint64_t id)
{
    int chunk_z = (int)((uint32_t)id);
    id >>= (sizeof(uint64_t) * 8 / 2);
    int chunk_x = (int)((uint32_t)id);
    return glm::ivec3(chunk_x, 0, chunk_z);
}

// Convert arbitrary global position to nearest voxel position
glm::ivec3 GetNearestVoxel(glm::vec3 global_pos)
{
    return glm::ivec3(
        glm::round(global_pos.x),
        glm::round(global_pos.y),
        glm::round(global_pos.z)
    );
}

// Get chunk coordinate the voxel position belongs to
glm::ivec3 VoxelToChunk(glm::ivec3 voxel_pos)
{
    return glm::ivec3(
        glm::floor((float)voxel_pos.x / (float)CHUNK_SIZE),
        0,
        glm::floor((float)voxel_pos.z / (float)CHUNK_SIZE)
    );
}

// Convert global voxel position to local (in chunk) voxel position
glm::ivec3 GlobalToLocalVoxel(glm::ivec3 voxel_pos)
{
    glm::ivec3 chunk_coord = VoxelToChunk(voxel_pos);
    return glm::ivec3(
        voxel_pos.x - chunk_coord.x * CHUNK_SIZE,
        voxel_pos.y,
        voxel_pos.z - chunk_coord.z * CHUNK_SIZE
    );
}

// Convert local (in chunk) voxel position to global voxel position
glm::ivec3 LocalToGlobalVoxel(glm::ivec3 voxel_pos, glm::ivec3 chunk_coord)
{
    return glm::ivec3(
        voxel_pos.x + chunk_coord.x * CHUNK_SIZE,
        voxel_pos.y,
        voxel_pos.z + chunk_coord.z * CHUNK_SIZE
    );
}
