FROM balenalib/raspberrypi3:build AS builder

RUN apt-get update && apt-get install build-essential cmake # libopenblas-base libopencv-dev

WORKDIR ./src

COPY RadarApi ./RadarApi
COPY *.cpp ./
COPY *.h ./
COPY CMakeLists.txt ./

WORKDIR ../build

RUN cmake ../src && make

FROM balenalib/raspberrypi3:run
#RUN install_packages libopenblas-base libopencv-core3.2 libopencv-contrib3.2
COPY --from=builder /build/RadarReader .

COPY start.sh .
RUN chmod +x start.sh

CMD ./start.sh && ./RadarReader


