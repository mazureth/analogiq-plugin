#!/bin/bash
set -e

# Check if Doxygen is available
if ! command -v doxygen &> /dev/null; then
    echo "❌ Doxygen is not installed. Please install it first:"
    echo "   macOS: brew install doxygen"
    echo "   Ubuntu/Debian: sudo apt-get install doxygen"
    echo "   Windows: Download from https://www.doxygen.nl/"
    exit 1
fi

echo "Configuring project with CMake..."
cmake -B build

echo "Building documentation..."
cmake --build build --target docs

DOCS_INDEX="docs/html/index.html"
if [ -f "$DOCS_INDEX" ]; then
  read -p "Documentation built. Would you like to open it in your browser? (y/n): " open_in_browser
  if [[ "$open_in_browser" =~ ^[Yy]$ ]]; then
    echo "Opening documentation in browser..."
    open "$DOCS_INDEX"
  else
    echo "You can view the documentation at $DOCS_INDEX"
  fi
else
  echo "Documentation index not found at $DOCS_INDEX"
fi 