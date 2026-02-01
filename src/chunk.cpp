
#include <cstdint>
#include <vector>
#include <unordered_map>

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
// Chunk Vertices
//

void ChunkVertices::SwapBuffers()
{
    _read_buffer ^= 1;

    // Clear new write buffers
    _opaque_buffers[_read_buffer ^ 1].clear();
    _transparent_buffers[_read_buffer ^ 1].clear();
}

void ChunkVertices::AddOpaqueVertex(BlockVertex &vertex)
{
    _opaque_buffers[_read_buffer ^ 1].push_back(vertex);
}

void ChunkVertices::AddTransparentVertex(BlockVertex &vertex)
{
    _transparent_buffers[_read_buffer ^ 1].push_back(vertex);
}

BlockVertex *ChunkVertices::GetOpaqueData()
{
    return _opaque_buffers[_read_buffer].data();
}

size_t ChunkVertices::GetOpaqueCount()
{
    return _opaque_buffers[_read_buffer].size();
}

size_t ChunkVertices::GetReservedOpaqueCount()
{
    return _reserved_opaque_count;
}

void ChunkVertices::SetReservedOpaqueCount(size_t count)
{
    _reserved_opaque_count = count;
}

BlockVertex *ChunkVertices::GetTransparentData()
{
    return _transparent_buffers[_read_buffer].data();
}

size_t ChunkVertices::GetTransparentCount()
{
    return _transparent_buffers[_read_buffer].size();
}

size_t ChunkVertices::GetReservedTransparentCount()
{
    return _reserved_transparent_count;
}

void ChunkVertices::SetReservedTransparentCount(size_t count)
{
    _reserved_transparent_count = count;
}

//
// Chunk
//

Chunk::Chunk(glm::ivec3 coords, bool is_border_chunk, ChunkManager *chunk_manager)
{
    _is_border_chunk = is_border_chunk;
    _coords = coords;
    _blocks = chunk_manager->AllocateBlockMemory();
    _chunk_manager = chunk_manager;
}

//
// Creates OpenGL data for the chunk.
//
// Must be called from the main thread!
//
// Normally I would do this in the constructor (RAII), but chunks can be created off the
// main thread, and only the main thread can make OpenGL calls.
//
void Chunk::GLCreate()
{
    // Opaques
    glGenVertexArrays(1, &_opaque_vao);
    glBindVertexArray(_opaque_vao);

    glGenBuffers(1, &_opaque_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _opaque_vbo);

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 13 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);

    _has_gl_data = true;
}

//
// Frees OpenGL data for the chunk.
//
// Must be called from the main thread!
//
// Normally I would do this in the destructor (RAII), and that would be fine since only the
// main thread is allowed to delete chunk objects, but I prefer to be explicit about this.
// Plus, it matches Chunk::GLCreate.
//
void Chunk::GLDestroy()
{
    glDeleteVertexArrays(1, &_opaque_vao);
    glDeleteVertexArrays(1, &_transparent_vao);
    glDeleteBuffers(1, &_opaque_vbo);
    glDeleteBuffers(1, &_transparent_vbo);
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

bool Chunk::HasGLData()
{
    return _has_gl_data;
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

//
// The behavior of this function depends on whether the chunk is a border chunk.
//
// For a non-border chunk, everything is fully built, and it ends in the state READY_TO_UPLOAD.
//
// For a border chunk, only the block data and internal light map is built, and it ends in the
// state INTERNAL_DONE. If a border chunk is to be rendered, BuildExternal() should be called
// before UploadVertices().
//
void Chunk::Build()
{
    _chunk_manager->GetWorkerPool().SubmitJob([this]() { LoadBlocks(); });
}

//
// Fully rebuilds light map and vertices (for non-border chunks).
//
// This should not be called on a border chunk, because it would only recalculate the internal light map,
// which doesn't change anyway.
//
void Chunk::Rebuild()
{
    _chunk_manager->GetWorkerPool().SubmitJob([this]() { BuildLightmapInternal(); });
}

// Builds all data that depends on neighbor chunk data, ending in the state READY_TO_UPLOAD.
void Chunk::BuildExternal()
{
    _chunk_manager->GetWorkerPool().SubmitJob([this]() { BuildLightmapExternal(); });
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
    if (_vertices.GetOpaqueCount() <= _vertices.GetReservedOpaqueCount())
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, _vertices.GetOpaqueCount() * sizeof(BlockVertex), _vertices.GetOpaqueData());
    }
    else
    {
        glBufferData(GL_ARRAY_BUFFER, _vertices.GetOpaqueCount() * sizeof(BlockVertex), _vertices.GetOpaqueData(), GL_DYNAMIC_DRAW);
        _vertices.SetReservedOpaqueCount(_vertices.GetOpaqueCount());
    }

    // Transparents
    glBindBuffer(GL_ARRAY_BUFFER, _transparent_vbo);
    if (_vertices.GetTransparentCount() <= _vertices.GetReservedTransparentCount())
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, _vertices.GetTransparentCount() * sizeof(BlockVertex), _vertices.GetTransparentData());
    }
    else
    {
        glBufferData(GL_ARRAY_BUFFER, _vertices.GetTransparentCount() * sizeof(BlockVertex), _vertices.GetTransparentData(), GL_DYNAMIC_DRAW);
        _vertices.SetReservedTransparentCount(_vertices.GetTransparentCount());
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
    // glDrawArrays(GL_TRIANGLES, 0, _opaque_vertices.size());
    glDrawArrays(GL_TRIANGLES, 0, _vertices.GetOpaqueCount());
}

