FROM balenalib/raspberrypi3:build AS builder

RUN apt-get update && DEBIAN_FRONTEND="noninteractive" TZ="Europe/Zurich" apt-get install -y \
                      build-essential cmake git wget curl gpg \
                      ca-certificates lsb-release
                      # libopenblas-base libopencv-dev
RUN git clone https://github.com/apache/arrow.git
COPY install_arrow.sh ./
RUN chmod +x install_arrow.sh && ./install_arrow.sh

RUN git clone https://github.com/eclipse/paho.mqtt.c.git && git clone https://github.com/eclipse/paho.mqtt.cpp
COPY install_mqtt.sh ./
RUN chmod +x install_mqtt.sh && ./install_mqtt.sh

COPY apt.preferences /etc/apt/preferences
COPY apt.sources /etc/apt/sources.list
RUN apt-get update && apt-get install -y libzmq3-dev && \
 apt-get install -y -t testing nlohmann-json3-dev && apt-cache policy nlohmann-json3-dev

# Now install ZMQPP
RUN git clone git://github.com/zeromq/zmqpp.git
COPY install_zmqpp.sh ./
RUN chmod +x install_zmqpp.sh && ./install_zmqpp.sh

RUN cat /etc/os-release && apt-cache policy nlohmann-json3-dev

WORKDIR ./src

COPY RadarApi ./RadarApi
COPY src ./src
COPY include ./include
COPY CMakeLists.txt ./

WORKDIR ../build

RUN cmake .. /src && make

FROM balenalib/raspberrypi3:run
RUN apt-get update && apt-get install -y libzmq5 && rm -rf /var/lib/apt/lists/* && mkdir /radar/

ENV UDEV=1

#RUN install_packages libopenblas-base libopencv-core3.2 libopencv-contrib3.2
COPY --from=builder /build/RadarReader .

CMD ./RadarReader


