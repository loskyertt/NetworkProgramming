# src/thread/CMakeLists.txt

# thread 模块
add_library(thread STATIC
  impl/thread_pool.cpp
)
target_include_directories(thread
  PUBLIC  include
  PRIVATE impl
)

 add_subdirectory(tests)
