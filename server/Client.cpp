
#include "Client.h"

#include <utility>

Client::Client(ClientInfo clientInfo, MessageHolder & messages) :
        _clientInfo(std::move(clientInfo)), _messages(messages), _messagesMutex(_messages.getMessagesMutex()), _callBackOnMessagesChange(_messages.getCallback()) {}



Client::~Client() {
    exit();
}


void Client::initConnection() {
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
    }

    //std::cout << "RECEIVE |  " << socket_ << (name.empty() ? "" : "/" + name ) << ": " << _message << std::endl;

    // cut off the _end
    _message = _message.substr(0, _message.find(_end));

}


void Client::sendMessage(const std::string & message) const {
    auto messageToSend = message + _end;
    //std::cout << "SEND |  " << socket_ << (name.empty() ? "" : "/" + name ) << ": " << messageToSend << std::endl;
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

