cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w) #especificar que use librerias de la pico W


include(pico_sdk_import.cmake) #importar el sdk

project(TCPclient) #nombre del proyecto

pico_sdk_init() #cargar el sdk


include_directories(includes) #directorio extra que añadir al compilador

add_executable(TCPclient src/main.c) #el ejetucable a crear y sus archivos

target_link_libraries(TCPclient
                pico_stdlib #esta siempre tiene que estar
                pico_cyw43_arch_lwip_threadsafe_background #esta depende de si el wifi no se usa: pico_cyw43_arch_none
                hardware_pwm
                ) 

pico_enable_stdio_usb(TCPclient 1) #esto activa el tty
pico_enable_stdio_uart(TCPclient 0) #esto desactiva el uart

pico_add_extra_outputs(TCPclient) #añadir el .uf2 

include(../../secretos.cmake)
setup_wifi(Coche)