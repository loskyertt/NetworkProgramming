# src/CMakeLists.txt

# thread 模块：编译为静态库
add_library(thread STATIC
  impl/thread_pool.cpp
)
target_include_directories(thread
  PUBLIC  include
  PRIVATE impl
)
target_link_libraries(thread
  PRIVATE utility
)

# add_subdirectory(tests)
