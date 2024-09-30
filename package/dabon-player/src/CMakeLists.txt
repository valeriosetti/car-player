cmake_minimum_required(VERSION 3.12.4)

project(dabon-player)

add_executable(dabon-player)

set(WARNINGS "-Wall -Wextra")

set(CMAKE_BUILD_TYPE Debug)
set(DEBUG_C_FLAGS "-O0 -ggdb")

if (SKIP_WELCOME_SCREEN)
	message(STATUS "Skip welcome screen")
	set(PLAYER_C_FLAGS "${PLAYER_C_FLAGS} -DSKIP_WELCOME_SCREEN")
else()
	message(STATUS "Keep welcome screen")
endif()

set(CMAKE_C_FLAGS "${WARNINGS} ${PLAYER_C_FLAGS} ${DEBUG_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${WARNINGS} ${PLAYER_C_FLAGS} ${DEBUG_C_FLAGS}")

target_include_directories(dabon-player PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/player/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl/src
    ${CMAKE_CURRENT_SOURCE_DIR}/si468x_lib
)

target_sources(dabon-player PRIVATE
    player/src/main.c
    player/src/welcome_screen.c
    player/src/main_selection_screen.c
    player/src/music_player.c
    player/src/fm_player.c
    player/src/file_explorer.c
    player/src/spectrum.c
    player/src/gstreamer.c
)

# Add libdrm folder otherwise LVGL build with DRM support fails
find_path(DRM_HEADER "drm.h" PATH_SUFFIXES "libdrm" REQUIRED)
target_include_directories(dabon-player PRIVATE ${DRM_HEADER})

# Change CMP0077 policy otherwise the option() function's policy in old cmake
# might clear the setting of LV_CONF_PATH that we are doing here.
# set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(LV_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/player/inc/lv_conf.h)
include(${CMAKE_CURRENT_SOURCE_DIR}/lvgl/CMakeLists.txt)
target_include_directories(lvgl PRIVATE ${DRM_HEADER})
target_link_libraries(dabon-player lvgl)

include(${CMAKE_CURRENT_SOURCE_DIR}/si468x_lib/CMakeLists.txt)
target_link_libraries(dabon-player si468x_lib)

find_path(GSTREAMER_HEADER "gst/gst.h" PATH_SUFFIXES "gstreamer-1.0" REQUIRED)
target_include_directories(dabon-player PRIVATE ${GSTREAMER_HEADER})
find_path(GLIB_HEADER "glib.h" PATH_SUFFIXES "glib-2.0" REQUIRED)
target_include_directories(dabon-player PRIVATE ${GLIB_HEADER})
find_path(GLIBCONFIG_HEADER "glibconfig.h" PATH_SUFFIXES "lib/glib-2.0/include" REQUIRED)
target_include_directories(dabon-player PRIVATE ${GLIBCONFIG_HEADER})

function(add_library LIB_NAME)
    set(OUT_VAR ${LIB_NAME}_VAR)
    find_library(${OUT_VAR} ${LIB_NAME} REQUIRED)
    message(STATUS "Adding library: ${${OUT_VAR}}")
    target_link_libraries(dabon-player ${${OUT_VAR}})
endfunction()

add_library(pthread)
add_library(drm)
add_library(png)
add_library(gstreamer-1.0)
add_library(gobject-2.0)
add_library(glib-2.0)
