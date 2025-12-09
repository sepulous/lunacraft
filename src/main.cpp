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
#include "helpers.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image/stb_image.h>

static glm::vec2 viewport = {1280, 720};
static glm::mat4 ui_window_to_virtual = glm::mat4(1.0f);
static glm::mat4 ui_virtual_to_window = glm::mat4(1.0f);

static MouseState mouse_state;
enum class GameState {MAIN_MENU, IN_GAME};
static GameState game_state = GameState::MAIN_MENU;
static bool loading_moon = false;
static float loading_moon_progress = 0; // TODO: Progress bar

static int active_moon = -1;
static std::vector<Chunk> loaded_chunks;

static Player player;

static double last_mouse_x = 1280.0 / 2.0;
static double last_mouse_y = 720.0 / 2.0;
void MovePlayerCamera(GLFWwindow *window, double x_pos, double y_pos)
{
    float x_offset = x_pos - last_mouse_x;
    float y_offset = last_mouse_y - y_pos; // Reversed since y ranges from bottom to top
    last_mouse_x = x_pos;
    last_mouse_y = y_pos;
    player.camera.yaw += x_offset * player.camera.sensitivity;
    player.camera.pitch += y_offset * player.camera.sensitivity;
    player.camera.pitch = glm::clamp(player.camera.pitch, -89.8f, 89.8f);

    glm::vec3 direction;
    direction.x = cos(glm::radians(player.camera.yaw)) * cos(glm::radians(player.camera.pitch));
    direction.y = sin(glm::radians(player.camera.pitch));
    direction.z = sin(glm::radians(player.camera.yaw)) * cos(glm::radians(player.camera.pitch));
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
    constexpr size_t BLOCKS_IN_CHUNK = CHUNK_SIZE * CHUNK_SIZE * WORLD_HEIGHT_LIMIT;
    int render_distance = OptionsManager::GetOptions().render_distance;
    int chunks_to_process = (2*render_distance + 1 + 2) * (2*render_distance + 1 + 2) // Generation
                          + (2*render_distance + 1) * (2*render_distance + 1); // Meshing
    int chunks_processed = 0;

    std::filesystem::path moon_dir = Storage::MOON_DIR / (std::string("moon") + std::to_string(moon));
    std::filesystem::path chunk_dir = moon_dir / "chunks";
    if (!std::filesystem::exists(moon_dir))
        std::filesystem::create_directory(moon_dir);
    if (!std::filesystem::exists(chunk_dir))
        std::filesystem::create_directory(chunk_dir);

    // Default player data
    PlayerData player_data;
    player_data.health = player.health;
    player_data.suit_status = player.suit_status;
    player_data.position = player.position;
    player_data.camera_rotation = {player.camera.pitch, player.camera.yaw};
    std::ofstream player_data_file(moon_dir / "player.dat", std::ios::binary);
    player_data_file.write(reinterpret_cast<char *>(&player_data), sizeof(PlayerData));
    player_data_file.close();

    for (int chunk_x = -render_distance - 1; chunk_x <= render_distance + 1; chunk_x++)
    {
        for (int chunk_z = -render_distance - 1; chunk_z <= render_distance + 1; chunk_z++)
        {
            Chunk chunk({chunk_x, chunk_z});
            chunk.SetIsBorderChunk(chunk_x == -render_distance - 1 || chunk_x == render_distance + 1 || chunk_z == -render_distance - 1 || chunk_z == render_distance + 1);

            GenerateChunk(chunk.GetBlocks(), chunk_x, chunk_z, moon_settings.seed);

            // Save chunk to file
            uint64_t chunk_id = CombineChunkCoordinates(chunk_x, chunk_z);
            std::filesystem::path chunk_path = chunk_dir / (std::to_string(chunk_id) + ".chunk");
            std::ofstream chunk_file(chunk_path, std::ios::binary);
            chunk_file.write(reinterpret_cast<char *>(chunk.GetBlocks()), BLOCKS_IN_CHUNK * sizeof(uint16_t));
            chunk_file.close();

            loaded_chunks.push_back(chunk);
            chunks_processed++;
            loading_moon_progress = (float)chunks_processed / (float)chunks_to_process;
        }
    }

    for (Chunk& chunk : loaded_chunks)
    {
        if (!chunk.IsBorderChunk())
        {
            chunk.BuildVertices(loaded_chunks);
            chunks_processed++;
            loading_moon_progress = (float)chunks_processed / (float)chunks_to_process;
        }
    }

    // Save moon data
    std::ofstream moon_data_file(Storage::MOON_DIR / (std::string("moon") + std::to_string(moon)) / "moon.dat", std::ios::binary);
    moon_data_file.write(reinterpret_cast<char *>(&moon_settings), sizeof(MoonSettings));
    moon_data_file.close();
}