//
// Render chunk's transparent vertices.
//
// This should be called after Chunk::UploadVertices and Chunk::RenderOpaques.
//
void Chunk::RenderTransparents()
{
    glBindVertexArray(_transparent_vao);
    // glDrawArrays(GL_TRIANGLES, 0, _transparent_vertices.size());
    glDrawArrays(GL_TRIANGLES, 0, _vertices.GetTransparentCount());
}

void Chunk::SetState(ChunkState state)
{
    _state.store(state, std::memory_order::release);
}

void Chunk::LoadBlocks()
{
    // Update state
    SetState(ChunkState::LOADING_BLOCKS);

    // Load blocks
    uint64_t chunk_id = ChunkCoordsToID(_coords);
    std::filesystem::path chunk_file_path = Storage::MOON_DIR / (std::string("moon") + std::to_string(Moon::GetCurrentMoon()->GetID())) / "chunks" / (std::to_string(chunk_id) + ".chunk");
    if (std::filesystem::exists(chunk_file_path))
    {
        LoadChunkFromDisk(chunk_file_path, _blocks);
    }
    else
    {
        GenerateChunk(_blocks, _coords.x, _coords.z, Moon::GetCurrentMoon()->GetSettings().seed);
        WriteChunkToDisk(chunk_file_path, _blocks);
    }

    // Start next task
    _chunk_manager->GetWorkerPool().SubmitJob([this]() { BuildLightmapInternal(); });
}

