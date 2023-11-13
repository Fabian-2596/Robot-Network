#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sstream>
#include <string>
#include <cstring>

using namespace std;
const int PORT = 8080;

void handle_request(int client_socket) {
    char buffer[1024] = {0};
    read(client_socket, buffer, sizeof(buffer));
    istringstream request(buffer);
    string request_type, path, http_version;
    request >> request_type >> path >> http_version;

    if (request_type != "GET" && request_type != "POST") {
        string response = "HTTP/1.1 400 Bad Request\r\n\r\nUnsupported Request";
        write(client_socket, response.c_str(), response.length());
        close(client_socket);
        return;
    }

    if (request_type == "GET") {
		string response;
        if(path == "/status"){
			response = "HTTP/1.1 200 OK\r\n\r\nSystem Status: Alle Roboter sind aktiv";
		}
		else if(path == "/captain"){
			response = "HTTP/1.1 200 OK\r\n\r\nDer aktuelle Kapitän ist T-800";
			}
		else if(path == "/controller"){
			response = "HTTP/1.1 200 OK\r\n\r\nController Status: Der Zustand des Controllers ist Gesund";
			}
		else if(path == "/election"){
			response = "HTTP/1.1 200 OK\r\n\r\nNeue Kapitänswahl wird angestoßen";
		}	
		else{
			response = "HTTP/1.1 404 Not Found\r\n\r\nNicht gefunden";
			}
			
        write(client_socket, response.c_str(), response.length());
    }

    if (request_type == "POST") {
        string response = "HTTP/1.1 200 OK\r\n\r\nData received and stored successfully";
        write(client_socket, response.c_str(), response.length());
    }

    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        cerr << "Socket creation failed." << endl;
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    listen(server_socket, 5);

    cout << "Server listening on port " << PORT << "..." << endl;

    while (true) {
        socklen_t client_address_len = sizeof(client_address);

        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);

        if (client_socket < 0) {
            cerr << "Error in connection acceptance." << endl;
            return -1;
        }
        handle_request(client_socket);
    }

    close(server_socket);

    return 0;
}
