
#include <unordered_map>
#include <thread>

using namespace std::chrono_literals;

#include <glm/glm.hpp>

#include "chunk.h"
#include "shader.h"
#include "constants.h"
#include "helpers.h"
#include "block.h"
#include "mesher.h"
#include "moon.h"
#include "storage.h"
#include "chunk_generation.h"

//
// Chunk
//

Chunk::Chunk(glm::ivec3 coords, bool is_border_chunk, ChunkManager *chunk_manager)
{
    _is_border_chunk = is_border_chunk;
    _coords = coords;
    _blocks = chunk_manager->GetBlockMemory(ChunkCoordsToID(coords));
    _chunk_manager = chunk_manager;

    // Opaques
    glGenVertexArrays(1, &_opaque_vao);
    glBindVertexArray(_opaque_vao);

    glGenBuffers(1, &_opaque_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _opaque_vbo);

    glGenBuffers(1, &_opaque_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _opaque_ebo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    // Transparents
    glGenVertexArrays(1, &_transparent_vao);
    glBindVertexArray(_transparent_vao);

    glGenBuffers(1, &_transparent_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _transparent_vbo);

    glGenBuffers(1, &_transparent_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _transparent_ebo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);
}

Chunk::~Chunk()
{
    glDeleteVertexArrays(1, &_opaque_vao);
    glDeleteVertexArrays(1, &_transparent_vao);
    glDeleteBuffers(1, &_opaque_vbo);
    glDeleteBuffers(1, &_transparent_vbo);
    glDeleteBuffers(1, &_opaque_ebo);
    glDeleteBuffers(1, &_transparent_ebo);
}

ChunkState Chunk::GetState()
{
    return _state.load(std::memory_order::acquire);
}

void Chunk::SetIsBorderChunk(bool status)
{
    _is_border_chunk = status;
}

bool Chunk::IsBorderChunk()
{
    return _is_border_chunk;
}

bool Chunk::HasUploadedVertices()
{
    return _has_uploaded_vertices;
}

uint64_t Chunk::GetID()
{
    return ChunkCoordsToID(_coords);
}

glm::ivec3 Chunk::GetCoords()
{
    return _coords;
}

BlockID *Chunk::GetBlocks()
{
    return _blocks;
}

const Lightmap &Chunk::GetLightmap() const
{
    return _lightmap;
}

std::filesystem::path Chunk::GetFilePath()
{
    auto chunk_id = ChunkCoordsToID(_coords);
    return Storage::MOONS / (std::string("moon") + std::to_string(Moon::GetCurrentMoon()->GetID())) / "chunks" / (std::to_string(chunk_id) + ".chunk");
}

// Must only be called from the main thread!
void Chunk::Pin()
{
    _pins.fetch_add(1, std::memory_order_relaxed);
}

void Chunk::PinNeighbors()
{
    auto neighbors = _chunk_manager->GetNeighbors(_coords);
    for (auto &neighbor : neighbors)
        neighbor->Pin();
}

void Chunk::Unpin()
{
    _pins.fetch_sub(1, std::memory_order_release);
}

void Chunk::UnpinNeighbors()
{
    auto neighbors = _chunk_manager->GetNeighbors(_coords);
    for (auto &neighbor : neighbors)
        neighbor->Unpin();
}

int Chunk::GetPinCount()
{
    return _pins.load(std::memory_order_acquire);
}

void Chunk::MarkForDelete()
{
    _marked_for_delete.store(true, std::memory_order_relaxed);
}

bool Chunk::IsMarkedForDelete()
{
    return _marked_for_delete.load(std::memory_order_relaxed);
}

//
// Upload chunk's vertex data to the GPU.
//
// Chunk::GLInit must be called before this.
//
// This must be called before rendering the chunk, and after it has been fully built.
// See Chunk::Build for more information.
//
void Chunk::UploadVertices()
{
    // Opaques
    glBindBuffer(GL_ARRAY_BUFFER, _opaque_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _opaque_ebo);
    if (_opaque_vertices.size() <= _reserved_opaque_vertex_count)
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, _opaque_vertices.size() * sizeof(BlockVertex), _opaque_vertices.data());
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, _opaque_indices.size() * sizeof(uint16_t), _opaque_indices.data());
    }
    else
    {
        glBufferData(GL_ARRAY_BUFFER, _opaque_vertices.size() * sizeof(BlockVertex), _opaque_vertices.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _opaque_indices.size() * sizeof(uint16_t), _opaque_indices.data(), GL_STATIC_DRAW);
        _reserved_opaque_vertex_count = _opaque_vertices.size();
    }

    // Transparents
    glBindBuffer(GL_ARRAY_BUFFER, _transparent_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _transparent_ebo);
    if (_transparent_vertices.size() <= _reserved_transparent_vertex_count)
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, _transparent_vertices.size() * sizeof(BlockVertex), _transparent_vertices.data());
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, _transparent_indices.size() * sizeof(uint16_t), _transparent_indices.data());
    }
    else
    {
        glBufferData(GL_ARRAY_BUFFER, _transparent_vertices.size() * sizeof(BlockVertex), _transparent_vertices.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _transparent_indices.size() * sizeof(uint16_t), _transparent_indices.data(), GL_STATIC_DRAW);
        _reserved_transparent_vertex_count = _transparent_vertices.size();
    }

    _has_uploaded_vertices = true;

    SetState(ChunkState::RENDERABLE);
}

