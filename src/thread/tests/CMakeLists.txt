# src/thread/tests/CMakeLists.txt
add_executable(test_thread_pool01 test_thread_pool01.cpp)
target_link_libraries(test_thread_pool01 PRIVATE thread)

add_executable(test_thread_pool02 test_thread_pool02.cpp)
target_link_libraries(test_thread_pool02 PRIVATE thread)