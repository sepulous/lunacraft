
#include <filesystem>
#include <string>
#include <fstream>
#include <thread>
#include <stop_token>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image/stb_image.h>

#include "chunk_manager.h"
#include "chunk_gen.h"
#include "helpers.h"
#include "storage.h"

static void _ChunkWorker(std::stop_token, int, MoonSettings, BlockingQueue<ChunkTask>&, BlockingQueue<ChunkResult>&);

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

    // Start workers
    int num_workers = glm::max(1u, std::thread::hardware_concurrency() - 2);
    for (int i = 0; i < num_workers; i++)
        _workers.emplace_back(_ChunkWorker, moon_id, moon_settings, std::ref(_task_queue), std::ref(_result_queue));
}

ChunkManager::~ChunkManager()
{
    glDeleteTextures(1, &_texture_atlas);
}

void ChunkManager::QueueNewChunk(glm::ivec3 chunk_coords)
{
    uint64_t chunk_id = ChunkCoordsToID(chunk_coords);
    Chunk &chunk = _chunks[chunk_id];
    if (chunk.state == ChunkState::MISSING)
    {
        chunk.state = ChunkState::QUEUED;
        _task_queue.Push({ chunk_coords });
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
        if (it->second.state == ChunkState::UPLOADED)
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

void ChunkManager::BufferReadyChunks()
{
    ChunkResult result;
    while (_result_queue.TryPop(result))
    {
        uint64_t chunk_id = ChunkCoordsToID(result.coords);
        Chunk &chunk = _chunks[chunk_id];

        chunk.SetCoords(result.coords);
        chunk.SetBlocks(result.blocks);
        chunk.SetOpaqueVertices(result.opaque_vertices);
        chunk.SetTransparentVertices(result.transparent_vertices);
        chunk.BufferVertices();

        chunk.state = ChunkState::UPLOADED;
        _loaded_chunk_count++;
    }
}

void ChunkManager::RemoveDistantChunks(glm::ivec3 player_chunk, int render_distance)
{
    for (auto it = _chunks.begin(); it != _chunks.end(); )
    {
        glm::ivec3 coords = it->second.GetCoords();
        bool not_being_processed = it->second.state == ChunkState::UPLOADED;
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

static void _ChunkWorker(std::stop_token stoken, int moon_id, MoonSettings moon_settings, BlockingQueue<ChunkTask> &task_queue, BlockingQueue<ChunkResult> &result_queue)
{
    while (!stoken.stop_requested())
    {
        ChunkTask task;
        if (!task_queue.Pop(task, stoken))
            continue;

        BlockArray blocks;
        Lightmap lightmap;
        std::vector<BlockVertex> opaque_vertices;
        std::vector<BlockVertex> transparent_vertices;
        opaque_vertices.reserve(8192);
        transparent_vertices.reserve(128);

        // TODO: Let's rethink the need to create this data here. I'd like to make BuildLightmaps and BuildChunkVertices methods of Chunk, but
        //       I have to be able to work with a chunk object to do that, and I shyed away from that for some reason.

        // Load blocks
        uint64_t chunk_id = ChunkCoordsToID(task.coords);
        std::filesystem::path chunk_file_path = Storage::MOON_DIR / (std::string("moon") + std::to_string(moon_id)) / "chunks" / (std::to_string(chunk_id) + ".chunk");
        if (std::filesystem::exists(chunk_file_path))
        {
            std::ifstream chunk_file(chunk_file_path, std::ios::binary);
            chunk_file.read(reinterpret_cast<char *>(blocks.data()), BLOCKS_IN_CHUNK * sizeof(BlockID));
            chunk_file.close();
        }
        else
        {
            GenerateChunk(blocks.data(), task.coords.x, task.coords.z, moon_settings.seed);

            std::ofstream chunk_file(chunk_file_path, std::ios::binary);
            chunk_file.write(reinterpret_cast<char *>(blocks.data()), BLOCKS_IN_CHUNK * sizeof(BlockID));
            chunk_file.close();
        }

        BuildLightmap(blocks.data(), lightmap);
        BuildChunkVertices(blocks.data(), task.coords, opaque_vertices, transparent_vertices, lightmap);

        result_queue.Push({task.coords, blocks, std::move(opaque_vertices), std::move(transparent_vertices)});
    }
}
