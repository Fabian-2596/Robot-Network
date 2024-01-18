FROM ubuntu:latest

RUN ldconfig

RUN apt-get update && \
    apt-get install -y build-essential cmake libssl-dev git

RUN apt-get install -y libgrpc++-dev protobuf-compiler-grpc

RUN git clone https://github.com/eclipse/paho.mqtt.c.git

RUN cd paho.mqtt.c && \
    cmake -Bbuild -H. -DPAHO_WITH_SSL=ON && \
    cmake --build build/ --target install

RUN git clone https://github.com/eclipse/paho.mqtt.cpp.git && \ 
    cd paho.mqtt.cpp && \
    git checkout v1.3.1

RUN cd paho.mqtt.cpp && \
    cmake . && \
    make && \
    make install

RUN ldconfig

COPY . . 
RUN protoc -I=. --cpp_out=. --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` server.proto
RUN g++ -std=c++11 -o http_server Server.cpp server.grpc.pb.cc server.pb.cc -lpaho-mqttpp3 -lpaho-mqtt3a -lgrpc++ -lprotobuf -pthread
EXPOSE 8080
EXPOSE 50051
CMD ["./http_server"]

