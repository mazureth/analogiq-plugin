#!/bin/bash
set -e

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