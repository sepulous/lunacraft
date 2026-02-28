
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
// Chunk tasks
//

bool (Chunk::*ChunkTask::LOAD_BLOCKS)() = &Chunk::LoadBlocks;
bool (Chunk::*ChunkTask::BUILD_LIGHTMAP_INTERNAL)() = &Chunk::BuildLightmapInternal;
bool (Chunk::*ChunkTask::BUILD_LIGHTMAP_EXTERNAL)() = &Chunk::BuildLightmapExternal;
bool (Chunk::*ChunkTask::UPDATE_VERTEX_LIGHTING)() = &Chunk::UpdateVertexLighting;
bool (Chunk::*ChunkTask::BUILD_VERTICES)() = &Chunk::BuildVertices;
bool (Chunk::*ChunkTask::UNPIN_NEIGHBORS)() = &Chunk::UnpinNeighbors;

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    // Transparents
    glGenVertexArrays(1, &_transparent_vao);
    glBindVertexArray(_transparent_vao);

    glGenBuffers(1, &_transparent_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _transparent_vbo);

    glGenBuffers(1, &_transparent_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _transparent_ebo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(10 * sizeof(float)));
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

bool Chunk::UnpinNeighbors()
{
    auto neighbors = _chunk_manager->GetNeighbors(_coords);
    for (auto &neighbor : neighbors)
        neighbor->Unpin();

    return true;
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
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, _opaque_indices.size() * sizeof(uint32_t), _opaque_indices.data());
    }
    else
    {
        glBufferData(GL_ARRAY_BUFFER, _opaque_vertices.size() * sizeof(BlockVertex), _opaque_vertices.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _opaque_indices.size() * sizeof(uint32_t), _opaque_indices.data(), GL_STATIC_DRAW);
        _reserved_opaque_vertex_count = _opaque_vertices.size();
    }

    // Transparents
    glBindBuffer(GL_ARRAY_BUFFER, _transparent_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _transparent_ebo);
    if (_transparent_vertices.size() <= _reserved_transparent_vertex_count)
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, _transparent_vertices.size() * sizeof(BlockVertex), _transparent_vertices.data());
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, _transparent_indices.size() * sizeof(uint32_t), _transparent_indices.data());
    }
    else
    {
        glBufferData(GL_ARRAY_BUFFER, _transparent_vertices.size() * sizeof(BlockVertex), _transparent_vertices.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, _transparent_indices.size() * sizeof(uint32_t), _transparent_indices.data(), GL_STATIC_DRAW);
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
    glDrawElements(GL_TRIANGLES, _opaque_indices.size(), GL_UNSIGNED_INT, 0);
}

//
// Render chunk's transparent vertices.
//
// This should be called after Chunk::UploadVertices and Chunk::RenderOpaques.
//
void Chunk::RenderTransparents()
{
    glBindVertexArray(_transparent_vao);
    glDrawElements(GL_TRIANGLES, _transparent_indices.size(), GL_UNSIGNED_INT, 0);
}

void Chunk::SetState(ChunkState state)
{
    _state.store(state, std::memory_order::release);
}

//
// Generate block data, or load from disk.
//
bool Chunk::LoadBlocks()
{
    // Update state
    SetState(ChunkState::LOADING_BLOCKS);

    // Load blocks
    auto chunk_file_path = GetFilePath();
    if (std::filesystem::exists(chunk_file_path))
        LoadChunkFromDisk(chunk_file_path, _blocks);
    else
        GenerateChunk(_blocks, _coords.x, _coords.z, Moon::GetCurrentMoon()->GetSettings());

    return true;
}

