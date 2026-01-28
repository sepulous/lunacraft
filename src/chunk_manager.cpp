
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

void ChunkManager::Init(int moon_id, MoonSettings moon_settings)
{
    _moon_id = moon_id;

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

ChunkManager::~ChunkManager()
{
    glDeleteTextures(1, &_texture_atlas);
}

void ChunkManager::UploadReadyChunks()
{
    for (auto it = _chunks.begin(); it != _chunks.end(); ++it)
    {
        if (it->second.GetState() == ChunkState::READY_TO_UPLOAD)
        {
            it->second.UploadVertices(); // Sets chunk state to RENDERABLE
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
        Chunk &chunk = it->second;
        if (!chunk.IsBorderChunk() && chunk.GetState() == ChunkState::RENDERABLE) // Chunks that become border chunks are RENDERABLE, but shouldn't be rendered
        {
            glm::ivec3 chunk_coords = chunk.GetCoords();
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
                chunk.RenderOpaques();
                visible_chunks.push_back(&chunk);
            }
        }
    }

    // Render transparent blocks
    for (Chunk *chunk : visible_chunks)
    {
        chunk->RenderTransparents();
    }
}

void ChunkManager::CreateInitialPatch(glm::ivec3 player_chunk, int render_distance)
{
    for (int x = player_chunk.x - render_distance - 1; x <= player_chunk.x + render_distance + 1; x++)
    {
        for (int z = player_chunk.z - render_distance - 1; z <= player_chunk.z + render_distance + 1; z++)
        {
            bool is_border_chunk = x == player_chunk.x - render_distance - 1
                                || x == player_chunk.x + render_distance + 1
                                || z == player_chunk.z + render_distance + 1
                                || z == player_chunk.z - render_distance - 1;

            glm::ivec3 chunk_coords{x, 0, z};
            uint64_t chunk_id = ChunkCoordsToID(chunk_coords);
            auto [it, success] = _chunks.try_emplace(chunk_id, chunk_coords, is_border_chunk, &_worker_pool);
            it->second.Build();
        }
    }
}

void ChunkManager::MoveChunkPatch(glm::ivec3 player_chunk, int render_distance)
{
    // Remove all chunks outside the patch + border
    for (auto it = _chunks.begin(); it != _chunks.end(); )
    {
        glm::ivec3 coords = it->second.GetCoords();
        bool outside_patch = coords.x < player_chunk.x - render_distance - 1
                          || coords.x > player_chunk.x + render_distance + 1
                          || coords.z < player_chunk.z - render_distance - 1
                          || coords.z > player_chunk.z + render_distance + 1;

        if (outside_patch)
        {
            it = _chunks.erase(it);
            _loaded_chunk_count--;
        }
        else
        {
            ++it;
        }
    }

    // Update patch border
    for (int x = player_chunk.x - render_distance - 1; x <= player_chunk.x + render_distance + 1; x++)
    {
        for (int z = player_chunk.z - render_distance - 1; z <= player_chunk.z + render_distance + 1; z++)
        {
            bool is_border_chunk = x == player_chunk.x - render_distance - 1
                                || x == player_chunk.x + render_distance + 1
                                || z == player_chunk.z + render_distance + 1
                                || z == player_chunk.z - render_distance - 1;

            if (is_border_chunk)
            {
                uint64_t chunk_id = ChunkCoordsToID({x, 0, z});
                if (_chunks.contains(chunk_id)) // Update previously-non-border chunks that are now on the border
                {
                    Chunk &chunk = _chunks.at(chunk_id);
                    if (!chunk.IsBorderChunk())
                        chunk.SetIsBorderChunk(true);
                }
                else // Create new border chunk
                {
                    auto [it, success] = _chunks.try_emplace(chunk_id, glm::ivec3{x, 0, z}, true, &_worker_pool);
                    it->second.Build(); // Is a border chunk, so BuildExternal() must be called later
                }
            }
        }
    }

    // Add new chunks (update border chunks that now fall within the patch)
    for (int x = player_chunk.x - render_distance; x <= player_chunk.x + render_distance; x++)
    {
        for (int z = player_chunk.z - render_distance; z <= player_chunk.z + render_distance; z++)
        {
            uint64_t chunk_id = ChunkCoordsToID({x, 0, z});
            Chunk &chunk = _chunks.at(chunk_id);
            if (chunk.IsBorderChunk())
            {
                chunk.SetIsBorderChunk(false);
                chunk.BuildExternal();
            }

            // NOTE: There may be another case: there's no border chunk to convert, so we really have to create a new non-border chunk
        }
    }
}

std::unordered_map<uint64_t, Chunk> &ChunkManager::GetChunks()
{
    return _chunks;
}

int ChunkManager::GetLoadedChunkCount()
{
    return _loaded_chunk_count;
}
