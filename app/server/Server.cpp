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
#include <map>
#include <grpcpp/grpcpp.h>
#include "server.grpc.pb.h"
#include "DB/DB.cpp"
#include <mqtt/async_client.h>

using namespace std;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

const int PORT = 8080;
int robotid = 0;
std::map<int, time_t> robots;
DB myDB{};
const time_t limit = 10;
const std::string ELECTION_TOPIC = "election";
const std::string SERVER_ADDRESS{"tcp://mosquitto:1883"};

class MyCallback : public virtual mqtt::callback {
public:
    virtual void connection_lost(const std::string& cause) override {
        std::cout << "Connection lost: " << cause << std::endl;
    }

    virtual void delivery_complete(mqtt::delivery_token_ptr tok) override {
        std::cout << "Delivery complete for token: " << tok->get_message_id() << std::endl;
    }

    void message_arrived(mqtt::const_message_ptr msg) override {
        std::string payload = msg->to_string();
        for(int i = 0; i < myDB.getTeamSize(); i++){
            if(myDB.getTeam().playerList.at(i).id == stoi(payload)){
                Captain cp;
                Player pl;
                pl = myDB.getTeam().playerList.at(i);
                cp.player = pl;
                myDB.setCaptain(cp);
                cout << "new captain is " << myDB.getTeam().playerList.at(i).name << endl;
            }
        }
        
    }
};

void startElection(){
    mqtt::create_options createOpts(MQTTVERSION_5);
    mqtt::async_client client(SERVER_ADDRESS, "server", createOpts);
    MyCallback callback;
    client.set_callback(callback);
    try {
        cout << "\nConnecting..." << endl;
		mqtt::token_ptr tok = client.connect();
		cout << "Waiting for the connection..." << endl;
		tok->wait();
		cout << "  ...OK" << endl;

        std::cout << "Initiating the election process..." << std::endl;
        int qos = 0;
        mqtt::message_ptr pubmsg = mqtt::make_message(ELECTION_TOPIC, "start1");
        pubmsg->set_qos(qos);
        mqtt::delivery_token_ptr pubtok = client.publish(pubmsg);
        pubtok->wait();
        cout << "data published" << std::endl;
        client.subscribe("leader", 1)->wait();
        std::this_thread::sleep_for(std::chrono::seconds(5));

        client.disconnect()->wait();
        std::cout << "Disconnected." << std::endl;
    } catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
    }
}

void handle_request(int client_socket)
{
    char buffer[1024] = {0};
    read(client_socket, buffer, sizeof(buffer));
    auto start_time = chrono::high_resolution_clock::now();

    istringstream request(buffer);
    string request_type, path, http_version;
    request >> request_type >> path >> http_version;

    if (request_type != "GET" && request_type != "POST")
    {
        std::string response = "HTTP/1.1 400 Bad Request\r\n\r\nError 400\n";
        write(client_socket, response.c_str(), response.length());
        close(client_socket);
        return;
    }

    if (request_type == "GET")
    {
        std::string response;
        std::string players = "";
        if (path == "/status")
        {
            for(int i = 0; i < myDB.getTeamSize(); i++){
                if(myDB.getTeam().playerList.at(i).isActive == true){
                    players += to_string(myDB.getTeam().playerList.at(i).id) + " : " + myDB.getTeam().playerList.at(i).name + "\n";
                }
            }
            response = "HTTP/1.1 200 OK\r\n\r\nSystem Status: Es sind folgende Roboter aktiv\n" + players + "\n";
        }
        else if (path == "/captain")
        {
            response = "HTTP/1.1 200 OK\r\n\r\nDer aktuelle Kapitän ist " + myDB.getCaptain().player.name + "\n";
        }
        else if (path == "/controller")
        {
            response = "HTTP/1.1 200 OK\r\n\r\nController Status: Der Zustand des Controllers ist \"" + myDB.getConStatus() + "\"\n";
        }
        else if (path == "/election")
        {
            startElection();
            response = "HTTP/1.1 200 OK\r\n\r\nNeue Kapitänswahl wurde angestoßen\n";
        }
        else if (path == "/lastPOST")
        {
            response = "HTTP/1.1 200 OK\r\n\r\n";
            if (myDB.getPOSTData().empty())
            {
                response = response + "noInputYet\n";
            }
            else
            {
                response = response + myDB.getPOSTData().back() + "\n";
            }
        }
        else
        {
            response = "HTTP/1.1 404 Not Found\r\n\r\nHier gibt es nichts zu finden\n";
        }
        write(client_socket, response.c_str(), response.length());
        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> rtt = end_time - start_time;
        cout << "RTT Get: " << rtt.count() * 1000 << " mseconds" << endl;
    }

    if (request_type == "POST")
    {
        std::string data;
        while (getline(request, data))
        {
            if (data.substr(0, 14) == "Content-Length")
            {
                getline(request, data);
            }
        }
        std::string response = "HTTP/1.1 200 OK\r\n\r\nDaten erhalten : " + data + "\n";
        myDB.addPOSTData(data);
        //myDB.persistPOST();
        write(client_socket, response.c_str(), response.length());
        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> rtt = end_time - start_time;
        cout << "RTT Post: " << rtt.count() * 1000 << " mseconds" << endl;
        cout << data << endl;
    }
    close(client_socket);
}

