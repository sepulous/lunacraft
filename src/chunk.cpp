
#include <cstdint>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>

#include "chunk.h"
#include "shader.h"
#include "constants.h"
#include "helpers.h"
#include "block.h"

Chunk::Chunk(glm::vec2 position)
{
    _position = position;
    _blocks = new uint16_t[(CHUNK_SIZE + 2) * (CHUNK_SIZE + 2) * WORLD_HEIGHT_LIMIT];
}

glm::vec2 Chunk::GetPosition()
{
    return _position;
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

void Chunk::BuildVertices()
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

    struct Quad
    {
        BlockID block_type;
        glm::vec3 vert_1;
        glm::vec3 vert_2;
        float length; // along +z
        float height; // along +y
        int face;
    };

    glm::vec2 NEIGHBOR_OFFSETS[] = {
        glm::vec2(0, 1),
        glm::vec2(1, 0),
        glm::vec2(0, -1),
        glm::vec2(-1, 0)
    };

    glm::mat2x3 VERTEX_OFFSETS[] = {
        glm::mat2x3(
            -0.5f, 0.5f, -0.5f, // top (+y) left
            0.5f, 0.5f, -0.5f // top (+y) right
        ),
        glm::mat2x3(
            0.5f, -0.5f, -0.5f, // left (+x) bottom (v1)
            0.5f, 0.5f, -0.5f // left (+x) top (v2)
        ),
        glm::mat2x3(
            -0.5f, -0.5f, -0.5f, // bottom (-y) left
            0.5f, -0.5f, -0.5f // bottom (-y) right
        ),
        glm::mat2x3(
            -0.5f, -0.5f, -0.5f, // right (-x) bottom
            -0.5f, 0.5f, -0.5f // right (-x) top
        )
    };

    for (int localBlockX = 1; localBlockX <= CHUNK_SIZE; localBlockX++)
    {
        for (int localBlockY = 1; localBlockY < WORLD_HEIGHT_LIMIT - 1; localBlockY++) // TODO: Fix these indices (just avoiding out-of-bounds in neighbor checks)
        {
            BlockID base_block = BlockID::unknown;

            std::vector<Quad> all_quads;
            Quad new_quads[] = {
                {BlockID::unknown, glm::vec3(0), glm::vec3(0), 0, 0, 0}, // top (+y)
                {BlockID::unknown, glm::vec3(0), glm::vec3(0), 0, 0, 1}, // left (+x)
                {BlockID::unknown, glm::vec3(0), glm::vec3(0), 0, 0, 2}, // bottom (-y)
                {BlockID::unknown, glm::vec3(0), glm::vec3(0), 0, 0, 3}  // right (-x)
            };

            for (int localBlockZ = 1; localBlockZ <= CHUNK_SIZE; localBlockZ++)
            {
                BlockID current_block = static_cast<BlockID>(_blocks[GetChunkIndex(localBlockX, localBlockY, localBlockZ)]);

                glm::vec3 block_world_pos = glm::vec3(
                    _position.x * CHUNK_SIZE + (localBlockX - 1),
                    localBlockY,
                    _position.y * CHUNK_SIZE + (localBlockZ - 1)
                );

                if (base_block == BlockID::unknown)
                {
                    if (current_block != BlockID::air)
                    {
                        base_block = current_block;

                        for (int i = 0; i < 4; i++) // top -> right -> bottom -> left
                        {
                            glm::vec2 neighbor_offset = NEIGHBOR_OFFSETS[i];
                            glm::mat2x3 vertex_offsets = VERTEX_OFFSETS[i];

                            BlockID neighbor = static_cast<BlockID>(_blocks[GetChunkIndex(localBlockX + neighbor_offset.x, localBlockY + neighbor_offset.y, localBlockZ)]);
                            bool face_visible = ShouldRenderFace(base_block, neighbor);
                            if (face_visible)
                            {
                                new_quads[i].vert_1 = block_world_pos + vertex_offsets[0];
                                new_quads[i].vert_2 = block_world_pos + vertex_offsets[1];
                                new_quads[i].length = 1;
                            }
                            new_quads[i].block_type = base_block;
                        }
                    }
                }
                else
                {
                    for (int i = 0; i < 4; i++) // top -> right -> bottom -> left
                    {
                        glm::vec2 neighbor_offset = NEIGHBOR_OFFSETS[i];
                        glm::mat2x3 vertex_offsets = VERTEX_OFFSETS[i];

                        BlockID neighbor = static_cast<BlockID>(_blocks[GetChunkIndex(localBlockX + neighbor_offset.x, localBlockY + neighbor_offset.y, localBlockZ)]);
                        bool face_visible = ShouldRenderFace(current_block, neighbor);
                        if (current_block == base_block && face_visible)
                        {
                            if (new_quads[i].length == 0)
                            {
                                new_quads[i].vert_1 = block_world_pos + vertex_offsets[0];
                                new_quads[i].vert_2 = block_world_pos + vertex_offsets[1];
                            }
                            new_quads[i].length++;
                        }
                        else
                        {
                            if (new_quads[i].length > 0)
                            {
                                all_quads.push_back(new_quads[i]);
                                new_quads[i].length = 0;
                            }

                            if (face_visible && current_block != BlockID::air)
                            {
                                new_quads[i].vert_1 = block_world_pos + vertex_offsets[0];
                                new_quads[i].vert_2 = block_world_pos + vertex_offsets[1];
                                new_quads[i].length = 1;
                            }
                        }
                    }

                    if (base_block != current_block && current_block != BlockID::air)
                    {
                        base_block = current_block;
                        for (int i = 0; i < 4; i++)
                            new_quads[i].block_type = base_block;
                    }
                }

                BlockID behind = static_cast<BlockID>(_blocks[GetChunkIndex(localBlockX, localBlockY, localBlockZ - 1)]);
                bool face_visible = ShouldRenderFace(current_block, behind);
                if (current_block != BlockID::air && face_visible)
                {
                    all_quads.push_back({
                        current_block,
                        block_world_pos + glm::vec3(-0.5f, -0.5f, -0.5f),
                        block_world_pos + glm::vec3(0.5f, -0.5f, -0.5f),
                        0, // length = 0
                        1, // height = 1
                        5 // back
                    });
                }

                BlockID in_front = static_cast<BlockID>(_blocks[GetChunkIndex(localBlockX, localBlockY, localBlockZ + 1)]);
                face_visible = ShouldRenderFace(current_block, in_front);
                if (current_block != BlockID::air && face_visible)
                {
                    all_quads.push_back({
                        current_block,
                        block_world_pos + glm::vec3(-0.5f, -0.5f, +0.5f),
                        block_world_pos + glm::vec3(0.5f, -0.5f, +0.5f),
                        0, // length = 0
                        1, // height = 1
                        4 // front
                    });
                }
            }

            // Push remaining valid quads
            for (Quad quad : new_quads)
                if (quad.length > 0)
                    all_quads.push_back(quad);

            // Push quad vertices
            constexpr unsigned int block_indices[] = {
                11, 2, 1, 1, 12, 11, // +y
                7, 8, 9, 9, 8, 10, // +x
                0, 1, 2, 3, 1, 0, // -y
                1, 4, 5, 5, 6, 1, // -x
                1, 2, 11, 11, 12, 1, // +z
                11, 2, 1, 1, 12, 11, // -z
            };

            glm::vec3 face_normals[] = {
                glm::vec3(0, 1, 0),
                glm::vec3(1, 0, 0),
                glm::vec3(0, -1, 0),
                glm::vec3(-1, 0, 0),
                glm::vec3(0, 0, 1),
                glm::vec3(0, 0, -1)
            };

            for (Quad quad : all_quads)
            {
                int side = quad.face == 0 ? 0 : (quad.face == 2 ? 2 : 1);
                glm::vec2 tile_origin = TILE_ORIGINS[quad.block_type][side];

                BlockVertex block_vertices[] = {
                    BlockVertex(quad.vert_2 + glm::vec3(0, 0, quad.length),           glm::vec4(1.0f, quad.length, tile_origin),               glm::vec3(0)), // 0
                    BlockVertex(quad.vert_1,                                          glm::vec4(0.0f, 0.0f, tile_origin),                      glm::vec3(0)), // 1
                    BlockVertex(quad.vert_2,                                          glm::vec4(1.0f, 0.0f, tile_origin),                      glm::vec3(0)), // 2
                    BlockVertex(quad.vert_1 + glm::vec3(0, 0, quad.length),           glm::vec4(0.0f, quad.length, tile_origin),               glm::vec3(0)), // 3
                    BlockVertex(quad.vert_1 + glm::vec3(0, 0, quad.length),           glm::vec4(quad.length, 0.0f, tile_origin),               glm::vec3(0)), // 4
                    BlockVertex(quad.vert_2 + glm::vec3(0, 0, quad.length),           glm::vec4(quad.length, 1.0f, tile_origin),               glm::vec3(0)), // 5
                    BlockVertex(quad.vert_2,                                          glm::vec4(0.0f, 1.0f, tile_origin),                      glm::vec3(0)), // 6
                    BlockVertex(quad.vert_1,                                          glm::vec4(quad.length, 0.0f, tile_origin),               glm::vec3(0)), // 7
                    BlockVertex(quad.vert_2,                                          glm::vec4(quad.length, 1.0f, tile_origin),               glm::vec3(0)), // 8
                    BlockVertex(quad.vert_1 + glm::vec3(0, 0, quad.length),           glm::vec4(0.0f, 0.0f, tile_origin),                      glm::vec3(0)), // 9
                    BlockVertex(quad.vert_2 + glm::vec3(0, 0, quad.length),           glm::vec4(0.0f, 1.0f, tile_origin),                      glm::vec3(0)), // 10
                    BlockVertex(quad.vert_2 + glm::vec3(0, quad.height, quad.length), glm::vec4(1.0f, quad.length + quad.height, tile_origin), glm::vec3(0)), // 11
                    BlockVertex(quad.vert_1 + glm::vec3(0, quad.height, quad.length), glm::vec4(0.0f, quad.length + quad.height, tile_origin), glm::vec3(0)), // 12
                };
                
                int index_start = quad.face * 6;
                for (int i = index_start; i < index_start + 6; i++)
                {
                    if (quad.block_type == BlockID::light)
                        block_vertices[block_indices[i]].face_normal = glm::vec3(0); // Just a hack so light blocks are "unlit"
                    else
                        block_vertices[block_indices[i]].face_normal = face_normals[quad.face];

                    if (BlockIsOpaque(quad.block_type))
                        _opaque_vertices.push_back(block_vertices[block_indices[i]]);
                    else
                        _transparent_vertices.push_back(block_vertices[block_indices[i]]);
                }
            }
        }
    }
}
