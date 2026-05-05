cmake_minimum_required(VERSION 3.12.4)

project(dabon-player)

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb")

file(GLOB prj_src ${CMAKE_CURRENT_LIST_DIR}/prj/*.c)
file(GLOB eez/src/ui ${CMAKE_CURRENT_LIST_DIR}/ui/*.c)

add_executable(dabon-player
    ${prj_src}
    ${ui_src}
)

target_compile_definitions(dabon-player PRIVATE
    -DSOCKET_PATH="/var/lib/mpd/socket"
)

target_include_directories(dabon-player PRIVATE eez/src/ui)
target_include_directories(dabon-player PRIVATE prj)

find_package(PkgConfig REQUIRED)

# lvgl
pkg_check_modules(LVGL REQUIRED lvgl)
target_link_libraries(dabon-player ${LVGL_LIBRARIES})
target_include_directories(dabon-player PRIVATE ${LVGL_INCLUDE_DIRS})

# libdrm
pkg_check_modules(LIBDRM REQUIRED libdrm)
target_include_directories(dabon-player PRIVATE ${LIBDRM_INCLUDE_DIRS})


install(TARGETS dabon-player DESTINATION /usr/bin)