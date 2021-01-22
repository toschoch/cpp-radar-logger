cd zmqpp
mkdir release
cd release
cmake .. -DZMQPP_BUILD_SHARED=OFF -DZMQPP_BUILD_STATIC=ON
make
make install