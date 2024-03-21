
#include "Client.h"

Client::Client(int socket, std::set<Message> & messages, std::mutex & messagesMutex) :
        _socket(socket), _messages(messages), _messagesMutex(messagesMutex){}


int Client::getSocket() const {
    return _socket;
}


Client::~Client() {
    exit();
    shutdown(_socket, 0);
}


void Client::initConnection() {
    sendMessage(_internal"name");
    std::cout << _socket << ": sent name request" << std::endl;
    receiveMessage();
    _name = _message;
    std::cout << _socket << ": name: " << _name << std::endl;
    sendMessage(_internal"nameAck");
    std::cout << _socket << "/" + _name << ": sent nameAck" << std::endl;
    _active = true;
}


void Client::operator()() {
    try {
        initConnection();

        std::thread sendThread(&Client::sendThread, this);
        std::thread receiveThread(&Client::receiveThread, this);

        sendThread.join();
        receiveThread.join();

    }
    catch (std::runtime_error & e) {
        std::cout << _socket << "/" + _name << " EXCEPTION |  " << e.what() << std::endl;
        _active = false;
    }
}


void Client::exit() {
    if(!_active) {
        return;
    }
    std::cout << _socket << "/" + _name << ": sent exit" << std::endl;
    sendMessage(_internal"exit");
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
    _message.clear();

    while(!_message.contains(_end)) {

        clearBuffer();

        _sizeOfPreviousMessage = recv(_socket, _buffer, 4096, MSG_DONTWAIT);

        if(_sizeOfPreviousMessage < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            throw std::runtime_error("client disconnected or could not receive message");
        }

        _message += _buffer;
    }

    std::cout << "RECEIVE |  " << _socket << (_name.empty() ? "" : "/" + _name ) << ": " << _message << std::endl;

    // cut off the _end

    _message = _message.substr(0, _message.find(_end));

}


void Client::sendMessage(const std::string & message) const {
    auto messageToSend = message + _end;
    if(::send(_socket, messageToSend.c_str(), messageToSend.length(), 0) < 0) {
        throw std::runtime_error("Could not send message to client");
    }
}


void Client::submitMessage(const std::string & message) {
    std::lock_guard<std::mutex> lock(_messagesMutex);
    _messages.emplace(_name, _message, std::chrono::system_clock::now());
}


void Client::processMessage() {

    if(_message == _internal"exit") {
        std::cout << "user " << _name << " with socket " << _socket << " disconnected" << std::endl;
        _active = false;
        return;
    }

    if(_message == _internal"ping") {
        sendMessage(_internal"pong");
        return;
    }

    if(_message.contains(_text)){
        std::cout << "user " << _name << " with socket " << _socket << " sent message: " << _message << std::endl;
        submitMessage(_message.substr(_message.find(_text), _message.length() - strlen(_text)));
        return;
    }

}

void Client::sendThread() {

    /*while(_active) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        sendMessage(_internal"ping");
    }*/


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
            std::cout << _socket << "/" + _name << " EXCEPTION |  " << e.what() << std::endl;
            _active = false;
        }
    }

}