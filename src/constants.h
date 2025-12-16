#ifndef CONSTANTS_H
#define CONSTANTS_H

constexpr int CHUNK_SIZE = 32;
constexpr int WORLD_HEIGHT_LIMIT = 128;
constexpr int GROUND_LEVEL = 64;
constexpr int BLOCKS_IN_CHUNK = (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2) * WORLD_HEIGHT_LIMIT;

#endif
