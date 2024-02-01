#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <thread>
#include <grpcpp/grpcpp.h>
#include "robot.grpc.pb.h"
#include <mqtt/async_client.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using namespace std;

int robotid = 0;
bool isCaptain = false;
string role;
string name;

std::string ELECTION_TOPIC = "election";
const std::string SERVER_ADDRESS = "tcp://mosquitto:1883";
std::shared_ptr<grpc::Channel> channel;
bool loop{true};
bool mainIsAlive{true};

class ElectionCallback : public virtual mqtt::callback, public virtual mqtt::iaction_listener
{
public:
    mqtt::async_client &client_;

    void on_failure(const mqtt::token &tok) override
    {
        std::cerr << "Connection failure: " << std::endl;
    }

    void on_success(const mqtt::token &tok) override
    {
        std::cout << "Connection success" << std::endl;
    }

    void connection_lost(const std::string &cause) override
    {
        std::cerr << "Connection lost: " << cause << std::endl;
    }

    void message_arrived(mqtt::const_message_ptr msg) override
    {
        std::string payload = msg->to_string();
        isCaptain = false;
        std::string tmp = "start" + std::to_string(robotid);
        size_t colon_pos = payload.find(";");
        std::string topic = payload.substr(colon_pos + 1);
        if (payload == tmp + ";" + topic)
        {
            std::cout << "Election process initiated by the server." << std::endl;
            start_election(topic);
        }
        else if (payload.find("election:") == 0)
        {
            cout << "handle election message from robot " << robotid << endl;
            string msg = payload.substr(0, colon_pos);
            handle_election_message(msg, topic);
        }
    }

    void start_election(string topic)
    {
        cout << "started election" << endl;
        send_election_message(topic);
    }

    void send_election_message(string topic)
    {
        cout << "election message sent from robot " << robotid << endl;
        std::string election_message = "election:" + to_string(robotid) + ";" + topic;
        client_.publish(topic, election_message, 2, false);
    }

    void handle_election_message(const std::string &message, string topic)
    {
        size_t colon_pos = message.find(":");
        std::string sender_id = message.substr(colon_pos + 1);
        if (sender_id > to_string(robotid))
        {
            isCaptain = false;
        }
        else if (sender_id == to_string(robotid))
        {
            isCaptain = true;
            cout << "robot: " << robotid << " is now leader" << endl;
        }
        else if (sender_id < to_string(robotid))
        {
            send_election_message(topic);
        }
    }

public:
    ElectionCallback(mqtt::async_client &client)
        : client_(client)
    {
    }
};

class RobotClient
{
public:
    RobotClient(std::shared_ptr<Channel> channel) : stub_(RobotControl::NewStub(channel)) {}

    bool RegisterRobot(const RobotRegistration &registration)
    {
        RobotStatus response;
        grpc::ClientContext context;
        grpc::Status status = stub_->RegisterRobot(&context, registration, &response);
        if (status.ok())
        {
            robotid = response.robot_id();
            role = registration.robot_role();
            name = registration.robot_name();
            std::cout << "Robot registration successful. ID: " << robotid;
            std::cout << ", Name: " << registration.robot_name();
            std::cout << ", Rolle: " << registration.robot_role() << endl;
            return true;
        }
        else
        {
            std::cerr << "Error in robot registration: " << status.error_message() << std::endl;
            return false;
        }
    }

    RobotStatus GetRobotStatus(const RobotRegistration &registration)
    {
        RobotStatus response;
        grpc::ClientContext context;
        grpc::Status status = stub_->GetRobotStatus(&context, registration, &response);

        if (status.ok())
        {
            std::cout << "Robot status received. Status: " << response.is_active() << std::endl;
        }
        else
        {
            std::cerr << "Error getting robot status: " << status.error_message() << std::endl;
        }

        return response;
    }

    bool SendHeartbeat()
    {
        HeartbeatRequest request;
        request.set_robot_id(robotid);
        HeartbeatResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->SendHeartbeat(&context, request, &response);

        if (status.ok())
        {
            std::cout << "Heartbeat sent successfully." << std::endl;
            if (response.not_registered())
            {
                RobotRegistration Rrequest;
                Rrequest.set_robot_id(robotid);
                Rrequest.set_robot_name(name);
                Rrequest.set_robot_role(role);
                RobotStatus Rresponse;
                grpc::ClientContext Rcontext;

                grpc::Status status = stub_->RegisterRobot(&Rcontext, Rrequest, &Rresponse);
                if (status.ok()){return true;}else{return false;}
            }
            return true;
        }
        else
        {
            std::cerr << "Failed to send heartbeat: " << status.error_message() << std::endl;
            return false;
        }
    }

    bool Ping_gRPC()
    {
        Ping request;
        Ping response;

        grpc::ClientContext context;
        grpc::Status status = stub_->Ping_gRPC(&context, request, &response);
        if (status.ok())
        {
            cout << "Ping Success" << endl;
            return true;
        }
        else
        {
            cout << "Ping to Controller Failed" << endl;
            return false;
        }
    }

