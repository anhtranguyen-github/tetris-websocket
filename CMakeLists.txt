# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(TetrisGame VERSION 1.0)

# Set the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Add the executable
add_executable(tetris
    src/main.c
    src/tetris_game.c
    src/websocket_server.c
    src/client_handler.c
    src/utils.c
)

# Include directories
target_include_directories(tetris PRIVATE src config)

# Find SDL2 library
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(tetris ${SDL2_LIBRARIES})

# Find SDL2_ttf library using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
include_directories(${SDL2_TTF_INCLUDE_DIRS})
target_link_libraries(tetris ${SDL2_TTF_LIBRARIES})

# Link the ncurses library for terminal handling
target_link_libraries(tetris ncurses)

# Optional: Link additional libraries if needed (e.g., pthreads for multithreading)
# find_package(Threads REQUIRED)
# target_link_libraries(tetris PRIVATE Threads::Threads)
