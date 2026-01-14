cmake_minimum_required(VERSION 3.20)
project(Lunacraft LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

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
set(STATIC_SOUNDLIB ON CACHE BOOL "" FORCE)
set(SOUNDLIB_USE_SYSTEM_OPENAL OFF CACHE BOOL "" FORCE)
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
    src/chunk.cpp
    src/moon.cpp
    src/entity.cpp
    src/chunk_manager.cpp
    src/entity_manager.cpp
    src/sound_system.cpp
)

# Include directories
target_include_directories(Lunacraft PRIVATE include third_party/stb_truetype third_party/stb_image)

# Link dependencies
target_link_libraries(Lunacraft
    PRIVATE
    glad
    glfw
    OpenGL::GL
    Soundlib
)

target_compile_options(Lunacraft PRIVATE -Wno-volatile)

if (WIN32)
    add_custom_command(TARGET Lunacraft POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:OpenAL::OpenAL>
            $<TARGET_FILE_DIR:Lunacraft>
    )
endif()

if (APPLE)
    target_link_libraries(Lunacraft PRIVATE "-framework Cocoa")
    set_target_properties(Lunacraft PROPERTIES
        MACOSX_BUNDLE TRUE
    )
    add_custom_command(TARGET Lunacraft POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            $<TARGET_FILE_DIR:OpenAL::OpenAL>
            $<TARGET_BUNDLE_CONTENT_DIR:Lunacraft>/Frameworks
    )
endif()
