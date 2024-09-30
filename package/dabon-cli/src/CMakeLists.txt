cmake_minimum_required(VERSION 3.18)

project(dabon-cli)

add_executable(dabon-cli)

set(WARNINGS "-Wall -Wextra")

set(CMAKE_BUILD_TYPE Debug)
set(DEBUG_C_FLAGS "-O0 -ggdb")

set(CMAKE_C_FLAGS "${WARNINGS} ${DEBUG_C_FLAGS}")

include(${CMAKE_CURRENT_LIST_DIR}/si468x_lib/CMakeLists.txt)

target_sources(dabon-cli PRIVATE
    main.c
    shell_cmds.c
)

target_include_directories(dabon-cli PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/si468x_lib
)

target_link_libraries(dabon-cli
    si468x_lib
)

install(TARGETS dabon-cli DESTINATION /usr/bin)
