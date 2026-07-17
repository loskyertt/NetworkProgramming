# src/rank/CMakeLists.txt

get_filename_component(_pkg_root
        "${CMAKE_CURRENT_LIST_DIR}"
        ABSOLUTE
)

if (NOT TARGET rank::sorted_set)
    add_library(rank::sorted_set INTERFACE IMPORTED)

    set_target_properties(rank::sorted_set PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${_pkg_root}/include"
    )
endif ()

unset(_pkg_root)

add_subdirectory(examples)
