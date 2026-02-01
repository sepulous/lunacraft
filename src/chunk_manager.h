#pragma once

#include <unordered_map>
#include <memory>
#include <mutex>

#include <glm/glm.hpp>

#include <glad/glad.h>

#include "moon_settings.h"
#include "helpers.h"
#include "chunk_worker_pool.h"
#include "chunk.h"

class Moon;

// 
// ChunkManager hands out memory for block storage to chunks so it
// doesn't have to be allocated every time a chunk is created.
//
// I'm using a linked list for this purpose.
//
struct BlockMemoryNode
{
    BlockMemoryNode *next = nullptr;
    BlockID *blocks = nullptr; // After ChunkManager is constructed, every BlockMemoryNode is expected to have a valid blocks pointer
    bool in_use = false;
};

//
// ChunkManager manages the state and rendering of all loaded chunks.
//
class ChunkManager
{
    private:
        int _loaded_chunk_count = 0;
        GLuint _texture_atlas;
        BlockMemoryNode *_block_memory_head;
        std::unordered_map<uint64_t, std::shared_ptr<Chunk>> _chunks;
        std::mutex _chunks_mutex;
        ChunkWorkerPool _worker_pool;

    public:
        ChunkManager();
        ~ChunkManager();
        void Init(int moon_id, MoonSettings moon_settings);
        void CreateInitialPatch();
        void AdjustChunkPatch();
        void UploadReadyChunks();
        void RenderChunks(Plane frustum[6]);
        void RebuildChunks();
        BlockID *AllocateBlockMemory();
        std::shared_ptr<Chunk> GetOrCreateChunk(glm::ivec3 chunk_coords);
        std::unordered_map<uint64_t, std::shared_ptr<Chunk>> &GetChunks();
        ChunkWorkerPool &GetWorkerPool();
        int GetLoadedChunkCount();
};