//
// Build lightmap using internal block data.
//
bool Chunk::BuildLightmapInternal()
{
    // Update state
    SetState(ChunkState::LIGHT_INTERNAL);

    struct V3 { int x, y, z; }; // No glm vec construction; need max performance here
    std::vector<V3> to_expand;
    to_expand.reserve(BLOCKS_IN_CHUNK / 2);

    V3 offsets[] = {
        {-1, 0, 0},
        {+1, 0, 0},
        {0, -1, 0},
        {0, +1, 0},
        {0, 0, -1},
        {0, 0, +1},
    };

    //
    // Initial fill
    //
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            uint8_t skylight_level = 9;
            for (int y = WORLD_HEIGHT_LIMIT - 1; y >= 0; y--)
            {
                BlockID block = _blocks[GetChunkIndex(x, y, z)];

                // Sky light

                if (skylight_level != 0 && BlockIsOpaque(block))
                    skylight_level = 0;

                if (skylight_level != 0)
                {
                    _lightmap.SetSkyLevel(9, x, y, z);
                    to_expand.emplace_back(x, y, z);
                }

                // Block light

                if (block == BlockID::light)
                {
                    V3 neighbor_coords;
                    for (const V3 &offset : offsets)
                    {
                        neighbor_coords = {x + offset.x, y + offset.y, z + offset.z};

                        // I would use BlockIsInChunk(), but we can't afford the function overhead here
                        if ((unsigned)neighbor_coords.x >= CHUNK_SIZE || (unsigned)neighbor_coords.z >= CHUNK_SIZE || (unsigned)neighbor_coords.y >= WORLD_HEIGHT_LIMIT)
                            continue;

                        BlockID neighbor_block = _blocks[GetChunkIndex(neighbor_coords.x, neighbor_coords.y, neighbor_coords.z)];
                        if (BlockIsTransparent(neighbor_block))
                        {
                            _lightmap.SetBlockLevel(9, neighbor_coords.x, neighbor_coords.y, neighbor_coords.z);

                            // This neighbor might have already been pushed, but duplicates are on the order of 10,
                            // while the total size of to_expand is on the order of 10,000
                            to_expand.emplace_back(neighbor_coords);
                        }
                    }
                }
            }
        }
    }

    //
    // Expansion
    //
    while (to_expand.size() > 0)
    {
        V3 coords = to_expand.back();
        uint8_t sky_light = _lightmap.GetSkyLevel(coords.x, coords.y, coords.z);
        uint8_t block_light = _lightmap.GetBlockLevel(coords.x, coords.y, coords.z);
        to_expand.pop_back();

        for (const V3 &offset : offsets)
        {
            V3 neighbor_coords = {coords.x + offset.x, coords.y + offset.y, coords.z + offset.z};

            // I would use BlockIsInChunk(), but we can't afford the function overhead here
            if ((unsigned)neighbor_coords.x >= CHUNK_SIZE || (unsigned)neighbor_coords.z >= CHUNK_SIZE || (unsigned)neighbor_coords.y >= WORLD_HEIGHT_LIMIT)
                continue;

            BlockID neighbor_block = _blocks[GetChunkIndex(neighbor_coords.x, neighbor_coords.y, neighbor_coords.z)];

            if (BlockIsTransparent(neighbor_block)) // neighbor_block is transparent
            {
                // Sky light
                if (neighbor_coords.y <= coords.y && sky_light > 0)
                {
                    uint8_t sky_light_propagated = sky_light - 1;
                    if (sky_light_propagated > _lightmap.GetSkyLevel(neighbor_coords.x, neighbor_coords.y, neighbor_coords.z))
                    {
                        _lightmap.SetSkyLevel(sky_light_propagated, neighbor_coords.x, neighbor_coords.y, neighbor_coords.z);
                        to_expand.emplace_back(neighbor_coords);
                    }
                }

                // Block light
                if (block_light > 0)
                {
                    uint8_t block_light_propagated = block_light - 1;
                    if (block_light_propagated > _lightmap.GetBlockLevel(neighbor_coords.x, neighbor_coords.y, neighbor_coords.z))
                    {
                        _lightmap.SetBlockLevel(block_light_propagated, neighbor_coords.x, neighbor_coords.y, neighbor_coords.z);
                        to_expand.emplace_back(neighbor_coords);
                    }
                }
            }
        }
    }

    SetState(ChunkState::INTERNAL_DONE);

    return true;
}

