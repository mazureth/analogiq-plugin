#!/bin/bash

set -e  # exit on error
set -o pipefail  # propagate coverage failure through pipelines

# Configuration
COVERAGE_THRESHOLD=10.0
BUILD_DIR="build"
TEST_BINARY="${BUILD_DIR}/tests/analogiq_tests"
RAW_PROFILE="${BUILD_DIR}/default.profraw"
MERGED_PROFILE="default.profdata"
HTML_OUTPUT_DIR="coverage_html"

# Step 1: Prepare build
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
find . -name "*.gcda" -delete

# Step 2: Configure and build tests
cmake .. -DANALOGIQ_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="--coverage"
cmake --build . --target analogiq_tests

# Move back to the root directory
cd ..

# Step 3: Run tests and generate profraw
LLVM_PROFILE_FILE="$RAW_PROFILE" "$TEST_BINARY"

# Step 4: Convert to .profdata
llvm-profdata merge -sparse "$RAW_PROFILE" -o "$MERGED_PROFILE"

# Step 5: Show terminal report
echo "---------------- Coverage Report ----------------"
COVERAGE_OUTPUT=$(llvm-cov report "$TEST_BINARY" -instr-profile="$MERGED_PROFILE" -ignore-filename-regex=".*(_deps|tests).*" || true)
echo "$COVERAGE_OUTPUT"

# Step 6: Check overall coverage and fail if below threshold
TOTAL_LINE_COVERAGE=$(echo "$COVERAGE_OUTPUT" | awk '/TOTAL/ { print $(13) }' | tr -d '%')
if (( $(echo "$TOTAL_LINE_COVERAGE < $COVERAGE_THRESHOLD" | bc -l) )); then
  echo "❌ Coverage $TOTAL_LINE_COVERAGE% is below threshold of $COVERAGE_THRESHOLD%"
  exit 1
else
  echo "✅ Coverage $TOTAL_LINE_COVERAGE% meets threshold of $COVERAGE_THRESHOLD%"
fi

# Step 7 (optional): Generate HTML report
llvm-cov show "$TEST_BINARY" \
  -instr-profile="$MERGED_PROFILE" \
  -format=html -output-dir="$HTML_OUTPUT_DIR" \
  -ignore-filename-regex=".*(_deps|tests).*"

echo "📄 HTML coverage report generated at: $HTML_OUTPUT_DIR/index.html"
