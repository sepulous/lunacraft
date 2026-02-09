#include <cstdlib>
#include <string>
#include <filesystem>
#include <iostream>

#include "storage.h"

std::filesystem::path Storage::USER_DATA;
std::filesystem::path Storage::IMAGES;
std::filesystem::path Storage::SHADERS;
std::filesystem::path Storage::SOUNDS;
std::filesystem::path Storage::MOONS;
std::filesystem::path Storage::SCREENSHOTS;
std::filesystem::path Storage::FONTS;

void Storage::Init()
{
    auto root_dir = std::filesystem::path(LUNACRAFT_ROOT_DIR); // LUNACRAFT_ROOT_DIR is supplied by CMake
    IMAGES = root_dir / "images";
    SHADERS = root_dir / "shaders";
    SOUNDS = root_dir / "sounds";
    FONTS = root_dir / "fonts";

    USER_DATA = GetDataDir();
    if (!std::filesystem::exists(USER_DATA))
        std::filesystem::create_directory(USER_DATA);

    MOONS = USER_DATA / "moons";
    if (!std::filesystem::exists(MOONS))
        std::filesystem::create_directory(MOONS);

    SCREENSHOTS = USER_DATA / "screenshots";
    if (!std::filesystem::exists(SCREENSHOTS))
        std::filesystem::create_directory(SCREENSHOTS);
}

std::filesystem::path Storage::GetDataDir()
{
    #ifdef _WIN32
        const char *appdata = std::getenv("APPDATA");
        return appdata ? appdata : ".";
    #elif __APPLE__
        const char *home = std::getenv("HOME");
        return std::filesystem::path(home) / "Library/Application Support/Lunacraft";
    #else
        const char *xdg = std::getenv("XDG_DATA_HOME");
        if (xdg)
            return std::filesystem::path(xdg) / "lunacraft";

        const char *home = std::getenv("HOME");
        return std::filesystem::path(home) / ".local/share/lunacraft";
    #endif
}
