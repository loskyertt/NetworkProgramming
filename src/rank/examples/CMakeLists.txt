# src/rank/examples/CMakeLists.txt

add_executable(sorted_set_example sorted_set_example.cpp)
target_include_directories(sorted_set_example PRIVATE ../include)

add_test(NAME sorted_set_example COMMAND sorted_set_example)
