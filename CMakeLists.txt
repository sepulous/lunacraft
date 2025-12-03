cmake_minimum_required(VERSION 3.13)
project(Lunacraft)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Add GLAD
add_library(glad STATIC third_party/glad.c)
target_include_directories(glad PUBLIC include)

# Add GLFW (system-installed)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

# If you downloaded GLFW manually instead:
# add_subdirectory(libs/glfw)
# target_link_libraries(glfw)

# Soundlib
include(FetchContent)
FetchContent_Declare(
    Soundlib
    GIT_REPOSITORY https://github.com/sepulous/soundlib.git
    GIT_TAG master
)
set(STATIC_SOUNDLIB ON CACHE BOOL "Build Soundlib as static library" FORCE)
FetchContent_MakeAvailable(Soundlib)

# Main executable
add_executable(Lunacraft
    src/main.cpp
    src/shader.cpp
    src/ui.cpp
    src/storage.cpp
    src/options.cpp
    src/skybox.cpp
)

# Include directories
target_include_directories(Lunacraft
    PUBLIC
    include
    third_party
    ${GLFW_INCLUDE_DIRS}
)

# Link dependencies
target_link_libraries(Lunacraft
    PRIVATE
    glad
    ${GLFW_LIBRARIES}
    Soundlib
)

# On Linux: need to link OpenGL + X11 + others manually
if (UNIX AND NOT APPLE)
    target_link_libraries(Lunacraft PRIVATE dl X11 pthread Xrandr Xi GL)
endif()

# On macOS, link OpenGL and Cocoa frameworks
if (APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OpenGL_LIBRARY OpenGL)
    find_library(IOKit_LIBRARY IOKit)
    find_library(CoreVideo_LIBRARY CoreVideo)
    target_link_libraries(Lunacraft PRIVATE ${COCOA_LIBRARY} ${OpenGL_LIBRARY} ${IOKit_LIBRARY} ${CoreVideo_LIBRARY})
endif()
