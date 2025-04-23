cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w) #especificar que use librerias de la pico W


include(pico_sdk_import.cmake) #importar el sdk

project(Grande) #nombre del proyecto

pico_sdk_init() #cargar el sdk

add_executable(Grande src/main.c src/core1.c src/core0.c src/hw_config.c) #el ejetucable a crear y sus archivos

add_subdirectory(sd/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src build)

pico_enable_stdio_usb(Grande 1) #esto activa el tty
pico_enable_stdio_uart(Grande 0) #esto desactiva el uart

target_link_libraries(Grande
                pico_stdlib
                pico_multicore
                hardware_pwm
                hardware_i2c
                hardware_spi
                hardware_timer
                no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
                ) 

target_include_directories(Grande PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include/)


pico_add_extra_outputs(Grande) #añadir el .uf2 