void Chunk::BuildLightmapInternal()
{
    // Update state
    SetState(ChunkState::LIGHT_INTERNAL);

    std::vector<glm::ivec3> to_expand;

    //
    // Initial fill
    //
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            uint8_t skylight_level = 15;
            for (int y = WORLD_HEIGHT_LIMIT - 1; y >= 0; y--)
            {
                BlockID block = _blocks[GetChunkIndex(x, y, z)];

                // Sky light

                if (BlockIsOpaque(block))
                    skylight_level = 0;

                _lightmap.SetSkyLevel(skylight_level, {x, y, z});
                if (skylight_level == 15)
                    to_expand.emplace_back(x, y, z);

                // Block light

                glm::ivec3 neighbors[] = {
                    {x - 1, y, z},
                    {x + 1, y, z},
                    {x, y - 1, z},
                    {x, y + 1, z},
                    {x, y, z - 1},
                    {x, y, z + 1},
                };

                bool has_neighbor_light = false;
                for (const glm::ivec3 &neighbor_coords : neighbors)
                {
                    if (!BlockIsInChunk(neighbor_coords))
                        continue;

                    BlockID neighbor_block = _blocks[GetChunkIndex(neighbor_coords.x, neighbor_coords.y, neighbor_coords.z)];
                    if (!BlockIsOpaque(block) && neighbor_block == BlockID::light)
                    {
                        has_neighbor_light = true;
                        break;
                    }
                }

                if (has_neighbor_light)
                {
                    _lightmap.SetBlockLevel(15, {x, y, z});
                    to_expand.emplace_back(x, y, z);
                }
                else
                {
                    _lightmap.SetBlockLevel(0, {x, y, z});
                }
            }
        }
    }

    //
    // Expansion
    //
    while (to_expand.size() > 0)
    {
        glm::ivec3 coords = to_expand.back();
        uint8_t sky_light = _lightmap.GetSkyLevel(coords);
        uint8_t block_light = _lightmap.GetBlockLevel(coords);
        to_expand.pop_back();

        glm::ivec3 neighbors[] = {
            {coords.x - 1, coords.y, coords.z},
            {coords.x + 1, coords.y, coords.z},
            {coords.x, coords.y - 1, coords.z},
            {coords.x, coords.y + 1, coords.z},
            {coords.x, coords.y, coords.z - 1},
            {coords.x, coords.y, coords.z + 1},
        };

        for (const glm::ivec3 &neighbor_coords : neighbors)
        {
            if (!BlockIsInChunk(neighbor_coords))
                continue;

            BlockID neighbor_block = _blocks[GetChunkIndex(neighbor_coords.x, neighbor_coords.y, neighbor_coords.z)];

            if (!BlockIsOpaque(neighbor_block))
            {
                // Sky light
                if (neighbor_coords.y <= coords.y && sky_light > 0)
                {
                    uint8_t sky_light_propagated = sky_light - 1;
                    if (sky_light_propagated > _lightmap.GetSkyLevel(neighbor_coords))
                    {
                        _lightmap.SetSkyLevel(sky_light_propagated, neighbor_coords);
                        to_expand.push_back(neighbor_coords);
                    }
                }

                // Block light
                if (block_light > 0)
                {
                    uint8_t block_light_propagated = block_light - 1;
                    if (block_light_propagated > _lightmap.GetBlockLevel(neighbor_coords))
                    {
                        _lightmap.SetBlockLevel(block_light_propagated, neighbor_coords);
                        to_expand.push_back(neighbor_coords);
                    }
                }
            }
        }
    }

    if (IsBorderChunk())
    {
        SetState(ChunkState::INTERNAL_DONE);
    }
    else
    {
        _chunk_manager->GetWorkerPool().SubmitJob([this]() { BuildLightmapExternal(); });
    }
}

