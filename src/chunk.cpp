
#include <unordered_map>
#include <thread>

using namespace std::chrono_literals;

#include <glm/glm.hpp>

#include "chunk.h"
#include "shader.h"
#include "constants.h"
#include "helpers.h"
#include "block.h"
#include "moon.h"
#include "storage.h"
#include "chunk_generation.h"

//
// Chunk
//

Chunk::Chunk(glm::ivec3 coords, bool is_border_chunk, ChunkManager *chunk_manager)
{
    is_border_chunk_ = is_border_chunk;
    coords_ = coords;
    blocks_ = chunk_manager->GetBlockMemory(ChunkCoordsToID(coords));
    chunk_manager_ = chunk_manager;

    // Opaques
    glGenVertexArrays(1, &opaque_vao_);
    glBindVertexArray(opaque_vao_);

    glGenBuffers(1, &opaque_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, opaque_vbo_);

    glGenBuffers(1, &opaque_ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, opaque_ebo_);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 12 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    // Transparents
    glGenVertexArrays(1, &transparent_vao_);
    glBindVertexArray(transparent_vao_);

    glGenBuffers(1, &transparent_vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, transparent_vbo_);

    glGenBuffers(1, &transparent_ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, transparent_ebo_);

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
    glDeleteVertexArrays(1, &opaque_vao_);
    glDeleteVertexArrays(1, &transparent_vao_);
    glDeleteBuffers(1, &opaque_vbo_);
    glDeleteBuffers(1, &transparent_vbo_);
    glDeleteBuffers(1, &opaque_ebo_);
    glDeleteBuffers(1, &transparent_ebo_);
}

ChunkState Chunk::GetState()
{
    return state_.load(std::memory_order::acquire);
}

void Chunk::SetIsBorderChunk(bool status)
{
    is_border_chunk_ = status;
}

bool Chunk::IsBorderChunk()
{
    return is_border_chunk_;
}

bool Chunk::HasUploadedVertices()
{
    return has_uploaded_vertices_;
}

uint64_t Chunk::GetID()
{
    return ChunkCoordsToID(coords_);
}

glm::ivec3 Chunk::GetCoords()
{
    return coords_;
}

BlockID *Chunk::GetBlocks()
{
    return blocks_;
}

const Lightmap &Chunk::GetLightmap() const
{
    return lightmap_;
}

std::filesystem::path Chunk::GetFilePath()
{
    auto chunk_id = ChunkCoordsToID(coords_);
    return Storage::MOONS / (std::string("moon") + std::to_string(Moon::GetCurrentMoon()->GetID())) / "chunks" / (std::to_string(chunk_id) + ".chunk");
}

// Must only be called from the main thread!
void Chunk::Pin()
{
    pins_.fetch_add(1, std::memory_order_relaxed);
}

void Chunk::PinNeighbors()
{
    auto neighbors = chunk_manager_->GetAdjacentNeighbors(coords_);
    for (auto &neighbor : neighbors)
        neighbor->Pin();
}

void Chunk::Unpin()
{
    pins_.fetch_sub(1, std::memory_order_release);
}

void Chunk::UnpinNeighbors()
{
    auto neighbors = chunk_manager_->GetAdjacentNeighbors(coords_);
    for (auto &neighbor : neighbors)
        neighbor->Unpin();
}

int Chunk::GetPinCount()
{
    return pins_.load(std::memory_order_acquire);
}

void Chunk::MarkForDelete()
{
    marked_for_delete_.store(true, std::memory_order_relaxed);
}

bool Chunk::IsMarkedForDelete()
{
    return marked_for_delete_.load(std::memory_order_relaxed);
}

void Chunk::MarkAsDirty()
{
    dirty_.store(true, std::memory_order_release);
}

void Chunk::MarkAsClean()
{
    dirty_.store(false, std::memory_order_release);
}

