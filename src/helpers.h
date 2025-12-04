#ifndef HELPERS_H
#define HELPERS_H

#include "block.h"

int GetChunkIndex(int x, int y, int z);
bool BlockShouldBeRendered(uint16_t *chunk, BlockID block, int local_block_x, int local_block_y, int local_block_z);
bool BlockIsOpaque(BlockID block);
bool ShouldRenderFace(BlockID face_block, BlockID face_neighbor);
glm::vec3 GetLocalBlockPos(glm::vec3 global_block_pos);
uint64_t CombineChunkCoordinates(int chunk_x, int chunk_z);
glm::vec2 DecombineChunkCoordinates(uint64_t combined);

#endif
