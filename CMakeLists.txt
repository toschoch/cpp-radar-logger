cmake_minimum_required(VERSION 3.7.2)
project(RadarReader)

add_subdirectory(RadarApi)

set(CMAKE_CXX_STANDARD 14)

#find_package(OpenCV REQUIRED COMPONENTS core hdf)


FILE(GLOB CppSources src/*.cpp)

add_executable(${PROJECT_NAME} ${CppSources})
target_link_libraries(${PROJECT_NAME} PUBLIC radarapi -lzmqpp -lzmq)
#target_link_libraries(${PROJECT_NAME} PUBLIC
        #opencv_core opencv_hdf)