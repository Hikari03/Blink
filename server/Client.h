#pragma once

#include <string>
#include <sys/socket.h>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <mutex>
#include "Message.h"


class Client {

public:
    Client(int socket, std::vector<Message> & messages, std::mutex & messagesMutex);

    ~Client();

    [[nodiscard]] int getSocket() const;

    /**
     * @brief runs the client
     */
    void operator()();

    [[nodiscard]] bool isActive() const;

    /**
     * @brief sends exit _message to the client
     */
    void exit();


private:

    std::string _name;
    int _socket;
    int _sizeOfPreviousMessage = 0;
    std::string _message;
    bool _active = false;

    //outside references
    std::vector<Message> & _messages;
    std::mutex & _messagesMutex;

    //contd
    char _buffer[4096] = {0};

    /**
     * @brief gets _name of the client
     * @todo implement encryption
     */
    void initConnection();

    void clearBuffer();

    void sendThread();
    void receiveThread();

    void sendMessage(const std::string & message) const;
    void receiveMessage();

    void submitReceivedMessage();

};