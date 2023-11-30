FROM ubuntu:latest
RUN apt-get update && \
    apt-get install -y build-essential cmake libboost-system-dev libboost-thread-dev
RUN apt-get update && \
    apt-get install -y build-essential cmake libevent-dev
RUN apt-get update && \
    apt-get install -y build-essential cmake libthrift-dev
RUN apt-get update && apt-get install -y g++ && rm -rf /var/lib/apt/lists/*
COPY /app /app
WORKDIR /app
RUN g++ -o http_server Server.cpp -pthread
EXPOSE 8080
EXPOSE 9090
CMD ["./http_server"]

