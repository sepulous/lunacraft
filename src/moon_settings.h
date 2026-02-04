#pragma once

#include <cstdint>

struct MoonSettings
{
    float tree_cover = 0.5f;
    uint8_t terrain_roughness = 2;
    uint8_t wildlife_level = 2;
    uint64_t seed = 0;
    double world_time = 0;
    bool is_creative = false;
};
