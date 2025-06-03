#!/bin/bash

# Exit on error
set -e

# Create build directory if it doesn't exist
mkdir -p build

# Navigate to build directory
cd build

# Configure with CMake
cmake .. -DANALOGIQ_BUILD_TESTS=ON

# Build the tests
cmake --build . --target analogiq_tests

# Run the tests
./tests/analogiq_tests