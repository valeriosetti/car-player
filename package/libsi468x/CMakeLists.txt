cmake_minimum_required(VERSION 3.18)

project(si468x)

add_library(si468x STATIC
    si468x_common.c
    si468x_dab.c
    si468x_ext.c
    si468x_fm.c
    si468x_platform.c
    si468x_utils.c
)

target_include_directories(si468x PUBLIC .)

find_and_link_library(gpiod si468x)

install(FILES
    firmwares/dab_radio_5_0_5.bin
    firmwares/fmhd_radio_5_0_4.bin
    firmwares/rom00_patch.016.bin
    DESTINATION /usr/share/dabon-firmwares/
)

