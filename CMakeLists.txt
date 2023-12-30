cmake_minimum_required(VERSION 3.12)
project(YourLLVMProject)

# 设置LLVM安装路径
set(LLVM_DIR "/usr/bin")
list(APPEND CMAKE_PREFIX_PATH "${LLVM_DIR}")

# 查找LLVM包
find_package(LLVM REQUIRED CONFIG)

# 包含LLVM头文件
include_directories(${LLVM_INCLUDE_DIRS})

# 添加定义
add_definitions(${LLVM_DEFINITIONS})

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)

# 添加可执行文件
add_executable(brainfc main.cpp)

# 链接LLVM库
target_link_libraries(brainfc LLVM)
