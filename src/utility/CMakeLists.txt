# src/utility/CMakeLists.txt

# utility 模块：编译为静态库
add_library(utility STATIC
  impl/logger.cpp
)
target_include_directories(utility
  PUBLIC  include
  PRIVATE impl
)

# add_subdirectory(tests)
