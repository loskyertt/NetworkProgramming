# src/CMakeLists.txt
set(TARGET ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})

# utils 模块：编译为静态库
add_library(utils STATIC
  utils/impl/logger.cpp
)
target_include_directories(utils
  PUBLIC  utils/include
  PRIVATE utils/impl
)

# socket 模块：编译为静态库
add_library(socket STATIC
  socket/impl/socket.cpp
  socket/impl/server_socket.cpp
  socket/impl/client_socket.cpp
  socket/impl/selector.cpp
  socket/impl/scoket_handler.cpp
)
target_include_directories(socket
  PUBLIC  socket/include
  PRIVATE socket/impl
)
target_link_libraries(socket
  PRIVATE utils
)

# 主可执行文件
# 服务端
add_executable(server server.cpp)
target_link_libraries(server PRIVATE socket utils)
# 客户端
add_executable(client client.cpp)
target_link_libraries(client PRIVATE socket utils)
