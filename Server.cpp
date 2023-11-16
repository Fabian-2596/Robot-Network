#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sstream>
#include <string>
#include <cstring>
#include <chrono>

using namespace std;
const int PORT = 8080;

void handle_request(int client_socket) {
    char buffer[1024] = {0};
    read(client_socket, buffer, sizeof(buffer));
    auto start_time = chrono::high_resolution_clock::now();
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
        if (path == "/start"){
            response = "HTTP/1.1 200 OK\r\n\r\n  <input type=\"submit\" value=\"GET\" name=\"GET\" formmethod=\"get\" formtarget=\"_self\" />\n"
                       "  <input type=\"submit\" value=\"POST\" name=\"POST\" formmethod=\"post\" formtarget=\"_self\" />";
        }
        else if(path == "/status"){
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
        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> rtt = end_time - start_time;
        cout << "RTT: " << rtt.count()*1000 << " milliseconds" << endl;
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
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    listen(server_socket, 5);

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
