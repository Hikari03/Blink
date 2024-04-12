#pragma once

#include <cstring>
#include <string>
#include <sys/socket.h>
#include <stdexcept>
#include <iostream>
#include <set>
#include <mutex>
#include <thread>
#include "Message.h"
#include "MessageHolder.h"


#define _end "::--///-$$$"
#define _internal "INTERNAL::"
#define _text "MESSAGE::"


class Client {

public:
    Client(int socket, MessageHolder & messages);

    ~Client();

    [[nodiscard]] int getSocket() const;

    /**
     * @brief runs the client
     */
    void run();

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
    bool _active = true;

    //outside references
    MessageHolder & _messages;
    std::mutex & _messagesMutex;
    std::condition_variable & _callBackOnMessagesChange;

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

    void submitMessage(const std::string & message);

    void processMessage();

};