cmake_minimum_required(VERSION 3.12)
project(YourLLVMProject)

# Set the LLVM installation directory
set(LLVM_DIR "/usr/lib/llvm-10/cmake")
list(APPEND CMAKE_PREFIX_PATH "${LLVM_DIR}")

# Find LLVM package
find_package(LLVM REQUIRED CONFIG)

# Include LLVM-related directories
include_directories(${LLVM_INCLUDE_DIRS})

# Add definitions
add_definitions(${LLVM_DEFINITIONS})

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Add your source files and executable
add_executable(brainfc main.cpp)

# Link against LLVM
target_link_libraries(brainfc PRIVATE LLVM)
