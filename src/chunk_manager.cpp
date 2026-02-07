
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

ChunkManager::~ChunkManager()
{
    glDeleteTextures(1, &_texture_atlas);

    // Stop worker pool
    delete _worker_pool;

    // Free all block memory
    for (auto &memory : _block_memory)
        free(memory.blocks);
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

    // Reserve for maximum number of possible memory blocks (including border chunks)
    _block_memory.reserve((2*MAX_RENDER_DISTANCE + 3) * (2*MAX_RENDER_DISTANCE + 3));

    // Start worker pool
    _worker_pool = new ChunkWorkerPool{};
}

void ChunkManager::UploadReadyChunks()
{
    for (auto it = _chunks.begin(); it != _chunks.end(); ++it)
    {
        Chunk *chunk = it->second.get();
        if (!chunk->IsBorderChunk() && chunk->GetState() == ChunkState::READY_TO_UPLOAD) // Shouldn't waste GPU memory with border chunks. Most are never rendered.
        {
            chunk->UploadVertices();
            _loaded_chunk_count++;
        }
    }
}

void ChunkManager::RenderChunks(Plane frustum[6])
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture_atlas);
    glDepthFunc(GL_LESS);

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
void ChunkManager::CreateInitialPatch()
{
    // Create all chunks
    auto player_chunk = VoxelToChunk(GetNearestVoxel(Moon::GetCurrentMoon()->GetPlayer()->GetPosition()));
    int render_distance = OptionsManager::GetOptions().render_distance;
    for (int x = player_chunk.x - render_distance - 1; x <= player_chunk.x + render_distance + 1; x++)
    {
        for (int z = player_chunk.z - render_distance - 1; z <= player_chunk.z + render_distance + 1; z++)
        {
            auto chunk_id = ChunkCoordsToID({x, 0, z});
            bool is_border_chunk = x == player_chunk.x - render_distance - 1
                                || x == player_chunk.x + render_distance + 1
                                || z == player_chunk.z - render_distance - 1
                                || z == player_chunk.z + render_distance + 1;

            _chunks.emplace(chunk_id, std::make_shared<Chunk>(glm::ivec3{x, 0, z}, is_border_chunk, this));
        }
    }

    // Chunks expect their neighbors to exist when building, so we defer it
    for (auto it = _chunks.begin(); it != _chunks.end(); ++it)
        it->second->Build();
}

std::array<std::shared_ptr<Chunk>, 4> ChunkManager::GetAdjacentNeighbors(glm::ivec3 chunk_coords)
{
    std::array<std::shared_ptr<Chunk>, 4> neighbors;

    glm::ivec3 neighbor_coords[] = {
        {chunk_coords.x, 0, chunk_coords.z + 1}, // Front
        {chunk_coords.x + 1, 0, chunk_coords.z}, // Right
        {chunk_coords.x, 0, chunk_coords.z - 1}, // Back
        {chunk_coords.x - 1, 0, chunk_coords.z}  // Left
    };

    size_t idx = 0;
    for (auto &neighbor : neighbor_coords)
    {
        auto chunk_id = ChunkCoordsToID(neighbor);
        neighbors[idx] = _chunks.at(chunk_id); // Existence of neighbors should be guaranteed
        idx++;
    }

    return neighbors;
}

std::array<std::shared_ptr<Chunk>, 8> ChunkManager::GetAllNeighbors(glm::ivec3 chunk_coords)
{
    std::array<std::shared_ptr<Chunk>, 8> neighbors;

    glm::ivec3 neighbor_coords[] = {
        {chunk_coords.x - 1, 0, chunk_coords.z},
        {chunk_coords.x + 1, 0, chunk_coords.z},
        {chunk_coords.x, 0, chunk_coords.z - 1},
        {chunk_coords.x, 0, chunk_coords.z + 1},
        {chunk_coords.x - 1, 0, chunk_coords.z - 1},
        {chunk_coords.x - 1, 0, chunk_coords.z + 1},
        {chunk_coords.x + 1, 0, chunk_coords.z - 1},
        {chunk_coords.x + 1, 0, chunk_coords.z + 1},
    };

    size_t idx = 0;
    for (auto &neighbor : neighbor_coords)
    {
        auto chunk_id = ChunkCoordsToID(neighbor);
        neighbors[idx] = _chunks.at(chunk_id); // Existence of neighbors should be guaranteed
        idx++;
    }

    return neighbors;
}

