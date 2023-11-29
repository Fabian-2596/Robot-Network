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
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include "DB/DB.cpp"
#include "gen-cpp/RobotController.h"

using namespace std;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

const int PORT = 8080;
int currentId = 0;
vector<Player> team;
DB myDB{};

class RobotControllerHandler : virtual public RobotControllerIf {
 public:
  RobotControllerHandler() {
    // Your initialization goes here
  }

  void registerRobot(const string& name) {
    // Your implementation goes here
    printf("registerRobot\n");
  }

  bool checkRobotHealth(const int32_t id) {
    // Your implementation goes here
    printf("checkRobotHealth\n");
    return true;
  }

};


void handle_request(int client_socket) {
    //this_thread::sleep_for(chrono::seconds(10));
    char buffer[1024] = {0};
    read(client_socket, buffer, sizeof(buffer));
    auto start_time = chrono::high_resolution_clock::now();

    istringstream request(buffer);
    string request_type, path, http_version;
    request >> request_type >> path >> http_version;

    if (request_type != "GET" && request_type != "POST") {
        string response = "HTTP/1.1 400 Bad Request\r\n\r\nError 400\n";
        write(client_socket, response.c_str(), response.length());
        close(client_socket);
        return;
    }

    if (request_type == "GET") {
		string response;
        if(path == "/status"){
            int cnt_pl = myDB.getCountSpieler();
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
        string data;
        while (getline(request, data)) {
            if (data.substr(0, 14) == "Content-Length") {
                    getline(request,data);
            }
        }
        string response = "HTTP/1.1 200 OK\r\n\r\nDaten erhalten : " + data + "\n";
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

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    listen(server_socket, 5);

    int port = 9090;
    ::std::shared_ptr<RobotControllerHandler> handler(new RobotControllerHandler());
    ::std::shared_ptr<TProcessor> processor(new RobotControllerProcessor(handler));
    ::std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    ::std::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    ::std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
    server.serve();

    while (true) {
        socklen_t client_address_len = sizeof(client_address);

        cout << "Controller waiting for connections" << endl;
        myDB.setConStatus("waiting");
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);

        if (client_socket < 0) {
            cerr << "Error in connection acceptance." << endl;
            return -1;
        }
        myDB.setConStatus("processind");

        thread new_thread(handle_request, client_socket);
        cout << "Thread " << new_thread.get_id() << " erstellt" << endl; 
        new_thread.detach();
        
        myDB.setConStatus("task finished");
    }
    close(server_socket);
    myDB.setConStatus("exited");

    return 0;
}
