
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

    // Choose random fog color
    int fog_color = (int)((float)std::rand() * 4 / RAND_MAX);
    if (fog_color == 0)
        _fog_color = glm::vec4(0.067, 0.208, 0.314, 1);
    else if (fog_color == 1)
        _fog_color = glm::vec4(0.314, 0.067, 0.31, 1);
    else if (fog_color == 2)
        _fog_color = glm::vec4(0.067, 0.314, 0.188, 1);
    else if (fog_color == 3)
        _fog_color = glm::vec4(0.067, 0.094, 0.314, 1);
    else
        _fog_color = glm::vec4(0.239, 0.067, 0.314, 1);

    _id = moon_id;
    _settings = moon_settings;
    _world_time = moon_settings.world_time;
    _chunk_manager.Init(moon_id, moon_settings);
    _entity_manager.LinkChunkManager(&_chunk_manager);
}

ChunkManager &Moon::GetChunkManager()
{
    return _chunk_manager;
}

EntityManager &Moon::GetEntityManager()
{
    return _entity_manager;
}

void Moon::UpdateWorldTime(double delta_time)
{
    _world_time += delta_time;
}

void Moon::RenderSkybox(glm::mat4 view_proj)
{
    constexpr int PERIOD = LIGHT_PHASES * SECONDS_PER_LIGHT_PHASE;
    float skybox_angle = glm::radians(90.0f + (360.0f / PERIOD) * _world_time);
    _skybox.Update(view_proj, skybox_angle);
    _skybox.Render();
}

glm::vec4 Moon::GetFogColor()
{
    glm::vec3 fog_rgb = 0.5f * (1 + (float)glm::cos(2 * 3.1416f * _world_time / 300.0f)) * glm::vec3(_fog_color.r, _fog_color.g, _fog_color.b);
    return {fog_rgb.r, fog_rgb.g, fog_rgb.b, _fog_color.a};
}

void Moon::Unload()
{
    _chunk_manager.Unload();

    // Save world time to file
    _settings.world_time = _world_time;
    std::filesystem::path moon_data_path = Storage::MOON_DIR / (std::string("moon") + std::to_string(_id)) / "moon.dat";
    std::ofstream moon_data_file(moon_data_path, std::ios::binary);
    moon_data_file.write(reinterpret_cast<char *>(&_settings), sizeof(MoonSettings));
    moon_data_file.close();
}

int Moon::GetID()
{
    return _id;
}

MoonSettings Moon::GetSettings()
{
    return _settings;
}
