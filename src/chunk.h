#pragma once

#include <cstdint>
#include <vector>
#include <array>

#include <glad/glad.h>

#include <glm/glm.hpp>

#include "block.h"
#include "constants.h"
#include "chunk_worker_pool.h"

class ChunkManager;

// TODO: We know how many vertices there will be: 6 * (# of quads). So before we push to the vertex vectors, let's reserve enough space.

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
};

enum class ChunkState
{
    CREATED, // Chunk object was just created
    LOADING_BLOCKS, // Loading/generating block data
    LIGHT_INTERNAL, // Initial light map fill based on internal data
    INTERNAL_DONE, // Chunk has internal data but has not incorporated neighbor chunk data yet (border chunks rest on this until they are ready to be rendered)
    LIGHT_EXTERNAL, // Incorporation of neighbor chunks' light maps
    BUILDING_VERTICES, // Meshing and building vertex data
    READY_TO_UPLOAD, // Vertices are ready to be uploaded to the GPU (must be done on main thread)
    RENDERABLE // Chunk is ready to render
};

class Chunk
{
public:
    Chunk(const glm::ivec3 &coords, bool is_border_chunk, ChunkManager *chunk_manager);
    ~Chunk();

    Chunk(const Chunk&) = delete;
    Chunk &operator=(const Chunk&) = delete;

    Chunk(Chunk&&) = delete;
    Chunk &operator=(Chunk&&) = delete;

    ChunkState GetState();
    void SetIsBorderChunk(bool status);
    bool IsBorderChunk();
    glm::ivec3 GetCoords();
    BlockID *GetBlocks();
    Lightmap &GetLightMap();

    void Build();
    void BuildExternal();
    void Rebuild();
    void UploadVertices();
    void RenderOpaques();
    void RenderTransparents();

private:
    void SetState(ChunkState state);
    void LoadBlocks();
    void BuildLightMapInternal();
    void BuildLightMapExternal();
    void BuildVertices();

private:
    ChunkManager *_chunk_manager;
    std::atomic<ChunkState> _state{ChunkState::CREATED};
    bool _is_border_chunk;
    glm::ivec3 _coords;
    BlockID *_blocks;
    Lightmap _light_map;
    std::vector<BlockVertex> _opaque_vertices;
    GLuint _opaque_vao;
    GLuint _opaque_vbo;
    std::vector<BlockVertex> _transparent_vertices;
    GLuint _transparent_vao;
    GLuint _transparent_vbo;
};
