cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w) #especificar que use librerias de la pico W


include(pico_sdk_import.cmake) #importar el sdk

project(PWM) #nombre del proyecto

pico_sdk_init() #cargar el sdk


add_executable(PWM src/Musica.c) #el ejetucable a crear y sus archivos

target_link_libraries(PWM
                pico_stdlib #esta siempre tiene que estar
                hardware_pwm
                pico_multicore
                ) 

pico_enable_stdio_usb(PWM 1) #esto activa el tty
pico_enable_stdio_uart(PWM 0) #esto desactiva el uart

pico_add_extra_outputs(PWM) #añadir el .uf2 