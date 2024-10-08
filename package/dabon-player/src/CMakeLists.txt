cmake_minimum_required(VERSION 3.12.4)

project(dabon-player)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
include(utils)

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb")

if (SKIP_WELCOME_SCREEN)
	message(STATUS "Skip welcome screen")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSKIP_WELCOME_SCREEN")
else()
	message(STATUS "Keep welcome screen")
endif()

add_executable(dabon-player
    player/src/main.c
    player/src/welcome_screen.c
    player/src/main_selection_screen.c
    player/src/music_player.c
    player/src/fm_player.c
    player/src/file_explorer.c
    player/src/spectrum.c
    player/src/gstreamer.c
)

target_include_directories(dabon-player PRIVATE .)

add_subdirectory(libsi468x)
target_link_libraries(dabon-player si468x)

# Add libdrm folder otherwise LVGL build with DRM support fails
find_path(DRM_HEADER "drm.h" PATH_SUFFIXES "libdrm" REQUIRED)
target_include_directories(dabon-player PRIVATE ${DRM_HEADER})

# Change CMP0077 policy otherwise the option() function's policy in old cmake
# might clear the setting of LV_CONF_PATH that we are doing here.
# set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(LV_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/player/inc/lv_conf.h)
#include(${CMAKE_CURRENT_SOURCE_DIR}/lvgl/CMakeLists.txt)
add_subdirectory(lvgl)
target_include_directories(lvgl PRIVATE ${DRM_HEADER})
target_link_libraries(dabon-player lvgl)

find_path(GSTREAMER_HEADER "gst/gst.h" PATH_SUFFIXES "gstreamer-1.0" REQUIRED)
target_include_directories(dabon-player PRIVATE ${GSTREAMER_HEADER})
find_path(GLIB_HEADER "glib.h" PATH_SUFFIXES "glib-2.0" REQUIRED)
target_include_directories(dabon-player PRIVATE ${GLIB_HEADER})
find_path(GLIBCONFIG_HEADER "glibconfig.h" PATH_SUFFIXES "lib/glib-2.0/include" REQUIRED)
target_include_directories(dabon-player PRIVATE ${GLIBCONFIG_HEADER})

find_and_link_library(pthread dabon-player)
find_and_link_library(drm dabon-player)
find_and_link_library(png dabon-player)
find_and_link_library(gstreamer-1.0 dabon-player)
find_and_link_library(gobject-2.0 dabon-player)
find_and_link_library(glib-2.0 dabon-player)
