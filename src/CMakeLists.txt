# src/CMakeLists.txt
set(TARGET ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})

# utility 模块：编译为静态库
add_library(utility STATIC
  utility/impl/logger.cpp
)
target_include_directories(utility
  PUBLIC  utility/include
  PRIVATE utility/impl
)

# socket 模块：编译为静态库
add_library(socket STATIC
  socket/impl/socket.cpp
  socket/impl/server_socket.cpp
  socket/impl/client_socket.cpp
  # --- select ---
  socket/impl/select/selector.cpp
  socket/impl/select/select_handler.cpp
  # --- poll ---
  socket/impl/poll/poller.cpp
  socket/impl/poll/poll_handler.cpp
  # --- epoll ---
  socket/impl/epoll/epoller.cpp
)
target_include_directories(socket
  PUBLIC  socket/include
  PRIVATE socket/impl
)
target_link_libraries(socket
  PRIVATE utility
)

# thread 模块：编译为静态库
add_library(thread STATIC
  thread/impl/thread_pool.cpp
)
target_include_directories(thread
  PUBLIC  thread/include
  PRIVATE thread/impl
)
target_link_libraries(thread
  PRIVATE utility
)

# 主可执行文件
# 服务端
add_executable(server server.cpp)
target_link_libraries(server PRIVATE socket utility)
# 客户端
add_executable(client client.cpp)
target_link_libraries(client PRIVATE socket utility)

add_subdirectory(socket/tests)
add_subdirectory(thread/tests)
