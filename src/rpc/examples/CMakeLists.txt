# src/rpc/examples/CMakeLists.txt

add_executable(rpc_calculator_server rpc_calculator_server.cpp)
target_link_libraries(rpc_calculator_server PRIVATE rpc)

add_executable(rpc_calculator_client rpc_calculator_client.cpp)
target_link_libraries(rpc_calculator_client PRIVATE rpc)
