FROM balenalib/raspberrypi3:build AS builder

RUN install_packages build-essential libopenblas-base cmake libopencv-dev

WORKDIR ./src

COPY RadarApi ./RadarApi
COPY *.cpp ./
COPY *.h ./
COPY CMakeLists.txt ./

WORKDIR ../build

RUN cmake ../src && make

FROM balenalib/raspberrypi3:run
RUN install_packages libopenblas-base libopencv-core3.2 libopencv-contrib3.2
COPY --from=builder /build/RadarReader ./

CMD ./RadarReader


