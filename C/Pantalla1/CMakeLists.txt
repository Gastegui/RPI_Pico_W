cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w)

include(pico_sdk_import.cmake)

project(Pantalla1)

pico_sdk_init()

add_executable(Pantalla1 src/OledMio.c)

target_link_libraries(Pantalla1 pico_stdlib hardware_i2c)

pico_enable_stdio_uart(Pantalla1 1)
pico_enable_stdio_usb(Pantalla1 0)

pico_add_extra_outputs(Pantalla1)
