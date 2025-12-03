#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ui.h"
#include "shader.h"
#include "soundlib.h"
#include "storage.h"
#include "input.h"
#include "options.h"
#include "player.h"
#include "chunk.h"
#include "chunk_gen.h"
#include "block.h"
#include "skybox.h"
#include "constants.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image/stb_image.h>

static glm::vec2 viewport = {1280, 720};
static glm::mat4 ui_window_to_virtual = glm::mat4(1.0f);
static glm::mat4 ui_virtual_to_window = glm::mat4(1.0f);

enum class GameState {MAIN_MENU, IN_GAME};
static GameState game_state = GameState::MAIN_MENU;
static bool loading_moon = false;
static float loading_moon_progress = 0; // TODO: Progress bar
static MouseState mouse_state;

static std::vector<Chunk> loaded_chunks;

static Player player;
static float last_collision_check = 0;
static float last_fixed_update = 0;

GLuint water_vao;
std::vector<BlockVertex> water_vertices;

GLuint crystal_vao;
std::vector<BlockVertex> crystal_vertices;

float pitch = 0;
float yaw = -90;
float last_mouse_x = 1280.0f / 2.0f;
float last_mouse_y = 720.0f / 2.0f;
const float camera_speed = 8.0f;
const float camera_sensitivity = 0.05f;
void mouse_callback(GLFWwindow *window, double x_pos, double y_pos)
{
    float x_offset = x_pos - last_mouse_x;
    float y_offset = last_mouse_y - y_pos; // Reversed since y ranges from bottom to top
    last_mouse_x = x_pos;
    last_mouse_y = y_pos;
    yaw += x_offset * camera_sensitivity;
    pitch += y_offset * camera_sensitivity;
    pitch = glm::clamp(pitch, -89.8f, 89.8f);

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    player.camera.forward = glm::normalize(direction);
    player.camera.right = glm::normalize(glm::cross(player.camera.forward, player.camera.up));
}

// NOTE: Must compile shaders before calling this!
void UIRescale()
{
    glm::mat4 proj = glm::ortho(0.0f, viewport.x, 0.0f, viewport.y, -1.0f, 1.0f);
    glm::mat4 ui_matrix = proj * ui_virtual_to_window;

    ShaderManager::UI_IMAGE_SHADER.Use();
    glUniformMatrix4fv(glGetUniformLocation(ShaderManager::UI_IMAGE_SHADER.GetID(), "ui_matrix"), 1, GL_FALSE, glm::value_ptr(ui_matrix));

    ShaderManager::UI_TEXT_SHADER.Use();
    glUniformMatrix4fv(glGetUniformLocation(ShaderManager::UI_TEXT_SHADER.GetID(), "ui_matrix"), 1, GL_FALSE, glm::value_ptr(ui_matrix));
}

void UIUpdateTransforms()
{
    float scale = std::max(viewport.x / VIRTUAL_UI_WIDTH, viewport.y / VIRTUAL_UI_HEIGHT);
    float scaled_virtual_width = VIRTUAL_UI_WIDTH * scale;
    float scaled_virtual_height = VIRTUAL_UI_HEIGHT * scale;
    float offset_x = (viewport.x - scaled_virtual_width)  * 0.5f;
    float offset_y = (viewport.y - scaled_virtual_height) * 0.5f;
    ui_virtual_to_window = glm::mat4(1.0f);
    ui_virtual_to_window = glm::translate(ui_virtual_to_window, glm::vec3(offset_x, offset_y, 0.0f));
    ui_virtual_to_window = glm::scale(ui_virtual_to_window, glm::vec3(scale, scale, 1.0f));
    ui_window_to_virtual = glm::inverse(ui_virtual_to_window);
}

