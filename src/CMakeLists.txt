# src/CMakeLists.txt
set(TARGET ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})

# utils 模块：编译为静态库
add_library(utils STATIC
  utils/impl/logger.cpp
)
target_include_directories(utils
  PUBLIC  utils/include
  PRIVATE utils/impl
)

# socket 模块：编译为静态库
add_library(socket STATIC
  socket/impl/socket.cpp
)
target_include_directories(socket
  PUBLIC  socket/include
  PRIVATE socket/impl
)
target_link_libraries(socket
  PRIVATE utils
)

if(MSVC)
  set_target_properties(utils PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# 主可执行文件
add_executable(app main.cpp)
target_link_libraries(app PRIVATE socket utils)
