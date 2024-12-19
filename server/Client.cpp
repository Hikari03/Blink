#include "Client.h"

Client::Client(ClientInfo clientInfo, SharedResources & sharedResources) :
		_clientInfo(std::move(clientInfo)), _sharedResources(sharedResources), _messagesMutex(_sharedResources.getMessagesMutex()), _callBackOnMessagesChange(_sharedResources.getCallback()) {}

Client::~Client() {
    exit();
}

void Client::initConnection() {

	// Set timeout for recv
	struct timeval timeout;
	timeout.tv_sec = 5;  // Timeout in seconds
	timeout.tv_usec = 0; // Timeout in microseconds

	if (setsockopt(_clientInfo.socket_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		throw std::runtime_error("setsockopt failed");
	}

	std::cout << "initializing encryption with client " << _clientInfo.socket_ << std::endl;
	if(sodium_init() < 0)
		throw std::runtime_error("Could not initialize sodium");

	if(crypto_box_keypair(_keyPair.publicKey, _keyPair.secretKey) < 0)
		throw std::runtime_error("Could not generate keypair");

	auto pk_hex = std::make_unique<char[]>(crypto_box_PUBLICKEYBYTES * 2 + 1);

	sodium_bin2hex(pk_hex.get(), crypto_box_PUBLICKEYBYTES * 2 + 1,
					  _keyPair.publicKey, crypto_box_PUBLICKEYBYTES);

	std::cout << "public key: " << pk_hex.get() << std::endl;

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	sendMessage(_internal"publicKey:" + std::string(pk_hex.get(), crypto_box_PUBLICKEYBYTES * 2));

	receiveMessage();

	if(!_message.contains(_internal"publicKey:"))
		throw std::runtime_error("Could not receive pubKey");

	auto pubKey_hex = _message.substr(strlen(_internal"publicKey:"));

	if(sodium_hex2bin(_remotePublicKey, crypto_box_PUBLICKEYBYTES, pubKey_hex.c_str(), pubKey_hex.size(), nullptr, nullptr, nullptr) < 0){
		throw std::runtime_error("Could not decode public key");
	}

	//std::cout << "pubKey: " << _remotePublicKey << std::endl;

	_encrypted = true;

    sendMessage(_internal"name");
    //std::cout << socket_ << ": sent name request" << std::endl;
    receiveMessage();
    _clientInfo.name = _message;
    ///std::cout << socket_ << ": name: " << name << std::endl;
    sendMessage(_internal"nameAck");

	//sendMessage(_internal );
    //std::cout << socket_ << "/" + name << ": sent nameAck" << std::endl;
	printf("client %d/%s connected\n", _clientInfo.socket_, _clientInfo.name.c_str());
    _active = true;

	// unset timeout since we are done with initialization => valid connection
	timeout.tv_sec = 0;

	if (setsockopt(_clientInfo.socket_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		throw std::runtime_error("setsockopt failed");
	}
}


void Client::run() {
    try {
        initConnection();

		std::thread sendThread(&Client::sendThread, this);
        std::thread receiveThread(&Client::receiveThread, this);

		_sharedResources.setUserAsOnline(_clientInfo.name);
		_sharedResources.addMessage({_clientInfo.name, "joined the chat", std::chrono::system_clock::now()});

		sendThread.join();
        receiveThread.join();

		_sharedResources.setUserAsOffline(_clientInfo.name);
        _sharedResources.addMessage({_clientInfo.name, "left the chat", std::chrono::system_clock::now()});
    }
    catch (std::runtime_error & e) {
        std::cout << _clientInfo.socket_ << "/" + _clientInfo.name << " EXCEPTION |  " << e.what() << std::endl;
        _active = false;
    }

    printf("client %d/%s closed\n", _clientInfo.socket_, _clientInfo.name.c_str());
    _active = false;
}


void Client::exit() {
    if(!_active) {
        return;
    }
    std::cout << _clientInfo.socket_ << "/" + _clientInfo.name << ": sent exit" << std::endl;
    sendMessage(_internal"exit");
    _active = false;
	shutdown(_clientInfo.socket_, SHUT_RDWR);
	close(_clientInfo.socket_);
}


bool Client::isActive() const {
    return _active;
}


void Client::clearBuffer() {
    for(int i = 0; i < _sizeOfPreviousMessage; i++) {
        _buffer[i] = '\0';
    }
}


void Client::receiveMessage() {
    _message.clear();

    while(!_message.contains(_end)) {

        clearBuffer();

        _sizeOfPreviousMessage = recv(_clientInfo.socket_, _buffer, 4096, 0);


        if(_sizeOfPreviousMessage < 0) {
        	if (errno != EAGAIN && errno != EWOULDBLOCK)
				throw std::runtime_error("client disconnected or could not receive message");

        	if(errno == EAGAIN || errno == EWOULDBLOCK)
        		throw std::runtime_error("timeout");
        }

        _message += _buffer;

		//std::cout << "RECEIVE |  " << _clientInfo.socket_ << (_clientInfo.name.empty() ? "" : "/" + _clientInfo.name ) << ": " << _message << std::endl;

	}

    //std::cout << "RECEIVE |  " << _clientInfo.socket_ << (_clientInfo.name.empty() ? "" : "/" + _clientInfo.name ) << ": " << _message << std::endl;

	// cut off the _end
	_message = _message.substr(0, _message.find(_end));

	if(_encrypted)
		secretOpen(_message);

}


void Client::sendMessage(const std::string & message) {
	auto messageToSend = message;

	//std::cout << "SEND1 |  " << _clientInfo.socket_ << (_clientInfo.name.empty() ? "" : "/" + _clientInfo.name ) << ": " << messageToSend << std::endl;

	if(_encrypted) {
		secretSeal(messageToSend);

	}
	messageToSend += _end;

    //std::cout << "SEND2 |  " << _clientInfo.socket_ << (_clientInfo.name.empty() ? "" : "/" + _clientInfo.name ) << ": " << messageToSend << std::endl;
	if(!_active)
		return;
	try {
		if (::send(_clientInfo.socket_, messageToSend.c_str(), messageToSend.length(), 0) < 0) {
			throw std::runtime_error("Could not send message to client");
		}
	}
	catch (std::exception & e) {
		throw std::runtime_error("Could not send message to client");
	}
}


void Client::submitMessage(const std::string & message) {
	printf("client %d/%s: %s\n", _clientInfo.socket_, _clientInfo.name.c_str(), message.c_str());
    _sharedResources.addMessage({_clientInfo.name, message, std::chrono::system_clock::now()});
}


void Client::processMessage() {

    if(_message == _internal"exit") {
        std::cout << "user " << _clientInfo.name << " with socket " << _clientInfo.socket_ << " disconnected" << std::endl;
        sendMessage(_internal"exitAck");
        _active = false;
        return;
    }

    if(_message == _internal"ping") {
        sendMessage(_internal"pong");
        return;
    }

	if(_message == _internal"pong") {
		return;
	}

	if(_message == _internal"getMessages") { // this is for ncurses client support, don't change the value in serializeMessages!
		sendMessage(_text + _sharedResources.serializeMessages(17));
		return;
	}

	// returns all messages without the oldest one
	// this is because client gets "xxx joined the server" before he calls this
	if(_message == _internal"getHistory") {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		auto messages = _sharedResources.serializeMessages(_sharedResources.getMessagesCount());
		// remove the last message
		messages = messages.substr(messages.find_first_of('\n'));
		//remove first and last char
		messages = messages.substr(1, messages.size()-1);

		//std::cout << "sending history to " << _clientInfo.socket_ << "/" + _clientInfo.name << std::endl;
		//std::cout << "\"" << messages << "\"" << std::endl; //debug

		sendMessage(_text + messages);
		return;
	}

	if(_message == _internal"getAllMessages") {
		sendMessage(_text + _sharedResources.serializeMessages(_sharedResources.getMessagesCount()));
		return;
	}

	if(_message == _internal"ban") {
		std::cout << "user " << _clientInfo.name << " with socket " << _clientInfo.socket_ << " banned" << std::endl;
		_active = false;
		return;
	}

    if(_message.contains(_text)){
        //std::cout << "submitting message: " << _message.substr(sizeof(_text)-1, _message.length()) << std::endl;
        submitMessage(_message.substr(sizeof(_text)-1, _message.length()));
        return;
    }
}

void Client::sendThread() {
    std::unique_lock<std::mutex> lock(_messagesMutex);

	// send all messages on start
	if(_active) {
		sendMessage(_text + _sharedResources.serializeMessages(_sharedResources.getMessagesCount()));
		auto onlineUsers = [&]() {
			std::string users;
			for(const auto & user : _sharedResources.getOnlineUsers()) {
				users += user + ",";
			}
			return users;
		}();
		sendMessage(_internal"onlineUsers:" + onlineUsers);
	}

    while(_active) {
        _callBackOnMessagesChange.wait(lock);
		if(_active) { // if we have already kicked client, we cant send or else segfault (edge case)
			sendMessage(_text + _sharedResources.serializeMessages(1));
		}
			//std::cout << "onlineUsers: " << _sharedResources.getOnlineUsers().size() << std::endl;
		auto onlineUsers = [&]() {
			std::string users;
			for(const auto & user : _sharedResources.getOnlineUsers()) {
				users += user + ",";
			}
			return users;
		}();
		//std::cout << "onlineUsers: " << onlineUsers << std::endl;
		if (_active)
			sendMessage(_internal"onlineUsers:" + onlineUsers);

    }


}

void Client::receiveThread() {

    try {
        while(_active) {
            receiveMessage();
            processMessage();
        }
    }
    catch (std::runtime_error & e) {
        if(_active) {
            std::cout << _clientInfo.socket_ << "/" + _clientInfo.name << " EXCEPTION |  " << e.what() << std::endl;
            _active = false;
        }
    }
	_callBackOnMessagesChange.notify_all();

}

const ClientInfo &Client::info() const {
	return _clientInfo;
}

void Client::secretSeal(std::string & message) {

	auto cypherText = std::make_unique<unsigned char[]>(crypto_box_SEALBYTES + message.size());

	if(crypto_box_seal(cypherText.get(), reinterpret_cast<const unsigned char*>(message.c_str()),
					   message.size(), _remotePublicKey) < 0)
		throw std::runtime_error("Could not encrypt message");

	auto messageHex = std::make_unique<unsigned char[]>((crypto_box_SEALBYTES + message.size()) * 2 + 1);

	sodium_bin2hex(reinterpret_cast<char *>(messageHex.get()), (crypto_box_SEALBYTES + message.size()) * 2 + 1,
				   cypherText.get(), crypto_box_SEALBYTES + message.size());

	message = std::string(reinterpret_cast<char *>(messageHex.get()), (crypto_box_SEALBYTES + message.size()) * 2);
}

void Client::secretOpen(std::string & message) {

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