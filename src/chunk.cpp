
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <queue>
#include <fstream>

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

BlockVertex *ChunkVertices::GetTransparentData()
{
    return _transparent_buffers[_read_buffer].data();
}

size_t ChunkVertices::GetTransparentCount()
{
    return _transparent_buffers[_read_buffer].size();
}

//
// Chunk
//

Chunk::Chunk(glm::ivec3 coords, bool is_border_chunk, ChunkManager *chunk_manager)
{
    _is_border_chunk = is_border_chunk;
    _coords = coords;
    _blocks = (BlockID *)malloc(BLOCKS_IN_CHUNK * sizeof(BlockID));
    _chunk_manager = chunk_manager;
}

Chunk::~Chunk()
{
    free(_blocks);
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

void Chunk::SetHasUploadedVertices(bool status)
{
    _has_uploaded_vertices = status;
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

Lightmap &Chunk::GetLightmap()
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
    glBindBuffer(GL_ARRAY_BUFFER, _opaque_vbo);
    // glBufferData(GL_ARRAY_BUFFER, _opaque_vertices.size() * sizeof(BlockVertex), _opaque_vertices.data(), GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, _vertices.GetOpaqueCount() * sizeof(BlockVertex), _vertices.GetOpaqueData(), GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, _transparent_vbo);
    // glBufferData(GL_ARRAY_BUFFER, _transparent_vertices.size() * sizeof(BlockVertex), _transparent_vertices.data(), GL_DYNAMIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, _vertices.GetTransparentCount() * sizeof(BlockVertex), _vertices.GetTransparentData(), GL_DYNAMIC_DRAW);

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
        std::ifstream chunk_file(chunk_file_path, std::ios::binary);
        chunk_file.read(reinterpret_cast<char *>(_blocks), BLOCKS_IN_CHUNK * sizeof(BlockID));
        chunk_file.close();
    }
    else
    {
        // GenerateChunk(_blocks, _coords.x, _coords.z, moon_settings.seed);
        GenerateChunk(_blocks, _coords.x, _coords.z, Moon::GetCurrentMoon()->GetSettings().seed);

        std::ofstream chunk_file(chunk_file_path, std::ios::binary);
        chunk_file.write(reinterpret_cast<char *>(_blocks), BLOCKS_IN_CHUNK * sizeof(BlockID));
        chunk_file.close();
    }

    // Start next task
    _chunk_manager->GetWorkerPool().SubmitJob([this]() { BuildLightmapInternal(); });
}

void Chunk::BuildLightmapInternal()
{
    // Update state
    SetState(ChunkState::LIGHT_INTERNAL);

    std::vector<glm::ivec3> to_expand;
    std::vector<glm::ivec3> lights;

    // Skylight initial fill
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            uint8_t skylight_level = 15;
            for (int y = WORLD_HEIGHT_LIMIT - 1; y >= 0; y--)
            {
                BlockID block = _blocks[GetChunkIndex(x, y, z)];
                if (BlockIsOpaque(block))
                    skylight_level = 0;

                _lightmap.SetSkyLevel(skylight_level, {x, y, z});
                _lightmap.SetBlockLevel(0, {x, y, z});

                if (skylight_level == 15)
                    to_expand.emplace_back(x, y, z);

                if (block == BlockID::light)
                    lights.emplace_back(x, y, z);
            }
        }
    }

    // Skylight expansion
    while (to_expand.size() > 0)
    {
        glm::ivec3 coords = to_expand.back();
        uint8_t skylight_level = _lightmap.GetSkyLevel(coords);
        to_expand.pop_back();

        glm::ivec3 neighbors[] = {
            {coords.x - 1, coords.y, coords.z},
            {coords.x + 1, coords.y, coords.z},
            {coords.x, coords.y - 1, coords.z},
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
                uint8_t skylight_propagated = skylight_level - 1;
                if (skylight_propagated > _lightmap.GetSkyLevel(neighbor_coords))
                {
                    _lightmap.SetSkyLevel(skylight_propagated, neighbor_coords);
                    to_expand.push_back(neighbor_coords);
                }
            }
        }
    }

    // Blocklight initial fill
    for (glm::ivec3 light_coords : lights)
    {
        glm::ivec3 neighbors[] = {
            {light_coords.x - 1, light_coords.y, light_coords.z},
            {light_coords.x + 1, light_coords.y, light_coords.z},
            {light_coords.x, light_coords.y - 1, light_coords.z},
            {light_coords.x, light_coords.y + 1, light_coords.z},
            {light_coords.x, light_coords.y, light_coords.z - 1},
            {light_coords.x, light_coords.y, light_coords.z + 1},
        };

        for (const glm::ivec3 &neighbor_coords : neighbors)
        {
            if (!BlockIsInChunk(neighbor_coords))
                continue;

            BlockID neighbor_block = _blocks[GetChunkIndex(neighbor_coords.x, neighbor_coords.y, neighbor_coords.z)];
            if (!BlockIsOpaque(neighbor_block))
            {
                _lightmap.SetBlockLevel(15, neighbor_coords);
                to_expand.emplace_back(neighbor_coords.x, neighbor_coords.y, neighbor_coords.z);
            }
        }
    }

    // Blocklight expansion
    while (to_expand.size() > 0)
    {
        glm::ivec3 coords = to_expand.back();
        uint8_t blocklight_level = _lightmap.GetBlockLevel(coords);
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
                uint8_t blocklight_propagated = blocklight_level - 1;
                if (blocklight_propagated > _lightmap.GetBlockLevel(neighbor_coords))
                {
                    _lightmap.SetBlockLevel(blocklight_propagated, neighbor_coords);
                    to_expand.push_back(neighbor_coords);
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

    // Use neighbor light data...

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

    // Wait until all neighbors are done loading their blocks. This busy-waiting sucks, but will have to do for now.
    for (int i = 0; i < 4; i++)
    {
        if (neighbors[i]->GetState() <= ChunkState::LOADING_BLOCKS)
            i = -1;
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

        glm::vec3 light;

        double world_time = Moon::GetCurrentMoon()->GetWorldTime();
        double sin_world_time = glm::sin((world_time + 3*SECONDS_PER_LIGHT_PHASE) * (2 * 3.14159 / (LIGHT_PHASES * SECONDS_PER_LIGHT_PHASE)));
        glm::vec3 sunlight_direction = Moon::GetCurrentMoon()->GetSunlightDirection();
        double ambient_light = 0.5 * sin_world_time;
        if (ambient_light < 0)
            ambient_light *= -0.5;
        double sunlight_factor = ambient_light + 0.5;

        double dot = glm::dot(sunlight_direction, normal);
        if (dot < 0)
            dot = 0;

        
        uint8_t _sky_light, _block_light;
        if (IsBorderBlock(GlobalToLocalVoxel(quad.base_coords)))
        {
            _sky_light = 15;
            _block_light = 0;
        }
        else
        {
            glm::ivec3 adjacent = GlobalToLocalVoxel(quad.base_coords) - glm::ivec3(normal); // Block in front of the one this quad is based at
            _sky_light = _lightmap.GetSkyLevel(adjacent);
            _block_light = _lightmap.GetBlockLevel(adjacent);
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
            float red_green = block_light / 100.0f + scaled_sky_light * sunlight_factor * 0.01f;
            if (red_green > 1.0f)
                red_green = 1.0f;

            float blue = block_light / 100.0f + scaled_sky_light * 0.01f;
            if (blue > 1.0f)
                blue = 1.0f;

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
