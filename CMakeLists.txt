# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(TetrisGame VERSION 1.0)

# Set the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Set the directories for sources and includes
set(SRC_DIR src)
set(PROTOCOL_DIR ${SRC_DIR}/protocol)
set(INCLUDE_DIR ${SRC_DIR}/include)
set(DB_DIR ${SRC_DIR}/db)  # Add directory for database sources

# Add the executable for the server with PostgreSQL and UUID dependencies
add_executable(server ${SRC_DIR}/server/server.c ${SRC_DIR}/ultis.c)
target_include_directories(server PRIVATE ${PROTOCOL_DIR} /usr/include/postgresql)
target_link_libraries(server PRIVATE pthread pq uuid)  # Link with pthread, libpq, and libuuid

# Add the executable for the test client menu
add_executable(test_client_menu ${SRC_DIR}/client/test_client_menu.c ${PROTOCOL_DIR}/protocol.h)
target_include_directories(test_client_menu PRIVATE ${PROTOCOL_DIR})
target_link_libraries(test_client_menu PRIVATE pthread)

# Add the executable for the Tetris offline game
add_executable(tetris_offline ${SRC_DIR}/main.c ${SRC_DIR}/tetris_game.c)
target_include_directories(tetris_offline PRIVATE ${SRC_DIR} ${INCLUDE_DIR})
target_link_libraries(tetris_offline PRIVATE pthread SDL2 SDL2_ttf)

# Link ncurses library (if needed)
target_link_libraries(tetris_offline PRIVATE ncurses)

# Add the executable for the db with PostgreSQL dependencies
add_executable(db ${DB_DIR}/db.c)
target_include_directories(db PRIVATE /usr/include/postgresql)
target_link_libraries(db PRIVATE pq)

# Set output directories for executables
set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(test_client_menu PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(tetris_offline PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(db PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Optional: Add debugging flags for easier development
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(tetris_offline PRIVATE DEBUG)
endif()

# Add custom compile options or flags if necessary
# target_compile_options(tetris_offline PRIVATE -D CUSTOM_DEFINE=1)
