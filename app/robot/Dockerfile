FROM ubuntu:latest

RUN apt-get update && \
    apt-get install -y build-essential cmake

RUN apt-get install -y libgrpc++-dev protobuf-compiler-grpc
WORKDIR /
COPY . . 
RUN protoc -I=. --cpp_out=. --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` server.proto
RUN g++ -std=c++11 -o robot robot.cpp server.grpc.pb.cc server.pb.cc -lgrpc++ -lprotobuf -pthread
CMD ["./robot"]

