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

    void connect(std::string ip, int port);

    void send(std::string message);

    std::string receive();

    void close();

private:
    int _socket;
    struct sockaddr_in _server;

};