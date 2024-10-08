cmake_minimum_required(VERSION 3.18)

project(dabon-cli)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
include(utils)

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb")

add_subdirectory(libsi468x)

add_executable(dabon-cli
    main.c
    shell_cmds.c
)

target_link_libraries(dabon-cli
    si468x
)

install(TARGETS dabon-cli DESTINATION /usr/bin)
