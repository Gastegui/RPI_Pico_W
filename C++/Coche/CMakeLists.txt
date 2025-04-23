cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 23)
set(PICO_BOARD pico_w) #especificar que use librerias de la pico W
set(PICO_SDK_PATH "/opt/pico-sdk-2" CACHE STRING "Path to the Pico SDK")

set(PICOTOOL_FORCE_FETCH_FROM_GIT on)

include(pico_sdk_import.cmake) #importar el sdk

project(Coche)


pico_sdk_init() #cargar el sdk

include_directories(includes) #directorio extra que añadir al compilador

add_executable(Coche main.cpp
        core1.cpp
        classes/motor.h)

pico_enable_stdio_usb(Coche 1) #esto activa el tty
pico_enable_stdio_uart(Coche 0) #esto desactiva el uart

target_link_libraries(Coche
        pico_stdlib
        pico_multicore
        hardware_pwm
        hardware_i2c
        hardware_spi
        hardware_timer
        pico_cyw43_arch_lwip_threadsafe_background
)

pico_add_extra_outputs(Coche) #añadir el .uf2

include(../../secretos.cmake)
setup_wifi(Coche)