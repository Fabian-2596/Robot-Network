FROM ubuntu:latest
RUN apt-get update && apt-get install -y g++ && rm -rf /var/lib/apt/lists/*
COPY Server.cpp /app/server.cpp
WORKDIR /app
RUN g++ -o http_server server.cpp
EXPOSE 8080
CMD ["./http_server"]
