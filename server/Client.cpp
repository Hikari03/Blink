#include "Client.h"

Client::Client(int socket, std::vector<Message> & messages, std::mutex & messagesMutex) :
        _socket(socket), _messages(messages), _messagesMutex(messagesMutex){}

int Client::getSocket() const {
    return _socket;
}

Client::~Client() {
    exit();
    shutdown(_socket, 0);
}

void Client::initConnection() {
    sendMessage("::name");
    std::cout << _socket << ": sent name request" << std::endl;
    receiveMessage();
    _name = _buffer;
    std::cout << _socket << ": name: " << _name << std::endl;
    sendMessage("::nameAck");
    std::cout << _socket << "/" + _name << ": sent nameAck" << std::endl;
    _active = true;
}

void Client::operator()() {
    try {
        initConnection();


        while (_active) {
            receiveMessage();
            submitReceivedMessage();

        }
    }
    catch (std::runtime_error & e) {
        std::cout << _socket << "/" + _name << " EXCEPTION:" << e.what() << std::endl;
        _active = false;
    }
}

void Client::exit() {
    if(!_active) {
        return;
    }
    sendMessage("exit");
    std::cout << _socket << "/" + _name << ": sent exit" << std::endl;
    _active = false;
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
    clearBuffer();

    _sizeOfPreviousMessage = recv(_socket, _buffer, 4096, 0);

    if(_sizeOfPreviousMessage < 0) {
        throw std::runtime_error("client disconnected");
    }

    _message = _buffer;

    std::cout << "receive|::" << _socket << (_name.empty() ? "" : "/" + _name ) << ": " << _message << std::endl;

    if(_message == "exit") {
        std::cout << "user " << _name << " with socket " << _socket << " disconnected" << std::endl;
        _active = false;
    }

}

void Client::sendMessage(const std::string & message) const {
    if(::send(_socket, message.c_str(), message.length(), 0) < 0) {
        throw std::runtime_error("Could not send message");
    }
}


void Client::submitReceivedMessage() {
    std::lock_guard<std::mutex> lock(_messagesMutex);
    _messages.emplace_back(_name, _message, std::chrono::system_clock::now());
}


