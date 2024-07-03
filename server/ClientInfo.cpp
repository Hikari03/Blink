#include "ClientInfo.h"

bool ClientInfo::init(const std::string & name, const std::string & ip, const int & socket) {
	if(initalized)
		return false;

	this->name = name;
	this->ip = ip;
	this->socket_ = socket;
	this->initalized = true;
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
