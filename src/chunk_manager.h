#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

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
        std::unordered_map<uint64_t, Chunk> _chunks;
        std::vector<std::thread> _workers;
        BlockingQueue<ChunkTask> _task_queue;
        BlockingQueue<ChunkResult> _result_queue;

    public:
        ChunkManager();
        void Init(int moon_id, MoonSettings moon_settings);
        void QueueNewChunk(glm::ivec3 chunk_coords);
        void RenderChunks(Plane frustum[6]);
        void BufferReadyChunks();
        void RemoveDistantChunks(glm::ivec3 player_chunk, int render_distance);
        std::unordered_map<uint64_t, Chunk> &GetChunks();
        int GetLoadedChunkCount();
        void Unload();
};

#endif
