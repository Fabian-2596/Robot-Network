#include <iostream>
#include <memory>
#include <string>
#include <chrono>
#include <thread>

#include <grpcpp/grpcpp.h>
#include "robot.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using namespace std;
int robotid = 0;
class RobotClient {
public:
    RobotClient(std::shared_ptr<Channel> channel) : stub_(RobotControl::NewStub(channel)) {}

    RobotStatus RegisterRobot(const RobotRegistration& registration) {
        RobotStatus response;
        grpc::ClientContext context;
        grpc::Status status = stub_->RegisterRobot(&context, registration, &response);

        if (status.ok()) {
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

    RobotStatus SetRobotStatus(const RobotChangeStatus& chStatus){
        RobotStatus response;
        grpc::ClientContext context;
        grpc::Status status = stub_->SetRobotStatus(&context, chStatus, &response);

        if (status.ok()) {
            std::cout << "Robot status set. Status: " << response.is_active() << std::endl;
        } else {
            std::cerr << "Error getting robot status: " << status.error_message() << std::endl;
        }
        return response;
    }

private:
    std::unique_ptr<RobotControl::Stub> stub_;
};


bool loop{true};
void sendStatus(std::shared_ptr<grpc::Channel> channel, int robotID, bool status){
    RobotChangeStatus chStatus;
    chStatus.set_robot_id(robotID);
    chStatus.set_is_active(status);

    RobotClient robotClient(channel);

    while (loop)
    {
        this_thread::sleep_for(chrono::seconds(5));
        robotClient.SetRobotStatus(chStatus);
    }
    chStatus.set_is_active(false);
    robotClient.SetRobotStatus(chStatus);
}

int main() {
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel("robot-controller:50051", grpc::InsecureChannelCredentials());
    RobotClient robotClient(channel);

    RobotRegistration registration;
    registration.set_robot_name("Clara");
    registration.set_robot_id(5);
    RobotStatus status = robotClient.RegisterRobot(registration);

    status = robotClient.GetRobotStatus(registration);

    thread thr_sendStatus(sendStatus,channel,5,true);

    this_thread::sleep_for(chrono::seconds(20));
    loop = false;

    thr_sendStatus.join();

    return 0;
}
