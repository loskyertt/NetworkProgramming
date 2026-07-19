# src/rank/CMakeLists.txt

if (NOT TARGET rank::sorted_set)
    add_library(rank::sorted_set INTERFACE IMPORTED)

    set_target_properties(rank::sorted_set PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
    )
endif ()

add_subdirectory(examples)
