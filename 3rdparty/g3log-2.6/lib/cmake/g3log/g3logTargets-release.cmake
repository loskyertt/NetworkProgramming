#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "g3log" for configuration "Release"
set_property(TARGET g3log APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(g3log PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libg3log.a"
  )

list(APPEND _cmake_import_check_targets g3log )
list(APPEND _cmake_import_check_files_for_g3log "${_IMPORT_PREFIX}/lib/libg3log.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
