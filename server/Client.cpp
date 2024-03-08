#include "Client.h"

Client::Client(int socket, std::vector<std::pair<std::string, std::string>> & messages) :
                socket(socket), messages(messages){}

int Client::getSocket() const {
    return socket;
}

Client::~Client() {
    shutdown(socket, 0);
}

void Client::initConnection() {
    sendMessage("::name");
    std::cout << socket << ": sent name request" << std::endl;
    receiveMessage();
    name = buffer;
    std::cout << socket << ": name: " << name << std::endl;
    sendMessage("::nameAck");
    std::cout << socket << "/" + name << ": sent nameAck" << std::endl;
    active = true;
}

void Client::operator()(std::mutex & messagesMutex) {
    initConnection();



    while(active) {


    }
}

void Client::exit() {
    if(!active) {
        return;
    }
    sendMessage("exit");
    std::cout << socket << "/" + name << ": sent exit" << std::endl;
    active = false;
}

bool Client::isActive() const {
    return active;
}

void Client::clearBuffer() {
    for(int i = 0; i < sizeOfPreviousMessage; i++) {
        buffer[i] = '\0';
    }
}

void Client::receiveMessage() {
    clearBuffer();

    sizeOfPreviousMessage = recv(socket, buffer, 4096, 0);

    if(sizeOfPreviousMessage < 0) {
        throw std::runtime_error("client disconnected");
    }

    message = buffer;

    std::cout << "receive|::" << socket << ( name.empty() ? "" : "/" + name )<< ": " << message << std::endl;

    if(message == "exit") {
        std::cout << "user " << name << " with socket " << socket << " disconnected" << std::endl;
        active = false;
    }

}

void Client::sendMessage(const std::string & message) const {
    if(::send(socket, message.c_str(), message.length(), 0) < 0) {
        throw std::runtime_error("Could not send message");
    }
}


