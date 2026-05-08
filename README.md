
# Lunacraft

Lunacraft is a Minecraft-style procedural voxel game, originally developed by Charlie Deck for iOS in 2012. The game was taken off the app store around 2016. This is a cross-platform recreation for PC that aims to be as faithful to the original as possible.

This recreation was implemented in a custom engine using C++, OpenGL, OpenAL, and GLFW. It features multithreaded chunk processing, a deterministic, framerate-independent physics system, as well as various rendering optimizations, such as frustum culling, incrementally baked lighting, and greedy meshing.

# Download

Lunacraft is available on Windows through a self-contained Inno Setup installer, and on Linux through an AppImage. The best option for Linux users is to use AppImageLauncher to automatically integrate Lunacraft into their system (which installs the icon and .desktop file so it can be searched for). The alternative is to manually copy the .desktop file and icon, which requires appropriately modifying the default lunacraft.desktop file so the `Exec` and `Icon` properties point to the correct files.

# Building from source

Lunacraft requires a C++20 compiler and CMake >=3.21. While most dependencies are vendored, GLFW is fetched from the official repository, and OpenGL 3.3 (core) is assumed to be available. On Windows, by default, OpenAL is fetched from the official OpenAL Soft repository and built locally, while on Linux, OpenAL is assumed to be installed. In either case, you can control whether to use a system-installed version or fetch it with the `-DUSE_SYSTEM_OPENAL` option.

### Windows/Linux

Lunacraft can be built and installed from source as follows:
```
$ mkdir build
$ cmake -S . -B build
$ cmake --build build --config Release -j
$ sudo cmake --install build
```

On Windows this should install to `Program Files (x86)/Lunacraft`, and on Linux this should make the binary available in `PATH` and automatically copy the icon and .desktop file.

### macOS

Lunacraft has not been built/tested on macOS yet, although the steps should be the same as above. In the meantime, users are encouraged to report any successes/failures in building on macOS so this can be ironed out. The current CMakeLists.txt should support the building of an app bundle.
