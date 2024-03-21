#pragma once

#include <cstring>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdexcept>
#include <resolv.h>
#include <cerrno>
#include <vector>
#include <iostream>

#define _end "::--///-$$$"
#define _internal "INTERNAL::"
#define _text "MESSAGE::"


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
    char _buffer[4096] = {0};
    int _socket;
    ssize_t _sizeOfPreviousMessage = 0;
    sockaddr_in _server;

    void clearBuffer();

    [[nodiscard]] static std::vector<std::string> dnsLookup(const std::string & domain, int ipv = 4) ;

};