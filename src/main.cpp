#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "viewport.h"
#include "ui.h"
#include "shader.h"
#include "sound_system.h"
#include "storage.h"
#include "input.h"
#include "options.h"
#include "player.h"
#include "chunk.h"
#include "block.h"
#include "constants.h"
#include "helpers.h"
#include "moon.h"

enum class GameState {MAIN_MENU, IN_GAME};

static Viewport viewport;
static MouseState mouse_state;
static GameState game_state = GameState::MAIN_MENU;

static Moon *moon = nullptr;

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

    GLFWwindow *window = glfwCreateWindow(viewport.dimensions.x, viewport.dimensions.y, "Lunacraft", nullptr, nullptr);
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
        //viewport = {width, height};
        viewport.dimensions = {width, height};
        //UIUpdateTransforms(viewport, ui_virtual_to_window);
        UIUpdateTransforms(viewport);
        //UIRescale(viewport, ui_virtual_to_window);
        UIRescale(viewport);
    });
    //glViewport(0, 0, viewport.x, viewport.y);
    glViewport(0, 0, viewport.dimensions.x, viewport.dimensions.y);
    //UIUpdateTransforms(viewport, ui_virtual_to_window);
    UIUpdateTransforms(viewport);

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

    UIRescale(viewport);
    UIMainMenu ui_main_menu(window);
    UIGame ui_game;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bool left_game = false;

    double delta_time;
    double last_frame_time = 0;
    float last_pause_toggle_time = 0;
    float last_debug_toggle_time = 0;
    float last_debug_update_time = 0;
    float last_sound_update_time = 0;
    float next_music_time = glfwGetTime() + RandomRange(8 * 60, 12 * 60 + 1); // Every 8-12 minutes
    bool fullscreen = OptionsManager::GetOptions().fullscreen;
    while (!glfwWindowShouldClose(window))
    {
        double current_time = glfwGetTime();
        delta_time = current_time - last_frame_time;
        last_frame_time = current_time;

        //
        // Input (general)
        //

        // Update mouse click state
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

        // Update mouse position state
        if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) // Cursor position is unbounded (and thus meaningless) when disabled
        {
            double mouse_x, mouse_y;
            glfwGetCursorPos(window, &mouse_x, &mouse_y);

            glm::vec4 virtual_mouse = glm::inverse(viewport.ui_virtual_to_window) * glm::vec4(mouse_x, viewport.dimensions.y - mouse_y, 0.0f, 1.0f);
            mouse_state.position.x = virtual_mouse.x;
            mouse_state.position.y = virtual_mouse.y;
        }

        //
        // Updates (general)
        //

        // Update sound system
        if (current_time - last_sound_update_time >= 0.2f)
        {
            SoundSystem::Update(OptionsManager::GetOptions());
            if (moon != nullptr)
            {
                Player *player = moon->GetPlayer();
                SoundSystem::SetPlayerPosition(player->GetPosition());
                SoundSystem::SetPlayerOrientation(player->GetCamera().forward, player->GetCamera().up);
            }
            last_sound_update_time = current_time;
        }

        // Fullscreen toggle
        bool new_fullscreen_state = OptionsManager::GetOptions().fullscreen;
        if (new_fullscreen_state != fullscreen)
        {
            fullscreen = new_fullscreen_state;
            SetFullscreen(window, fullscreen);
        }

        //
        // Game state specific 
        //

        if (game_state == GameState::MAIN_MENU)
        {
            //
            // Updates
            //

            ui_main_menu.Update(delta_time, mouse_state);

            float moon_load_progress = 0;
            if (moon != nullptr)
            {
                moon->GetChunkManager().BufferReadyChunks();
                moon_load_progress = moon->GetLoadProgress();
            }

            if (moon_load_progress == 0)
            {
                if (ui_main_menu.IsLaunchButtonClicked())
                {
                    auto [moon_id, moon_settings] = ui_main_menu.GetMoonData();
                    LoadMoon(moon_id, moon_settings);
                    ui_main_menu.SetLaunchButtonClicked(false);
                }
            }
            else
            {
                ui_main_menu.SetLoadProgressLevel(moon_load_progress);
                if (moon_load_progress >= 1.0f) // Only runs once when loading in
                {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    glfwSetCursorPos(window, viewport.last_mouse_pos.x, viewport.last_mouse_pos.y);
                    if (glfwRawMouseMotionSupported())
                        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); // More natural mouse motion
                    glfwSetCursorPosCallback(window, UpdateCamera);
                    UpdateCamera(window, viewport.last_mouse_pos.x, viewport.last_mouse_pos.y);

                    ui_main_menu.ResetMoonSettings();
                    moon_load_progress = 0;
                    ui_main_menu.SetLoadProgressLevel(0);
                    game_state = GameState::IN_GAME;
                }
            }

            //
            // Rendering
            //

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDepthFunc(GL_LEQUAL);

            ui_main_menu.Render(delta_time);
        }
        else // IN_GAME
        {
            UIPauseMenu &ui_pause_menu = ui_game.GetPauseMenu();
            UIDebugMenu &ui_debug_menu = ui_game.GetDebugMenu();
            int old_render_distance = OptionsManager::GetOptions().render_distance;

            //
            // Input
            //

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && current_time - last_pause_toggle_time > 0.2f)
            {
                ui_pause_menu.SetActive(!ui_pause_menu.IsActive());
                if (ui_pause_menu.IsActive())
                {
                    glfwGetCursorPos(window, &viewport.last_mouse_pos.x, &viewport.last_mouse_pos.y);
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    glfwSetCursorPosCallback(window, nullptr);
                }
                else
                {
                    
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    glfwSetCursorPos(window, viewport.last_mouse_pos.x, viewport.last_mouse_pos.y);
                    if (glfwRawMouseMotionSupported())
                        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); // More natural mouse motion
                    glfwSetCursorPosCallback(window, UpdateCamera);
                }

                last_pause_toggle_time = current_time;
            }

            if (!ui_pause_menu.IsActive())
            {
                Player *player = moon->GetPlayer();
                Camera &player_camera = player->GetCamera();

                glm::vec3 player_input_direction = glm::vec3(0);
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                    player_input_direction += glm::vec3(player_camera.forward.x, 0, player_camera.forward.z);
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                    player_input_direction -= glm::vec3(player_camera.forward.x, 0, player_camera.forward.z);
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                    player_input_direction -= player_camera.right;
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                    player_input_direction += player_camera.right;
                if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && player->IsGrounded())
                    player->SetJumping(true);

                player->SetInputDirection(player_input_direction);
            }

            if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS && current_time - last_debug_toggle_time > 0.2f)
            {
                Options options = OptionsManager::GetOptions();
                options.show_debug_info = !options.show_debug_info;
                OptionsManager::SetOptions(options);
                last_debug_toggle_time = current_time;
            }

            //
            // Updates
            //

            // Play random song periodically
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

            // Update debug menu
            if (ui_debug_menu.IsActive() && current_time - last_debug_update_time > 0.2f)
            {
                DebugInfo debug_info;
                debug_info.fps = (int)(1 / delta_time);
                debug_info.player_pos = moon->GetPlayer()->GetCamera().position;
                debug_info.seed = moon->GetSettings().seed;
                ui_debug_menu.Update(debug_info);
                last_debug_update_time = current_time;
            }

            // Handle quit/resume buttons (this combines Update and Input; let's try to do better)
            if (ui_pause_menu.IsActive())
            {
                ui_pause_menu.Update(mouse_state);
                if (ui_pause_menu.QuitClicked())
                {
                    PlayerData player_data = moon->GetPlayer()->GetPlayerData();
                    std::ofstream player_data_file(Storage::MOON_DIR / (std::string("moon") + std::to_string(moon->GetID())) / "player.dat", std::ios::binary);
                    player_data_file.write(reinterpret_cast<char *>(&player_data), sizeof(PlayerData));
                    player_data_file.close();

                    delete moon;
                    moon = nullptr;

                    ui_pause_menu.SetActive(false);
                    ui_main_menu.RefreshMoonButtonText();
                    game_state = GameState::MAIN_MENU;
                    left_game = true;
                    continue;
                }
                else if (ui_pause_menu.ResumeClicked())
                {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    glfwSetCursorPos(window, viewport.last_mouse_pos.x, viewport.last_mouse_pos.y);
                    if (glfwRawMouseMotionSupported())
                        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); // More natural mouse motion
                    glfwSetCursorPosCallback(window, UpdateCamera);
                }
            }

            moon->GetPlayer()->SetCameraSensitivity(0.05f * OptionsManager::GetOptions().sensitivity);
            ui_debug_menu.SetActive(OptionsManager::GetOptions().show_debug_info);

            moon->Update(delta_time, old_render_distance);

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

            glm::mat4 projection = glm::perspective(glm::radians(45.0), viewport.dimensions.x / viewport.dimensions.y, 0.1, 500.0); // This only really needs to be recomputed when the viewport changes
            moon->Render(projection);
            ui_game.Render();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    OptionsManager::SaveOptions();
    SoundSystem::Exit();
    glfwTerminate();

    return 0;
}

