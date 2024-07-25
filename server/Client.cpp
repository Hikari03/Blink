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

	unsigned char pk[crypto_box_PUBLICKEYBYTES];
	unsigned char sk[crypto_box_SECRETKEYBYTES];

	if(crypto_box_keypair(pk, sk) < 0)
		throw std::runtime_error("Could not generate keypair");

	auto pk_base64 = std::make_unique<char[]>(sodium_base64_encoded_len(crypto_box_PUBLICKEYBYTES, sodium_base64_VARIANT_ORIGINAL));
	auto pk_base64_len = sodium_base64_encoded_len(crypto_box_PUBLICKEYBYTES, sodium_base64_VARIANT_ORIGINAL);

	sodium_bin2base64(pk_base64.get(), pk_base64_len,
					  pk, crypto_box_PUBLICKEYBYTES, sodium_base64_VARIANT_ORIGINAL);

	std::cout << "public key: " << pk_base64.get() << std::endl;

	std::this_thread::sleep_for(std::chrono::milliseconds(100));


	sendMessage(_internal"publicKey:" + std::string(pk_base64.get(), pk_base64_len-1));

	receiveMessage();

	if(!_message.contains(_internal"symKey:"))
		throw std::runtime_error("Could not receive symKey");

	auto symKey_b64 = _message.substr(strlen(_internal"symKey:"));

	auto symKey_bin = std::make_unique<unsigned char[]>(crypto_box_SEALBYTES + crypto_secretbox_KEYBYTES);

	if(sodium_base642bin(symKey_bin.get(), crypto_box_SEALBYTES + crypto_secretbox_KEYBYTES, symKey_b64.c_str(), symKey_b64.size(), nullptr, nullptr, nullptr, sodium_base64_VARIANT_ORIGINAL) < 0)
		throw std::runtime_error("Could not decode symKey");

	if(crypto_box_seal_open(_symKey, symKey_bin.get(), crypto_box_SEALBYTES + crypto_secretbox_KEYBYTES, pk, sk) < 0)
		throw std::runtime_error("Could not open sealed symKey");

	std::cout << "symKey: " << _symKey << std::endl;

	receiveMessage();

	if(!_message.contains(_internal"nonce:"))
		throw std::runtime_error("Could not receive nonce");

	auto nonce_b64 = _message.substr(strlen(_internal"nonce:"));

	if(sodium_base642bin(_nonce, crypto_secretbox_NONCEBYTES, nonce_b64.c_str(), nonce_b64.size(), nullptr, nullptr, nullptr, sodium_base64_VARIANT_ORIGINAL) < 0)
		throw std::runtime_error("Could not decode nonce");

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

	if(_encrypted)
		secretOpen(_message);

    // cut off the _end
    _message = _message.substr(0, _message.find(_end));

}


void Client::sendMessage(const std::string & message) {
	auto messageToSend = message;

	if(_encrypted) {
		secretSeal(messageToSend);

	}
	messageToSend += _end;

    std::cout << "SEND |  " << _clientInfo.socket_ << (_clientInfo.name.empty() ? "" : "/" + _clientInfo.name ) << ": " << messageToSend << std::endl;
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

	auto cypherText = std::make_unique<unsigned char[]>(crypto_secretbox_MACBYTES + message.size());

	if(crypto_secretbox_easy(cypherText.get(), (unsigned char *)message.c_str(), message.size(), _nonce, _symKey) < 0)
		throw std::runtime_error("Could not encrypt message");

	auto cypherText_b64 = std::make_unique<char[]>(sodium_base64_encoded_len(crypto_secretbox_MACBYTES + message.size(), sodium_base64_VARIANT_ORIGINAL));

	sodium_bin2base64(cypherText_b64.get(), sodium_base64_encoded_len(crypto_secretbox_MACBYTES + message.size(), sodium_base64_VARIANT_ORIGINAL),
					  cypherText.get(), crypto_secretbox_MACBYTES + message.size(), sodium_base64_VARIANT_ORIGINAL);

	message = std::string(cypherText_b64.get(), sodium_base64_encoded_len(crypto_secretbox_MACBYTES + message.size(), sodium_base64_VARIANT_ORIGINAL)-1);
}

void Client::secretOpen(std::string & message) {

	auto cypherText_bin = std::make_unique<unsigned char[]>(message.size());

	if(sodium_base642bin(cypherText_bin.get(), message.size(), message.c_str(), message.size(), nullptr, nullptr, nullptr, sodium_base64_VARIANT_ORIGINAL) < 0)
		throw std::runtime_error("Could not decode message");

	auto plainText = std::make_unique<char[]>(message.size() / 4 * 3 - crypto_secretbox_MACBYTES);

	if(crypto_secretbox_open_easy((unsigned char *)plainText.get(), cypherText_bin.get(), message.size()/ 4 * 3 - crypto_secretbox_MACBYTES, _nonce, _symKey) < 0)
		throw std::runtime_error("Could not decrypt message");

	message = std::string(plainText.get(), message.size() / 4 * 3 - crypto_secretbox_MACBYTES);
}