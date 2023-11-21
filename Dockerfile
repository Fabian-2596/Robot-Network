FROM ubuntu:latest
RUN apt-get update && apt-get install -y g++ && rm -rf /var/lib/apt/lists/*
COPY /app /app
WORKDIR /app
RUN g++ -o http_server Server.cpp -std=c++11 -pthread
EXPOSE 8080
CMD ["./http_server"]