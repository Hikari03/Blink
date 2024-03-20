#pragma once

#include <cstring>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdexcept>
#include <resolv.h>
#include <vector>
#include <iostream>


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

    [[nodiscard]] static std::vector<std::string> dnsLookup(const std::string & domain, int ipv = 4) ;

};