    void StartElection()
    {
        ClientContext context;
        Request request;
        Response response;
        std::shared_ptr<grpc::ClientReaderWriter<Request, Response>> stream(stub_->StartElection(&context));
        while (true)
        {
            stream->Write(request);
            stream->Read(&response);
            if (response.start() == true && robotid == response.robot_id())
            {
                cout << "Election started for role: " << response.robot_role() << endl;
                mqtt::async_client client(SERVER_ADDRESS, to_string(0));
                ElectionCallback cb(client);
                client.set_callback(cb);

                try
                {
                    cout << "\nConnecting..." << endl;
                    mqtt::token_ptr tok = client.connect();
                    cout << "Waiting for the connection..." << endl;
                    tok->wait();
                    cout << "  ...OK" << endl;
                    int qos = 0;
                    string tmp = "start" + to_string(robotid) + ";" + response.robot_role();
                    mqtt::message_ptr pubmsg = mqtt::make_message(ELECTION_TOPIC, tmp);
                    pubmsg->set_qos(qos);
                    mqtt::delivery_token_ptr pubtok = client.publish(pubmsg);
                    pubtok->wait();
                    cout << "data published" << std::endl;
                    client.disconnect();
                }
                catch (const mqtt::exception &exc)
                {
                    std::cerr << "Error: " << exc.what() << std::endl;
                }
            }
            // std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void newCaptain()
    {
        NewCaptainRequest request;
        request.set_robot_id(robotid);
        NewCaptainResponse response;
        grpc::ClientContext context;
        grpc::Status status = stub_->ResultElection(&context, request, &response);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

private:
    std::unique_ptr<RobotControl::Stub> stub_;
};

void pingMain()
{
    while (true)
    {
        this_thread::sleep_for(chrono::seconds(1));
        std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel("robot-controller:50051", grpc::InsecureChannelCredentials());
        RobotClient robotClient(channel);
        if (robotClient.Ping_gRPC())
        {
            mainIsAlive = true;
            return;
        }
    }
}

void sendBeat(std::shared_ptr<grpc::Channel> channel)
{
    while (loop)
    {
        string test = "robot-controller:50051";
        std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel("robot-controller:50051", grpc::InsecureChannelCredentials());
        RobotClient robotClient(channel);

        if (!robotClient.SendHeartbeat())
        {
            mainIsAlive = false;
            cout << "switch to backup" << endl;
            thread pingMain_thread(pingMain);
            pingMain_thread.detach();
            while (!mainIsAlive && loop)
            {
                channel = grpc::CreateChannel("robot-controller-backup:50051", grpc::InsecureChannelCredentials());
                RobotClient robotClientBackup(channel);
                if (!robotClientBackup.SendHeartbeat())
                {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void election(std::shared_ptr<grpc::Channel> channel)
{
    RobotClient robotClient(channel);
    robotClient.StartElection();
}

void checkCaptain(std::shared_ptr<grpc::Channel> channel)
{
    RobotClient robotClient(channel);
    while (true)
    {
        if (isCaptain)
            robotClient.newCaptain();
    }
}

int main(int argc, char *argv[])
{
    channel = grpc::CreateChannel("robot-controller:50051", grpc::InsecureChannelCredentials());
    RobotClient robotClient(channel);

    RobotRegistration registration;
    if (argc > 1)
    {
        registration.set_robot_name(argv[1]);
        if (argc > 2)
        {
            registration.set_robot_id(stoi(argv[2]));
        }
        if (argc > 3)
        {
            registration.set_robot_role(argv[3]);
        }
    }

    RobotStatus status;
    if (!robotClient.RegisterRobot(registration))
    {
        channel = grpc::CreateChannel("robot-controller-backup:50051", grpc::InsecureChannelCredentials());
        RobotClient robotClientBackup(channel);
        if (!robotClientBackup.RegisterRobot(registration))
        {
            cerr << "The robot could not register at any server" << endl;
            return -1;
        }
    }

    mqtt::async_client client(SERVER_ADDRESS, argv[1]);
    ElectionCallback cb(client);
    client.set_callback(cb);

    try
    {
        cout << "\nConnecting..." << endl;
        mqtt::token_ptr tok = client.connect();
        cout << "Waiting for the connection..." << endl;
        tok->wait();
        cout << "  ...OK" << endl;
        client.subscribe(ELECTION_TOPIC, 1)->wait();
        client.subscribe(role, 1)->wait();
        cout << "test" << endl;
    }
    catch (const mqtt::exception &exc)
    {
        std::cerr << "Error: " << exc.what() << std::endl;
    }

    status = robotClient.GetRobotStatus(registration);
    thread thr_beat(sendBeat, channel);
    thread thr_election(election, channel);
    thread thr_check(checkCaptain, channel);
    thr_beat.join();
    return 0;
}
