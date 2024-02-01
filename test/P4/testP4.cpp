#include <gtest/gtest.h>
#include <grpcpp/grpcpp.h>
#include "testP4.grpc.pb.h"

class BackupTest
{
public:
    BackupTest(std::shared_ptr<Channel> channel) : stub_(BackupControl::NewStub(channel)) {}

    bool RegisterRobot(const RobotRegistration &registration)
    {
        RobotStatus response;
        grpc::ClientContext context;
        grpc::Status status = stub_->RegisterRobot(&context, registration, &response);
        if (status.ok())
        {
            robotid = response.robot_id();
            std::cout << "Robot registration successful. ID: " << robotid;
            std::cout << ", Name: " << registration.robot_name() << std::endl;
            return true;
        }
        else
        {
            std::cerr << "Error in robot registration: " << status.error_message() << std::endl;
            return false;
        }
    }
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

private:
    std::unique_ptr<RobotControl::Stub> stub_;
};

class BackupServerTest : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        system("docker run -d -p 8080:8080 -p 50051:50051 --name robot-controller vs-robot-controller");
        system("docker run -d -p 8081:8080 -p 50052:50051 --name robot-controller-backup vs-robot-controller");

        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    static void TearDownTestCase()
    {
        system("docker stop robot-controller");
        system("docker rm robot-controller");
        system("docker stop robot-controller-backup");
        system("docker rm robot-controller-backup");
    }
};

TEST_F(BackupServerTest, GrpcInteraction)
{
    auto channel = grpc::CreateChannel("robot-controller:50051", grpc::InsecureChannelCredentials());
    BackupTest testClient(channel);
    
    grpc::ClientContext context;
    RobotRegistration request;
    request.set_name("TestRobot");
    request.set_robot_id(123);
    RobotStatus response;
    grpc::Status status = stub->RegisterRobot(&context, request, &response);

    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.robot_id(), 123);
}

TEST_F(BackupServerTest, GrpcInteraction)
{

    system("docker stop robot-controller");
    std::this_thread::sleep_for(std::chrono::seconds(10));

    auto channel = grpc::CreateChannel("robot-controller-backup:50051", grpc::InsecureChannelCredentials());
    BackupTest testClient(channel);

    grpc::ClientContext context;
    ServerRequest request;
    DBResponse response;
    grpc::Status status = stub->DBSync(&context, request, &response);

    EXPECT_TRUE(status.ok());
    EXPECT_EQ(response.player_name(0), "TestRobot");
    EXPECT_EQ(response.player_id(0), 123);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
