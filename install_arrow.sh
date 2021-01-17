cd arrow/cpp
mkdir release
cd release
cmake .. -DARROW_IPC=ON -DARROW_DEPENDENCY_USE_SHARED=OFF -DARROW_BUILD_STATIC=ON -DARROW_BUILD_SHARED=OFF
make
make install