#!/bin/bash

set -e

BUILD_TYPE=""
RUN_AFTER_BUILD=false

usage() {
    echo "Usage: $0 [debug|release] [--run]"
    exit 1
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        debug|Debug)
            BUILD_TYPE="Debug"
            ;;
        release|Release)
            BUILD_TYPE="Release"
            ;;
        --run)
            RUN_AFTER_BUILD=true
            ;;
        *)
            usage
            ;;
    esac
    shift
done

if [ -z "$BUILD_TYPE" ]; then
    usage
fi

# Set build dir based on type
BUILD_DIR="build/${BUILD_TYPE,,}"  # lowercase
echo ">> Building in $BUILD_DIR ($BUILD_TYPE mode)"

# cd $BUILD_DIR
# Configure with CMake
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=$BUILD_TYPE

if [ ! -f $BUILD_DIR/compile_commands.json ]; then
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build
    ln -sf build/compile_commands.json .
fi

# Build
cmake --build "$BUILD_DIR" -- -j$(nproc)

# Run if requested
if [ "$RUN_AFTER_BUILD" = true ]; then
    echo ">> Running cli-chat-server..."
    ASAN_OPTIONS=detect_leaks=1 "$BUILD_DIR/cli-chat-server"
fi
