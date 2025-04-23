cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 23)
set(PICO_BOARD pico_w)
set(PICO_SDK_PATH "/opt/pico-sdk-2" CACHE STRING "Path to the Pico SDK")

include(pico_sdk_import.cmake)

project(Grande)

pico_sdk_init()

add_executable(Grande src/main.cpp
        headers/Led.h
        headers/Boton.h
        headers/ICallback.h
        headers/Oled.h
        headers/LETRAS.h
        headers/Encoder.h
        headers/CONSTANTES.h
        headers/Buzzer.h
        src/core_1.cpp
        headers/core1.h)
#        headers/SD.h
#        src/hw_config.c)

#add_subdirectory(sd/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src build)

pico_enable_stdio_usb(Grande 1) #esto activa el tty
pico_enable_stdio_uart(Grande 0) #esto desactiva el uart

target_link_libraries(Grande
        pico_stdlib
        pico_multicore
        hardware_pwm
        hardware_i2c
        hardware_spi
        hardware_timer
#        no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
        pico_cyw43_arch_none
)
pico_add_extra_outputs(Grande)