//
// Incorporate neighbor lightmaps.
//
// Requires all neighbors.
//
bool Chunk::BuildLightmapExternal()
{
    SetState(ChunkState::LIGHT_EXTERNAL);

    // Reschedule if any neighbors aren't ready
    auto neighbors = _chunk_manager->GetNeighbors(_coords);
    for (auto &neighbor : neighbors)
    {
        if (neighbor->GetState() <= ChunkState::LIGHT_INTERNAL)
        {
            std::this_thread::sleep_for(2ms);
            return false;
        }
    }

    struct V3 { int x, y, z; }; // No glm vec construction; need max performance here
    std::vector<V3> to_expand;
    to_expand.reserve(BLOCKS_IN_CHUNK / 2);

    //
    // Initial fill
    //
    for (auto &neighbor : neighbors)
    {
        auto neighbor_chunk_coords = neighbor->GetCoords();
        const Lightmap &neighbor_lightmap = neighbor->GetLightmap();
        V3 my_block_coords;
        V3 neighbor_block_coords;
        for (int xz = 0; xz < CHUNK_SIZE; xz++)
        {
            for (int y = 0; y < WORLD_HEIGHT_LIMIT; y++)
            {
                // Get coordinates for block pair
                if (neighbor_chunk_coords.z == _coords.z + 1) // Front neighbor
                {
                    my_block_coords = {xz, y, CHUNK_SIZE - 1};
                    neighbor_block_coords = {xz, y, 0};
                }
                else if (neighbor_chunk_coords.x == _coords.x + 1) // Right neighbor
                {
                    my_block_coords = {CHUNK_SIZE - 1, y, xz};
                    neighbor_block_coords = {0, y, xz};
                }
                else if (neighbor_chunk_coords.z == _coords.z - 1) // Back neighbor
                {
                    my_block_coords = {xz, y, 0};
                    neighbor_block_coords = {xz, y, CHUNK_SIZE - 1};
                }
                else // Left neighbor
                {
                    my_block_coords = {0, y, xz};
                    neighbor_block_coords = {CHUNK_SIZE - 1, y, xz};
                }

                BlockID my_block = _blocks[GetChunkIndex(my_block_coords.x, my_block_coords.y, my_block_coords.z)];
                if (!BlockIsOpaque(my_block))
                {
                    BlockID neighbor_block = neighbor->GetBlocks()[GetChunkIndex(neighbor_block_coords.x, neighbor_block_coords.y, neighbor_block_coords.z)];

                    // Sky light
                    uint8_t neighbor_sky_light = neighbor_lightmap.GetSkyLevel(neighbor_block_coords.x, neighbor_block_coords.y, neighbor_block_coords.z);
                    if (neighbor_sky_light > 0)
                    {
                        uint8_t propagated = neighbor_sky_light - 1;
                        if (propagated > _lightmap.GetSkyLevel(my_block_coords.x, my_block_coords.y, my_block_coords.z))
                        {
                            _lightmap.SetSkyLevel(propagated, my_block_coords.x, my_block_coords.y, my_block_coords.z);
                            to_expand.emplace_back(my_block_coords.x, my_block_coords.y, my_block_coords.z);
                        }
                    }

                    // Block light
                    if (neighbor_block == BlockID::light)
                    {
                        _lightmap.SetBlockLevel(9, my_block_coords.x, my_block_coords.y, my_block_coords.z);
                        to_expand.emplace_back(my_block_coords.x, my_block_coords.y, my_block_coords.z);
                    }
                    else
                    {
                        uint8_t neighbor_block_light = neighbor_lightmap.GetBlockLevel(neighbor_block_coords.x, neighbor_block_coords.y, neighbor_block_coords.z);
                        if (neighbor_block_light > 0 && !BlockIsOpaque(my_block))
                        {
                            uint8_t propagated = neighbor_block_light - 1;
                            if (propagated > _lightmap.GetBlockLevel(my_block_coords.x, my_block_coords.y, my_block_coords.z))
                            {
                                _lightmap.SetBlockLevel(propagated, my_block_coords.x, my_block_coords.y, my_block_coords.z);
                                to_expand.emplace_back(my_block_coords.x, my_block_coords.y, my_block_coords.z);
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

    V3 offsets[] = {
        {-1, 0, 0},
        {+1, 0, 0},
        {0, -1, 0},
        {0, +1, 0},
        {0, 0, -1},
        {0, 0, +1},
    };
    
    while (to_expand.size() > 0)
    {
        V3 coords = to_expand.back();
        uint8_t sky_light = _lightmap.GetSkyLevel(coords.x, coords.y, coords.z);
        uint8_t block_light = _lightmap.GetBlockLevel(coords.x, coords.y, coords.z);
        to_expand.pop_back();

        for (const auto &offset : offsets)
        {
            V3 neighbor_coords{coords.x + offset.x, coords.y + offset.y, coords.z + offset.z};

            if (!BlockIsInChunk(neighbor_coords.x, neighbor_coords.y, neighbor_coords.z))
                continue;

            BlockID neighbor_block = _blocks[GetChunkIndex(neighbor_coords.x, neighbor_coords.y, neighbor_coords.z)];

            if (!BlockIsOpaque(neighbor_block))
            {
                // Sky light
                if (neighbor_coords.y <= coords.y && sky_light > 0)
                {
                    uint8_t sky_light_propagated = sky_light - 1;
                    if (sky_light_propagated > _lightmap.GetSkyLevel(neighbor_coords.x, neighbor_coords.y, neighbor_coords.z))
                    {
                        _lightmap.SetSkyLevel(sky_light_propagated, neighbor_coords.x, neighbor_coords.y, neighbor_coords.z);
                        to_expand.push_back(neighbor_coords);
                    }
                }

                // Block light
                if (block_light > 0)
                {
                    uint8_t block_light_propagated = block_light - 1;
                    if (block_light_propagated > _lightmap.GetBlockLevel(neighbor_coords.x, neighbor_coords.y, neighbor_coords.z))
                    {
                        _lightmap.SetBlockLevel(block_light_propagated, neighbor_coords.x, neighbor_coords.y, neighbor_coords.z);
                        to_expand.push_back(neighbor_coords);
                    }
                }
            }
        }
    }

    return true;
}

bool Chunk::UpdateVertexLighting()
{
    SetState(ChunkState::UPDATING_VERTEX_LIGHTING);

    auto neighbors = _chunk_manager->GetNeighbors(_coords);
    for (auto &neighbor : neighbors)
    {
        if (neighbor->GetState() <= ChunkState::LIGHT_INTERNAL) // Reschedule if any neighbors aren't ready
        {
            std::this_thread::sleep_for(2ms);
            return false;
        }
    }

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

    SetState(ChunkState::READY_TO_UPLOAD);

    return true;
}

bool Chunk::BuildVertices()
{
    SetState(ChunkState::BUILDING_VERTICES);

    auto neighbors = _chunk_manager->GetNeighbors(_coords);
    for (auto &neighbor : neighbors)
    {
        if (neighbor->GetState() <= ChunkState::LIGHT_INTERNAL) // Reschedule if any neighbors aren't ready
        {
            std::this_thread::sleep_for(2ms);
            return false;
        }
    }

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
        if (normal.x != 0)
            base_pos = {quad.base_coords.x - normal.x*0.5f, quad.base_coords.y - 0.5f, quad.base_coords.z - 0.5f};
        else if (normal.y != 0)
            base_pos = {quad.base_coords.x - 0.5f, quad.base_coords.y - normal.y*0.5f, quad.base_coords.z - 0.5f};
        else
            base_pos = {quad.base_coords.x - 0.5f, quad.base_coords.y - 0.5f, quad.base_coords.z - normal.z*0.5f};

        // Determine texture tiling repeats
        int quad_width = glm::length(quad.du);
        int quad_height = glm::length(quad.dv);

        //
        // Lighting
        //
        // These calculations were reverse engineered from an unknown version of
        // the original game (definitely at least v1.91).
        //

        glm::vec3 light;

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
        glm::ivec3 light_sample_voxel_coords = quad.base_coords - glm::ivec3(normal);
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
            light = glm::vec3(glm::max(scaled_sky_light, block_light) / 100.0f);
        }
        else
        {
            float red_green = glm::clamp(block_light / 100.0f + scaled_sky_light * sunlight_factor * 0.01f, 0.0f, 1.0f);
            float blue = glm::clamp(block_light / 100.0f + scaled_sky_light * 0.01f, 0.0f, 1.0f);
            light = {red_green, red_green, blue};
        }

        // Push vertices
        auto &vertices = BlockIsOpaque(quad.block) ? _opaque_vertices : _transparent_vertices;
        vertices.emplace_back(base_pos,                     glm::vec4{0,          0,           tile_origin}, normal, light); // 0
        vertices.emplace_back(base_pos + quad.dv,           glm::vec4{0,          quad_height, tile_origin}, normal, light); // 1
        vertices.emplace_back(base_pos + quad.dv + quad.du, glm::vec4{quad_width, quad_height, tile_origin}, normal, light); // 2
        vertices.emplace_back(base_pos + quad.du,           glm::vec4{quad_width, 0,           tile_origin}, normal, light); // 3

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

    SetState(ChunkState::READY_TO_UPLOAD);

    return true;
}

//
// Lightmap
//

Lightmap::Lightmap()
{
    _map = (uint8_t *)calloc(BLOCKS_IN_CHUNK, sizeof(uint8_t));
}

Lightmap::~Lightmap()
{
    free(_map);
}

uint8_t Lightmap::GetSkyLevel(glm::ivec3 coords) const
{
    uint8_t entry = _map[GetChunkIndex(coords.x, coords.y, coords.z)];
    return (entry >> 4);
}

uint8_t Lightmap::GetSkyLevel(int x, int y, int z) const
{
    uint8_t entry = _map[GetChunkIndex(x, y, z)];
    return (entry >> 4);
}

// Level must be in the range [0, 15]
void Lightmap::SetSkyLevel(uint8_t sky_level, glm::ivec3 coords)
{
    uint8_t entry = _map[GetChunkIndex(coords.x, coords.y, coords.z)];
    _map[GetChunkIndex(coords.x, coords.y, coords.z)] = (entry & 0x0F) | (sky_level << 4);
}

// Level must be in the range [0, 15]
void Lightmap::SetSkyLevel(uint8_t sky_level, int x, int y, int z)
{
    uint8_t entry = _map[GetChunkIndex(x, y, z)];
    _map[GetChunkIndex(x, y, z)] = (entry & 0x0F) | (sky_level << 4);
}

uint8_t Lightmap::GetBlockLevel(glm::ivec3 coords) const
{
    uint8_t entry = _map[GetChunkIndex(coords.x, coords.y, coords.z)];
    return (entry & 0x0f);
}

uint8_t Lightmap::GetBlockLevel(int x, int y, int z) const
{
    uint8_t entry = _map[GetChunkIndex(x, y, z)];
    return (entry & 0x0f);
}

// Level must be in the range [0, 15]
void Lightmap::SetBlockLevel(uint8_t block_level, glm::ivec3 coords)
{
    uint8_t entry = _map[GetChunkIndex(coords.x, coords.y, coords.z)];
    _map[GetChunkIndex(coords.x, coords.y, coords.z)] = (entry & 0xF0) | block_level;
}

// Level must be in the range [0, 15]
void Lightmap::SetBlockLevel(uint8_t block_level, int x, int y, int z)
{
    uint8_t entry = _map[GetChunkIndex(x, y, z)];
    _map[GetChunkIndex(x, y, z)] = (entry & 0xF0) | block_level;
}
