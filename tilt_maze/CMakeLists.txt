cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(TiltMaze)

pico_sdk_init()

add_executable(
    TiltMaze
    main.c 
)

target_link_libraries(
    TiltMaze
    pico_stdlib
    hardware_i2c
)

pico_enable_stdio_usb(TiltMaze 1)
pico_enable_stdio_uart(TiltMaze 0)

pico_add_extra_outputs(TiltMaze)