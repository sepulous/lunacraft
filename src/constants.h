#pragma once

constexpr int CHUNK_SIZE = 32;
constexpr int WORLD_HEIGHT_LIMIT = 128;
constexpr int BLOCKS_IN_CHUNK = CHUNK_SIZE * CHUNK_SIZE * WORLD_HEIGHT_LIMIT;
constexpr int GROUND_LEVEL = 64;
constexpr int LIGHT_PHASES = 11;
constexpr int SECONDS_PER_LIGHT_PHASE = 30;
constexpr float LIGHT_CYCLE_OMEGA = 2.0f * 3.1416f / (LIGHT_PHASES * SECONDS_PER_LIGHT_PHASE);
constexpr double FIXED_DELTA_TIME = 0.01;
constexpr int MAX_RENDER_DISTANCE = 12;
constexpr float GRAVITY = 4.0f;
