#!/bin/bash

# Exit on error
set -e

# Check for any flag
FLAG=""
if [ ! -z "$1" ]; then
    FLAG="$1"
fi

# Remove build directory if it exists
if [ -d "build" ]; then
    echo "Removing existing build directory..."
    rm -rf build
fi

# Configure and build the project
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release $FLAG

echo "Clean build completed successfully!" 