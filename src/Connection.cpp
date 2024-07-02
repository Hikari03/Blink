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
        if (auto result = dnsLookup(ip); result.empty()) {
            throw std::runtime_error("Invalid address / Address not supported");
        } else {
            ip = result[0];
            if(inet_pton(AF_INET, ip.c_str(), &_server.sin_addr) <= 0) {
                throw std::runtime_error("Invalid address / Address not supported");
            }
        }
    }

    connect(_socket, (struct sockaddr*)&_server, sizeof(_server));

}

void Connection::send(const std::string & message) const {

    auto messageToSend = message + _end;

    if(::send(_socket, messageToSend.c_str(), messageToSend.length(), 0) < 0) {
        throw std::runtime_error("Could not send message");
    }
}

void Connection::sendMessage(const std::string & message) const {
    send(_text + message);
}

std::string Connection::receive() {
    std::string message;


    while (!message.contains(_end)) {

        clearBuffer();

        _sizeOfPreviousMessage = recv(_socket, _buffer, 4096, 0);

        if(_sizeOfPreviousMessage < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != MSG_WAITALL) {
            throw std::runtime_error("Could not receive message from server: " + std::string(strerror(errno)));
        }

        message += _buffer;
    }

    // remove the _end string
    message = message.substr(0, message.find(_end));

	//TODO handle this in App.cpp
    if(message.contains(_text))
        message = message.substr(sizeof(_text)-1, message.length());

    return message;
}

void Connection::_close() {
    send(_internal"exit");
    shutdown(_socket, 0);
    _active = false;
}

Connection::~Connection() {
    if(_active)
        _close();
}

void Connection::clearBuffer() {

    for(int i = 0; i < _sizeOfPreviousMessage; i++) {
        _buffer[i] = '\0';
    }

}

std::vector<std::string> Connection::dnsLookup(const std::string & domain, int ipv) {
    // credit to http://www.zedwood.com/article/cpp-dns-lookup-ipv4-and-ipv6

    std::vector<std::string> output;

    struct addrinfo hints, *res, *p;
    int status, ai_family;
    char ip_address[INET6_ADDRSTRLEN];

    ai_family = ipv==6 ? AF_INET6 : AF_INET; //v4 vs v6?
    ai_family = ipv==0 ? AF_UNSPEC : ai_family; // AF_UNSPEC (any), or chosen
    memset(&hints, 0, sizeof hints);
    hints.ai_family = ai_family;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(domain.c_str(), NULL, &hints, &res)) != 0) {
        //cerr << "getaddrinfo: "<< gai_strerror(status) << endl;
        return output;
    }

    //cout << "DNS Lookup: " << host_name << " ipv:" << ipv << endl;

    for(p = res;p != NULL; p = p->ai_next) {
        void *addr;
        if (p->ai_family == AF_INET) { // IPv4
            auto *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
        } else { // IPv6
            auto *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
        }

        // convert the IP to a string
        inet_ntop(p->ai_family, addr, ip_address, sizeof ip_address);
        output.emplace_back(ip_address);
    }

    freeaddrinfo(res); // free the linked list

    return output;
}