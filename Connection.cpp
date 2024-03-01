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

    if(inet_pton(AF_INET, ip.c_str(), &_server.sin_addr) <= 0) {
        throw std::runtime_error("Invalid address/ Address not supported");
    }
    //bind(_socket, (struct sockaddr*)&_server, sizeof(_server));

    connect(_socket, (struct sockaddr*)&_server, sizeof(_server));

}

void Connection::send(std::string message) {
    if(::send(_socket, message.c_str(), message.length(), 0) < 0) {
        throw std::runtime_error("Could not send message");
    }
}

std::string Connection::receive() {
    char buffer[4096] = {0};
    if(recv(_socket, buffer, 4096, 0) < 0) {
        throw std::runtime_error("Could not receive message");
    }
    return std::string(buffer);
}

void Connection::close() {
    ::close(_socket);
}

Connection::~Connection() {
    close();
}