cmake_minimum_required(VERSION 3.10)

# Set the C++ compiler
set(CMAKE_CXX_COMPILER clang++)

# Set the project name and version
project(Lab5 VERSION 1.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define paths
set(IMAGES_DIR "${CMAKE_SOURCE_DIR}/images")

# Add the executable
add_executable(Lab5
    Main.cpp
    src/ImageManager.cpp
    src/FrequencyDomainManager.cpp
)

# Include directories
target_include_directories(Lab5 PRIVATE src)

# Add compile definitions
target_compile_definitions(Lab5 PRIVATE
    IMAGES_DIR="${IMAGES_DIR}"
)