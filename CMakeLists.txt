cmake_minimum_required(VERSION 3.20)
project(Lunacraft LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
include(GNUInstallDirs)

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.10
)
FetchContent_MakeAvailable(glfw)

# OpenGL
find_package(OpenGL REQUIRED)

# GLAD
add_library(glad STATIC third_party/glad/src/glad.c)
target_include_directories(glad PUBLIC third_party/glad/include)
target_link_libraries(glad PUBLIC OpenGL::GL)

# Soundlib
set(STATIC_SOUNDLIB ON CACHE BOOL "" FORCE) # Include Soundlib in binary
if (NOT USE_SYSTEM_OPENAL)
    set(USE_SYSTEM_OPENAL OFF CACHE BOOL "" FORCE) # Fetch OpenAL by default
endif()
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE) # Don't need this
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)    #
set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)    #
set(ALSOFT_EAX OFF CACHE BOOL "" FORCE)      #
FetchContent_Declare(
    Soundlib
    GIT_REPOSITORY https://github.com/sepulous/soundlib.git
    GIT_TAG master
)
FetchContent_MakeAvailable(Soundlib)

# Main executable
add_executable(Lunacraft
    src/main.cpp
    src/shader.cpp
    src/ui.cpp
    src/storage.cpp
    src/options.cpp
    src/skybox.cpp
    src/helpers.cpp
    src/sound_system.cpp
    src/input.cpp
    src/viewport.cpp
    src/moon.cpp
    src/chunk.cpp
    src/chunk_manager.cpp
    src/chunk_generation.cpp
    src/chunk_worker_pool.cpp
    src/simplex_noise.cpp
    src/entity_manager.cpp
    src/entity.cpp
    src/player.cpp
    src/inventory.cpp
    src/item.cpp
    src/mesh.cpp
    src/dropped_item.cpp
    src/minilight.cpp
    src/slug.cpp

    third_party/stb_image/stb_image/stb_image.cpp
    third_party/stb_truetype/stb_truetype/stb_truetype.cpp
)

# Include directories
target_include_directories(Lunacraft PRIVATE
    third_party/stb_truetype
    third_party/stb_image
    third_party/soundlib
    third_party/glm
)

# Link dependencies
target_link_libraries(Lunacraft PRIVATE
    glad
    glfw
    OpenGL::GL
    Soundlib
)

target_compile_options(Lunacraft PRIVATE -Wno-volatile)

target_compile_definitions(Lunacraft PRIVATE
    LUNACRAFT_ROOT_DIR="${CMAKE_INSTALL_FULL_DATADIR}/lunacraft"
)

# ---------------- INSTALL ----------------

# Install executable
install(TARGETS Lunacraft
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}   # Windows / Linux
    BUNDLE  DESTINATION .                         # macOS
)

# Install assets
if (APPLE)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/
        DESTINATION Lunacraft.app/Contents/Resources
    )
else()
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/lunacraft
    )
endif()

# Install .desktop file and icon (Linux)
if (NOT (WIN32 OR APPLE))
    install(FILES lunacraft.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )

    install(FILES ${CMAKE_SOURCE_DIR}/assets/images/icon.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/128x128/apps
        RENAME lunacraft.png
    )
endif()

# Copy OpenAL (Windows)
if (WIN32)
    install(FILES $<TARGET_FILE:OpenAL::OpenAL>
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
