#pragma once

#include <cstdint>
#include <vector>
#include <array>

#include <glad/glad.h>

#include <glm/glm.hpp>

#include "block.h"
#include "constants.h"

typedef std::array<BlockID, BLOCKS_IN_CHUNK> BlockArray; // TODO: Get rid of this. It's stupid.

class Lightmap
{
    private:
        uint8_t _map[BLOCKS_IN_CHUNK];

    public:
        Lightmap() = default;
        ~Lightmap() = default;
        uint8_t GetSkyLevel(glm::ivec3 coords) const;
        void SetSkyLevel(glm::ivec3 coords, uint8_t level);
        uint8_t GetBlockLevel(glm::ivec3 coords) const;
        void SetBlockLevel(glm::ivec3 coords, uint8_t level);
        uint8_t GetTotalLightLevel(glm::ivec3 coords) const;
};

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
        Lightmap _lightmap;
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

void BuildLightmap(BlockID *blocks, Lightmap &lightmap);
void BuildChunkVertices(BlockID *blocks, glm::ivec3 chunk_coords, std::vector<BlockVertex> &opaque_vertices, std::vector<BlockVertex> &transparent_vertices, const Lightmap &light_map);
