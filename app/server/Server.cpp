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

using namespace std;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

int httpPort = 8080;
int robotid = 0;
std::map<int, time_t> robots;
DB myDB{};
const time_t limit = 10;
bool DBisSYNCED = false;
bool isBackup = false;
bool controllerInUse = true;
bool threadSTOP = false;
bool start = false;
string role = "";

class RobotControlServiceImpl final : public RobotControl::Service
{
public:
    grpc::Status RegisterRobot(grpc::ServerContext *context, const RobotRegistration *request, RobotStatus *response) override
    {
        response->set_is_active(true);
        Player p;
        bool idExists = false;
        for (const Player &pl : myDB.getTeam().playerList)
        {
            if (pl.id == request->robot_id())
            {
                idExists = true;
            }
        }

        if (idExists || request->robot_id() == 0)
        {
            cout << "ID " << request->robot_id() << " exists" << endl;
            p.id = myDB.getHighestID() + 1;
        }
        else
        {
            p.id = request->robot_id();
        }
        if (p.id > myDB.getHighestID())
        {
            myDB.setHighestID(p.id);
        }

        p.name = request->robot_name();
        p.role = request->robot_role();
        response->set_is_active(p.isActive);
        response->set_robot_id(p.id);
        p.isActive = true;
        for (int i = 0; i < myDB.getTeamSize(); i++)
        {
            if (myDB.getTeam().playerList.at(i).id == p.id && myDB.getTeam().playerList.at(i).name == p.name)
            {
                myDB.setPlayerStatus(myDB.getTeam().playerList.at(i).id, true);
                return grpc::Status::OK;
            }
        }
        myDB.addPlayer(p);
        return grpc::Status::OK;
    }

    grpc::Status GetRobotStatus(grpc::ServerContext *context, const RobotRegistration *request, RobotStatus *response) override
    {
        for (int i{}; i < myDB.getTeamSize(); ++i)
        {
            if (myDB.getTeam().playerList.at(i).id == request->robot_id())
            {
                response->set_is_active(myDB.getTeam().playerList.at(i).isActive);
                cout << "Status of robot " << myDB.getTeam().playerList.at(i).name << " is " << myDB.getTeam().playerList.at(i).isActive << endl;
                return grpc::Status::OK;
            }
        }
        return grpc::Status::CANCELLED;
    }

    grpc::Status SendHeartbeat(grpc::ServerContext *context, const HeartbeatRequest *request, HeartbeatResponse *response) override
    {
        int id = request->robot_id();
        response->set_success(true);
        time_t lastBeat = time(NULL);
        if (robots.find(id) != robots.end())
        {
            robots.erase(id);
        }
        robots.insert({id, lastBeat});

        bool foundInDB{false};
        for (int i{}; i < myDB.getTeamSize(); i++)
        {
            if (myDB.getTeam().playerList[i].id == id)
            {
                foundInDB = true;
                break;
            }
        }
        if (foundInDB)
        {
            response->set_not_registered(false);
        }
        else
        {
            response->set_not_registered(true);
        }
        std::cout << "Received heartbeat from client: " << id << std::endl;
        return grpc::Status::OK;
    }

    grpc::Status SyncDB(grpc::ServerContext *context, const ServerRequest *request, DBResponse *response) override
    {
        // cout << request->name << endl;

        Team team = myDB.getTeam();
        response->set_captain_name(team.captain.player.name);

        for (int i{}; i < myDB.getTeamSize(); i++)
        {
            response->add_player_name(team.playerList.at(i).name);
            response->add_player_id(team.playerList.at(i).id);
        }

        response->set_highest_id(myDB.getHighestID());
        return grpc::Status::OK;
    }

    grpc::Status Ping_gRPC(grpc::ServerContext *context, const ServerRequest *request, Ping *response) override
    {
        response->set_is_active(true);
        return grpc::Status::OK;
    }

