#pragma once

#include <cstring>
#include <string>
#include <sys/socket.h>
#include <stdexcept>
#include <iostream>
#include <set>
#include <mutex>
#include <thread>
#include <sodium.h>
#include "Message.h"
#include "MessageHolder.h"
#include "ClientInfo.h"


#define _end "::--///-$$$"
#define _internal "INTERNAL::"
#define _text "MESSAGE::"


class Client {

public:
    Client(const ClientInfo clientInfo, MessageHolder & messages);

    ~Client();

    [[nodiscard]] const ClientInfo & info() const;

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

	ClientInfo _clientInfo;
    int _sizeOfPreviousMessage = 0;
    std::string _message;
	unsigned char _symKey[crypto_secretbox_KEYBYTES];
	unsigned char _nonce[crypto_secretbox_NONCEBYTES];
    bool _active = true;
	bool _encrypted = false;

    //outside references
    MessageHolder & _messages;
    std::mutex & _messagesMutex;
    std::condition_variable & _callBackOnMessagesChange;

    //contd
    char _buffer[4096] = {0};

    /**
     * @brief gets name of the client
     * @todo implement encryption
     */
    void initConnection();

    void clearBuffer();

    void sendThread();
    void receiveThread();

    void sendMessage(const std::string & message);
    void receiveMessage();

    void submitMessage(const std::string & message);

    void processMessage();

	void secretOpen(std::string & message);
	void secretSeal(std::string & message);
};