static void _CreateMoon(int moon, MoonSettings moon_settings)
{
    constexpr size_t BLOCKS_IN_CHUNK = (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2) * WORLD_HEIGHT_LIMIT;
    int render_distance = OptionsManager::GetOptions().render_distance;
    int chunks_to_process = (2*render_distance + 1) * (2*render_distance + 1);
    int chunks_processed = 0;

    std::filesystem::path moon_dir = Storage::MOON_DIR / (std::string("moon") + std::to_string(moon));
    std::filesystem::path chunk_dir = moon_dir / "chunks";
    if (!std::filesystem::exists(moon_dir))
        std::filesystem::create_directory(moon_dir);
    if (!std::filesystem::exists(chunk_dir))
        std::filesystem::create_directory(chunk_dir);

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

    struct Quad
    {
        BlockID block_type;
        glm::vec3 vert_1;
        glm::vec3 vert_2;
        float length; // along +z
        float height; // along +y
        int face;
    };

    for (int chunk_x = -render_distance; chunk_x <= render_distance; chunk_x++)
    {
        for (int chunk_z = -render_distance; chunk_z <= render_distance; chunk_z++)
        {
            // Create chunk data
            Chunk chunk;
            chunk.position = {chunk_x, chunk_z};
            chunk.chunk_data = new uint16_t[BLOCKS_IN_CHUNK];
            GenerateChunk(chunk.chunk_data, chunk_x, chunk_z, moon_settings.seed);

            // Save chunk to file
            uint64_t chunk_id = CombineChunkCoordinates(chunk_x, chunk_z);
            std::filesystem::path chunk_path = chunk_dir / (std::to_string(chunk_id) + ".chunk");
            std::ofstream chunk_file(chunk_path, std::ios::binary);
            chunk_file.write(reinterpret_cast<char *>(chunk.chunk_data), BLOCKS_IN_CHUNK * sizeof(uint16_t));
            chunk_file.close();

            // Generate vertex data
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
                        BlockID current_block = static_cast<BlockID>(chunk.chunk_data[GetChunkIndex(localBlockX, localBlockY, localBlockZ)]);

                        glm::vec3 block_world_pos = glm::vec3(
                            chunk.position.x * CHUNK_SIZE + (localBlockX - 1),
                            localBlockY,
                            chunk.position.y * CHUNK_SIZE + (localBlockZ - 1)
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

                                    BlockID neighbor = static_cast<BlockID>(chunk.chunk_data[GetChunkIndex(localBlockX + neighbor_offset.x, localBlockY + neighbor_offset.y, localBlockZ)]);
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

                                BlockID neighbor = static_cast<BlockID>(chunk.chunk_data[GetChunkIndex(localBlockX + neighbor_offset.x, localBlockY + neighbor_offset.y, localBlockZ)]);
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

                        BlockID behind = static_cast<BlockID>(chunk.chunk_data[GetChunkIndex(localBlockX, localBlockY, localBlockZ - 1)]);
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

                        BlockID in_front = static_cast<BlockID>(chunk.chunk_data[GetChunkIndex(localBlockX, localBlockY, localBlockZ + 1)]);
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
                            BlockVertex(quad.vert_2 + glm::vec3(0, 0, quad.length),           glm::vec2(1.0f, quad.length),               tile_origin, glm::vec3(0)), // 0
                            BlockVertex(quad.vert_1,                                          glm::vec2(0.0f, 0.0f),                      tile_origin, glm::vec3(0)), // 1
                            BlockVertex(quad.vert_2,                                          glm::vec2(1.0f, 0.0f),                      tile_origin, glm::vec3(0)), // 2
                            BlockVertex(quad.vert_1 + glm::vec3(0, 0, quad.length),           glm::vec2(0.0f, quad.length),               tile_origin, glm::vec3(0)), // 3
                            BlockVertex(quad.vert_1 + glm::vec3(0, 0, quad.length),           glm::vec2(quad.length, 0.0f),               tile_origin, glm::vec3(0)), // 4
                            BlockVertex(quad.vert_2 + glm::vec3(0, 0, quad.length),           glm::vec2(quad.length, 1.0f),               tile_origin, glm::vec3(0)), // 5
                            BlockVertex(quad.vert_2,                                          glm::vec2(0.0f, 1.0f),                      tile_origin, glm::vec3(0)), // 6
                            BlockVertex(quad.vert_1,                                          glm::vec2(quad.length, 0.0f),               tile_origin, glm::vec3(0)), // 7
                            BlockVertex(quad.vert_2,                                          glm::vec2(quad.length, 1.0f),               tile_origin, glm::vec3(0)), // 8
                            BlockVertex(quad.vert_1 + glm::vec3(0, 0, quad.length),           glm::vec2(0.0f, 0.0f),                      tile_origin, glm::vec3(0)), // 9
                            BlockVertex(quad.vert_2 + glm::vec3(0, 0, quad.length),           glm::vec2(0.0f, 1.0f),                      tile_origin, glm::vec3(0)), // 10
                            BlockVertex(quad.vert_2 + glm::vec3(0, quad.height, quad.length), glm::vec2(1.0f, quad.length + quad.height), tile_origin, glm::vec3(0)), // 11
                            BlockVertex(quad.vert_1 + glm::vec3(0, quad.height, quad.length), glm::vec2(0.0f, quad.length + quad.height), tile_origin, glm::vec3(0)), // 12
                        };
                        
                        int index_start = quad.face * 6;
                        for (int i = index_start; i < index_start + 6; i++)
                        {
                            if (quad.block_type == BlockID::light)
                                block_vertices[block_indices[i]].face_normal = glm::vec3(0); // Just a hack so light blocks are "unlit"
                            else
                                block_vertices[block_indices[i]].face_normal = face_normals[quad.face];

                            if (quad.block_type == BlockID::water)
                                water_vertices.push_back(block_vertices[block_indices[i]]);
                            else if (quad.block_type == BlockID::sulphur_crystal || quad.block_type == BlockID::boron_crystal || quad.block_type == BlockID::blue_crystal)
                                crystal_vertices.push_back(block_vertices[block_indices[i]]);
                            else
                                chunk.vertices.push_back(block_vertices[block_indices[i]]);
                        }
                    }
                }
            }
            loaded_chunks.push_back(chunk);

            chunks_processed++;
            loading_moon_progress = (float)chunks_processed / (float)chunks_to_process;
        }
    }
}

