FROM ubuntu:latest

# Installieren Sie grundlegende Build-Tools und Abhängigkeiten
RUN apt-get update && \
    apt-get install -y build-essential cmake

# Installieren Sie gRPC-Abhängigkeiten
RUN apt-get install -y libgrpc++-dev protobuf-compiler-grpc
WORKDIR /
COPY . . 
RUN protoc -I=. --cpp_out=. --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` server.proto
RUN g++ -std=c++11 -o http_server Server.cpp server.grpc.pb.cc server.pb.cc -lgrpc++ -lprotobuf -pthread
EXPOSE 8080
EXPOSE 50051
CMD ["./http_server"]

