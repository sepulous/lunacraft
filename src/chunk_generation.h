#pragma once

#include <cstdint>

#include "block.h"

struct MoonSettings;

uint32_t GetStructureSeed(uint64_t world_seed, int chunk_x, int chunk_z);
void GenerateHeightMap(uint8_t *height_map, int chunk_x, int chunk_z, uint64_t seed, float amplitude, float frequency, float persistence, int octaves, float roughness);
void GenerateChunk(BlockID *chunk, int chunk_x, int chunk_z, MoonSettings settings);