    grpc::Status StartElection(grpc::ServerContext *context, grpc::ServerReaderWriter<Response, Request> *stream) override
    {
        Request request;
        while (stream->Read(&request))
        {
            Response response;
            response.set_robot_id(1);
            response.set_robot_role(role);
            response.set_start(start);
            stream->Write(response);
        }
        return grpc::Status::OK;
    }

    grpc::Status ResultElection(grpc::ServerContext *context, const NewCaptainRequest *request, NewCaptainResponse *response) override
    {
        for (int i = 0; i < myDB.getTeamSize(); i++)
        {
            if (myDB.getTeam().playerList.at(i).id == request->robot_id())
            {
                Captain cp;
                Player pl;
                pl = myDB.getTeam().playerList.at(i);
                cp.player = pl;
                myDB.setCaptain(cp);
                start = false;
                role = "";
            }
        }
        return grpc::Status::OK;
    }
};

class RobotClient
{
public:
    RobotClient(std::shared_ptr<Channel> channel) : stub_(RobotControl::NewStub(channel)) {}

    grpc::Status SyncDB()
    {
        grpc::ClientContext context;
        ServerRequest request;
        DBResponse response;

        request.set_name("DBSync");

        grpc::Status status = stub_->SyncDB(&context, request, &response);

        if (status.ok())
        {

            Captain cpt{};
            Player pl{};

            pl.name = response.captain_name();
            pl.id = response.captain_id();
            cpt.player = pl;
            myDB.setCaptain(cpt);

            for (int i = 0; i < response.player_name_size(); ++i)
            {
                pl.name = response.player_name(i);
                pl.id = response.player_id(i);
                pl.isActive = true;
                myDB.addPlayer(pl);
                std::pair<int, time_t> tmp(pl.id, time(nullptr));
                robots.insert(tmp);
            }

            myDB.setHighestID(response.highest_id());

            cout << "DBSync-SUCCESS" << endl;
            return grpc::Status::OK;
        }
        else
        {
            cout << "DBSync-ERROR: Could not connect via gRPC" << endl;
            return grpc::Status::CANCELLED;
        }
    }

    bool Ping_gRPC()
    {
        grpc::ClientContext context;
        ServerRequest request;
        Ping response;

        request.set_name("ServerPing");
        response.set_is_active(false);

        grpc::Status status = stub_->Ping_gRPC(&context, request, &response);
        return response.is_active();
    }

private:
    std::unique_ptr<RobotControl::Stub> stub_;
};

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
            for (int i = 0; i < myDB.getTeamSize(); i++)
            {
                // if (myDB.getTeam().playerList.at(i).isActive == true)
                // {
                players += to_string(myDB.getTeam().playerList.at(i).id) + " : " + myDB.getTeam().playerList.at(i).name + " - " + to_string(myDB.getTeam().playerList.at(i).isActive) + "\n";
                //}
            }
            response = "HTTP/1.1 200 OK\r\n\r\nSystem Status: Es sind folgende Roboter registriert\n" + players + "\n";
        }
        else if (path == "/captain")
        {
            response = "HTTP/1.1 200 OK\r\n\r\nDer aktuelle Kapitän ist " + myDB.getCaptain().player.name + "\n";
        }
        else if (path == "/controller")
        {
            response = "HTTP/1.1 200 OK\r\n\r\nController Status: Der Zustand des Controllers ist \"" + myDB.getConStatus() + "\"\n";
        }
        else if (path == "/election/verteidiger")
        {
            start = true;
            role = "Verteidiger";
            response = "HTTP/1.1 200 OK\r\n\r\nNeue Kapitänswahl wurde angestoßen für Verteidiger\n";
        }
        else if (path == "/election")
        {
            start = true;
            role = "election";
            response = "HTTP/1.1 200 OK\r\n\r\nNeue Kapitänswahl wurde angestoßen für Verteidiger\n";
        }
        else if (path == "/election/stuermer")
        {
            start = true;
            role = "Stuermer";
            response = "HTTP/1.1 200 OK\r\n\r\nNeue Kapitänswahl wurde angestoßen für Stuermer\n";
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
        // myDB.persistPOST();
        write(client_socket, response.c_str(), response.length());
        auto end_time = chrono::high_resolution_clock::now();
        chrono::duration<double> rtt = end_time - start_time;
        cout << "RTT Post: " << rtt.count() * 1000 << " mseconds" << endl;
        cout << data << endl;
    }
    close(client_socket);
}

