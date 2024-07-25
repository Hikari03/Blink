#include "Connection.h"

Connection::Connection() {
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket == -1) {
        throw std::runtime_error("Could not create socket");
    }

	if (sodium_init() < 0) {
		throw std::runtime_error("Could not initialize sodium");
	}

	crypto_secretbox_keygen(_symKey);
	randombytes_buf(_nonce, sizeof _nonce);

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


    while (!message.contains(_end)) {

        clearBuffer();

        _sizeOfPreviousMessage = recv(_socket, _buffer, 4096, 0);

        if(_sizeOfPreviousMessage < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != MSG_WAITALL) {
            throw std::runtime_error("Could not receive message from server: " + std::string(strerror(errno)));
        }

        message += _buffer;
		std::cout << "RECEIVE |  " << message << std::endl;
    }


	if(_encrypted)
		_secretOpen(message);

    // remove the _end string
    message = message.substr(0, message.find(_end));

	if(message.contains(_internal"publicKey:")){
		std::string publicKey = message.substr(strlen(_internal"publicKey:"));

		auto publicKey_c = publicKey.c_str();

		unsigned char publicKey_bin[crypto_box_PUBLICKEYBYTES];

		if(sodium_base642bin(publicKey_bin, crypto_box_PUBLICKEYBYTES, publicKey_c, publicKey.size(), nullptr, nullptr, nullptr, sodium_base64_VARIANT_ORIGINAL) < 0){
			throw std::runtime_error("Could not decode public key");
		}

		unsigned char sealedSymKey[crypto_box_SEALBYTES + crypto_secretbox_KEYBYTES];

		crypto_box_seal(sealedSymKey, _symKey, crypto_secretbox_KEYBYTES, publicKey_bin);

		std::string sealedSymKey_b64(sodium_base64_encoded_len(crypto_box_SEALBYTES + crypto_secretbox_KEYBYTES, sodium_base64_VARIANT_ORIGINAL), '\0');

		sodium_bin2base64(sealedSymKey_b64.data(), sealedSymKey_b64.size(), sealedSymKey, crypto_box_SEALBYTES + crypto_secretbox_KEYBYTES, sodium_base64_VARIANT_ORIGINAL);
		sealedSymKey_b64.erase(sealedSymKey_b64.length()-1);
		sendInternal("symKey:" + sealedSymKey_b64);

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		std::string nonce_b64(sodium_base64_encoded_len(crypto_secretbox_NONCEBYTES,sodium_base64_VARIANT_ORIGINAL), '\0');
		sodium_bin2base64(nonce_b64.data(), nonce_b64.size(), _nonce, crypto_secretbox_NONCEBYTES, sodium_base64_VARIANT_ORIGINAL);
		nonce_b64.erase(nonce_b64.length()-1);
		sendInternal("nonce:" + nonce_b64);
		_encrypted = true;
	}

    return message;
}

void Connection::close() {
    send(_internal"exit");
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

	auto cypherText = std::make_unique<unsigned char[]>(crypto_secretbox_MACBYTES + message.size());

	if(crypto_secretbox_easy(cypherText.get(), (unsigned char *)message.c_str(), message.size(), _nonce, _symKey) < 0)
		throw std::runtime_error("Could not encrypt message");

	auto cypherText_b64 = std::make_unique<char[]>(sodium_base64_encoded_len(crypto_secretbox_MACBYTES + message.size(), sodium_base64_VARIANT_ORIGINAL));

	sodium_bin2base64(cypherText_b64.get(), sodium_base64_encoded_len(crypto_secretbox_MACBYTES + message.size(), sodium_base64_VARIANT_ORIGINAL),
					  cypherText.get(), crypto_secretbox_MACBYTES + message.size(), sodium_base64_VARIANT_ORIGINAL);

	message = std::string(cypherText_b64.get(), sodium_base64_encoded_len(crypto_secretbox_MACBYTES + message.size(), sodium_base64_VARIANT_ORIGINAL)-1);
}

void Connection::_secretOpen(std::string & message) {

	auto cypherText_bin = std::make_unique<unsigned char[]>(message.size());

	if(sodium_base642bin(cypherText_bin.get(), message.size(), message.c_str(), message.size(), nullptr, nullptr, nullptr, sodium_base64_VARIANT_ORIGINAL) < 0)
		throw std::runtime_error("Could not decode message");

	auto plainText = std::make_unique<char[]>(message.size() / 4 * 3 - crypto_secretbox_MACBYTES);

	if(crypto_secretbox_open_easy((unsigned char *)plainText.get(), cypherText_bin.get(), message.size()/ 4 * 3 - crypto_secretbox_MACBYTES, _nonce, _symKey) < 0)
		throw std::runtime_error("Could not decrypt message");

	message = std::string(plainText.get(), message.size() / 4 * 3 - crypto_secretbox_MACBYTES);
}