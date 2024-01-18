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
const std::string ELECTION_TOPIC = "election";
const std::string SERVER_ADDRESS = "tcp://mosquitto:1883";

class ElectionCallback : public virtual mqtt::callback, public virtual mqtt::iaction_listener {
public:
    mqtt::async_client& client_;

    void on_failure(const mqtt::token& tok) override {
        std::cerr << "Connection failure: " << std::endl;
    }

    void on_success(const mqtt::token& tok) override {
        std::cout << "Connection success" << std::endl;
    }

    void connection_lost(const std::string& cause) override {
        std::cerr << "Connection lost: " << cause << std::endl;
    }

    void message_arrived(mqtt::const_message_ptr msg) override {
        std::string payload = msg->to_string();
        std::string tmp = "start" + std::to_string(robotid);
        std::string tmp2 = "election" + std::to_string(robotid);
        if (payload == tmp) {
            std::cout << "Election process initiated by the server." << std::endl;
            start_election();
        } else if (payload.find("election:") == 0) {
            cout << "handle election message from robot " << robotid << endl;
            handle_election_message(payload);
        }
    }

    void start_election() {
        cout << "started election" << endl;
        send_election_message();    
    }

    void send_election_message() {
        cout << "election message sent from robot " << robotid << endl;
        std::string election_message = "election:" + to_string(robotid);
        client_.publish(ELECTION_TOPIC, election_message, 2, false);
    }

    void handle_election_message(const std::string& message) {
        size_t colon_pos = message.find(":");
        std::string sender_id = message.substr(colon_pos + 1);
        if (sender_id > to_string(robotid)) {
            isCaptain = false;
        } 
        else if(sender_id == to_string(robotid)){
            isCaptain = true;
            cout << "robot: " << robotid << " is now leader" << endl; 
            client_.publish("leader", to_string(robotid), 2, false);
        }
        else if(sender_id < to_string(robotid)){
            send_election_message();
        }
    }
    
public:
	ElectionCallback(mqtt::async_client& client)
				: client_(client) {

                }
};

class RobotClient {
public:
    RobotClient(std::shared_ptr<Channel> channel) : stub_(RobotControl::NewStub(channel)) {}

    RobotStatus RegisterRobot(const RobotRegistration& registration) {
        RobotStatus response;
        grpc::ClientContext context;
        grpc::Status status = stub_->RegisterRobot(&context, registration, &response);
        if (status.ok()) {
            robotid = registration.robot_id();
            std::cout << "Robot registration successful. ID: " << registration.robot_id();
            std::cout << ", Name: " << registration.robot_name() << std::endl;
        } else {
            std::cerr << "Error in robot registration: " << status.error_message() << std::endl;
        }

        return response;
    }

    RobotStatus GetRobotStatus(const RobotRegistration& registration) {
        RobotStatus response;
        grpc::ClientContext context;
        grpc::Status status = stub_->GetRobotStatus(&context, registration, &response);

        if (status.ok()) {
            std::cout << "Robot status received. Status: " << response.is_active() << std::endl;
        } else {
            std::cerr << "Error getting robot status: " << status.error_message() << std::endl;
        }

        return response;
    }

    void SendHeartbeat() {
        HeartbeatRequest request;
        request.set_robot_id(robotid);
        HeartbeatResponse response;

        grpc::ClientContext context;
        grpc::Status status = stub_->SendHeartbeat(&context, request, &response);

        if (status.ok()) {
            std::cout << "Heartbeat sent successfully." << std::endl;
        } else {
            std::cerr << "Failed to send heartbeat: " << status.error_message() << std::endl;
        }
    }

private:
    std::unique_ptr<RobotControl::Stub> stub_;
};

bool loop{true};

void sendBeat(std::shared_ptr<grpc::Channel> channel){
    RobotClient robotClient(channel);
    
    while (loop)
    {
        robotClient.SendHeartbeat();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}


int main(int argc, char *argv[]) {
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel("robot-controller:50051", grpc::InsecureChannelCredentials());
    RobotClient robotClient(channel);

    RobotRegistration registration;
    registration.set_robot_name(argv[2]);
    registration.set_robot_id(stoi(argv[1]));
    RobotStatus status = robotClient.RegisterRobot(registration);

    mqtt::async_client client(SERVER_ADDRESS, argv[1]);
    ElectionCallback cb(client);
    client.set_callback(cb);

    try {
        cout << "\nConnecting..." << endl;
	    mqtt::token_ptr tok = client.connect();
        cout << "Waiting for the connection..." << endl;
		tok->wait();
		cout << "  ...OK" << endl;
        client.subscribe(ELECTION_TOPIC, 1)->wait();

    } catch (const mqtt::exception& exc) {
        std::cerr << "Error: " << exc.what() << std::endl;
    }

    status = robotClient.GetRobotStatus(registration);
    thread thr_beat(sendBeat, channel);
    thr_beat.join();
    return 0;
}
