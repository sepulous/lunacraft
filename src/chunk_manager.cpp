
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

void ChunkManager::QueueNewChunk(glm::ivec3 chunk_coords)
{
    uint64_t chunk_id = ChunkCoordsToID(chunk_coords);
    auto [it, is_new_chunk] = _chunks.try_emplace(chunk_id, chunk_coords, false, &_worker_pool);
    if (is_new_chunk)
    {
        Chunk &chunk = it->second;
        chunk.Build();
    }
}

void ChunkManager::BufferReadyChunks()
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
        if (it->second.GetState() == ChunkState::RENDERABLE)
        {
            glm::ivec3 chunk_coords = it->second.GetCoords();
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
                it->second.RenderOpaques();
                visible_chunks.push_back(&it->second);
            }
        }
    }

    // Render transparent blocks
    for (Chunk *chunk : visible_chunks)
    {
        chunk->RenderTransparents();
    }
}

void ChunkManager::RemoveDistantChunks(glm::ivec3 player_chunk, int render_distance)
{
    for (auto it = _chunks.begin(); it != _chunks.end(); )
    {
        glm::ivec3 coords = it->second.GetCoords();
        bool not_being_processed = it->second.GetState() == ChunkState::RENDERABLE;
        bool distant = coords.x < player_chunk.x - render_distance
                    || coords.x > player_chunk.x + render_distance
                    || coords.z < player_chunk.z - render_distance
                    || coords.z > player_chunk.z + render_distance;

        if (not_being_processed && distant)
        {
            it = _chunks.erase(it);
            _loaded_chunk_count--;
        }
        else
        {
            ++it;
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
