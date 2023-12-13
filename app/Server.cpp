#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <chrono>
#include <thread>
#include <grpcpp/grpcpp.h>
#include "server.grpc.pb.h"
#include "DB/DB.cpp"

using namespace std;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

const int PORT = 8080;
int robotid = 0;
vector<Player> team;
DB myDB{};


void handle_request(int client_socket) {
    //this_thread::sleep_for(chrono::seconds(10));
    char buffer[1024] = {0};
    read(client_socket, buffer, sizeof(buffer));
    auto start_time = chrono::high_resolution_clock::now();

    istringstream request(buffer);
    string request_type, path, http_version;
    request >> request_type >> path >> http_version;

    if (request_type != "GET" && request_type != "POST") {
        std::string response = "HTTP/1.1 400 Bad Request\r\n\r\nError 400\n";
        write(client_socket, response.c_str(), response.length());
        close(client_socket);
        return;
    }

    if (request_type == "GET") {
		std::string response;
        if(path == "/status"){
            //int cnt_pl = myDB.getCountSpieler();
            int cnt_pl = team.size();
            switch (cnt_pl)
            {
            case 0:
                response = "HTTP/1.1 200 OK\r\n\r\nSystem Status: Im Moment ist KEIN Roboter aktiv\n";
                break;
            case 1:
                response = "HTTP/1.1 200 OK\r\n\r\nSystem Status: Im Moment ist nur 1 Roboter aktiv\n";
                break;
            default:
                response = "HTTP/1.1 200 OK\r\n\r\nSystem Status: Im Moment sind " + to_string(cnt_pl) + " Roboter aktiv\n";
                break;
            }
		}
		else if(path == "/captain"){
			response = "HTTP/1.1 200 OK\r\n\r\nDer aktuelle Kapitän ist " + myDB.getCaptain().player.name + "\n";
			}
		else if(path == "/controller"){
			response = "HTTP/1.1 200 OK\r\n\r\nController Status: Der Zustand des Controllers ist \"" + myDB.getConStatus() + "\"\n";
			}
		else if(path == "/election"){
			response = "HTTP/1.1 200 OK\r\n\r\nNeue Kapitänswahl wird angestoßen (Noch keine Funktion)\n";
		}
        else if(path == "/lastPOST"){
            response = "HTTP/1.1 200 OK\r\n\r\n";
            if(myDB.getPOSTData().empty()){
                response = response + "noInputYet\n";
            }else
            {
                response = response + myDB.getPOSTData().back() + "\n";
            }
        }
		else{
			response = "HTTP/1.1 404 Not Found\r\n\r\nHier gibt es nichts zu finden\n";
			}
        write(client_socket, response.c_str(), response.length());
        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> rtt = end_time - start_time;
        cout << "RTT Get: " << rtt.count()*1000 << " mseconds" << endl;
    }
  
    if (request_type == "POST") {
        std::string data;
        while (getline(request, data)) {
            if (data.substr(0, 14) == "Content-Length") {
                    getline(request,data);
            }
        }
        std::string response = "HTTP/1.1 200 OK\r\n\r\nDaten erhalten : " + data + "\n";
        myDB.addPOSTData(data);
        //myDB.persistPOST();
        write(client_socket, response.c_str(), response.length());
        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> rtt = end_time - start_time;
        cout << "RTT Post: " << rtt.count()*1000 << " mseconds" << endl;
        cout << data << endl;
    }
    close(client_socket);

}

class RobotControlServiceImpl final : public RobotControl::Service {
public:
    grpc::Status RegisterRobot(grpc::ServerContext* context, const RobotRegistration* request, RobotStatus* response) override {
        // Implementieren Sie hier die Anmeldelogik für den Roboter
        // Setzen Sie response entsprechend, um den Status des Roboters zu aktualisieren
        response->set_is_active(true);
        Player p;
        p.id = request->robot_id();
        p.name = request->robot_name();
        p.isActive = response->is_active();
        team.push_back(p);
        return grpc::Status::OK;
    }

    grpc::Status GetRobotStatus(grpc::ServerContext* context, const RobotRegistration* request, RobotStatus* response) override {
        // Implementieren Sie hier die Logik, um den Status des Roboters abzurufen
        // Setzen Sie response entsprechend, um den aktuellen Status zurückzugeben
        response->set_is_active(true);
        return grpc::Status::OK;
    }

};

void httpServer() {
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
        cout << "HTTP Server waiting for connections" << endl;
        myDB.setConStatus("waiting");
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
        if (client_socket < 0) {
            cerr << "Error in HTTP connection acceptance." << endl;
            return;
        }
        myDB.setConStatus("processing");
        thread new_thread(handle_request, client_socket);
        cout << "HTTP Thread " << new_thread.get_id() << " created" << endl;
        myDB.setConStatus("task finished");
    }

    close(server_socket);
    myDB.setConStatus("exited");
}

void grpcServer() {
    RobotControlServiceImpl service;
    ServerBuilder builder;
    builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> grpc_server(builder.BuildAndStart());
    cout << "gRPC server listening on port 50051" << endl;
    grpc_server->Wait();
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
    thread grpc_thread(grpcServer);
    while (true) {
        socklen_t client_address_len = sizeof(client_address);
        cout << "HTTP Server waiting for connections" << endl;
        myDB.setConStatus("waiting");
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
        if (client_socket < 0) {
            cerr << "Error in HTTP connection acceptance." << endl;
            return -1;
        }
        myDB.setConStatus("processing");
        thread new_thread(handle_request, client_socket);
        new_thread.detach();
        cout << "HTTP Thread " << new_thread.get_id() << " created" << endl;
        myDB.setConStatus("task finished");
    }
    
    close(server_socket);
    myDB.setConStatus("exited");
    
    return 0;
}
