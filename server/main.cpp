#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <vector>

void clearBuffer(char * buffer, int size) {
    for(int i = 0; i < size; i++) {
        buffer[i] = '\0';
    }
}

bool clientServer(int clientSocket) {
    const std::string msg = "::name";
    send(clientSocket, msg.c_str(), msg.size(), 0);
    std::cout << "sent name request to: " << clientSocket << std::endl;

    char buffer[4096] = {0};
    int bufferOccupiedSize = recv(clientSocket, buffer, 4096, 0);

    std::string clientName = std::string(buffer);
    std::cout << "name: " << clientName << std::endl;

    send(clientSocket, "::nameAck", 9, 0);
    std::cout << "sent nameAck to: " << clientName << std::endl;

    std::string message;

    clearBuffer(buffer, bufferOccupiedSize);

    recv(clientSocket, buffer, 4096, 0);
    message = std::string(buffer);
    std::cout << "message: " << message << std::endl;
    if(message == "exit"){
        std::cout << "user " << clientName << " with socket " << clientSocket << " disconnected" << std::endl;
        shutdown(clientSocket, 0);
        return 0;
    }

    std::cout << "exiting without exit call on socket " << clientSocket << " and username " << clientName << std::endl;

    shutdown(clientSocket, 0);
    return 0;
}

bool terminal(bool & turnOff){
    while(true) {
        std::string input;
        std::cout << "Type 'q' to turn off server" << std::endl;
        input = std::cin.get();
        if(input == "q") {
            turnOff = 1;
            return 0;
        }
    }
}

int main() {

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(6999);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    int nOfClients = 0;
    std::vector<std::thread> clients;

    bool turnOff = false;

    std::thread terminalThread(terminal, std::ref(turnOff));

    while(true) {
        listen(serverSocket, 5);

        int clientSocket = accept(serverSocket, nullptr, nullptr);

        std::cout << "accepted client number " << nOfClients << std::endl;

        clients.push_back(std::thread(clientServer, clientSocket));

        nOfClients++;

        if(turnOff)
            break;

    }

    shutdown(serverSocket, 0);
}