#pragma once

#include <cstdint>

#include "constants.h"

struct MoonSettings
{
    uint64_t seed = 0;
    double world_time = 0;
    float skybox_phase = LIGHT_CYCLE_OMEGA * SECONDS_PER_LIGHT_PHASE;
    float tree_cover = 0.5f;
    float terrain_roughness = 0.5f;
    uint8_t wildlife_level = 2;
    bool is_creative = false;
    bool skybox_reversed = false;
};
