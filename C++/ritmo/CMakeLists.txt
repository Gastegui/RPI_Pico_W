cmake_minimum_required(VERSION 3.29)
project(ritmo)

set(CMAKE_CXX_STANDARD 23)

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_image REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

add_executable(ritmo main.cpp)

target_link_libraries(ritmo ${SDL2_LIBRARIES} SDL2_ttf SDL2_mixer SDL2_image)