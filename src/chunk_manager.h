#pragma once

#include <unordered_map>
#include <vector>
#include <thread>

#include <glm/glm.hpp>

#include "chunk.h"
#include "blocking_queue.h"
#include "moon_settings.h"
#include "helpers.h"

struct ChunkTask
{
    glm::ivec3 coords;
};

struct ChunkResult
{
    glm::ivec3 coords;
    BlockArray blocks;
    std::vector<BlockVertex> opaque_vertices;
    std::vector<BlockVertex> transparent_vertices;
};

class ChunkManager
{
    private:
        int _moon_id;
        int _loaded_chunk_count = 0;
        GLuint _texture_atlas;
        std::unordered_map<uint64_t, Chunk> _chunks;
        BlockingQueue<ChunkTask> _task_queue;
        BlockingQueue<ChunkResult> _result_queue;
        std::vector<std::jthread> _workers; // These should be destroyed before the queues

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
