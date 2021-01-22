cmake_minimum_required(VERSION 3.7.2)
project(RadarReader)

add_subdirectory(RadarApi)

set(CMAKE_CXX_STANDARD 14)

#find_package(OpenCV REQUIRED COMPONENTS core hdf)

find_package(Arrow REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
#find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(PahoMqttCpp REQUIRED)

FILE(GLOB CppSources src/*.cpp)

add_executable(${PROJECT_NAME} ${CppSources} include/shared_queue.h include/radar_enums.h)
target_link_libraries(${PROJECT_NAME} PRIVATE radarapi zmq zmqpp-static arrow_static Threads::Threads paho-mqttpp3 paho-mqtt3a)
#target_link_libraries(${PROJECT_NAME} PUBLIC
        #opencv_core opencv_hdf)