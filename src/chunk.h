#pragma once

#include <filesystem>
#include <cstdint>
#include <vector>
#include <array>
#include <atomic>

#include <glad/glad.h>

#include <glm/glm.hpp>

#include "block.h"
#include "constants.h"

class ChunkManager;

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
    INTERNAL_DONE, // Chunk has internal data but has not incorporated neighbor chunk data yet
    LIGHT_EXTERNAL, // Incorporation of neighbor chunks' light maps
    UPDATING_VERTEX_LIGHTING, // Updating light values of vertices (not modifying lightmap)
    BUILDING_VERTICES, // Meshing and building vertex data
    READY_TO_UPLOAD, // Vertices are ready to be uploaded to the GPU (must be done on main thread)
    RENDERABLE // Chunk is ready to render
};

class Chunk
{
public:
    Chunk(glm::ivec3 coords, bool is_border_chunk, ChunkManager *chunk_manager);
    ~Chunk();

    Chunk(const Chunk&) = delete;
    Chunk &operator=(const Chunk&) = delete;

    Chunk(Chunk&&) = delete;
    Chunk &operator=(Chunk&&) = delete;

    ChunkState GetState();
    void SetIsBorderChunk(bool status);
    bool IsBorderChunk();
    bool HasUploadedVertices();
    uint64_t GetID();
    glm::ivec3 GetCoords();
    BlockID *GetBlocks();
    const Lightmap &GetLightmap() const;
    std::filesystem::path GetFilePath();

// Main thread tasks
public:
    void UploadVertices();
    void RenderOpaques();
    void RenderTransparents();

// Worker tasks
public:
    bool LoadBlocks();
    bool BuildLightmapInternal();
    bool BuildLightmapExternal();
    bool UpdateVertexLighting();
    bool BuildVertices();
    bool UnpinNeighbors(); // This really belongs in the section below, it's just more ergonomic to include it in a chunk job

// Lifetime control (main thread)
public:
    void Pin();
    void PinNeighbors();
    void Unpin();
    int GetPinCount();
    void MarkForDelete();
    bool IsMarkedForDelete();

private:
    void SetState(ChunkState state);

private:
    ChunkManager *_chunk_manager;
    std::atomic<ChunkState> _state{ChunkState::CREATED};
    bool _has_uploaded_vertices = false;
    bool _is_border_chunk;
    glm::ivec3 _coords;
    BlockID *_blocks;
    Lightmap _lightmap;

    // Opaque vertices
    std::vector<BlockVertex> _opaque_vertices;
    std::vector<uint16_t> _opaque_indices;
    size_t _reserved_opaque_vertex_count = 0;
    GLuint _opaque_vao;
    GLuint _opaque_vbo;
    GLuint _opaque_ebo;

    // Transparent vertices
    std::vector<BlockVertex> _transparent_vertices;
    std::vector<uint16_t> _transparent_indices;
    size_t _reserved_transparent_vertex_count = 0;
    GLuint _transparent_vao;
    GLuint _transparent_vbo;
    GLuint _transparent_ebo;

    // Lifetime control
    std::atomic<int> _pins{0};
    std::atomic<bool> _marked_for_delete{false};
};

struct ChunkTask
{
    static bool (Chunk::*LOAD_BLOCKS)();
    static bool (Chunk::*BUILD_LIGHTMAP_INTERNAL)();
    static bool (Chunk::*BUILD_LIGHTMAP_EXTERNAL)();
    static bool (Chunk::*UPDATE_VERTEX_LIGHTING)();
    static bool (Chunk::*BUILD_VERTICES)();
    static bool (Chunk::*UNPIN_NEIGHBORS)();
};
