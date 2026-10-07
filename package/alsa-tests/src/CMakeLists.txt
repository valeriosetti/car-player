cmake_minimum_required(VERSION 3.10)

project(alsa-tests C)

# Locate libasound (ALSA) via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(ALSA REQUIRED alsa)

# Build one binary per *.c source file in this directory
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

foreach(src ${TEST_SOURCES})
    get_filename_component(target ${src} NAME_WE)

    add_executable(${target} ${src})

    target_include_directories(${target} PRIVATE ${ALSA_INCLUDE_DIRS})
    target_link_libraries(${target} PRIVATE ${ALSA_LIBRARIES} m)
    target_compile_options(${target} PRIVATE ${ALSA_CFLAGS_OTHER})

    install(TARGETS ${target} RUNTIME DESTINATION bin)
endforeach()
