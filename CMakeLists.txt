cmake_minimum_required(VERSION 3.20)
project(Lunacraft VERSION 1.0.0 LANGUAGES C CXX)

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
    src/green_mob.cpp
    src/brown_mob.cpp
    src/blue_mob.cpp
    src/giraffe.cpp
    src/turret.cpp
    src/astronaut.cpp
    src/fxaa.cpp

    third_party/stb_image/stb_image/stb_image.cpp
    third_party/stb_image_write/stb_image_write/stb_image_write.cpp
    third_party/stb_truetype/stb_truetype/stb_truetype.cpp
)

# Include directories
target_include_directories(Lunacraft PRIVATE
    third_party/stb_truetype
    third_party/stb_image
    third_party/stb_image_write
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

target_compile_options(Lunacraft PRIVATE -Wno-volatile) # Make GLM less annoying

target_compile_definitions(Lunacraft PRIVATE
    LUNACRAFT_VERSION="${PROJECT_VERSION}"
)

# ---------------- INSTALL ----------------

# rpath handling (Linux and macOS)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
if(APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks;@executable_path/../Resources")
elseif(UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN")
endif()

# macOS: build as .app bundle
if(APPLE)
    set_target_properties(Lunacraft PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Lunacraft"
    )
endif()

# Install executable
if(APPLE)
    install(TARGETS Lunacraft
        BUNDLE DESTINATION .
    )
else()
    install(TARGETS Lunacraft
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# Install assets
if(APPLE)
    set(MACOS_RES_DIR "Lunacraft.app/Contents/Resources")
    install(DIRECTORY assets DESTINATION ${MACOS_RES_DIR})
else()
    set(LUNACRAFT_DATA_DIR "${CMAKE_INSTALL_DATADIR}/lunacraft")
    install(DIRECTORY assets DESTINATION ${LUNACRAFT_DATA_DIR})
endif()

# Copy OpenAL DLL (Windows)
if(WIN32)
    install(TARGETS OpenAL::OpenAL
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# Linux .desktop
if(UNIX AND NOT APPLE)
    install(FILES lunacraft.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )

    install(FILES assets/images/icon.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
        RENAME lunacraft.png
    )
endif()

#
# CPack
#

set(CPACK_PACKAGE_NAME "Lunacraft")
set(CPACK_PACKAGE_VENDOR "sepulous")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

if(WIN32)
    set(CPACK_GENERATOR "INNOSETUP")
    set(CPACK_INNOSETUP_DISPLAY_NAME "Lunacraft")
    set(CPACK_INNOSETUP_PACKAGE_NAME "Lunacraft")
    set(CPACK_INNOSETUP_MENU_LINKS "Lunacraft.exe" "Lunacraft")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")  # produces .dmg
else()
    set(CPACK_GENERATOR "TGZ")  # .tar.gz fallback for Linux
endif()

include(CPack)