void CreateMoon(int moon, MoonSettings moon_settings)
{
    loading_moon = true;
    active_moon = moon;
    std::thread worker(_CreateMoon, moon, moon_settings);
    worker.detach();
}

static void _LoadMoon(int moon)
{
    std::filesystem::path chunk_folder = Storage::MOON_DIR / (std::string("moon") + std::to_string(moon)) / "chunks";
    int chunk_count = std::count_if(
        std::filesystem::directory_iterator(chunk_folder),
        std::filesystem::directory_iterator{},
        [](const std::filesystem::directory_entry& e){ return e.is_regular_file(); }
    );

    constexpr size_t BLOCKS_IN_CHUNK = (CHUNK_SIZE + 2) * (CHUNK_SIZE + 2) * WORLD_HEIGHT_LIMIT;
    int render_distance = OptionsManager::GetOptions().render_distance;
    int chunks_to_process = glm::min((2*render_distance + 1) * (2*render_distance + 1), chunk_count); //  TODO: Once we dynamically load chunks this isn't necessary
    int chunks_processed = 0;

    // Load player data
    PlayerData player_data;
    std::ifstream player_data_file(Storage::MOON_DIR / (std::string("moon") + std::to_string(moon)) / "player.dat", std::ios::binary);
    player_data_file.read(reinterpret_cast<char *>(&player_data), sizeof(PlayerData));
    player_data_file.close();

    player.position = player_data.position;
    player.prev_position = player.position;
    player.next_position = player.position;
    player.health = player_data.health;
    player.suit_status = player_data.suit_status;
    player.camera.pitch = player_data.camera_rotation.x;
    player.camera.yaw = player_data.camera_rotation.y;

    // Load moon
    glm::vec2 player_chunk_pos = {glm::floor(player.position.x / CHUNK_SIZE), glm::floor(player.position.z / CHUNK_SIZE)};
    for (const auto& chunk_entry : std::filesystem::directory_iterator(chunk_folder))
    {
        uint64_t chunk_id = std::stoull(chunk_entry.path().stem().string());
        glm::vec2 chunk_pos = DecombineChunkCoordinates(chunk_id);
        // TODO: This isn't necessary once we add dynamic chunk loading
        if (chunks_to_process == chunk_count || (chunk_pos.x >= player_chunk_pos.x - render_distance - 1 && chunk_pos.x <= player_chunk_pos.x + render_distance + 1 && chunk_pos.y >= player_chunk_pos.y - render_distance - 1 && chunk_pos.y <= player_chunk_pos.y + render_distance + 1))
        {
            // Initialize chunk
            Chunk chunk(chunk_pos);
            chunk.SetIsBorderChunk(chunk_pos.x == player_chunk_pos.x - render_distance - 1 || chunk_pos.x == player_chunk_pos.x + render_distance + 1 || chunk_pos.y == player_chunk_pos.y - render_distance - 1 || chunk_pos.y == player_chunk_pos.y + render_distance + 1);

            // Read blocks
            std::ifstream chunk_file(chunk_entry.path(), std::ios::binary);
            chunk_file.read(reinterpret_cast<char *>(chunk.GetBlocks()), BLOCKS_IN_CHUNK * sizeof(uint16_t));
            chunk_file.close();

            loaded_chunks.push_back(chunk);
            chunks_processed++;
            loading_moon_progress = (float)chunks_processed / (float)chunks_to_process;
        }
    }

    for (Chunk& chunk : loaded_chunks)
    {
        if (!chunk.IsBorderChunk())
            chunk.BuildVertices(loaded_chunks);
    }
}

void LoadMoon(int moon)
{
    loading_moon = true;
    active_moon = moon;
    std::thread worker(_LoadMoon, moon);
    worker.detach();
}

