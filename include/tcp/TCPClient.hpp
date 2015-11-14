#ifndef COLLIE_TCP_TCPCLIENT_H
#define COLLIE_TCP_TCPCLIENT_H

#include <functional>
#include <memory>
#include <string>

namespace Collie {

class SocketAddress;

namespace TCP {

class Connector;
class TCPSocket;

class TCPClient {
public:
    using ConnectCallback = std::function<void(std::shared_ptr<TCPSocket>)>;

    TCPClient();
    TCPClient(const TCPClient &) = delete;
    TCPClient & operator=(const TCPClient &) = delete;
    ~TCPClient();

    void setConnectCallback(const ConnectCallback & cb) {
        connectCallback = cb;
    }
    void setConnectCallback(const ConnectCallback && cb) {
        connectCallback = std::move(cb);
    }
    void connect(const std::string & host, const unsigned port,
                 const size_t threadNum = 1, const size_t connectNum = 1);

private:
    std::unique_ptr<Connector> connector;
    ConnectCallback connectCallback;
    std::shared_ptr<SocketAddress> remoteAddr;
};
}
}

#endif /* COLLIE_TCP_TCPCLIENT_H */