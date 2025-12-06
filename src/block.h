#ifndef BLOCKS_H
#define BLOCKS_H

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
    glm::vec3 position; // Global coordinates
    glm::vec4 uv; // {repeat_width, repeat_height, tile_origin_x, tile_origin_y}
    glm::vec3 face_normal;
    
    BlockVertex(glm::vec3 position, glm::vec4 uv, glm::vec3 face_normal) : position(position), uv(uv), face_normal(face_normal) {}
};

#endif