cmake_minimum_required(VERSION 3.10)
project(Lunacraft)

set(CMAKE_CXX_STANDARD 17)

# Add GLAD
add_library(glad STATIC third_party/glad.c)
target_include_directories(glad PUBLIC include)

# Add GLFW (system-installed)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

# If you downloaded GLFW manually instead:
# add_subdirectory(libs/glfw)
# target_link_libraries(glfw)

# Main executable
add_executable(Lunacraft
    src/main.cpp
    src/shader.cpp
    src/ui.cpp
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
    glad
    ${GLFW_LIBRARIES}
)

# On Linux: need to link OpenGL + X11 + others manually
if (UNIX AND NOT APPLE)
    target_link_libraries(Lunacraft dl X11 pthread Xrandr Xi GL)
endif()

# On macOS, link OpenGL and Cocoa frameworks
if (APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OpenGL_LIBRARY OpenGL)
    find_library(IOKit_LIBRARY IOKit)
    find_library(CoreVideo_LIBRARY CoreVideo)
    target_link_libraries(Lunacraft ${COCOA_LIBRARY} ${OpenGL_LIBRARY} ${IOKit_LIBRARY} ${CoreVideo_LIBRARY})
endif()
