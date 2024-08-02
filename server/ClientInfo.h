#pragma once

#include <string>
#include <netinet/in.h>
#include <bits/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <algorithm>

struct ClientInfo {

	ClientInfo() = default;

	bool init(const std::string & name, const std::string & ip, const int & socket);

	[[nodiscard]] std::string getName() const;
	[[nodiscard]] std::string getIp() const;
	[[nodiscard]] int getSocket() const;

	static std::string convertAddrToString(const sockaddr_in & addr);

	std::string name;
	std::string ip;
	int socket_ = 0;

	bool initialized = false;

private:
	std::vector<std::string> onlineUsers;

};
