
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <queue>

#include <glm/glm.hpp>

#include "chunk.h"
#include "shader.h"
#include "constants.h"
#include "helpers.h"
#include "block.h"
#include "mesher.h"

Chunk::Chunk()
{

}

Chunk::Chunk(glm::ivec3 coords)
{
    _coords = coords;
}

Chunk::Chunk(Chunk &&other) noexcept
{
    _opaque_vao = other._opaque_vao;
    _opaque_vbo = other._opaque_vbo;
    _transparent_vao = other._transparent_vao;
    _transparent_vbo = other._transparent_vbo;
    _coords = other._coords;
    _opaque_vertices = std::move(other._opaque_vertices);
    _transparent_vertices = std::move(other._transparent_vertices);
    _blocks = std::move(other._blocks);
}

Chunk &Chunk::operator=(Chunk &&other) noexcept
{
    if (this != &other)
    {
        _opaque_vao = other._opaque_vao;
        _opaque_vbo = other._opaque_vbo;
        _transparent_vao = other._transparent_vao;
        _transparent_vbo = other._transparent_vbo;
        _coords = other._coords;
        _opaque_vertices = std::move(other._opaque_vertices);
        _transparent_vertices = std::move(other._transparent_vertices);
        _blocks = std::move(other._blocks);
    }
    return *this;
}

Chunk::~Chunk()
{
    glDeleteVertexArrays(1, &_opaque_vao);
    glDeleteVertexArrays(1, &_transparent_vao);
    glDeleteBuffers(1, &_opaque_vbo);
    glDeleteBuffers(1, &_transparent_vbo);
}

glm::ivec3 Chunk::GetCoords()
{
    return _coords;
}

BlockArray &Chunk::GetBlocks()
{
    return _blocks;
}

void Chunk::RenderOpaques()
{
    glBindVertexArray(_opaque_vao);
    glDrawArrays(GL_TRIANGLES, 0, _opaque_vertices.size());
}

void Chunk::RenderTransparents()
{
    glBindVertexArray(_transparent_vao);
    glDrawArrays(GL_TRIANGLES, 0, _transparent_vertices.size());
}

void Chunk::BufferVertices()
{
    // Opaques
    glGenVertexArrays(1, &_opaque_vao);
    glBindVertexArray(_opaque_vao);

    glGenBuffers(1, &_opaque_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _opaque_vbo);
    glBufferData(GL_ARRAY_BUFFER, _opaque_vertices.size() * sizeof(BlockVertex), _opaque_vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);
    
    // Transparents
    glGenVertexArrays(1, &_transparent_vao);
    glBindVertexArray(_transparent_vao);

    glGenBuffers(1, &_transparent_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _transparent_vbo);
    glBufferData(GL_ARRAY_BUFFER, _transparent_vertices.size() * sizeof(BlockVertex), _transparent_vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(3);
}

void Chunk::SetCoords(glm::ivec3 coords)
{
    _coords = coords;
}

void Chunk::SetBlocks(BlockArray &blocks)
{
    _blocks = std::move(blocks);
}

void Chunk::SetOpaqueVertices(std::vector<BlockVertex> &opaque_vertices)
{
    _opaque_vertices = std::move(opaque_vertices);
}

void Chunk::SetTransparentVertices(std::vector<BlockVertex> &transparent_vertices)
{
    _transparent_vertices = std::move(transparent_vertices);
}

static std::array<float, BLOCKS_IN_CHUNK> BuildSkylightMap(BlockID *blocks);

void BuildChunkVertices(BlockID *blocks, glm::ivec3 chunk_coords, std::vector<BlockVertex> &opaque_vertices, std::vector<BlockVertex> &transparent_vertices)
{
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
        for (auto it = ATLAS_TILE_MAP.begin(); it != ATLAS_TILE_MAP.end(); it++)
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

    std::array<float, BLOCKS_IN_CHUNK> skylight_map = BuildSkylightMap(blocks);
    std::vector<BlockQuad> quads = GreedyMesh(blocks);

    for (BlockQuad &quad : quads)
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
            base_pos = {quad.base.x + 0.5f + CHUNK_SIZE * chunk_coords.x, quad.base.y - 0.5f, quad.base.z - 0.5f + CHUNK_SIZE * chunk_coords.z};
        else if (normal.y != 0)
            base_pos = {quad.base.x - 0.5f + CHUNK_SIZE * chunk_coords.x, quad.base.y + 0.5f, quad.base.z - 0.5f + CHUNK_SIZE * chunk_coords.z};
        else
            base_pos = {quad.base.x - 0.5f + CHUNK_SIZE * chunk_coords.x, quad.base.y - 0.5f, quad.base.z + 0.5f + CHUNK_SIZE * chunk_coords.z};

        // Determine texture tiling repeats
        int quad_width = glm::length(quad.du);
        int quad_height = glm::length(quad.dv);

        // Compute coordinates for skylight map
        auto base = quad.base_coords - glm::ivec3{normal.x, normal.y, normal.z};
        auto scaled_du = (1.0f - (1.0f / glm::length(quad.du))) * quad.du;
        auto scaled_dv = (1.0f - (1.0f / glm::length(quad.dv))) * quad.dv;
        auto base_du = base + glm::ivec3{scaled_du.x, scaled_du.y, scaled_du.z};
        auto base_dv = base + glm::ivec3{scaled_dv.x, scaled_dv.y, scaled_dv.z};
        auto base_du_dv = base + glm::ivec3{scaled_du.x + scaled_dv.x, scaled_du.y + scaled_dv.y, scaled_du.z + scaled_dv.z};

        // Push vertices
        auto &vertices = BlockIsOpaque(quad.block) ? opaque_vertices : transparent_vertices;
        if (!quad.back_face)
        {
            vertices.emplace_back(base_pos,                     glm::vec4{0,          0,           tile_origin}, normal, skylight_map[GetChunkIndex(base.x, base.y, base.z)]);
            vertices.emplace_back(base_pos + quad.dv,           glm::vec4{0,          quad_height, tile_origin}, normal, skylight_map[GetChunkIndex(base_dv.x, base_dv.y, base_dv.z)]);
            vertices.emplace_back(base_pos + quad.dv + quad.du, glm::vec4{quad_width, quad_height, tile_origin}, normal, skylight_map[GetChunkIndex(base_du_dv.x, base_du_dv.y, base_du_dv.z)]);
            vertices.emplace_back(base_pos + quad.dv + quad.du, glm::vec4{quad_width, quad_height, tile_origin}, normal, skylight_map[GetChunkIndex(base_du_dv.x, base_du_dv.y, base_du_dv.z)]);
            vertices.emplace_back(base_pos + quad.du,           glm::vec4{quad_width, 0,           tile_origin}, normal, skylight_map[GetChunkIndex(base_du.x, base_du.y, base_du.z)]);
            vertices.emplace_back(base_pos,                     glm::vec4{0,          0,           tile_origin}, normal, skylight_map[GetChunkIndex(base.x, base.y, base.z)]);
        }
        else
        {
            vertices.emplace_back(base_pos,                     glm::vec4{0,          0,           tile_origin}, normal, skylight_map[GetChunkIndex(base.x, base.y, base.z)]);
            vertices.emplace_back(base_pos + quad.du,           glm::vec4{quad_width, 0,           tile_origin}, normal, skylight_map[GetChunkIndex(base_du.x, base_du.y, base_du.z)]);
            vertices.emplace_back(base_pos + quad.dv + quad.du, glm::vec4{quad_width, quad_height, tile_origin}, normal, skylight_map[GetChunkIndex(base_du_dv.x, base_du_dv.y, base_du_dv.z)]);
            vertices.emplace_back(base_pos + quad.dv + quad.du, glm::vec4{quad_width, quad_height, tile_origin}, normal, skylight_map[GetChunkIndex(base_du_dv.x, base_du_dv.y, base_du_dv.z)]);
            vertices.emplace_back(base_pos + quad.dv,           glm::vec4{0,          quad_height, tile_origin}, normal, skylight_map[GetChunkIndex(base_dv.x, base_dv.y, base_dv.z)]);
            vertices.emplace_back(base_pos,                     glm::vec4{0,          0,           tile_origin}, normal, skylight_map[GetChunkIndex(base.x, base.y, base.z)]);
        }
    }
}

