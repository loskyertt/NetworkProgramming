# tests/CMakeLists.txt

add_executable(unit_tests
  unit/test_circle.cpp
  unit/test_rectangle.cpp
)

target_link_libraries(unit_tests
  PRIVATE core GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
