#include <iostream>
#include <netinet/in.h>

#include "Client.h"
#include "accepter.cpp"
#include "cleaner.cpp"
#include "terminal.cpp"
#include "Message.h"

std::mutex clientsMutex;
std::mutex messagesMutex;

int main() {

    std::cout << "main: starting server" << std::endl;

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress = {AF_INET,
                                 htons(6999),
                                 INADDR_ANY};

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    std::vector<std::thread> clientRunners;
    // holds chat
    std::vector<Message> messages;
    std::list<Client> clients;

    bool turnOff = false;

    std::thread terminalThread(terminal, std::ref(turnOff));
    std::thread cleanerThread(cleaner, std::ref(clients), std::ref(clientRunners), std::ref(turnOff), std::ref(clientsMutex));

    listen(serverSocket, 10);

    bool newClientAccepted = false;
    int acceptedSocket = 0;

    std::thread accepterThread(accepter, std::ref(serverSocket), std::ref(acceptedSocket), std::ref(newClientAccepted), std::ref(turnOff));

    std::cout << "main: entering main loop" << std::endl;

    /*
     * ########################################################################################
     * ################################## MAIN LOOP ###########################################
     * ########################################################################################
     */

    while(true) {

        // critical section - clients
        if(newClientAccepted) {
            std::lock_guard<std::mutex> lock(clientsMutex);

            // create client
            clients.emplace_back(acceptedSocket, messages, messagesMutex);

            // run client (its functor)
            clientRunners.emplace_back(clients.back(), std::ref(messagesMutex));

            newClientAccepted = false;
        }


        if(turnOff) {
            // cleanup
            std::cout << "main: cleaning up threads" << std::endl;
            terminalThread.join();
            std::cout << "main: terminal closed" << std::endl;
            accepterThread.join();
            std::cout << "main: accepter closed" << std::endl;
            cleanerThread.join();
            std::cout << "main: cleaner closed" << std::endl;
            for(auto & client : clientRunners) {
                client.join();
                std::cout << "main: client closed" << std::endl;
            }
            break;
        }

    }

    shutdown(serverSocket, 0);
}