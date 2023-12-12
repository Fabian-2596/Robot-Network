FROM ubuntu:latest

RUN apt-get update && \
    apt-get install -y automake bison flex g++ libboost-all-dev libevent-dev libssl-dev libtool make pkg-config wget

RUN wget https://dlcdn.apache.org/thrift/0.19.0/thrift-0.19.0.tar.gz
RUN tar -xvf thrift-0.19.0.tar.gz
WORKDIR /thrift-0.19.0
RUN ./bootstrap.sh
RUN ./configure
RUN make
RUN make install
WORKDIR /
COPY . . 
RUN
RUN g++ -o http_server Server.cpp -levent -lthriftnb -std=c++11 -pthread
EXPOSE 8080
EXPOSE 9090
CMD ["./http_server"]

