#include <iostream>
#include "Connection.h"

Connection::Connection() {
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket == -1) {
        throw std::runtime_error("Could not create socket");
    }

}

void Connection::connectToServer(std::string ip, int port) {
    _server.sin_family = AF_INET;
    _server.sin_port = htons(port);

    if(ip == "localhost" || ip.empty())
        ip = "127.0.0.1";

    if(inet_pton(AF_INET, ip.c_str(), &_server.sin_addr) <= 0) {
        throw std::runtime_error("Invalid address/ Address not supported");
    }

    connect(_socket, (struct sockaddr*)&_server, sizeof(_server));

}

void Connection::send(const std::string & message) const {
    if(::send(_socket, message.c_str(), message.length(), 0) < 0) {
        throw std::runtime_error("Could not send message");
    }
}

std::string Connection::receive() {
    char buffer[4096] = {0};
    if(recv(_socket, buffer, 4096, 0) < 0) {
        throw std::runtime_error("Could not receive message");
    }

    // if we receive "exit" we need to exit app
    if(std::string(buffer) == "exit") {
        throw std::runtime_error("Server closed connection");
    }

    return {buffer};
}

void Connection::close() const {
    send("exit");
    shutdown(_socket, 0);
}

Connection::~Connection() {
    close();
}