#!/bin/bash

# run script: dos2unix build_project.sh
# start db: sudo service postgresql start
# enter db: sudo -i -u postgres psql > \c tetris
# gcc main.c tetris_game.c client_utils.c ultis.c -o game.o -lSDL2 -lSDL2_ttf -lpthread
#gcc -o test_game test_object.c ../src/server/object.c ../src/ultis.c -I../src/server 
#gcc -g -o test_game test_object.c ../src/server/object.c ../src/ultis.c -I../src/server -I/usr/include/postgresql -lpq

# Set the project root directory
PROJECT_ROOT=$(pwd)

# Create the build directory if it doesn't exist
if [ ! -d "$PROJECT_ROOT/build" ]; then
    echo "Creating 'build' directory..."
    mkdir build
else
    echo "'build' directory already exists."
fi

# Navigate into the build directory
cd build

# Run CMake to configure the project
echo "Running CMake configuration..."
cmake ..

# Build the project
echo "Building the project..."
make

echo "Building server..."
make server

echo "Building test_client_menu..."
make test_client_menu

echo "Building tetris_offline..."
make tetris_offline

echo "Build process completed."

cd ..