void CreateMoon(int moon, MoonSettings moon_settings)
{
    loading_moon = true;
    std::thread worker(_CreateMoon, moon, moon_settings);
    worker.detach();
}

static void _LoadMoon(int moon)
{

}

void LoadMoon(int moon)
{
    loading_moon = true;
    std::thread worker(_LoadMoon, moon);
    worker.detach();
}

bool TestAABBWorld(const AABB& box)
{
    float minX = glm::round(box.center.x - box.extents.x);
    float maxX = glm::round(box.center.x + box.extents.x);
    float minY = glm::round(box.center.y - box.extents.y);
    float maxY = glm::round(box.center.y + box.extents.y);
    float minZ = glm::round(box.center.z - box.extents.z);
    float maxZ = glm::round(box.center.z + box.extents.z);

    for (int x = minX; x <= maxX; x++)
    {
        for (int y = minY; y <= maxY; y++)
        {
            for (int z = minZ; z <= maxZ; z++)
            {
                int chunk_x = glm::floor((float)x / CHUNK_SIZE);
                int chunk_z = glm::floor((float)z / CHUNK_SIZE);
                for (Chunk& chunk : loaded_chunks)
                {
                    if ((int)chunk.position.x == chunk_x && (int)chunk.position.y == chunk_z)
                    {
                        glm::vec3 local_block_pos = GetLocalBlockPos(glm::vec3(x, y, z));
                        if ((BlockID)chunk.chunk_data[GetChunkIndex(local_block_pos.x, local_block_pos.y, local_block_pos.z)] != BlockID::air)
                            return true;
                        break;
                    }
                }
            }
        }
    }

    return false;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #if __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWwindow *window = glfwCreateWindow(viewport.x, viewport.y, "Lunacraft", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
        viewport = {width, height};
        UIUpdateTransforms();
        UIRescale();
    });
    glViewport(0, 0, viewport.x, viewport.y);
    UIUpdateTransforms();

    Storage::Init();
    OptionsManager::Init();
    ShaderManager::CompileAllShaders();
    Soundlib::Init();

    UIRescale();
    UIMainMenu ui_main_menu(window);
    UILoadMoonMenu ui_load_moon_menu;

    Skybox skybox;

    /////////////////////////////////////////////////////////
    // Load texture atlas
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    std::filesystem::path atlas_path = Storage::IMAGE_DIR / "texture_atlas.png";
    unsigned char *texture_atlas_data = stbi_load(atlas_path.c_str(), &width, &height, &nrChannels, 0);

    unsigned int texture_atlas;
    glGenTextures(1, &texture_atlas);

    glBindTexture(GL_TEXTURE_2D, texture_atlas);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_atlas_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(texture_atlas_data);
    /////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////

    // Water rendering data
    unsigned int water_vao;
    glGenVertexArrays(1, &water_vao);
    glBindVertexArray(water_vao);

    unsigned int water_vbo;
    glGenBuffers(1, &water_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, water_vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Crystal rendering data
    unsigned int crystal_vao;
    glGenVertexArrays(1, &crystal_vao);
    glBindVertexArray(crystal_vao);

    unsigned int crystal_vbo;
    glGenBuffers(1, &crystal_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, crystal_vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(3);

    /////////////////////////////////////////////////////////

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float delta_time;
    float last_frame_time = 0;
    const float fixed_delta_time = 0.02f;
    float accumulator = 0;
    while (!glfwWindowShouldClose(window))
    {
        float current_time = glfwGetTime();
        delta_time = current_time - last_frame_time;
        last_frame_time = current_time;

        //
        // Input
        //

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            if (!mouse_state.left_clicked && !mouse_state.left_held)
            {
                mouse_state.left_clicked = true;
            }
            else if (mouse_state.left_clicked && !mouse_state.left_held)
            {
                mouse_state.left_clicked = false;
                mouse_state.left_held = true;
            }
        }
        else
        {
            mouse_state.left_clicked = false;
            mouse_state.left_held = false;
        }

        // Cursor position is unbounded (and thus meaningless) when disabled. We don't care about the cursor position in that case anyway.
        if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
        {
            double mouse_x, mouse_y;
            glfwGetCursorPos(window, &mouse_x, &mouse_y);

            glm::vec4 virtual_mouse = ui_window_to_virtual * glm::vec4(mouse_x, viewport.y - mouse_y, 0.0f, 1.0f);
            mouse_state.position.x = virtual_mouse.x;
            mouse_state.position.y = virtual_mouse.y;
        }

        if (game_state == GameState::IN_GAME)
        {
            player.input_direction = glm::vec3(0);
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                player.input_direction += glm::vec3(player.camera.forward.x, 0, player.camera.forward.z);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                player.input_direction -= glm::vec3(player.camera.forward.x, 0, player.camera.forward.z);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                player.input_direction -= player.camera.right;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                player.input_direction += player.camera.right;
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && player.is_grounded)
                player.is_jumping = true;
        }

        //
        // Update/Render
        //

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (game_state == GameState::MAIN_MENU)
        {
            if (!loading_moon)
            {
                ui_main_menu.Update(delta_time, mouse_state);
                ui_main_menu.Render(delta_time);
            }
            else
            {
                ui_main_menu.Render(delta_time);
                ui_load_moon_menu.Render();
                if (loading_moon_progress >= 1.0f) // Only runs once when loading in
                {
                    // Buffer water data
                    glBindBuffer(GL_ARRAY_BUFFER, water_vbo);
                    glBufferData(GL_ARRAY_BUFFER, water_vertices.size() * sizeof(BlockVertex), water_vertices.data(), GL_STATIC_DRAW);

                    // Buffer crystal data
                    glBindBuffer(GL_ARRAY_BUFFER, crystal_vbo);
                    glBufferData(GL_ARRAY_BUFFER, crystal_vertices.size() * sizeof(BlockVertex), crystal_vertices.data(), GL_STATIC_DRAW);

                    // Buffer opaque data
                    for (Chunk& chunk : loaded_chunks)
                    {
                        // VAO
                        glGenVertexArrays(1, &chunk.vao);
                        glBindVertexArray(chunk.vao);

                        // VBO
                        glGenBuffers(1, &chunk.vbo);
                        glBindBuffer(GL_ARRAY_BUFFER, chunk.vbo);
                        glBufferData(GL_ARRAY_BUFFER, chunk.vertices.size() * sizeof(BlockVertex), chunk.vertices.data(), GL_STATIC_DRAW);

                        // BlockVertex attribute pointers
                        // Position
                        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)0);
                        glEnableVertexAttribArray(0);
                        // UV coords
                        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(3 * sizeof(float)));
                        glEnableVertexAttribArray(1);
                        // Tile index
                        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(5 * sizeof(float)));
                        glEnableVertexAttribArray(2);
                        // Face normal
                        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(7 * sizeof(float)));
                        glEnableVertexAttribArray(3);
                    }
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    if (glfwRawMouseMotionSupported())
                        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); // More natural mouse motion
                    glfwSetCursorPosCallback(window, mouse_callback);
                    game_state = GameState::IN_GAME;
                }
            }
        }
        else // IN_GAME
        {
            accumulator += delta_time;

            // Physics updates
            if (accumulator >= fixed_delta_time)
                player.FixedUpdate();

            // Resolve collisions
            glm::vec3 next_position = player.position;
            while (accumulator >= fixed_delta_time)
            {
                player.prev_position = player.position;

                // Gravity
                player.velocity.y -= 4.0f * fixed_delta_time;

                // X
                next_position.x += player.velocity.x * fixed_delta_time;
                player.aabb.center.x = next_position.x;
                if (TestAABBWorld(player.aabb))
                {
                    next_position.x = player.position.x; // Don't actually move
                    player.aabb.center.x = player.position.x;
                    player.velocity.x = 0; // So that if we loop again, the entity can't repeat the collision
                }

                // Z
                next_position.z += player.velocity.z * fixed_delta_time;
                player.aabb.center.z = next_position.z;
                if (TestAABBWorld(player.aabb))
                {
                    next_position.z = player.position.z; // Don't actually move
                    player.aabb.center.z = player.position.z;
                    player.velocity.z = 0; // If we loop again, the player can't repeat the collision
                }

                // Y
                next_position.y += player.velocity.y * fixed_delta_time;
                player.aabb.center.y = next_position.y;
                if (TestAABBWorld(player.aabb))
                {
                    if (player.velocity.y <= 0)
                        player.is_grounded = true;
                    next_position.y = player.position.y; // Don't actually move
                    player.aabb.center.y = player.position.y;
                    player.velocity.y = 0; // If we loop again, the player can't repeat the collision
                }
                else
                {
                    player.is_grounded = false;
                }

                player.next_position = next_position;

                accumulator -= fixed_delta_time;
            }

            // Interpolate position
            float alpha = accumulator / fixed_delta_time;
            player.position = glm::mix(player.prev_position, player.next_position, alpha);
            player.aabb.center = player.position;

            // Non-physics updates
            player.Update();

            //
            // Render chunks
            //

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);

            glm::mat4 view;
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), viewport.x / viewport.y, 0.1f, 300.0f);

            Shader block_shader = ShaderManager::BLOCK_SHADER;
            block_shader.Use();

            // Render opaques
            view = glm::lookAt(player.camera.position, player.camera.position + player.camera.forward, player.camera.up);
            glUniformMatrix4fv(glGetUniformLocation(block_shader.GetID(), "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(block_shader.GetID(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            glm::vec3 main_light = glm::normalize(glm::vec3(1.0f, -1.0f, 0.0f));
            glUniform3f(glGetUniformLocation(block_shader.GetID(), "MainLight"), 1.0f, -1.0f, 0.0f);

            glm::vec3 camera_forward_xz_normalized = glm::normalize(glm::vec3(player.camera.forward.x, 0, player.camera.forward.z));
            glm::vec3 camera_position_xz = glm::vec3(player.camera.position.x, 0, player.camera.position.z) - (float)CHUNK_SIZE * camera_forward_xz_normalized;
            for (Chunk& chunk : loaded_chunks)
            {
                glm::vec3 chunk_world_position = glm::vec3(CHUNK_SIZE * (chunk.position.x + 0.5f), 0, CHUNK_SIZE * (chunk.position.y + 0.5f));
                glm::vec3 displacement_xz_normalized = glm::normalize(chunk_world_position - camera_position_xz);
                float cos_angle = glm::dot(displacement_xz_normalized, camera_forward_xz_normalized);
                if (cos_angle >= glm::cos(glm::radians(45.0f)))
                {
                    glBindVertexArray(chunk.vao);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, texture_atlas);
                    glDrawArrays(GL_TRIANGLES, 0, chunk.vertices.size());
                    glBindVertexArray(0);
                }
            }

            // Render water
            glBindVertexArray(water_vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_atlas);
            glDrawArrays(GL_TRIANGLES, 0, water_vertices.size());
            glBindVertexArray(0);

            // Render crystals
            glBindVertexArray(crystal_vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_atlas);
            glDrawArrays(GL_TRIANGLES, 0, crystal_vertices.size());
            glBindVertexArray(0);

            //
            // Render skybox
            //

            view = glm::mat4(glm::mat3(glm::lookAt(player.camera.position, player.camera.position + player.camera.forward, player.camera.up)));
            float skybox_angle = 0.02f * current_time;
            skybox.Update(projection * view, skybox_angle);
            skybox.Render();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    OptionsManager::SaveOptions();
    Soundlib::Exit();
    glfwTerminate();

    return 0;
}
