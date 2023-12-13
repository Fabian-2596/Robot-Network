Build docker image with "docker build -t server ."
Run the image with "docker run -p 8080:8080 server"

Server is accessible over localhost:8080
Status is seen over localhost:8080/status
Captain is seen over localhost:8080/captain
Controller health is seen over localhost:8080/controller
To start election for a new captain localhost:8080/election
