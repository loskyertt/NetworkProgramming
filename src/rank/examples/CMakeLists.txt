# src/rank/examples/CMakeLists.txt

add_executable(sorted_set_example sorted_set_example.cpp)
target_link_libraries(sorted_set_example PRIVATE rank::sorted_set)

add_test(NAME sorted_set_example COMMAND sorted_set_example)
