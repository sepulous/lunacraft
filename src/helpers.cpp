
#include "helpers.h"
#include "constants.h"
#include "block.h"

// This is tied to how the chunks are generated (see chunk_gen.h)
int GetChunkIndex(int x, int y, int z)
{
    return y + (WORLD_HEIGHT_LIMIT * z) + (WORLD_HEIGHT_LIMIT * CHUNK_SIZE * x);
}

bool BlockShouldBeRendered(uint16_t *chunk, BlockID block, int localBlockX, int localBlockY, int localBlockZ)
{
    uint16_t idCheck = (block == BlockID::water) ? 38 : 37;

    // Top check
    if (localBlockY < WORLD_HEIGHT_LIMIT - 1 && chunk[GetChunkIndex(localBlockX, localBlockY + 1, localBlockZ)] > idCheck)
        return true;

    // Bottom check
    if (localBlockY > 0 && chunk[GetChunkIndex(localBlockX, localBlockY - 1, localBlockZ)] > idCheck)
        return true;

    // Front and back checks
    bool backTest = chunk[GetChunkIndex(localBlockX, localBlockY, localBlockZ - 1)] > idCheck;
    bool frontTest = chunk[GetChunkIndex(localBlockX, localBlockY, localBlockZ + 1)] > idCheck;

    if (frontTest || backTest)
        return true;

    // Left and right checks
    bool leftTest = chunk[GetChunkIndex(localBlockX - 1, localBlockY, localBlockZ)] > idCheck;
    bool rightTest = chunk[GetChunkIndex(localBlockX + 1, localBlockY, localBlockZ)] > idCheck;

    return leftTest || rightTest;
}

bool BlockIsOpaque(BlockID block)
{
    return !(block == BlockID::air || block == BlockID::water || block == BlockID::sulphur_crystal || block == BlockID::boron_crystal || block == BlockID::blue_crystal || block == BlockID::glass);
}

bool ShouldRenderFace(BlockID face_block, BlockID face_neighbor)
{
    // TODO: Add minilights (after figuring out how to just have one minilight block)
    return face_block != BlockID::air && !BlockIsOpaque(face_neighbor) && face_block != face_neighbor;
}

glm::vec3 GetLocalBlockPos(glm::vec3 globalBlockPos)
{
    int chunkX = glm::abs(glm::floor(globalBlockPos.x / CHUNK_SIZE));
    int chunkZ = glm::abs(glm::floor(globalBlockPos.z / CHUNK_SIZE));
    int globalBlockPosX = (int)globalBlockPos.x;
    int globalBlockPosZ = (int)globalBlockPos.z;
    int localBlockPosX;
    int localBlockPosZ;

    if (globalBlockPosX >= 0 || ((-globalBlockPosX) % CHUNK_SIZE == 0))
        localBlockPosX = globalBlockPosX % CHUNK_SIZE;
    else
        localBlockPosX = globalBlockPosX + chunkX*CHUNK_SIZE;

    if (globalBlockPosZ >= 0 || ((-globalBlockPosZ) % CHUNK_SIZE == 0))
        localBlockPosZ = globalBlockPosZ % CHUNK_SIZE;
    else
        localBlockPosZ = globalBlockPosZ + chunkZ*CHUNK_SIZE;

    return glm::vec3(localBlockPosX, globalBlockPos.y, localBlockPosZ);
}

uint64_t CombineChunkCoordinates(int chunk_x, int chunk_z)
{
    uint64_t combined = (uint64_t)((uint32_t)chunk_x);
    combined <<= (sizeof(uint64_t) * 8 / 2);
    combined |= (uint64_t)((uint32_t)chunk_z);
    return combined;
}

glm::vec2 DecombineChunkCoordinates(uint64_t combined)
{
    int chunk_z = (int)((uint32_t)combined);
    combined >>= (sizeof(uint64_t) * 8 / 2);
    int chunk_x = (int)((uint32_t)combined);
    return glm::vec2(chunk_x, chunk_z);
}
