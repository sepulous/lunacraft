#ifndef CHUNK_H
#define CHUNK_H

#include <cstdint>
#include <vector>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "block.h"

class Chunk
{
    private:
        glm::vec2 _position;
        uint16_t *_blocks;
        std::vector<BlockVertex> _opaque_vertices;
        GLuint _opaque_vao;
        GLuint _opaque_vbo;
        std::vector<BlockVertex> _transparent_vertices;
        GLuint _transparent_vao;
        GLuint _transparent_vbo;

    public:
        Chunk(glm::vec2 position);
        glm::vec2 GetPosition();
        uint16_t *GetBlocks();
        void BuildVertices();
        void BufferVertices();
        void RenderOpaques();
        void RenderTransparents();
};

#endif
