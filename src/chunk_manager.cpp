
#include <filesystem>
#include <string>
#include <fstream>
#include <thread>
#include <stop_token>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image/stb_image.h>

#include "chunk_manager.h"
#include "chunk_generation.h"
#include "helpers.h"
#include "storage.h"
#include "options.h"
#include "moon.h"

ChunkManager::ChunkManager()
{
    _block_memory_head.blocks = (BlockID *)malloc(BLOCKS_IN_CHUNK * sizeof(BlockID));
}

ChunkManager::~ChunkManager()
{
    glDeleteTextures(1, &_texture_atlas);

    BlockMemoryNode *node = &_block_memory_head;
    do
    {
        free(node->blocks);
        node = node->next;
    } while (node != nullptr);
}

void ChunkManager::Init(int moon_id, MoonSettings moon_settings)
{
    // Load texture atlas
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    std::filesystem::path atlas_path = Storage::IMAGE_DIR / "texture_atlas.png";
    unsigned char *texture_atlas_data = stbi_load(reinterpret_cast<const char *>(atlas_path.u8string().c_str()), &width, &height, &nrChannels, 0);

    glGenTextures(1, &_texture_atlas);
    glBindTexture(GL_TEXTURE_2D, _texture_atlas);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_atlas_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(texture_atlas_data);

    // Create chunks folder
    std::filesystem::path moon_dir = Storage::MOON_DIR / (std::string("moon") + std::to_string(moon_id));
    std::filesystem::path chunk_dir = moon_dir / "chunks";
    if (!std::filesystem::exists(chunk_dir))
        std::filesystem::create_directory(chunk_dir);
}

void ChunkManager::UploadReadyChunks()
{
    std::unique_lock<std::mutex> lock(_chunks_mutex);

    for (auto it = _chunks.begin(); it != _chunks.end(); ++it)
    {
        Chunk *chunk = it->second.get();
        if (!chunk->IsBorderChunk() && chunk->GetState() == ChunkState::READY_TO_UPLOAD) // Shouldn't waste GPU memory with border chunks. Most are never rendered.
        {
            if (!chunk->HasGLData())
                chunk->GLCreate();
            chunk->UploadVertices();
            chunk->SetHasUploadedVertices(true);
            _loaded_chunk_count++;
        }
    }
}

void ChunkManager::RenderChunks(Plane frustum[6])
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture_atlas);
    glDepthFunc(GL_LESS);

    std::unique_lock<std::mutex> lock(_chunks_mutex);

    std::vector<Chunk *> visible_chunks;

    for (auto it = _chunks.begin(); it != _chunks.end(); ++it)
    {
        Chunk *chunk = it->second.get();
        if (chunk->HasUploadedVertices() && !chunk->IsBorderChunk()) // Border chunks should never be rendered, and patch chunks aren't necessarily ready
        {
            glm::ivec3 chunk_coords = chunk->GetCoords();
            float x0 = chunk_coords.x * CHUNK_SIZE;
            float y0 = 0;
            float z0 = chunk_coords.z * CHUNK_SIZE;
            float x1 = x0 + CHUNK_SIZE;
            float y1 = WORLD_HEIGHT_LIMIT;
            float z1 = z0 + CHUNK_SIZE;

            glm::vec3 min(x0, y0, z0);
            glm::vec3 max(x1, y1, z1);

            if (ChunkInFrustum(frustum, min, max))
            {
                chunk->RenderOpaques();
                visible_chunks.push_back(chunk);
            }
        }
    }

    // Render transparent blocks
    for (Chunk *chunk : visible_chunks)
    {
        chunk->RenderTransparents();
    }
}

void ChunkManager::RebuildChunks()
{
    std::unique_lock<std::mutex> lock(_chunks_mutex);

    for (auto it = _chunks.begin(); it != _chunks.end(); ++it)
    {
        Chunk *chunk = it->second.get();
        if (!chunk->IsBorderChunk())
            chunk->Rebuild();
    }
}

//
// Create initial chunk patch around player spawn.
//
// This only needs to build the chunk the player spawns in, because required
// neighbors are recursively loaded in.
//
void ChunkManager::CreateInitialPatch()
{
    auto player_chunk = VoxelToChunk(GetNearestVoxel(Moon::GetCurrentMoon()->GetPlayer()->GetPosition()));
    uint64_t chunk_id = ChunkCoordsToID(player_chunk);
    auto [it, success] = _chunks.try_emplace(chunk_id, std::make_shared<Chunk>(player_chunk, false, this));
    it->second->Build();
}

