#include <iostream>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include "gen-cpp/register_types.h"
#include "gen-cpp/RobotController.h"

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

int main() {
    std::string controllerAddress = "localhost";
    int controllerPort = 9090;
    std::string name = "TestRoboter";
    int id = 0;

    std::shared_ptr<TTransport> socket(new TSocket(controllerAddress, controllerPort));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

    RobotControllerClient client(protocol);

    try {
        transport->open();
        client.registerRobot(id, name);
        transport->close();
    } catch (const TException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
