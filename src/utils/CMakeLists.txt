# src/utils/CMakeLists.txt

# utils 模块：编译为静态库
add_library(utils STATIC
  impl/logger.cpp
)
target_include_directories(utils
  PUBLIC  include
  PRIVATE impl
)

# add_subdirectory(tests)
