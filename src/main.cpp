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
#include "rng.h"
#include "fxaa.h"

enum class GameState {MAIN_MENU, IN_GAME};

static GameState game_state = GameState::MAIN_MENU;
static Moon *moon = nullptr;

void SetFullscreen(GLFWwindow *window, bool fullscreen);
void LoadMoon(int moon_id, MoonSettings moon_settings);

int main()
{
    Storage::Init();
    OptionsManager::Init();
    bool fullscreen = OptionsManager::GetOptions().fullscreen;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #if __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWwindow *window;
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glm::ivec2 initial_viewport_dims;
    if (fullscreen)
    {
        initial_viewport_dims = {mode->width, mode->height};
        Viewport::SetDimensions(initial_viewport_dims);
        window = glfwCreateWindow(initial_viewport_dims.x, initial_viewport_dims.y, "Lunacraft", monitor, nullptr);
    }
    else
    {
        initial_viewport_dims = {(float)mode->width / 2.0f, (float)mode->height / 2.0f};
        Viewport::SetDimensions(initial_viewport_dims);
        window = glfwCreateWindow(initial_viewport_dims.x, initial_viewport_dims.y, "Lunacraft", nullptr, nullptr);
        glfwSetWindowMonitor(
            window,
            nullptr,
            initial_viewport_dims.x - (initial_viewport_dims.x / 2.0f),
            initial_viewport_dims.y - (initial_viewport_dims.y / 2.0f),
            initial_viewport_dims.x,
            initial_viewport_dims.y,
            mode->refreshRate
        );
    }
    
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    bool vsync = false;

    glfwMakeContextCurrent(window);
    glfwSwapInterval((int)vsync);

    glfwShowWindow(window);
    glfwFocusWindow(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glViewport(0, 0, initial_viewport_dims.x, initial_viewport_dims.y);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
        Viewport::SetDimensions({width, height});
        UIRescale();
        if (FXAA::IsSetup())
            FXAA::Resize(width, height);
    });

    // Input callbacks
    glfwSetKeyCallback(window, Input::KeyInputCallback);
    glfwSetCharCallback(window, Input::CharInputCallback);
    glfwSetMouseButtonCallback(window, Input::MouseButtonCallback);
    glfwSetCursorPosCallback(window, Input::MousePositionCallback);
    glfwSetScrollCallback(window, Input::MouseScrollCallback);

    ShaderManager::CompileAllShaders();
    SoundSystem::Init();
    SoundSystem::Play(SoundSystem::Sound::SONG_1);

    UIRescale();
    UIMainMenu ui_main_menu;
    UIGame ui_game;

    FXAA::Setup();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    double delta_time;
    double last_frame_time = 0;
    float last_debug_update_time = 0;
    float last_sound_update_time = 0;
    float next_music_time = glfwGetTime() + RNG{}.Range(8 * 60, 12 * 60); // Every 8-12 minutes
    while (!glfwWindowShouldClose(window))
    {
        double current_time = glfwGetTime();
        delta_time = current_time - last_frame_time;
        last_frame_time = current_time;

        Input::BeginFrame();
        glfwPollEvents();

        // Update sound system
        if (current_time - last_sound_update_time >= 0.2f)
        {
            if (moon != nullptr)
            {
                auto &camera = moon->GetPlayer()->GetCamera();
                SoundSystem::SetPlayerPosition(camera.position);
                SoundSystem::SetPlayerOrientation(camera.forward, camera.up);
            }
            SoundSystem::Update(OptionsManager::GetOptions());
            last_sound_update_time = current_time;
        }

        // Fullscreen toggle
        bool new_fullscreen_state = OptionsManager::GetOptions().fullscreen;
        if (new_fullscreen_state != fullscreen)
        {
            fullscreen = new_fullscreen_state;
            SetFullscreen(window, new_fullscreen_state);
        }

        //
        // Main
        //

        if (game_state == GameState::MAIN_MENU)
        {
            //
            // Updates
            //

            ui_main_menu.Update(delta_time);

            if (ui_main_menu.IsQuitClicked())
            {
                glfwSetWindowShouldClose(window, true);
                continue;
            }

            float moon_load_progress = 0;
            if (moon != nullptr)
            {
                moon->GetChunkManager().HandleChunkJobs();
                moon->GetChunkManager().UploadReadyChunks();
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
                    if (glfwRawMouseMotionSupported())
                        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); // More natural mouse motion

                    moon->GetEntityManager().LoadInitialEntities();
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
            UIInventory &ui_inventory = ui_game.GetInventoryUI();
            UIDeathScreen &ui_death_screen = ui_game.GetDeathScreen();

            //
            // Input
            //

            if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
            {
                ui_pause_menu.SetActive(!ui_pause_menu.IsActive());
                if (ui_pause_menu.IsActive())
                {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
                else if (!ui_inventory.IsActive())
                {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    if (glfwRawMouseMotionSupported())
                        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); // More natural mouse motion

                    double x, y;
                    glfwGetCursorPos(window, &x, &y);
                    Input::SetMousePosition(x, y);
                }
            }

            if (Input::IsKeyPressed(GLFW_KEY_F3))
            {
                Options options = OptionsManager::GetOptions();
                options.show_debug_info = !options.show_debug_info;
                OptionsManager::SetOptions(options);
            }

            if (Input::IsKeyPressed(GLFW_KEY_E) && !ui_pause_menu.IsActive())
            {
                ui_inventory.SetActive(!ui_inventory.IsActive());
                if (ui_inventory.IsActive())
                {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
                else
                {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    if (glfwRawMouseMotionSupported())
                        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); // More natural mouse motion

                    double x, y;
                    glfwGetCursorPos(window, &x, &y);
                    Input::SetMousePosition(x, y);
                }
            }

            //
            // Updates
            //

            auto player = moon->GetPlayer();

            // Player death
            if (player->IsDead())
            {
                if (player->IsInControl())
                {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    player->DisableControl();
                    ui_death_screen.SetActive(true);
                }

                ui_death_screen.Update();

                if (ui_death_screen.ClickedOk())
                {
                    // Reset/save player data
                    PlayerData player_data = player->GetPlayerData();
                    player_data.health = 100;
                    player_data.suit_status = 100;
                    player_data.jetpack_energy = player->GetMaxJetpackEnergy();
                    player_data.position = {CHUNK_SIZE / 2.0f, 114.0f + 0.5f - 0.9f, CHUNK_SIZE / 2.0f};
                    std::ofstream player_data_file(Storage::MOONS / (std::string("moon") + std::to_string(moon->GetID())) / "player.dat", std::ios::binary);
                    player_data_file.write(reinterpret_cast<char *>(&player_data), sizeof(PlayerData));
                    player_data_file.close();

                    // Unload moon
                    delete moon;
                    moon = nullptr;

                    // Reset to main menu
                    ui_pause_menu.SetActive(false);
                    ui_death_screen.SetActive(false);
                    ui_death_screen.ResetClickedOk();
                    ui_main_menu.RefreshMoonButtonText();
                    game_state = GameState::MAIN_MENU;
                    continue;
                }
            }

            // Play random song periodically
            if (current_time >= next_music_time)
            {
                RNG rng;

                int song = rng.Range(1, 4);
                if (song == 1)
                    SoundSystem::Play(SoundSystem::Sound::SONG_2);
                else if (song == 2)
                    SoundSystem::Play(SoundSystem::Sound::SONG_3);
                else if (song == 3)
                    SoundSystem::Play(SoundSystem::Sound::SONG_4);
                else
                    SoundSystem::Play(SoundSystem::Sound::SONG_5);

                next_music_time = current_time + rng.Range(8 * 60, 12 * 60); // Every 8-12 minutes
            }

            // Update debug menu
            if (ui_debug_menu.IsActive() && current_time - last_debug_update_time > 0.2f)
            {
                DebugInfo debug_info;
                debug_info.fps = (int)(1 / delta_time);
                debug_info.player_pos = player->GetCamera().position;
                debug_info.seed = moon->GetSettings().seed;
                ui_debug_menu.Update(debug_info);
                last_debug_update_time = current_time;
            }

            // Update inventory
            ui_inventory.Update(player);

            // Handle quit/resume buttons
            if (ui_pause_menu.IsActive())
            {
                if (player->IsInControl())
                    player->DisableControl();

                ui_pause_menu.Update();
                if (ui_pause_menu.QuitClicked())
                {
                    // Save player data
                    PlayerData player_data = player->GetPlayerData();
                    std::ofstream player_data_file(Storage::MOONS / (std::string("moon") + std::to_string(moon->GetID())) / "player.dat", std::ios::binary);
                    player_data_file.write(reinterpret_cast<char *>(&player_data), sizeof(PlayerData));
                    player_data_file.close();

                    // Unload moon
                    delete moon;
                    moon = nullptr;

                    // Reset to main menu
                    ui_pause_menu.SetActive(false);
                    ui_main_menu.RefreshMoonButtonText();
                    game_state = GameState::MAIN_MENU;
                    continue;
                }
                else if (ui_pause_menu.ResumeClicked() && !ui_inventory.IsActive())
                {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    if (glfwRawMouseMotionSupported())
                        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE); // More natural mouse motion

                    double x, y;
                    glfwGetCursorPos(window, &x, &y);
                    Input::SetMousePosition(x, y);
                }
            }
            else
            {
                ui_debug_menu.SetActive(OptionsManager::GetOptions().show_debug_info);

                if (ui_inventory.IsActive())
                    player->DisableControl();
                else if (!player->IsDead())
                    player->EnableControl();

                player->SetCameraSensitivity(0.05f * OptionsManager::GetOptions().sensitivity);
                player->UpdateCamera();

                moon->Update(delta_time);
            }

            // Display message
            std::string message = Moon::GetCurrentMoon()->PopPendingMessage();
            if (!message.empty())
                ui_game.SetAlert(message);
            
            ui_game.Update(delta_time);

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

            moon->Render(Viewport::GetProjectionMatrix());
            ui_game.Render();
        }

        glfwSwapBuffers(window);
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
    std::filesystem::path moon_dir = Storage::MOONS / (std::string("moon") + std::to_string(moon_id));
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
        player->SetJetpackEnergy(player_data.jetpack_energy);
        player->SetCameraRotation({player_data.camera_rotation.x, player_data.camera_rotation.y});
        player->SetInventory(player_data.inventory);
    }
    else
    {
        player->SetInventory(Inventory{moon_settings.is_creative});
        player->SetJetpackEnergy(player->GetMaxJetpackEnergy());
        PlayerData player_data = player->GetPlayerData();
        std::ofstream player_data_file(player_data_path, std::ios::binary);
        player_data_file.write(reinterpret_cast<char *>(&player_data), sizeof(PlayerData));
        player_data_file.close();
    }
    
    // Load initial chunks around player
    moon->GetChunkManager().CreateInitialPatch();
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
        int width = (float)mode->width / 2.0f;
        int height = (float)mode->height / 2.0f;
        glfwSetWindowMonitor(
            window,
            nullptr,
            width - ((float)width / 2.0f),
            height - ((float)height / 2.0f),
            width,
            height,
            mode->refreshRate
        );
    }
}
