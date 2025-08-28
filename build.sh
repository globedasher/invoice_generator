#!/bin/bash

# Build script for Invoice Generator C++ application

echo "Building Invoice Generator..."

# Create build directory
mkdir -p build
cd build

# Run CMake configuration
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

# Build the project
echo "Building project..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build completed successfully!"
echo "Executable location: build/InvoiceGeneratorGUI"