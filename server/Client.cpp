#include "Client.h"

Client::Client(ClientInfo clientInfo, MessageHolder & messages) :
        _clientInfo(std::move(clientInfo)), _messages(messages), _messagesMutex(_messages.getMessagesMutex()), _callBackOnMessagesChange(_messages.getCallback()) {}



Client::~Client() {
    exit();
}


void Client::initConnection() {

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
    //std::cout << socket_ << "/" + name << ": sent nameAck" << std::endl;
	printf("client %d/%s connected\n", _clientInfo.socket_, _clientInfo.name.c_str());
    _active = true;
}


void Client::run() {
    try {
        initConnection();

        std::thread sendThread(&Client::sendThread, this);
        std::thread receiveThread(&Client::receiveThread, this);
		_messages.addMessage({_clientInfo.name, "joined the chat", std::chrono::system_clock::now()});

        sendThread.join();
        receiveThread.join();

        _messages.addMessage({_clientInfo.name, "left the chat", std::chrono::system_clock::now()});

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


        if(_sizeOfPreviousMessage < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            throw std::runtime_error("client disconnected or could not receive message");
        }

        _message += _buffer;

		std::cout << "RECEIVE |  " << _clientInfo.socket_ << (_clientInfo.name.empty() ? "" : "/" + _clientInfo.name ) << ": " << _message << std::endl;

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
    if(::send(_clientInfo.socket_, messageToSend.c_str(), messageToSend.length(), 0) < 0) {
        throw std::runtime_error("Could not send message to client");
    }
}


void Client::submitMessage(const std::string & message) {
	printf("client %d/%s: %s\n", _clientInfo.socket_, _clientInfo.name.c_str(), message.c_str());
    _messages.addMessage({_clientInfo.name, message, std::chrono::system_clock::now()});
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

	if(_message == _internal"getMessages") {
		sendMessage(_text + _messages.serializeMessages(17));
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
    while(_active) {
        _callBackOnMessagesChange.wait(lock);
		if(_active) // if we have already kicked client, we cant send or else segfault (edge case)
        	sendMessage(_text + _messages.serializeMessages(17));
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

	auto cypherText_bin = std::make_unique<unsigned char[]>(message.size()/2);

	if(sodium_hex2bin(cypherText_bin.get(), message.size() / 2,
					  reinterpret_cast<const char *>(message.c_str()),message.size(),
					  nullptr, nullptr, nullptr) < 0)
		throw std::runtime_error("Could not decode message");

	auto decrypted = std::make_unique<unsigned char[]>(message.size()/2 - crypto_box_SEALBYTES);

	if(crypto_box_seal_open(decrypted.get(), cypherText_bin.get(), message.size() / 2, _keyPair.publicKey, _keyPair.secretKey) < 0)
		throw std::runtime_error("Could not decrypt message");

	message = std::string(reinterpret_cast<char*>(decrypted.get()), message.size()/2 - crypto_box_SEALBYTES);
}