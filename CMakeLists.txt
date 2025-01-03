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
add_executable(server 
    ${SRC_DIR}/server/server.c 
    ${SRC_DIR}/ultis.c 
    ${PROTOCOL_DIR}/network.c 
    ${PROTOCOL_DIR}/protocol.h 
    ${SRC_DIR}/server/database.h 
    ${SRC_DIR}/server/object.h
)

# Include directories for server
target_include_directories(server PRIVATE 
    ${PROTOCOL_DIR}          # Protocol-related headers
    ${SRC_DIR}/server        # Server-specific headers
    ${SRC_DIR}/config        # Configuration headers
    /usr/include/postgresql  # PostgreSQL headers
)

# Link required libraries for server
target_link_libraries(server PRIVATE 
    pthread    # Multithreading support
    pq         # PostgreSQL client library
    uuid       # UUID generation
)

# Add the executable for the db with PostgreSQL dependencies
add_executable(db ${DB_DIR}/db.c)
target_include_directories(db PRIVATE /usr/include/postgresql)
target_link_libraries(db PRIVATE pq)

# Set output directories for executables
set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_target_properties(db PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Optional: Add debugging flags for easier development
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
endif()

# Add custom compile options or flags if necessary
# target_compile_options(tetris_offline PRIVATE -D CUSTOM_DEFINE=1)