//
// Chunks call this to request memory to store their block data, instead of
// managing such memory themselves. This allows the memory to be reused.
//
BlockID *ChunkManager::AllocateBlockMemory()
{
    // Start with head node
    BlockMemoryNode *node = &_block_memory_head;

    // Find first node that is not in use
    while (node->in_use && node->next != nullptr) { node = node->next; }

    if (node->in_use && node->next == nullptr) // If no such node exists, create a new one
    {
        //
        // We could double the number of free nodes instead of only creating one at a time.
        //
        // As it stands, initially this system is no better than having chunks allocate block
        // memory each time they're created. However, once the maximum number of possible nodes
        // is hit (for a given render distance), block data never needs to be allocated again,
        // and we have exactly as many nodes as we need.
        //

        BlockID *new_blocks = (BlockID *)malloc(BLOCKS_IN_CHUNK * sizeof(BlockID));
        BlockMemoryNode *new_node = new BlockMemoryNode{.next = nullptr, .blocks = new_blocks, .in_use = true};
        node->next = new_node;
        return new_blocks;
    }
    else // Otherwise, use existing node
    {
        node->in_use = true;
        return node->blocks;
    }
}

std::shared_ptr<Chunk> ChunkManager::GetOrCreateChunk(glm::ivec3 chunk_coords)
{
    std::unique_lock<std::mutex> lock(_chunks_mutex);

    uint64_t chunk_id = ChunkCoordsToID(chunk_coords);
    if (_chunks.contains(chunk_id))
    {
        return _chunks.at(chunk_id);
    }
    else
    {
        auto player_chunk = VoxelToChunk(GetNearestVoxel(Moon::GetCurrentMoon()->GetPlayer()->GetPosition()));
        auto render_distance = OptionsManager::GetOptions().render_distance;
        bool is_border_chunk = chunk_coords.x == player_chunk.x - render_distance - 1
                            || chunk_coords.x == player_chunk.x + render_distance + 1
                            || chunk_coords.z == player_chunk.z - render_distance - 1
                            || chunk_coords.z == player_chunk.z + render_distance + 1;

        auto [it, success] = _chunks.try_emplace(chunk_id, std::make_shared<Chunk>(chunk_coords, is_border_chunk, this));
        it->second->Build();
        return it->second;
    }
}

void ChunkManager::AdjustChunkPatch()
{
    std::unique_lock<std::mutex> lock(_chunks_mutex);

    auto player_chunk = VoxelToChunk(GetNearestVoxel(Moon::GetCurrentMoon()->GetPlayer()->GetPosition()));
    auto render_distance = OptionsManager::GetOptions().render_distance;

    // Remove all chunks outside the patch + border
    for (auto it = _chunks.begin(); it != _chunks.end(); )
    {
        auto chunk = it->second;

        glm::ivec3 coords = chunk->GetCoords();
        bool outside_border = coords.x < player_chunk.x - render_distance - 1
                          || coords.x > player_chunk.x + render_distance + 1
                          || coords.z < player_chunk.z - render_distance - 1
                          || coords.z > player_chunk.z + render_distance + 1;

        auto chunk_state = chunk->GetState();
        bool can_be_deleted = chunk.use_count() == 1 && (chunk_state == ChunkState::INTERNAL_DONE || chunk_state >= ChunkState::READY_TO_UPLOAD);

        //
        // The ChunkManager ultimately owns the chunks, so it should be the last thing to delete its shared pointer.
        // When there are no more owners, the chunk data is freed, so this must not happen unless the chunk is
        // in an idle state (not accessing its own data).
        //

        if (outside_border && can_be_deleted)
        {
            chunk->GLDestroy(); // Deletes OpenGL data
            it = _chunks.erase(it); // Frees CPU data
            _loaded_chunk_count--;
        }
        else
        {
            ++it;
        }
    }

    // Update patch + border
    for (int x = player_chunk.x - render_distance - 1; x <= player_chunk.x + render_distance + 1; x++)
    {
        for (int z = player_chunk.z - render_distance - 1; z <= player_chunk.z + render_distance + 1; z++)
        {
            uint64_t chunk_id = ChunkCoordsToID({x, 0, z});
            bool on_new_border = x == player_chunk.x - render_distance - 1
                                || x == player_chunk.x + render_distance + 1
                                || z == player_chunk.z + render_distance + 1
                                || z == player_chunk.z - render_distance - 1;

            auto [it, success] = _chunks.try_emplace(chunk_id, std::make_shared<Chunk>(glm::ivec3{x, 0, z}, on_new_border, this));
            auto chunk = it->second;
            if (on_new_border)
            {
                if (success) // New border chunk
                    chunk->Build();
                else // Convert existing 
                    chunk->SetIsBorderChunk(true);
            }
            else
            {
                if (!success && chunk->IsBorderChunk())
                {
                    chunk->SetIsBorderChunk(false); // Convert to non-border chunk
                    chunk->BuildExternal(); // Finish building
                }
            }
        }
    }
}

std::unordered_map<uint64_t, std::shared_ptr<Chunk>> &ChunkManager::GetChunks()
{
    return _chunks;
}

ChunkWorkerPool &ChunkManager::GetWorkerPool()
{
    return _worker_pool;
}

int ChunkManager::GetLoadedChunkCount()
{
    return _loaded_chunk_count;
}