void Chunk::BuildLightmapExternal()
{
    SetState(ChunkState::LIGHT_EXTERNAL);

    std::vector<std::shared_ptr<Chunk>> neighbors
    {
        _chunk_manager->GetOrCreateChunk({_coords.x - 1, 0, _coords.z}),
        _chunk_manager->GetOrCreateChunk({_coords.x + 1, 0, _coords.z}),
        _chunk_manager->GetOrCreateChunk({_coords.x,     0, _coords.z - 1}),
        _chunk_manager->GetOrCreateChunk({_coords.x,     0, _coords.z + 1}),
        _chunk_manager->GetOrCreateChunk({_coords.x - 1, 0, _coords.z - 1}),
        _chunk_manager->GetOrCreateChunk({_coords.x - 1, 0, _coords.z + 1}),
        _chunk_manager->GetOrCreateChunk({_coords.x + 1, 0, _coords.z - 1}),
        _chunk_manager->GetOrCreateChunk({_coords.x + 1, 0, _coords.z + 1})
    };

    // Reschedule if any neighbors aren't ready
    for (auto &neighbor : neighbors)
    {
        if (neighbor->GetState() <= ChunkState::LIGHT_INTERNAL)
        {
            _chunk_manager->GetWorkerPool().SubmitJob([this]() { BuildLightmapExternal(); });
            return;
        }
    }

    std::vector<glm::ivec3> to_expand;

    //
    // Initial fill
    //
    for (auto &neighbor : neighbors)
    {
        auto neighbor_chunk_coords = neighbor->GetCoords();
        auto neighbor_chunk_displacement = neighbor_chunk_coords - _coords;
        if (glm::abs(neighbor_chunk_displacement.x) + glm::abs(neighbor_chunk_displacement.z) == 1) // Directly adjacent neighbor
        {
            glm::ivec3 my_block_coords;
            glm::ivec3 neighbor_block_coords;
            const Lightmap &neighbor_lightmap = neighbor->GetLightmap();

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

                    BlockID my_block = _blocks[GetChunkIndex(my_block_coords)];
                    if (!BlockIsOpaque(my_block))
                    {
                        BlockID neighbor_block = neighbor->GetBlocks()[GetChunkIndex(neighbor_block_coords)];

                        // Sky light
                        uint8_t neighbor_sky_light = neighbor_lightmap.GetSkyLevel(neighbor_block_coords);
                        if (neighbor_sky_light > 0)
                        {
                            uint8_t propagated = neighbor_sky_light - 1;
                            if (propagated > _lightmap.GetSkyLevel(my_block_coords))
                            {
                                _lightmap.SetSkyLevel(propagated, my_block_coords);
                                to_expand.push_back(my_block_coords);
                            }
                        }

                        // Block light
                        if (neighbor_block == BlockID::light)
                        {
                            _lightmap.SetBlockLevel(15, my_block_coords);
                            to_expand.push_back(my_block_coords);
                        }
                        else
                        {
                            uint8_t neighbor_block_light = neighbor_lightmap.GetBlockLevel(neighbor_block_coords);
                            if (neighbor_block_light > 0 && !BlockIsOpaque(my_block))
                            {
                                uint8_t propagated = neighbor_block_light - 1;
                                if (propagated > _lightmap.GetBlockLevel(my_block_coords))
                                {
                                    _lightmap.SetBlockLevel(propagated, my_block_coords);
                                    to_expand.push_back(my_block_coords);
                                }
                            }
                        }
                    }
                }
            }
        }
        else // Corner neighbor
        {
            glm::ivec3 my_block_coords;
            glm::ivec3 neighbor_block_coords;
            const Lightmap &neighbor_lightmap = neighbor->GetLightmap();

            for (int y = 0; y < WORLD_HEIGHT_LIMIT; y++)
            {
                // Get coordinates for block pair
                if (neighbor_chunk_coords.z == _coords.z + 1) // Front left neighbor
                {
                    my_block_coords = {0, y, CHUNK_SIZE - 1};
                    neighbor_block_coords = {CHUNK_SIZE - 1, y, 0};
                }
                else if (neighbor_chunk_coords.x == _coords.x + 1) // Front right neighbor
                {
                    my_block_coords = {CHUNK_SIZE - 1, y, CHUNK_SIZE - 1};
                    neighbor_block_coords = {0, y, 0};
                }
                else if (neighbor_chunk_coords.z == _coords.z - 1) // Back right neighbor
                {
                    my_block_coords = {CHUNK_SIZE - 1, y, 0};
                    neighbor_block_coords = {0, y, CHUNK_SIZE - 1};
                }
                else // Back left neighbor
                {
                    my_block_coords = {0, y, 0};
                    neighbor_block_coords = {CHUNK_SIZE - 1, y, CHUNK_SIZE - 1};
                }

                BlockID my_block = _blocks[GetChunkIndex(my_block_coords)];
                if (!BlockIsOpaque(my_block))
                {
                    BlockID neighbor_block = neighbor->GetBlocks()[GetChunkIndex(neighbor_block_coords)];

                    // Sky light
                    uint8_t neighbor_sky_light = neighbor_lightmap.GetSkyLevel(neighbor_block_coords);
                    if (neighbor_sky_light > 1)
                    {
                        uint8_t propagated = neighbor_sky_light - 2;
                        if (propagated > _lightmap.GetSkyLevel(my_block_coords))
                        {
                            _lightmap.SetSkyLevel(propagated, my_block_coords);
                            to_expand.push_back(my_block_coords);
                        }
                    }

                    // Block light
                    if (neighbor_block == BlockID::light)
                    {
                        _lightmap.SetBlockLevel(13, my_block_coords);
                        to_expand.push_back(my_block_coords);
                    }
                    else
                    {
                        uint8_t neighbor_block_light = neighbor_lightmap.GetBlockLevel(neighbor_block_coords);
                        if (neighbor_block_light > 1 && !BlockIsOpaque(my_block))
                        {
                            uint8_t propagated = neighbor_block_light - 2;
                            if (propagated > _lightmap.GetBlockLevel(my_block_coords))
                            {
                                _lightmap.SetBlockLevel(propagated, my_block_coords);
                                to_expand.push_back(my_block_coords);
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
    while (to_expand.size() > 0)
    {
        glm::ivec3 coords = to_expand.back();
        uint8_t sky_light = _lightmap.GetSkyLevel(coords);
        uint8_t block_light = _lightmap.GetBlockLevel(coords);
        to_expand.pop_back();

        glm::ivec3 neighbors[] = {
            {coords.x - 1, coords.y, coords.z},
            {coords.x + 1, coords.y, coords.z},
            {coords.x, coords.y - 1, coords.z},
            {coords.x, coords.y + 1, coords.z},
            {coords.x, coords.y, coords.z - 1},
            {coords.x, coords.y, coords.z + 1},
        };

        for (const glm::ivec3 &neighbor_coords : neighbors)
        {
            if (!BlockIsInChunk(neighbor_coords))
                continue;

            BlockID neighbor_block = _blocks[GetChunkIndex(neighbor_coords.x, neighbor_coords.y, neighbor_coords.z)];

            if (!BlockIsOpaque(neighbor_block))
            {
                // Sky light
                if (neighbor_coords.y <= coords.y && sky_light > 0)
                {
                    uint8_t sky_light_propagated = sky_light - 1;
                    if (sky_light_propagated > _lightmap.GetSkyLevel(neighbor_coords))
                    {
                        _lightmap.SetSkyLevel(sky_light_propagated, neighbor_coords);
                        to_expand.push_back(neighbor_coords);
                    }
                }

                // Block light
                if (block_light > 0)
                {
                    uint8_t block_light_propagated = block_light - 1;
                    if (block_light_propagated > _lightmap.GetBlockLevel(neighbor_coords))
                    {
                        _lightmap.SetBlockLevel(block_light_propagated, neighbor_coords);
                        to_expand.push_back(neighbor_coords);
                    }
                }
            }
        }
    }

    _chunk_manager->GetWorkerPool().SubmitJob([this]() { BuildVertices(); });
}

void Chunk::BuildVertices()
{
    SetState(ChunkState::BUILDING_VERTICES);

    static std::unordered_map<BlockID, glm::vec3> ATLAS_TILE_MAP = { // Tile coordinates are: (top, side, bottom)
        {BlockID::aluminum,        glm::vec3(32, 32, 32)},
        {BlockID::aluminum_ore,    glm::vec3(17, 17, 17)},
        {BlockID::amethyst_ore,    glm::vec3(168, 168, 168)},
        {BlockID::beacon,          glm::vec3(89, 90, 104)},
        {BlockID::beryllium,       glm::vec3(140, 140, 140)},
        {BlockID::calcite,         glm::vec3(112, 112, 112)},
        {BlockID::carbon,          glm::vec3(73, 73, 73)},
        {BlockID::chalchanthite,   glm::vec3(182, 182, 182)},
        {BlockID::dirt,            glm::vec3(1, 1, 1)},
        {BlockID::feldspar,        glm::vec3(154, 154, 154)},
        {BlockID::gold_ore,        glm::vec3(45, 45, 45)},
        {BlockID::granite,         glm::vec3(155, 155, 155)},
        {BlockID::graphite,        glm::vec3(126, 126, 126)},
        {BlockID::gravel,          glm::vec3(5, 5, 5)},
        {BlockID::light,           glm::vec3(9, 9, 9)}, // NOTE: There are four variants in the atlas
        {BlockID::magnetite,       glm::vec3(18, 18, 18)},
        {BlockID::molybdenum_ore,  glm::vec3(31, 31, 31)},
        {BlockID::moon_bark,       glm::vec3(118, 118, 118)}, // NOTE: Might actually be different on top and bottom
        {BlockID::moon_leaf,       glm::vec3(6, 6, 6)},
        {BlockID::moon_wood,       glm::vec3(33, 19, 33)},
        {BlockID::neptunium,       glm::vec3(86, 86, 86)},
        {BlockID::notchium,        glm::vec3(44, 44, 44)},
        {BlockID::notchium_ore,    glm::vec3(74, 74, 74)},
        {BlockID::phosphate,       glm::vec3(113, 113, 113)},
        {BlockID::polymer,         glm::vec3(30, 30, 30)},
        {BlockID::quartz_ore,      glm::vec3(169, 169, 169)},
        {BlockID::rock,            glm::vec3(3, 3, 3)},
        {BlockID::sand,            glm::vec3(2, 2, 2)},
        {BlockID::shale_gravel,    glm::vec3(16, 16, 16)},
        {BlockID::silver_ore,      glm::vec3(59, 59, 59)},
        {BlockID::snow,            glm::vec3(4, 4, 4)},
        {BlockID::sulphur_ore,     glm::vec3(141, 141, 141)},
        {BlockID::titanium,        glm::vec3(88, 88, 88)},
        {BlockID::titanium_ore,    glm::vec3(87, 87, 87)},
        {BlockID::topsoil,         glm::vec3(14, 0, 1)},
        {BlockID::xenostone,       glm::vec3(127, 127, 127)},
        {BlockID::zircon_ore,      glm::vec3(183, 183, 183)},
        {BlockID::water,           glm::vec3(7, 7, 7)},
        {BlockID::sulphur_crystal, glm::vec3(21, 21, 21)},
        {BlockID::blue_crystal,    glm::vec3(58, 58, 58)},
        {BlockID::boron_crystal,   glm::vec3(22, 22, 22)},
        {BlockID::glass,           glm::vec3(35, 35, 35)},
        {BlockID::minilight_pz,    glm::vec3(9, 9, 9)},
        {BlockID::minilight_nz,    glm::vec3(9, 9, 9)},
        {BlockID::minilight_px,    glm::vec3(9, 9, 9)},
        {BlockID::minilight_nx,    glm::vec3(9, 9, 9)},
        {BlockID::minilight_py,    glm::vec3(9, 9, 9)},
        {BlockID::minilight_ny,    glm::vec3(9, 9, 9)}
    };

    static std::unordered_map<BlockID, glm::mat3x2> TILE_ORIGINS;
    static bool tile_origins_built = false;
    if (!tile_origins_built) // This is kind of a silly hack, but I'd rather be explicit about the tile coordinates and build from them
    {
        for (auto it = ATLAS_TILE_MAP.begin(); it != ATLAS_TILE_MAP.end(); ++it)
        {
            BlockID block_id = it->first;
            glm::vec3 atlas_tiles = it->second;
            glm::vec2 top_tile_origin = glm::vec2(
                ((int)atlas_tiles.x % 14) / 14.0f,
                (13 - ((int)atlas_tiles.x / 14)) / 14.0f
            );
            glm::vec2 side_tile_origin = glm::vec2(
                ((int)atlas_tiles.y % 14) / 14.0f,
                (13 - ((int)atlas_tiles.y / 14)) / 14.0f
            );
            glm::vec2 bottom_tile_origin = glm::vec2(
                ((int)atlas_tiles.z % 14) / 14.0f,
                (13 - ((int)atlas_tiles.z / 14)) / 14.0f
            );
            TILE_ORIGINS.emplace(block_id, glm::mat3x2(
                top_tile_origin, side_tile_origin, bottom_tile_origin
            ));
        }

        tile_origins_built = true;
    }

    std::array<std::shared_ptr<Chunk>, 4> neighbors;
    neighbors[0] = _chunk_manager->GetOrCreateChunk({_coords.x, 0, _coords.z + 1}); // Front
    neighbors[1] = _chunk_manager->GetOrCreateChunk({_coords.x + 1, 0, _coords.z}); // Right
    neighbors[2] = _chunk_manager->GetOrCreateChunk({_coords.x, 0, _coords.z - 1}); // Back
    neighbors[3] = _chunk_manager->GetOrCreateChunk({_coords.x - 1, 0, _coords.z}); // Left

    // Reschedule if any neighbors aren't ready
    for (auto &neighbor : neighbors)
    {
        if (neighbor->GetState() <= ChunkState::LIGHT_INTERNAL)
        {
            _chunk_manager->GetWorkerPool().SubmitJob([this]() { BuildVertices(); });
            return;
        }
    }

    std::vector<BlockQuad> quads = GreedyMesh(_blocks, neighbors);

    for (const BlockQuad &quad : quads)
    {
        // Determine vertex normal
        glm::vec3 normal = glm::normalize(glm::cross(quad.du, quad.dv));
        if (quad.back_face)
            normal = -normal;

        // Determine texture atlas tile
        int side = normal.y > 0 ? 2 : (normal.y < 0 ? 0 : 1);
        glm::vec2 tile_origin = TILE_ORIGINS[quad.block][side];

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
        float sin_world_time = glm::sin((snapped_world_time + 3*SECONDS_PER_LIGHT_PHASE) * (2 * 3.14159f / (LIGHT_PHASES * SECONDS_PER_LIGHT_PHASE)));
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
        float sky_light = (float)_sky_light * (100.0f / 15.0f);      // Apparently Charlie's light values were in [0, 100]. Mine are in [0, 15], 
        float block_light = (float)_block_light * (100.0f / 15.0f);  // so let's scale to [0, 100] so his code works as-is

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
        BlockVertex vert_1{base_pos,                     glm::vec4{0,          0,           tile_origin}, normal, light};
        BlockVertex vert_2{base_pos + quad.dv,           glm::vec4{0,          quad_height, tile_origin}, normal, light};
        BlockVertex vert_3{base_pos + quad.dv + quad.du, glm::vec4{quad_width, quad_height, tile_origin}, normal, light};
        BlockVertex vert_4{base_pos + quad.du,           glm::vec4{quad_width, 0,           tile_origin}, normal, light};
        if (!quad.back_face)
        {
            if (BlockIsOpaque(quad.block))
            {
                _vertices.AddOpaqueVertex(vert_1);
                _vertices.AddOpaqueVertex(vert_2);
                _vertices.AddOpaqueVertex(vert_3);
                _vertices.AddOpaqueVertex(vert_3);
                _vertices.AddOpaqueVertex(vert_4);
                _vertices.AddOpaqueVertex(vert_1);
            }
            else
            {
                _vertices.AddTransparentVertex(vert_1);
                _vertices.AddTransparentVertex(vert_2);
                _vertices.AddTransparentVertex(vert_3);
                _vertices.AddTransparentVertex(vert_3);
                _vertices.AddTransparentVertex(vert_4);
                _vertices.AddTransparentVertex(vert_1);
            }
        }
        else
        {
            if (BlockIsOpaque(quad.block))
            {
                _vertices.AddOpaqueVertex(vert_1);
                _vertices.AddOpaqueVertex(vert_4);
                _vertices.AddOpaqueVertex(vert_3);
                _vertices.AddOpaqueVertex(vert_3);
                _vertices.AddOpaqueVertex(vert_2);
                _vertices.AddOpaqueVertex(vert_1);
            }
            else
            {
                _vertices.AddTransparentVertex(vert_1);
                _vertices.AddTransparentVertex(vert_4);
                _vertices.AddTransparentVertex(vert_3);
                _vertices.AddTransparentVertex(vert_3);
                _vertices.AddTransparentVertex(vert_2);
                _vertices.AddTransparentVertex(vert_1);
            }
        }
    }

    _vertices.SwapBuffers();

    SetState(ChunkState::READY_TO_UPLOAD);
}

//
// Lightmap
//

uint8_t Lightmap::GetSkyLevel(glm::ivec3 coords) const
{
    uint8_t entry = _map[GetChunkIndex(coords.x, coords.y, coords.z)];
    return (entry >> 4);
}

// Level must be in the range [0, 15]
void Lightmap::SetSkyLevel(uint8_t sky_level, glm::ivec3 coords)
{
    uint8_t entry = _map[GetChunkIndex(coords.x, coords.y, coords.z)];
    _map[GetChunkIndex(coords.x, coords.y, coords.z)] = (entry & 0x0F) | (sky_level << 4);
}

uint8_t Lightmap::GetBlockLevel(glm::ivec3 coords) const
{
    uint8_t entry = _map[GetChunkIndex(coords.x, coords.y, coords.z)];
    return (entry & 0x0f);
}

// Level must be in the range [0, 15]
void Lightmap::SetBlockLevel(uint8_t block_level, glm::ivec3 coords)
{
    uint8_t entry = _map[GetChunkIndex(coords.x, coords.y, coords.z)];
    _map[GetChunkIndex(coords.x, coords.y, coords.z)] = (entry & 0xF0) | block_level;
}
