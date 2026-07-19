# src/utility/CMakeLists.txt

# utility::logger 模块
add_library(utility_logger STATIC
        impl/logger/logger.cpp
)
target_include_directories(utility_logger
  PUBLIC  include
  PRIVATE impl
)

if (NOT TARGET utility::logger)
    add_library(utility::logger ALIAS utility_logger)
endif ()

# utility::g3log 模块
add_library(utility_g3log STATIC
        impl/logger/custom_file_sink.cpp
)
target_include_directories(utility_g3log
        PUBLIC  include
        PRIVATE impl
)

if(g3log_FOUND)
    target_link_libraries(utility_g3log PUBLIC g3log)
else()
    message(FATAL_ERROR "g3log not found, disable logging")
endif()

if (NOT TARGET utility::g3log)
    add_library(utility::g3log ALIAS utility_g3log)
endif ()

# 测试子目录
add_subdirectory(tests)
