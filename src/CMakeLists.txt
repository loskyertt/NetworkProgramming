# src/CMakeLists.txt
set(TARGET ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})

# utility 模块
add_subdirectory(utility)

# socket 模块
add_subdirectory(socket)

# thread 模块
add_subdirectory(thread)

# rpc 模块
add_subdirectory(rpc)

# memory_pool 模块
add_subdirectory(memory)

# 主可执行文件
# 服务端
add_executable(server server.cpp)
target_link_libraries(server PRIVATE socket utility)
# 客户端
add_executable(client client.cpp)
target_link_libraries(client PRIVATE socket utility)
