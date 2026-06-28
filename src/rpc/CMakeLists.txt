# src/rpc/CMakeLists.txt

# rpc 模块：编译为静态库
add_library(rpc STATIC
  impl/rpc_serializer.cpp
  impl/rpc_message.cpp
  impl/rpc_channel.cpp
  impl/rpc_server.cpp
)
target_include_directories(rpc
  PUBLIC  include
  PRIVATE impl
)
target_link_libraries(rpc
  PUBLIC socket thread utility
)

add_subdirectory(tests)
