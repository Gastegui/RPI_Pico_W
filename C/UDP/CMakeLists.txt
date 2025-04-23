cmake_minimum_required(VERSION 3.13)

#SDK
include(pico_sdk_import.cmake)

project(UDP)                                       #NOMBRE DEL PROYECTO

pico_sdk_init()

include_directories(src)

add_executable(UDP src/main.c)                     #ARCHIVOS DEL PROYECTO (creo)

target_link_libraries(UDP                          #NOMBRE DEL PROYECTO
                pico_stdlib                        #esta siempre tiene que estar
                pico_cyw43_arch_lwip_threadsafe_background #esta depende de si el wifi no se usa: pico_cyw43_arch_none
                ) 

pico_enable_stdio_usb(UDP 1)                        #esto activa el tty
pico_enable_stdio_uart(UDP 0)                       #esto desactiva el uart

pico_add_extra_outputs(UDP)

include(../../secretos.cmake)
setup_wifi(Coche)

