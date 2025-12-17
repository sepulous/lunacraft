
#include <filesystem>
#include <fstream>
#include <thread>

#include "moon.h"
#include "storage.h"
#include "options.h"
#include "player.h"
#include "helpers.h"

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

    _id = moon_id;
    _settings = moon_settings;
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

void Moon::Unload()
{
    _chunk_manager.Unload();
}

int Moon::GetID()
{
    return _id;
}
