#pragma once

#include <cstdint>

#include "constants.h"

struct MoonSettings
{
    glm::vec4 base_fog_color;
    uint64_t seed = 0;
    double world_time = 42.0;
    float skybox_phase = 0;
    float tree_cover = 0.5f;
    float terrain_roughness = 0.5f;
    uint8_t wildlife_level = 2;
    bool is_creative = false;
    bool skybox_reversed = false;
};
