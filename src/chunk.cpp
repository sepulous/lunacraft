
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

Chunk::Chunk(glm::ivec3 coords)
{
    _coords = coords;
    _blocks = new uint16_t[CHUNK_SIZE * CHUNK_SIZE * WORLD_HEIGHT_LIMIT];
}

Chunk::Chunk(Chunk&& other) noexcept
{
    _is_border_chunk = other._is_border_chunk;
    _opaque_vao = other._opaque_vao;
    _opaque_vbo = other._opaque_vbo;
    _transparent_vao = other._transparent_vao;
    _transparent_vbo = other._transparent_vbo;
    _coords = other._coords;
    _opaque_vertices = std::move(other._opaque_vertices);
    _transparent_vertices = std::move(other._transparent_vertices);
    _blocks = other._blocks;
    other._blocks = nullptr;
}

Chunk& Chunk::operator=(Chunk&& other) noexcept
{
    if (this != &other)
    {
        _is_border_chunk = other._is_border_chunk;
        _opaque_vao = other._opaque_vao;
        _opaque_vbo = other._opaque_vbo;
        _transparent_vao = other._transparent_vao;
        _transparent_vbo = other._transparent_vbo;
        _coords = other._coords;
        _opaque_vertices = std::move(other._opaque_vertices);
        _transparent_vertices = std::move(other._transparent_vertices);
        _blocks = other._blocks;
        other._blocks = nullptr;
    }
    return *this;
}

void Chunk::SetIsBorderChunk(bool value)
{
    _is_border_chunk = value;
}

bool Chunk::IsBorderChunk()
{
    return _is_border_chunk;
}

glm::ivec3 Chunk::GetCoords()
{
    return _coords;
}

uint16_t *Chunk::GetBlocks()
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    // Transparents
    glGenVertexArrays(1, &_transparent_vao);
    glBindVertexArray(_transparent_vao);

    glGenBuffers(1, &_transparent_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _transparent_vbo);
    glBufferData(GL_ARRAY_BUFFER, _transparent_vertices.size() * sizeof(BlockVertex), _transparent_vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void Chunk::BuildVertices(std::vector<Chunk>& loaded_chunks)
{
    std::unordered_map<BlockID, glm::vec3> ATLAS_TILE_MAP = {
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
        {BlockID::sand,            glm::vec3(2, 2, 2)}, // NOTE: Not certain about this one
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

    std::unordered_map<BlockID, glm::mat3x2> TILE_ORIGINS;
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
        TILE_ORIGINS.insert({block_id, glm::mat3x2(
            top_tile_origin, side_tile_origin, bottom_tile_origin
        )});
    }

    std::vector<Chunk *> neighbor_chunks(4, nullptr); // {front, right, back, left}
    for (Chunk& chunk : loaded_chunks)
    {
        if (chunk._coords.x == _coords.x && chunk._coords.z == _coords.z + 1)
            neighbor_chunks[0] = &chunk;
        else if (chunk._coords.x == _coords.x + 1 && chunk._coords.z == _coords.z)
            neighbor_chunks[1] = &chunk;
        else if (chunk._coords.x == _coords.x && chunk._coords.z == _coords.z - 1)
            neighbor_chunks[2] = &chunk;
        else if (chunk._coords.x == _coords.x - 1 && chunk._coords.z == _coords.z)
            neighbor_chunks[3] = &chunk;
    }

    std::vector<BlockQuad> quads = GreedyMesh(_blocks, neighbor_chunks);

    for (BlockQuad quad : quads)
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
            base_pos = {quad.base.x + 0.5f + CHUNK_SIZE * _coords.x, quad.base.y - 0.5f, quad.base.z - 0.5f + CHUNK_SIZE * _coords.z};
        else if (normal.y != 0)
            base_pos = {quad.base.x - 0.5f + CHUNK_SIZE * _coords.x, quad.base.y + 0.5f, quad.base.z - 0.5f + CHUNK_SIZE * _coords.z};
        else
            base_pos = {quad.base.x - 0.5f + CHUNK_SIZE * _coords.x, quad.base.y - 0.5f, quad.base.z + 0.5f + CHUNK_SIZE * _coords.z};

        // Determine texture tiling repeats
        int quad_width = glm::length(quad.du);
        int quad_height = glm::length(quad.dv);

        if (quad.block == BlockID::light) // Trick to make light blocks unlit (lighting involves dot product)
            normal = {0, 0, 0};

        BlockVertex vert_1(base_pos,                     {0,          0,           tile_origin}, normal);
        BlockVertex vert_2(base_pos + quad.dv,           {0,          quad_height, tile_origin}, normal);
        BlockVertex vert_3(base_pos + quad.dv + quad.du, {quad_width, quad_height, tile_origin}, normal);
        BlockVertex vert_4(base_pos + quad.dv + quad.du, {quad_width, quad_height, tile_origin}, normal);
        BlockVertex vert_5(base_pos + quad.du,           {quad_width, 0,           tile_origin}, normal);
        BlockVertex vert_6(base_pos,                     {0,          0,           tile_origin}, normal);

        if (BlockIsOpaque(quad.block))
        {
            if (!quad.back_face)
            {
                _opaque_vertices.push_back(vert_1);
                _opaque_vertices.push_back(vert_2);
                _opaque_vertices.push_back(vert_3);
                _opaque_vertices.push_back(vert_4);
                _opaque_vertices.push_back(vert_5);
                _opaque_vertices.push_back(vert_6);
            }
            else
            {
                _opaque_vertices.push_back(vert_6);
                _opaque_vertices.push_back(vert_5);
                _opaque_vertices.push_back(vert_4);
                _opaque_vertices.push_back(vert_3);
                _opaque_vertices.push_back(vert_2);
                _opaque_vertices.push_back(vert_1);
            }
        }
        else
        {
            if (!quad.back_face)
            {
                _transparent_vertices.push_back(vert_1);
                _transparent_vertices.push_back(vert_2);
                _transparent_vertices.push_back(vert_3);
                _transparent_vertices.push_back(vert_4);
                _transparent_vertices.push_back(vert_5);
                _transparent_vertices.push_back(vert_6);
            }
            else
            {
                _transparent_vertices.push_back(vert_6);
                _transparent_vertices.push_back(vert_5);
                _transparent_vertices.push_back(vert_4);
                _transparent_vertices.push_back(vert_3);
                _transparent_vertices.push_back(vert_2);
                _transparent_vertices.push_back(vert_1);
            }
        }
    }
}
