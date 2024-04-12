#include <iostream>
#include <netinet/in.h>

#include "Client.h"
#include "accepter.h"
#include "cleaner.h"
#include "terminal.h"
#include "Message.h"
#include "MessageHolder.h"


int main() {

    std::mutex clientsMutex;
    std::mutex messagesMutex;

    std::cout << "main: starting server" << std::endl;

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress = {AF_INET,
                                 htons(6999),
                                 INADDR_ANY};

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    std::vector<std::jthread> clientRunners;
    // holds chat
    MessageHolder messages(messagesMutex);
    std::list<Client> clients;
    std::condition_variable callBack;

    bool turnOff = false;

    std::thread terminalThread(terminal, std::ref(callBack), std::ref(turnOff));
    std::thread cleanerThread(cleaner, std::ref(clients), std::ref(turnOff), std::ref(clientsMutex));

    listen(serverSocket, 10);

    bool newClientAccepted = false;
    int acceptedSocket = 0;


    std::thread accepterThread(accepter, std::ref(callBack), std::ref(serverSocket), std::ref(acceptedSocket), std::ref(newClientAccepted), std::ref(turnOff));

    std::cout << "main: entering main loop" << std::endl;

    /*
     * ########################################################################################
     * ################################## MAIN LOOP ###########################################
     * ########################################################################################
     */

    while(true) {

        // critical section - clients

        std::unique_lock<std::mutex> lock(clientsMutex);
        callBack.wait(lock);

        if(newClientAccepted) {

            // create client
            clients.emplace_back(acceptedSocket, messages);

            // run client (its functor)
            clientRunners.emplace_back(&Client::run, &clients.back());

            newClientAccepted = false;
        }



        if(turnOff) {
            // cleanup
            lock.unlock();
            std::cout << "main: cleaning up threads" << std::endl;
            terminalThread.join();
            std::cout << "main: terminal closed" << std::endl;
            accepterThread.join();
            std::cout << "main: accepter closed" << std::endl;
            cleanerThread.join();
            std::cout << "main: cleaner closed" << std::endl;
            break;
        }

    }

    std::cout << "main: closing server" << std::endl;

    shutdown(serverSocket, 0);
}