bool Chunk::IsDirty()
{
    return dirty_.load(std::memory_order_acquire);
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
    glBindBuffer(GL_ARRAY_BUFFER, opaque_vbo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, opaque_ebo_);
    if (opaque_vertices_.size() <= reserved_opaque_vertex_count_)
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, opaque_vertices_.size() * sizeof(BlockVertex), opaque_vertices_.data());
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, opaque_indices_.size() * sizeof(uint16_t), opaque_indices_.data());
    }
    else
    {
        glBufferData(GL_ARRAY_BUFFER, opaque_vertices_.size() * sizeof(BlockVertex), opaque_vertices_.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, opaque_indices_.size() * sizeof(uint16_t), opaque_indices_.data(), GL_STATIC_DRAW);
        reserved_opaque_vertex_count_ = opaque_vertices_.size();
    }

    // Transparents
    glBindBuffer(GL_ARRAY_BUFFER, transparent_vbo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, transparent_ebo_);
    if (transparent_vertices_.size() <= reserved_transparent_vertex_count_)
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, transparent_vertices_.size() * sizeof(BlockVertex), transparent_vertices_.data());
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, transparent_indices_.size() * sizeof(uint16_t), transparent_indices_.data());
    }
    else
    {
        glBufferData(GL_ARRAY_BUFFER, transparent_vertices_.size() * sizeof(BlockVertex), transparent_vertices_.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, transparent_indices_.size() * sizeof(uint16_t), transparent_indices_.data(), GL_STATIC_DRAW);
        reserved_transparent_vertex_count_ = transparent_vertices_.size();
    }

    has_uploaded_vertices_ = true;

    SetState(ChunkState::RENDERABLE);
}

//
// Render chunk's opaque vertices.
//
// This should be called after Chunk::UploadVertices and before Chunk::RenderTransparents.
//
void Chunk::RenderOpaques()
{
    glBindVertexArray(opaque_vao_);
    glDrawElements(GL_TRIANGLES, opaque_indices_.size(), GL_UNSIGNED_SHORT, 0);
}

//
// Render chunk's transparent vertices.
//
// This should be called after Chunk::UploadVertices and Chunk::RenderOpaques.
//
void Chunk::RenderTransparents()
{
    glBindVertexArray(transparent_vao_);
    glDrawElements(GL_TRIANGLES, transparent_indices_.size(), GL_UNSIGNED_SHORT, 0);
}

