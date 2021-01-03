cmake_minimum_required(VERSION 3.7.2)
project(RadarReader)

add_subdirectory(RadarApi)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED COMPONENTS core hdf)

add_executable(${PROJECT_NAME}  main.cpp Logger.cpp Logger.h utils.cpp utils.h callbacks.cpp callbacks.h)
target_link_libraries(${PROJECT_NAME} PUBLIC radarapi)
target_link_libraries(${PROJECT_NAME} PUBLIC
        opencv_core opencv_hdf)