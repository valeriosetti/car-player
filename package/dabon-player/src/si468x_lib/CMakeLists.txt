add_library(si468x_lib STATIC
    ${CMAKE_CURRENT_LIST_DIR}/si468x_common.c
    ${CMAKE_CURRENT_LIST_DIR}/si468x_dab.c
    ${CMAKE_CURRENT_LIST_DIR}/si468x_ext.c
    ${CMAKE_CURRENT_LIST_DIR}/si468x_fm.c
    ${CMAKE_CURRENT_LIST_DIR}/si468x_platform.c
    ${CMAKE_CURRENT_LIST_DIR}/si468x_utils.c
)

target_include_directories(si468x_lib PRIVATE ${CMAKE_CURRENT_LIST_DIR})

function(add_library LIB_NAME)
    set(OUT_VAR ${LIB_NAME}_VAR)
    find_library(${OUT_VAR} ${LIB_NAME} REQUIRED)
    message(STATUS "Adding library: ${${OUT_VAR}}")
    target_link_libraries(si468x_lib ${${OUT_VAR}})
endfunction()

add_library(gpiod)

install(FILES
    ${CMAKE_CURRENT_LIST_DIR}/firmwares/dab_radio_5_0_5.bin
    ${CMAKE_CURRENT_LIST_DIR}/firmwares/fmhd_radio_5_0_4.bin
    ${CMAKE_CURRENT_LIST_DIR}/firmwares/rom00_patch.016.bin
    DESTINATION /usr/share/dabon-firmwares/)
