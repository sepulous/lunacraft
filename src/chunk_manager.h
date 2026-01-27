#pragma once

#include <unordered_map>

#include <glm/glm.hpp>

#include "chunk.h"
#include "moon_settings.h"
#include "helpers.h"
#include "chunk_worker_pool.h"

class ChunkManager
{
    private:
        int _moon_id;
        int _loaded_chunk_count = 0;
        GLuint _texture_atlas;
        std::unordered_map<uint64_t, Chunk> _chunks;
        ChunkWorkerPool _worker_pool;

    public:
        ChunkManager() = default;
        ~ChunkManager();
        void Init(int moon_id, MoonSettings moon_settings);
        void QueueNewChunk(glm::ivec3 chunk_coords);
        void RenderChunks(Plane frustum[6]);
        void BufferReadyChunks();
        void RemoveDistantChunks(glm::ivec3 player_chunk, int render_distance);
        std::unordered_map<uint64_t, Chunk> &GetChunks();
        int GetLoadedChunkCount();
};
