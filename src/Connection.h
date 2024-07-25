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
#include <mutex>
#include <memory>
#include <thread>
#include <sodium.h>

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

    void send(const std::string & message);
    void sendMessage(const std::string & message);
	void sendInternal(const std::string & message);


    std::string receive();
	void close();

private:
    char _buffer[4096] = {0};
	unsigned char _symKey[crypto_secretbox_KEYBYTES];
	unsigned char _nonce[crypto_secretbox_NONCEBYTES];
	std::mutex _sendMutex;
    int _socket;
    ssize_t _sizeOfPreviousMessage = 0;
    sockaddr_in _server;
    bool _active = true;
	bool _encrypted = false;

    void clearBuffer();

    [[nodiscard]] static std::vector<std::string> dnsLookup(const std::string & domain, int ipv = 4);

	void _send(const char * message, size_t length);

	void _secretOpen(std::string & message);
	void _secretSeal(std::string & message);

};