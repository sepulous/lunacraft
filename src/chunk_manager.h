#pragma once

#include <unordered_map>
#include <vector>
#include <queue>
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
    std::unique_ptr<BlockID[]> blocks;
    uint64_t owner; // Chunk ID
    bool in_use;
};

class ChunkManager
{
    public:
        ChunkManager() = default;
        ~ChunkManager();

        ChunkManager(const ChunkManager&) = delete;
        ChunkManager &operator=(const ChunkManager&) = delete;

        ChunkManager(ChunkManager&&) = delete;
        ChunkManager &operator=(ChunkManager&&) = delete;

        void Init(int moon_id, MoonSettings moon_settings);
        void HandleChunkJobs();
        void CreateInitialPatch();
        void AdjustChunkPatch();
        void UploadReadyChunks();
        void RenderChunks(Plane frustum[6]);
        void UpdateGlobalLighting();
        void HandlePlayerModification(glm::ivec3 voxel, BlockID block_placed = BlockID::air);
        void HandleBrownMobExplosion(glm::ivec3 explosion_center);
        BlockID *GetBlockMemory(uint64_t chunk_id);
        std::array<Chunk *, 4> GetAdjacentNeighbors(glm::ivec3 chunk_coords);
        std::array<Chunk *, 8> GetAllNeighbors(glm::ivec3 chunk_coords);
        Chunk *GetChunk(glm::ivec3 chunk_coords);
        std::vector<Chunk *> GetAllChunks();
        BlockID GetBlockAt(glm::ivec3 voxel);
        ChunkWorkerPool *GetWorkerPool();
        int GetLoadedChunkCount();
        void WriteAllChunksToDisk();

    private:
        void ReuseBlockMemory(uint64_t chunk_id);

    private:
        int loaded_chunk_count_ = 0;
        GLuint texture_atlas_;
        std::vector<BlockMemory> block_memory_;
        std::queue<ChunkJob> job_queue_;
        std::vector<uint64_t> need_entities_;
        std::unordered_map<uint64_t, std::unique_ptr<Chunk>> chunks_;
        std::unique_ptr<ChunkWorkerPool> worker_pool_;
};
