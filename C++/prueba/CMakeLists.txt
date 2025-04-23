cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 23)
set(PICO_BOARD pico_w)
set(PICO_SDK_PATH "/opt/pico-sdk-2" CACHE STRING "Path to the Pico SDK")
set(picotool_DIR "/opt/picotool")

include(pico_sdk_import.cmake)

project(Blink)

pico_sdk_init()

add_executable(Blink main.cpp)

target_link_libraries(Blink
        pico_stdlib
        pico_cyw43_arch_none
)

pico_add_extra_outputs(Blink)
