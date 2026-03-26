#!/bin/bash

# run_all_tests.sh - Run all test suites
#
# Usage: ./run_all_tests.sh

set -e

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"

# Build the object file
echo "Building animate.o..."
cd "$PARENT_DIR" && make animate.o && cd "$SCRIPT_DIR"

# Compile test programs
echo ""
echo "Compiling tests..."

gcc -Wall -Wextra -fsanitize=address -I../include \
    test_canvas.c ../animate.o -o test_canvas -lm

gcc -Wall -Wextra -fsanitize=address -I../include \
    test_sprite.c ../animate.o -o test_sprite -lm

gcc -Wall -Wextra -fsanitize=address -I../include \
    test_placement.c ../animate.o -o test_placement -lm

gcc -Wall -Wextra -fsanitize=address -I../include \
    test_frame.c ../animate.o -o test_frame -lm

# Run tests
echo ""
echo "Running tests..."
echo ""

echo "=== Canvas Tests ==="
./test_canvas
echo ""

echo "=== Sprite Tests ==="
./test_sprite
echo ""

echo "=== Placement Tests ==="
./test_placement
echo ""

echo "=== Frame Tests ==="
./test_frame
echo ""

# Clean up test executables
rm -f test_canvas test_sprite test_placement test_frame

echo "=========================================="
echo "All tests completed!"
echo "=========================================="
