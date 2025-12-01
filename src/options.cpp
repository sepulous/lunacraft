
#include <filesystem>
#include <fstream>

#include "options.h"
#include "storage.h"

Options OptionsManager::_options;

void OptionsManager::Init()
{
    std::filesystem::path options_file_path =  Storage::ROOT_DIR / "options.dat";
    if (std::filesystem::exists(options_file_path))
    {
        char options_data[sizeof(Options)];
        std::ifstream options_file(options_file_path, std::ios::binary);
        options_file.read(options_data, sizeof(Options));
        _options = *(reinterpret_cast<Options *>(options_data));
        options_file.close();
    }
    else
    {
        std::ofstream options_file(options_file_path, std::ios::binary);
        options_file.write(reinterpret_cast<char *>(&_options), sizeof(Options));
        options_file.close();
    }
}

Options OptionsManager::GetOptions()
{
    return _options;
}

void OptionsManager::SetOptions(Options new_options)
{
    _options = new_options;
}

void OptionsManager::SaveOptions()
{
    std::filesystem::path options_file_path =  Storage::ROOT_DIR / "options.dat";
    std::ofstream options_file(options_file_path, std::ios::binary);
    options_file.write(reinterpret_cast<char *>(&_options), sizeof(Options));
    options_file.close();
}
