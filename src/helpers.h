#pragma once

#include <filesystem>

#include "block.h"

struct Plane
{
    glm::vec3 normal;
    float d;
};

uint64_t ChunkCoordsToID(const glm::ivec3 &chunk_coords) noexcept;
glm::ivec3 ChunkIDToCoords(uint64_t id) noexcept;
glm::ivec3 GetNearestVoxel(const glm::vec3 &global_pos) noexcept;
glm::ivec3 VoxelToChunk(const glm::ivec3 &voxel_pos) noexcept;
glm::ivec3 GlobalToLocalVoxel(const glm::ivec3 &voxel_pos) noexcept;
glm::ivec3 LocalToGlobalVoxel(const glm::ivec3 &voxel_pos, const glm::ivec3 &chunk_coord) noexcept;
bool ChunkInFrustum(const Plane frustum[6], const glm::vec3 &chunk_min, const glm::vec3 &chunk_max);
void GetFrustumPlanes(const glm::mat4 &view_proj, Plane *frustum);
uint64_t SplitMix64(uint64_t& x);
void LoadChunkFromDisk(std::filesystem::path chunk_file_path, BlockID *blocks);
void WriteChunkToDisk(std::filesystem::path chunk_file_path, BlockID *blocks);

inline int GetChunkIndex(int x, int y, int z) noexcept
{
    // This formula is tied to the loop order when chunks are generated (see chunk_gen.h)
    return y + (WORLD_HEIGHT_LIMIT * z) + (WORLD_HEIGHT_LIMIT * CHUNK_SIZE * x);
}

inline int GetChunkIndex(const glm::ivec3 &block_coords) noexcept
{
    return block_coords.y + (WORLD_HEIGHT_LIMIT * block_coords.z) + (WORLD_HEIGHT_LIMIT * CHUNK_SIZE * block_coords.x);
}

inline bool BlockIsOpaque(BlockID block) noexcept
{
    //return !(block == BlockID::air || block == BlockID::water || block == BlockID::sulphur_crystal || block == BlockID::boron_crystal || block == BlockID::blue_crystal || block == BlockID::glass);
    return block > BlockID::glass; // The opaque blocks are after glass
}

inline bool BlockIsTransparent(BlockID block) noexcept
{
    return block < BlockID::aluminum; // The transparent blocks are before aluminum
}

inline bool IsBorderBlock(const glm::ivec3 &block_coords) noexcept
{
    return block_coords.x == 0 || block_coords.x == CHUNK_SIZE - 1 || block_coords.z == 0 || block_coords.z == CHUNK_SIZE - 1;
}

inline bool BlockIsInChunk(int x, int y, int z) noexcept
{
    return (unsigned)x < CHUNK_SIZE
        && (unsigned)z < CHUNK_SIZE
        && (unsigned)y < WORLD_HEIGHT_LIMIT;
}

inline bool BlockIsInChunk(const glm::ivec3 &block_coords) noexcept
{
    return (unsigned)block_coords.x < CHUNK_SIZE
        && (unsigned)block_coords.z < CHUNK_SIZE
        && (unsigned)block_coords.y < WORLD_HEIGHT_LIMIT;
}

inline bool ShouldRenderFace(BlockID face, BlockID neighbor_face) noexcept
{
    return !(face == BlockID::air || face == neighbor_face || BlockIsOpaque(neighbor_face));
}
