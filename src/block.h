#pragma once

#include <cstdint>
#include <unordered_map>

#include <glm/glm.hpp>

#include "constants.h"

enum class BlockID : uint8_t
{
    // Transparents
    air,
    water,
    sulphur_crystal,
    blue_crystal,
    boron_crystal,
    glass,

    // Opaques
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
    xenostone,
    zircon_ore,
    minilight_pz,
    minilight_nz,
    minilight_px,
    minilight_nx,
    minilight_py,
    minilight_ny
};

struct BlockVertex
{
    glm::vec3 position; // Global coordinates
    glm::vec4 uv; // {repeat_width, repeat_height, tile_origin_x, tile_origin_y}
    glm::vec3 face_normal;
    glm::vec2 light;
};

inline std::unordered_map<BlockID, glm::mat3x2> GetAtlasTileOrigins()
{
    static const std::unordered_map<BlockID, glm::vec3> ATLAS_TILE_MAP = { // Tile coordinates are: (top, side, bottom)
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
        {BlockID::sand,            glm::vec3(2, 2, 2)},
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

    static auto TILE_ORIGINS = []()
    {
        std::unordered_map<BlockID, glm::mat3x2> origins;

        for (auto &[block_id, atlas_tiles] : ATLAS_TILE_MAP)
        {
            glm::vec2 top_tile_origin = glm::vec2(
                ((int)atlas_tiles.x % 14) / 14.0f,
                (13 - ((int)atlas_tiles.x / 14)) / 14.0f
            );

            glm::vec2 side_tile_origin = glm::vec2(
                ((int)atlas_tiles.y % 14) / 14.0f,
                (13 - ((int)atlas_tiles.y / 14)) / 14.0f
            );

            glm::vec2 bottom_tile_origin = glm::vec2(
                ((int)atlas_tiles.z % 14) / 14.0f,
                (13 - ((int)atlas_tiles.z / 14)) / 14.0f
            );

            origins.emplace(block_id, glm::mat3x2(
                top_tile_origin, side_tile_origin, bottom_tile_origin
            ));
        }

        return origins;
    }();

    return TILE_ORIGINS;
}