void LoadMoon(int moon_id, MoonSettings moon_settings)
{
    moon = new Moon(moon_id, moon_settings);
    Player *player = moon->GetPlayer();

    // Create/fetch player data
    std::filesystem::path moon_dir = Storage::MOON_DIR / (std::string("moon") + std::to_string(moon_id));
    std::filesystem::path player_data_path = moon_dir / "player.dat";
    if (std::filesystem::exists(player_data_path))
    {
        PlayerData player_data;
        std::ifstream player_data_file(player_data_path, std::ios::binary);
        player_data_file.read(reinterpret_cast<char *>(&player_data), sizeof(PlayerData));
        player_data_file.close();

        player->SetPosition(player_data.position);
        player->SetPrevPosition(player_data.position);
        player->SetNextPosition(player_data.position);
        player->SetHealth(player_data.health);
        player->SetSuitStatus(player_data.suit_status);
        player->SetCameraRotation({player_data.camera_rotation.x, player_data.camera_rotation.y});
    }
    else
    {
        PlayerData player_data = player->GetPlayerData();
        std::ofstream player_data_file(player_data_path, std::ios::binary);
        player_data_file.write(reinterpret_cast<char *>(&player_data), sizeof(PlayerData));
        player_data_file.close();
    }
    
    // Load initial chunks around player
    ChunkManager &chunk_manager = moon->GetChunkManager();
    int render_distance = OptionsManager::GetOptions().render_distance;
    glm::ivec3 player_chunk_coords = VoxelToChunk({player->GetPosition().x, player->GetPosition().y, player->GetPosition().z});
    for (int dx = -render_distance; dx <= render_distance; dx++)
        for (int dz = -render_distance; dz <= render_distance; dz++)
            chunk_manager.QueueNewChunk({player_chunk_coords.x + dx, 0, player_chunk_coords.z + dz});
}

void UpdateCamera(GLFWwindow *window, double x_pos, double y_pos)
{
    if (moon != nullptr)
    {
        float x_offset = x_pos - viewport.last_mouse_pos.x;
        float y_offset = viewport.last_mouse_pos.y - y_pos; // Reversed since y ranges from bottom to top
        viewport.last_mouse_pos = {x_pos, y_pos};
        moon->GetPlayer()->UpdateCamera(x_pos, y_pos, x_offset, y_offset);
    }
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
