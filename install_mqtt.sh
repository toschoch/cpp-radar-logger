cd paho.mqtt.c
mkdir release
cd release
cmake .. -DPAHO_ENABLE_TESTING=OFF -DPAHO_BUILD_STATIC=ON -DPAHO_BUILD_SHARED=OFF -DPAHO_HIGH_PERFORMANCE=ON
make
make install
cd ../..

cd paho.mqtt.cpp
mkdir release
cd release
cmake .. -DPAHO_BUILD_STATIC=ON -DPAHO_BUILD_SHARED=OFF -DPAHO_WITH_SSL=OFF
make
make install