void Chunk::SetState(ChunkState state)
{
    state_.store(state, std::memory_order::release);
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
        LoadChunkFromDisk(chunk_file_path, blocks_);
    else
        GenerateChunk(blocks_, coords_.x, coords_.z, Moon::GetCurrentMoon()->GetSettings());
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

    // Right now I'm removing block light by completely rebuilding lightmaps, rather than just unpropagating it.
    // The latter is obviously much more efficient, but I need a non-annoying way to supply neighbor chunks with
    // the information they need to unpropagate.
    lightmap_.ClearBlockLight();

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
                BlockID &block = blocks_[idx];

                if (skylight != 0 && BlockIsOpaque(block))
                    skylight = 0;

                if (skylight != 0)
                {
                    lightmap_.SetSkyLevel(9, idx);
                    queue.push_back(idx);
                }

                if (block == BlockID::minilight)
                {
                    lightmap_.SetBlockLevel(9, idx);
                    queue.push_back(idx);
                }
                else if (block == BlockID::light)
                {
                    // Y+1
                    if (y < H - 1 && !BlockIsOpaque(blocks_[idx + 1]))
                    {
                        lightmap_.SetBlockLevel(9, idx + 1);
                        queue.push_back(idx + 1);
                    }

                    // Y-1
                    if (y > 0 && !BlockIsOpaque(blocks_[idx - 1]))
                    {
                        lightmap_.SetBlockLevel(9, idx - 1);
                        queue.push_back(idx - 1);
                    }

                    // Z+1
                    if (z < S - 1 && !BlockIsOpaque(blocks_[idx + SZ]))
                    {
                        lightmap_.SetBlockLevel(9, idx + SZ);
                        queue.push_back(idx + SZ);
                    }

                    // Z-1
                    if (z > 0 && !BlockIsOpaque(blocks_[idx - SZ]))
                    {
                        lightmap_.SetBlockLevel(9, idx - SZ);
                        queue.push_back(idx - SZ);
                    }

                    // X+1
                    if (x < S - 1 && !BlockIsOpaque(blocks_[idx + SX]))
                    {
                        lightmap_.SetBlockLevel(9, idx + SX);
                        queue.push_back(idx + SX);
                    }

                    // X-1
                    if (x > 0 && !BlockIsOpaque(blocks_[idx - SX]))
                    {
                        lightmap_.SetBlockLevel(9, idx - SX);
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

        uint8_t sky = lightmap_.GetSkyLevel(idx);
        uint8_t sky_prop = sky == 0 ? 0 : sky - 1;
        uint8_t block = lightmap_.GetBlockLevel(idx);
        uint8_t block_prop = block == 0 ? 0 : block - 1;

        int y = idx & 127;
        int z = (idx >> 7) & 31;
        int x = idx >> 12;

        if (y > 0)
        {
            uint32_t n = idx - 1;
            bool pushed = false;
            
            if (sky_prop > lightmap_.GetSkyLevel(n))
            {
                lightmap_.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > lightmap_.GetBlockLevel(n))
            {
                lightmap_.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (y < 127)
        {
            uint32_t n = idx + 1;

            if (block_prop > lightmap_.GetBlockLevel(n))
            {
                lightmap_.SetBlockLevel(block_prop, n);
                queue.push_back(n);
            }
        }

        if (z > 0)
        {
            uint32_t n = idx - SZ;
            bool pushed = false;
            
            if (sky_prop > lightmap_.GetSkyLevel(n))
            {
                lightmap_.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > lightmap_.GetBlockLevel(n))
            {
                lightmap_.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (z < 31)
        {
            uint32_t n = idx + SZ;
            bool pushed = false;
            
            if (sky_prop > lightmap_.GetSkyLevel(n))
            {
                lightmap_.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > lightmap_.GetBlockLevel(n))
            {
                lightmap_.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (x > 0)
        {
            uint32_t n = idx - SX;
            bool pushed = false;
            
            if (sky_prop > lightmap_.GetSkyLevel(n))
            {
                lightmap_.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > lightmap_.GetBlockLevel(n))
            {
                lightmap_.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (x < 31)
        {
            uint32_t n = idx + SX;
            bool pushed = false;
            
            if (sky_prop > lightmap_.GetSkyLevel(n))
            {
                lightmap_.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > lightmap_.GetBlockLevel(n))
            {
                lightmap_.SetBlockLevel(block_prop, n);
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
    auto neighbors = chunk_manager_->GetAdjacentNeighbors(coords_);
    for (auto &neighbor : neighbors)
    {
        auto neighbor_chunkcoords_ = neighbor->GetCoords();
        const Lightmap &neighborlightmap_ = neighbor->GetLightmap();
        uint32_t idx;
        glm::ivec3 neighbor_blockcoords_;
        for (int xz = 0; xz < CHUNK_SIZE; xz++)
        {
            for (int y = 0; y < WORLD_HEIGHT_LIMIT; y++)
            {
                // Get coordinates for block pair
                if (neighbor_chunkcoords_.z == coords_.z + 1) // Front neighbor
                {
                    idx = GetChunkIndex(xz, y, CHUNK_SIZE - 1);
                    neighbor_blockcoords_ = {xz, y, 0};
                }
                else if (neighbor_chunkcoords_.x == coords_.x + 1) // Right neighbor
                {
                    idx = GetChunkIndex(CHUNK_SIZE - 1, y, xz);
                    neighbor_blockcoords_ = {0, y, xz};
                }
                else if (neighbor_chunkcoords_.z == coords_.z - 1) // Back neighbor
                {
                    idx = GetChunkIndex(xz, y, 0);
                    neighbor_blockcoords_ = {xz, y, CHUNK_SIZE - 1};
                }
                else // Left neighbor
                {
                    idx = GetChunkIndex(0, y, xz);
                    neighbor_blockcoords_ = {CHUNK_SIZE - 1, y, xz};
                }

                BlockID my_block = blocks_[idx];
                if (!BlockIsOpaque(my_block))
                {
                    BlockID neighbor_block = neighbor->GetBlocks()[GetChunkIndex(neighbor_blockcoords_)];

                    // Sky light
                    uint8_t neighbor_sky_light = neighborlightmap_.GetSkyLevel(neighbor_blockcoords_);
                    if (neighbor_sky_light > 0)
                    {
                        uint8_t propagated = neighbor_sky_light - 1;
                        if (propagated > lightmap_.GetSkyLevel(idx))
                        {
                            lightmap_.SetSkyLevel(propagated, idx);
                            queue.push_back(idx);
                        }
                    }

                    // Block light
                    if (neighbor_block == BlockID::light)
                    {
                        lightmap_.SetBlockLevel(9, idx);
                        queue.push_back(idx);
                    }
                    else
                    {
                        uint8_t neighbor_block_light = neighborlightmap_.GetBlockLevel(neighbor_blockcoords_);
                        if (neighbor_block_light > 0 && !BlockIsOpaque(my_block))
                        {
                            uint8_t propagated = neighbor_block_light - 1;
                            if (propagated > lightmap_.GetBlockLevel(idx))
                            {
                                lightmap_.SetBlockLevel(propagated, idx);
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

        uint8_t sky = lightmap_.GetSkyLevel(idx);
        uint8_t sky_prop = sky == 0 ? 0 : sky - 1;
        uint8_t block = lightmap_.GetBlockLevel(idx);
        uint8_t block_prop = block == 0 ? 0 : block - 1;

        int y = idx & 127;
        int z = (idx >> 7) & 31;
        int x = idx >> 12;

        if (y > 0)
        {
            uint32_t n = idx - 1;
            bool pushed = false;
            
            if (sky_prop > lightmap_.GetSkyLevel(n))
            {
                lightmap_.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > lightmap_.GetBlockLevel(n))
            {
                lightmap_.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (y < 127)
        {
            uint32_t n = idx + 1;

            if (block_prop > lightmap_.GetBlockLevel(n))
            {
                lightmap_.SetBlockLevel(block_prop, n);
                queue.push_back(n);
            }
        }

        if (z > 0)
        {
            uint32_t n = idx - SZ;
            bool pushed = false;
            
            if (sky_prop > lightmap_.GetSkyLevel(n))
            {
                lightmap_.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > lightmap_.GetBlockLevel(n))
            {
                lightmap_.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (z < 31)
        {
            uint32_t n = idx + SZ;
            bool pushed = false;
            
            if (sky_prop > lightmap_.GetSkyLevel(n))
            {
                lightmap_.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > lightmap_.GetBlockLevel(n))
            {
                lightmap_.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (x > 0)
        {
            uint32_t n = idx - SX;
            bool pushed = false;
            
            if (sky_prop > lightmap_.GetSkyLevel(n))
            {
                lightmap_.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > lightmap_.GetBlockLevel(n))
            {
                lightmap_.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }

        if (x < 31)
        {
            uint32_t n = idx + SX;
            bool pushed = false;
            
            if (sky_prop > lightmap_.GetSkyLevel(n))
            {
                lightmap_.SetSkyLevel(sky_prop, n);
                queue.push_back(n);
                pushed = true;
            }

            if (block_prop > lightmap_.GetBlockLevel(n))
            {
                lightmap_.SetBlockLevel(block_prop, n);
                if (!pushed)
                    queue.push_back(n);
            }
        }
    }
}

void Chunk::UpdateVertexLighting()
{
    SetState(ChunkState::UPDATING_VERTEX_LIGHTING);

    auto neighbors = chunk_manager_->GetAdjacentNeighbors(coords_);

    // These calculations were reverse engineered from the latest version of the original game (v2.01),
    // although they've been slightly adjusted to ensure all chunks agree on ambient_light, and are
    // never pitch black.
    int light_phase = Moon::GetCurrentMoon()->GetLightPhase();
    float sin_world_time = glm::sin(((light_phase + 0.1f) * SECONDS_PER_LIGHT_PHASE) * LIGHT_CYCLE_OMEGA);
    glm::vec3 sunlight_direction = Moon::GetCurrentMoon()->GetSunlightDirection();
    float ambient_light = 0.5f * sin_world_time;
    if (ambient_light < 0)
        ambient_light *= -0.5f;
    float sunlight_factor = ambient_light + 0.5f;

    // We need to determine the global voxel position of the base of the quad (the inverse of what we do in BuildVertices), but some
    // vertex positions include offsets (du and dv), so we can't always do this directly. But we submit vertices in groups of six,
    // and for the first one in each group we always have vertex.position == quad.basecoords_, so we can use the first one to determine
    // the lighting and apply it to the whole group.
    std::vector<BlockVertex> *vertex_lists[2] = {&opaque_vertices_, &transparent_vertices_};
    for (auto vertices : vertex_lists)
    {
        for (int i = 0; i < vertices->size(); i += 4)
        {
            auto &first = vertices->at(i);

            if (first.light.r != 2.0f) // Skip light blocks
            {
                glm::vec2 light;

                glm::ivec3 voxel_g;
                if (first.face_normal.y != 0)
                    voxel_g = {first.position.x + 0.5f, first.position.y + 0.5f, first.position.z + 0.5f};
                else
                    voxel_g = {first.position.x + 0.5f, first.position.y - 0.5f, first.position.z + 0.5f};

                float dot = glm::dot(sunlight_direction, first.face_normal);
                if (dot < 0)
                    dot = 0;

                uint8_t _sky_light, _block_light;
                glm::ivec3 light_sample_voxel_coords = voxel_g - glm::ivec3(first.face_normal);
                glm::ivec3 light_sample_chunk_coords = VoxelToChunk(light_sample_voxel_coords);
                if (light_sample_chunk_coords == coords_)
                {
                    _sky_light = lightmap_.GetSkyLevel(GlobalToLocalVoxel(light_sample_voxel_coords));
                    _block_light = lightmap_.GetBlockLevel(GlobalToLocalVoxel(light_sample_voxel_coords));
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
                    light = glm::vec2(glm::max(scaled_sky_light, block_light) / 100.0f);
                }
                else
                {
                    float red_green = glm::clamp(block_light / 100.0f + scaled_sky_light * sunlight_factor * 0.01f, 0.0f, 1.0f);
                    float blue = glm::clamp(block_light / 100.0f + scaled_sky_light * 0.01f, 0.0f, 1.0f);
                    light = {red_green, blue};
                }

                vertices->at(i).light = light;
                vertices->at(i+1).light = light;
                vertices->at(i+2).light = light;
                vertices->at(i+3).light = light;
            }
        }
    }

    UnpinNeighbors();

    SetState(ChunkState::READY_TO_UPLOAD);
}

void Chunk::BuildVertices()
{
    SetState(ChunkState::BUILDING_VERTICES);

    // These calculations were reverse engineered from the latest version of the original game (v2.01),
    // although they've been slightly adjusted to ensure all chunks agree on ambient_light, and are
    // never pitch black.
    int light_phase = Moon::GetCurrentMoon()->GetLightPhase();
    float sin_world_time = glm::sin(((light_phase + 0.1f) * SECONDS_PER_LIGHT_PHASE) * LIGHT_CYCLE_OMEGA);
    glm::vec3 sunlight_direction = Moon::GetCurrentMoon()->GetSunlightDirection();
    float ambient_light = 0.5f * sin_world_time;
    if (ambient_light < 0)
        ambient_light *= -0.5f;
    float sunlight_factor = ambient_light + 0.5f;

    auto neighbors = chunk_manager_->GetAdjacentNeighbors(coords_);
    auto quads = GreedyMesh(neighbors);
    auto tile_origins = GetAtlasTileOrigins();

    opaque_vertices_.clear();
    opaque_indices_.clear();
    transparent_vertices_.clear();
    transparent_indices_.clear();
    int opaque_index_base = 0;
    int transparent_index_base = 0;
    for (const MeshQuad &quad : quads)
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
            base_pos = {quad.base_coords.x + coords_.x*CHUNK_SIZE - 0.5f, quad.base_coords.y + 0.5f, quad.base_coords.z + coords_.z*CHUNK_SIZE -0.5f};
        else
            base_pos = {quad.base_coords.x + coords_.x*CHUNK_SIZE - 0.5f, quad.base_coords.y - 0.5f, quad.base_coords.z + coords_.z*CHUNK_SIZE - 0.5f};

        // Determine texture tiling repeats
        int quad_width = glm::length(quad.du);
        int quad_height = glm::length(quad.dv);

        //
        // Lighting
        //

        float dot = glm::dot(sunlight_direction, normal);
        if (dot < 0)
            dot = 0;

        glm::vec2 v_light;

        if (quad.block == BlockID::light)
        {
            v_light = glm::vec2{2.0f};
        }
        else
        {
            uint8_t sky_light, block_light;
            glm::ivec3 light_sample_voxel_coords = LocalToGlobalVoxel(quad.base_coords, coords_) - glm::ivec3(normal);
            glm::ivec3 light_sample_chunk_coords = VoxelToChunk(light_sample_voxel_coords);
            if (light_sample_chunk_coords == coords_)
            {
                sky_light = lightmap_.GetSkyLevel(GlobalToLocalVoxel(light_sample_voxel_coords));
                block_light = lightmap_.GetBlockLevel(GlobalToLocalVoxel(light_sample_voxel_coords));
            }
            else
            {
                for (auto &neighbor : neighbors)
                {
                    if (neighbor->GetCoords() == light_sample_chunk_coords)
                    {
                        sky_light = neighbor->GetLightmap().GetSkyLevel(GlobalToLocalVoxel(light_sample_voxel_coords));
                        block_light = neighbor->GetLightmap().GetBlockLevel(GlobalToLocalVoxel(light_sample_voxel_coords));
                        break;
                    }
                }
            }
            float sky_light_f = (float)sky_light * (100.0f / 9.0f);      // Apparently Charlie's light values were in [0, 100]. Mine are in [0, 9], 
            float block_light_f = (float)block_light * (100.0f / 9.0f);  // so let's scale to [0, 100] so his code works as-is

            float corrected_sky_light = (sky_light_f * ambient_light + (1.0 - ambient_light) * sky_light_f * dot) * sunlight_factor;
            float scaled_sky_light;
            if (corrected_sky_light != 0)
                scaled_sky_light = ((corrected_sky_light / 100.0f) * 68.0f) + 32.0f;
            else
                scaled_sky_light = 0;

            if (sin_world_time > 0)
            {
                v_light = glm::vec2(glm::max(scaled_sky_light, block_light_f) / 100.0f);
            }
            else
            {
                float red_green = glm::clamp(block_light_f / 100.0f + scaled_sky_light * sunlight_factor * 0.01f, 0.0f, 1.0f);
                float blue = glm::clamp(block_light_f / 100.0f + scaled_sky_light * 0.01f, 0.0f, 1.0f);
                v_light = {red_green, blue};
            }
        }

        // Push vertices
        auto &vertices = BlockIsOpaque(quad.block) ? opaque_vertices_ : transparent_vertices_;
        vertices.emplace_back(base_pos,                     glm::vec4{0,          0,           tile_origin}, normal, v_light);
        vertices.emplace_back(base_pos + quad.dv,           glm::vec4{0,          quad_height, tile_origin}, normal, v_light);
        vertices.emplace_back(base_pos + quad.dv + quad.du, glm::vec4{quad_width, quad_height, tile_origin}, normal, v_light);
        vertices.emplace_back(base_pos + quad.du,           glm::vec4{quad_width, 0,           tile_origin}, normal, v_light);

        // Push indices
        auto &indices = BlockIsOpaque(quad.block) ? opaque_indices_ : transparent_indices_;
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

/*
    The algorithm works as follows.
    
    For each axis, we consider all planes perpendicular to that axis slicing through the block faces (if N is the length of the chunk along the axis, there
    are N+1 such planes). It should be clear that each plane represents a set of block faces that could be rendered. For each of these planes, we first construct
    a mask that indicates which block face, if any, should be rendered at each position, as well as whether it's a front or back face.

    Then, for each column of this mask, we move up to the first renderable face. If none is found, we move on to the next column. If one is found, we begin with
    a 1x1 quad, and extend its height until a different mask value is reached. We then extend this new quad's width as far as possible to get the final quad.
    Finally, all mask positions covered by the final quad are marked as non-renderable, and we continue iterating through this column.
*/
std::vector<MeshQuad> Chunk::GreedyMesh(std::array<Chunk *, 4> neighbors)
{
    struct MaskEntry
    {
        BlockID block;
        uint8_t light;
        bool back_face;

        MaskEntry() { block = BlockID::air, back_face = false; }
        MaskEntry(BlockID block, uint8_t light, bool back_face) : block(block), light(light), back_face(back_face) {}
        bool operator==(const MaskEntry& other) { return block == other.block && light == other.light && back_face == other.back_face; }
        bool operator!=(const MaskEntry& other) { return block != other.block || light != other.light || back_face != other.back_face; }
    };

    std::vector<MaskEntry> mask;
    mask.reserve(CHUNK_SIZE * WORLD_HEIGHT_LIMIT);

    std::vector<MeshQuad> quads;
    quads.reserve(1500); // 1500 = empirically determined maximum (with padding). Depends on generation and meshing algorithms.

    auto front_neighbor = neighbors[0];
    auto right_neighbor = neighbors[1];
    auto back_neighbor = neighbors[2];
    auto left_neighbor = neighbors[3];

    //
    // X axis
    //

    // For each ZY plane
    for (int block_x = 0; block_x < CHUNK_SIZE; block_x++)
    {
        mask.clear();

        // Build mask
        for (int block_z = 0; block_z < CHUNK_SIZE; block_z++)
        {
            for (int block_y = 0; block_y < WORLD_HEIGHT_LIMIT; block_y++)
            {
                glm::ivec3 left_block_coords;
                BlockID left_block;

                glm::ivec3 right_block_coords = {block_x, block_y, block_z};
                BlockID right_block = blocks_[GetChunkIndex(right_block_coords)];

                if (block_x == 0)
                {
                    left_block_coords = {CHUNK_SIZE - 1, block_y, block_z};
                    left_block = left_neighbor->GetBlocks()[GetChunkIndex(left_block_coords)];
                }
                else
                {
                    left_block_coords = {block_x - 1, block_y, block_z};
                    left_block = blocks_[GetChunkIndex(left_block_coords)];
                }

                if (ShouldRenderFace(left_block, right_block) && !ShouldRenderFace(right_block, left_block))
                {
                    uint8_t light = lightmap_.GetCombinedLight(right_block_coords);
                    mask.emplace_back(left_block, light, false);
                }
                else if (ShouldRenderFace(right_block, left_block) && !ShouldRenderFace(left_block, right_block))
                {
                    uint8_t light;
                    if (block_x == 0)
                        light = left_neighbor->GetLightmap().GetCombinedLight(left_block_coords);
                    else
                        light = lightmap_.GetCombinedLight(left_block_coords);

                    mask.emplace_back(right_block, light, true);
                }
                else
                {
                    mask.emplace_back(BlockID::air, 0, false); // Don't render
                }
            }
        }

        // Use mask to construct quads
        for (int block_z = 0; block_z < CHUNK_SIZE; block_z++)
        {
            int y = 0;
            while (y < WORLD_HEIGHT_LIMIT)
            {
                MaskEntry base_entry = mask[y + block_z * WORLD_HEIGHT_LIMIT];

                // Skip empty faces
                if (base_entry.block == BlockID::air)
                {
                    y++;
                    continue;
                }

                // Determine quad height (along Y)
                int quad_height = 1;
                while (y + quad_height < WORLD_HEIGHT_LIMIT && mask[(y + quad_height) + block_z * WORLD_HEIGHT_LIMIT] == base_entry)
                    quad_height++;

                // Determine quad width (along Z)
                int quad_width = 1;
                for (int z2 = block_z + 1; z2 < CHUNK_SIZE; z2++)
                {
                    bool column_matches = true;
                    for (int y2 = y; y2 < y + quad_height; y2++)
                    {
                        if (mask[y2 + z2 * WORLD_HEIGHT_LIMIT] != base_entry)
                        {
                            column_matches = false;
                            break;
                        }
                    }

                    if (!column_matches)
                        break;

                    quad_width++;
                }

                // Mark mask entries covered by this quad as "handled"
                for (int z2 = block_z; z2 < block_z + quad_width; z2++)
                    for (int y2 = y; y2 < y + quad_height; y2++)
                        mask[y2 + z2 * WORLD_HEIGHT_LIMIT].block = BlockID::air;

                // Push quad
                quads.emplace_back(
                    base_entry.block,
                    glm::ivec3{block_x, y, block_z},
                    glm::vec3{0, 0, quad_width},
                    glm::vec3{0, quad_height, 0},
                    base_entry.back_face
                );

                // Advance y past this quad
                y += quad_height;
            }
        }
    }

    //
    // Z axis
    //

    // For each XY plane
    for (int block_z = 0; block_z < CHUNK_SIZE; block_z++)
    {
        mask.clear();

        // Build mask
        for (int block_x = 0; block_x < CHUNK_SIZE; block_x++)
        {
            for (int block_y = 0; block_y < WORLD_HEIGHT_LIMIT; block_y++)
            {
                glm::ivec3 back_block_coords;
                BlockID back_block;

                glm::ivec3 front_block_coords = {block_x, block_y, block_z};
                BlockID front_block = blocks_[GetChunkIndex(front_block_coords)];

                if (block_z == 0)
                {
                    back_block_coords = {block_x, block_y, CHUNK_SIZE - 1};
                    back_block = back_neighbor->GetBlocks()[GetChunkIndex(back_block_coords)];
                }
                else
                {
                    back_block_coords = {block_x, block_y, block_z - 1};
                    back_block = blocks_[GetChunkIndex(back_block_coords)];
                }

                if (ShouldRenderFace(back_block, front_block) && !ShouldRenderFace(front_block, back_block))
                {
                    uint8_t light = lightmap_.GetCombinedLight(front_block_coords);
                    mask.emplace_back(back_block, light, false);
                }
                else if (ShouldRenderFace(front_block, back_block) && !ShouldRenderFace(back_block, front_block))
                {
                    uint8_t light;
                    if (block_z == 0)
                        light = back_neighbor->GetLightmap().GetCombinedLight(back_block_coords);
                    else
                        light = lightmap_.GetCombinedLight(back_block_coords);

                    mask.emplace_back(front_block, light, true);
                }
                else
                {
                    mask.emplace_back(BlockID::air, 0, false); // Don't render
                }
            }
        }

        // Use mask to construct quads
        for (int block_x = 0; block_x < CHUNK_SIZE; ++block_x)
        {
            int y = 0;
            while (y < WORLD_HEIGHT_LIMIT)
            {
                MaskEntry base_entry = mask[y + block_x * WORLD_HEIGHT_LIMIT];

                // Skip empty cells
                if (base_entry.block == BlockID::air)
                {
                    y++;
                    continue;
                }

                // Determine quad height (along Y)
                int quad_height = 1;
                while (y + quad_height < WORLD_HEIGHT_LIMIT && mask[(y + quad_height) + block_x * WORLD_HEIGHT_LIMIT] == base_entry)
                    quad_height++;

                // Determine quad width (along X)
                int quad_width = 1;
                for (int x2 = block_x + 1; x2 < CHUNK_SIZE; x2++)
                {
                    bool column_matches = true;
                    for (int y2 = y; y2 < y + quad_height; y2++)
                    {
                        if (mask[y2 + x2 * WORLD_HEIGHT_LIMIT] != base_entry)
                        {
                            column_matches = false;
                            break;
                        }
                    }

                    if (!column_matches)
                        break;

                    quad_width++;
                }

                // Mark mask entries covered by this quad as "handled"
                for (int x2 = block_x; x2 < block_x + quad_width; x2++)
                    for (int y2 = y; y2 < y + quad_height; y2++)
                        mask[y2 + x2 * WORLD_HEIGHT_LIMIT].block = BlockID::air;

                // Push quad
                quads.emplace_back(
                    base_entry.block,
                    glm::ivec3{block_x, y, block_z},
                    glm::vec3{quad_width, 0, 0},
                    glm::vec3{0, quad_height, 0},
                    !base_entry.back_face
                );

                // Advance y past this quad
                y += quad_height;
            }
        }
    }

    //
    // Y axis
    //

    // For each XZ plane
    for (int block_y = 0; block_y < WORLD_HEIGHT_LIMIT; block_y++)
    {
        mask.clear();

        // Build mask
        for (int block_x = 0; block_x < CHUNK_SIZE; block_x++)
        {
            for (int block_z = 0; block_z < CHUNK_SIZE; block_z++)
            {
                if (block_y < WORLD_HEIGHT_LIMIT - 1) // Skip rendering top of chunk
                {
                    BlockID bottom_block = blocks_[GetChunkIndex(block_x, block_y, block_z)];  // These blocks are on opposite sides of the plane
                    BlockID top_block = blocks_[GetChunkIndex(block_x, block_y + 1, block_z)]; //

                    if (ShouldRenderFace(bottom_block, top_block) && !ShouldRenderFace(top_block, bottom_block))
                    {
                        uint8_t light = lightmap_.GetCombinedLight(block_x, block_y + 1, block_z);
                        mask.emplace_back(bottom_block, light, false);
                    }
                    else if (ShouldRenderFace(top_block, bottom_block) && !ShouldRenderFace(bottom_block, top_block))
                    {
                        uint8_t light = lightmap_.GetCombinedLight(block_x, block_y, block_z);
                        mask.emplace_back(top_block, light, true);
                    }
                    else
                    {
                        mask.emplace_back(BlockID::air, 0, false); // Don't render
                    }
                }
                else
                {
                    mask.emplace_back(BlockID::air, 0, false);
                }
            }
        }

        // Use mask to construct quads
        for (int block_x = 0; block_x < CHUNK_SIZE; block_x++)
        {
            int z = 0;
            while (z < CHUNK_SIZE)
            {
                MaskEntry base_entry = mask[z + block_x * CHUNK_SIZE];

                // Skip empty cells
                if (base_entry.block == BlockID::air)
                {
                    z++;
                    continue;
                }

                // Determine quad height (along Z)
                int quad_height = 1;
                while (z + quad_height < CHUNK_SIZE && mask[(z + quad_height) + block_x * CHUNK_SIZE] == base_entry)
                    quad_height++;

                // Determine quad width (along X)
                int quad_width = 1;
                for (int x2 = block_x + 1; x2 < CHUNK_SIZE; x2++)
                {
                    bool column_matches = true;
                    for (int z2 = z; z2 < z + quad_height; z2++)
                    {
                        if (mask[z2 + x2 * CHUNK_SIZE] != base_entry)
                        {
                            column_matches = false;
                            break;
                        }
                    }

                    if (!column_matches)
                        break;

                    quad_width++;
                }

                // Mark mask entries covered by this quad as "handled"
                for (int x2 = block_x; x2 < block_x + quad_width; x2++)
                    for (int z2 = z; z2 < z + quad_height; z2++)
                        mask[z2 + x2 * CHUNK_SIZE].block = BlockID::air;

                // Push quad
                quads.emplace_back(
                    base_entry.block,
                    glm::ivec3{block_x, block_y, z},
                    glm::vec3{quad_width, 0, 0},
                    glm::vec3{0, 0, quad_height},
                    base_entry.back_face
                );

                // Advance z past this quad
                z += quad_height;
            }
        }
    }

    return quads;
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

uint8_t Lightmap::GetCombinedLight(glm::ivec3 coords) const
{
    return (_sky_light[GetChunkIndex(coords)] << 4) | _block_light[GetChunkIndex(coords)];
}

uint8_t Lightmap::GetCombinedLight(int x, int y, int z) const
{
    return (_sky_light[GetChunkIndex(x, y, z)] << 4) | _block_light[GetChunkIndex(x, y, z)];
}

void Lightmap::ClearBlockLight()
{
    std::memset(_block_light, 0, BLOCKS_IN_CHUNK * sizeof(uint8_t));
}
