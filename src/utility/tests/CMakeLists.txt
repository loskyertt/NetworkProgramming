# tests/CMakeLists.txt
add_executable(test_logger test_logger.cpp)
target_link_libraries(test_logger PRIVATE utility)

add_executable(test_my_g3log test_my_g3log.cpp)
target_link_libraries(test_my_g3log PRIVATE utility)
