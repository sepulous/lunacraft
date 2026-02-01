#pragma once

#include <filesystem>
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

//
// We need to be able to update chunk vertices while rendering, so the vertex data is double-buffered.
//
// SwapBuffers() must be called after building chunk data.
//
class ChunkVertices
{
public:
    void SwapBuffers();
    void AddOpaqueVertex(BlockVertex &vertex);
    void AddTransparentVertex(BlockVertex &vertex);
    BlockVertex *GetOpaqueData();
    size_t GetOpaqueCount();
    size_t GetReservedOpaqueCount();
    void SetReservedOpaqueCount(size_t count);
    BlockVertex *GetTransparentData();
    size_t GetTransparentCount();
    size_t GetReservedTransparentCount();
    void SetReservedTransparentCount(size_t count);

private:
    uint8_t _read_buffer = 0;
    size_t _reserved_opaque_count = 0;      // So chunks know whether to reallocate
    size_t _reserved_transparent_count = 0; // GPU memory
    std::vector<BlockVertex> _opaque_buffers[2];
    std::vector<BlockVertex> _transparent_buffers[2];
};

class Lightmap
{
    private:
        uint8_t *_map;

    public:
        Lightmap();
        ~Lightmap();
        uint8_t GetSkyLevel(glm::ivec3 coords) const;
        uint8_t GetSkyLevel(int x, int y, int z) const;
        void SetSkyLevel(uint8_t level, glm::ivec3 coords);
        void SetSkyLevel(uint8_t level, int x, int y, int z);
        uint8_t GetBlockLevel(glm::ivec3 coords) const;
        uint8_t GetBlockLevel(int x, int y, int z) const;
        void SetBlockLevel(uint8_t level, glm::ivec3 coords);
        void SetBlockLevel(uint8_t level, int x, int y, int z);
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
    Chunk(glm::ivec3 coords, bool is_border_chunk, ChunkManager *chunk_manager);
    ~Chunk() = default;

    Chunk(const Chunk&) = delete;
    Chunk &operator=(const Chunk&) = delete;

    Chunk(Chunk&&) = delete;
    Chunk &operator=(Chunk&&) = delete;

    void GLCreate();
    void GLDestroy();

    ChunkState GetState();
    void SetIsBorderChunk(bool status);
    bool IsBorderChunk();
    bool HasUploadedVertices();
    bool HasGLData();
    glm::ivec3 GetCoords();
    BlockID *GetBlocks();
    const Lightmap &GetLightmap() const;
    std::filesystem::path GetFilePath();

    void Build();
    void BuildExternal();
    void Rebuild();
    void UploadVertices();
    void RenderOpaques();
    void RenderTransparents();

private:
    void SetState(ChunkState state);
    void LoadBlocks();
    void BuildLightmapInternal();
    void BuildLightmapExternal();
    void BuildVertices();

private:
    ChunkManager *_chunk_manager;
    std::atomic<ChunkState> _state{ChunkState::CREATED};
    bool _has_gl_data = false;
    bool _has_uploaded_vertices = false;
    bool _is_border_chunk;
    glm::ivec3 _coords;
    BlockID *_blocks;
    Lightmap _lightmap;
    ChunkVertices _vertices;
    GLuint _opaque_vao;
    GLuint _opaque_vbo;
    GLuint _transparent_vao;
    GLuint _transparent_vbo;
};
