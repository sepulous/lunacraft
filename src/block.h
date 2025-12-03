#ifndef BLOCKS_H
#define BLOCKS_H

#include <unordered_map>
#include <cstdint>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "constants.h"

enum class BlockID : uint16_t
{
    aluminum,
    aluminum_ore,
    amethyst_ore,
    beacon,
    beryllium,
    calcite,
    carbon,
    chalchanthite,
    dirt,
    feldspar,
    gold_ore,
    granite,
    graphite,
    gravel,
    light,
    magnetite,
    molybdenum_ore,
    moon_bark,
    moon_leaf,
    moon_wood,
    neptunium,
    notchium,
    notchium_ore,
    phosphate,
    polymer,
    quartz_ore,
    rock,
    sand,
    shale_gravel,
    silver_ore,
    snow,
    sulphur_ore,
    titanium,
    titanium_ore,
    topsoil,
    unknown,
    xenostone,
    zircon_ore,
    water,
    air,
    sulphur_crystal,
    blue_crystal,
    boron_crystal,
    glass,
    minilight_pz,
    minilight_nz,
    minilight_px,
    minilight_nx,
    minilight_py,
    minilight_ny
};

struct BlockVertex
{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec2 tile_origin;
    glm::vec3 face_normal;
    BlockVertex(glm::vec3 position, glm::vec2 uv, glm::vec2 tile_origin, glm::vec3 face_normal) : position(position), uv(uv), tile_origin(tile_origin), face_normal(face_normal) {}
};

std::unordered_map<BlockID, glm::vec3> ATLAS_TILE_MAP = {
    {BlockID::aluminum,        glm::vec3(32, 32, 32)},
    {BlockID::aluminum_ore,    glm::vec3(17, 17, 17)},
    {BlockID::amethyst_ore,    glm::vec3(168, 168, 168)},
    {BlockID::beacon,          glm::vec3(89, 90, 104)},
    {BlockID::beryllium,       glm::vec3(140, 140, 140)},
    {BlockID::calcite,         glm::vec3(112, 112, 112)},
    {BlockID::carbon,          glm::vec3(73, 73, 73)},
    {BlockID::chalchanthite,   glm::vec3(182, 182, 182)},
    {BlockID::dirt,            glm::vec3(1, 1, 1)},
    {BlockID::feldspar,        glm::vec3(154, 154, 154)},
    {BlockID::gold_ore,        glm::vec3(45, 45, 45)},
    {BlockID::granite,         glm::vec3(155, 155, 155)},
    {BlockID::graphite,        glm::vec3(126, 126, 126)},
    {BlockID::gravel,          glm::vec3(5, 5, 5)},
    {BlockID::light,           glm::vec3(9, 9, 9)}, // NOTE: There are four variants in the atlas
    {BlockID::magnetite,       glm::vec3(18, 18, 18)},
    {BlockID::molybdenum_ore,  glm::vec3(31, 31, 31)},
    {BlockID::moon_bark,       glm::vec3(118, 118, 118)}, // NOTE: Might actually be different on top and bottom
    {BlockID::moon_leaf,       glm::vec3(6, 6, 6)},
    {BlockID::moon_wood,       glm::vec3(33, 19, 33)},
    {BlockID::neptunium,       glm::vec3(86, 86, 86)},
    {BlockID::notchium,        glm::vec3(44, 44, 44)},
    {BlockID::notchium_ore,    glm::vec3(74, 74, 74)},
    {BlockID::phosphate,       glm::vec3(113, 113, 113)},
    {BlockID::polymer,         glm::vec3(30, 30, 30)},
    {BlockID::quartz_ore,      glm::vec3(169, 169, 169)},
    {BlockID::rock,            glm::vec3(3, 3, 3)},
    {BlockID::sand,            glm::vec3(2, 2, 2)}, // NOTE: Not certain about this one
    {BlockID::shale_gravel,    glm::vec3(16, 16, 16)},
    {BlockID::silver_ore,      glm::vec3(59, 59, 59)},
    {BlockID::snow,            glm::vec3(4, 4, 4)},
    {BlockID::sulphur_ore,     glm::vec3(141, 141, 141)},
    {BlockID::titanium,        glm::vec3(88, 88, 88)},
    {BlockID::titanium_ore,    glm::vec3(87, 87, 87)},
    {BlockID::topsoil,         glm::vec3(14, 0, 1)},
    {BlockID::xenostone,       glm::vec3(127, 127, 127)},
    {BlockID::zircon_ore,      glm::vec3(183, 183, 183)},
    {BlockID::water,           glm::vec3(7, 7, 7)},
    {BlockID::sulphur_crystal, glm::vec3(21, 21, 21)},
    {BlockID::blue_crystal,    glm::vec3(58, 58, 58)},
    {BlockID::boron_crystal,   glm::vec3(22, 22, 22)},
    {BlockID::glass,           glm::vec3(35, 35, 35)},
    {BlockID::minilight_pz,    glm::vec3(9, 9, 9)},
    {BlockID::minilight_nz,    glm::vec3(9, 9, 9)},
    {BlockID::minilight_px,    glm::vec3(9, 9, 9)},
    {BlockID::minilight_nx,    glm::vec3(9, 9, 9)},
    {BlockID::minilight_py,    glm::vec3(9, 9, 9)},
    {BlockID::minilight_ny,    glm::vec3(9, 9, 9)}
};

int GetChunkIndex(int x, int y, int z)
{
    return y + WORLD_HEIGHT_LIMIT * z + (WORLD_HEIGHT_LIMIT * (CHUNK_SIZE + 2)) * x;
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
    return !BlockIsOpaque(face_neighbor) && face_block != face_neighbor;
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

    return glm::vec3(localBlockPosX + 1, globalBlockPos.y, localBlockPosZ + 1);
}

#endif