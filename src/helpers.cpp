
#include "helpers.h"
#include "constants.h"
#include "block.h"

int GetChunkIndex(int x, int y, int z) noexcept
{
    // This formula is tied to the loop order when chunks are generated (see chunk_gen.h)
    return y + (WORLD_HEIGHT_LIMIT * z) + (WORLD_HEIGHT_LIMIT * (CHUNK_SIZE + 2) * x);
}

bool BlockIsOpaque(BlockID block) noexcept
{
    return !(block == BlockID::air || block == BlockID::water || block == BlockID::sulphur_crystal || block == BlockID::boron_crystal || block == BlockID::blue_crystal || block == BlockID::glass);
}

bool ShouldRenderFace(BlockID face, BlockID neighbor_face) noexcept
{
    return !(face == BlockID::air || face == neighbor_face || BlockIsOpaque(neighbor_face));
}

uint64_t ChunkCoordsToID(const glm::ivec3& chunk_coords) noexcept
{
    uint64_t combined = (uint64_t)((uint32_t)chunk_coords.x);
    combined <<= (sizeof(uint64_t) * 8 / 2);
    combined |= (uint64_t)((uint32_t)chunk_coords.z);
    return combined;
}

glm::ivec3 ChunkIDToCoords(uint64_t id) noexcept
{
    int chunk_z = (int)((uint32_t)id);
    id >>= (sizeof(uint64_t) * 8 / 2);
    int chunk_x = (int)((uint32_t)id);
    return glm::ivec3(chunk_x, 0, chunk_z);
}

// Convert arbitrary global position to nearest voxel position
glm::ivec3 GetNearestVoxel(const glm::vec3& global_pos) noexcept
{
    return glm::ivec3(
        glm::round(global_pos.x),
        glm::round(global_pos.y),
        glm::round(global_pos.z)
    );
}

// Get chunk coordinate the voxel position belongs to
glm::ivec3 VoxelToChunk(const glm::ivec3& voxel_pos) noexcept
{
    return glm::ivec3(
        glm::floor((float)voxel_pos.x / (float)CHUNK_SIZE),
        0,
        glm::floor((float)voxel_pos.z / (float)CHUNK_SIZE)
    );
}

// Convert global voxel position to local (in chunk) voxel position
glm::ivec3 GlobalToLocalVoxel(const glm::ivec3& global_voxel_pos) noexcept
{
    glm::ivec3 chunk_coord = VoxelToChunk(global_voxel_pos);
    return glm::ivec3(
        (global_voxel_pos.x - chunk_coord.x * CHUNK_SIZE) + 1,
        global_voxel_pos.y,
        (global_voxel_pos.z - chunk_coord.z * CHUNK_SIZE) + 1
    );
}

// Convert local (in chunk) voxel position to global voxel position
glm::ivec3 LocalToGlobalVoxel(const glm::ivec3& local_voxel_pos, const glm::ivec3& chunk_coord) noexcept
{
    return glm::ivec3(
        (local_voxel_pos.x - 1) + chunk_coord.x * CHUNK_SIZE,
        local_voxel_pos.y,
        (local_voxel_pos.z - 1) + chunk_coord.z * CHUNK_SIZE
    );
}

bool ChunkInFrustum(const Plane frustum[6], const glm::vec3& chunk_min, const glm::vec3& chunk_max)
{
    for (int i = 0; i < 6; i++)
    {
        const Plane& p = frustum[i];

        // Compute the most positive point relative to plane normal
        glm::vec3 positive = {
            (p.normal.x >= 0 ? chunk_max.x : chunk_min.x),
            (p.normal.y >= 0 ? chunk_max.y : chunk_min.y),
            (p.normal.z >= 0 ? chunk_max.z : chunk_min.z),
        };

        // If that point is behind the plane, the chunk is outside
        if (glm::dot(p.normal, positive) + p.d < 0)
            return false;
    }

    return true;
}
