# src/rpc/tests/CMakeLists.txt

# 自动化测试
set(RPC_TEST_TARGETS
  test_rpc_serializer
  test_rpc_message
  test_rpc_channel_server
)

foreach(target ${RPC_TEST_TARGETS})
  add_executable(${target} ${target}.cpp)
  target_link_libraries(${target} PRIVATE rpc GTest::gtest_main)
  add_test(NAME ${target} COMMAND ${target})
endforeach()
