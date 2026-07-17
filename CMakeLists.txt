cmake_minimum_required(VERSION 3.20.0)

# 导出编译命令（用于 clangd）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(network_programming VERSION 0.0.1 LANGUAGES C CXX)

# 设置标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# 开启 gtest_discover_tests 注册测试用例依赖
enable_testing()

# 脚本
#include(cmake/check_arch.cmake)

# 第三方库
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/3rdparty/g3log-2.6")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/3rdparty/googletest-1.17.0")

find_package(g3log REQUIRED)
find_package(GTest REQUIRED)

# 子模块
# add_subdirectory(tests)
add_subdirectory(examples)
add_subdirectory(src)
