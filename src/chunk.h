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

struct MeshQuad
{
    BlockID block;
    glm::ivec3 base_coords; // Local voxel position
    glm::vec3 du;
    glm::vec3 dv;
    bool back_face;
};

class Lightmap
{
    private:
        uint8_t *_block_light;
        uint8_t *_sky_light;

    public:
        Lightmap();
        ~Lightmap();
        uint8_t GetSkyLevel(glm::ivec3 coords) const;
        uint8_t GetSkyLevel(int x, int y, int z) const;
        uint8_t GetSkyLevel(uint32_t idx) const;

        void SetSkyLevel(uint8_t level, glm::ivec3 coords);
        void SetSkyLevel(uint8_t level, int x, int y, int z);
        void SetSkyLevel(uint8_t level, uint32_t idx);

        uint8_t GetBlockLevel(glm::ivec3 coords) const;
        uint8_t GetBlockLevel(int x, int y, int z) const;
        uint8_t GetBlockLevel(uint32_t idx) const;

        void SetBlockLevel(uint8_t level, glm::ivec3 coords);
        void SetBlockLevel(uint8_t level, int x, int y, int z);
        void SetBlockLevel(uint8_t level, uint32_t idx);

        uint8_t GetCombinedLight(glm::ivec3 coords) const;
        uint8_t GetCombinedLight(int x, int y, int z) const;

        void ClearBlockLight();
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
    void LoadBlocks();
    void BuildLightmapInternal();
    void BuildLightmapExternal();
    void UpdateVertexLighting();
    void BuildVertices();

// Lifetime control (main thread)
public:
    void Pin();
    void PinNeighbors();
    void Unpin();
    void UnpinNeighbors();
    int GetPinCount();
    void MarkForDelete();
    bool IsMarkedForDelete();

// Job control
public:
    void MarkAsDirty(); // Main thread (when job is submitted)
    void MarkAsClean(); // Worker thread (when job is finished)
    bool IsDirty();

private:
    void SetState(ChunkState state);
    std::vector<MeshQuad> GreedyMesh(std::array<Chunk *, 4> neighbors);

private:
    ChunkManager *chunk_manager_;
    std::atomic<ChunkState> state_{ChunkState::CREATED};
    std::atomic<bool> dirty_{false};
    bool has_uploaded_vertices_ = false;
    bool is_border_chunk_;
    glm::ivec3 coords_;
    BlockID *blocks_;
    Lightmap lightmap_;

    // Lifetime control
    std::atomic<int> pins_{0};
    std::atomic<bool> marked_for_delete_{false};

    // Opaque data
    std::vector<BlockVertex> opaque_vertices_;
    std::vector<uint16_t> opaque_indices_;
    GLuint opaque_vaos_[2];
    GLuint opaque_vbos_[2];
    GLuint opaque_ebos_[2];
    size_t opaque_counts_[2];

    // Transparent data
    std::vector<BlockVertex> transparent_vertices_;
    std::vector<uint16_t> transparent_indices_;
    GLuint transparent_vaos_[2];
    GLuint transparent_vbos_[2];
    GLuint transparent_ebos_[2];
    size_t transparent_counts_[2];

    unsigned gl_index_ = 0; // Index for double-buffering
};
