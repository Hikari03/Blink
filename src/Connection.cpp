#include "Connection.h"

Connection::Connection() {
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket == -1) {
        throw std::runtime_error("Could not create socket");
    }

	if (sodium_init() < 0) {
		throw std::runtime_error("Could not initialize sodium");
	}

	crypto_box_keypair(_keyPair.publicKey, _keyPair.secretKey);
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

void Connection::_send(const char * message, size_t length) {
	std::lock_guard<std::mutex> lock(_sendMutex);
	if(::send(_socket, message, length, 0) < 0) {
		throw std::runtime_error("Could not send message");
	}
}

void Connection::send(const std::string & message){
    auto messageToSend = message;

	printf("SEND | %s\n", messageToSend.c_str());

	if(_encrypted)
		_secretSeal(messageToSend);

	messageToSend += _end;

	_send(messageToSend.c_str(), messageToSend.size());
}

void Connection::sendMessage(const std::string & message) {
    send(_text + message);
}

void Connection::sendInternal(const std::string &message) {
	send(_internal + message);
}

std::string Connection::receive() {
    std::string message;

	if(_moreInBuffer) {
		message = _messagesBuffer[0];
		_messagesBuffer.erase(_messagesBuffer.begin());
		if(_messagesBuffer.empty())
			_moreInBuffer = false;
		return message;
	}


    while (!message.contains(_end)) {

        clearBuffer();

        _sizeOfPreviousMessage = recv(_socket, _buffer, 4096, 0);

        if(_sizeOfPreviousMessage < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != MSG_WAITALL) {
            throw std::runtime_error("Could not receive message from server: " + std::string(strerror(errno)));
        }

        message += _buffer;
    }

	printf("RECEIVE | %s\n", message.c_str());

	// remove the _end string
	message = message.substr(0, message.find(_end));

	if(_encrypted)
		_secretOpen(message);

	//std::cout << "RECEIVE BEFORE PARSE | " << message << std::endl;

	//split the message into potential multiple messages and store them in _messagesBuffer
	_messagesBuffer = [&]() {
		std::vector<std::string> messages;
		std::string tmpMessage;
		size_t pos = 0;
		bool first = true;

		while ((pos = message.find(_end)) != std::string::npos) {
			if(first){
				tmpMessage = message.substr(0, pos);
				first = false;
			}
			else
				messages.push_back(message.substr(0, pos));
			message.erase(0, pos + strlen(_end));
		}
		if(first)
			tmpMessage = message;
		else
			messages.push_back(message);

		message = tmpMessage;
		return messages;
	}();

	if(_messagesBuffer.empty())
		_moreInBuffer = false;
	else
		_moreInBuffer = true;

	std::cout << "RECEIVE | " << message << std::endl;
	std::cout << "RECEIVE BUFFER | "
			  << std::accumulate(_messagesBuffer.begin(), _messagesBuffer.end(), std::string(),
									[](const std::string & a, const std::string & b) {
										return a + b + " | ";
									})
			  << std::endl;

	if(message.contains(_internal"publicKey:")) {
		std::string publicKey = message.substr(strlen(_internal"publicKey:"));

		if(sodium_hex2bin(_remotePublicKey, crypto_box_PUBLICKEYBYTES, publicKey.c_str(), publicKey.size(), nullptr, nullptr, nullptr) < 0){
			throw std::runtime_error("Could not decode public key");
		}

		auto pk_hex = std::make_unique<char[]>(crypto_box_PUBLICKEYBYTES * 2 + 1);

		sodium_bin2hex(pk_hex.get(), crypto_box_PUBLICKEYBYTES * 2 + 1,
						  _keyPair.publicKey, crypto_box_PUBLICKEYBYTES);

		//std::cout << "public key: " << pk_base64.get() << std::endl;

		send(_internal"publicKey:" + std::string(pk_hex.get(), crypto_box_PUBLICKEYBYTES * 2));

		_encrypted = true;
	}

    return message;
}

void Connection::close() {
    //send(_internal"exit");
    shutdown(_socket, 0);
    _active = false;
}

Connection::~Connection() {
    if(_active)
        close();
}

void Connection::clearBuffer() {

	memset(_buffer, '\0', _sizeOfPreviousMessage);

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

void Connection::_secretSeal(std::string & message) {

	auto cypherText = std::make_unique<unsigned char[]>(crypto_box_SEALBYTES + message.size());

	if(crypto_box_seal(cypherText.get(), reinterpret_cast<const unsigned char*>(message.c_str()),
					   message.size(), _remotePublicKey) < 0)
		throw std::runtime_error("Could not encrypt message");

	auto messageHex = std::make_unique<unsigned char[]>((crypto_box_SEALBYTES + message.size()) * 2 + 1);

	sodium_bin2hex(reinterpret_cast<char *>(messageHex.get()), (crypto_box_SEALBYTES + message.size()) * 2 + 1,
				   cypherText.get(), crypto_box_SEALBYTES + message.size());

	message = std::string(reinterpret_cast<char *>(messageHex.get()), (crypto_box_SEALBYTES + message.size()) * 2);
}

void Connection::_secretOpen(std::string & message) {

	auto cypherTextBin = std::make_unique<unsigned char[]>(message.size() / 2);

	if(sodium_hex2bin(cypherTextBin.get(), message.size() / 2,
					  reinterpret_cast<const char *>(message.c_str()), message.size(),
					  nullptr, nullptr, nullptr) < 0)
		throw std::runtime_error("Could not decode message");

	auto decrypted = std::make_unique<unsigned char[]>(message.size()/2 - crypto_box_SEALBYTES);

	if(crypto_box_seal_open(decrypted.get(), cypherTextBin.get(), message.size() / 2, _keyPair.publicKey, _keyPair.secretKey) < 0)
		throw std::runtime_error("Could not decrypt message");

	message = std::string(reinterpret_cast<char*>(decrypted.get()), message.size()/2 - crypto_box_SEALBYTES);
}