#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <thread>
#include <vector>
#include <list>

#include "Client.h"

std::mutex clientsMutex;
std::mutex messagesMutex;

bool SetSocketBlockingEnabled(int fd, bool blocking)
{
    if (fd < 0) return false;

#ifdef _WIN32
    unsigned long mode = blocking ? 0 : 1;
    return (ioctlsocket(fd, FIONBIO, &mode) == 0);
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return (fcntl(fd, F_SETFL, flags) == 0);
#endif
}

void terminal(bool & turnOff){
    while(true) {
        std::string input;
        std::cout << "Type 'q' to turn off server" << std::endl;
        std::cin >> input;
        if(input == "q") {
            turnOff = true;
            std::cout << "turning off server, please wait up to 1 minute" << std::endl;
            return;
        }
    }
}

void cleaner(std::list<Client> & clients, std::vector<std::thread> & clientRunners, const bool & turnOff) {
    while(!turnOff) {

        std::this_thread::sleep_for(std::chrono::seconds(30));


        // critical section - clients
        {
            std::lock_guard<std::mutex> lock(clientsMutex);

            auto it = clients.begin();

            for (long unsigned int i = 0; i < clients.size(); i++, it++) {

                if (!(*it).isActive()) {
                    clientRunners[i].join();
                    clientRunners.erase(clientRunners.begin() + i);

                    std::cout << "cleaner: client number " << (*it).getSocket() << " removed" << std::endl;

                    clients.erase(it);
                }
            }
        }

    }
}

// accepts new clients
void accepter(const int & serverSocket, int & acceptedSocket, bool & newClientAccepted, const bool & turnOff) {

    //set to non-blocking

    bool blockSuccess = SetSocketBlockingEnabled(serverSocket, false);

    std::cout << "main: serverSocket is: " << (blockSuccess ? "non-blocking" : "blocking") << std::endl;

    while(true) {
        if(turnOff)
            return;

        if(!newClientAccepted) {

            if(blockSuccess) std::this_thread::sleep_for(std::chrono::seconds(1));

            acceptedSocket = accept(serverSocket, nullptr, nullptr);

            if(acceptedSocket < 0) {
                continue;
            }

            newClientAccepted = true;
            std::cout << "main: accepted client number " << acceptedSocket << std::endl;
        }
    }
}

int main() {

    std::cout << "main: starting server" << std::endl;

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress = {AF_INET,
                                 htons(6999),
                                 INADDR_ANY};

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    std::vector<std::thread> clientRunners;
    // holds chat
    std::vector<std::pair<std::string, std::string>> messages;
    std::list<Client> clients;

    bool turnOff = false;

    std::thread terminalThread(terminal, std::ref(turnOff));
    std::thread cleanerThread(cleaner, std::ref(clients), std::ref(clientRunners), std::ref(turnOff));

    listen(serverSocket, 10);

    bool newClientAccepted = false;
    int acceptedSocket = 0;

    std::thread accepterThread(accepter, std::ref(serverSocket), std::ref(acceptedSocket), std::ref(newClientAccepted), std::ref(turnOff));

    std::cout << "main: entering main loop" << std::endl;

    while(true) {

        // critical section - clients
        if(newClientAccepted) {
            std::lock_guard<std::mutex> lock(clientsMutex);

            // create client
            clients.emplace_back(acceptedSocket, messages);

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