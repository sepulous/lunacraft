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
    // Chunk data
    glm::vec2 position;
    uint16_t *blocks;

    // Rendering data
    std::vector<BlockVertex> opaque_vertices;
    GLuint opaque_vao;
    GLuint opaque_vbo;
    std::vector<BlockVertex> transparent_vertices;
    GLuint transparent_vao;
    GLuint transparent_vbo;
};

#endif
