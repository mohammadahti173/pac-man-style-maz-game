cmake_minimum_required(VERSION 3.20)

project(PacManMaze LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build a Release game by default when no build type is given.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# SFML 3 is required. The MSYS2 package name is mingw-w64-ucrt-x86_64-sfml.
find_package(SFML 3 COMPONENTS Graphics Audio REQUIRED)

add_executable(PacManMaze
    src/main.cpp
    src/Game.cpp
    src/Maze.cpp
    src/Player.cpp
    src/Enemy.cpp
    src/Menu.cpp
    src/SoundManager.cpp
)

target_include_directories(PacManMaze PRIVATE include)

target_link_libraries(PacManMaze PRIVATE
    SFML::Graphics
    SFML::Audio
)

# Copy the assets next to the executable so the game can always find its
# fonts and sounds, even when it is started from the build folder.
add_custom_command(TARGET PacManMaze POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:PacManMaze>/assets
    COMMENT "Copying game assets..."
)

# On Windows, show a console window so helpful messages are visible.
# The game itself uses a normal SFML render window.
if(WIN32)
    target_compile_definitions(PacManMaze PRIVATE NOMINMAX)
endif()