//
// Render chunk's opaque vertices.
//
// This should be called after Chunk::UploadVertices and before Chunk::RenderTransparents.
//
void Chunk::RenderOpaques()
{
    glBindVertexArray(_opaque_vao);
    glDrawElements(GL_TRIANGLES, _opaque_indices.size(), GL_UNSIGNED_SHORT, 0);
}

//
// Render chunk's transparent vertices.
//
// This should be called after Chunk::UploadVertices and Chunk::RenderOpaques.
//
void Chunk::RenderTransparents()
{
    glBindVertexArray(_transparent_vao);
    glDrawElements(GL_TRIANGLES, _transparent_indices.size(), GL_UNSIGNED_SHORT, 0);
}

void Chunk::SetState(ChunkState state)
{
    _state.store(state, std::memory_order::release);
}

//
// Generate block data, or load from disk.
//
void Chunk::LoadBlocks()
{
    // Update state
    SetState(ChunkState::LOADING_BLOCKS);

    // Load blocks
    auto chunk_file_path = GetFilePath();
    if (std::filesystem::exists(chunk_file_path))
        LoadChunkFromDisk(chunk_file_path, _blocks);
    else
        GenerateChunk(_blocks, _coords.x, _coords.z, Moon::GetCurrentMoon()->GetSettings());
}

