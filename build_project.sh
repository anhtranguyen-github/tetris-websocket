#!/bin/bash

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

echo "Build process completed."

cd ..