class RobotControlServiceImpl final : public RobotControl::Service
{
public:
    grpc::Status RegisterRobot(grpc::ServerContext *context, const RobotRegistration *request, RobotStatus *response) override
    {
        response->set_is_active(true);
        Player p;
        p.id = request->robot_id();
        p.name = request->robot_name();
        p.isActive = response->is_active();
        for(int i = 0; i < myDB.getTeamSize(); i++){
            if (myDB.getTeam().playerList.at(i).id == p.id && myDB.getTeam().playerList.at(i).name == p.name) {
                myDB.setPlayerStatus(myDB.getTeam().playerList.at(i).id, true);
                return grpc::Status::OK;
            }
        }
        myDB.addPlayer(p);
        return grpc::Status::OK;
    }

    grpc::Status GetRobotStatus(grpc::ServerContext *context, const RobotRegistration *request, RobotStatus *response) override
    {
        for (int i{}; i < myDB.getTeamSize(); ++i){
            if (myDB.getTeam().playerList.at(i).id == request->robot_id()){
                response->set_is_active(myDB.getTeam().playerList.at(i).isActive);
                cout << "Status of robot " << myDB.getTeam().playerList.at(i).name << " is " <<myDB.getTeam().playerList.at(i).isActive << endl;
                return grpc::Status::OK;
            }
        }
        return grpc::Status::CANCELLED;
    }

    grpc::Status SetRobotStatus(grpc::ServerContext *context, const RobotSetStatus *request, RobotStatus *response) override
    {
        if (myDB.setPlayerStatus(request->robot_id(), request->is_active())){
            return grpc::Status::OK;
        }
        return grpc::Status::OK;
    }

    grpc::Status SendHeartbeat(grpc::ServerContext* context, const HeartbeatRequest* request, HeartbeatResponse* response) override {
        int id = request->robot_id();
        response->set_success(true);
        time_t lastBeat = time(NULL);
        if(robots.find(id) != robots.end()){
            robots.erase(id);
        }
        robots.insert({id, lastBeat});
        std::cout << "Received heartbeat from client: " << id << std::endl;
        return grpc::Status::OK;
    }
};

void CheckPlayerState(){
    while (true)
    {
        vector<string> unactivePlayer{};
        this_thread::sleep_for(chrono::seconds(5));

        for (auto const& x : robots){
            time_t t = time(nullptr);
            if((t - x.second) > limit){
                for(int i = 0; i < myDB.getTeamSize(); i++){
                    if(myDB.getTeam().playerList.at(i).id == x.first){
                        myDB.setPlayerStatus(myDB.getTeam().playerList.at(i).id, false);
                    }
                }
            }
        }

        for (int i = 0; i < myDB.getTeamSize(); i++)
        {
            if (myDB.getTeam().playerList.at(i).isActive == false){
                unactivePlayer.push_back(myDB.getTeam().playerList.at(i).name);
            }
        }
        if (unactivePlayer.size() != 0){
            startElection();
            cout << "Status: Player ";
            for (int i{}; i < unactivePlayer.size(); i++){
                cout << unactivePlayer[i];
                if (i < (unactivePlayer.size()-1)){
                    cout << " and ";
                }
            }
            cout << " are inactiv" << endl;
        }
        else{
            cout << "Status: All Players are active" << endl;
        }
    }
}

void grpcServer()
{
    RobotControlServiceImpl service;
    ServerBuilder builder;
    builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<Server> grpc_server(builder.BuildAndStart());
    cout << "gRPC server listening on port 50051" << endl;
    grpc_server->Wait();
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);
    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    listen(server_socket, 5);
    thread grpc_thread(grpcServer);
    thread thr_checkStatus(CheckPlayerState);

    while (true)
    {
        socklen_t client_address_len = sizeof(client_address);
        cout << "HTTP Server waiting for connections" << endl;
        myDB.setConStatus("waiting");
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket < 0)
        {
            cerr << "Error in HTTP connection acceptance." << endl;
            return -1;
        }
        myDB.setConStatus("processing");
        thread http_thread(handle_request, client_socket);
        cout << "HTTP Thread " << http_thread.get_id() << " created" << endl;
        http_thread.detach();
        myDB.setConStatus("task finished");
    }
    thr_checkStatus.join();
    close(server_socket);
    myDB.setConStatus("exited");

    return 0;
}
