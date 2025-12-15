#ifndef CHUNK_H
#define CHUNK_H

#include <cstdint>
#include <vector>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "block.h"

void BuildChunkVertices(uint16_t *blocks, glm::ivec3 chunk_coords, std::vector<BlockVertex>& opaque_vertices, std::vector<BlockVertex>& transparent_vertices);

enum class ChunkState
{
    MISSING,
    QUEUED,
    UPLOADED
};

class Chunk
{
    private:
        glm::ivec3 _coords;
        uint16_t *_blocks;
        std::vector<BlockVertex> _opaque_vertices;
        GLuint _opaque_vao;
        GLuint _opaque_vbo;
        std::vector<BlockVertex> _transparent_vertices;
        GLuint _transparent_vao;
        GLuint _transparent_vbo;

    public:
        ChunkState state = ChunkState::MISSING;

        Chunk();
        Chunk(glm::ivec3 coords);

        // Copy
        Chunk(const Chunk&) = delete;
        Chunk& operator=(const Chunk&) = delete;

        // Move
        Chunk(Chunk&& other) noexcept;
        Chunk& operator=(Chunk&& other) noexcept;

        void Free();

        glm::ivec3 GetCoords();
        void SetCoords(glm::ivec3 coords);
        uint16_t *GetBlocks();
        void SetBlocks(uint16_t *blocks);
        void SetOpaqueVertices(std::vector<BlockVertex> &opaque_vertices);
        void SetTransparentVertices(std::vector<BlockVertex> &transparent_vertices);

        void BufferVertices();
        void RenderOpaques();
        void RenderTransparents();
};

#endif
