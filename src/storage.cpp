#include <cstdlib>
#include <string>
#include <filesystem>

#if defined(_WIN32)
    #include <windows.h>
    #include <shlobj.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
#elif defined(__linux__)
    #include <unistd.h>
#endif

#include "storage.h"

std::filesystem::path Storage::USER_DATA;
std::filesystem::path Storage::IMAGES;
std::filesystem::path Storage::SHADERS;
std::filesystem::path Storage::SOUNDS;
std::filesystem::path Storage::MOONS;
std::filesystem::path Storage::SCREENSHOTS;
std::filesystem::path Storage::FONTS;

static std::filesystem::path GetDataPath();
static std::filesystem::path GetAssetPath();

void Storage::Init()
{
    auto ASSETS = GetAssetPath() / "assets";
    IMAGES = ASSETS / "images";
    SHADERS = ASSETS / "shaders";
    SOUNDS = ASSETS / "sounds";
    FONTS = ASSETS / "fonts";

    USER_DATA = GetDataPath();
    if (!std::filesystem::exists(USER_DATA))
        std::filesystem::create_directory(USER_DATA);

    MOONS = USER_DATA / "moons";
    if (!std::filesystem::exists(MOONS))
    {
        std::filesystem::create_directory(MOONS);
        std::filesystem::create_directory(MOONS / "moon0");
        std::filesystem::create_directory(MOONS / "moon1");
        std::filesystem::create_directory(MOONS / "moon2");
        std::filesystem::create_directory(MOONS / "moon3");
    }

    SCREENSHOTS = USER_DATA / "screenshots";
    if (!std::filesystem::exists(SCREENSHOTS))
        std::filesystem::create_directory(SCREENSHOTS);
}

static std::filesystem::path GetAssetPath()
{
    //
    // Get executable dir
    //

    std::filesystem::path executable_dir;

    #if defined(_WIN32)
        wchar_t buffer[MAX_PATH];
        GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        executable_dir = std::filesystem::path(buffer).parent_path();

    #elif defined(__APPLE__)
        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);
        std::string buffer(size, '\0');
        _NSGetExecutablePath(buffer.data(), &size);
        executable_dir = std::filesystem::path(buffer).parent_path();

    #elif defined(__linux__)
        char buffer[4096];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1)
        {
            buffer[len] = '\0';
            executable_dir = std::filesystem::canonical(buffer).parent_path();
        }
        else
        {
            throw std::runtime_error("Failed to get executable path");
        }

    #endif

    //
    // Get asset path
    //

    #if defined(_WIN32)
        return executable_dir;

    #elif defined(__APPLE__)
        return executable_dir / "../Resources";

    #elif defined(__linux__)
        const char *appdir = std::getenv("APPDIR");
        if (appdir)
            return std::filesystem::path(appdir) / "usr/share/lunacraft";
        else
            return executable_dir / "../share/lunacraft";
            
    #endif
}

static std::filesystem::path GetDataPath()
{
    #if defined(_WIN32)
        wchar_t* path = nullptr;
        SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path);
        std::filesystem::path result(path);
        CoTaskMemFree(path);
        return result / "Lunacraft";

    #elif defined(__APPLE__)
        const char* home = std::getenv("HOME");
        return std::filesystem::path(home) / "Library/Application Support/Lunacraft";

    #else // Linux
        const char* xdg = std::getenv("XDG_DATA_HOME");
        if (xdg)
        {
            return std::filesystem::path(xdg) / "lunacraft";
        }
        else
        {
            const char* home = std::getenv("HOME");
            return std::filesystem::path(home) / ".local/share/lunacraft";
        }

    #endif
}
