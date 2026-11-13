cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(Lab5)

pico_sdk_init()

add_executable(Lab5    
    main.c
    ts_lcd.c
    TouchScreen.c
    TFTMaster.c
)

pico_generate_pio_header(Lab5 ${CMAKE_CURRENT_LIST_DIR}/SPIPIO.pio)

target_link_libraries(Lab5  
    pico_stdlib
    hardware_gpio 
    hardware_pio 
    hardware_irq
    hardware_adc
    hardware_i2c
    pico_bootsel_via_double_reset
)

pico_enable_stdio_uart(Lab5 1)
pico_add_extra_outputs(Lab5)