static std::array<float, BLOCKS_IN_CHUNK> BuildSkylightMap(BlockID *blocks)
{
    std::array<float, BLOCKS_IN_CHUNK> skylight_map;
    std::queue<glm::ivec3> to_expand;

    // Vertical fill
    for (int x = 0; x <= CHUNK_SIZE + 1; x++)
    {
        for (int z = 0; z <= CHUNK_SIZE + 1; z++)
        {
            bool blocked = false;
            for (int y = WORLD_HEIGHT_LIMIT - 1; y >= 0; y--)
            {
                int chunk_index = GetChunkIndex(x, y, z);

                if (BlockIsOpaque(blocks[chunk_index]))
                    blocked = true;

                skylight_map[chunk_index] = blocked ? 0.0f : 15.0f;

                if (!blocked && x != 0 && z != 0 && x != CHUNK_SIZE + 1 && z != CHUNK_SIZE + 1)
                    to_expand.emplace(x, y, z);
            }
        }
    }

    // Outward expansion
    while (!to_expand.empty())
    {
        glm::ivec3 coords = to_expand.front();
        to_expand.pop();

        float this_sky_light = skylight_map[GetChunkIndex(coords.x, coords.y, coords.z)];
        
        glm::ivec3 neighbor_coords[] = {
            {coords.x - 1, coords.y, coords.z},
            {coords.x, coords.y - 1, coords.z},
            {coords.x, coords.y, coords.z - 1},
            {coords.x + 1, coords.y, coords.z},
            {coords.x, coords.y + 1, coords.z},
            {coords.x, coords.y, coords.z + 1}
        };

        for (glm::ivec3 &neighbor_coord : neighbor_coords)
        {
            if (neighbor_coord.x == 0 || neighbor_coord.x == CHUNK_SIZE + 1 || neighbor_coord.z == 0 || neighbor_coord.x == CHUNK_SIZE + 1 || neighbor_coord.y == -1 || neighbor_coord.y == WORLD_HEIGHT_LIMIT)
                continue;

            int neighbor_chunk_index = GetChunkIndex(neighbor_coord.x, neighbor_coord.y, neighbor_coord.z);
            if (!BlockIsOpaque(blocks[neighbor_chunk_index]))
            {
                auto propagated = this_sky_light - 1.0f;
                auto &neighbor_sky_light = skylight_map[neighbor_chunk_index];
                if (propagated > neighbor_sky_light)
                {
                    neighbor_sky_light = propagated;
                    to_expand.emplace(neighbor_coord.x, neighbor_coord.y, neighbor_coord.z);
                }
            }
        }
    }

    return skylight_map;
}
