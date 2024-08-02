#include "ClientInfo.h"

bool ClientInfo::init(const std::string & name, const std::string & ip, const int & socket) {
	if(initialized)
		return false;

	this->name = name;
	this->ip = ip;
	this->socket_ = socket;
	this->initialized = true;
	return true;
}

std::string ClientInfo::getName() const {
	return name;
}

std::string ClientInfo::getIp() const {
	return ip;
}

int ClientInfo::getSocket() const {
	return socket_;
}

std::string ClientInfo::convertAddrToString(const sockaddr_in &addr) {
	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
	return {ip};
}

void ClientInfo::initOnlineUsers(const std::vector<std::string> & users) {
	onlineUsers = users;
}

void ClientInfo::setUserAsOnline() {
	if(std::find(onlineUsers.begin(), onlineUsers.end(), name) == onlineUsers.end())
		onlineUsers.push_back(name);
}
