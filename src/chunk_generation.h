#pragma once

#include <cstdint>

#include "block.h"

struct MoonSettings;

void GenerateChunk(BlockID *chunk, int chunk_x, int chunk_z, MoonSettings settings);
