# src/CMakeLists.txt

# socket 模块：编译为静态库
add_library(socket STATIC
  impl/socket.cpp
  impl/server_socket.cpp
  impl/client_socket.cpp
  # --- select ---
  impl/select/selector.cpp
  impl/select/select_handler.cpp
  # --- poll ---
  impl/poll/poller.cpp
  impl/poll/poll_handler.cpp
  # --- epoll ---
  impl/epoll/epoller.cpp
)
target_include_directories(socket
  PUBLIC  include
  PRIVATE impl
)
target_link_libraries(socket
  PRIVATE utility
)

add_subdirectory(tests)
