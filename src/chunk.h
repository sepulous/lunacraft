#ifndef CHUNK_H
#define CHUNK_H

#include <cstdint>
#include <vector>
#include <array>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "block.h"
#include "constants.h"

typedef std::array<BlockID, BLOCKS_IN_CHUNK> BlockArray;

void BuildChunkVertices(BlockID *blocks, glm::ivec3 chunk_coords, std::vector<BlockVertex> &opaque_vertices, std::vector<BlockVertex> &transparent_vertices);

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
        BlockArray _blocks;
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

        ~Chunk();

        // Copy
        Chunk(const Chunk&) = delete;
        Chunk& operator=(const Chunk&) = delete;

        // Move
        Chunk(Chunk&& other) noexcept;
        Chunk& operator=(Chunk&& other) noexcept;

        glm::ivec3 GetCoords();
        void SetCoords(glm::ivec3 coords);
        BlockArray &GetBlocks();
        void SetBlocks(BlockArray &blocks);
        void SetOpaqueVertices(std::vector<BlockVertex> &opaque_vertices);
        void SetTransparentVertices(std::vector<BlockVertex> &transparent_vertices);

        void BufferVertices();
        void RenderOpaques();
        void RenderTransparents();
};

#endif
