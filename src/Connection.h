#pragma once

#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>


/**
 * @brief Handles the connection to the server
 */
class Connection {

public:
    Connection();
    ~Connection();

    void connectToServer(std::string ip, int port);

    void send(const std::string & message) const;

    std::string receive();

    void close() const;

private:
    int _socket;
    sockaddr_in _server;

};