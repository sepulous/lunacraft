#ifndef CHUNK_H
#define CHUNK_H

#include <cstdint>
#include <vector>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "block.h"

struct Chunk
{
    glm::vec2 position;
    uint16_t *chunk_data;
    std::vector<BlockVertex> vertices;
    GLuint vao;
    GLuint vbo;
};

uint64_t CombineChunkCoordinates(int chunk_x, int chunk_z)
{
    uint64_t combined = (uint64_t)((uint32_t)chunk_x);
    combined <<= (sizeof(uint64_t) * 8 / 2);
    combined |= (uint64_t)((uint32_t)chunk_z);
    return combined;
}

glm::vec2 DecombineChunkCoordinates(uint64_t combined)
{
    int chunk_z = (int)((uint32_t)combined);
    combined >>= (sizeof(uint64_t) * 8 / 2);
    int chunk_x = (int)((uint32_t)combined);
    return glm::vec2(chunk_x, chunk_z);
}

#endif