#pragma once

#include "block.h"

struct Plane
{
    glm::vec3 normal;
    float d;
};

int GetChunkIndex(int x, int y, int z) noexcept;
bool BlockIsOpaque(BlockID block) noexcept;
bool ShouldRenderFace(BlockID face, BlockID neighbor_face) noexcept;
uint64_t ChunkCoordsToID(const glm::ivec3 &chunk_coords) noexcept;
glm::ivec3 ChunkIDToCoords(uint64_t id) noexcept;
glm::ivec3 GetNearestVoxel(const glm::vec3 &global_pos) noexcept;
glm::ivec3 VoxelToChunk(const glm::ivec3 &voxel_pos) noexcept;
glm::ivec3 GlobalToLocalVoxel(const glm::ivec3 &voxel_pos) noexcept;
glm::ivec3 LocalToGlobalVoxel(const glm::ivec3 &voxel_pos, const glm::ivec3 &chunk_coord) noexcept;
bool ChunkInFrustum(const Plane frustum[6], const glm::vec3 &chunk_min, const glm::vec3 &chunk_max);
void GetFrustumPlanes(const glm::mat4 &view_proj, Plane *frustum);
uint64_t SplitMix64(uint64_t& x);
int RandomRange(int min_inclusive, int max_exclusive);
