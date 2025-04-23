cmake_minimum_required(VERSION 3.13)

#SDK
include(pico_sdk_import.cmake)

project(Blink)                                      #NOMBRE DEL PROYECTO

pico_sdk_init()


add_executable(Blink src/main.c)                    #ARCHIVOS DEL PROYECTO (creo)

target_link_libraries(Blink                         #NOMBRE DEL PROYECTO
                pico_stdlib
                pico_cyw43_arch_none
                )

pico_add_extra_outputs(Blink)


