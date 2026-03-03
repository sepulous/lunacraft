#pragma once

constexpr int CHUNK_SIZE = 32;
constexpr int WORLD_HEIGHT_LIMIT = 128;
constexpr int BLOCKS_IN_CHUNK = CHUNK_SIZE * CHUNK_SIZE * WORLD_HEIGHT_LIMIT;
constexpr int GROUND_LEVEL = 64;
constexpr int LIGHT_PHASES = 11;
constexpr int SECONDS_PER_LIGHT_PHASE = 30;
constexpr double FIXED_DELTA_TIME = 0.01;
constexpr int MAX_RENDER_DISTANCE = 12;
