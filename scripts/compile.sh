
#!/bin/bash
# Get the script location and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Change to project root if not already there
if [[ "$(pwd)" != "$PROJECT_ROOT" ]]; then
    cd "$PROJECT_ROOT"
fi

# If ./compile.sh clean
if [ "$1" == "clean" ]; then
    rm -rf build
fi

# Check if build directory exists
if [ ! -d build ]; then
    mkdir build
fi

cd build

case "$1" in
    "release")
        cmake -DCMAKE_BUILD_TYPE=Release ..
        echo "Release mode enabled"
        ;;
    *)
        cmake -DCMAKE_BUILD_TYPE=Debug .. 
        echo "Debug mode enabled"
        ;;
esac

make

echo "Compilation successful."