// I'd like to make this more efficient so I can do it regularly. Then I don't need to rely on
// any information (whether player chunk changed or render distance changed). Just grab the
// current player chunk and render distance and make sure everything is correct.
//
// It honestly makes sense to have a fixed interval. Then it's completely predictable, and
// doesn't need to know if anything changed.
void ChunkManager::AdjustChunkPatch()
{
    auto player_chunk = VoxelToChunk(GetNearestVoxel(Moon::GetCurrentMoon()->GetPlayer()->GetPosition()));
    auto render_distance = OptionsManager::GetOptions().render_distance;

    // Remove all chunks outside the patch + border
    for (auto it = _chunks.begin(); it != _chunks.end(); )
    {
        auto &chunk = it->second;

        glm::ivec3 coords = chunk->GetCoords();
        bool marked_for_delete = chunk->IsMarkedForDelete();
        bool outside_border = coords.x < player_chunk.x - render_distance - 1
                          || coords.x > player_chunk.x + render_distance + 1
                          || coords.z < player_chunk.z - render_distance - 1
                          || coords.z > player_chunk.z + render_distance + 1;

        if (marked_for_delete || outside_border)
        {
            chunk->MarkForDelete();
            if (chunk->GetPinCount() < 1)
            {
                // Write chunk to disk and then take block memory back
                auto file_path = chunk->GetFilePath();
                auto blocks = chunk->GetBlocks();
                auto chunk_id = chunk->GetID();
                _worker_pool->SubmitJob([this, file_path, blocks, chunk_id]() {
                    WriteChunkToDisk(file_path, blocks);
                    ReuseBlockMemory(chunk_id);
                });

                // Erase chunk
                it = _chunks.erase(it);
                _loaded_chunk_count--;
            }
        }
        else
        {
            ++it;
        }
    }

    std::vector<std::shared_ptr<Chunk>> to_convert;

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
                if (success) // Build new border chunk
                    chunk->Build();
                else // Convert existing non-border chunk
                    chunk->SetIsBorderChunk(true);
            }
            else // In patch
            {
                if (!success && chunk->IsBorderChunk()) // Is an existing border chunk
                {
                    // We can't convert and build yet, as we haven't guaranteed the existence
                    // of all neighbors, so we must defer.
                    to_convert.push_back(chunk);
                }
            }
        }
    }

    // Convert border chunks that should now be rendered and finish building
    for (auto &chunk : to_convert)
    {
        chunk->SetIsBorderChunk(false);
        chunk->BuildExternal();
    }
}

BlockID *ChunkManager::GetBlockMemory(uint64_t chunk_id)
{
    // Check for free memory blocks
    for (auto &memory : _block_memory)
    {
        if (!memory.in_use)
        {
            memory.in_use = true;
            memory.owner = chunk_id;
            return memory.blocks;
        }
    }

    // No free memory blocks; create new one
    BlockMemory new_memory {
        .blocks = (BlockID *)malloc(BLOCKS_IN_CHUNK * sizeof(BlockID)),
        .in_use = true,
        .owner = chunk_id
    };
    _block_memory.push_back(new_memory);
    return new_memory.blocks;
}

void ChunkManager::ReuseBlockMemory(uint64_t chunk_id)
{
    for (auto &memory : _block_memory)
    {
        if (memory.owner == chunk_id)
        {
            memory.in_use = false;
            return;
        }
    }
}

std::shared_ptr<Chunk> ChunkManager::GetChunk(glm::ivec3 chunk_coords)
{
    auto chunk_id = ChunkCoordsToID(chunk_coords);
    if (_chunks.contains(chunk_id))
        return _chunks.at(chunk_id);
    else
        return nullptr;
}

std::unordered_map<uint64_t, std::shared_ptr<Chunk>> &ChunkManager::GetChunks()
{
    return _chunks;
}

ChunkWorkerPool *ChunkManager::GetWorkerPool()
{
    return _worker_pool;
}

int ChunkManager::GetLoadedChunkCount()
{
    return _loaded_chunk_count;
}

void ChunkManager::WriteAllChunksToDisk()
{
    for (auto &[chunk_id, chunk] : _chunks)
        WriteChunkToDisk(chunk->GetFilePath(), chunk->GetBlocks());
}
