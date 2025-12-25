cmake_minimum_required(VERSION 3.28)
project(plane_management)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(raylib CONFIG REQUIRED)

add_executable(app
    src/objects.cpp
    src/game.cpp
    src/utils.cpp
    src/utils.cpp
    src/main.cpp
)

target_compile_options(app PRIVATE -g3)
target_include_directories(app PRIVATE libs/)

target_link_libraries(app PRIVATE raylib)

add_custom_command(TARGET app POST_BUILD # for lsp support on linux
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
      )