struct Plane
{
    glm::vec3 normal;
    float d;
};

void GetFrustumPlanes(const glm::mat4& view_proj, Plane *frustum)
{
    int a, b;
    for (int i = 0; i < 6; i++)
    {
        a = i / 2;
        b = 1 - 2*(i & 1);

        frustum[i].normal.x = view_proj[0][3] + view_proj[0][a] * b;
        frustum[i].normal.y = view_proj[1][3] + view_proj[1][a] * b;
        frustum[i].normal.z = view_proj[2][3] + view_proj[2][a] * b;
        frustum[i].d        = view_proj[3][3] + view_proj[3][a] * b;

        float len = glm::length(frustum[i].normal);
        frustum[i].normal /= len;
        frustum[i].d      /= len;
    }
};

bool ChunkInFrustum(const Plane frustum[6], const glm::vec3& chunk_min, const glm::vec3& chunk_max)
{
    for (int i = 0; i < 6; i++)
    {
        const Plane& p = frustum[i];

        // Compute the most positive point relative to plane normal
        glm::vec3 positive = {
            (p.normal.x >= 0 ? chunk_max.x : chunk_min.x),
            (p.normal.y >= 0 ? chunk_max.y : chunk_min.y),
            (p.normal.z >= 0 ? chunk_max.z : chunk_min.z),
        };

        // If that point is behind the plane, the chunk is outside
        if (glm::dot(p.normal, positive) + p.d < 0)
            return false;
    }

    return true;
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
                    glm::vec2 chunk_pos = chunk.GetPosition();
                    if ((int)chunk_pos.x == chunk_x && (int)chunk_pos.y == chunk_z)
                    {
                        glm::vec3 local_block_pos = GetLocalBlockPos(glm::vec3(x, y, z));
                        if ((BlockID)chunk.GetBlocks()[GetChunkIndex(local_block_pos.x, local_block_pos.y, local_block_pos.z)] != BlockID::air)
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

    /////////////////////////////////////////
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    std::filesystem::path atlas_path = Storage::IMAGE_DIR / "texture_atlas.png";
    unsigned char *texture_atlas_data = stbi_load(atlas_path.c_str(), &width, &height, &nrChannels, 0);

    GLuint texture_atlas;
    glGenTextures(1, &texture_atlas);
    glBindTexture(GL_TEXTURE_2D, texture_atlas);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_atlas_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(texture_atlas_data);
    /////////////////////////////////////////

    UIRescale();
    UIMainMenu ui_main_menu(window);
    UILoadMoonMenu ui_load_moon_menu;
    UIPauseMenu ui_pause_menu;

    Skybox skybox;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bool wireframe = false;
    float last_wireframe_toggle = 0;

    float delta_time;
    float last_frame_time = 0;
    const float fixed_delta_time = 0.02f;
    float accumulator = 0;
    float last_fps_update = 0;
    float last_pause_toggle_time = 0;
    while (!glfwWindowShouldClose(window))
    {
        float current_time = glfwGetTime();
        delta_time = current_time - last_frame_time;
        last_frame_time = current_time;

        if (current_time - last_fps_update > 0.5f)
        {
            printf("FPS: %f\n", (1 / delta_time));
            last_fps_update = current_time;
        }

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

        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS && current_time - last_wireframe_toggle > 0.2f)
        {
            wireframe = !wireframe;
            last_wireframe_toggle = current_time;
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
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && current_time - last_pause_toggle_time > 0.2f)
            {
                ui_pause_menu.SetActive(!ui_pause_menu.IsActive());
                if (ui_pause_menu.IsActive())
                {
                    glfwGetCursorPos(window, &last_mouse_x, &last_mouse_y);
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    glfwSetCursorPosCallback(window, nullptr);
                }
                else
                {
                    
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    glfwSetCursorPos(window, last_mouse_x, last_mouse_y);
                    if (glfwRawMouseMotionSupported())
                        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); // More natural mouse motion
                    glfwSetCursorPosCallback(window, MovePlayerCamera);
                }

                last_pause_toggle_time = current_time;
            }

            if (!ui_pause_menu.IsActive())
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
        }

        //
        // Update/Render
        //

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (game_state == GameState::MAIN_MENU)
        {
            glDepthFunc(GL_LEQUAL);

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
                    for (Chunk& chunk : loaded_chunks)
                        if (!chunk.IsBorderChunk())
                            chunk.BufferVertices();

                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    glfwSetCursorPos(window, last_mouse_x, last_mouse_y);
                    if (glfwRawMouseMotionSupported())
                        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); // More natural mouse motion
                    glfwSetCursorPosCallback(window, MovePlayerCamera);
                    MovePlayerCamera(window, last_mouse_x, last_mouse_y);

                    loading_moon = false;
                    loading_moon_progress = 0;
                    game_state = GameState::IN_GAME;
                }
            }
        }
        else // IN_GAME
        {

            //
            // Updates
            //

            if (ui_pause_menu.IsActive())
            {
                ui_pause_menu.Update(mouse_state);
                if (ui_pause_menu.QuitClicked())
                {
                    loaded_chunks.clear();
                    ui_pause_menu.SetActive(false);
                    ui_main_menu.RefreshMoonButtonText();

                    PlayerData player_data;
                    player_data.health = player.health;
                    player_data.suit_status = player.suit_status;
                    player_data.position = player.position;
                    player_data.camera_rotation = {player.camera.pitch, player.camera.yaw};
                    std::ofstream player_data_file(Storage::MOON_DIR / (std::string("moon") + std::to_string(active_moon)) / "player.dat", std::ios::binary);
                    player_data_file.write(reinterpret_cast<char *>(&player_data), sizeof(PlayerData));
                    player_data_file.close();

                    game_state = GameState::MAIN_MENU;
                }
                else if (ui_pause_menu.ResumeClicked())
                {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    glfwSetCursorPos(window, last_mouse_x, last_mouse_y);
                    if (glfwRawMouseMotionSupported())
                        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); // More natural mouse motion
                    glfwSetCursorPosCallback(window, MovePlayerCamera);
                }
            }

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
            // Rendering
            //

            glm::mat4 view = glm::lookAt(player.camera.position, player.camera.position + player.camera.forward, player.camera.up);
            glm::mat4 projection = glm::perspective(glm::radians(45.0f), viewport.x / viewport.y, 0.1f, 300.0f);
            glm::mat4 view_projection = projection * view;

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);

            if (wireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            Shader block_shader = ShaderManager::BLOCK_SHADER;
            block_shader.Use();

            glUniformMatrix4fv(glGetUniformLocation(block_shader.GetID(), "view_projection"), 1, GL_FALSE, glm::value_ptr(view_projection));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_atlas);

            glDepthFunc(GL_LESS);

            Plane frustum[6];
            GetFrustumPlanes(view_projection, frustum);
            std::vector<Chunk *> visible_chunks;

            // Render opaque blocks
            for (Chunk& chunk : loaded_chunks)
            {
                if (!chunk.IsBorderChunk())
                {
                    glm::vec2 chunk_pos = chunk.GetPosition();
                    float x0 = chunk_pos.x * CHUNK_SIZE;
                    float y0 = 0;
                    float z0 = chunk_pos.y * CHUNK_SIZE;
                    float x1 = x0 + CHUNK_SIZE;
                    float y1 = WORLD_HEIGHT_LIMIT;
                    float z1 = z0 + CHUNK_SIZE;

                    glm::vec3 min(x0, y0, z0);
                    glm::vec3 max(x1, y1, z1);

                    if (ChunkInFrustum(frustum, min, max))
                    {
                        chunk.RenderOpaques();
                        visible_chunks.push_back(&chunk);
                    }
                }
            }

            // Render transparent blocks
            for (Chunk *chunk : visible_chunks)
            {
                chunk->RenderTransparents();
            }

            // Render skybox
            view = glm::mat4(glm::mat3(view));
            view_projection = projection * view;
            float skybox_angle = 0.02f * current_time;
            skybox.Update(view_projection, skybox_angle);
            skybox.Render();

            if (wireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            glDepthFunc(GL_LEQUAL);

            if (ui_pause_menu.IsActive())
                ui_pause_menu.Render();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    OptionsManager::SaveOptions();
    Soundlib::Exit();
    glfwTerminate();

    return 0;
}
