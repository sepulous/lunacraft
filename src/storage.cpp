#include <cstdlib>
#include <filesystem>
#include <iostream>

#include "storage.h"

std::filesystem::path Storage::ROOT_DIR;
std::filesystem::path Storage::ASSET_DIR;
std::filesystem::path Storage::SHADER_DIR;
std::filesystem::path Storage::MOON_DIR;
std::filesystem::path Storage::SCREENSHOT_DIR;

void Storage::Init()
{
    #if defined(_WIN32) || defined(_WIN64)
        ROOT_DIR = std::filesystem::path(std::getenv("APPDATA")) / ".lunacraft";
    #elif defined(__APPLE__)
        ROOT_DIR = std::filesystem::path(std::getenv("HOME")) / "Library" / "Application Support" / ".lunacraft";
    #elif defined(__linux__)
        ROOT_DIR = std::filesystem::path(std::getenv("HOME")) / ".local" / "share" / ".lunacraft";
    #else
        ROOT_DIR = std::filesystem::path(std::getenv("HOME")) / ".lunacraft";
    #endif

    if (!std::filesystem::exists(ROOT_DIR))
        std::filesystem::create_directory(ROOT_DIR);

    ASSET_DIR = ROOT_DIR / "assets";
    if (!std::filesystem::exists(ASSET_DIR))
        std::filesystem::create_directory(ASSET_DIR);

    SHADER_DIR = ASSET_DIR / "shaders";
    if (!std::filesystem::exists(SHADER_DIR))
        std::filesystem::create_directory(SHADER_DIR);

    MOON_DIR = ROOT_DIR / "moons";
    if (!std::filesystem::exists(MOON_DIR))
        std::filesystem::create_directory(MOON_DIR);

    SCREENSHOT_DIR = ROOT_DIR / "screenshots";
    if (!std::filesystem::exists(SCREENSHOT_DIR))
        std::filesystem::create_directory(SCREENSHOT_DIR);
}
