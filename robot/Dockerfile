
FROM gcc:latest
WORKDIR /
COPY . . 
RUN apt-get update && \
    apt-get install -y \
    libboost-system-dev \
    libboost-thread-dev \
    libevent-dev \
    cmake \
    build-essential \
    libthrift-dev

RUN gcc -o robot1 robot.cpp -pthread
CMD ["./robot.cpp"]
EXPOSE 9090
