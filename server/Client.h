#pragma once

#include <string>
#include <sys/socket.h>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <mutex>


class Client {

public:
    Client(int socket, std::vector<std::pair<std::string, std::string>> & messages);

    ~Client();

    [[nodiscard]] int getSocket() const;

    /**
     * @brief runs the client
     */
    void operator()(std::mutex & messagesMutex);

    [[nodiscard]] bool isActive() const;

    /**
     * @brief sends exit message to the client
     */
    void exit();


private:

    std::string name;
    int socket;
    int sizeOfPreviousMessage = 0;
    std::string message;
    bool active = false;

    //outside references
    std::vector<std::pair<std::string, std::string>> & messages;

    //contd
    char buffer[4096] = {0};

    /**
     * @brief gets name of the client
     * @todo implement encryption
     */
    void initConnection();

    void clearBuffer();

    void sendThread();
    void receiveThread();

    void sendMessage(const std::string & message) const;
    void receiveMessage();

};