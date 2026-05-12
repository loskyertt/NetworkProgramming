# src/utility/CMakeLists.txt

# utility 模块：编译为静态库
add_library(utility STATIC
  impl/logger.cpp
  impl/custom_file_sink.cpp
)
target_include_directories(utility
  PUBLIC  include
  PRIVATE impl
)
target_link_libraries(utility
  PUBLIC g3log
)

add_subdirectory(tests)
