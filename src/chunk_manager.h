#pragma once

#include <unordered_map>
#include <memory>

#include <glm/glm.hpp>

#include <glad/glad.h>

#include "moon_settings.h"
#include "helpers.h"
#include "chunk_worker_pool.h"
#include "chunk.h"

class Moon;

struct BlockMemory
{
    BlockID *blocks;
    bool in_use;
    uint64_t owner; // Chunk ID
};

class ChunkManager
{
    private:
        int _loaded_chunk_count = 0;
        GLuint _texture_atlas;
        std::vector<BlockMemory> _block_memory;
        std::unordered_map<uint64_t, Chunk *> _chunks;
        ChunkWorkerPool *_worker_pool;

    public:
        ChunkManager() = default;
        ~ChunkManager();
        void Init(int moon_id, MoonSettings moon_settings);
        void CreateInitialPatch();
        void AdjustChunkPatch();
        void UploadReadyChunks();
        void RenderChunks(Plane frustum[6]);
        void RebuildChunks();
        BlockID *GetBlockMemory(uint64_t chunk_id);
        std::array<Chunk *, 4> GetAdjacentNeighbors(glm::ivec3 chunk_coords);
        std::array<Chunk *, 8> GetAllNeighbors(glm::ivec3 chunk_coords);
        Chunk *GetChunk(glm::ivec3 chunk_coords);
        std::unordered_map<uint64_t, Chunk *> &GetChunks();
        ChunkWorkerPool *GetWorkerPool();
        int GetLoadedChunkCount();
        void WriteAllChunksToDisk();

    private:
        void ReuseBlockMemory(uint64_t chunk_id);
};
