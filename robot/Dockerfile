FROM ubuntu:latest

WORKDIR /
COPY . . 
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    libboost-system-dev \
    libboost-thread-dev \
    libevent-dev \
    libthrift-dev

RUN gcc -o robot1 robot.cpp -lthriftnb -levent -std=c++11 -pthread 
CMD ["./robot.cpp"]
EXPOSE 9090
