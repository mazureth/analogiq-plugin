#!/bin/bash

# Exit on error
set -e

# Check for any flag
FLAG=""
if [ ! -z "$1" ]; then
    FLAG="$1"
fi

# Build the project
cmake --build build --config Release $FLAG

echo "Build completed successfully!" 