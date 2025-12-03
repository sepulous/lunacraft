#ifndef STORAGE_H
#define STORAGE_H

#include <filesystem>

/*
    .lunacraft/
        assets/
            sounds/
            images/
        moons/
            moon0/
                chunks/
                entities/
                moon.dat
                player.dat
            .
            .
            .
        screenshots/
        options.dat
*/

class Storage
{
    public:
        static std::filesystem::path ROOT_DIR;
        static std::filesystem::path ASSET_DIR;
        static std::filesystem::path IMAGE_DIR;
        static std::filesystem::path SHADER_DIR;
        static std::filesystem::path MOON_DIR;
        static std::filesystem::path SCREENSHOT_DIR;

    public:
        Storage() = delete;
        static void Init();
};

#endif
