cmake_minimum_required(VERSION 3.7.2)
project(radarapi)

FILE(GLOB CSources src/*.c)
add_library(${PROJECT_NAME} ${CSources})
target_include_directories(${PROJECT_NAME} PUBLIC include)