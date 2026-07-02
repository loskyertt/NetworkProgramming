# src/memory/CMakeLists.txt

# rpc 模块：编译为静态库
add_library(memory STATIC
        impl/memory_pool/fixed_memory_pool.cpp
        impl/memory_pool/thread_safe_pool.cpp
)
target_include_directories(memory
        PUBLIC include
        PRIVATE impl
)

add_subdirectory(tests)
