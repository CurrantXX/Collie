#include <iostream>
#include <string>
#include "../include/tcp/TcpClient.hpp"
#include "../include/tcp/TcpConnection.hpp"
#include "../include/Global.hpp"

using Collie::Tcp::TcpClient;
using Collie::Tcp::TcpConnection;
using namespace Collie;

int
main(int argc, char *argv[]) {

    auto & logger = Logger::LogHandler::getHandler();
    logger.setLogLevel(TRACE);
    logger.init();


    TcpClient client;
    client.setConnectCallback([](std::shared_ptr<TcpConnection> conn) {
            conn->send("Hello, here is client");
            const std::string content = conn->recvAll();
            std::cout << content << std::endl;
            conn->disconnect();
        });
    client.connect("127.0.0.1", 8080);
    return 0;
}
