cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Lab4)

# Set the C++ standard and specify the compiler
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER clang++)

# Define paths
set(IMAGES_DIR "${CMAKE_SOURCE_DIR}/images")

# Add the executable
add_executable(Lab4
    Main.cpp
    src/ImageManager.cpp
)

# Include directories
target_include_directories(Lab4 PRIVATE src)

# Add compile definitions
target_compile_definitions(Lab4 PRIVATE
    IMAGES_DIR="${IMAGES_DIR}"
)