
#include <filesystem>
#include <fstream>
#include <thread>
#include <cstdlib>

#include "moon.h"
#include "storage.h"
#include "options.h"
#include "player.h"
#include "helpers.h"
#include "constants.h"
#include "shader.h"

Moon::Moon(int moon_id, MoonSettings moon_settings)
{
    // Create moon folder (if necessary)
    std::filesystem::path moon_dir = Storage::MOON_DIR / (std::string("moon") + std::to_string(moon_id));
    if (!std::filesystem::exists(moon_dir))
        std::filesystem::create_directory(moon_dir);

    // Use moon data from file (if it already exists)
    std::filesystem::path moon_data_path = moon_dir / "moon.dat";
    if (std::filesystem::exists(moon_data_path))
    {
        std::ifstream moon_data_file(moon_data_path, std::ios::binary);
        moon_data_file.read(reinterpret_cast<char *>(&moon_settings), sizeof(MoonSettings));
        moon_data_file.close();
    }
    else
    {
        std::ofstream moon_data_file(moon_data_path, std::ios::binary);
        moon_data_file.write(reinterpret_cast<char *>(&moon_settings), sizeof(MoonSettings));
        moon_data_file.close();
    }

    // Choose random fog color (this code was reverse engineered
    // from an unknown version of the original Lunacraft)

    int _case = std::rand() % 6;
    float _rand = (float)(std::rand() % 1'000'000) / 1'000'000.0f;
    float comp1 = (1.0 - _rand * 0.85) * 0.9;
    float comp2 = (1.0 - (1.0 - _rand) * 0.85) * 0.9;

    if (_case == 0)
        _base_fog_color = {0.9, comp2, 0.135, 1};
    else if (_case == 1)
        _base_fog_color = {comp1, 0.9, 0.135, 1};
    else if (_case == 2)
        _base_fog_color = {0.135, 0.9, comp2, 1};
    else if (_case == 3)
        _base_fog_color = {0.135, comp1, 0.9, 1};
    else if (_case == 4)
        _base_fog_color = {comp2, 0.135, 0.9, 1};
    else
        _base_fog_color = {0.9, 0.135, comp1, 1};

    int render_distance = OptionsManager::GetOptions().render_distance;
    _initial_chunk_count = (2*render_distance + 1) * (2*render_distance + 1);
    _id = moon_id;
    _settings = moon_settings;
    _world_time = moon_settings.world_time;
    _player = new Player;
    _chunk_manager.Init(moon_id, moon_settings);
    _entity_manager.LinkChunkManager(&_chunk_manager);
    _entity_manager.AddEntity(_player);
}

Moon::~Moon()
{
    // Save world time to file
    _settings.world_time = _world_time;
    std::filesystem::path moon_data_path = Storage::MOON_DIR / (std::string("moon") + std::to_string(_id)) / "moon.dat";
    std::ofstream moon_data_file(moon_data_path, std::ios::binary);
    moon_data_file.write(reinterpret_cast<char *>(&_settings), sizeof(MoonSettings));
    moon_data_file.close();
}

Player *Moon::GetPlayer()
{
    return _player;
}

ChunkManager &Moon::GetChunkManager()
{
    return _chunk_manager;
}

EntityManager &Moon::GetEntityManager()
{
    return _entity_manager;
}

glm::vec4 Moon::GetFogColor()
{
    glm::vec3 fog_rgb = 0.5f * (1 + (float)glm::cos(2 * 3.1416 * _world_time / (LIGHT_PHASES * SECONDS_PER_LIGHT_PHASE))) * glm::vec3(_base_fog_color);
    return {fog_rgb.r, fog_rgb.g, fog_rgb.b, _base_fog_color.a};
}

int Moon::GetID()
{
    return _id;
}

float Moon::GetLoadProgress()
{
    int loaded_chunks = _chunk_manager.GetLoadedChunkCount();
    return (float)loaded_chunks / (float)_initial_chunk_count;
}

MoonSettings Moon::GetSettings()
{
    return _settings;
}

void Moon::Update(double delta_time, int old_render_distance)
{
    _world_time += delta_time;
    _accumulator += delta_time;

    glm::ivec3 old_player_chunk = VoxelToChunk(GetNearestVoxel(_player->GetPosition())); // Save before allowing player to move

    // Fixed updates
    if (_accumulator >= FIXED_DELTA_TIME)
        _entity_manager.FixedUpdate();

    // Physics
    int physics_steps = 0;
    if (_accumulator >= FIXED_DELTA_TIME)
        physics_steps = (int)((_accumulator - FIXED_DELTA_TIME) / FIXED_DELTA_TIME) + 1;
    _accumulator -= physics_steps * FIXED_DELTA_TIME;
    _entity_manager.RunPhysics(physics_steps, _accumulator / FIXED_DELTA_TIME);

    // Non-physics updates
    _entity_manager.Update();
    
    // Load new chunks around player (and unload old ones)
    glm::ivec3 current_player_chunk = VoxelToChunk(GetNearestVoxel(_player->GetPosition()));
    int current_render_distance = OptionsManager::GetOptions().render_distance;
    if (current_player_chunk != old_player_chunk || old_render_distance != current_render_distance)
    {
        _chunk_manager.RemoveDistantChunks(current_player_chunk, current_render_distance);
        for (int dx = -current_render_distance; dx <= current_render_distance; dx++)
            for (int dz = -current_render_distance; dz <= current_render_distance; dz++)
                _chunk_manager.QueueNewChunk({current_player_chunk.x + dx, 0, current_player_chunk.z + dz});
    }

    _chunk_manager.BufferReadyChunks();
}

void Moon::Render(glm::mat4 projection)
{
    //
    // Render world
    //

    Options options = OptionsManager::GetOptions();

    Camera player_camera = _player->GetCamera();
    glm::mat4 view = glm::lookAt(player_camera.position, player_camera.position + player_camera.forward, player_camera.up);
    glm::mat4 view_projection = projection * view;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    Shader &block_shader = ShaderManager::BLOCK_SHADER;
    block_shader.Use();
    block_shader.SetMat4("u_view_projection", view_projection);
    block_shader.SetVec3("u_ws_camera_position", player_camera.position);
    glm::vec4 fog_color = GetFogColor();
    if (!options.show_fog)
        fog_color.a = 0;
    block_shader.SetVec4("u_fog_color", fog_color);
    block_shader.SetFloat("u_fog_distance", options.render_distance * (CHUNK_SIZE / 1.5f));

    Plane frustum[6];
    GetFrustumPlanes(view_projection, frustum);
    _chunk_manager.RenderChunks(frustum);

    //
    // Render skybox
    //

    view = glm::mat4(glm::mat3(view));
    view_projection = projection * view;
    constexpr int PERIOD = LIGHT_PHASES * SECONDS_PER_LIGHT_PHASE;
    float skybox_angle = glm::radians(90.0f + (360.0f / PERIOD) * _world_time);
    _skybox.Update(view_projection, skybox_angle);
    _skybox.Render();
}
