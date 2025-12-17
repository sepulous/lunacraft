#ifndef HELPERS_H
#define HELPERS_H

#include "block.h"

struct Plane
{
    glm::vec3 normal;
    float d;
};

int GetChunkIndex(int x, int y, int z);
bool BlockIsOpaque(BlockID block);
bool ShouldRenderFace(BlockID face, BlockID neighbor_face);
uint64_t ChunkCoordsToID(glm::ivec3 chunk_coords);
glm::ivec3 ChunkIDToCoords(uint64_t id);
glm::ivec3 GetNearestVoxel(glm::vec3 global_pos);
glm::ivec3 VoxelToChunk(glm::ivec3 voxel_pos);
glm::ivec3 GlobalToLocalVoxel(glm::ivec3 voxel_pos);
glm::ivec3 LocalToGlobalVoxel(glm::ivec3 voxel_pos, glm::ivec3 chunk_coord);
bool ChunkInFrustum(const Plane frustum[6], const glm::vec3& chunk_min, const glm::vec3& chunk_max);

#endif
