#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <queue>
#include <deque>
#include <map>
#include <thread>
#include <mutex>
#include <optional>
#include <memory>
#include <atomic>
#include <unordered_map>

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
#include "block.h"
#include "skybox.h"
#include "constants.h"
#include "helpers.h"
#include "moon.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb_image/stb_image.h>

enum class GameState {MAIN_MENU, IN_GAME};

static glm::dvec2 viewport = {1280, 720};
static glm::mat4 ui_virtual_to_window = glm::mat4(1.0f);
static glm::dvec2 last_mouse_pos = {viewport.x / 2.0, viewport.y / 2.0};

static Moon *moon = nullptr;
static Player player;

static MouseState mouse_state;
static GameState game_state = GameState::MAIN_MENU;
static float loading_moon_progress = 0; 

static void UpdateCamera(GLFWwindow *window, double x_pos, double y_pos)
{
    float x_offset = x_pos - last_mouse_pos.x;
    float y_offset = last_mouse_pos.y - y_pos; // Reversed since y ranges from bottom to top
    last_mouse_pos = {x_pos, y_pos};
    player.UpdateCamera(x_pos, y_pos, x_offset, y_offset);
}

void LoadMoon(int moon_id, MoonSettings moon_settings)
{
    moon = new Moon(moon_id, moon_settings);

    // Create/fetch player data
    std::filesystem::path moon_dir = Storage::MOON_DIR / (std::string("moon") + std::to_string(moon_id));
    std::filesystem::path player_data_path = moon_dir / "player.dat";
    if (std::filesystem::exists(player_data_path))
    {
        PlayerData player_data;
        std::ifstream player_data_file(player_data_path, std::ios::binary);
        player_data_file.read(reinterpret_cast<char *>(&player_data), sizeof(PlayerData));
        player_data_file.close();

        player.position = player_data.position;
        player.prev_position = player.position;
        player.next_position = player.position;
        player.health = player_data.health;
        player.suit_status = player_data.suit_status;
        player.camera.pitch = player_data.camera_rotation.x;
        player.camera.yaw = player_data.camera_rotation.y;
    }
    else
    {
        PlayerData player_data;
        player_data.health = player.health;
        player_data.suit_status = player.suit_status;
        player_data.position = player.position;
        player_data.camera_rotation = {player.camera.pitch, player.camera.yaw};

        std::ofstream player_data_file(player_data_path, std::ios::binary);
        player_data_file.write(reinterpret_cast<char *>(&player_data), sizeof(PlayerData));
        player_data_file.close();
    }
    
    // Load initial chunks around player
    ChunkManager &chunk_manager = moon->GetChunkManager();
    int render_distance = OptionsManager::GetOptions().render_distance;
    glm::ivec3 player_chunk_coords = VoxelToChunk({player.position.x, player.position.y, player.position.z});
    for (int dx = -render_distance; dx <= render_distance; dx++)
        for (int dz = -render_distance; dz <= render_distance; dz++)
            chunk_manager.QueueNewChunk({player_chunk_coords.x + dx, 0, player_chunk_coords.z + dz});
}

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

