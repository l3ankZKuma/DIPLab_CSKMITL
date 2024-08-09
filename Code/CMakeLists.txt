cmake_minimum_required(VERSION 3.16)  # Increased to 3.16 for better PCH support

# Set the C++ compiler
set(CMAKE_CXX_COMPILER clang++)

# Set the project name and version
project(Prog VERSION 1.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable PCH support
set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)

# Define paths
set(IMAGES_DIR "${CMAKE_SOURCE_DIR}/images")

# Add the executable
add_executable(
    Prog
    Main.cpp
    src/ImageManager.cpp
)

# Include directories
target_include_directories(Prog PRIVATE src)

# Add compile definitions
target_compile_definitions(Prog PRIVATE
    IMAGES_DIR="${IMAGES_DIR}"
)

# Add precompiled header
target_precompile_headers(Prog PRIVATE src/pch.h)

# Set optimization flags
target_compile_options(Prog PRIVATE
    -O3
    -march=native
    -mtune=native
)

# Enable Link Time Optimization (LTO)
set_target_properties(Prog PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)