void CheckPlayerState()
{
    while (true && !threadSTOP)
    {
        vector<string> unactivePlayer{};
        this_thread::sleep_for(chrono::seconds(5));

        for (auto const &x : robots)
        {
            time_t t = time(nullptr);

            if ((t - x.second) > limit)
            {
                for (int i = 0; i < myDB.getTeamSize(); i++)
                {
                    if (myDB.getTeam().playerList.at(i).id == x.first)
                    {
                        if ((t - x.second) > limit * 10)
                        {
                            myDB.removePlayer(myDB.getTeam().playerList.at(i));
                        }
                        else
                        {
                            myDB.setPlayerStatus(myDB.getTeam().playerList.at(i).id, false);
                        }
                    }
                }
            }
        }

        for (int i = 0; i < myDB.getTeamSize(); i++)
        {
            if (myDB.getTeam().playerList.at(i).isActive == false)
            {
                unactivePlayer.push_back(myDB.getTeam().playerList.at(i).name);
            }
        }
        if (unactivePlayer.size() != 0)
        {
            cout << "Status: Player ";
            for (int i{}; i < unactivePlayer.size(); i++)
            {
                cout << unactivePlayer[i];
                if (i < (unactivePlayer.size() - 1))
                {
                    cout << " and ";
                }
            }
            cout << " are inactiv" << endl;
        }
        else
        {
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
    unique_ptr<grpc::Server> grpc_server(builder.BuildAndStart());

    cout << "gRPC server listening" << endl;
    grpc_server->Wait();
}

void dbSync()
{
    int i{};
    do
    {
        std::shared_ptr<grpc::Channel> channel{};

        if (isBackup)
        {
            channel = grpc::CreateChannel("robot-controller:50051", grpc::InsecureChannelCredentials());
        }
        else
        {
            channel = grpc::CreateChannel("robot-controller-backup:50051", grpc::InsecureChannelCredentials());
        }
        RobotClient ServerClient(channel);
        grpc::Status status = ServerClient.SyncDB();
        if (status.ok())
        {
            DBisSYNCED = true;
            i = 0;
        }
        else
        {
            i++;
        }
        if (i >= 2)
        {
            cerr << "DBSync ERROR TIMEOUT" << endl;
            return;
        }
        this_thread::sleep_for(chrono::seconds(5));
    } while (isBackup);
}

bool ping_gRPC()
{
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel("robot-controller:50051", grpc::InsecureChannelCredentials());
    RobotClient ServerClient(channel);
    return ServerClient.Ping_gRPC();
}

int httpServer()
{
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(httpPort);
    bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    listen(server_socket, 5);

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

    close(server_socket);
    return 0;
}

int main()
{

    cout << "Controller Start" << endl;
    thread http_thread(httpServer);
    thread grpc_thread(grpcServer);

    while (true)
    {
        threadSTOP = false;
        thread dbsync_thread(dbSync);

        while (ping_gRPC())
        {
            isBackup = true;
            this_thread::sleep_for(chrono::seconds(1));
        }

        thread thr_checkStatus(CheckPlayerState);

        while (controllerInUse)
        {
            if (isBackup && ping_gRPC())
            {
                cout << "Main in charge. Quitting Job" << endl;
                controllerInUse = false;
            };
            this_thread::sleep_for(chrono::seconds(1));
        }
        threadSTOP = true;
        dbsync_thread.join();
        thr_checkStatus.join();
    }

    http_thread.join();
    myDB.setConStatus("exited");
    return 0;
}
