# examples/CMakeLists.txt
file(GLOB_RECURSE ALL_SRCS *.cpp)
foreach(v ${ALL_SRCS})
  message("Processing: ${v}")
  get_filename_component(TARGET_NAME ${v} NAME_WE) # 生成目标名（去掉路径和扩展名）

  add_executable(${TARGET_NAME} ${v})
endforeach()

# RPC 示例需要链接 rpc 库
target_link_libraries(rpc_calculator_server PRIVATE rpc)
target_link_libraries(rpc_calculator_client PRIVATE rpc)