bool TestAABBWorld(const AABB& box)
{
    float min_x = glm::round(box.center.x - box.extents.x);
    float max_x = glm::round(box.center.x + box.extents.x);
    float min_y = glm::round(box.center.y - box.extents.y);
    float max_y = glm::round(box.center.y + box.extents.y);
    float min_z = glm::round(box.center.z - box.extents.z);
    float max_z = glm::round(box.center.z + box.extents.z);

    // TODO: This should be part of PhysicsManager

    auto &chunks = moon->GetChunkManager().GetChunks();
    for (int x = min_x; x <= max_x; x++)
    {
        for (int y = min_y; y <= max_y; y++)
        {
            for (int z = min_z; z <= max_z; z++)
            {
                glm::ivec3 box_chunk_coords = VoxelToChunk({x, y, z});
                uint64_t chunk_id = ChunkCoordsToID(box_chunk_coords);

                auto it = chunks.find(chunk_id);
                if (it != chunks.end())
                {
                    glm::ivec3 local_block_pos = GlobalToLocalVoxel({x, y, z});
                    if ((BlockID)it->second.GetBlocks()[GetChunkIndex(local_block_pos.x, local_block_pos.y, local_block_pos.z)] != BlockID::air)
                        return true;
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
        UIUpdateTransforms(viewport, ui_virtual_to_window);
        UIRescale(viewport, ui_virtual_to_window);
    });
    glViewport(0, 0, viewport.x, viewport.y);
    UIUpdateTransforms(viewport, ui_virtual_to_window);

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
    /////////////////////////////////////////e

    UIRescale(viewport, ui_virtual_to_window);
    UIMainMenu ui_main_menu(window);
    UILoadMoonMenu ui_load_moon_menu;
    UIPauseMenu ui_pause_menu;

    Soundlib::Init();
    Soundlib::SetListenerVolume(0.6f);
    Soundlib::Sound music((Storage::ASSET_DIR / "sounds" / "theme1.mp3").c_str());
    Soundlib::SoundSource source(music);
    source.Play();

    Skybox skybox; // TODO: I think the Moon should own the skybox (which is determined by world time, after all)

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
            printf("FPS: %i\n", (int)(1 / delta_time));
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

        // Cursor position is unbounded (and thus meaningless) when disabled. We don't care about the cursor position in that case anyway.
        if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
        {
            double mouse_x, mouse_y;
            glfwGetCursorPos(window, &mouse_x, &mouse_y);

            glm::vec4 virtual_mouse = glm::inverse(ui_virtual_to_window) * glm::vec4(mouse_x, viewport.y - mouse_y, 0.0f, 1.0f);
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
                    glfwGetCursorPos(window, &last_mouse_pos.x, &last_mouse_pos.y);
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    glfwSetCursorPosCallback(window, nullptr);
                }
                else
                {
                    
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    glfwSetCursorPos(window, last_mouse_pos.x, last_mouse_pos.y);
                    if (glfwRawMouseMotionSupported())
                        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); // More natural mouse motion
                    glfwSetCursorPosCallback(window, UpdateCamera);
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

            // Update moon loading progress
            if (moon != nullptr)
            {
                moon->GetChunkManager().BufferReadyChunks();
                int render_distance = OptionsManager::GetOptions().render_distance;
                int chunks_to_load = (2*render_distance + 1) * (2*render_distance + 1);
                int loaded_chunks = moon->GetChunkManager().GetLoadedChunkCount();
                loading_moon_progress = (float)loaded_chunks / (float)chunks_to_load;
            }

            if (loading_moon_progress == 0)
            {
                ui_main_menu.Update(delta_time, mouse_state);
                ui_main_menu.Render(delta_time);
            }
            else
            {
                ui_main_menu.Render(delta_time);
                ui_load_moon_menu.Render();

                ui_load_moon_menu.SetProgressLevel(loading_moon_progress);
                if (loading_moon_progress >= 1.0f) // Only runs once when loading in
                {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    glfwSetCursorPos(window, last_mouse_pos.x, last_mouse_pos.y);
                    if (glfwRawMouseMotionSupported())
                        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); // More natural mouse motion
                    glfwSetCursorPosCallback(window, UpdateCamera);
                    UpdateCamera(window, last_mouse_pos.x, last_mouse_pos.y);

                    ui_main_menu.ResetMoonSettings();
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
                    PlayerData player_data;
                    player_data.health = player.health;
                    player_data.suit_status = player.suit_status;
                    player_data.position = player.position;
                    player_data.camera_rotation = {player.camera.pitch, player.camera.yaw};
                    std::ofstream player_data_file(Storage::MOON_DIR / (std::string("moon") + std::to_string(moon->GetID())) / "player.dat", std::ios::binary);
                    player_data_file.write(reinterpret_cast<char *>(&player_data), sizeof(PlayerData));
                    player_data_file.close();

                    moon->Unload();
                    moon = nullptr;

                    ui_pause_menu.SetActive(false);
                    ui_main_menu.RefreshMoonButtonText();
                    game_state = GameState::MAIN_MENU;
                    continue;
                }
                else if (ui_pause_menu.ResumeClicked())
                {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    glfwSetCursorPos(window, last_mouse_pos.x, last_mouse_pos.y);
                    if (glfwRawMouseMotionSupported())
                        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); // More natural mouse motion
                    glfwSetCursorPosCallback(window, UpdateCamera);
                }
            }

            glm::ivec3 old_player_chunk = VoxelToChunk(GetNearestVoxel(player.position));

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

            ChunkManager &chunk_manager = moon->GetChunkManager();
            int render_distance = OptionsManager::GetOptions().render_distance;
            
            // Load new chunks around player
            glm::ivec3 new_player_chunk = VoxelToChunk(GetNearestVoxel(player.position));
            if (new_player_chunk != old_player_chunk)
                for (int dx = -render_distance; dx <= render_distance; dx++)
                    for (int dz = -render_distance; dz <= render_distance; dz++)
                        chunk_manager.QueueNewChunk({new_player_chunk.x + dx, 0, new_player_chunk.z + dz});

            // Remove distant chunks and upload new ones that are ready
            chunk_manager.RemoveDistantChunks(new_player_chunk, render_distance);
            chunk_manager.BufferReadyChunks();

            //
            // Rendering
            //

            glm::mat4 view = glm::lookAt(player.camera.position, player.camera.position + player.camera.forward, player.camera.up);
            glm::mat4 projection = glm::perspective(glm::radians(45.0), viewport.x / viewport.y, 0.1, 500.0);
            glm::mat4 view_projection = projection * view;

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);

            Shader block_shader = ShaderManager::BLOCK_SHADER;
            block_shader.Use();
            block_shader.SetMat4("view_projection", view_projection);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_atlas);

            glDepthFunc(GL_LESS);

            Plane frustum[6];
            GetFrustumPlanes(view_projection, frustum);
            chunk_manager.RenderChunks(frustum);

            // Render skybox
            view = glm::mat4(glm::mat3(view));
            view_projection = projection * view;
            float skybox_angle = 0.02f * current_time;
            skybox.Update(view_projection, skybox_angle);
            skybox.Render();

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
