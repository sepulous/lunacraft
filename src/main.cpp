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
#include "sound_system.h"
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

void SetFullscreen(GLFWwindow *window, bool fullscreen);
void UpdateCamera(GLFWwindow *window, double x_pos, double y_pos);
void GetFrustumPlanes(const glm::mat4& view_proj, Plane *frustum);
void LoadMoon(int moon_id, MoonSettings moon_settings);

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
    SoundSystem::Init();
    SoundSystem::PlayAt(SoundSystem::Sound::SONG_1, {16, 70, 16});

    if (OptionsManager::GetOptions().fullscreen)
    {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(
            window,
            monitor,
            0,
            0,
            mode->width,
            mode->height,
            mode->refreshRate
        );
    }

    /////////////////////////////////////////
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    std::filesystem::path atlas_path = Storage::IMAGE_DIR / "texture_atlas.png";
    unsigned char *texture_atlas_data = stbi_load(reinterpret_cast<const char *>(atlas_path.u8string().c_str()), &width, &height, &nrChannels, 0);

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
    UIDebugMenu ui_debug_menu;

    Skybox skybox;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float delta_time;
    float last_frame_time = 0;
    float accumulator = 0;
    float last_pause_toggle_time = 0;
    float last_debug_toggle_time = 0;
    float last_debug_update_time = 0;
    float last_sound_update_time = 0;
    float next_music_time = glfwGetTime() + RandomRange(8 * 60, 12 * 60 + 1); // Every 8-12 minutes
    bool fullscreen = OptionsManager::GetOptions().fullscreen;
    while (!glfwWindowShouldClose(window))
    {
        float current_time = glfwGetTime();
        delta_time = current_time - last_frame_time;
        last_frame_time = current_time;

        // Update sound system
        if (current_time - last_sound_update_time >= 0.2f)
        {
            SoundSystem::Update(OptionsManager::GetOptions());
            SoundSystem::SetPlayerPosition(player.GetPosition());
            SoundSystem::SetPlayerOrientation(player.GetCamera().forward, player.GetCamera().up);
            last_sound_update_time = current_time;
        }

        // Fullscreen toggle
        if (OptionsManager::GetOptions().fullscreen != fullscreen)
        {
            fullscreen = OptionsManager::GetOptions().fullscreen;
            SetFullscreen(window, fullscreen);
        }

        // Play random song
        if (current_time >= next_music_time)
        {
            int song = RandomRange(0, 4);
            if (song == 0)
                SoundSystem::Play(SoundSystem::Sound::SONG_2);
            else if (song == 1)
                SoundSystem::Play(SoundSystem::Sound::SONG_3);
            else if (song == 2)
                SoundSystem::Play(SoundSystem::Sound::SONG_4);
            else
                SoundSystem::Play(SoundSystem::Sound::SONG_5);

            next_music_time = current_time + RandomRange(8 * 60, 12 * 60 + 1); // Every 8-12 minutes
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
                Camera player_camera = player.GetCamera();
                glm::vec3 player_input_direction = glm::vec3(0);
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                    player_input_direction += glm::vec3(player_camera.forward.x, 0, player_camera.forward.z);
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                    player_input_direction -= glm::vec3(player_camera.forward.x, 0, player_camera.forward.z);
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                    player_input_direction -= player_camera.right;
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                    player_input_direction += player_camera.right;
                if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && player.IsGrounded())
                    player.SetJumping(true);

                player.SetInputDirection(player_input_direction);
            }

            if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS && current_time - last_debug_toggle_time > 0.2f)
            {
                Options options = OptionsManager::GetOptions();
                options.show_debug_info = !options.show_debug_info;
                OptionsManager::SetOptions(options);
                last_debug_toggle_time = current_time;
            }
        }

        //
        // Update/Render
        //

        if (game_state == GameState::MAIN_MENU)
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

            // Render
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

            moon->UpdateWorldTime(delta_time);

            ChunkManager &chunk_manager = moon->GetChunkManager();
            EntityManager &entity_manager = moon->GetEntityManager();

            if (ui_debug_menu.IsActive() && current_time - last_debug_update_time > 0.2f)
            {
                DebugInfo debug_info;
                debug_info.fps = (int)(1 / delta_time);
                debug_info.player_pos = player.GetCamera().position;
                debug_info.seed = moon->GetSettings().seed;
                ui_debug_menu.Update(debug_info);
                last_debug_update_time = current_time;
            }

            int old_render_distance = OptionsManager::GetOptions().render_distance;
            if (ui_pause_menu.IsActive())
            {
                ui_pause_menu.Update(mouse_state);
                if (ui_pause_menu.QuitClicked())
                {
                    PlayerData player_data;
                    player_data.health = player.GetHealth();
                    player_data.suit_status = player.GetSuitStatus();
                    player_data.position = player.GetPosition();
                    player_data.camera_rotation = player.GetCameraRotation();
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
            int current_render_distance = OptionsManager::GetOptions().render_distance;

            player.SetCameraSensitivity(0.05f * OptionsManager::GetOptions().sensitivity);
            ui_debug_menu.SetActive(OptionsManager::GetOptions().show_debug_info);

            glm::ivec3 old_player_chunk = VoxelToChunk(GetNearestVoxel(player.GetPosition()));

            accumulator += delta_time;

            // Fixed updates
            if (accumulator >= FIXED_DELTA_TIME)
                entity_manager.FixedUpdate();

            // Physics
            int physics_steps = 0;
            if (accumulator >= FIXED_DELTA_TIME)
                physics_steps = (int)((accumulator - FIXED_DELTA_TIME) / FIXED_DELTA_TIME) + 1;
            accumulator -= physics_steps * FIXED_DELTA_TIME;
            entity_manager.RunPhysics(physics_steps, accumulator / FIXED_DELTA_TIME);

            // Non-physics updates
            entity_manager.Update();
            
            // Load new chunks around player
            glm::ivec3 new_player_chunk = VoxelToChunk(GetNearestVoxel(player.GetPosition()));
            if (new_player_chunk != old_player_chunk || old_render_distance != current_render_distance)
                for (int dx = -current_render_distance; dx <= current_render_distance; dx++)
                    for (int dz = -current_render_distance; dz <= current_render_distance; dz++)
                        chunk_manager.QueueNewChunk({new_player_chunk.x + dx, 0, new_player_chunk.z + dz});

            // Remove distant chunks and upload new ones that are ready
            chunk_manager.RemoveDistantChunks(new_player_chunk, current_render_distance);
            chunk_manager.BufferReadyChunks();

            //
            // Rendering
            //

            if (OptionsManager::GetOptions().show_fog)
            {
                glm::vec4 fog_color = moon->GetFogColor();
                glClearColor(fog_color.r, fog_color.g, fog_color.b, fog_color.a);
            }
            else
            {
                glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            }
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Camera player_camera = player.GetCamera();
            glm::mat4 view = glm::lookAt(player_camera.position, player_camera.position + player_camera.forward, player_camera.up);
            glm::mat4 projection = glm::perspective(glm::radians(45.0), viewport.x / viewport.y, 0.1, 500.0);
            glm::mat4 view_projection = projection * view;

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);

            Shader block_shader = ShaderManager::BLOCK_SHADER;
            block_shader.Use();
            block_shader.SetMat4("view_projection", view_projection);
            block_shader.SetVec3("camera_pos_ws", player_camera.position);
            glm::vec4 fog_color = moon->GetFogColor();
            if (!OptionsManager::GetOptions().show_fog)
                fog_color.a = 0;
            block_shader.SetVec4("fog_color", fog_color);
            block_shader.SetFloat("fog_distance", current_render_distance * (CHUNK_SIZE / 1.5f));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture_atlas);

            glDepthFunc(GL_LESS);

            Plane frustum[6];
            GetFrustumPlanes(view_projection, frustum);
            chunk_manager.RenderChunks(frustum);

            // Render skybox
            view = glm::mat4(glm::mat3(view));
            view_projection = projection * view;
            moon->RenderSkybox(view_projection);

            glDepthFunc(GL_LEQUAL);

            if (ui_debug_menu.IsActive())
                ui_debug_menu.Render();

            if (ui_pause_menu.IsActive())
                ui_pause_menu.Render();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    OptionsManager::SaveOptions();
    SoundSystem::Exit();
    glfwTerminate();

    return 0;
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

        player.SetPosition(player_data.position);
        player.SetPrevPosition(player_data.position);
        player.SetNextPosition(player_data.position);
        player.SetHealth(player_data.health);
        player.SetSuitStatus(player_data.suit_status);
        player.SetCameraRotation({player_data.camera_rotation.x, player_data.camera_rotation.y});
    }
    else
    {
        PlayerData player_data;
        player_data.health = player.GetHealth();
        player_data.suit_status = player.GetSuitStatus();
        player_data.position = player.GetPosition();
        player_data.camera_rotation = player.GetCameraRotation();

        std::ofstream player_data_file(player_data_path, std::ios::binary);
        player_data_file.write(reinterpret_cast<char *>(&player_data), sizeof(PlayerData));
        player_data_file.close();
    }

    moon->GetEntityManager().AddEntity(&player);
    
    // Load initial chunks around player
    ChunkManager &chunk_manager = moon->GetChunkManager();
    int render_distance = OptionsManager::GetOptions().render_distance;
    glm::ivec3 player_chunk_coords = VoxelToChunk({player.GetPosition().x, player.GetPosition().y, player.GetPosition().z});
    for (int dx = -render_distance; dx <= render_distance; dx++)
        for (int dz = -render_distance; dz <= render_distance; dz++)
            chunk_manager.QueueNewChunk({player_chunk_coords.x + dx, 0, player_chunk_coords.z + dz});
}

void UpdateCamera(GLFWwindow *window, double x_pos, double y_pos)
{
    float x_offset = x_pos - last_mouse_pos.x;
    float y_offset = last_mouse_pos.y - y_pos; // Reversed since y ranges from bottom to top
    last_mouse_pos = {x_pos, y_pos};
    player.UpdateCamera(x_pos, y_pos, x_offset, y_offset);
}

void SetFullscreen(GLFWwindow *window, bool fullscreen)
{
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (fullscreen)
    {
        glfwSetWindowMonitor(
            window,
            monitor,
            0, 0,
            mode->width,
            mode->height,
            mode->refreshRate
        );
    }
    else
    {
        glfwSetWindowMonitor(
            window,
            nullptr,
            ((float)mode->width / 2.0f) - (1280.0f / 2.0f),
            ((float)mode->height / 2.0f) - (720.0f / 2.0f),
            1280,
            720,
            0 // Refresh rate is ignored in windowed mode
        );
    }
}
