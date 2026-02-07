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

class ChunkManager
{
    private:
        int _loaded_chunk_count = 0;
        GLuint _texture_atlas;
        std::unordered_map<uint64_t, std::shared_ptr<Chunk>> _chunks;
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
        std::array<std::shared_ptr<Chunk>, 4> GetAdjacentNeighbors(glm::ivec3 chunk_coords);
        std::array<std::shared_ptr<Chunk>, 8> GetAllNeighbors(glm::ivec3 chunk_coords);
        std::shared_ptr<Chunk> GetChunk(glm::ivec3 chunk_coords);
        std::unordered_map<uint64_t, std::shared_ptr<Chunk>> &GetChunks();
        ChunkWorkerPool *GetWorkerPool();
        int GetLoadedChunkCount();
        void WriteAllChunksToDisk();
};
