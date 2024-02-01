
#include <gtest/gtest.h>
#include <grpcpp/create_channel.h>
#include "testP2.grpc.pb.h" 

const std::string server_address = "localhost:50051";


class gRPCTest : public ::testing::Test {
protected:
    void SetUp() override {
        channel = grpc::CreateChannel("127.0.0.0:50051", grpc::InsecureChannelCredentials());
        stub = testgRPC::MyService::NewStub(channel);
    }

    void TearDown() override {
    }

    std::shared_ptr<grpc::Channel> channel;
    std::unique_ptr<testgRPC::MyService::Stub> stub;
};


TEST_F(gRPCTest, RegisterRobotTest) {
    
    RobotRegistration request;
    request.set_robot_id(9999);
    request.set_robot_name("testRobot");
    grpc::ClientContext context;
    RobotStatus response;
    grpc::Status status = stub->RegisterRobot(&context, request, &response);

    ASSERT_TRUE(status.ok());
    ASSERT_TRUE(request.robot_id() == response.robot_id());
}

TEST_F(gRPCTest, SetRobotStatusTest) {
    
    RobotSetStatus request;
    request.robot_id = 9999;
    request.is_active = false;
    grpc::ClientContext context;
    RobotStatus response;
    grpc::Status status = stub->SetRobotStatus(&context, request, &response);

    ASSERT_TRUE(status.ok());
}

TEST_F(gRPCTest, SendHeartbeatTest) {

    HeartbeatRequest request;
    request.robot_id = 9999;
    grpc::ClientContext context;
    HeartbeatResponse response;
    grpc::Status status = stub->SendHeartbeat(&context, request, &response);

    ASSERT_TRUE(status.ok());
    ASSERT_TRUE(response.success());
}
