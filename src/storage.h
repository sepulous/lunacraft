#pragma once

#include <filesystem>

class Storage
{
    public:
        static std::filesystem::path USER_DATA;
        static std::filesystem::path IMAGES;
        static std::filesystem::path SHADERS;
        static std::filesystem::path SOUNDS;
        static std::filesystem::path MOONS;
        static std::filesystem::path SCREENSHOTS;
        static std::filesystem::path FONTS;

    public:
        Storage() = delete;
        static void Init();

    private:
        static std::filesystem::path GetDataDir();
};