//
// Build lightmap using internal block data.
//
void Chunk::BuildLightmapInternal()
{
    // Update state
    SetState(ChunkState::LIGHT_INTERNAL);

    constexpr int H  = WORLD_HEIGHT_LIMIT;
    constexpr int S  = CHUNK_SIZE;
    constexpr int SZ = H;      // stride for z++
    constexpr int SX = H * S;  // stride for x++
    constexpr int SY = 1;      // stride for y++

    std::vector<uint32_t> queue;
    queue.reserve(BLOCKS_IN_CHUNK);

    for (int x = 0; x < S; x++)
    {
        int x_base = x * SX;

        for (int z = 0; z < S; z++)
        {
            int column_base = x_base + z * SZ;

            uint8_t skylight = 9;

            // Start at top of column
            uint32_t idx = column_base + (H - 1);

            for (int y = H - 1; y >= 0; y--, idx--)
            {
                BlockID &block = _blocks[idx];

                if (skylight != 0 && BlockIsOpaque(block))
                    skylight = 0;

                if (skylight != 0)
                {
                    _lightmap.SetSkyLevel(9, idx);
                    queue.push_back(idx);
                }

                if (block == BlockID::light)
                {
                    // Y+1
                    if (y < H - 1 && !BlockIsOpaque(_blocks[idx + 1]))
                    {
                        _lightmap.SetBlockLevel(9, idx + 1);
                        queue.push_back(idx + 1);
                    }

                    // Y-1
                    if (y > 0 && !BlockIsOpaque(_blocks[idx - 1]))
                    {
                        _lightmap.SetBlockLevel(9, idx - 1);
                        queue.push_back(idx - 1);
                    }

                    // Z+1
                    if (z < S - 1 && !BlockIsOpaque(_blocks[idx + SZ]))
                    {
                        _lightmap.SetBlockLevel(9, idx + SZ);
                        queue.push_back(idx + SZ);
                    }

                    // Z-1
                    if (z > 0 && !BlockIsOpaque(_blocks[idx - SZ]))
                    {
                        _lightmap.SetBlockLevel(9, idx - SZ);
                        queue.push_back(idx - SZ);
                    }

                    // X+1
                    if (x < S - 1 && !BlockIsOpaque(_blocks[idx + SX]))
                    {
                        _lightmap.SetBlockLevel(9, idx + SX);
                        queue.push_back(idx + SX);
                    }

                    // X-1
                    if (x > 0 && !BlockIsOpaque(_blocks[idx - SX]))
                    {
                        _lightmap.SetBlockLevel(9, idx - SX);
                        queue.push_back(idx - SX);
                    }
                }
            }
        }
    }

    size_t head = 0;
    while (head < queue.size())
    {
        uint32_t idx = queue[head++];

        uint8_t sky = _lightmap.GetSkyLevel(idx);
        uint8_t sky_prop = sky == 0 ? 0 : sky - 1;
        uint8_t block = _lightmap.GetBlockLevel(idx);
        uint8_t block_prop = block == 0 ? 0 : block - 1;

        int y = idx & 127;
        int z = (idx >> 7) & 31;
        int x = idx >> 12;

        if (y > 0)
        {
            uint32_t n = idx - 1;
            bool pushed = false;
            
            if (sky_prop > _lightmap.GetSkyLevel(n))
            {
                _lightmap.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > _lightmap.GetBlockLevel(n))
            {
                _lightmap.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (y < 127)
        {
            uint32_t n = idx + 1;
            bool pushed = false;
            
            if (sky_prop > _lightmap.GetSkyLevel(n))
            {
                _lightmap.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > _lightmap.GetBlockLevel(n))
            {
                _lightmap.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (z > 0)
        {
            uint32_t n = idx - SZ;
            bool pushed = false;
            
            if (sky_prop > _lightmap.GetSkyLevel(n))
            {
                _lightmap.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > _lightmap.GetBlockLevel(n))
            {
                _lightmap.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (z < 31)
        {
            uint32_t n = idx + SZ;
            bool pushed = false;
            
            if (sky_prop > _lightmap.GetSkyLevel(n))
            {
                _lightmap.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > _lightmap.GetBlockLevel(n))
            {
                _lightmap.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (x > 0)
        {
            uint32_t n = idx - SX;
            bool pushed = false;
            
            if (sky_prop > _lightmap.GetSkyLevel(n))
            {
                _lightmap.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > _lightmap.GetBlockLevel(n))
            {
                _lightmap.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (x < 31)
        {
            uint32_t n = idx + SX;
            bool pushed = false;
            
            if (sky_prop > _lightmap.GetSkyLevel(n))
            {
                _lightmap.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > _lightmap.GetBlockLevel(n))
            {
                _lightmap.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }
    }

    SetState(ChunkState::INTERNAL_DONE);
}

//
// Incorporate neighbor lightmaps.
//
// Requires all neighbors.
//
void Chunk::BuildLightmapExternal()
{
    SetState(ChunkState::LIGHT_EXTERNAL);

    constexpr int H  = WORLD_HEIGHT_LIMIT;
    constexpr int S  = CHUNK_SIZE;
    constexpr int SZ = H;      // stride for z++
    constexpr int SX = H * S;  // stride for x++
    constexpr int SY = 1;      // stride for y++

    std::vector<uint32_t> queue;
    queue.reserve(BLOCKS_IN_CHUNK);

    //
    // Initial fill
    //
    auto neighbors = _chunk_manager->GetNeighbors(_coords);
    for (auto &neighbor : neighbors)
    {
        auto neighbor_chunk_coords = neighbor->GetCoords();
        const Lightmap &neighbor_lightmap = neighbor->GetLightmap();
        uint32_t idx;
        glm::ivec3 neighbor_block_coords;
        for (int xz = 0; xz < CHUNK_SIZE; xz++)
        {
            for (int y = 0; y < WORLD_HEIGHT_LIMIT; y++)
            {
                // Get coordinates for block pair
                if (neighbor_chunk_coords.z == _coords.z + 1) // Front neighbor
                {
                    idx = GetChunkIndex(xz, y, CHUNK_SIZE - 1);
                    neighbor_block_coords = {xz, y, 0};
                }
                else if (neighbor_chunk_coords.x == _coords.x + 1) // Right neighbor
                {
                    idx = GetChunkIndex(CHUNK_SIZE - 1, y, xz);
                    neighbor_block_coords = {0, y, xz};
                }
                else if (neighbor_chunk_coords.z == _coords.z - 1) // Back neighbor
                {
                    idx = GetChunkIndex(xz, y, 0);
                    neighbor_block_coords = {xz, y, CHUNK_SIZE - 1};
                }
                else // Left neighbor
                {
                    idx = GetChunkIndex(0, y, xz);
                    neighbor_block_coords = {CHUNK_SIZE - 1, y, xz};
                }

                BlockID my_block = _blocks[idx];
                if (!BlockIsOpaque(my_block))
                {
                    BlockID neighbor_block = neighbor->GetBlocks()[GetChunkIndex(neighbor_block_coords)];

                    // Sky light
                    uint8_t neighbor_sky_light = neighbor_lightmap.GetSkyLevel(neighbor_block_coords);
                    if (neighbor_sky_light > 0)
                    {
                        uint8_t propagated = neighbor_sky_light - 1;
                        if (propagated > _lightmap.GetSkyLevel(idx))
                        {
                            _lightmap.SetSkyLevel(propagated, idx);
                            queue.push_back(idx);
                        }
                    }

                    // Block light
                    if (neighbor_block == BlockID::light)
                    {
                        _lightmap.SetBlockLevel(9, idx);
                        queue.push_back(idx);
                    }
                    else
                    {
                        uint8_t neighbor_block_light = neighbor_lightmap.GetBlockLevel(neighbor_block_coords);
                        if (neighbor_block_light > 0 && !BlockIsOpaque(my_block))
                        {
                            uint8_t propagated = neighbor_block_light - 1;
                            if (propagated > _lightmap.GetBlockLevel(idx))
                            {
                                _lightmap.SetBlockLevel(propagated, idx);
                                queue.push_back(idx);
                            }
                        }
                    }
                }
            }
        }
    }

    //
    // Expansion
    //

    size_t head = 0;
    while (head < queue.size())
    {
        uint32_t idx = queue[head++];

        uint8_t sky = _lightmap.GetSkyLevel(idx);
        uint8_t sky_prop = sky == 0 ? 0 : sky - 1;
        uint8_t block = _lightmap.GetBlockLevel(idx);
        uint8_t block_prop = block == 0 ? 0 : block - 1;

        int y = idx & 127;
        int z = (idx >> 7) & 31;
        int x = idx >> 12;

        if (y > 0)
        {
            uint32_t n = idx - 1;
            bool pushed = false;
            
            if (sky_prop > _lightmap.GetSkyLevel(n))
            {
                _lightmap.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > _lightmap.GetBlockLevel(n))
            {
                _lightmap.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (y < 127)
        {
            uint32_t n = idx + 1;
            bool pushed = false;
            
            if (sky_prop > _lightmap.GetSkyLevel(n))
            {
                _lightmap.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > _lightmap.GetBlockLevel(n))
            {
                _lightmap.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (z > 0)
        {
            uint32_t n = idx - SZ;
            bool pushed = false;
            
            if (sky_prop > _lightmap.GetSkyLevel(n))
            {
                _lightmap.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > _lightmap.GetBlockLevel(n))
            {
                _lightmap.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (z < 31)
        {
            uint32_t n = idx + SZ;
            bool pushed = false;
            
            if (sky_prop > _lightmap.GetSkyLevel(n))
            {
                _lightmap.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > _lightmap.GetBlockLevel(n))
            {
                _lightmap.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (x > 0)
        {
            uint32_t n = idx - SX;
            bool pushed = false;
            
            if (sky_prop > _lightmap.GetSkyLevel(n))
            {
                _lightmap.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > _lightmap.GetBlockLevel(n))
            {
                _lightmap.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (x < 31)
        {
            uint32_t n = idx + SX;
            bool pushed = false;
            
            if (sky_prop > _lightmap.GetSkyLevel(n))
            {
                _lightmap.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > _lightmap.GetBlockLevel(n))
            {
                _lightmap.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }
    }
}

void Chunk::UpdateVertexLighting()
{
    SetState(ChunkState::UPDATING_VERTEX_LIGHTING);

    auto neighbors = _chunk_manager->GetNeighbors(_coords);

    // We need to determine the global voxel position of the base of the quad (the inverse of what we do in BuildVertices), but some
    // vertex positions include offsets (du and dv), so we can't always do this directly. But we submit vertices in groups of six,
    // and for the first one in each group we always have vertex.position == quad.base_coords, so we can use the first one to determine
    // the lighting and apply it to the whole group.
    std::vector<BlockVertex> *vertex_lists[2] = {&_opaque_vertices, &_transparent_vertices};
    for (auto vertices : vertex_lists)
    {
        for (int i = 0; i < vertices->size(); i += 4)
        {
            auto &first = vertices->at(i);

            glm::ivec3 voxel_g;
            if (first.face_normal.x != 0)
                voxel_g = {first.position.x + first.face_normal.x*0.5f, first.position.y + 0.5f, first.position.z + 0.5f};
            else if (first.face_normal.y != 0)
                voxel_g = {first.position.x + 0.5f, first.position.y + first.face_normal.y*0.5f, first.position.z + 0.5f};
            else
                voxel_g = {first.position.x + 0.5f, first.position.y + 0.5f, first.position.z + first.face_normal.z*0.5f};

            glm::vec3 light;

            float world_time = Moon::GetCurrentMoon()->GetWorldTime();
            float snapped_world_time = (((int)world_time % 330) / 30) * 30; // Snap world time to beginning of phase so all chunks in the same phase agree on ambient_light
            float sin_world_time = glm::sin((snapped_world_time + SECONDS_PER_LIGHT_PHASE) * (2 * 3.14159f / (LIGHT_PHASES * SECONDS_PER_LIGHT_PHASE))); // The offset initializes moon on Phase 1
            glm::vec3 sunlight_direction = Moon::GetCurrentMoon()->GetSunlightDirection();
            float ambient_light = 0.5f * sin_world_time;
            if (ambient_light < 0)
                ambient_light *= -0.5f;
            float sunlight_factor = ambient_light + 0.5f;

            float dot = glm::dot(sunlight_direction, first.face_normal);
            if (dot < 0)
                dot = 0;

            uint8_t _sky_light, _block_light;
            glm::ivec3 light_sample_voxel_coords = voxel_g - glm::ivec3(first.face_normal);
            glm::ivec3 light_sample_chunk_coords = VoxelToChunk(light_sample_voxel_coords);
            if (light_sample_chunk_coords == _coords)
            {
                _sky_light = _lightmap.GetSkyLevel(GlobalToLocalVoxel(light_sample_voxel_coords));
                _block_light = _lightmap.GetBlockLevel(GlobalToLocalVoxel(light_sample_voxel_coords));
            }
            else
            {
                for (auto &neighbor : neighbors)
                {
                    if (neighbor->GetCoords() == light_sample_chunk_coords)
                    {
                        _sky_light = neighbor->GetLightmap().GetSkyLevel(GlobalToLocalVoxel(light_sample_voxel_coords));
                        _block_light = neighbor->GetLightmap().GetBlockLevel(GlobalToLocalVoxel(light_sample_voxel_coords));
                        break;
                    }
                }
            }
            float sky_light = (float)_sky_light * (100.0f / 9.0f);      // Apparently Charlie's light values were in [0, 100]. Mine are in [0, 9], 
            float block_light = (float)_block_light * (100.0f / 9.0f);  // so let's scale to [0, 100] so his code works as-is

            float corrected_sky_light = (sky_light * ambient_light + (1.0 - ambient_light) * sky_light * dot) * sunlight_factor;
            float scaled_sky_light = 0;
            if (corrected_sky_light != 0)
                scaled_sky_light = ((corrected_sky_light / 100.0) * 68.0) + 32;

            if (sin_world_time > 0)
            {
                light = glm::vec3(glm::max(scaled_sky_light, block_light) / 100.0f);
            }
            else
            {
                float red_green = glm::clamp(block_light / 100.0f + scaled_sky_light * sunlight_factor * 0.01f, 0.0f, 1.0f);
                float blue = glm::clamp(block_light / 100.0f + scaled_sky_light * 0.01f, 0.0f, 1.0f);
                light = {red_green, red_green, blue};
            }

            vertices->at(i).light = light;
            vertices->at(i+1).light = light;
            vertices->at(i+2).light = light;
            vertices->at(i+3).light = light;
        }
    }

    UnpinNeighbors();

    SetState(ChunkState::READY_TO_UPLOAD);
}

void Chunk::BuildVertices()
{
    SetState(ChunkState::BUILDING_VERTICES);

    auto neighbors = _chunk_manager->GetNeighbors(_coords);

    std::vector<BlockQuad> quads = GreedyMesh(_blocks, neighbors);

    auto tile_origins = GetAtlasTileOrigins();

    _opaque_vertices.clear();
    _opaque_indices.clear();
    _transparent_vertices.clear();
    _transparent_indices.clear();
    int opaque_index_base = 0;
    int transparent_index_base = 0;
    for (const BlockQuad &quad : quads)
    {
        // Determine vertex normal
        glm::vec3 normal = glm::normalize(glm::cross(quad.du, quad.dv));
        if (quad.back_face)
            normal = -normal;

        // Determine texture atlas tile
        int side = normal.y > 0 ? 2 : (normal.y < 0 ? 0 : 1);
        glm::vec2 tile_origin = tile_origins[quad.block][side];

        // Determine global base vertex position
        glm::vec3 base_pos;
        if (normal.y != 0)
            base_pos = {quad.base_coords.x + _coords.x*CHUNK_SIZE - 0.5f, quad.base_coords.y + (quad.back_face ? 0.5f : 0.5f), quad.base_coords.z + _coords.z*CHUNK_SIZE -0.5f};
        else
            base_pos = {quad.base_coords.x + _coords.x*CHUNK_SIZE - 0.5f, quad.base_coords.y - 0.5f, quad.base_coords.z + _coords.z*CHUNK_SIZE - 0.5f};

        // Determine texture tiling repeats
        int quad_width = glm::length(quad.du);
        int quad_height = glm::length(quad.dv);

        //
        // Lighting
        //
        // These calculations were reverse engineered from the latest version of the original game (v2.01)
        //

        glm::vec2 light;

        float world_time = Moon::GetCurrentMoon()->GetWorldTime();
        float snapped_world_time = (((int)world_time % 330) / 30) * 30; // Snap world time to beginning of phase so all chunks in the same phase agree on ambient_light
        float sin_world_time = glm::sin((snapped_world_time + SECONDS_PER_LIGHT_PHASE) * (2 * 3.14159f / (LIGHT_PHASES * SECONDS_PER_LIGHT_PHASE))); // The offset initializes moon on Phase 1
        glm::vec3 sunlight_direction = Moon::GetCurrentMoon()->GetSunlightDirection();
        float ambient_light = 0.5f * sin_world_time;
        if (ambient_light < 0)
            ambient_light *= -0.5f;
        float sunlight_factor = ambient_light + 0.5f;

        float dot = glm::dot(sunlight_direction, normal);
        if (dot < 0)
            dot = 0;

        uint8_t _sky_light, _block_light;
        glm::ivec3 light_sample_voxel_coords = LocalToGlobalVoxel(quad.base_coords, _coords) - glm::ivec3(normal);
        glm::ivec3 light_sample_chunk_coords = VoxelToChunk(light_sample_voxel_coords);
        if (light_sample_chunk_coords == _coords)
        {
            _sky_light = _lightmap.GetSkyLevel(GlobalToLocalVoxel(light_sample_voxel_coords));
            _block_light = _lightmap.GetBlockLevel(GlobalToLocalVoxel(light_sample_voxel_coords));
        }
        else
        {
            for (auto &neighbor : neighbors)
            {
                if (neighbor->GetCoords() == light_sample_chunk_coords)
                {
                    _sky_light = neighbor->GetLightmap().GetSkyLevel(GlobalToLocalVoxel(light_sample_voxel_coords));
                    _block_light = neighbor->GetLightmap().GetBlockLevel(GlobalToLocalVoxel(light_sample_voxel_coords));
                    break;
                }
            }
        }
        float sky_light = (float)_sky_light * (100.0f / 9.0f);      // Apparently Charlie's light values were in [0, 100]. Mine are in [0, 9], 
        float block_light = (float)_block_light * (100.0f / 9.0f);  // so let's scale to [0, 100] so his code works as-is

        float corrected_sky_light = (sky_light * ambient_light + (1.0 - ambient_light) * sky_light * dot) * sunlight_factor;
        float scaled_sky_light;
        if (corrected_sky_light != 0)
            scaled_sky_light = ((corrected_sky_light / 100.0) * 68.0) + 32;
        else
            scaled_sky_light = 0;

        if (sin_world_time > 0)
        {
            light = glm::vec2(glm::max(scaled_sky_light, block_light) / 100.0f);
        }
        else
        {
            float red_green = glm::clamp(block_light / 100.0f + scaled_sky_light * sunlight_factor * 0.01f, 0.0f, 1.0f);
            float blue = glm::clamp(block_light / 100.0f + scaled_sky_light * 0.01f, 0.0f, 1.0f);
            light = {red_green, blue};
        }

        // Push vertices
        auto &vertices = BlockIsOpaque(quad.block) ? _opaque_vertices : _transparent_vertices;
        vertices.emplace_back(base_pos,                     glm::vec4{0,          0,           tile_origin}, normal, light);
        vertices.emplace_back(base_pos + quad.dv,           glm::vec4{0,          quad_height, tile_origin}, normal, light);
        vertices.emplace_back(base_pos + quad.dv + quad.du, glm::vec4{quad_width, quad_height, tile_origin}, normal, light);
        vertices.emplace_back(base_pos + quad.du,           glm::vec4{quad_width, 0,           tile_origin}, normal, light);

        // Push indices
        auto &indices = BlockIsOpaque(quad.block) ? _opaque_indices : _transparent_indices;
        auto &index_base = BlockIsOpaque(quad.block) ? opaque_index_base : transparent_index_base;
        if (!quad.back_face)
        {
            indices.push_back(index_base + 0);
            indices.push_back(index_base + 1);
            indices.push_back(index_base + 2);
            indices.push_back(index_base + 2);
            indices.push_back(index_base + 3);
            indices.push_back(index_base + 0);
        }
        else
        {
            indices.push_back(index_base + 0);
            indices.push_back(index_base + 3);
            indices.push_back(index_base + 2);
            indices.push_back(index_base + 2);
            indices.push_back(index_base + 1);
            indices.push_back(index_base + 0);
        }
        index_base += 4;
    }

    UnpinNeighbors();

    SetState(ChunkState::READY_TO_UPLOAD);
}

//
// Lightmap
//

Lightmap::Lightmap()
{
    _sky_light = (uint8_t *)calloc(BLOCKS_IN_CHUNK, sizeof(uint8_t));
    _block_light = (uint8_t *)calloc(BLOCKS_IN_CHUNK, sizeof(uint8_t));
}

Lightmap::~Lightmap()
{
    free(_sky_light);
    free(_block_light);
}

uint8_t Lightmap::GetSkyLevel(glm::ivec3 coords) const
{
    uint8_t entry = _sky_light[GetChunkIndex(coords)];
    return (entry >> 4);
}

uint8_t Lightmap::GetSkyLevel(int x, int y, int z) const
{
    uint8_t entry = _sky_light[GetChunkIndex(x, y, z)];
    return (entry >> 4);
}

uint8_t Lightmap::GetSkyLevel(uint32_t idx) const
{
    uint8_t entry = _sky_light[idx];
    return (entry >> 4);
}

// Level must be in the range [0, 15]
void Lightmap::SetSkyLevel(uint8_t sky_level, glm::ivec3 coords)
{
    uint8_t entry = _sky_light[GetChunkIndex(coords.x, coords.y, coords.z)];
    _sky_light[GetChunkIndex(coords.x, coords.y, coords.z)] = (entry & 0x0F) | (sky_level << 4);
}

// Level must be in the range [0, 15]
void Lightmap::SetSkyLevel(uint8_t sky_level, int x, int y, int z)
{
    uint8_t entry = _sky_light[GetChunkIndex(x, y, z)];
    _sky_light[GetChunkIndex(x, y, z)] = (entry & 0x0F) | (sky_level << 4);
}

// Level must be in the range [0, 15]
void Lightmap::SetSkyLevel(uint8_t sky_level, uint32_t idx)
{
    uint8_t entry = _sky_light[idx];
    _sky_light[idx] = (entry & 0x0F) | (sky_level << 4);
}

uint8_t Lightmap::GetBlockLevel(glm::ivec3 coords) const
{
    uint8_t entry = _block_light[GetChunkIndex(coords.x, coords.y, coords.z)];
    return (entry & 0x0f);
}

uint8_t Lightmap::GetBlockLevel(int x, int y, int z) const
{
    uint8_t entry = _block_light[GetChunkIndex(x, y, z)];
    return (entry & 0x0f);
}

uint8_t Lightmap::GetBlockLevel(uint32_t idx) const
{
    uint8_t entry = _block_light[idx];
    return (entry & 0x0f);
}

// Level must be in the range [0, 15]
void Lightmap::SetBlockLevel(uint8_t block_level, glm::ivec3 coords)
{
    uint8_t entry = _block_light[GetChunkIndex(coords.x, coords.y, coords.z)];
    _block_light[GetChunkIndex(coords.x, coords.y, coords.z)] = (entry & 0xF0) | block_level;
}

// Level must be in the range [0, 15]
void Lightmap::SetBlockLevel(uint8_t block_level, int x, int y, int z)
{
    uint8_t entry = _block_light[GetChunkIndex(x, y, z)];
    _block_light[GetChunkIndex(x, y, z)] = (entry & 0xF0) | block_level;
}

// Level must be in the range [0, 15]
void Lightmap::SetBlockLevel(uint8_t block_level, uint32_t idx)
{
    uint8_t entry = _block_light[idx];
    _block_light[idx] = (entry & 0xF0) | block_level;
}
